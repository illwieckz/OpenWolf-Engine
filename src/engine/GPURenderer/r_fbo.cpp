////////////////////////////////////////////////////////////////////////////////////////
// Copyright(C) 2006 Kirk Barnes
// Copyright(C) 2006 - 2008 Robert Beckebans <trebor_7@users.sourceforge.net>
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
// File name:   r_fbo.cpp
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#include <OWLib/precompiled.h>

/*
=============
R_CheckFBO
=============
*/
bool R_CheckFBO( const FBO_t* fbo )
{
    GLenum code = glCheckNamedFramebufferStatusEXT( fbo->frameBuffer, GL_FRAMEBUFFER_EXT );
    
    if( code == GL_FRAMEBUFFER_COMPLETE_EXT )
        return true;
        
    // an error occured
    switch( code )
    {
        case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
            CL_RefPrintf( PRINT_WARNING, "R_CheckFBO: (%s) Unsupported framebuffer format\n", fbo->name );
            break;
            
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
            CL_RefPrintf( PRINT_WARNING, "R_CheckFBO: (%s) Framebuffer incomplete attachment\n", fbo->name );
            break;
            
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
            CL_RefPrintf( PRINT_WARNING, "R_CheckFBO: (%s) Framebuffer incomplete, missing attachment\n", fbo->name );
            break;
            
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
            CL_RefPrintf( PRINT_WARNING, "R_CheckFBO: (%s) Framebuffer incomplete, missing draw buffer\n", fbo->name );
            break;
            
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
            CL_RefPrintf( PRINT_WARNING, "R_CheckFBO: (%s) Framebuffer incomplete, missing read buffer\n", fbo->name );
            break;
            
        default:
            CL_RefPrintf( PRINT_WARNING, "R_CheckFBO: (%s) unknown error 0x%X\n", fbo->name, code );
            break;
    }
    
    return false;
}

/*
============
FBO_Create
============
*/
FBO_t* FBO_Create( StringEntry name, S32 width, S32 height )
{
    FBO_t*          fbo;
    
    if( strlen( name ) >= MAX_QPATH )
    {
        Com_Error( ERR_DROP, "FBO_Create: \"%s\" is too long", name );
    }
    
    if( width <= 0 || width > glRefConfig.maxRenderbufferSize )
    {
        Com_Error( ERR_DROP, "FBO_Create: bad width %i", width );
    }
    
    if( height <= 0 || height > glRefConfig.maxRenderbufferSize )
    {
        Com_Error( ERR_DROP, "FBO_Create: bad height %i", height );
    }
    
    if( tr.numFBOs == MAX_FBOS )
    {
        Com_Error( ERR_DROP, "FBO_Create: MAX_FBOS hit" );
    }
    
    fbo = tr.fbos[tr.numFBOs] = ( FBO_t* )Hunk_Alloc( sizeof( *fbo ), h_low );
    Q_strncpyz( fbo->name, name, sizeof( fbo->name ) );
    fbo->index = tr.numFBOs++;
    fbo->width = width;
    fbo->height = height;
    
    glGenFramebuffersEXT( 1, &fbo->frameBuffer );
    
    return fbo;
}

/*
=================
FBO_CreateBuffer
=================
*/
void FBO_CreateBuffer( FBO_t* fbo, S32 format, S32 index, S32 multisample )
{
    U32* pRenderBuffer;
    GLenum attachment;
    bool absent;
    
    switch( format )
    {
        case GL_RGB:
        case GL_RGBA:
        case GL_RGB8:
        case GL_RGBA8:
        case GL_RGB16F_ARB:
        case GL_RGBA16F_ARB:
        case GL_RGB32F_ARB:
        case GL_RGBA32F_ARB:
            fbo->colorFormat = format;
            pRenderBuffer = &fbo->colorBuffers[index];
            attachment = GL_COLOR_ATTACHMENT0_EXT + index;
            break;
            
        case GL_DEPTH_COMPONENT:
        case GL_DEPTH_COMPONENT16_ARB:
        case GL_DEPTH_COMPONENT24_ARB:
        case GL_DEPTH_COMPONENT32_ARB:
            fbo->depthFormat = format;
            pRenderBuffer = &fbo->depthBuffer;
            attachment = GL_DEPTH_ATTACHMENT_EXT;
            break;
            
        case GL_STENCIL_INDEX:
        case GL_STENCIL_INDEX1_EXT:
        case GL_STENCIL_INDEX4_EXT:
        case GL_STENCIL_INDEX8_EXT:
        case GL_STENCIL_INDEX16_EXT:
            fbo->stencilFormat = format;
            pRenderBuffer = &fbo->stencilBuffer;
            attachment = GL_STENCIL_ATTACHMENT_EXT;
            break;
            
        case GL_DEPTH_STENCIL_EXT:
        case GL_DEPTH24_STENCIL8_EXT:
            fbo->packedDepthStencilFormat = format;
            pRenderBuffer = &fbo->packedDepthStencilBuffer;
            attachment = 0; // special for stencil and depth
            break;
            
        default:
            CL_RefPrintf( PRINT_WARNING, "FBO_CreateBuffer: invalid format %d\n", format );
            return;
    }
    
    absent = *pRenderBuffer == 0;
    if( absent )
        glGenRenderbuffersEXT( 1, pRenderBuffer );
        
    if( glRefConfig.framebufferMultisample )
        glNamedRenderbufferStorageMultisampleEXT( *pRenderBuffer, multisample, format, fbo->width, fbo->height );
    else
        glNamedRenderbufferStorageEXT( *pRenderBuffer, format, fbo->width, fbo->height );
        
    if( absent )
    {
        if( attachment == 0 )
        {
            glNamedFramebufferRenderbufferEXT( fbo->frameBuffer, GL_DEPTH_ATTACHMENT_EXT,   GL_RENDERBUFFER_EXT, *pRenderBuffer );
            glNamedFramebufferRenderbufferEXT( fbo->frameBuffer, GL_STENCIL_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, *pRenderBuffer );
        }
        else
        {
            glNamedFramebufferRenderbufferEXT( fbo->frameBuffer, attachment, GL_RENDERBUFFER_EXT, *pRenderBuffer );
        }
    }
}


/*
=================
FBO_AttachImage
=================
*/
void FBO_AttachImage( FBO_t* fbo, image_t* image, GLenum attachment, GLuint cubemapside )
{
    GLenum target = GL_TEXTURE_2D;
    S32 index;
    
    if( image->flags & IMGFLAG_CUBEMAP )
        target = GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB + cubemapside;
        
    glNamedFramebufferTexture2DEXT( fbo->frameBuffer, attachment, target, image->texnum, 0 );
    index = attachment - GL_COLOR_ATTACHMENT0_EXT;
    if( index >= 0 && index <= 15 )
        fbo->colorImage[index] = image;
}

/*
============
FBO_Bind
============
*/
void FBO_Bind( FBO_t* fbo )
{
    if( !glRefConfig.framebufferObject )
    {
        CL_RefPrintf( PRINT_WARNING, "FBO_Bind() called without framebuffers enabled!\n" );
        return;
    }
    
    if( glState.currentFBO == fbo )
        return;
        
    if( r_logFile->integer )
    {
        // don't just call LogComment, or we will get a call to va() every frame!
        GLimp_LogComment( ( UTF8* )va( "--- FBO_Bind( %s ) ---\n", fbo ? fbo->name : "NULL" ) );
    }
    
    GL_BindFramebuffer( GL_FRAMEBUFFER_EXT, fbo ? fbo->frameBuffer : 0 );
    glState.currentFBO = fbo;
}

/*
============
idRenderSystemLocal::FBOInit
============
*/
void idRenderSystemLocal::FBOInit( void )
{
    S32 i, hdrFormat;
    
    CL_RefPrintf( PRINT_ALL, "------- idRenderSystemLocal::FBOInit -------\n" );
    
    if( !glRefConfig.framebufferObject )
        return;
        
    tr.numFBOs = 0;
    
    GL_CheckErrors();
    
    R_IssuePendingRenderCommands();
    
    hdrFormat = GL_RGBA8;
    if( r_truehdr->integer && glRefConfig.framebufferObject && glRefConfig.textureFloat )
        hdrFormat = GL_RGBA16F_ARB;
        
        
    //
    // Generic FBO...
    //
    {
        tr.genericFbo = FBO_Create( "_generic", tr.genericFBOImage->width, tr.genericFBOImage->height );
        FBO_Bind( tr.genericFbo );
        FBO_AttachImage( tr.genericFbo, tr.genericFBOImage, GL_COLOR_ATTACHMENT0_EXT, 0 );
        R_CheckFBO( tr.genericFbo );
    }
    
    //
    // Bloom VBO's...
    //
    {
        tr.bloomRenderFBO[0] = FBO_Create( "_bloom0", tr.bloomRenderFBOImage[0]->width, tr.bloomRenderFBOImage[0]->height );
        FBO_Bind( tr.bloomRenderFBO[0] );
        FBO_AttachImage( tr.bloomRenderFBO[0], tr.bloomRenderFBOImage[0], GL_COLOR_ATTACHMENT0_EXT, 0 );
        R_CheckFBO( tr.bloomRenderFBO[0] );
        
        
        tr.bloomRenderFBO[1] = FBO_Create( "_bloom1", tr.bloomRenderFBOImage[1]->width, tr.bloomRenderFBOImage[1]->height );
        FBO_Bind( tr.bloomRenderFBO[1] );
        FBO_AttachImage( tr.bloomRenderFBO[1], tr.bloomRenderFBOImage[1], GL_COLOR_ATTACHMENT0_EXT, 0 );
        R_CheckFBO( tr.bloomRenderFBO[1] );
        
        
        tr.bloomRenderFBO[2] = FBO_Create( "_bloom2", tr.bloomRenderFBOImage[2]->width, tr.bloomRenderFBOImage[2]->height );
        FBO_Bind( tr.bloomRenderFBO[2] );
        FBO_AttachImage( tr.bloomRenderFBO[2], tr.bloomRenderFBOImage[2], GL_COLOR_ATTACHMENT0_EXT, 0 );
        R_CheckFBO( tr.bloomRenderFBO[2] );
    }
    
    //
    // Anamorphic VBO's...
    //
    {
        tr.anamorphicRenderFBO[0] = FBO_Create( "_anamorphic0", tr.anamorphicRenderFBOImage[0]->width, tr.anamorphicRenderFBOImage[0]->height );
        FBO_AttachImage( tr.anamorphicRenderFBO[0], tr.anamorphicRenderFBOImage[0], GL_COLOR_ATTACHMENT0_EXT, 0 );
        R_CheckFBO( tr.anamorphicRenderFBO[0] );
        
        tr.anamorphicRenderFBO[1] = FBO_Create( "_anamorphic1", tr.anamorphicRenderFBOImage[1]->width, tr.anamorphicRenderFBOImage[1]->height );
        FBO_AttachImage( tr.anamorphicRenderFBO[1], tr.anamorphicRenderFBOImage[1], GL_COLOR_ATTACHMENT0_EXT, 0 );
        R_CheckFBO( tr.anamorphicRenderFBO[1] );
        
        tr.anamorphicRenderFBO[2] = FBO_Create( "_anamorphic2", tr.anamorphicRenderFBOImage[2]->width, tr.anamorphicRenderFBOImage[2]->height );
        FBO_AttachImage( tr.anamorphicRenderFBO[2], tr.anamorphicRenderFBOImage[2], GL_COLOR_ATTACHMENT0_EXT, 0 );
        R_CheckFBO( tr.anamorphicRenderFBO[2] );
    }
    
    // only create a render FBO if we need to resolve MSAA or do HDR
    // otherwise just render straight to the screen (tr.renderFbo = NULL)
    {
        tr.renderFbo = FBO_Create( "_render", tr.renderDepthImage->width, tr.renderDepthImage->height );
        FBO_AttachImage( tr.renderFbo, tr.renderImage, GL_COLOR_ATTACHMENT0_EXT, 0 );
        FBO_AttachImage( tr.renderFbo, tr.renderDepthImage, GL_DEPTH_ATTACHMENT_EXT, 0 );
        R_CheckFBO( tr.renderFbo );
    }
    
    // clear render buffer
    // this fixes the corrupt screen bug with r_hdr 1 on older hardware
    if( tr.renderFbo )
    {
        GL_BindFramebuffer( GL_FRAMEBUFFER_EXT, tr.renderFbo->frameBuffer );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    }
    
    if( tr.screenScratchImage )
    {
        tr.screenScratchFbo = FBO_Create( "screenScratch", tr.screenScratchImage->width, tr.screenScratchImage->height );
        FBO_AttachImage( tr.screenScratchFbo, tr.screenScratchImage, GL_COLOR_ATTACHMENT0_EXT, 0 );
        FBO_AttachImage( tr.screenScratchFbo, tr.renderDepthImage, GL_DEPTH_ATTACHMENT_EXT, 0 );
        R_CheckFBO( tr.screenScratchFbo );
    }
    
    if( tr.sunRaysImage )
    {
        tr.sunRaysFbo = FBO_Create( "_sunRays", tr.renderDepthImage->width, tr.renderDepthImage->height );
        FBO_AttachImage( tr.sunRaysFbo, tr.sunRaysImage, GL_COLOR_ATTACHMENT0_EXT, 0 );
        FBO_AttachImage( tr.sunRaysFbo, tr.renderDepthImage, GL_DEPTH_ATTACHMENT_EXT, 0 );
        R_CheckFBO( tr.sunRaysFbo );
    }
    
    if( MAX_DRAWN_PSHADOWS && tr.pshadowMaps[0] )
    {
        for( i = 0; i < MAX_DRAWN_PSHADOWS; i++ )
        {
            tr.pshadowFbos[i] = FBO_Create( va( "_shadowmap%d", i ), tr.pshadowMaps[i]->width, tr.pshadowMaps[i]->height );
            // FIXME: this next line wastes 16mb with 16x512x512 sun shadow maps, skip if OpenGL 4.3+ or ARB_framebuffer_no_attachments
            FBO_CreateBuffer( tr.pshadowFbos[i], GL_RGBA8, 0, 0 );
            FBO_AttachImage( tr.pshadowFbos[i], tr.pshadowMaps[i], GL_DEPTH_ATTACHMENT_EXT, 0 );
            R_CheckFBO( tr.pshadowFbos[i] );
        }
    }
    
    if( tr.sunShadowDepthImage[0] )
    {
        for( i = 0; i < 4; i++ )
        {
            tr.sunShadowFbo[i] = FBO_Create( "_sunshadowmap", tr.sunShadowDepthImage[i]->width, tr.sunShadowDepthImage[i]->height );
            // FIXME: this next line wastes 16mb with 4x1024x1024 sun shadow maps, skip if OpenGL 4.3+ or ARB_framebuffer_no_attachments
            // This at least gets sun shadows working on older GPUs (Intel)
            FBO_CreateBuffer( tr.sunShadowFbo[i], GL_RGBA8, 0, 0 );
            FBO_AttachImage( tr.sunShadowFbo[i], tr.sunShadowDepthImage[i], GL_DEPTH_ATTACHMENT_EXT, 0 );
            R_CheckFBO( tr.sunShadowFbo[i] );
        }
    }
    
    if( tr.screenShadowImage )
    {
        tr.screenShadowFbo = FBO_Create( "_screenshadow", tr.screenShadowImage->width, tr.screenShadowImage->height );
        FBO_AttachImage( tr.screenShadowFbo, tr.screenShadowImage, GL_COLOR_ATTACHMENT0_EXT, 0 );
        R_CheckFBO( tr.screenShadowFbo );
    }
    
    if( tr.textureScratchImage[0] )
    {
        for( i = 0; i < 2; i++ )
        {
            tr.textureScratchFbo[i] = FBO_Create( va( "_texturescratch%d", i ), tr.textureScratchImage[i]->width, tr.textureScratchImage[i]->height );
            FBO_AttachImage( tr.textureScratchFbo[i], tr.textureScratchImage[i], GL_COLOR_ATTACHMENT0_EXT, 0 );
            R_CheckFBO( tr.textureScratchFbo[i] );
        }
    }
    
    if( tr.calcLevelsImage )
    {
        tr.calcLevelsFbo = FBO_Create( "_calclevels", tr.calcLevelsImage->width, tr.calcLevelsImage->height );
        FBO_AttachImage( tr.calcLevelsFbo, tr.calcLevelsImage, GL_COLOR_ATTACHMENT0_EXT, 0 );
        R_CheckFBO( tr.calcLevelsFbo );
    }
    
    if( tr.targetLevelsImage )
    {
        tr.targetLevelsFbo = FBO_Create( "_targetlevels", tr.targetLevelsImage->width, tr.targetLevelsImage->height );
        FBO_AttachImage( tr.targetLevelsFbo, tr.targetLevelsImage, GL_COLOR_ATTACHMENT0_EXT, 0 );
        R_CheckFBO( tr.targetLevelsFbo );
    }
    
    if( tr.quarterImage[0] )
    {
        for( i = 0; i < 2; i++ )
        {
            tr.quarterFbo[i] = FBO_Create( va( "_quarter%d", i ), tr.quarterImage[i]->width, tr.quarterImage[i]->height );
            FBO_AttachImage( tr.quarterFbo[i], tr.quarterImage[i], GL_COLOR_ATTACHMENT0_EXT, 0 );
            R_CheckFBO( tr.quarterFbo[i] );
        }
    }
    
    if( tr.hdrDepthImage )
    {
        tr.hdrDepthFbo = FBO_Create( "_hdrDepth", tr.hdrDepthImage->width, tr.hdrDepthImage->height );
        FBO_AttachImage( tr.hdrDepthFbo, tr.hdrDepthImage, GL_COLOR_ATTACHMENT0_EXT, 0 );
        R_CheckFBO( tr.hdrDepthFbo );
    }
    
    if( tr.screenSsaoImage )
    {
        tr.screenSsaoFbo = FBO_Create( "_screenssao", tr.screenSsaoImage->width, tr.screenSsaoImage->height );
        FBO_AttachImage( tr.screenSsaoFbo, tr.screenSsaoImage, GL_COLOR_ATTACHMENT0_EXT, 0 );
        R_CheckFBO( tr.screenSsaoFbo );
    }
    
    if( tr.renderCubeImage )
    {
        tr.renderCubeFbo = FBO_Create( "_renderCubeFbo", tr.renderCubeImage->width, tr.renderCubeImage->height );
        FBO_AttachImage( tr.renderCubeFbo, tr.renderCubeImage, GL_COLOR_ATTACHMENT0_EXT, 0 );
        FBO_CreateBuffer( tr.renderCubeFbo, GL_DEPTH_COMPONENT24_ARB, 0, 0 );
        R_CheckFBO( tr.renderCubeFbo );
    }
    
    GL_CheckErrors();
    
    GL_BindFramebuffer( GL_FRAMEBUFFER_EXT, 0 );
    glState.currentFBO = NULL;
}

/*
============
idRenderSystemLocal::FBOShutdown
============
*/
void idRenderSystemLocal::FBOShutdown( void )
{
    S32             i, j;
    FBO_t*          fbo;
    
    CL_RefPrintf( PRINT_ALL, "------- idRenderSystemLocal::FBOShutdown -------\n" );
    
    if( !glRefConfig.framebufferObject )
        return;
        
    FBO_Bind( NULL );
    
    for( i = 0; i < tr.numFBOs; i++ )
    {
        fbo = tr.fbos[i];
        
        for( j = 0; j < glRefConfig.maxColorAttachments; j++ )
        {
            if( fbo->colorBuffers[j] )
                glDeleteRenderbuffersEXT( 1, &fbo->colorBuffers[j] );
        }
        
        if( fbo->depthBuffer )
            glDeleteRenderbuffersEXT( 1, &fbo->depthBuffer );
            
        if( fbo->stencilBuffer )
            glDeleteRenderbuffersEXT( 1, &fbo->stencilBuffer );
            
        if( fbo->frameBuffer )
            glDeleteFramebuffersEXT( 1, &fbo->frameBuffer );
    }
}

/*
============
R_FBOList_f
============
*/
void R_FBOList_f( void )
{
    S32             i;
    FBO_t*          fbo;
    
    if( !glRefConfig.framebufferObject )
    {
        CL_RefPrintf( PRINT_ALL, "GL_EXT_framebuffer_object is not available.\n" );
        return;
    }
    
    CL_RefPrintf( PRINT_ALL, "             size       name\n" );
    CL_RefPrintf( PRINT_ALL, "----------------------------------------------------------\n" );
    
    for( i = 0; i < tr.numFBOs; i++ )
    {
        fbo = tr.fbos[i];
        
        CL_RefPrintf( PRINT_ALL, "  %4i: %4i %4i %s\n", i, fbo->width, fbo->height, fbo->name );
    }
    
    CL_RefPrintf( PRINT_ALL, " %i FBOs\n", tr.numFBOs );
}

void FBO_BlitFromTexture( struct image_s* src, vec4_t inSrcTexCorners, vec2_t inSrcTexScale, FBO_t* dst, ivec4_t inDstBox, struct shaderProgram_s* shaderProgram, vec4_t inColor, S32 blend )
{
    ivec4_t dstBox;
    vec4_t color;
    vec4_t quadVerts[4];
    vec2_t texCoords[4];
    vec2_t invTexRes;
    FBO_t* oldFbo = glState.currentFBO;
    mat4_t projection;
    S32 width, height;
    
    if( !src )
    {
        CL_RefPrintf( PRINT_WARNING, "Tried to blit from a NULL texture!\n" );
        return;
    }
    
    width  = dst ? dst->width  : glConfig.vidWidth;
    height = dst ? dst->height : glConfig.vidHeight;
    
    if( inSrcTexCorners )
    {
        VectorSet2( texCoords[0], inSrcTexCorners[0], inSrcTexCorners[1] );
        VectorSet2( texCoords[1], inSrcTexCorners[2], inSrcTexCorners[1] );
        VectorSet2( texCoords[2], inSrcTexCorners[2], inSrcTexCorners[3] );
        VectorSet2( texCoords[3], inSrcTexCorners[0], inSrcTexCorners[3] );
    }
    else
    {
        VectorSet2( texCoords[0], 0.0f, 1.0f );
        VectorSet2( texCoords[1], 1.0f, 1.0f );
        VectorSet2( texCoords[2], 1.0f, 0.0f );
        VectorSet2( texCoords[3], 0.0f, 0.0f );
    }
    
    // framebuffers are 0 bottom, Y up.
    if( inDstBox )
    {
        dstBox[0] = inDstBox[0];
        dstBox[1] = height - inDstBox[1] - inDstBox[3];
        dstBox[2] = inDstBox[0] + inDstBox[2];
        dstBox[3] = height - inDstBox[1];
    }
    else
    {
        VectorSet4( dstBox, 0, height, width, 0 );
    }
    
    if( inSrcTexScale )
    {
        VectorCopy2( inSrcTexScale, invTexRes );
    }
    else
    {
        VectorSet2( invTexRes, 1.0f, 1.0f );
    }
    
    if( inColor )
    {
        VectorCopy4( inColor, color );
    }
    else
    {
        VectorCopy4( colorWhite, color );
    }
    
    if( !shaderProgram )
    {
        shaderProgram = &tr.textureColorShader;
    }
    
    FBO_Bind( dst );
    
    glViewport( 0, 0, width, height );
    glScissor( 0, 0, width, height );
    
    Mat4Ortho( 0, width, height, 0, 0, 1, projection );
    
    GL_Cull( CT_TWO_SIDED );
    
    GL_BindToTMU( src, TB_COLORMAP );
    
    VectorSet4( quadVerts[0], dstBox[0], dstBox[1], 0.0f, 1.0f );
    VectorSet4( quadVerts[1], dstBox[2], dstBox[1], 0.0f, 1.0f );
    VectorSet4( quadVerts[2], dstBox[2], dstBox[3], 0.0f, 1.0f );
    VectorSet4( quadVerts[3], dstBox[0], dstBox[3], 0.0f, 1.0f );
    
    invTexRes[0] /= src->width;
    invTexRes[1] /= src->height;
    
    GL_State( blend );
    
    GLSL_BindProgram( shaderProgram );
    
    GLSL_SetUniformMat4( shaderProgram, UNIFORM_MODELVIEWPROJECTIONMATRIX, projection );
    GLSL_SetUniformVec4( shaderProgram, UNIFORM_COLOR, color );
    GLSL_SetUniformVec2( shaderProgram, UNIFORM_INVTEXRES, invTexRes );
    GLSL_SetUniformVec2( shaderProgram, UNIFORM_AUTOEXPOSUREMINMAX, tr.refdef.autoExposureMinMax );
    GLSL_SetUniformVec3( shaderProgram, UNIFORM_TONEMINAVGMAXLINEAR, tr.refdef.toneMinAvgMaxLinear );
    
    RB_InstantQuad2( quadVerts, texCoords );
    
    FBO_Bind( oldFbo );
}

void FBO_Blit( FBO_t* src, ivec4_t inSrcBox, vec2_t srcTexScale, FBO_t* dst, ivec4_t dstBox, struct shaderProgram_s* shaderProgram, vec4_t color, S32 blend )
{
    vec4_t srcTexCorners;
    
    if( !src )
    {
        CL_RefPrintf( PRINT_WARNING, "Tried to blit from a NULL FBO!\n" );
        return;
    }
    
    if( inSrcBox )
    {
        srcTexCorners[0] =  inSrcBox[0]                / ( F32 )src->width;
        srcTexCorners[1] = ( inSrcBox[1] + inSrcBox[3] ) / ( F32 )src->height;
        srcTexCorners[2] = ( inSrcBox[0] + inSrcBox[2] ) / ( F32 )src->width;
        srcTexCorners[3] =  inSrcBox[1]                / ( F32 )src->height;
    }
    else
    {
        VectorSet4( srcTexCorners, 0.0f, 0.0f, 1.0f, 1.0f );
    }
    
    FBO_BlitFromTexture( src->colorImage[0], srcTexCorners, srcTexScale, dst, dstBox, shaderProgram, color, blend | GLS_DEPTHTEST_DISABLE );
}

void FBO_FastBlit( FBO_t* src, ivec4_t srcBox, FBO_t* dst, ivec4_t dstBox, S32 buffers, S32 filter )
{
    ivec4_t srcBoxFinal, dstBoxFinal;
    GLuint srcFb, dstFb;
    
    if( !glRefConfig.framebufferBlit )
    {
        FBO_Blit( src, srcBox, NULL, dst, dstBox, NULL, NULL, 0 );
        return;
    }
    
    srcFb = src ? src->frameBuffer : 0;
    dstFb = dst ? dst->frameBuffer : 0;
    
    if( !srcBox )
    {
        S32 width =  src ? src->width  : glConfig.vidWidth;
        S32 height = src ? src->height : glConfig.vidHeight;
        
        VectorSet4( srcBoxFinal, 0, 0, width, height );
    }
    else
    {
        VectorSet4( srcBoxFinal, srcBox[0], srcBox[1], srcBox[0] + srcBox[2], srcBox[1] + srcBox[3] );
    }
    
    if( !dstBox )
    {
        S32 width  = dst ? dst->width  : glConfig.vidWidth;
        S32 height = dst ? dst->height : glConfig.vidHeight;
        
        VectorSet4( dstBoxFinal, 0, 0, width, height );
    }
    else
    {
        VectorSet4( dstBoxFinal, dstBox[0], dstBox[1], dstBox[0] + dstBox[2], dstBox[1] + dstBox[3] );
    }
    
    GL_BindFramebuffer( GL_READ_FRAMEBUFFER_EXT, srcFb );
    GL_BindFramebuffer( GL_DRAW_FRAMEBUFFER_EXT, dstFb );
    glBlitFramebufferEXT( srcBoxFinal[0], srcBoxFinal[1], srcBoxFinal[2], srcBoxFinal[3],
                          dstBoxFinal[0], dstBoxFinal[1], dstBoxFinal[2], dstBoxFinal[3],
                          buffers, filter );
                          
    GL_BindFramebuffer( GL_FRAMEBUFFER_EXT, 0 );
    glState.currentFBO = NULL;
}

void FBO_FastBlitIndexed( FBO_t* src, FBO_t* dst, S32 srcReadBuffer, S32 dstDrawBuffer, S32 buffers, S32 filter )
{
    assert( src != NULL );
    assert( dst != NULL );
    
    glBindFramebufferEXT( GL_READ_FRAMEBUFFER_EXT, src->frameBuffer );
    glReadBuffer( GL_COLOR_ATTACHMENT0_EXT + srcReadBuffer );
    
    glBindFramebufferEXT( GL_DRAW_FRAMEBUFFER_EXT, dst->frameBuffer );
    glDrawBuffer( GL_COLOR_ATTACHMENT0_EXT + dstDrawBuffer );
    
    glBlitFramebufferEXT( 0, 0, src->width, src->height,
                          0, 0, dst->width, dst->height,
                          buffers, filter );
                          
    glReadBuffer( GL_COLOR_ATTACHMENT0_EXT );
    
    glState.currentFBO = dst;
    
    glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
    glState.currentFBO = NULL;
}
