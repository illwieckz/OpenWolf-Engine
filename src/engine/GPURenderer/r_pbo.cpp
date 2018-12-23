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

//For some future usage :)
#if 0

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
    qglGenBuffersARB( 1, &pboReadbackHandle );
    qglGenBuffersARB( 2, &pboWriteHandle[0] );
    
    qglBindBufferARB( GL_PIXEL_PACK_BUFFER_ARB, pboReadbackHandle );
    qglBufferDataARB( GL_PIXEL_PACK_BUFFER_ARB, glConfig.vidWidth * glConfig.vidHeight * 4, 0, GL_STREAM_READ_ARB );
    
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
    qglBindBufferARB( GL_PIXEL_UNPACK_BUFFER_ARB, pboWriteHandle[pbo] );
    //Dushan
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
        qglReadBuffer( GL_COLOR_ATTACHMENT0_EXT );
        
        qglBindBufferARB( GL_PIXEL_PACK_BUFFER_ARB, pboReadbackHandle );
        
        qglReadPixels( 0, 0, glConfig.vidWidth, glConfig.vidHeight, GL_RGBA, GL_UNSIGNED_BYTE, 0 );
        qglBindBufferARB( GL_PIXEL_PACK_BUFFER_ARB, 0 );
    }
    else
    {
        qglBindBufferARB( GL_PIXEL_PACK_BUFFER_ARB, pboReadbackHandle );
        buffer = ( U8* )qglMapBufferARB( GL_PIXEL_PACK_BUFFER_ARB, GL_READ_ONLY_ARB );
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
        qglUnmapBufferARB( GL_PIXEL_PACK_BUFFER_ARB );
        
    }
    qglBindBufferARB( GL_PIXEL_PACK_BUFFER_ARB, 0 );
}
#endif
