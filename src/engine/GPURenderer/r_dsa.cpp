////////////////////////////////////////////////////////////////////////////////////////
// Copyright(C) 2016 James Canete
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
// File name:   r_dsa.cpp
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#include <OWLIb/precompiled.h>

static struct
{
    U32 textures[NUM_TEXTURE_BUNDLES];
    U32 texunit;
    
    U32 program;
    
    U32 drawFramebuffer;
    U32 readFramebuffer;
    U32 renderbuffer;
}
glDsaState;

void GL_BindNullTextures()
{
    S32 i;
    
    if( glRefConfig.directStateAccess )
    {
        for( i = 0; i < NUM_TEXTURE_BUNDLES; i++ )
        {
            glBindMultiTextureEXT( GL_TEXTURE0 + i, GL_TEXTURE_2D, 0 );
            glDsaState.textures[i] = 0;
        }
    }
    else
    {
        for( i = 0; i < NUM_TEXTURE_BUNDLES; i++ )
        {
            glActiveTexture( GL_TEXTURE0 + i );
            glBindTexture( GL_TEXTURE_2D, 0 );
            glDsaState.textures[i] = 0;
        }
        
        glActiveTexture( GL_TEXTURE0 );
        glDsaState.texunit = GL_TEXTURE0;
    }
}

S32 GL_BindMultiTexture( U32 texunit, U32 target, U32 texture )
{
    U32 tmu = texunit - GL_TEXTURE0;
    
    if( glDsaState.textures[tmu] == texture )
        return 0;
        
    if( target >= GL_TEXTURE_CUBE_MAP_POSITIVE_X && target <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z )
        target = GL_TEXTURE_CUBE_MAP;
        
    glBindMultiTextureEXT( texunit, target, texture );
    glDsaState.textures[tmu] = texture;
    return 1;
}

void GLDSA_BindMultiTextureEXT( U32 texunit, U32 target, U32 texture )
{
    if( glDsaState.texunit != texunit )
    {
        glActiveTexture( texunit );
        glDsaState.texunit = texunit;
    }
    
    glBindTexture( target, texture );
}

void GLDSA_TextureParameterfEXT( U32 texture, U32 target, U32 pname, F32 param )
{
    GL_BindMultiTexture( glDsaState.texunit, target, texture );
    glTexParameterf( target, pname, param );
}

void GLDSA_TextureParameteriEXT( U32 texture, U32 target, U32 pname, S32 param )
{
    GL_BindMultiTexture( glDsaState.texunit, target, texture );
    glTexParameteri( target, pname, param );
}

void GLDSA_TextureImage2DEXT( U32 texture, U32 target, S32 level, S32 internalformat, S32 width, S32 height, S32 border, U32 format, U32 type, const void* pixels )
{
    GL_BindMultiTexture( glDsaState.texunit, target, texture );
    glTexImage2D( target, level, internalformat, width, height, border, format, type, pixels );
}

void GLDSA_TextureSubImage2DEXT( U32 texture, U32 target, S32 level, S32 xoffset, S32 yoffset, S32 width, S32 height, U32 format, U32 type, const void* pixels )
{
    GL_BindMultiTexture( glDsaState.texunit, target, texture );
    glTexSubImage2D( target, level, xoffset, yoffset, width, height, format, type, pixels );
}

void GLDSA_CopyTextureSubImage2DEXT( U32 texture, U32 target, S32 level, S32 xoffset, S32 yoffset, S32 x, S32 y, S32 width, S32 height )
{
    GL_BindMultiTexture( glDsaState.texunit, target, texture );
    glCopyTexSubImage2D( target, level, xoffset, yoffset, x, y, width, height );
}

void  GLDSA_CompressedTextureImage2DEXT( U32 texture, U32 target, S32 level, U32 internalformat, S32 width, S32 height, S32 border, S32 imageSize, const void* data )
{
    GL_BindMultiTexture( glDsaState.texunit, target, texture );
    glCompressedTexImage2D( target, level, internalformat, width, height, border, imageSize, data );
}

void GLDSA_CompressedTextureSubImage2DEXT( U32 texture, U32 target, S32 level, S32 xoffset, S32 yoffset, S32 width, S32 height, U32 format, S32 imageSize, const void* data )
{
    GL_BindMultiTexture( glDsaState.texunit, target, texture );
    glCompressedTexSubImage2D( target, level, xoffset, yoffset, width, height, format, imageSize, data );
}

void GLDSA_GenerateTextureMipmapEXT( U32 texture, U32 target )
{
    GL_BindMultiTexture( glDsaState.texunit, target, texture );
    glGenerateMipmap( target );
}

void GL_BindNullProgram()
{
    glUseProgram( 0 );
    glDsaState.program = 0;
}

S32 GL_UseProgram( U32 program )
{
    if( glDsaState.program == program )
        return 0;
        
    glUseProgram( program );
    glDsaState.program = program;
    return 1;
}

void GLDSA_ProgramUniform1iEXT( U32 program, S32 location, S32 v0 )
{
    GL_UseProgram( program );
    glUniform1i( location, v0 );
}

void GLDSA_ProgramUniform1fEXT( U32 program, S32 location, F32 v0 )
{
    GL_UseProgram( program );
    glUniform1f( location, v0 );
}

void GLDSA_ProgramUniform2fEXT( U32 program, S32 location,
                                F32 v0, F32 v1 )
{
    GL_UseProgram( program );
    glUniform2f( location, v0, v1 );
}

void GLDSA_ProgramUniform3fEXT( U32 program, S32 location,
                                F32 v0, F32 v1, F32 v2 )
{
    GL_UseProgram( program );
    glUniform3f( location, v0, v1, v2 );
}

void GLDSA_ProgramUniform4fEXT( U32 program, S32 location,
                                F32 v0, F32 v1, F32 v2, F32 v3 )
{
    GL_UseProgram( program );
    glUniform4f( location, v0, v1, v2, v3 );
}

void GLDSA_ProgramUniform1fvEXT( U32 program, S32 location, S32 count, const F32* value )
{
    GL_UseProgram( program );
    glUniform1fv( location, count, value );
}

void GLDSA_ProgramUniformMatrix4fvEXT( U32 program, S32 location, S32 count, GLboolean transpose, const F32* value )
{
    GL_UseProgram( program );
    glUniformMatrix4fv( location, count, transpose, value );
}

void GL_BindNullFramebuffers()
{
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    glDsaState.drawFramebuffer = glDsaState.readFramebuffer = 0;
    glBindRenderbuffer( GL_RENDERBUFFER, 0 );
    glDsaState.renderbuffer = 0;
}

void GL_BindFramebuffer( U32 target, U32 framebuffer )
{
    switch( target )
    {
        case GL_FRAMEBUFFER:
            if( framebuffer != glDsaState.drawFramebuffer || framebuffer != glDsaState.readFramebuffer )
            {
                glBindFramebuffer( target, framebuffer );
                glDsaState.drawFramebuffer = glDsaState.readFramebuffer = framebuffer;
            }
            break;
            
        case GL_DRAW_FRAMEBUFFER:
            if( framebuffer != glDsaState.drawFramebuffer )
            {
                glBindFramebuffer( target, framebuffer );
                glDsaState.drawFramebuffer = framebuffer;
            }
            break;
            
        case GL_READ_FRAMEBUFFER:
            if( framebuffer != glDsaState.readFramebuffer )
            {
                glBindFramebuffer( target, framebuffer );
                glDsaState.readFramebuffer = framebuffer;
            }
            break;
    }
}

void GL_BindRenderbuffer( U32 renderbuffer )
{
    if( renderbuffer != glDsaState.renderbuffer )
    {
        glBindRenderbuffer( GL_RENDERBUFFER, renderbuffer );
        glDsaState.renderbuffer = renderbuffer;
    }
}

void GLDSA_NamedRenderbufferStorageEXT( U32 renderbuffer, U32 internalformat, S32 width, S32 height )
{
    GL_BindRenderbuffer( renderbuffer );
    glRenderbufferStorage( GL_RENDERBUFFER, internalformat, width, height );
}

void GLDSA_NamedRenderbufferStorageMultisampleEXT( U32 renderbuffer, S32 samples, U32 internalformat, S32 width, S32 height )
{
    GL_BindRenderbuffer( renderbuffer );
    glRenderbufferStorageMultisample( GL_RENDERBUFFER, samples, internalformat, width, height );
}

U32 GLDSA_CheckNamedFramebufferStatusEXT( U32 framebuffer, U32 target )
{
    GL_BindFramebuffer( target, framebuffer );
    return glCheckFramebufferStatus( target );
}

void GLDSA_NamedFramebufferTexture2DEXT( U32 framebuffer, U32 attachment, U32 textarget, U32 texture, S32 level )
{
    GL_BindFramebuffer( GL_FRAMEBUFFER, framebuffer );
    glFramebufferTexture2D( GL_FRAMEBUFFER, attachment, textarget, texture, level );
}

void GLDSA_NamedFramebufferRenderbufferEXT( U32 framebuffer, U32 attachment, U32 renderbuffertarget, U32 renderbuffer )
{
    GL_BindFramebuffer( GL_FRAMEBUFFER, framebuffer );
    glFramebufferRenderbuffer( GL_FRAMEBUFFER, attachment, renderbuffertarget, renderbuffer );
}
