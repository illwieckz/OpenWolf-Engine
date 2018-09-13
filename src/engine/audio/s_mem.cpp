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
// File name:   s_mem.cpp
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#include <OWLib/precompiled.h>

#define DEF_COMSOUNDMEGS "32"

/*
===============================================================================

memory management

===============================================================================
*/

static sndBuffer* buffer = NULL;
static sndBuffer* freelist = NULL;
static S32 inUse = 0;
static S32 totalInUse = 0;

S16* sfxScratchBuffer = NULL;
sfx_t* sfxScratchPointer = NULL;
S32 sfxScratchIndex = 0;

void SND_free( sndBuffer* v )
{
    *( sndBuffer** ) v = freelist;
    freelist = ( sndBuffer* ) v;
    inUse += sizeof( sndBuffer );
}

sndBuffer* SND_malloc( void )
{
    sndBuffer* v;
    
    while( !freelist )
    {
        S_FreeOldestSound();
    }
    
    inUse -= sizeof( sndBuffer );
    totalInUse += sizeof( sndBuffer );
    
    v = freelist;
    freelist = *( sndBuffer** ) freelist;
    v->next = NULL;
    return v;
}

void SND_setup( void )
{
    sndBuffer* p, *q;
    cvar_t* cv;
    S32 scs;
    
    cv = Cvar_Get( "com_soundMegs", DEF_COMSOUNDMEGS, CVAR_LATCH | CVAR_ARCHIVE );
    
    scs = ( cv->integer * 1536 );
    
    buffer = ( sndBuffer* )malloc( scs * sizeof( sndBuffer ) );
    
    // allocate the stack based hunk allocator
    sfxScratchBuffer = ( S16* )malloc( SND_CHUNK_SIZE * sizeof( S16 ) * 4 );	//Hunk_Alloc(SND_CHUNK_SIZE * sizeof(short) * 4);
    sfxScratchPointer = NULL;
    
    inUse = scs * sizeof( sndBuffer );
    p = buffer;;
    q = p + scs;
    while( --q > p )
        *( sndBuffer** ) q = q - 1;
        
    *( sndBuffer** ) q = NULL;
    freelist = p + scs - 1;
    
    Com_Printf( "Sound memory manager started\n" );
}

/*
================
ResampleSfx

resample / decimate to the current source rate
================
*/
static void ResampleSfx( sfx_t* sfx, S32 inrate, S32 inwidth, U8* data, bool compressed )
{
    S32 outcount, srcsample, i, sample, samplefrac, fracstep, part;
    F32 stepscale;
    sndBuffer* chunk;
    
    stepscale = ( F32 )inrate / dma.speed;	// this is usually 0.5, 1, or 2
    
    outcount = sfx->soundLength / stepscale;
    sfx->soundLength = outcount;
    
    samplefrac = 0;
    fracstep = stepscale * 256;
    chunk = sfx->soundData;
    
    for( i = 0; i < outcount; i++ )
    {
        srcsample = samplefrac >> 8;
        samplefrac += fracstep;
        if( inwidth == 2 )
        {
            sample = ( ( ( S16* )data )[srcsample] );
        }
        else
        {
            sample = ( S32 )( ( U8 )( data[srcsample] ) - 128 ) << 8;
        }
        part = ( i & ( SND_CHUNK_SIZE - 1 ) );
        if( part == 0 )
        {
            sndBuffer* newchunk;
            
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
    U8*           data;
    S16*          samples;
    snd_info_t      info;
    
    // player specific sounds are never directly loaded
    if( sfx->soundName[0] == '*' )
    {
        return false;
    }
    
    // load it in
    data = ( U8* )S_CodecLoad( sfx->soundName, &info );
    if( !data )
        return false;
        
    if( info.width == 1 )
    {
        Com_DPrintf( S_COLOR_YELLOW "WARNING: %s is a 8 bit wav file\n", sfx->soundName );
    }
    
    if( info.rate != 22050 )
    {
        Com_DPrintf( S_COLOR_YELLOW "WARNING: %s is not a 22kHz wav file\n", sfx->soundName );
    }
    
    samples = ( S16* )Hunk_AllocateTempMemory( info.samples * sizeof( S16 ) * 2 );
    
    sfx->lastTimeUsed = Com_Milliseconds() + 1;
    
    sfx->soundLength = info.samples;
    sfx->soundData = NULL;
    ResampleSfx( sfx, info.rate, info.width, data + info.dataofs, false );
    
    Hunk_FreeTempMemory( samples );
    Z_Free( data );
    
    return true;
}

void idSoundSystemLocal::DisplayFreeMemory( void )
{
    Com_Printf( "%d bytes free sound buffer memory, %d total used\n", inUse, totalInUse );
}
