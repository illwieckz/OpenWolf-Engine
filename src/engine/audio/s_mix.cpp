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
// File name:   s_mix.cpp
// Version:     v1.01
// Created:
// Compilers:   Visual Studio 2017, gcc 7.3.0
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#include <OWLib/precompiled.h>

static portable_samplepair_t paintbuffer[PAINTBUFFER_SIZE];
static S32 snd_vol;

S32* snd_p;
S32 snd_linear_count;
S16* snd_out;

void S_WriteLinearBlastStereo16( void )
{
    S32 i, val;
    
    for( i = 0; i < snd_linear_count; i += 2 )
    {
        val = snd_p[i] >> 8;
        if( val > 0x7fff )
            snd_out[i] = 0x7fff;
        else if( val < -32768 )
            snd_out[i] = -32768;
        else
            snd_out[i] = val;
            
        val = snd_p[i + 1] >> 8;
        if( val > 0x7fff )
            snd_out[i + 1] = 0x7fff;
        else if( val < -32768 )
            snd_out[i + 1] = -32768;
        else
            snd_out[i + 1] = val;
    }
}

void S_TransferStereo16( U64* pbuf, S32 endtime )
{
    S32 lpos, ls_paintedtime;
    
    snd_p = ( int* )paintbuffer;
    ls_paintedtime = s_paintedtime;
    
    while( ls_paintedtime < endtime )
    {
        // handle recirculating buffer issues
        lpos = ls_paintedtime & ( ( dma.samples >> 1 ) - 1 );
        
        snd_out = ( short* )pbuf + ( lpos << 1 );
        
        snd_linear_count = ( dma.samples >> 1 ) - lpos;
        if( ls_paintedtime + snd_linear_count > endtime )
            snd_linear_count = endtime - ls_paintedtime;
            
        snd_linear_count <<= 1;
        
        // write a linear blast of samples
        S_WriteLinearBlastStereo16();
        
        snd_p += snd_linear_count;
        ls_paintedtime += ( snd_linear_count >> 1 );
        
        if( CL_VideoRecording() )
            CL_WriteAVIAudioFrame( ( U8* ) snd_out, snd_linear_count << 1 );
    }
}

/*
===================
S_TransferPaintBuffer

===================
*/
void S_TransferPaintBuffer( S32 endtime )
{
    S32 out_idx, count, out_mask;
    S32* p;
    S32 step, val;
    U64* pbuf;
    
    pbuf = ( U64* )dma.buffer;
    
    if( s_testsound->integer )
    {
        S32 i, count;
        
        // write a fixed sine wave
        count = ( endtime - s_paintedtime );
        for( i = 0; i < count; i++ )
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
        p = ( S32* )paintbuffer;
        count = ( endtime - s_paintedtime ) * dma.channels;
        out_mask = dma.samples - 1;
        out_idx = s_paintedtime * dma.channels & out_mask;
        step = 3 - dma.channels;
        
        if( dma.samplebits == 16 )
        {
            S16* out = ( S16* )pbuf;
            
            while( count-- )
            {
                val = *p >> 8;
                p += step;
                if( val > 0x7fff )
                    val = 0x7fff;
                else if( val < -32768 )
                    val = -32768;
                out[out_idx] = val;
                out_idx = ( out_idx + 1 ) & out_mask;
            }
        }
        else if( dma.samplebits == 8 )
        {
            U8*  out = ( U8* )pbuf;
            
            while( count-- )
            {
                val = *p >> 8;
                p += step;
                if( val > 0x7fff )
                    val = 0x7fff;
                else if( val < -32768 )
                    val = -32768;
                out[out_idx] = ( val >> 8 ) + 128;
                out_idx = ( out_idx + 1 ) & out_mask;
            }
        }
    }
}


/*
===============================================================================

CHANNEL MIXING

===============================================================================
*/

static void S_PaintChannelFrom16_scalar( channel_t* ch, const sfx_t* sc, S32 count, S32 sampleOffset, S32 bufferOffset )
{
    S32 data, aoff, boff, leftvol, rightvol, i, j;
    portable_samplepair_t* samp;
    sndBuffer* chunk;
    S16* samples;
    F32 ooff, fdata, fdiv, fleftvol, frightvol;
    
    samp = &paintbuffer[bufferOffset];
    
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
    
    if( !ch->doppler || ch->dopplerScale == 1.0f )
    {
        leftvol = ch->leftvol * snd_vol;
        rightvol = ch->rightvol * snd_vol;
        samples = chunk->sndChunk;
        for( i = 0; i < count; i++ )
        {
            data = samples[sampleOffset++];
            samp[i].left += ( data * leftvol ) >> 8;
            samp[i].right += ( data * rightvol ) >> 8;
            
            if( sampleOffset == SND_CHUNK_SIZE )
            {
                chunk = chunk->next;
                samples = chunk->sndChunk;
                sampleOffset = 0;
            }
        }
    }
    else
    {
        fleftvol = ch->leftvol * snd_vol;
        frightvol = ch->rightvol * snd_vol;
        
        ooff = sampleOffset;
        samples = chunk->sndChunk;
        
        for( i = 0; i < count; i++ )
        {
            aoff = ooff;
            ooff = ooff + ch->dopplerScale;
            boff = ooff;
            fdata = 0;
            for( j = aoff; j < boff; j++ )
            {
                if( j == SND_CHUNK_SIZE )
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

static void S_PaintChannelFrom16( channel_t* ch, const sfx_t* sc, S32 count, S32 sampleOffset, S32 bufferOffset )
{
    S_PaintChannelFrom16_scalar( ch, sc, count, sampleOffset, bufferOffset );
}

/*
===================
S_PaintChannels
===================
*/
void S_PaintChannels( S32 endtime )
{
    S32 i, end, stream, ltime, count, sampleOffset;
    channel_t*      ch;
    sfx_t*          sc;
    
    if( s_muted->integer )
        snd_vol = 0;
    else
        snd_vol = s_volume->value * 255;
        
    while( s_paintedtime < endtime )
    {
        // if paintbuffer is smaller than DMA buffer
        // we may need to fill it multiple times
        end = endtime;
        if( endtime - s_paintedtime > PAINTBUFFER_SIZE )
        {
            end = s_paintedtime + PAINTBUFFER_SIZE;
        }
        
        // clear the paint buffer and mix any raw samples...
        ::memset( paintbuffer, 0, sizeof( paintbuffer ) );
        for( stream = 0; stream < MAX_RAW_STREAMS; stream++ )
        {
            if( s_rawend[stream] >= s_paintedtime )
            {
                // copy from the streaming sound source
                const portable_samplepair_t* rawsamples = s_rawsamples[stream];
                const int       stop = ( end < s_rawend[stream] ) ? end : s_rawend[stream];
                
                for( i = s_paintedtime; i < stop; i++ )
                {
                    const int       s = i & ( MAX_RAW_SAMPLES - 1 );
                    
                    paintbuffer[i - s_paintedtime].left += rawsamples[s].left;
                    paintbuffer[i - s_paintedtime].right += rawsamples[s].right;
                }
            }
        }
        
        // paint in the channels.
        ch = s_channels;
        for( i = 0; i < MAX_CHANNELS; i++, ch++ )
        {
            if( !ch->thesfx || ( ch->leftvol < 0.25 && ch->rightvol < 0.25 ) )
            {
                continue;
            }
            
            ltime = s_paintedtime;
            sc = ch->thesfx;
            
            sampleOffset = ltime - ch->startSample;
            count = end - ltime;
            if( sampleOffset + count > sc->soundLength )
            {
                count = sc->soundLength - sampleOffset;
            }
            
            if( count > 0 )
            {
                S_PaintChannelFrom16( ch, sc, count, sampleOffset, ltime - s_paintedtime );
            }
        }
        
        // paint in the looped channels.
        ch = loop_channels;
        for( i = 0; i < numLoopChannels; i++, ch++ )
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
                sampleOffset = ( ltime % sc->soundLength );
                
                count = end - ltime;
                if( sampleOffset + count > sc->soundLength )
                {
                    count = sc->soundLength - sampleOffset;
                }
                
                if( count > 0 )
                {
                    S_PaintChannelFrom16( ch, sc, count, sampleOffset, ltime - s_paintedtime );
                    ltime += count;
                }
            }
            while( ltime < end );
        }
        
        // transfer out according to DMA format
        S_TransferPaintBuffer( end );
        s_paintedtime = end;
    }
}
