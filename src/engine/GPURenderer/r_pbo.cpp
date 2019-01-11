////////////////////////////////////////////////////////////////////////////////////////
// Copyright(C) 2018 Dusan Jocic <dusanjocic@msn.com>
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
// File name:   r_pbo.cpp
// Version:     v1.01
// Created:
// Compilers:   Visual Studio 2017, gcc 7.3.0
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#include <OWLib/precompiled.h>

/*
==============
idRenderSystemLocal::Init
==============
*/
void idRenderSystemLocal::PBOInit( void )
{
    CL_RefPrintf( PRINT_ALL, "------- idRenderSystemLocal::PBOInit -------\n" );
    
    // Generate the pixel buffer object.
    qglGenBuffers( 1, &pboReadbackHandle );
    qglGenBuffers( 2, &pboWriteHandle[0] );
    
    qglBindBuffer( GL_PIXEL_PACK_BUFFER, pboReadbackHandle );
    qglBufferData( GL_PIXEL_PACK_BUFFER, glConfig.vidWidth * glConfig.vidHeight * 4, 0, GL_STREAM_READ );
    
    buffer = NULL;
    UnbindPBO();
}
/*
==============
idRenderSystemLocal::WriteToPBO
==============
*/
void idRenderSystemLocal::WriteToPBO( S32 pbo, U8* buffer, S32 DestX, S32 DestY, S32 Width, S32 Height )
{
    qglBindBuffer( GL_PIXEL_UNPACK_BUFFER, pboWriteHandle[pbo] );
    //Dushan - maybe in the future hdr???
    //GL_EXT_subtexture
    qglTexSubImage2D( GL_TEXTURE_2D, 0, DestX, DestY, Width, Height, GL_RGBA, GL_UNSIGNED_BYTE, buffer );
}

/*
==============
idRenderSystemLocal::ReadPBO
==============
*/
U8* idRenderSystemLocal::ReadPBO( bool readBack )
{
    if( !readBack )
    {
        qglReadBuffer( GL_COLOR_ATTACHMENT0 );
        qglBindBuffer( GL_PIXEL_PACK_BUFFER, pboReadbackHandle );
        qglReadPixels( 0, 0, glConfig.vidWidth, glConfig.vidHeight, GL_RGBA, GL_UNSIGNED_BYTE, 0 );
        qglBindBuffer( GL_PIXEL_PACK_BUFFER, 0 );
    }
    else
    {
        qglBindBuffer( GL_PIXEL_PACK_BUFFER, pboReadbackHandle );
        qglMapBuffer( GL_PIXEL_PACK_BUFFER, GL_READ_ONLY );
    }
    
    return buffer;
}

/*
==============
idRenderSystemLocal::UnbindPBO
==============
*/
void idRenderSystemLocal::UnbindPBO( void )
{
    if( buffer != NULL )
    {
        qglUnmapBuffer( GL_PIXEL_PACK_BUFFER );
        
    }
    qglBindBuffer( GL_PIXEL_PACK_BUFFER, 0 );
}
