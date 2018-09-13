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
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#include <OWLib/precompiled.h>

//For some future usage :)

/*
==============
idRenderSystemLocal::Init
==============
*/
void idRenderSystemLocal::PBOInit( void )
{
    CL_RefPrintf( PRINT_ALL, "------- idRenderSystemLocal::PBOInit -------\n" );
    
    // query support for pixel buffer objects "GL_ARB_pixel_buffer_object"
    
    // Generate the pixel buffer object.
    glGenBuffersARB( 1, &pboReadbackHandle );
    glGenBuffersARB( 2, &pboWriteHandle[0] );
    
    glBindBufferARB( GL_PIXEL_PACK_BUFFER_ARB, pboReadbackHandle );
    glBufferDataARB( GL_PIXEL_PACK_BUFFER_ARB, glConfig.vidWidth * glConfig.vidHeight * 4, 0, GL_STREAM_READ_ARB );
    
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
    glBindBufferARB( GL_PIXEL_UNPACK_BUFFER_ARB, pboWriteHandle[pbo] );
    //Dushan
    //GL_EXT_subtexture
    glTexSubImage2D( GL_TEXTURE_2D, 0, DestX, DestY, Width, Height, GL_RGBA, GL_UNSIGNED_BYTE, buffer );
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
        glReadBuffer( GL_COLOR_ATTACHMENT0_EXT );
        
        glBindBufferARB( GL_PIXEL_PACK_BUFFER_ARB, pboReadbackHandle );
        
        glReadPixels( 0, 0, glConfig.vidWidth, glConfig.vidHeight, GL_RGBA, GL_UNSIGNED_BYTE, 0 );
        glBindBufferARB( GL_PIXEL_PACK_BUFFER_ARB, 0 );
    }
    else
    {
        glBindBufferARB( GL_PIXEL_PACK_BUFFER_ARB, pboReadbackHandle );
        buffer = ( U8* )glMapBufferARB( GL_PIXEL_PACK_BUFFER_ARB, GL_READ_ONLY_ARB );
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
        glUnmapBufferARB( GL_PIXEL_PACK_BUFFER_ARB );
        
    }
    glBindBufferARB( GL_PIXEL_PACK_BUFFER_ARB, 0 );
}