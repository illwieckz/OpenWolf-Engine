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
// File name:   con_log.cpp
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#ifdef DEDICATED
#include <null/null_precompiled.h>
#else
#include <OWLib/precompiled.h>
#endif

#define MAX_LOG 32768

static UTF8          consoleLog[ MAX_LOG ];
static U32  writePos = 0;
static U32  readPos = 0;

/*
==================
CON_LogSize
==================
*/
U32 CON_LogSize( void )
{
    if( readPos <= writePos )
        return writePos - readPos;
    else
        return writePos + MAX_LOG - readPos;
}

/*
==================
CON_LogFree
==================
*/
static U32 CON_LogFree( void )
{
    return MAX_LOG - CON_LogSize( ) - 1;
}

/*
==================
CON_LogWrite
==================
*/
U32 CON_LogWrite( StringEntry in )
{
    U32 length = strlen( in );
    U32 firstChunk;
    U32 secondChunk;
    
    while( CON_LogFree( ) < length && CON_LogSize( ) > 0 )
    {
        // Free enough space
        while( consoleLog[ readPos ] != '\n' && CON_LogSize( ) > 1 )
            readPos = ( readPos + 1 ) % MAX_LOG;
            
        // Skip past the '\n'
        readPos = ( readPos + 1 ) % MAX_LOG;
    }
    
    if( CON_LogFree( ) < length )
        return 0;
        
    if( writePos + length > MAX_LOG )
    {
        firstChunk  = MAX_LOG - writePos;
        secondChunk = length - firstChunk;
    }
    else
    {
        firstChunk  = length;
        secondChunk = 0;
    }
    
    ::memcpy( consoleLog + writePos, in, firstChunk );
    ::memcpy( consoleLog, in + firstChunk, secondChunk );
    
    writePos = ( writePos + length ) % MAX_LOG;
    
    return length;
}

/*
==================
CON_LogRead
==================
*/
U32 CON_LogRead( UTF8* out, U32 outSize )
{
    U32 firstChunk;
    U32 secondChunk;
    
    if( CON_LogSize( ) < outSize )
        outSize = CON_LogSize( );
        
    if( readPos + outSize > MAX_LOG )
    {
        firstChunk  = MAX_LOG - readPos;
        secondChunk = outSize - firstChunk;
    }
    else
    {
        firstChunk  = outSize;
        secondChunk = 0;
    }
    
    ::memcpy( out, consoleLog + readPos, firstChunk );
    ::memcpy( out + firstChunk, out, secondChunk );
    
    readPos = ( readPos + outSize ) % MAX_LOG;
    
    return outSize;
}
