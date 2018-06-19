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
// File name:   snd_mix.cpp
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description: portable code to mix sounds for snd_dma.c
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#include <audio/snd_local.h>

portable_samplepair_t paintbuffer[PAINTBUFFER_SIZE];
static S32 snd_vol;

// TTimo not static, required by unix/snd_mixa.s
S32*     snd_p;
S32 snd_linear_count;
S16*   snd_out;

/*
===================
S_WriteLinearBlastStereo16
===================
*/
void S_WriteLinearBlastStereo16( void )
{
    S32 i;
    S32 val;
    
    for( i = 0 ; i < snd_linear_count ; i += 2 )
    {
        val = snd_p[i] >> 8;
        if( val > 0x7fff )
        {
            snd_out[i] = 0x7fff;
        }
        else if( val < ( S16 )0x8000 )
        {
            snd_out[i] = ( S16 )0x8000;
        }
        else
        {
            snd_out[i] = val;
        }
        
        val = snd_p[i + 1] >> 8;
        if( val > 0x7fff )
        {
            snd_out[i + 1] = 0x7fff;
        }
        else if( val < ( S16 )0x8000 )
        {
            snd_out[i + 1] = ( S16 )0x8000;
        }
        else
        {
            snd_out[i + 1] = val;
        }
    }
}

/*
===================
S_TransferStereo16
===================
*/
void S_TransferStereo16( U64* pbuf, S32 endtime )
{
    S32 lpos;
    S32 ls_paintedtime;
    
    snd_p = ( S32* ) paintbuffer;
    ls_paintedtime = s_paintedtime;
    
    while( ls_paintedtime < endtime )
    {
        // handle recirculating buffer issues
        lpos = ls_paintedtime & ( ( dma.samples >> 1 ) - 1 );
        
        snd_out = ( S16* ) pbuf + ( lpos << 1 );
        
        snd_linear_count = ( dma.samples >> 1 ) - lpos;
        if( ls_paintedtime + snd_linear_count > endtime )
        {
            snd_linear_count = endtime - ls_paintedtime;
        }
        
        snd_linear_count <<= 1;
        
        // write a linear blast of samples
        S_WriteLinearBlastStereo16();
        
        snd_p += snd_linear_count;
        ls_paintedtime += ( snd_linear_count >> 1 );
    }
}

/*
===================
S_TransferPaintBuffer
===================
*/
void S_TransferPaintBuffer( S32 endtime )
{
    S32 out_idx;
    S32 count;
    S32 out_mask;
    S32*     p;
    S32 step;
    S32 val;
    U64* pbuf;
    
    pbuf = ( U64* )dma.buffer;
    if( !pbuf )
    {
        return;
    }
    
    if( s_testsound->integer )
    {
        S32 i;
        S32 count;
        
        // write a fixed sine wave
        count = ( endtime - s_paintedtime );
        for( i = 0 ; i < count ; i++ )
            paintbuffer[i].left = paintbuffer[i].right = sin( ( s_paintedtime + i ) * 0.1 ) * 20000 * 256;
    }
    
    
    if( dma.samplebits == 16 && dma.channels == 2 )
    {
        // optimized case
        S_TransferStereo16( pbuf, endtime );
    }
    else
    {
        // general case
        p = ( S32* ) paintbuffer;
        count = ( endtime - s_paintedtime ) * dma.channels;
        out_mask = dma.samples - 1;
        out_idx = s_paintedtime * dma.channels & out_mask;
        step = 3 - dma.channels;
        
        if( dma.samplebits == 16 )
        {
            S16* out = ( S16* ) pbuf;
            while( count-- )
            {
                val = *p >> 8;
                p += step;
                if( val > 0x7fff )
                {
                    val = 0x7fff;
                }
                else if( val < -32768 )
                {
                    val = -32768;
                }
                out[out_idx] = val;
                out_idx = ( out_idx + 1 ) & out_mask;
            }
        }
        else if( dma.samplebits == 8 )
        {
            U8* out = ( U8* ) pbuf;
            while( count-- )
            {
                val = *p >> 8;
                p += step;
                if( val > 0x7fff )
                {
                    val = 0x7fff;
                }
                else if( val < -32768 )
                {
                    val = -32768;
                }
                out[out_idx] = ( val >> 8 ) + 128;
                out_idx = ( out_idx + 1 ) & out_mask;
            }
        }
    }
}

/*
===============================================================================

LIP SYNCING

===============================================================================
*/

#ifdef TALKANIM

U8 s_entityTalkAmplitude[MAX_CLIENTS];

/*
===================
S_SetVoiceAmplitudeFrom16
===================
*/
void S_SetVoiceAmplitudeFrom16( const sfx_t* sc, S32 sampleOffset, S32 count, S32 entnum )
{
    S32 data, i, sfx_count;
    sndBuffer* chunk;
    S16* samples;
    
    if( count <= 0 )
    {
        return; // must have gone ahead of the end of the sound
    }
    chunk = sc->soundData;
    while( sampleOffset >= SND_CHUNK_SIZE )
    {
        chunk = chunk->next;
        sampleOffset -= SND_CHUNK_SIZE;
        if( !chunk )
        {
            chunk = sc->soundData;
        }
    }
    
    sfx_count = 0;
    samples = chunk->sndChunk;
    for( i = 0; i < count; i++ )
    {
        if( sampleOffset >= SND_CHUNK_SIZE )
        {
            chunk = chunk->next;
            samples = chunk->sndChunk;
            sampleOffset = 0;
        }
        data  = samples[sampleOffset++];
        if( abs( data ) > 5000 )
        {
            sfx_count += ( data * 255 ) >> 8;
        }
    }
    //Com_Printf("Voice sfx_count = %d, count = %d\n", sfx_count, count );
    // adjust the sfx_count according to the frametime (scale down for longer frametimes)
    sfx_count = abs( sfx_count );
    sfx_count = ( S32 )( ( F32 )sfx_count / ( 2.0 * ( F32 )count ) );
    if( sfx_count > 255 )
    {
        sfx_count = 255;
    }
    if( sfx_count < 25 )
    {
        sfx_count = 0;
    }
    //Com_Printf("sfx_count = %d\n", sfx_count );
    // update the amplitude for this entity
    s_entityTalkAmplitude[entnum] = ( U8 )sfx_count;
}

/*
===================
S_SetVoiceAmplitudeFromADPCM
===================
*/
void S_SetVoiceAmplitudeFromADPCM( const sfx_t* sc, S32 sampleOffset, S32 count, S32 entnum )
{
    S32 data, i, sfx_count;
    sndBuffer* chunk;
    S16* samples;
    
    if( count <= 0 )
    {
        return; // must have gone ahead of the end of the sound
    }
    i = 0;
    chunk = sc->soundData;
    while( sampleOffset >= ( SND_CHUNK_SIZE * 4 ) )
    {
        chunk = chunk->next;
        sampleOffset -= ( SND_CHUNK_SIZE * 4 );
        i++;
    }
    
    if( i != sfxScratchIndex || sfxScratchPointer != sc )
    {
        S_AdpcmGetSamples( chunk, sfxScratchBuffer );
        sfxScratchIndex = i;
        sfxScratchPointer = sc;
    }
    
    sfx_count = 0;
    samples = sfxScratchBuffer;
    for( i = 0; i < count; i++ )
    {
        if( sampleOffset >= SND_CHUNK_SIZE * 4 )
        {
            chunk = chunk->next;
            S_AdpcmGetSamples( chunk, sfxScratchBuffer );
            sampleOffset = 0;
            sfxScratchIndex++;
        }
        data  = samples[sampleOffset++];
        if( abs( data ) > 5000 )
        {
            sfx_count += ( data * 255 ) >> 8;
        }
    }
    //Com_Printf("Voice sfx_count = %d, count = %d\n", sfx_count, count );
    // adjust the sfx_count according to the frametime (scale down for longer frametimes)
    sfx_count = abs( sfx_count );
    sfx_count = ( S32 )( ( F32 )sfx_count / ( 2.0 * ( F32 )count ) );
    if( sfx_count > 255 )
    {
        sfx_count = 255;
    }
    if( sfx_count < 25 )
    {
        sfx_count = 0;
    }
    //Com_Printf("sfx_count = %d\n", sfx_count );
    // update the amplitude for this entity
    s_entityTalkAmplitude[entnum] = ( U8 )sfx_count;
}

/*
===================
S_SetVoiceAmplitudeFromWavelet
===================
*/
void S_SetVoiceAmplitudeFromWavelet( const sfx_t* sc, S32 sampleOffset, S32 count, S32 entnum )
{
    S32 data, i, sfx_count;
    sndBuffer* chunk;
    S16* samples;
    
    if( count <= 0 )
    {
        return; // must have gone ahead of the end of the sound
    }
    i = 0;
    chunk = sc->soundData;
    while( sampleOffset >= ( SND_CHUNK_SIZE_FLOAT * 4 ) )
    {
        chunk = chunk->next;
        sampleOffset -= ( SND_CHUNK_SIZE_FLOAT * 4 );
        i++;
    }
    if( i != sfxScratchIndex || sfxScratchPointer != sc )
    {
        decodeWavelet( chunk, sfxScratchBuffer );
        sfxScratchIndex = i;
        sfxScratchPointer = sc;
    }
    sfx_count = 0;
    samples = sfxScratchBuffer;
    for( i = 0; i < count; i++ )
    {
        if( sampleOffset >= ( SND_CHUNK_SIZE_FLOAT * 4 ) )
        {
            chunk = chunk->next;
            decodeWavelet( chunk, sfxScratchBuffer );
            sfxScratchIndex++;
            sampleOffset = 0;
        }
        data = samples[sampleOffset++];
        if( abs( data ) > 5000 )
        {
            sfx_count += ( data * 255 ) >> 8;
        }
    }
    
    //Com_Printf("Voice sfx_count = %d, count = %d\n", sfx_count, count );
    // adjust the sfx_count according to the frametime (scale down for longer frametimes)
    sfx_count = abs( sfx_count );
    sfx_count = ( S32 )( ( F32 )sfx_count / ( 2.0 * ( F32 )count ) );
    if( sfx_count > 255 )
    {
        sfx_count = 255;
    }
    if( sfx_count < 25 )
    {
        sfx_count = 0;
    }
    //Com_Printf("sfx_count = %d\n", sfx_count );
    // update the amplitude for this entity
    s_entityTalkAmplitude[entnum] = ( U8 )sfx_count;
}

/*
===================
S_SetVoiceAmplitudeFromMuLaw
===================
*/
void S_SetVoiceAmplitudeFromMuLaw( const sfx_t* sc, S32 sampleOffset, S32 count, S32 entnum )
{
    S32 data, i, sfx_count;
    sndBuffer* chunk;
    U8* samples;
    
    if( count <= 0 )
    {
        return; // must have gone ahead of the end of the sound
    }
    chunk = sc->soundData;
    while( sampleOffset >= ( SND_CHUNK_SIZE * 2 ) )
    {
        chunk = chunk->next;
        sampleOffset -= ( SND_CHUNK_SIZE * 2 );
        if( !chunk )
        {
            chunk = sc->soundData;
        }
    }
    sfx_count = 0;
    samples = ( U8* )chunk->sndChunk + sampleOffset;
    for( i = 0; i < count; i++ )
    {
        if( samples >= ( U8* )chunk->sndChunk + ( SND_CHUNK_SIZE * 2 ) )
        {
            chunk = chunk->next;
            samples = ( U8* )chunk->sndChunk;
        }
        data  = mulawToShort[*samples];
        if( abs( data ) > 5000 )
        {
            sfx_count += ( data * 255 ) >> 8;
        }
        samples++;
    }
    //Com_Printf("Voice sfx_count = %d, count = %d\n", sfx_count, count );
    // adjust the sfx_count according to the frametime (scale down for longer frametimes)
    sfx_count = abs( sfx_count );
    sfx_count = ( S32 )( ( F32 )sfx_count / ( 2.0 * ( F32 )count ) );
    if( sfx_count > 255 )
    {
        sfx_count = 255;
    }
    if( sfx_count < 25 )
    {
        sfx_count = 0;
    }
    //Com_Printf("sfx_count = %d\n", sfx_count );
    // update the amplitude for this entity
    s_entityTalkAmplitude[entnum] = ( U8 )sfx_count;
}

/*
===================
idSoundSystemLocal::GetVoiceAmplitude
===================
*/
S32 idSoundSystemLocal::GetVoiceAmplitude( S32 entityNum )
{
    if( entityNum >= MAX_CLIENTS )
    {
        Com_Printf( "Error: idSoundSystemLocal::GetVoiceAmplitude() called for a non-client\n" );
        return 0;
    }
    
    return ( S32 )s_entityTalkAmplitude[entityNum];
}
#else

// NERVE - SMF
S32 S_GetVoiceAmplitude( S32 entityNum )
{
    return 0;
}
// -NERVE - SMF

#endif

/*
===============================================================================

CHANNEL MIXING

===============================================================================
*/

/*
===================
S_PaintChannelFrom16
===================
*/
static void S_PaintChannelFrom16( channel_t* ch, const sfx_t* sc, S32 count, S32 sampleOffset, S32 bufferOffset )
{
    S32 data, aoff, boff;
    S32 leftvol, rightvol;
    S32 i, j;
    portable_samplepair_t*   samp;
    sndBuffer*               chunk;
    S16*                   samples;
    F32 ooff, fdata, fdiv, fleftvol, frightvol;
    
    samp = &paintbuffer[ bufferOffset ];
    
    if( ch->doppler )
    {
        sampleOffset = sampleOffset * ch->oldDopplerScale;
    }
    
    chunk = sc->soundData;
    while( sampleOffset >= SND_CHUNK_SIZE )
    {
        chunk = chunk->next;
        sampleOffset -= SND_CHUNK_SIZE;
        if( !chunk )
        {
            chunk = sc->soundData;
        }
    }
    
    if( !ch->doppler )
    {
        leftvol = ch->leftvol * snd_vol;
        rightvol = ch->rightvol * snd_vol;
        
        samples = chunk->sndChunk;
        for( i = 0; i < count; i++ )
        {
            if( sampleOffset >= SND_CHUNK_SIZE )
            {
                chunk = chunk->next;
                if( chunk == NULL )
                {
                    chunk = sc->soundData;
                }
                samples = chunk->sndChunk;
                sampleOffset -= SND_CHUNK_SIZE;
            }
            data  = samples[sampleOffset++];
            samp[i].left += ( data * leftvol ) >> 8;
            samp[i].right += ( data * rightvol ) >> 8;
        }
    }
    else
    {
        fleftvol = ch->leftvol * snd_vol;
        frightvol = ch->rightvol * snd_vol;
        
        ooff = sampleOffset;
        samples = chunk->sndChunk;
        
        for( i = 0 ; i < count ; i++ )
        {
            aoff = ooff;
            ooff = ooff + ch->dopplerScale;
            boff = ooff;
            fdata = 0;
            for( j = aoff; j < boff; j++ )
            {
                if( j >= SND_CHUNK_SIZE )
                {
                    chunk = chunk->next;
                    if( !chunk )
                    {
                        chunk = sc->soundData;
                    }
                    samples = chunk->sndChunk;
                    ooff -= SND_CHUNK_SIZE;
                }
                fdata += samples[j & ( SND_CHUNK_SIZE - 1 )];
            }
            fdiv = 256 * ( boff - aoff );
            samp[i].left += ( fdata * fleftvol ) / fdiv;
            samp[i].right += ( fdata * frightvol ) / fdiv;
        }
    }
}

/*
===================
S_PaintChannelFromWavelet
===================
*/
void S_PaintChannelFromWavelet( channel_t* ch, sfx_t* sc, S32 count, S32 sampleOffset, S32 bufferOffset )
{
    S32 data;
    S32 leftvol, rightvol;
    S32 i;
    portable_samplepair_t*   samp;
    sndBuffer*               chunk;
    S16*                   samples;
    
    leftvol = ch->leftvol * snd_vol;
    rightvol = ch->rightvol * snd_vol;
    
    i = 0;
    samp = &paintbuffer[ bufferOffset ];
    chunk = sc->soundData;
    while( sampleOffset >= ( SND_CHUNK_SIZE_FLOAT * 4 ) )
    {
        chunk = chunk->next;
        sampleOffset -= ( SND_CHUNK_SIZE_FLOAT * 4 );
        i++;
    }
    
    if( i != sfxScratchIndex || sfxScratchPointer != sc )
    {
        decodeWavelet( chunk, sfxScratchBuffer );
        sfxScratchIndex = i;
        sfxScratchPointer = sc;
    }
    
    samples = sfxScratchBuffer;
    
    // FIXME: doppler
    
    for( i = 0; i < count; i++ )
    {
        if( sampleOffset >= ( SND_CHUNK_SIZE_FLOAT * 4 ) )
        {
            chunk = chunk->next;
            decodeWavelet( chunk, sfxScratchBuffer );
            sfxScratchIndex++;
            sampleOffset = 0;
        }
        data  = samples[sampleOffset++];
        samp[i].left += ( data * leftvol ) >> 8;
        samp[i].right += ( data * rightvol ) >> 8;
    }
}

/*
===================
S_PaintChannelFromADPCM
===================
*/
void S_PaintChannelFromADPCM( channel_t* ch, sfx_t* sc, S32 count, S32 sampleOffset, S32 bufferOffset )
{
    S32 data;
    S32 leftvol, rightvol;
    S32 i;
    portable_samplepair_t*   samp;
    sndBuffer*               chunk;
    S16*                   samples;
    
    leftvol = ch->leftvol * snd_vol;
    rightvol = ch->rightvol * snd_vol;
    
    i = 0;
    samp = &paintbuffer[ bufferOffset ];
    chunk = sc->soundData;
    
    if( ch->doppler )
    {
        sampleOffset = sampleOffset * ch->oldDopplerScale;
    }
    
    while( sampleOffset >= ( SND_CHUNK_SIZE * 4 ) )
    {
        chunk = chunk->next;
        sampleOffset -= ( SND_CHUNK_SIZE * 4 );
        i++;
    }
    
    if( i != sfxScratchIndex || sfxScratchPointer != sc )
    {
        S_AdpcmGetSamples( chunk, sfxScratchBuffer );
        sfxScratchIndex = i;
        sfxScratchPointer = sc;
    }
    
    samples = sfxScratchBuffer;
    for( i = 0; i < count; i++ )
    {
        if( sampleOffset >= SND_CHUNK_SIZE * 4 )
        {
            chunk = chunk->next;
            if( !chunk )
            {
                chunk = sc->soundData;
            }
            S_AdpcmGetSamples( chunk, sfxScratchBuffer );
            sampleOffset = 0;
            sfxScratchIndex++;
        }
        data = samples[sampleOffset++];
        samp[i].left += ( data * leftvol ) >> 8;
        samp[i].right += ( data * rightvol ) >> 8;
    }
}

/*
===================
S_PaintChannelFromMuLaw
===================
*/
void S_PaintChannelFromMuLaw( channel_t* ch, sfx_t* sc, S32 count, S32 sampleOffset, S32 bufferOffset )
{
    S32 data;
    S32 leftvol, rightvol;
    S32 i;
    portable_samplepair_t* samp;
    sndBuffer* chunk;
    U8* samples;
    F32 ooff;
    
    leftvol = ch->leftvol * snd_vol;
    rightvol = ch->rightvol * snd_vol;
    
    samp = &paintbuffer[ bufferOffset ];
    chunk = sc->soundData;
    while( sampleOffset >= ( SND_CHUNK_SIZE * 2 ) )
    {
        chunk = chunk->next;
        sampleOffset -= ( SND_CHUNK_SIZE * 2 );
        if( !chunk )
        {
            chunk = sc->soundData;
        }
    }
    
    if( !ch->doppler )
    {
        samples = ( U8* )chunk->sndChunk + sampleOffset;
        for( i = 0; i < count; i++ )
        {
            if( samples >= ( U8* )chunk->sndChunk + ( SND_CHUNK_SIZE * 2 ) )
            {
                chunk = chunk->next;
                samples = ( U8* )chunk->sndChunk;
            }
            data  = mulawToShort[*samples];
            samp[i].left += ( data * leftvol ) >> 8;
            samp[i].right += ( data * rightvol ) >> 8;
            samples++;
        }
    }
    else
    {
        ooff = sampleOffset;
        samples = ( U8* )chunk->sndChunk;
        for( i = 0; i < count; i++ )
        {
            if( ooff >= SND_CHUNK_SIZE * 2 )
            {
                chunk = chunk->next;
                if( !chunk )
                {
                    chunk = sc->soundData;
                }
                samples = ( U8* )chunk->sndChunk;
                ooff = 0.0;
            }
            data  = mulawToShort[samples[( S32 )( ooff )]];
            ooff = ooff + ch->dopplerScale;
            samp[i].left += ( data * leftvol ) >> 8;
            samp[i].right += ( data * rightvol ) >> 8;
        }
    }
}

#define TALK_FUTURE_SEC 0.25        // go this far into the future (seconds)

/*
===================
S_PaintChannels
===================
*/
void S_PaintChannels( S32 endtime )
{
    S32 i, si;
    S32 end;
    channel_t* ch;
    sfx_t*   sc;
    S32 ltime, count;
    S32 sampleOffset;
    streamingSound_t* ss;
    bool firstPass = true;
    
    if( s_mute->value )
    {
        snd_vol = 0;
    }
    else
    {
        snd_vol = s_volume->value * 256;
    }
    
    if( snd.volCurrent < 1 )    // only when fading (at map start/end)
    {
        snd_vol = ( S32 )( ( F32 )snd_vol * snd.volCurrent );
    }
    
    //Com_Printf ("%i to %i\n", s_paintedtime, endtime);
    while( s_paintedtime < endtime )
    {
        // if paintbuffer is smaller than DMA buffer
        // we may need to fill it multiple times
        end = endtime;
        if( endtime - s_paintedtime > PAINTBUFFER_SIZE )
        {
            //%	Com_DPrintf("endtime exceeds PAINTBUFFER_SIZE %i\n", endtime - s_paintedtime);
            end = s_paintedtime + PAINTBUFFER_SIZE;
        }
        
        // clear paint buffer for the current time
        ::memset( paintbuffer, 0, ( end - s_paintedtime ) * sizeof( portable_samplepair_t ) );
        // mix all streaming sounds into paint buffer
        for( si = 0, ss = streamingSounds; si < MAX_STREAMING_SOUNDS; si++, ss++ )
        {
            // if this streaming sound is still playing
            if( s_rawend[si] >= s_paintedtime )
            {
                // copy from the streaming sound source
                S32 s;
                S32 stop;
//				F32 fsir, fsil; // TTimo: unused

                stop = ( end < s_rawend[si] ) ? end : s_rawend[si];
                
                // precalculating this saves zillions of cycles
//DAJ				fsir = ((F32)s_rawVolume[si].left/255.0f);
//DAJ				fsil = ((F32)s_rawVolume[si].right/255.0f);
                for( i = s_paintedtime ; i < stop ; i++ )
                {
                    s = i & ( MAX_RAW_SAMPLES - 1 );
//DAJ					paintbuffer[i-s_paintedtime].left += (S32)((F32)s_rawsamples[si][s].left * fsir);
//DAJ					paintbuffer[i-s_paintedtime].right += (S32)((F32)s_rawsamples[si][s].right * fsil);
                    //DAJ even faster
                    paintbuffer[i - s_paintedtime].left += ( s_rawsamples[si][s].left * s_rawVolume[si].left ) >> 8;
                    paintbuffer[i - s_paintedtime].right += ( s_rawsamples[si][s].right * s_rawVolume[si].right ) >> 8;
                }
#ifdef TALKANIM
                if( firstPass && ss->channel == CHAN_VOICE && ss->entnum < MAX_CLIENTS )
                {
                    S32 talkcnt, talktime;
                    S32 sfx_count, vstop;
                    S32 data;
                    
                    // we need to go into the future, since the interpolated behaviour of the facial
                    // animation creates lag in the time it takes to display the current facial frame
                    talktime = s_paintedtime + ( S32 )( TALK_FUTURE_SEC * ( F32 )s_khz->integer * 1000 );
                    vstop = ( talktime + 100 < s_rawend[si] ) ? talktime + 100 : s_rawend[si];
                    talkcnt = 1;
                    sfx_count = 0;
                    
                    for( i = talktime ; i < vstop ; i++ )
                    {
                        s = i & ( MAX_RAW_SAMPLES - 1 );
                        data = abs( ( s_rawsamples[si][s].left ) / 8000 );
                        if( data > sfx_count )
                        {
                            sfx_count = data;
                        }
                    }
                    
                    if( sfx_count > 255 )
                    {
                        sfx_count = 255;
                    }
                    if( sfx_count < 25 )
                    {
                        sfx_count = 0;
                    }
                    
                    //Com_Printf("sfx_count = %d\n", sfx_count );
                    
                    // update the amplitude for this entity
                    // rain - the announcer is ent -1, so make sure we're >= 0
                    if( ss->entnum >= 0 )
                    {
                        s_entityTalkAmplitude[ss->entnum] = ( U8 )sfx_count;
                    }
                }
#endif
            }
        }
        
        // paint in the channels.
        ch = s_channels;
        for( i = 0; i < MAX_CHANNELS; i++, ch++ )
        {
            if( ch->startSample == START_SAMPLE_IMMEDIATE || !ch->thesfx || ( ch->leftvol < 0.25 && ch->rightvol < 0.25 ) )
            {
                continue;
            }
            
            ltime = s_paintedtime;
            sc = ch->thesfx;
            
            // (SA) hmm, why was this commented out?
            if( !sc->inMemory )
            {
                S_memoryLoad( sc );
            }
            
            sampleOffset = ltime - ch->startSample;
            count = end - ltime;
            if( sampleOffset + count > sc->soundLength )
            {
                count = sc->soundLength - sampleOffset;
            }
            
            if( count > 0 )
            {
#ifdef TALKANIM
                // Ridah, talking animations
                // TODO: check that this entity has talking animations enabled!
                if( firstPass && ch->entchannel == CHAN_VOICE && ch->entnum < MAX_CLIENTS )
                {
                    S32 talkofs, talkcnt, talktime;
                    // we need to go into the future, since the interpolated behaviour of the facial
                    // animation creates lag in the time it takes to display the current facial frame
                    talktime = ltime + ( S32 )( TALK_FUTURE_SEC * ( F32 )s_khz->integer * 1000 );
                    talkofs = talktime - ch->startSample;
                    talkcnt = 100;
                    if( talkofs + talkcnt < sc->soundLength )
                    {
                        if( sc->soundCompressionMethod == 1 )
                        {
                            S_SetVoiceAmplitudeFromADPCM( sc, talkofs, talkcnt, ch->entnum );
                        }
                        else if( sc->soundCompressionMethod == 2 )
                        {
                            S_SetVoiceAmplitudeFromWavelet( sc, talkofs, talkcnt, ch->entnum );
                        }
                        else if( sc->soundCompressionMethod == 3 )
                        {
                            S_SetVoiceAmplitudeFromMuLaw( sc, talkofs, talkcnt, ch->entnum );
                        }
                        else
                        {
                            S_SetVoiceAmplitudeFrom16( sc, talkofs, talkcnt, ch->entnum );
                        }
                    }
                }
#endif
                if( sc->soundCompressionMethod == 1 )
                {
                    S_PaintChannelFromADPCM( ch, sc, count, sampleOffset, ltime - s_paintedtime );
                }
                else if( sc->soundCompressionMethod == 2 )
                {
                    S_PaintChannelFromWavelet( ch, sc, count, sampleOffset, ltime - s_paintedtime );
                }
                else if( sc->soundCompressionMethod == 3 )
                {
                    S_PaintChannelFromMuLaw( ch, sc, count, sampleOffset, ltime - s_paintedtime );
                }
                else
                {
                    S_PaintChannelFrom16( ch, sc, count, sampleOffset, ltime - s_paintedtime );
                }
            }
        }
        
        // paint in the looped channels.
        ch = loop_channels;
        for( i = 0; i < numLoopChannels ; i++, ch++ )
        {
            if( !ch->thesfx || ( !ch->leftvol && !ch->rightvol ) )
            {
                continue;
            }
            
            ltime = s_paintedtime;
            sc = ch->thesfx;
            
            if( sc->soundData == NULL || sc->soundLength == 0 )
            {
                continue;
            }
            // we might have to make two passes if it
            // is a looping sound effect and the end of
            // the sample is hit
            do
            {
                //%	sampleOffset = (ltime % sc->soundLength);
                //%	sampleOffset = (ltime - ch->startSample) % sc->soundLength;	// ydnar
                sampleOffset = ( ltime /*- ch->startSample*/ ) % sc->soundLength; // ydnar
                
                count = end - ltime;
                if( sampleOffset + count > sc->soundLength )
                {
                    count = sc->soundLength - sampleOffset;
                }
                
                if( count > 0 )
                {
#ifdef TALKANIM
                    // Ridah, talking animations
                    // TODO: check that this entity has talking animations enabled!
                    if( firstPass && ch->entchannel == CHAN_VOICE && ch->entnum < MAX_CLIENTS )
                    {
                        S32 talkofs, talkcnt, talktime;
                        // we need to go into the future, since the interpolated behaviour of the facial
                        // animation creates lag in the time it takes to display the current facial frame
                        talktime = ltime + ( S32 )( TALK_FUTURE_SEC * ( F32 )s_khz->integer * 1000 );
                        talkofs = talktime % sc->soundLength;
                        talkcnt = 100;
                        if( talkofs + talkcnt < sc->soundLength )
                        {
                            if( sc->soundCompressionMethod == 1 )
                            {
                                S_SetVoiceAmplitudeFromADPCM( sc, talkofs, talkcnt, ch->entnum );
                            }
                            else if( sc->soundCompressionMethod == 2 )
                            {
                                S_SetVoiceAmplitudeFromWavelet( sc, talkofs, talkcnt, ch->entnum );
                            }
                            else if( sc->soundCompressionMethod == 3 )
                            {
                                S_SetVoiceAmplitudeFromMuLaw( sc, talkofs, talkcnt, ch->entnum );
                            }
                            else
                            {
                                S_SetVoiceAmplitudeFrom16( sc, talkofs, talkcnt, ch->entnum );
                            }
                        }
                    }
#endif
                    if( sc->soundCompressionMethod == 1 )
                    {
                        S_PaintChannelFromADPCM( ch, sc, count, sampleOffset, ltime - s_paintedtime );
                    }
                    else if( sc->soundCompressionMethod == 2 )
                    {
                        S_PaintChannelFromWavelet( ch, sc, count, sampleOffset, ltime - s_paintedtime );
                    }
                    else if( sc->soundCompressionMethod == 3 )
                    {
                        S_PaintChannelFromMuLaw( ch, sc, count, sampleOffset, ltime - s_paintedtime );
                    }
                    else
                    {
                        S_PaintChannelFrom16( ch, sc, count, sampleOffset, ltime - s_paintedtime );
                    }
                    ltime += count;
                }
            }
            while( ltime < end );
        }
        
        // transfer out according to DMA format
        S_TransferPaintBuffer( end );
        //bani
        //CL_WriteWaveFilePacket( end );
        s_paintedtime = end;
        firstPass = false;
    }
}