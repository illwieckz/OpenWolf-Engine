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
// File name:   snd_mem.cpp
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description: sound caching
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#include <audio/snd_local.h>

#define DEF_COMSOUNDMEGS "24"    // (SA) upped for GD

/*
===============================================================================

SOUND MEMORY MANAGENT

===============================================================================
*/

static sndBuffer*   buffer = NULL;
static sndBuffer*   freelist = NULL;
static S32 inUse = 0;
static S32 totalInUse = 0;
static S32 totalAllocated = 0;

S16* sfxScratchBuffer = NULL;
const sfx_t* sfxScratchPointer = NULL;
S32 sfxScratchIndex = 0;

extern cvar_t*   s_nocompressed;

/*
================
SND_free
================
*/
void SND_free( sndBuffer* v )
{
    *( sndBuffer** )v = freelist;
    freelist = ( sndBuffer* )v;
    inUse += sizeof( sndBuffer );
    totalInUse -= sizeof( sndBuffer );
}

/*
================
SND_malloc
================
*/
sndBuffer*  SND_malloc()
{
    sndBuffer* v;
    
    while( freelist == NULL )
    {
        S_FreeOldestSound();
    }
    
    inUse -= sizeof( sndBuffer );
    totalInUse += sizeof( sndBuffer );
    totalAllocated += sizeof( sndBuffer );
    
    v = freelist;
    freelist = *( sndBuffer** )freelist;
    v->next = NULL;
    return v;
}

/*
================
SND_setup
================
*/
void SND_setup()
{
    sndBuffer* p, *q;
    cvar_t*  cv;
    S32 scs;
    
    cv = Cvar_Get( "com_soundMegs", DEF_COMSOUNDMEGS, CVAR_LATCH | CVAR_ARCHIVE );
    
    scs = cv->integer * 512;
    
    buffer = ( sndBuffer* )malloc( scs * sizeof( sndBuffer ) );
    // allocate the stack based hunk allocator
    sfxScratchBuffer = ( S16* )malloc( SND_CHUNK_SIZE * sizeof( S16 ) * 4 ); //Hunk_Alloc(SND_CHUNK_SIZE * sizeof(U16) * 4);
    sfxScratchPointer = NULL;
    
    inUse = scs * sizeof( sndBuffer );
    totalInUse = 0;
    totalAllocated = 0;
    p = buffer;;
    q = p + scs;
    while( --q > p )
    {
        *( sndBuffer** )q = q - 1;
    }
    *( sndBuffer** )q = NULL;
    freelist = p + scs - 1;
    
    Com_Printf( "Sound memory manager started\n" );
}

/*
===============================================================================

WAV loading

===============================================================================
*/

static U8*    data_p;
static U8*    iff_end;
static U8*    last_chunk;
static U8*    iff_data;
static S32 iff_chunk_len;

/*
================
GetLittleShort
================
*/
static S16 GetLittleShort( void )
{
    S16 val = 0;
    val = *data_p;
    val = val + ( *( data_p + 1 ) << 8 );
    data_p += 2;
    return val;
}

/*
================
GetLittleLong
================
*/
static S32 GetLittleLong( void )
{
    S32 val = 0;
    val = *data_p;
    val = val + ( *( data_p + 1 ) << 8 );
    val = val + ( *( data_p + 2 ) << 16 );
    val = val + ( *( data_p + 3 ) << 24 );
    data_p += 4;
    return val;
}

/*
================
FindNextChunk
================
*/
static void FindNextChunk( UTF8* name )
{
    while( 1 )
    {
        data_p = last_chunk;
        
        if( data_p >= iff_end )    // didn't find the chunk
        {
            data_p = NULL;
            return;
        }
        
        data_p += 4;
        iff_chunk_len = GetLittleLong();
        if( iff_chunk_len < 0 )
        {
            data_p = NULL;
            return;
        }
        data_p -= 8;
        last_chunk = data_p + 8 + ( ( iff_chunk_len + 1 ) & ~1 );
        if( !strncmp( ( UTF8* )data_p, name, 4 ) )
        {
            return;
        }
    }
}

/*
================
FindChunk
================
*/
static void FindChunk( UTF8* name )
{
    last_chunk = iff_data;
    FindNextChunk( name );
}

typedef struct waveFormat_s
{
    StringEntry  name;
    S32 format;
} waveFormat_t;

static waveFormat_t waveFormats[] =
{
    { "Windows PCM", 1 },
    { "Antex ADPCM", 14 },
    { "Antex ADPCME", 33 },
    { "Antex ADPCM", 40 },
    { "Audio Processing Technology", 25 },
    { "Audiofile, Inc.", 24 },
    { "Audiofile, Inc.", 26 },
    { "Control Resources Limited", 34 },
    { "Control Resources Limited", 37 },
    { "Creative ADPCM", 200 },
    { "Dolby Laboratories", 30 },
    { "DSP Group, Inc", 22 },
    { "DSP Solutions, Inc.", 15 },
    { "DSP Solutions, Inc.", 16 },
    { "DSP Solutions, Inc.", 35 },
    { "DSP Solutions ADPCM", 36 },
    { "Echo Speech Corporation", 23 },
    { "Fujitsu Corp.", 300 },
    { "IBM Corporation", 5 },
    { "Ing C. Olivetti & C., S.p.A.", 1000 },
    { "Ing C. Olivetti & C., S.p.A.", 1001 },
    { "Ing C. Olivetti & C., S.p.A.", 1002 },
    { "Ing C. Olivetti & C., S.p.A.", 1003 },
    { "Ing C. Olivetti & C., S.p.A.", 1004 },
    { "Intel ADPCM", 11 },
    { "Intel ADPCM", 11 },
    { "Unknown", 0 },
    { "Microsoft ADPCM", 2 },
    { "Microsoft Corporation", 6 },
    { "Microsoft Corporation", 7 },
    { "Microsoft Corporation", 31 },
    { "Microsoft Corporation", 50 },
    { "Natural MicroSystems ADPCM", 38 },
    { "OKI ADPCM", 10 },
    { "Sierra ADPCM", 13 },
    { "Speech Compression", 21 },
    { "Videologic ADPCM", 12 },
    { "Yamaha ADPCM", 20 },
    { NULL, 0 }
};

static StringEntry GetWaveFormatName( const S32 format )
{
    S32 i = 0;
    
    while( waveFormats[i].name )
    {
        if( format == waveFormats[i].format )
        {
            return( waveFormats[i].name );
        }
        i++;
    }
    
    return( "Unknown" );
    
}

/*
============
GetWavinfo
============
*/
static wavinfo_t GetWavinfo( UTF8* name, U8* wav, S32 wavlength )
{
    wavinfo_t info;
    
    ::memset( &info, 0, sizeof( info ) );
    
    if( !wav )
    {
        return info;
    }
    
    iff_data = wav;
    iff_end = wav + wavlength;
    
// find "RIFF" chunk
    FindChunk( "RIFF" );
    if( !( data_p && !strncmp( ( UTF8* )data_p + 8, "WAVE", 4 ) ) )
    {
        Com_Printf( "Missing RIFF/WAVE chunks\n" );
        return info;
    }
    
// get "fmt " chunk
    iff_data = data_p + 12;
// DumpChunks ();

    FindChunk( "fmt " );
    if( !data_p )
    {
        Com_Printf( "Missing fmt chunk\n" );
        return info;
    }
    data_p += 8;
    info.format = GetLittleShort();
    info.channels = GetLittleShort();
    info.rate = GetLittleLong();
    data_p += 4 + 2;
    info.width = GetLittleShort() / 8;
    
    if( info.format != 1 )
    {
        Com_Printf( "Unsupported format: %s\n", GetWaveFormatName( info.format ) );
        Com_Printf( "Microsoft PCM format only\n" );
        return info;
    }
    
// find data chunk
    FindChunk( "data" );
    if( !data_p )
    {
        Com_Printf( "Missing data chunk\n" );
        return info;
    }
    
    data_p += 4;
    info.samples = GetLittleLong() / info.width;
    info.dataofs = data_p - wav;
    
    return info;
}

/*
================
ResampleSfx

resample / decimate to the current source rate
================
*/
static void ResampleSfx( sfx_t* sfx, S32 inrate, S32 inwidth, U8* data, bool compressed )
{
    S32 outcount;
    S32 srcsample;
    F32 stepscale;
    S32 i;
    S32 sample, samplefrac, fracstep;
    S32 part;
    sndBuffer*   chunk;
    
    stepscale = ( F32 )inrate / dma.speed; // this is usually 0.5, 1, or 2
    
    outcount = sfx->soundLength / stepscale;
    sfx->soundLength = outcount;
    
    samplefrac = 0;
    fracstep = stepscale * 256;
    chunk = sfx->soundData;
    
    // Gordon: use the littleshort version only if we need to
    if( LittleShort( 256 ) == 256 )
    {
        for( i = 0 ; i < outcount ; i++ )
        {
            srcsample = samplefrac >> 8;
            samplefrac += fracstep;
            if( inwidth == 2 )
            {
                sample = ( ( S16* )data )[srcsample];
            }
            else
            {
                sample = ( S32 )( ( U8 )( data[srcsample] ) - 128 ) << 8;
            }
            part  = ( i & ( SND_CHUNK_SIZE - 1 ) );
            if( part == 0 )
            {
                sndBuffer*   newchunk;
                newchunk = SND_malloc();
                if( chunk == NULL )
                {
                    sfx->soundData = newchunk;
                }
                else
                {
                    chunk->next = newchunk;
                }
                chunk = newchunk;
            }
            
            chunk->sndChunk[part] = sample;
        }
    }
    else
    {
        for( i = 0 ; i < outcount ; i++ )
        {
            srcsample = samplefrac >> 8;
            samplefrac += fracstep;
            if( inwidth == 2 )
            {
                sample = LittleShort( ( ( S16* )data )[srcsample] );
            }
            else
            {
                sample = ( S32 )( ( U8 )( data[srcsample] ) - 128 ) << 8;
            }
            part  = ( i & ( SND_CHUNK_SIZE - 1 ) );
            if( part == 0 )
            {
                sndBuffer*   newchunk;
                newchunk = SND_malloc();
                if( chunk == NULL )
                {
                    sfx->soundData = newchunk;
                }
                else
                {
                    chunk->next = newchunk;
                }
                chunk = newchunk;
            }
            
            chunk->sndChunk[part] = sample;
        }
    }
}

/*
================
ResampleSfx

resample / decimate to the current source rate
================
*/
static S32 ResampleSfxRaw( S16* sfx, S32 inrate, S32 inwidth, S32 samples, U8* data )
{
    S32 outcount;
    S32 srcsample;
    F32 stepscale;
    S32 i;
    S32 sample, samplefrac, fracstep;
    
    stepscale = ( F32 )inrate / dma.speed; // this is usually 0.5, 1, or 2
    
    outcount = samples / stepscale;
    
    samplefrac = 0;
    fracstep = stepscale * 256;
    
    // Gordon: use the littleshort version only if we need to
    if( LittleShort( 256 ) == 256 )
    {
        for( i = 0 ; i < outcount ; i++ )
        {
            srcsample = samplefrac >> 8;
            samplefrac += fracstep;
            if( inwidth == 2 )
            {
                sample = ( ( S16* )data )[srcsample];
            }
            else
            {
                sample = ( S32 )( ( U8 )( data[srcsample] ) - 128 ) << 8;
            }
            sfx[i] = sample;
        }
    }
    else
    {
        for( i = 0 ; i < outcount ; i++ )
        {
            srcsample = samplefrac >> 8;
            samplefrac += fracstep;
            if( inwidth == 2 )
            {
                sample = LittleShort( ( ( U16* )data )[srcsample] );
            }
            else
            {
                sample = ( S32 )( ( U8 )( data[srcsample] ) - 128 ) << 8;
            }
            sfx[i] = sample;
        }
    }
    return outcount;
}


//=============================================================================

/*
==============
S_LoadSound

The filename may be different than sfx->name in the case
of a forced fallback of a player specific sound
==============
*/
bool S_LoadSound( sfx_t* sfx )
{
    U8*    data;
    S16*   samples;
    wavinfo_t info;
    S32 size;
    
    // player specific sounds are never directly loaded
    if( sfx->soundName[0] == '*' )
    {
        return false;
    }
    
    // load it in
    size = FS_ReadFile( sfx->soundName, ( void** )&data );
    if( !data )
    {
        return false;
    }
    
    info = GetWavinfo( sfx->soundName, data, size );
    if( info.channels != 1 )
    {
        Com_Printf( "%s is a stereo wav file\n", sfx->soundName );
        FS_FreeFile( data );
        return false;
    }
    
    if( info.width == 1 )
    {
        Com_DPrintf( S_COLOR_YELLOW "WARNING: %s is a 8 bit wav file\n", sfx->soundName );
    }
    
    if( info.rate != 22050 )
    {
        Com_DPrintf( S_COLOR_YELLOW "WARNING: %s is not a 22kHz wav file\n", sfx->soundName );
    }
    
    samples = ( S16* )Hunk_AllocateTempMemory( info.samples * sizeof( S16 ) * 2 );
    
    // DHM - Nerve
    sfx->lastTimeUsed = Sys_Milliseconds() + 1;
    
    // each of these compression schemes works just fine
    // but the 16bit quality is much nicer and with a local
    // install assured we can rely upon the sound memory
    // manager to do the right thing for us and page
    // sound in as needed
    
    
    if( s_nocompressed->value )
    {
        sfx->soundCompressionMethod = 0;
        sfx->soundLength = info.samples;
        sfx->soundData = NULL;
        ResampleSfx( sfx, info.rate, info.width, data + info.dataofs, false );
    }
    else if( sfx->soundCompressed == true )
    {
        sfx->soundCompressionMethod = 1;
        sfx->soundData = NULL;
        sfx->soundLength = ResampleSfxRaw( samples, info.rate, info.width, info.samples, ( data + info.dataofs ) );
        S_AdpcmEncodeSound( sfx, samples );
#ifdef COMPRESSION
    }
    else if( info.samples > ( SND_CHUNK_SIZE * 16 ) && info.width > 1 )
    {
        sfx->soundCompressionMethod = 3;
        sfx->soundData = NULL;
        sfx->soundLength = ResampleSfxRaw( samples, info.rate, info.width, info.samples, ( data + info.dataofs ) );
        encodeMuLaw( sfx, samples );
    }
    else if( info.samples > ( SND_CHUNK_SIZE * 6400 ) && info.width > 1 )
    {
        sfx->soundCompressionMethod = 2;
        sfx->soundData = NULL;
        sfx->soundLength = ResampleSfxRaw( samples, info.rate, info.width, info.samples, ( data + info.dataofs ) );
        encodeWavelet( sfx, samples );
#endif
    }
    else
    {
        sfx->soundCompressionMethod = 0;
        sfx->soundLength = info.samples;
        sfx->soundData = NULL;
        ResampleSfx( sfx, info.rate, info.width, data + info.dataofs, false );
    }
    Hunk_FreeTempMemory( samples );
    FS_FreeFile( data );
    
    return true;
}

/*
================
idSoundSystemLocal::DisplayFreeMemory
================
*/
void idSoundSystemLocal::DisplayFreeMemory( void )
{
    Com_Printf( "%d bytes (%.2fMB) free sound buffer memory, %d bytes (%.2fMB) total used\n%d bytes (%.2fMB) sound buffer memory have been allocated since the last SND_setup", inUse, inUse / Square( 1024.f ), totalInUse, totalInUse / Square( 1024.f ), totalAllocated, totalAllocated / Square( 1024.f ) );
}
