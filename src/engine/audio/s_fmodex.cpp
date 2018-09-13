/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2008 by wad
This file is part of q3++ source code.

q3++ source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

q3++ source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include <string>
#include <vector>
#include <queue>
#include <map>
#include "../snd_public.h"
#include "../../client/client.h"
#include "../FmodEx/fmod.hpp"
#include "../FmodEx/fmod_errors.h"
#ifdef _DEBUG
//#pragma comment(lib, "../FmodEx/fmodexL_vc.lib")		// VERY slow
#pragma comment(lib, "../FmodEx/fmodex_vc.lib")
#else
#pragma comment(lib, "../FmodEx/fmodex_vc.lib")
#endif

// =======================================================================
// Internal sound data & structures
// =======================================================================
bool					s_soundStarted = false;
bool					s_soundMuted = false;
static int				listener_number = 0;
cvar_t*					s_khz;
cvar_t*					s_volume;
cvar_t*					s_musicVolume;

FMOD::System*			fsystem;

////////////////////////////////////////////////////////
class cSound
{
private:
    std::string		filename;
    FMOD::Sound*	sample;
    FMOD::Channel*	channel;
    int				filesize;
public:
    bool ERRCHECK( FMOD_RESULT result, const char* funcname )
    {
        if( result == FMOD_OK ) return false;
        if( result == FMOD_ERR_OUTPUT_DRIVERCALL )
        {
            Com_Printf( S_COLOR_RED "%s FMOD_ERR_OUTPUT_DRIVERCALL (%s) A call to a standard soundcard driver failed, which could possibly mean a bug in the driver or resources were missing or exhausted.\n", funcname, filename.c_str() );
            return true;
        }
        if( result == FMOD_ERR_INVALID_HANDLE )
        {
            Com_Printf( S_COLOR_RED "%s FMOD_ERR_INVALID_HANDLE (%s) An invalid object handle was used.\n", funcname, filename.c_str() );
            return true;
        }
        Com_Error( ERR_FATAL, "%s FMOD error: (%d) %s\n", funcname, result, FMOD_ErrorString( result ) );
        return true;
    };
    cSound( std::string& in_filename, void* in_data, int length )
    {
        filename = in_filename;
        sample = NULL;
        channel = NULL;
        if( in_data && length )
        {
            filesize = length;
            unsigned int modes = FMOD_OPENMEMORY | FMOD_3D | FMOD_LOOP_OFF;
            FMOD_CREATESOUNDEXINFO exinfo;
            memset( &exinfo, 0, sizeof( FMOD_CREATESOUNDEXINFO ) );
            exinfo.cbsize = sizeof( FMOD_CREATESOUNDEXINFO );
            exinfo.length = length;
            FMOD_RESULT result = fsystem->createSound( ( const char* )in_data, modes, &exinfo, &sample );
            ERRCHECK( result, "createSound()" );
//			FSOUND_Sample_SetMinMaxDistance( sample, 0.0f, 1000000.0f);
        }
    };
    ~cSound()
    {
        if( channel )
        {
            channel->stop();
            channel = NULL;
        }
        if( sample )
        {
            sample->release();
            sample = NULL;
        }
    };
    ////////////////////////////////////////////////////
    std::string& GetName()
    {
        return filename;
    };
    ////////////////////////////////////////////////////
    int GetSize()
    {
        return filesize;
    };
    ////////////////////////////////////////////////////
    float GetFrequency()
    {
        if( !sample ) return 0.0f;
        float freq = 0.0f;
        sample->getDefaults( &freq, NULL, NULL, NULL );
        return freq;
    };
    ////////////////////////////////////////////////////
    int GetBits()
    {
        if( !sample ) return 0;
        int channels = 0, bits = 0;
        sample->getFormat( NULL, NULL, &channels, &bits );
        return bits;
    }
    ////////////////////////////////////////////////////
    int GetChannels()
    {
        if( !sample ) return 0;
        int channels = 0, bits = 0;
        sample->getFormat( NULL, NULL, &channels, &bits );
        return channels;
    }
    ////////////////////////////////////////////////////
    FMOD::Channel* Play( const float* emmiter_pos, bool s3d )
    {
        if( !sample ) return NULL;
        if( channel )
        {
            channel->stop();
        }
        fsystem->playSound( FMOD_CHANNEL_FREE, sample, false, &channel );
        channel->setVolume( s_volume->value );
        if( s3d && channel && emmiter_pos )
        {
            FMOD_VECTOR pos;
            pos.x = emmiter_pos[0];
            pos.y = emmiter_pos[1];
            pos.z = emmiter_pos[2];
            FMOD_VECTOR vel;
            vel.x = 0.0f;
            vel.y = 0.0f;
            vel.z = 0.0f;
            channel->set3DAttributes( &pos, &vel );
        }
        return channel;
    };
    ////////////////////////////////////////////////////
    FMOD::Channel* PlayLooped( const float* emmiter_pos, bool s3d )
    {
        sample->setMode( FMOD_LOOP_NORMAL );
        return Play( emmiter_pos, s3d );
    };
    ////////////////////////////////////////////////////
};
////////////////////////////////////////////////////////
#pragma pack(push,1)
struct video_sample_pair
{
    signed short	left;
    signed short	right;
};
#pragma pack(pop)
////////////////////////////////////////////////////////
class cVideoSample
{
private:
    video_sample_pair*		data;
    int						size;	// in number of samples
public:
    cVideoSample( video_sample_pair* in_data, int in_size )
    {
        size = in_size;
        data = new video_sample_pair[in_size];
        memcpy( data, in_data, sizeof( video_sample_pair ) *size );
    };
    ~cVideoSample()
    {
        if( data ) delete[] data;
    };
    void* GetData( void )
    {
        return data;
    };
    int GetSize( void )
    {
        return size;
    };
};
////////////////////////////////////////////////////////
typedef std::queue<cVideoSample*>			video_music_samples_q;
static video_music_samples_q				vsamples;
static cVideoSample*						vsample = NULL;
FMOD::Sound*								vstream = NULL;
mtHandle_t									video_mutex;
////////////////////////////////////////////////////////
class cFmod
{
private:
    bool			inited;
    ////////////////////////////////////////////////////
    unsigned int	background_intro_index;
    unsigned int	background_loop_index;
    FMOD::Channel* 	background_intro_channel;
    FMOD::Channel* 	background_loop_channel;
    ////////////////////////////////////////////////////
    typedef std::vector<cSound*>	sounds_m;
    sounds_m						sounds;
    ////////////////////////////////////////////////////
    struct Vec3
    {
        float	data[3];
        Vec3()
        {
            data[0] = data[1] = data[2] = 0.0f;
        };
        Vec3( const Vec3& ivec )
        {
            data[0] = ivec.data[0];
            data[1] = ivec.data[1];
            data[2] = ivec.data[2];
        };
        Vec3( const float* ivec )
        {
            data[0] = ivec[0];
            data[1] = ivec[1];
            data[2] = ivec[2];
        };
        void Set( const float* ivec )
        {
            data[0] = ivec[0];
            data[1] = ivec[1];
            data[2] = ivec[2];
        };
    };
    typedef std::map<int, Vec3>		entities_m;
    entities_m						entities;
    ////////////////////////////////////////////////////
public:
    cFmod()
    {
        inited = false;
        video_mutex = Sys::AllocateMutex();
        background_intro_index = 0;
        background_loop_index = 0;
        background_intro_channel = NULL;
        background_loop_channel = NULL;
    };
    ~cFmod()
    {
        Shutdown();
        Sys::UnlockMutex( video_mutex );
    };
    ////////////////////////////////////////////////////
    static void ERRCHECK( FMOD_RESULT result )
    {
        if( result != FMOD_OK )
        {
            Com::Errorf( ERR_FATAL, "FMOD error: (%d) %s\n", result, FMOD_ErrorString( result ) );
        }
    }
    ////////////////////////////////////////////////////
    bool Init( void )
    {
        if( inited ) return false;
        FMOD_RESULT result = FMOD::System_Create( &fsystem );
        ERRCHECK( result );
        unsigned int		version;
        result = fsystem->getVersion( &version );
        ERRCHECK( result );
        if( version < FMOD_VERSION )
        {
            Com::Printf( "Error!  You are using an old version of FMOD %08x.  This program requires %08x\n", version, FMOD_VERSION );
            return false;
        }
        FMOD_CAPS			caps;
        FMOD_SPEAKERMODE	speakermode;
        result = fsystem->getDriverCaps( 0, &caps, 0, 0, &speakermode );
        ERRCHECK( result );
        result = fsystem->setSpeakerMode( speakermode );		// Set the user selected speaker mode.
        ERRCHECK( result );
        if( caps & FMOD_CAPS_HARDWARE_EMULATED )	 			// The user has the 'Acceleration' slider set to off!  This is really bad for latency!.
        {
            // You might want to warn the user about this.
            result = fsystem->setDSPBufferSize( 1024, 10 );	// At 48khz, the latency between issuing an fmod command and hearing it will now be about 213ms.
            ERRCHECK( result );
        }
        result = fsystem->init( 1000, FMOD_INIT_NORMAL, 0 );	// 1000 channels max.
        if( result == FMOD_ERR_OUTPUT_CREATEBUFFER )  		// Ok, the speaker mode selected isn't supported by this soundcard.  Switch it back to stereo...
        {
            result = fsystem->setSpeakerMode( FMOD_SPEAKERMODE_STEREO );
            ERRCHECK( result );
            result = fsystem->init( 1000, FMOD_INIT_NORMAL, 0 );	// 1000 channels max.
            ERRCHECK( result );
        }
        inited = true;
        Com::Printf( "FMOD loaded.\n" );
        return true;
    };
    ////////////////////////////////////////////////////
    void ClearSounds( void )
    {
        for( sounds_m::iterator it = sounds.begin(); it != sounds.end(); it++ )
        {
            cSound* sound = *it;
            delete sound;
            *it = NULL;
        }
        sounds.clear();
    }
    ////////////////////////////////////////////////////
    void Shutdown( void )
    {
        if( !inited ) return;
        StopBackgroundMusic();
        ClearSounds();
        FMOD_RESULT result = fsystem->close();
        ERRCHECK( result );
        result = fsystem->release();
        ERRCHECK( result );
        inited = false;
    };
    ////////////////////////////////////////////////////
    void SoundsList( void )
    {
        int totalsize = 0;
        for( sounds_m::iterator it = sounds.begin(); it != sounds.end(); it++ )
        {
            cSound* sound = *it;
            Com::Printf( "%s S:%i F:%.0f B:%i C:%i\n", sound->GetName().c_str(), sound->GetSize(), sound->GetFrequency(), sound->GetBits(), sound->GetChannels() );
            totalsize += sound->GetSize();
        }
        Com::Printf( "total %i bytes for songs\n", totalsize );
    }
    ////////////////////////////////////////////////////
    void PlayBackgroundMusic( std::string& introname, std::string& loopname )
    {
        if( introname != "" )
        {
            background_intro_index = RegisterSound( introname );
        }
        if( loopname != "" )
        {
            background_loop_index = RegisterSound( loopname );
        }
    };
    ////////////////////////////////////////////////////
    void UpdateBackgroundMusic( void )
    {
        static float old_music_volume = 0.0f;
        if( old_music_volume != s_musicVolume->value )
        {
            old_music_volume = s_musicVolume->value;
            if( background_intro_channel != NULL )
            {
                background_intro_channel->setVolume( s_musicVolume->value );
            }
            if( background_loop_channel != NULL )
            {
                background_loop_channel->setVolume( s_musicVolume->value );
            }
        }
        if( background_loop_channel != NULL )
        {
            bool isplaying = false;
            background_loop_channel->isPlaying( &isplaying );
            if( isplaying ) return;												// loop still played
            background_loop_channel = NULL;
            return;																// intro and loop done
        }
        if( background_intro_channel != NULL )
        {
            bool isplaying = false;
            background_intro_channel->isPlaying( &isplaying );
            if( isplaying ) return;												// loop still played
            background_intro_channel = NULL;
            if( background_loop_index )
            {
                background_loop_channel = PlaySoundLooped( background_loop_index, NULL, false );
                background_loop_channel->setVolume( s_musicVolume->value );
                Com::Printf( "background music loop started\n" );
            }
            return;																// intro done, loop started
        }
        if( background_intro_index )
        {
            background_intro_channel = PlaySound( background_intro_index, NULL, false );
            background_intro_channel->setVolume( s_musicVolume->value );
            Com::Printf( "background music intro started\n" );
        }
    };
    ////////////////////////////////////////////////////
    void StopBackgroundMusic( void )
    {
        if( background_intro_channel != NULL )
        {
            background_intro_channel->stop();
            background_intro_channel = NULL;
        }
        if( background_loop_channel != NULL )
        {
            background_loop_channel->stop();
            background_loop_channel = NULL;
        }
        background_intro_index = 0;
        background_loop_index = 0;
    };
    ////////////////////////////////////////////////////
    unsigned int RegisterSound( std::string& filename )
    {
        for( unsigned int index = 0; index != sounds.size(); index++ )
        {
            if( sounds[index]->GetName() == filename )
            {
                return index + 1;	// indexes are 1-based.
            }
        }
        void* data;
        int length = FS::ReadFile( filename.c_str(), &data );
        if( length <= 0 )
        {
            std::string filename_wav = filename + ".wav";
            length = FS::ReadFile( filename_wav.c_str(), &data );
            if( length <= 0 )
            {
                Com::Printf( "Error loading '%s' sound.\n", filename.c_str() );
                return 0;
            }
        }
        unsigned int index = sounds.size();
        cSound* sound = new cSound( filename, data, length );
        sounds.push_back( sound );
        FS::FreeFile( data );
        return index + 1;	// indexes are 1-based.
    };
    ////////////////////////////////////////////////////
    FMOD::Channel* PlaySound( unsigned int index, int entity, bool s3d )
    {
        index--;			// indexes are 1-based.
        if( index >= sounds.size() )
        {
            return NULL;
        }
        if( !s3d )
        {
            return sounds[index]->Play( NULL, false );
        }
        entities_m::iterator it = entities.find( entity );
        if( it == entities.end() )
        {
            return sounds[index]->Play( NULL, false );
        }
        return sounds[index]->Play( it->second.data, s3d );
    };
    ////////////////////////////////////////////////////
    FMOD::Channel* PlaySoundLooped( unsigned int index, int entity, bool s3d )
    {
        index--;			// indexes are 1-based.
        if( index >= sounds.size() )
        {
            return NULL;
        }
        if( !s3d )
        {
            return sounds[index]->PlayLooped( NULL, false );
        }
        entities_m::iterator it = entities.find( entity );
        if( it == entities.end() )
        {
            return sounds[index]->PlayLooped( NULL, false );
        }
        return sounds[index]->PlayLooped( it->second.data, s3d );
    };
    ////////////////////////////////////////////////////
    void SetListenerPosition( float* in_pos, float* direction )
    {
        FMOD_VECTOR pos;
        pos.x = in_pos[0];
        pos.y = in_pos[1];
        pos.z = in_pos[2];
        FMOD_VECTOR vel;
        vel.x = 0.0f;
        vel.y = 0.0f;
        vel.z = 0.0f;
        FMOD_VECTOR dir;
        dir.x = direction[0];
        dir.y = direction[1];
        dir.z = direction[2];
        FMOD_VECTOR top;
        top.x = 0.0f;
        top.y = 1.0f;
        top.z = 0.0f;
        FMOD_RESULT result = fsystem->set3DListenerAttributes( 0, &pos, &vel, &dir, &top );
        ERRCHECK( result );
    };
    ////////////////////////////////////////////////////
    void UpdateEntityPos( const int entity, const float* pos )
    {
        entities_m::iterator it = entities.find( entity );
        if( it == entities.end() )
        {
            Vec3 tmp( pos );
            entities.insert( entities_m::value_type( entity, tmp ) );
        }
        it = entities.find( entity );
        it->second.Set( pos );
    };
    ////////////////////////////////////////////////////
    void SoundUpdate( void )
    {
        fsystem->update();
    };
    ////////////////////////////////////////////////////
    static void AddVideoSample( video_sample_pair* data, int size )
    {
        cVideoSample* sample = new cVideoSample( data, size );
        Sys::LockMutex( video_mutex );
        vsamples.push( sample );
        Sys::UnlockMutex( video_mutex );
    };
    ////////////////////////////////////////////////////
    static FMOD_RESULT F_CALLBACK VstreamCallback( FMOD_SOUND* /*sound*/, void* data, unsigned int datalen )
    {
        if( vsample )
        {
            delete vsample;
            vsample = NULL;
        }
        Sys::LockMutex( video_mutex );
        if( vsamples.empty() )
        {
            Sys::UnlockMutex( video_mutex );
            return FMOD_OK;
        }
        vsample = vsamples.front();
        vsamples.pop();
        Sys::UnlockMutex( video_mutex );
        unsigned int clen = vsample->GetSize() * sizeof( video_sample_pair );
        //Com::Printf( S_COLOR_RED "VstreamCallback: len=%i clen=%i\n", datalen, clen);
        if( datalen < clen ) clen = datalen;
        memcpy( data, vsample->GetData(), clen );
        return FMOD_OK;
    }
    ////////////////////////////////////////////////////
    void UpdateVideoSamples( void )
    {
        Sys::LockMutex( video_mutex );
        if( vsamples.empty() )
        {
            Sys::UnlockMutex( video_mutex );
            return;
        }
        Sys::UnlockMutex( video_mutex );
        if( vstream != NULL ) return;
        FMOD_CREATESOUNDEXINFO exinfo;
        memset( &exinfo, 0, sizeof( FMOD_CREATESOUNDEXINFO ) );
        exinfo.cbsize = sizeof( FMOD_CREATESOUNDEXINFO );
        exinfo.decodebuffersize = 2944 / 2 / 2;
        exinfo.length = 2944;
        exinfo.defaultfrequency = 22050;
        exinfo.format = FMOD_SOUND_FORMAT_PCM16;
        exinfo.numchannels = 2;
        exinfo.pcmreadcallback = VstreamCallback;
        FMOD_RESULT result = fsystem->createStream( NULL, FMOD_2D | FMOD_OPENUSER | FMOD_OPENRAW | FMOD_CREATESTREAM | FMOD_LOOP_NORMAL | FMOD_HARDWARE, &exinfo, &vstream );
        ERRCHECK( result );
        FMOD::Channel* channel = 0;
        fsystem->playSound( FMOD_CHANNEL_FREE, vstream, false, &channel );
        ERRCHECK( result );
        channel->setVolume( s_musicVolume->value );
    };
    ////////////////////////////////////////////////////
    void StopVideoSamples( void )
    {
        if( vstream != NULL )
        {
            vstream->release();
            vstream = NULL;
        }
        Sys::LockMutex( video_mutex );
        while( !vsamples.empty() )
        {
            vsample = vsamples.front();
            vsamples.pop();
            delete vsample;
        }
        Sys::UnlockMutex( video_mutex );
        vsample = NULL;
    };
    ////////////////////////////////////////////////////
} FmodSound;

////////////////////////////////////////////////////////
// ====================================================================
void Snd::SoundInfo_f( void )
{
    Com::Printf( "----- Sound Info -----\n" );
    if( !s_soundStarted )
    {
        Com::Printf( "sound system not started\n" );
    }
    else
    {
        if( s_soundMuted )
        {
            Com::Printf( "sound system is muted\n" );
        }
    }
    Com::Printf( "----------------------\n" );
}

// ====================================================================
void Snd::Init( void )
{
    Com::Printf( "\n------- sound initialization -------\n" );
    s_volume = Cvar::Get( "s_volume", "0.8", CVAR_ARCHIVE );
    s_musicVolume = Cvar::Get( "s_musicvolume", "0.25", CVAR_ARCHIVE );
    s_khz = Cvar::Get( "s_khz", "22", CVAR_ARCHIVE );
    cvar_t* cv = Cvar::Get( "s_initsound", "1", 0 );
    if( !cv->integer )
    {
        Com::Printf( "not initializing.\n" );
        Com::Printf( "------------------------------------\n" );
        return;
    }
    Cmd::AddCommand( "play", Snd::Play_f );
    Cmd::AddCommand( "music", Snd::Music_f );
    Cmd::AddCommand( "s_list", Snd::SoundList_f );
    Cmd::AddCommand( "s_info", Snd::SoundInfo_f );
    Cmd::AddCommand( "s_stop", Snd::StopAllSounds );
    Com::Printf( "------------------------------------\n" );
    FmodSound.Init();
    s_soundStarted = true;
    s_soundMuted = true;
    Snd::StopAllSounds();
    Snd::SoundInfo_f();
}

// =======================================================================
void Snd::Shutdown( void )
{
    if( !s_soundStarted )
    {
        return;
    }
    FmodSound.Shutdown();
    s_soundStarted = false;
    Cmd::RemoveCommand( "play" );
    Cmd::RemoveCommand( "music" );
    Cmd::RemoveCommand( "s_stop" );
    Cmd::RemoveCommand( "s_list" );
    Cmd::RemoveCommand( "s_info" );
}

/*===================
Disables sounds until the next Snd::BeginRegistration.
This is called when the hunk is cleared and the sounds
are no longer valid.
===================*/
void Snd::DisableSounds( void )
{
    Snd::StopAllSounds();
    FmodSound.ClearSounds();
    s_soundMuted = true;
}

/*=====================Snd::BeginRegistration=====================*/
void Snd::BeginRegistration( void )
{
    s_soundMuted = false;		// we can play again
    Snd::RegisterSound( "sound/feedback/hit.wav" );		// changed to a sound in baseq3
}

/*
================== Creates a default buzz sound if the file can't be loaded ==================*/
sfxHandle_t	Snd::RegisterSound( const char* name )
{
    std::string filename = name;
    return FmodSound.RegisterSound( filename );
}

/*==================== Snd::StartSound
Validates the parms and ques the sound up
if pos is NULL, the sound will be dynamically sourced from the entity
Entchannel 0 will never override a playing sound
====================*/
void Snd::StartSound( vec3_t /*origin*/, int entityNum, int /*entchannel*/, sfxHandle_t sfxHandle )
{
    FmodSound.PlaySound( sfxHandle, entityNum, true );
}

/*================== Snd::StartLocalSound ==================*/
void Snd::StartLocalSound( sfxHandle_t sfxHandle, int /*channelNum*/ )
{
    FmodSound.PlaySound( sfxHandle, listener_number, false );
}

/*==================Snd::ClearSoundBuffer
If we are about to perform file access, clear the buffer
so sound doesn't stutter.
==================*/
void Snd::ClearSoundBuffer( void )
{
    FmodSound.StopVideoSamples();
}

/*================== Snd::StopAllSounds ==================*/
void Snd::StopAllSounds( void )
{
    Snd::StopBackgroundTrack();
    Snd::ClearSoundBuffer();
}

/*==============================================================
continuous looping sounds are added each frame
==============================================================*/
void Snd::StopLoopingSound( int /*entityNum*/ )
{
}

/*==================
Snd::ClearLoopingSounds
==================*/
void Snd::ClearLoopingSounds( bool /*killall*/ )
{
}

/*==================
Snd::AddLoopingSound
Called during entity generation for a frame
Include velocity in case I get around to doing doppler...
==================*/
void Snd::AddLoopingSound( int /*entityNum*/, const vec3_t /*origin*/, const vec3_t /*velocity*/, sfxHandle_t /*sfxHandle*/ )
{
}

/*==================
Snd::AddLoopingSound
Called during entity generation for a frame
Include velocity in case I get around to doing doppler...
==================*/
void Snd::AddRealLoopingSound( int /*entityNum*/, const vec3_t /*origin*/, const vec3_t /*velocity*/, sfxHandle_t /*sfxHandle*/ )
{
}

/*==================
Snd::AddLoopSounds
Spatialize all of the looping sounds.
All sounds are on the same cycle, so any duplicates can just
sum up the channel multipliers.
==================*/
void Snd::AddLoopSounds( void )
{
}

//=============================================================================

/*=====================
Snd::UpdateEntityPosition
let the sound system know where an entity currently is
======================*/
void Snd::UpdateEntityPosition( int entityNum, const vec3_t origin )
{
    if( origin )
    {
        float pos[3];
        pos[0] = origin[0];
        pos[1] = origin[2];
        pos[2] = origin[1];
        FmodSound.UpdateEntityPos( entityNum, pos );
    }
}

/*============ Change the volumes of all the playing sounds for changes in their positions ============*/
void Snd::Respatialize( int entityNum, const vec3_t head, vec3_t axis[3], int /*inwater*/ )
{
    listener_number = entityNum;
    float pos[3];
    pos[0] = head[0];
    pos[1] = head[2];
    pos[2] = head[1];
    
    vec3_t forward = { 1.0f, 0.0f, 0.0f };
    vec3_t vec;
    VectorRotate( forward, axis, vec );
    float dir[3];
    dir[0] = vec[0];
    dir[1] = vec[2];
    dir[2] = -vec[1];
    FmodSound.SetListenerPosition( pos, dir );
    
    fsystem->set3DSettings( 1.0f, 1.0f, 0.001f );
    
}

/*============ Called once each time through the main loop ============*/
void Snd::Update( void )
{
    FmodSound.UpdateVideoSamples();
    FmodSound.UpdateBackgroundMusic();
    FmodSound.SoundUpdate();
}

//===============================================================================*/
void Snd::Play_f( void )
{
    sfxHandle_t	h;
    char		name[256];
    int i = 1;
    while( i < Cmd::Argc() )
    {
        if( !Q_strrchr( Cmd::Argv( i ), '.' ) )
        {
            Com_sprintf( name, sizeof( name ), "%s.wav", Cmd::Argv( 1 ) );
        }
        else
        {
            Q_strncpyz( name, Cmd::Argv( i ), sizeof( name ) );
        }
        h = Snd::RegisterSound( name );
        if( h )
        {
            Snd::StartLocalSound( h, CHAN_LOCAL_SOUND );
        }
        i++;
    }
}

/////////////////////////////////////////////////////////////
void Snd::Music_f( void )
{
    int c = Cmd::Argc();
    if( c == 2 )
    {
        Snd::StopBackgroundTrack();
        Snd::StartBackgroundTrack( Cmd::Argv( 1 ), Cmd::Argv( 1 ) );
    }
    else if( c == 3 )
    {
        Snd::StopBackgroundTrack();
        Snd::StartBackgroundTrack( Cmd::Argv( 1 ), Cmd::Argv( 2 ) );
    }
    else
    {
        Com::Printf( "music <musicfile> [loopfile]\n" );
        return;
    }
}

/////////////////////////////////////////////////////////////
void Snd::SoundList_f( void )
{
    FmodSound.SoundsList();
}

/*====================== Snd::StopBackgroundTrack ======================*/
void Snd::StopBackgroundTrack( void )
{
    FmodSound.StopBackgroundMusic();
}

/*====================== Snd::StartBackgroundTrack ======================*/
void Snd::StartBackgroundTrack( const char* intro, const char* loop )
{
    if( !intro )
    {
        intro = "";
    }
    if( !loop || !loop[0] )
    {
        loop = intro;
    }
    Com::Printf( "Snd::StartBackgroundTrack( %s, %s )\n", intro, loop );
    std::string introname = intro;
    std::string loopname = loop;
    FmodSound.PlayBackgroundMusic( introname, loopname );
}

/*============Snd::RawSamples Music streaming ============*/
void Snd::RawSamples( int samples, int /*rate*/, int /*width*/, int /*s_channels*/, const byte* data, float /*volume*/ )
{
    video_sample_pair* sample = new video_sample_pair[samples];
    signed short* ptr = ( signed short* )data;
    for( int i = 0; i < samples; i++ )
    {
        sample[i].left = ptr[i * 2];
        sample[i].right = ptr[i * 2 + 1];
    }
    FmodSound.AddVideoSample( sample, samples );
    delete sample;
    
}

