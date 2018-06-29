////////////////////////////////////////////////////////////////////////////////////////
// Copyright(C) 2011 James Canete (use.less01@gmail.com)
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
// File name:   r_extensions.cpp
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description: extensions needed by the renderer not in sdl_glimp.c
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#include <OWLib/precompiled.h>

void GLimp_InitExtraExtensions()
{
    StringEntry extension;
    StringEntry result[3] = {"...ignoring %s\n", "...using %s\n", "...%s not found\n"};
    
    // Check OpenGL version
    sscanf( glConfig.version_string, "%d.%d", &glRefConfig.openglMajorVersion, &glRefConfig.openglMinorVersion );
    if( glRefConfig.openglMajorVersion < 2 ) Com_Error( ERR_FATAL, "OpenGL 2.0 required!" );
    CL_RefPrintf( PRINT_ALL, "...using OpenGL %s\n", glConfig.version_string );
    
    bool q_gl_version_at_least_3_0 = ( glRefConfig.openglMajorVersion >= 3 );
    bool q_gl_version_at_least_3_2 = ( glRefConfig.openglMajorVersion > 3 ||
                                       ( glRefConfig.openglMajorVersion == 3 && glRefConfig.openglMinorVersion > 2 ) );
                                       
    // Check if we need Intel graphics specific fixes.
    glRefConfig.intelGraphics = false;
    if( strstr( ( UTF8* )glGetString( GL_RENDERER ), "Intel" ) ) glRefConfig.intelGraphics = true;
    
    // OpenGL 3.0 - GL_ARB_framebuffer_object
    extension = "GL_ARB_framebuffer_object";
    glRefConfig.framebufferObject = false;
    glRefConfig.framebufferBlit = false;
    glRefConfig.framebufferMultisample = false;
    if( q_gl_version_at_least_3_0 )
    {
        glRefConfig.framebufferObject = !!r_ext_framebuffer_object->integer;
        glRefConfig.framebufferBlit = true;
        glRefConfig.framebufferMultisample = true;
        
        glGetIntegerv( GL_MAX_RENDERBUFFER_SIZE, &glRefConfig.maxRenderbufferSize );
        glGetIntegerv( GL_MAX_COLOR_ATTACHMENTS, &glRefConfig.maxColorAttachments );
        
        CL_RefPrintf( PRINT_ALL, result[glRefConfig.framebufferObject], extension );
    }
    else
    {
        CL_RefPrintf( PRINT_ALL, result[2], extension );
    }
    
    // OpenGL 3.0 - GL_ARB_vertex_array_object
    extension = "GL_ARB_vertex_array_object";
    glRefConfig.vertexArrayObject = false;
    if( q_gl_version_at_least_3_0 )
    {
        if( q_gl_version_at_least_3_0 )
        {
            // force VAO, core context requires it
            glRefConfig.vertexArrayObject = true;
        }
        else
        {
            glRefConfig.vertexArrayObject = !!r_arb_vertex_array_object->integer;
        }
        
        CL_RefPrintf( PRINT_ALL, result[glRefConfig.vertexArrayObject], extension );
    }
    else
    {
        CL_RefPrintf( PRINT_ALL, result[2], extension );
    }
    
    // OpenGL 3.0 - GL_ARB_texture_float
    extension = "GL_ARB_texture_float";
    glRefConfig.textureFloat = false;
    if( q_gl_version_at_least_3_0 )
    {
        glRefConfig.textureFloat = !!r_ext_texture_float->integer;
        
        CL_RefPrintf( PRINT_ALL, result[glRefConfig.textureFloat], extension );
    }
    else
    {
        CL_RefPrintf( PRINT_ALL, result[2], extension );
    }
    
    // OpenGL 3.2 - GL_ARB_depth_clamp
    extension = "GL_ARB_depth_clamp";
    glRefConfig.depthClamp = false;
    if( q_gl_version_at_least_3_2 )
    {
        glRefConfig.depthClamp = true;
        
        CL_RefPrintf( PRINT_ALL, result[glRefConfig.depthClamp], extension );
    }
    else
    {
        CL_RefPrintf( PRINT_ALL, result[2], extension );
    }
    
    // OpenGL 3.2 - GL_ARB_seamless_cube_map
    extension = "GL_ARB_seamless_cube_map";
    glRefConfig.seamlessCubeMap = false;
    if( q_gl_version_at_least_3_2 )
    {
        glRefConfig.seamlessCubeMap = !!r_arb_seamless_cube_map->integer;
        
        CL_RefPrintf( PRINT_ALL, result[glRefConfig.seamlessCubeMap], extension );
    }
    else
    {
        CL_RefPrintf( PRINT_ALL, result[2], extension );
    }
    
    // Determine GLSL version
    if( 1 )
    {
        UTF8 version[256];
        
        Q_strncpyz( version, ( UTF8* )glGetString( GL_SHADING_LANGUAGE_VERSION ), sizeof( version ) );
        
        sscanf( version, "%d.%d", &glRefConfig.glslMajorVersion, &glRefConfig.glslMinorVersion );
        
        CL_RefPrintf( PRINT_ALL, "...using GLSL version %s\n", version );
    }
}
