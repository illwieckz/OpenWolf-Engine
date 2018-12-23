////////////////////////////////////////////////////////////////////////////////////////
// Copyright(C) 1999 - 2010 id Software LLC, a ZeniMax Media company.
// Copyright(C) 2011 - 2018 Dusan Jocic <dusanjocic@msn.com>
//
// This file is part of OpenWolf.
//
// OpenWolf is free software; you can redistribute it
// and / or modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of the License,
// or (at your option) any later version.
//
// OpenWolf is distributed in the hope that it will be
// useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with OpenWolf; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110 - 1301  USA
//
// -------------------------------------------------------------------------------------
// File name:   s_main.cpp
// Version:     v1.01
// Created:
// Compilers:   Visual Studio 2017, gcc 7.3.0
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#include <OWLib/precompiled.h>

cvar_t* s_volume;
cvar_t* s_muted;
cvar_t* s_musicVolume;
cvar_t* s_doppler;
cvar_t* s_backend;
cvar_t* s_muteWhenMinimized;
cvar_t* s_muteWhenUnfocused;

static soundInterface_t si;

/*
=================
S_ValidateInterface
=================
*/
static bool S_ValidSoundInterface( soundInterface_t* si )
{
    if( !si->Shutdown )
        return false;
    if( !si->StartSound )
        return false;
    if( !si->StartLocalSound )
        return false;
    if( !si->StartBackgroundTrack )
        return false;
    if( !si->StopBackgroundTrack )
        return false;
    if( !si->RawSamples )
        return false;
    if( !si->StopAllSounds )
        return false;
    if( !si->ClearLoopingSounds )
        return false;
    if( !si->AddLoopingSound )
        return false;
    if( !si->AddRealLoopingSound )
        return false;
    if( !si->StopLoopingSound )
        return false;
    if( !si->Respatialize )
        return false;
    if( !si->UpdateEntityPosition )
        return false;
    if( !si->Update )
        return false;
    if( !si->DisableSounds )
        return false;
    if( !si->BeginRegistration )
        return false;
    if( !si->RegisterSound )
        return false;
    if( !si->ClearSoundBuffer )
        return false;
    if( !si->SoundInfo )
        return false;
    if( !si->SoundList )
        return false;
        
    return true;
}

/*
=================
idSoundSystemLocal::StartSound
=================
*/
void idSoundSystemLocal::StartSound( vec3_t origin, S32 entnum, S32 entchannel, sfxHandle_t sfx )
{
    if( si.StartSound )
    {
        si.StartSound( origin, entnum, entchannel, sfx );
    }
}

/*
=================
idSoundSystemLocal::StartLocalSound
=================
*/
void idSoundSystemLocal::StartLocalSound( sfxHandle_t sfx, S32 channelNum )
{
    if( si.StartLocalSound )
    {
        si.StartLocalSound( sfx, channelNum );
    }
}

/*
=================
idSoundSystemLocal::StartBackgroundTrack
=================
*/
void idSoundSystemLocal::StartBackgroundTrack( StringEntry intro, StringEntry loop )
{
    if( si.StartBackgroundTrack )
    {
        si.StartBackgroundTrack( intro, loop );
    }
}

/*
=================
idSoundSystemLocal::StopBackgroundTrack
=================
*/
void idSoundSystemLocal::StopBackgroundTrack( void )
{
    if( si.StopBackgroundTrack )
    {
        si.StopBackgroundTrack();
    }
}

/*
=================
idSoundSystemLocal::RawSamples
=================
*/
void idSoundSystemLocal::RawSamples( S32 stream, S32 samples, S32 rate, S32 width, S32 channels, const U8* data, F32 volume )
{
    if( si.RawSamples )
    {
        si.RawSamples( stream, samples, rate, width, channels, data, volume );
    }
}

/*
=================
idSoundSystemLocal::StopAllSounds
=================
*/
void idSoundSystemLocal::StopAllSounds( void )
{
    if( si.StopAllSounds )
    {
        si.StopAllSounds();
    }
}

/*
=================
idSoundSystemLocal::ClearLoopingSounds
=================
*/
void idSoundSystemLocal::ClearLoopingSounds( bool  killall )
{
    if( si.ClearLoopingSounds )
    {
        si.ClearLoopingSounds( killall );
    }
}

/*
=================
idSoundSystemLocal::AddLoopingSound
=================
*/
void idSoundSystemLocal::AddLoopingSound( S32 entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx )
{
    if( si.AddLoopingSound )
    {
        si.AddLoopingSound( entityNum, origin, velocity, sfx );
    }
}

/*
=================
idSoundSystemLocal::AddRealLoopingSound
=================
*/
void idSoundSystemLocal::AddRealLoopingSound( S32 entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx )
{
    if( si.AddRealLoopingSound )
    {
        si.AddRealLoopingSound( entityNum, origin, velocity, sfx );
    }
}

/*
=================
idSoundSystemLocal::StopLoopingSound
=================
*/
void idSoundSystemLocal::StopLoopingSound( S32 entityNum )
{
    if( si.StopLoopingSound )
    {
        si.StopLoopingSound( entityNum );
    }
}

/*
=================
idSoundSystemLocal::Respatialize
=================
*/
void idSoundSystemLocal::Respatialize( S32 entityNum, const vec3_t origin, vec3_t axis[3], S32 inwater )
{
    if( si.Respatialize )
    {
        si.Respatialize( entityNum, origin, axis, inwater );
    }
}

/*
=================
idSoundSystemLocal::UpdateEntityPosition
=================
*/
void idSoundSystemLocal::UpdateEntityPosition( S32 entityNum, const vec3_t origin )
{
    if( si.UpdateEntityPosition )
    {
        si.UpdateEntityPosition( entityNum, origin );
    }
}

/*
=================
idSoundSystemLocal::Update
=================
*/
void idSoundSystemLocal::Update( void )
{
    if( s_muted->integer )
    {
        if( !( s_muteWhenMinimized->integer && com_minimized->integer ) && !( s_muteWhenUnfocused->integer && com_unfocused->integer ) )
        {
            s_muted->integer = false;
            s_muted->modified = true;
        }
    }
    else
    {
        if( ( s_muteWhenMinimized->integer && com_minimized->integer ) || ( s_muteWhenUnfocused->integer && com_unfocused->integer ) )
        {
            s_muted->integer = true;
            s_muted->modified = true;
        }
    }
    
    if( si.Update )
    {
        si.Update();
    }
}

/*
=================
idSoundSystemLocal::DisableSounds
=================
*/
void idSoundSystemLocal::DisableSounds( void )
{
    if( si.DisableSounds )
    {
        si.DisableSounds();
    }
}

/*
=================
idSoundSystemLocal::BeginRegistration
=================
*/
void idSoundSystemLocal::BeginRegistration( void )
{
    if( si.BeginRegistration )
    {
        si.BeginRegistration();
    }
}

/*
=================
idSoundSystemLocal::RegisterSound
=================
*/
sfxHandle_t idSoundSystemLocal::RegisterSound( StringEntry sample )
{
    if( si.RegisterSound )
    {
        return si.RegisterSound( sample );
    }
    else
    {
        return 0;
    }
}

/*
=================
idSoundSystemLocal::ClearSoundBuffer
=================
*/
void idSoundSystemLocal::ClearSoundBuffer( void )
{
    if( si.ClearSoundBuffer )
    {
        si.ClearSoundBuffer();
    }
}

/*
=================
S_SoundInfo
=================
*/
void S_SoundInfo( void )
{
    if( si.SoundInfo )
    {
        si.SoundInfo();
    }
}

/*
=================
S_SoundList
=================
*/
void S_SoundList( void )
{
    if( si.SoundList )
    {
        si.SoundList();
    }
}

/*
 =================
 S_SetReverb
 =================
 */
void S_SetReverb( S32 reverb, vec3_t pos, F32 minDist, F32 maxDist )
{
    if( si.SetReverb )
    {
        si.SetReverb( reverb, pos, minDist, maxDist );
    }
}

//=============================================================================

/*
=================
S_Play_f
=================
*/
void S_Play_f( void )
{
    S32 i;
    sfxHandle_t h;
    UTF8 name[256];
    
    if( !si.RegisterSound || !si.StartLocalSound )
    {
        return;
    }
    
    i = 1;
    while( i < Cmd_Argc() )
    {
        if( !Q_strrchr( Cmd_Argv( i ), '.' ) )
        {
            Com_sprintf( name, sizeof( name ), "%s.ogg", Cmd_Argv( 1 ) );
        }
        else
        {
            Q_strncpyz( name, Cmd_Argv( i ), sizeof( name ) );
        }
        h = si.RegisterSound( name );
        if( h )
        {
            si.StartLocalSound( h, CHAN_LOCAL_SOUND );
        }
        i++;
    }
}

/*
=================
S_Music_f
=================
*/
void S_Music_f( void )
{
    S32 c;
    
    if( !si.StartBackgroundTrack )
    {
        return;
    }
    
    c = Cmd_Argc();
    
    if( c == 2 )
    {
        si.StartBackgroundTrack( Cmd_Argv( 1 ), NULL );
    }
    else if( c == 3 )
    {
        si.StartBackgroundTrack( Cmd_Argv( 1 ), Cmd_Argv( 2 ) );
    }
    else
    {
        Com_Printf( "music <musicfile> [loopfile]\n" );
        return;
    }
    
}

/*
=================
S_Music_f
=================
*/
void S_StopMusic_f( void )
{
    if( !si.StopBackgroundTrack )
        return;
        
    si.StopBackgroundTrack();
}


//=============================================================================

/*
=================
idSoundSystemLocal::Init
=================
*/
void idSoundSystemLocal::Init( void )
{
    cvar_t* cv;
    bool started = false;
    
    Com_Printf( "------ Initializing Sound ------\n" );
    
    s_volume = cvarSystem->Get( "s_volume", "0.8", CVAR_ARCHIVE );
    s_musicVolume = cvarSystem->Get( "s_musicvolume", "0.25", CVAR_ARCHIVE );
    s_muted = cvarSystem->Get( "s_muted", "0", CVAR_ROM );
    s_doppler = cvarSystem->Get( "s_doppler", "1", CVAR_ARCHIVE );
    s_backend = cvarSystem->Get( "s_backend", "", CVAR_ROM );
    s_muteWhenMinimized = cvarSystem->Get( "s_muteWhenMinimized", "0", CVAR_ARCHIVE );
    s_muteWhenUnfocused = cvarSystem->Get( "s_muteWhenUnfocused", "0", CVAR_ARCHIVE );
    
    cv = cvarSystem->Get( "s_initsound", "1", 0 );
    if( !cv->integer )
    {
        Com_Printf( "Sound disabled.\n" );
    }
    else
    {
        S_CodecInit();
        
        Cmd_AddCommand( "play", S_Play_f );
        Cmd_AddCommand( "music", S_Music_f );
        Cmd_AddCommand( "stopmusic", S_StopMusic_f );
        Cmd_AddCommand( "s_list", S_SoundList );
        //Cmd_AddCommand( "s_stop", StopAllSounds );
        Cmd_AddCommand( "s_info", S_SoundInfo );
        
        cv = cvarSystem->Get( "s_useOpenAL", "1", CVAR_ARCHIVE );
        if( cv->integer )
        {
            //OpenAL
            started = S_AL_Init( &si );
            cvarSystem->Set( "s_backend", "OpenAL" );
        }
        
        if( !started )
        {
            started = S_Base_Init( &si );
            cvarSystem->Set( "s_backend", "base" );
        }
        
        if( started )
        {
            if( !S_ValidSoundInterface( &si ) )
            {
                Com_Error( ERR_FATAL, "Sound interface invalid." );
            }
            
            S_SoundInfo();
            Com_Printf( "Sound initialization successful.\n" );
        }
        else
        {
            Com_Printf( "Sound initialization failed.\n" );
        }
    }
    
    Com_Printf( "--------------------------------\n" );
}

/*
=================
idSoundSystemLocal::Shutdown
=================
*/
void idSoundSystemLocal::Shutdown( void )
{
    if( si.Shutdown )
    {
        si.Shutdown();
    }
    
    ::memset( &si, 0, sizeof( soundInterface_t ) );
    
    Cmd_RemoveCommand( "play" );
    Cmd_RemoveCommand( "music" );
    Cmd_RemoveCommand( "stopmusic" );
    Cmd_RemoveCommand( "s_list" );
    Cmd_RemoveCommand( "s_stop" );
    Cmd_RemoveCommand( "s_info" );
    
    S_CodecShutdown();
}
