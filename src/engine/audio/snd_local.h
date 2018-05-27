////////////////////////////////////////////////////////////////////////////////////////
// Copyright(C) 1999 - 2010 id Software LLC, a ZeniMax Media company.
// Copyright(C) 2011 - 2018 Dusan Jocic <dusanjocic@msn.com>
//
// This file is part of the OpenWolf GPL Source Code.
// OpenWolf Source Code is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// OpenWolf Source Code is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with OpenWolf Source Code.  If not, see <http://www.gnu.org/licenses/>.
//
// In addition, the OpenWolf Source Code is also subject to certain additional terms.
// You should have received a copy of these additional terms immediately following the
// terms and conditions of the GNU General Public License which accompanied the
// OpenWolf Source Code. If not, please request a copy in writing from id Software
// at the address below.
//
// If you have questions concerning this license or the applicable additional terms,
// you may contact in writing id Software LLC, c/o ZeniMax Media Inc.,
// Suite 120, Rockville, Maryland 20850 USA.
//
// -------------------------------------------------------------------------------------
// File name:   snd_local.h
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description: private sound definations
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#ifndef __SND_LOCAL_H__
#define __SND_LOCAL_H__

#ifndef __Q_SHARED_H__
#include <qcommon/q_shared.h>
#endif
#ifndef _QCOMMON_H_
#include <qcommon/qcommon.h>
#endif

#define PAINTBUFFER_SIZE        4096                    // this is in samples
#define SND_CHUNK_SIZE          1024                    // samples
#define SND_CHUNK_SIZE_FLOAT    ( SND_CHUNK_SIZE / 2 )  // floats
#define SND_CHUNK_SIZE_BYTE     ( SND_CHUNK_SIZE * 2 )  // floats

#define TALKANIM

typedef struct portable_samplepair_s
{
    S32 left;           // the final values will be clamped to +/- 0x00ffff00 and shifted down
    S32 right;
} portable_samplepair_t;

typedef struct adpcm_state
{
    S16 sample;         /* Previous output value */
    UTF8 index;         /* Index into stepsize table */
#if defined( __MACOS__ )
    UTF8 pad;           /* //DAJ added pad for alignment */
#endif
} adpcm_state_t;

typedef struct sndBuffer_s
{
    S16 sndChunk[SND_CHUNK_SIZE];
    struct sndBuffer_s*      next;
    S32 size;
    adpcm_state_t adpcm;
} sndBuffer;

typedef struct sfx_s
{
    sndBuffer*       soundData;
    bool defaultSound;                  // couldn't be loaded, so use buzz
    bool inMemory;                      // not in Memory
    bool soundCompressed;               // not in Memory
    S32 soundCompressionMethod;
    S32 soundLength;
    UTF8 soundName[MAX_QPATH];
    S32 lastTimeUsed;
    S32 duration;
    struct sfx_s*    next;
} sfx_t;

typedef struct
{
    S32 channels;
    S32 samples;                        // mono samples in buffer
    S32 submission_chunk;               // don't mix less than this #
    S32 samplebits;
    S32 speed;
    S32 samplepos;
    U8* buffer;
} dma_t;

#define START_SAMPLE_IMMEDIATE  0x7fffffff

typedef struct loopSound_s
{
    vec3_t origin;
    vec3_t velocity;
    sfx_t* sfx;
    S32 mergeFrame;
    bool active;
    bool kill;
    bool doppler;
    F32 dopplerScale;
    F32 oldDopplerScale;
    S32 framenum;
    F32 range;            //----(SA)	added
    S32 vol;
    bool loudUnderWater;    // (SA) set if this sound should be played at full vol even when under water (under water loop sound for ex.)
    S32 startTime, startSample;         // ydnar: so looping sounds can be out of phase
} loopSound_t;

typedef struct
{
    S32* prt;                   //DAJ BUGFIX for freelist/endlist pointer
    S32 allocTime;
    S32 startSample;            // START_SAMPLE_IMMEDIATE = set immediately on next mix
    S32 entnum;                 // to allow overriding a specific sound
    S32 entchannel;             // to allow overriding a specific sound
    S32 leftvol;                // 0-255 volume after spatialization
    S32 rightvol;               // 0-255 volume after spatialization
    S32 master_vol;             // 0-255 volume before spatialization
    F32 dopplerScale;
    F32 oldDopplerScale;
    vec3_t origin;              // only use if fixed_origin is set
    bool fixed_origin;          // use origin instead of fetching entnum's origin
    sfx_t* thesfx;              // sfx structure
    bool doppler;
    S32 flags;                  //----(SA)	added
    bool threadReady;
} channel_t;

#define WAV_FORMAT_PCM      1

typedef struct
{
    S32 format;
    S32 rate;
    S32 width;
    S32 channels;
    S32 samples;
    S32 dataofs;                // chunk starts this many bytes from file start
} wavinfo_t;


/*
====================================================================

  SYSTEM SPECIFIC FUNCTIONS

====================================================================
*/

// initializes cycling through a DMA buffer and returns information on it
bool SNDDMA_Init( void );

// gets the current DMA position
S32     SNDDMA_GetDMAPos( void );

// shutdown the DMA xfer.
void    SNDDMA_Shutdown( void );

void    SNDDMA_BeginPainting( void );

void    SNDDMA_Submit( void );

//====================================================================

#if defined( __MACOS__ )
#define MAX_CHANNELS 64
#else
#define MAX_CHANNELS            96
#endif

extern channel_t s_channels[MAX_CHANNELS];
extern channel_t loop_channels[MAX_CHANNELS];
extern S32 numLoopChannels;

extern S32 s_paintedtime;
extern vec3_t listener_forward;
extern vec3_t listener_right;
extern vec3_t listener_up;
extern dma_t dma;

#ifdef TALKANIM
extern U8 s_entityTalkAmplitude[MAX_CLIENTS];
#endif

//----(SA)	some flags for queued music tracks
#define QUEUED_PLAY_ONCE    -1
#define QUEUED_PLAY_LOOPED  -2
#define QUEUED_PLAY_ONCE_SILENT -3  // when done it goes quiet
//----(SA)	end

// Ridah, streaming sounds
typedef struct
{
    fileHandle_t file;
    wavinfo_t info;
    S32 samples;
    UTF8 name[MAX_QPATH];           //----(SA)	added
    UTF8 loop[MAX_QPATH];
    S32 looped;                 //----(SA)	added
    S32 entnum;
    S32 channel;
    S32 attenuation;
    S32 kill;           //----(SA)	changed
    
    S32 fadeStart;              //----(SA)	added
    S32 fadeEnd;                //----(SA)	added
    F32 fadeStartVol;         //----(SA)	added
    F32 fadeTargetVol;        //----(SA)	added
} streamingSound_t;

typedef struct
{
    vec3_t origin;
    bool fixedOrigin;
    S32 entityNum;
    S32 entityChannel;
    sfxHandle_t sfx;
    S32 flags;
    S32 volume;
} s_pushStack;

#define MAX_PUSHSTACK   64
#define LOOP_HASH       128
#define MAX_LOOP_SOUNDS 1024

// removed many statics into a common sound struct
typedef struct
{
    sfx_t*       sfxHash[LOOP_HASH];
    S32 numLoopSounds;
    loopSound_t loopSounds[MAX_LOOP_SOUNDS];
    
    F32 volTarget;
    F32 volStart;
    S32 volTime1;
    S32 volTime2;
    F32 volFadeFrac;
    F32 volCurrent;
    
    bool stopSounds;
    
    channel_t*   freelist;
    channel_t*   endflist;
    
    S32 s_numSfx;
    
    s_pushStack pushPop[MAX_PUSHSTACK];
    S32 tart;
    
    bool s_soundPainted;
    S32 s_clearSoundBuffer;
    
    S32 s_soundStarted;
    S32 s_soundMute;                // 0 - not muted, 1 - muted, 2 - no new sounds, but play out remaining sounds (so they can die if necessary)
    
    vec3_t entityPositions[MAX_GENTITIES];
    
    UTF8 nextMusicTrack[MAX_QPATH];         // extracted from CS_MUSIC_QUEUE //----(SA)	added
    S32 nextMusicTrackType;
} snd_t;

extern snd_t snd;   // globals for sound

#define MAX_STREAMING_SOUNDS    12  // need to keep it low, or the rawsamples will get too big
#define MAX_RAW_SAMPLES         16384

extern streamingSound_t streamingSounds[MAX_STREAMING_SOUNDS];
extern S32 s_rawend[MAX_STREAMING_SOUNDS];
extern portable_samplepair_t s_rawsamples[MAX_STREAMING_SOUNDS][MAX_RAW_SAMPLES];
extern portable_samplepair_t s_rawVolume[MAX_STREAMING_SOUNDS];


extern cvar_t*   s_volume;
extern cvar_t*   s_nosound;
extern cvar_t*   s_khz;
extern cvar_t*   s_show;
extern cvar_t*   s_mixahead;
extern cvar_t*   s_mute;

extern cvar_t*   s_testsound;
extern cvar_t*   s_separation;
extern cvar_t*   s_currentMusic;    //----(SA)	added
extern cvar_t*   s_debugMusic;      //----(SA)	added

// fretn
extern cvar_t*   s_bits;
extern cvar_t*   s_numchannels;

bool S_LoadSound( sfx_t* sfx );

void        SND_free( sndBuffer* v );
sndBuffer*  SND_malloc();
void        SND_setup();

void S_PaintChannels( S32 endtime );

void S_memoryLoad( sfx_t* sfx );
portable_samplepair_t* S_GetRawSamplePointer();

// spatializes a channel
void S_Spatialize( channel_t* ch );

// adpcm functions
S32  S_AdpcmMemoryNeeded( const wavinfo_t* info );
void S_AdpcmEncodeSound( sfx_t* sfx, S16* samples );
void S_AdpcmGetSamples( sndBuffer* chunk, S16* to );

// wavelet function

#define SENTINEL_MULAW_ZERO_RUN 127
#define SENTINEL_MULAW_FOUR_BIT_RUN 126

void S_FreeOldestSound();

void encodeWavelet( sfx_t* sfx, S16* packets );
void decodeWavelet( sndBuffer* stream, S16* packets );

void encodeMuLaw( sfx_t* sfx, S16* packets );
extern S16 mulawToShort[256];

extern S16* sfxScratchBuffer;
extern const sfx_t* sfxScratchPointer;
extern S32 sfxScratchIndex;

extern U8 s_entityTalkAmplitude[MAX_CLIENTS];
extern F32 S_GetStreamingFade( streamingSound_t* ss );

//
// idSoundSystemLocal
//
class idSoundSystemLocal : public idSoundSystem
{
public:
    virtual void Init( void );
    virtual void Shutdown( void );
    virtual void UpdateThread( void );
    virtual void StartSound( vec3_t origin, S32 entityNum, S32 entchannel, sfxHandle_t sfxHandle, S32 volume );
    virtual void StartSoundEx( vec3_t origin, S32 entityNum, S32 entchannel, sfxHandle_t sfxHandle, S32 flags, S32 volume );
    virtual void StartLocalSound( sfxHandle_t sfxHandle, S32 channelNum, S32 volume );
    virtual void ClearSoundBuffer( bool killStreaming );
    virtual void StartBackgroundTrack( StringEntry intro, StringEntry loop, S32 fadeupTime );
    virtual void StopBackgroundTrack( void );
    virtual void FadeStreamingSound( F32 targetvol, S32 time, S32 ssNum );
    virtual void FadeAllSounds( F32 targetVol, S32 time, bool stopsounds );
    virtual F32 StartStreamingSound( StringEntry intro, StringEntry loop, S32 entnum, S32 channel, S32 attenuation );
    virtual void StopStreamingSound( S32 index );
    virtual void StopEntStreamingSound( S32 entNum );
    
    // cinematics and voice-over-network will send raw samples
    // 1.0 volume will be direct output of source samples
    virtual void RawSamples( S32 samples, S32 rate, S32 width, S32 channels, const U8* data, F32 lvol, F32 rvol, S32 streamingIndex );
    
    // stop all sounds and the background track
    virtual void StopAllSounds( void );
    
    // all continuous looping sounds must be added before calling Update
    virtual void ClearLoopingSounds( void );
    virtual void ClearSounds( bool clearStreaming, bool clearMusic );
    virtual void AddLoopingSound( const vec3_t origin, const vec3_t velocity, const S32 range, sfxHandle_t sfxHandle, S32 volume, S32 soundTime );
    virtual void AddRealLoopingSound( const vec3_t origin, const vec3_t velocity, const S32 range, sfxHandle_t sfxHandle, S32 volume, S32 soundTime );
    virtual S32 GetSoundLength( sfxHandle_t sfxHandle );
    virtual S32 GetCurrentSoundTime( void );
    
    // recompute the reletive volumes for all running sounds
    // reletive to the given entityNum / orientation
    virtual void Respatialize( S32 entityNum, const vec3_t origin, vec3_t axis[3], S32 inwater );
    
    // let the sound system know where an entity currently is
    virtual void UpdateEntityPosition( S32 entityNum, const vec3_t origin );
    
    virtual void Update( void );
    virtual void DisableSounds( void );
    virtual void BeginRegistration( void );
    
    // RegisterSound will allways return a valid sample, even if it
    // has to create a placeholder.  This prevents continuous filesystem
    // checks for missing files
    virtual sfxHandle_t RegisterSound( StringEntry name, bool compressed );
    virtual void DisplayFreeMemory( void );
    virtual S32 GetVoiceAmplitude( S32 entityNum );
    virtual void Reload( void );
    virtual void StopLoopingSound( S32 entityNum );
    virtual S32 SoundDuration( sfxHandle_t handle );
};

extern idSoundSystemLocal soundSystemLocal;

#endif // !__SND_LOCAL_H__

