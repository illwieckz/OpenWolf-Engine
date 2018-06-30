////////////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2011 Andrei Drexler, Richard Allen, James Canete
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
// File name:   r_postprocess.cpp
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#include <OWLib/precompiled.h>

void RB_ToneMap( FBO_t* hdrFbo, ivec4_t hdrBox, FBO_t* ldrFbo, ivec4_t ldrBox, S32 autoExposure )
{
    ivec4_t srcBox, dstBox;
    vec4_t color;
    static S32 lastFrameCount = 0;
    
    if( autoExposure )
    {
        if( lastFrameCount == 0 || tr.frameCount < lastFrameCount || tr.frameCount - lastFrameCount > 5 )
        {
            // determine average log luminance
            FBO_t* srcFbo, *dstFbo, *tmp;
            S32 size = 256;
            
            lastFrameCount = tr.frameCount;
            
            VectorSet4( dstBox, 0, 0, size, size );
            
            FBO_Blit( hdrFbo, hdrBox, NULL, tr.textureScratchFbo[0], dstBox, &tr.calclevels4xShader[0], NULL, 0 );
            
            srcFbo = tr.textureScratchFbo[0];
            dstFbo = tr.textureScratchFbo[1];
            
            // downscale to 1x1 texture
            while( size > 1 )
            {
                VectorSet4( srcBox, 0, 0, size, size );
                //size >>= 2;
                size >>= 1;
                VectorSet4( dstBox, 0, 0, size, size );
                
                if( size == 1 )
                    dstFbo = tr.targetLevelsFbo;
                    
                //FBO_Blit(targetFbo, srcBox, NULL, tr.textureScratchFbo[nextScratch], dstBox, &tr.calclevels4xShader[1], NULL, 0);
                FBO_FastBlit( srcFbo, srcBox, dstFbo, dstBox, GL_COLOR_BUFFER_BIT, GL_LINEAR );
                
                tmp = srcFbo;
                srcFbo = dstFbo;
                dstFbo = tmp;
            }
        }
        
        // blend with old log luminance for gradual change
        VectorSet4( srcBox, 0, 0, 0, 0 );
        
        color[0] =
            color[1] =
                color[2] = 1.0f;
        if( glRefConfig.textureFloat )
            color[3] = 0.03f;
        else
            color[3] = 0.1f;
            
        FBO_Blit( tr.targetLevelsFbo, srcBox, NULL, tr.calcLevelsFbo, NULL,  NULL, color, GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA );
    }
    
    // tonemap
    color[0] =
        color[1] =
            color[2] = pow( 2, r_cameraExposure->value ); //exp2(r_cameraExposure->value);
    color[3] = 1.0f;
    
    if( autoExposure )
        GL_BindToTMU( tr.calcLevelsImage,  TB_LEVELSMAP );
    else
        GL_BindToTMU( tr.fixedLevelsImage, TB_LEVELSMAP );
        
    FBO_Blit( hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.tonemapShader, color, 0 );
}

/*
=============
RB_BokehBlur


Blurs a part of one framebuffer to another.

Framebuffers can be identical.
=============
*/
void RB_BokehBlur( FBO_t* src, ivec4_t srcBox, FBO_t* dst, ivec4_t dstBox, F32 blur )
{
//	ivec4_t srcBox, dstBox;
    vec4_t color;
    
    blur *= 10.0f;
    
    if( blur < 0.004f )
        return;
        
    if( glRefConfig.framebufferObject )
    {
        // bokeh blur
        if( blur > 0.0f )
        {
            ivec4_t quarterBox;
            
            quarterBox[0] = 0;
            quarterBox[1] = tr.quarterFbo[0]->height;
            quarterBox[2] = tr.quarterFbo[0]->width;
            quarterBox[3] = -tr.quarterFbo[0]->height;
            
            // create a quarter texture
            //FBO_Blit(NULL, NULL, NULL, tr.quarterFbo[0], NULL, NULL, NULL, 0);
            FBO_FastBlit( src, srcBox, tr.quarterFbo[0], quarterBox, GL_COLOR_BUFFER_BIT, GL_LINEAR );
        }
        
#ifndef HQ_BLUR
        if( blur > 1.0f )
        {
            // create a 1/16th texture
            //FBO_Blit(tr.quarterFbo[0], NULL, NULL, tr.textureScratchFbo[0], NULL, NULL, NULL, 0);
            FBO_FastBlit( tr.quarterFbo[0], NULL, tr.textureScratchFbo[0], NULL, GL_COLOR_BUFFER_BIT, GL_LINEAR );
        }
#endif
        
        if( blur > 0.0f && blur <= 1.0f )
        {
            // Crossfade original with quarter texture
            VectorSet4( color, 1, 1, 1, blur );
            
            FBO_Blit( tr.quarterFbo[0], NULL, NULL, dst, dstBox, NULL, color, GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA );
        }
#ifndef HQ_BLUR
        // ok blur, but can see some pixelization
        else if( blur > 1.0f && blur <= 2.0f )
        {
            // crossfade quarter texture with 1/16th texture
            FBO_Blit( tr.quarterFbo[0], NULL, NULL, dst, dstBox, NULL, NULL, 0 );
            
            VectorSet4( color, 1, 1, 1, blur - 1.0f );
            
            FBO_Blit( tr.textureScratchFbo[0], NULL, NULL, dst, dstBox, NULL, color, GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA );
        }
        else if( blur > 2.0f )
        {
            // blur 1/16th texture then replace
            S32 i;
            
            for( i = 0; i < 2; i++ )
            {
                vec2_t blurTexScale;
                F32 subblur;
                
                subblur = ( ( blur - 2.0f ) / 2.0f ) / 3.0f * ( F32 )( i + 1 );
                
                blurTexScale[0] =
                    blurTexScale[1] = subblur;
                    
                color[0] =
                    color[1] =
                        color[2] = 0.5f;
                color[3] = 1.0f;
                
                if( i != 0 )
                    FBO_Blit( tr.textureScratchFbo[0], NULL, blurTexScale, tr.textureScratchFbo[1], NULL, &tr.bokehShader, color, GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE );
                else
                    FBO_Blit( tr.textureScratchFbo[0], NULL, blurTexScale, tr.textureScratchFbo[1], NULL, &tr.bokehShader, color, 0 );
            }
            
            FBO_Blit( tr.textureScratchFbo[1], NULL, NULL, dst, dstBox, NULL, NULL, 0 );
        }
#else // higher quality blur, but slower
        else if( blur > 1.0f )
        {
            // blur quarter texture then replace
            S32 i;
        
            src = tr.quarterFbo[0];
            dst = tr.quarterFbo[1];
        
            VectorSet4( color, 0.5f, 0.5f, 0.5f, 1 );
        
            for( i = 0; i < 2; i++ )
            {
                vec2_t blurTexScale;
                F32 subblur;
        
                subblur = ( blur - 1.0f ) / 2.0f * ( F32 )( i + 1 );
        
                blurTexScale[0] =
                    blurTexScale[1] = subblur;
        
                color[0] =
                    color[1] =
                        color[2] = 1.0f;
                if( i != 0 )
                    color[3] = 1.0f;
                else
                    color[3] = 0.5f;
        
                FBO_Blit( tr.quarterFbo[0], NULL, blurTexScale, tr.quarterFbo[1], NULL, &tr.bokehShader, color, GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA );
            }
        
            FBO_Blit( tr.quarterFbo[1], NULL, NULL, dst, dstBox, NULL, NULL, 0 );
        }
#endif
    }
}


static void RB_RadialBlur( FBO_t* srcFbo, FBO_t* dstFbo, S32 passes, F32 stretch, F32 x, F32 y, F32 w, F32 h, F32 xcenter, F32 ycenter, F32 alpha )
{
    ivec4_t srcBox, dstBox;
    S32 srcWidth, srcHeight;
    vec4_t color;
    const F32 inc = 1.f / passes;
    const F32 mul = powf( stretch, inc );
    F32 scale;
    
    alpha *= inc;
    VectorSet4( color, alpha, alpha, alpha, 1.0f );
    
    srcWidth  = srcFbo ? srcFbo->width  : glConfig.vidWidth;
    srcHeight = srcFbo ? srcFbo->height : glConfig.vidHeight;
    
    VectorSet4( srcBox, 0, 0, srcWidth, srcHeight );
    
    VectorSet4( dstBox, x, y, w, h );
    FBO_Blit( srcFbo, srcBox, NULL, dstFbo, dstBox, NULL, color, 0 );
    
    --passes;
    scale = mul;
    while( passes > 0 )
    {
        F32 iscale = 1.f / scale;
        F32 s0 = xcenter * ( 1.f - iscale );
        F32 t0 = ( 1.0f - ycenter ) * ( 1.f - iscale );
        
        srcBox[0] = s0 * srcWidth;
        srcBox[1] = t0 * srcHeight;
        srcBox[2] = iscale * srcWidth;
        srcBox[3] = iscale * srcHeight;
        
        FBO_Blit( srcFbo, srcBox, NULL, dstFbo, dstBox, NULL, color, GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE );
        
        scale *= mul;
        --passes;
    }
}


static bool RB_UpdateSunFlareVis( void )
{
    U32 sampleCount = 0;
    if( !glRefConfig.occlusionQuery )
        return true;
        
    tr.sunFlareQueryIndex ^= 1;
    if( !tr.sunFlareQueryActive[tr.sunFlareQueryIndex] )
        return true;
        
    /* debug code */
    if( 0 )
    {
        S32 iter;
        for( iter = 0 ; ; ++iter )
        {
            S32 available = 0;
            glGetQueryObjectiv( tr.sunFlareQuery[tr.sunFlareQueryIndex], GL_QUERY_RESULT_AVAILABLE, &available );
            if( available )
                break;
        }
        
        CL_RefPrintf( PRINT_DEVELOPER, "Waited %d iterations\n", iter );
    }
    
    glGetQueryObjectuiv( tr.sunFlareQuery[tr.sunFlareQueryIndex], GL_QUERY_RESULT, &sampleCount );
    return sampleCount > 0;
}

void RB_SunRays( FBO_t* srcFbo, ivec4_t srcBox, FBO_t* dstFbo, ivec4_t dstBox )
{
    vec4_t color;
    F32 dot;
    const F32 cutoff = 0.25f;
    bool colorize = true;
    
//	F32 w, h, w2, h2;
    mat4_t mvp;
    vec4_t pos, hpos;
    
    dot = DotProduct( tr.sunDirection, backEnd.viewParms.orientation.axis[0] );
    if( dot < cutoff )
        return;
        
    if( !RB_UpdateSunFlareVis() )
        return;
        
    // From RB_DrawSun()
    {
        F32 dist;
        mat4_t trans, model;
        
        Mat4Translation( backEnd.viewParms.orientation.origin, trans );
        Mat4Multiply( backEnd.viewParms.world.modelMatrix, trans, model );
        Mat4Multiply( backEnd.viewParms.projectionMatrix, model, mvp );
        
        dist = backEnd.viewParms.zFar / 1.75;		// div sqrt(3)
        
        VectorScale( tr.sunDirection, dist, pos );
    }
    
    // project sun point
    //Mat4Multiply(backEnd.viewParms.projectionMatrix, backEnd.viewParms.world.modelMatrix, mvp);
    Mat4Transform( mvp, pos, hpos );
    
    // transform to UV coords
    hpos[3] = 0.5f / hpos[3];
    
    pos[0] = 0.5f + hpos[0] * hpos[3];
    pos[1] = 0.5f + hpos[1] * hpos[3];
    
    // initialize quarter buffers
    {
        F32 mul = 1.f;
        ivec4_t rayBox, quarterBox;
        S32 srcWidth  = srcFbo ? srcFbo->width  : glConfig.vidWidth;
        S32 srcHeight = srcFbo ? srcFbo->height : glConfig.vidHeight;
        
        VectorSet4( color, mul, mul, mul, 1 );
        
        rayBox[0] = srcBox[0] * tr.sunRaysFbo->width  / srcWidth;
        rayBox[1] = srcBox[1] * tr.sunRaysFbo->height / srcHeight;
        rayBox[2] = srcBox[2] * tr.sunRaysFbo->width  / srcWidth;
        rayBox[3] = srcBox[3] * tr.sunRaysFbo->height / srcHeight;
        
        quarterBox[0] = 0;
        quarterBox[1] = tr.quarterFbo[0]->height;
        quarterBox[2] = tr.quarterFbo[0]->width;
        quarterBox[3] = -tr.quarterFbo[0]->height;
        
        // first, downsample the framebuffer
        if( colorize )
        {
            FBO_FastBlit( srcFbo, srcBox, tr.quarterFbo[0], quarterBox, GL_COLOR_BUFFER_BIT, GL_LINEAR );
            FBO_Blit( tr.sunRaysFbo, rayBox, NULL, tr.quarterFbo[0], quarterBox, NULL, color, GLS_SRCBLEND_DST_COLOR | GLS_DSTBLEND_ZERO );
        }
        else
        {
            FBO_FastBlit( tr.sunRaysFbo, rayBox, tr.quarterFbo[0], quarterBox, GL_COLOR_BUFFER_BIT, GL_LINEAR );
        }
    }
    
    // radial blur passes, ping-ponging between the two quarter-size buffers
    {
        const F32 stretch_add = 2.f / 3.f;
        F32 stretch = 1.f + stretch_add;
        S32 i;
        for( i = 0; i < 2; ++i )
        {
            RB_RadialBlur( tr.quarterFbo[i & 1], tr.quarterFbo[( ~i ) & 1], 5, stretch, 0.f, 0.f, tr.quarterFbo[0]->width, tr.quarterFbo[0]->height, pos[0], pos[1], 1.125f );
            stretch += stretch_add;
        }
    }
    
    // add result back on top of the main buffer
    {
        F32 mul = 1.f;
        
        VectorSet4( color, mul, mul, mul, 1 );
        
        FBO_Blit( tr.quarterFbo[0], NULL, NULL, dstFbo, dstBox, NULL, color, GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE );
    }
}

static void RB_BlurAxis( FBO_t* srcFbo, FBO_t* dstFbo, F32 strength, bool horizontal )
{
    F32 dx, dy;
    F32 xmul, ymul;
    F32 weights[3] =
    {
        0.227027027f,
        0.316216216f,
        0.070270270f,
    };
    F32 offsets[3] =
    {
        0.f,
        1.3846153846f,
        3.2307692308f,
    };
    
    xmul = horizontal;
    ymul = 1.f - xmul;
    
    xmul *= strength;
    ymul *= strength;
    
    {
        ivec4_t srcBox, dstBox;
        vec4_t color;
        
        VectorSet4( color, weights[0], weights[0], weights[0], 1.0f );
        VectorSet4( srcBox, 0, 0, srcFbo->width, srcFbo->height );
        VectorSet4( dstBox, 0, 0, dstFbo->width, dstFbo->height );
        FBO_Blit( srcFbo, srcBox, NULL, dstFbo, dstBox, NULL, color, 0 );
        
        VectorSet4( color, weights[1], weights[1], weights[1], 1.0f );
        dx = offsets[1] * xmul;
        dy = offsets[1] * ymul;
        VectorSet4( srcBox, dx, dy, srcFbo->width, srcFbo->height );
        FBO_Blit( srcFbo, srcBox, NULL, dstFbo, dstBox, NULL, color, GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE );
        VectorSet4( srcBox, -dx, -dy, srcFbo->width, srcFbo->height );
        FBO_Blit( srcFbo, srcBox, NULL, dstFbo, dstBox, NULL, color, GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE );
        
        VectorSet4( color, weights[2], weights[2], weights[2], 1.0f );
        dx = offsets[2] * xmul;
        dy = offsets[2] * ymul;
        VectorSet4( srcBox, dx, dy, srcFbo->width, srcFbo->height );
        FBO_Blit( srcFbo, srcBox, NULL, dstFbo, dstBox, NULL, color, GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE );
        VectorSet4( srcBox, -dx, -dy, srcFbo->width, srcFbo->height );
        FBO_Blit( srcFbo, srcBox, NULL, dstFbo, dstBox, NULL, color, GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE );
    }
}

void RB_HBlur( FBO_t* srcFbo, FBO_t* dstFbo, F32 strength )
{
    RB_BlurAxis( srcFbo, dstFbo, strength, true );
}

void RB_VBlur( FBO_t* srcFbo, FBO_t* dstFbo, F32 strength )
{
    RB_BlurAxis( srcFbo, dstFbo, strength, false );
}

void RB_GaussianBlur( F32 blur )
{
    //float mul = 1.f;
    float factor = Com_Clamp( 0.f, 1.f, blur );
    
    if( factor <= 0.f )
        return;
        
    {
        ivec4_t srcBox, dstBox;
        vec4_t color;
        
        VectorSet4( color, 1, 1, 1, 1 );
        
        // first, downsample the framebuffer
        FBO_FastBlit( NULL, NULL, tr.quarterFbo[0], NULL, GL_COLOR_BUFFER_BIT, GL_LINEAR );
        FBO_FastBlit( tr.quarterFbo[0], NULL, tr.textureScratchFbo[0], NULL, GL_COLOR_BUFFER_BIT, GL_LINEAR );
        
        // set the alpha channel
        glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE );
        FBO_BlitFromTexture( tr.whiteImage, NULL, NULL, tr.textureScratchFbo[0], NULL, NULL, color, GLS_DEPTHTEST_DISABLE );
        glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
        
        // blur the tiny buffer horizontally and vertically
        RB_HBlur( tr.textureScratchFbo[0], tr.textureScratchFbo[1], factor );
        RB_VBlur( tr.textureScratchFbo[1], tr.textureScratchFbo[0], factor );
        
        // finally, merge back to framebuffer
        VectorSet4( srcBox, 0, 0, tr.textureScratchFbo[0]->width, tr.textureScratchFbo[0]->height );
        VectorSet4( dstBox, 0, 0, glConfig.vidWidth,              glConfig.vidHeight );
        color[3] = factor;
        FBO_Blit( tr.textureScratchFbo[0], srcBox, NULL, NULL, dstBox, NULL, color, GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA );
    }
}

void RB_GaussianBlur( FBO_t* srcFbo, FBO_t* intermediateFbo, FBO_t* dstFbo, float spread )
{
    // Blur X
    vec2_t scale;
    VectorSet2( scale, spread, spread );
    
    FBO_Blit( srcFbo, NULL, scale, intermediateFbo, NULL, &tr.gaussianBlurShader[0], NULL, GLS_SRCBLEND_ONE | GLS_DSTBLEND_ZERO );
    
    // Blur Y
    FBO_Blit( intermediateFbo, NULL, scale, dstFbo, NULL, &tr.gaussianBlurShader[1], NULL, GLS_SRCBLEND_ONE | GLS_DSTBLEND_ZERO );
}

void RB_DarkExpand( FBO_t* hdrFbo, ivec4_t hdrBox, FBO_t* ldrFbo, ivec4_t ldrBox )
{
    vec4_t color;
    
    // bloom
    color[0] =
        color[1] =
            color[2] = pow( 2, r_cameraExposure->value );
    color[3] = 1.0f;
    
    GLSL_BindProgram( &tr.darkexpandShader );
    
    GL_BindToTMU( tr.fixedLevelsImage, TB_LEVELSMAP );
    
    {
        vec2_t screensize;
        screensize[0] = glConfig.vidWidth;
        screensize[1] = glConfig.vidHeight;
        
        GLSL_SetUniformVec2( &tr.darkexpandShader, UNIFORM_DIMENSIONS, screensize );
    }
    
    FBO_Blit( hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.darkexpandShader, color, GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA );
}

void RB_Anamorphic( FBO_t* hdrFbo, ivec4_t hdrBox, FBO_t* ldrFbo, ivec4_t ldrBox )
{
    vec4_t	color;
    ivec4_t halfBox;
    vec2_t	texScale, texHalfScale, texDoubleScale;
    
    texScale[0] = texScale[1] = 1.0f;
    texHalfScale[0] = texHalfScale[1] = texScale[0] / 8.0;
    texDoubleScale[0] = texDoubleScale[1] = texScale[0] * 8.0;
    
    // bloom
    color[0] =
        color[1] =
            color[2] = pow( 2, r_cameraExposure->value );
    color[3] = 1.0f;
    
    halfBox[0] = backEnd.viewParms.viewportX      * tr.anamorphicRenderFBOImage[0]->width / ( float )glConfig.vidWidth;
    halfBox[1] = backEnd.viewParms.viewportY      * tr.anamorphicRenderFBOImage[0]->height / ( float )glConfig.vidHeight;
    halfBox[2] = backEnd.viewParms.viewportWidth  * tr.anamorphicRenderFBOImage[0]->width / ( float )glConfig.vidWidth;
    halfBox[3] = backEnd.viewParms.viewportHeight * tr.anamorphicRenderFBOImage[0]->height / ( float )glConfig.vidHeight;
    
    //
    // Darken to VBO...
    //
    
    GLSL_BindProgram( &tr.anamorphicDarkenShader );
    
    GL_BindToTMU( tr.fixedLevelsImage, TB_DIFFUSEMAP );
    
    {
        vec2_t screensize;
        screensize[0] = glConfig.vidWidth;
        screensize[1] = glConfig.vidHeight;
        
        GLSL_SetUniformVec2( &tr.anamorphicDarkenShader, UNIFORM_DIMENSIONS, screensize );
    }
    
    {
        vec4_t local0;
        VectorSet4( local0, r_anamorphicDarkenPower->value, 0.0, 0.0, 0.0 );
        GLSL_SetUniformVec4( &tr.anamorphicDarkenShader, UNIFORM_LOCAL0, local0 );
    }
    
    FBO_Blit( hdrFbo, NULL, texHalfScale, tr.anamorphicRenderFBO[1], NULL, &tr.anamorphicDarkenShader, color, 0 );
    FBO_FastBlit( tr.anamorphicRenderFBO[1], NULL, tr.anamorphicRenderFBO[0], NULL, GL_COLOR_BUFFER_BIT, GL_LINEAR );
    
    //
    // Blur the new darken'ed VBO...
    //
    
    for( S32 i = 0; i < /*r_bloomPasses->integer*/ 8; i++ )
    {
        //
        // Bloom X axis... (to VBO 1)
        //
        
        //for (S32 width = 1; width < 12 ; width++)
        {
            GLSL_BindProgram( &tr.anamorphicBlurShader );
            
            GL_BindToTMU( tr.anamorphicRenderFBOImage[0], TB_DIFFUSEMAP );
            
            {
                vec2_t screensize;
                screensize[0] = tr.anamorphicRenderFBOImage[0]->width;
                screensize[1] = tr.anamorphicRenderFBOImage[0]->height;
                
                GLSL_SetUniformVec2( &tr.anamorphicBlurShader, UNIFORM_DIMENSIONS, screensize );
            }
            
            {
                vec4_t local0;
                //VectorSet4(local0, (float)width, 0.0, 0.0, 0.0);
                VectorSet4( local0, 1.0, 0.0, 16.0, 0.0 );
                GLSL_SetUniformVec4( &tr.anamorphicBlurShader, UNIFORM_LOCAL0, local0 );
            }
            
            FBO_Blit( tr.anamorphicRenderFBO[0], NULL, NULL, tr.anamorphicRenderFBO[1], NULL, &tr.anamorphicBlurShader, color, 0 );
            FBO_FastBlit( tr.anamorphicRenderFBO[1], NULL, tr.anamorphicRenderFBO[0], NULL, GL_COLOR_BUFFER_BIT, GL_LINEAR );
        }
    }
    
    //
    // Copy (and upscale) the bloom image to our full screen image...
    //
    
    FBO_Blit( tr.anamorphicRenderFBO[0], NULL, texDoubleScale, tr.anamorphicRenderFBO[2], NULL, &tr.anamorphicBlurShader, color, 0 );
    
    //
    // Combine the screen with the bloom'ed VBO...
    //
    
    
    GLSL_BindProgram( &tr.anamorphicCombineShader );
    
    GL_BindToTMU( tr.fixedLevelsImage, TB_DIFFUSEMAP );
    
    GLSL_SetUniformInt( &tr.anamorphicCombineShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP );
    GLSL_SetUniformInt( &tr.anamorphicCombineShader, UNIFORM_NORMALMAP, TB_NORMALMAP );
    
    GL_BindToTMU( tr.anamorphicRenderFBOImage[2], TB_NORMALMAP );
    
    {
        vec4_t local0;
        VectorSet4( local0, 0.6, 0.0, 0.0, 0.0 );
        GLSL_SetUniformVec4( &tr.anamorphicCombineShader, UNIFORM_LOCAL0, local0 );
    }
    
    FBO_Blit( hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.anamorphicCombineShader, color, GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA );
    
    //
    // Render the results now...
    //
    
    FBO_FastBlit( ldrFbo, NULL, hdrFbo, NULL, GL_COLOR_BUFFER_BIT, GL_LINEAR );
}


void RB_LensFlare( FBO_t* hdrFbo, ivec4_t hdrBox, FBO_t* ldrFbo, ivec4_t ldrBox )
{
    vec4_t color;
    
    // bloom
    color[0] =
        color[1] =
            color[2] = pow( 2, r_cameraExposure->value );
    color[3] = 1.0f;
    
    GLSL_BindProgram( &tr.lensflareShader );
    
    GL_BindToTMU( tr.fixedLevelsImage, TB_LEVELSMAP );
    
    {
        vec2_t screensize;
        screensize[0] = glConfig.vidWidth;
        screensize[1] = glConfig.vidHeight;
        
        GLSL_SetUniformVec2( &tr.lensflareShader, UNIFORM_DIMENSIONS, screensize );
    }
    
    FBO_Blit( hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.lensflareShader, color, 0 );
}


void RB_MultiPost( FBO_t* hdrFbo, ivec4_t hdrBox, FBO_t* ldrFbo, ivec4_t ldrBox )
{
    vec4_t color;
    
    // bloom
    color[0] =
        color[1] =
            color[2] = pow( 2, r_cameraExposure->value );
    color[3] = 1.0f;
    
    GLSL_BindProgram( &tr.multipostShader );
    
    GL_BindToTMU( tr.fixedLevelsImage, TB_LEVELSMAP );
    
    {
        vec2_t screensize;
        screensize[0] = glConfig.vidWidth;
        screensize[1] = glConfig.vidHeight;
        
        GLSL_SetUniformVec2( &tr.multipostShader, UNIFORM_DIMENSIONS, screensize );
    }
    
    FBO_Blit( hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.multipostShader, color, 0 );
}

void RB_VolumetricDLight( FBO_t* hdrFbo, ivec4_t hdrBox, FBO_t* ldrFbo, ivec4_t ldrBox )
{
    vec4_t color;
    
    // bloom
    color[0] =
        color[1] =
            color[2] = pow( 2, r_cameraExposure->value );
    color[3] = 1.0f;
    
    if( !backEnd.refdef.num_dlights )
    {
        return;
    }
    
    for( S32 l = 0; l < backEnd.refdef.num_dlights; l++ )
    {
        dlight_t*	dl = &backEnd.refdef.dlights[l];
        
        GLSL_BindProgram( &tr.volumelightShader );
        
        GL_BindToTMU( tr.fixedLevelsImage, TB_LEVELSMAP );
        
        /*
        matrix_t    matrix;
        Matrix16Identity(matrix);
        GL_SetModelviewMatrix(matrix);
        Matrix16Ortho( backEnd.viewParms.viewportX, backEnd.viewParms.viewportX + backEnd.viewParms.viewportWidth,
        backEnd.viewParms.viewportY, backEnd.viewParms.viewportY + backEnd.viewParms.viewportHeight,
        -99999, 99999, matrix );
        GL_SetProjectionMatrix(matrix);
        
        GLSL_SetUniformMat4(&tr.volumelightShader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);
        GLSL_SetUniformMat4(&tr.volumelightShader, UNIFORM_MODELMATRIX, backEnd.orientation.transformMatrix);
        */
        
        
        GL_SetModelviewMatrix( backEnd.viewParms.orientation.modelMatrix );
        GL_SetProjectionMatrix( backEnd.viewParms.projectionMatrix );
        
        
        GLSL_SetUniformMat4( &tr.volumelightShader, UNIFORM_MODELVIEWPROJECTIONMATRIX, backEnd.viewParms.projectionMatrix );
        GLSL_SetUniformMat4( &tr.volumelightShader, UNIFORM_MODELMATRIX, backEnd.viewParms.orientation.modelMatrix );
        
        {
            vec2_t screensize;
            screensize[0] = glConfig.vidWidth;
            screensize[1] = glConfig.vidHeight;
            
            GLSL_SetUniformVec2( &tr.volumelightShader, UNIFORM_DIMENSIONS, screensize );
        }
        
        {
            vec4_t local0;
            local0[0] = dl->origin[0];
            local0[1] = dl->origin[1];
            local0[2] = dl->origin[2];
            local0[3] = 0.0;
            
            GLSL_SetUniformVec4( &tr.volumelightShader, UNIFORM_LOCAL0, local0 );
        }
        
        {
            vec4_t local1;
            local1[0] = dl->color[0];
            local1[1] = dl->color[1];
            local1[2] = dl->color[2];
            local1[3] = 0.0;
            
            GLSL_SetUniformVec4( &tr.volumelightShader, UNIFORM_LOCAL1, local1 );
        }
        
        FBO_Blit( hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.volumelightShader, color, 0 );
    }
}

void RB_HDR( FBO_t* hdrFbo, ivec4_t hdrBox, FBO_t* ldrFbo, ivec4_t ldrBox )
{
    vec4_t color;
    
    // bloom
    color[0] =
        color[1] =
            color[2] = pow( 2, r_cameraExposure->value );
    color[3] = 1.0f;
    
    GLSL_BindProgram( &tr.hdrShader );
    
    GL_BindToTMU( tr.fixedLevelsImage, TB_LEVELSMAP );
    
    {
        vec4_t viewInfo;
        
        float zmax = backEnd.viewParms.zFar;
        float zmin = r_znear->value;
        
        VectorSet4( viewInfo, zmax / zmin, zmax, 0.0, 0.0 );
        //VectorSet4(viewInfo, zmin, zmax, 0.0, 0.0);
        
        GLSL_SetUniformVec4( &tr.hdrShader, UNIFORM_VIEWINFO, viewInfo );
    }
    
    {
        vec2_t screensize;
        screensize[0] = glConfig.vidWidth;
        screensize[1] = glConfig.vidHeight;
        
        GLSL_SetUniformVec2( &tr.hdrShader, UNIFORM_DIMENSIONS, screensize );
        
        //CL_RefPrintf(PRINT_WARNING, "Sent dimensions %f %f.\n", screensize[0], screensize[1]);
    }
    
    FBO_Blit( hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.hdrShader, color, GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA );
}

void RB_Anaglyph( FBO_t* hdrFbo, ivec4_t hdrBox, FBO_t* ldrFbo, ivec4_t ldrBox )
{
    vec4_t color;
    
    // bloom
    color[0] =
        color[1] =
            color[2] = pow( 2, r_cameraExposure->value );
    color[3] = 1.0f;
    
    GLSL_BindProgram( &tr.anaglyphShader );
    
    GL_BindToTMU( tr.fixedLevelsImage, TB_LEVELSMAP );
    
    //qglUseProgramObjectARB(tr.fakedepthShader.program);
    
    {
        vec4_t viewInfo;
        
        float zmax = backEnd.viewParms.zFar;
        float zmin = r_znear->value;
        
        VectorSet4( viewInfo, zmax / zmin, zmax, 0.0, 0.0 );
        //VectorSet4(viewInfo, zmin, zmax, 0.0, 0.0);
        
        GLSL_SetUniformVec4( &tr.anaglyphShader, UNIFORM_VIEWINFO, viewInfo );
    }
    
    {
        vec2_t screensize;
        screensize[0] = glConfig.vidWidth;
        screensize[1] = glConfig.vidHeight;
        
        GLSL_SetUniformVec2( &tr.anaglyphShader, UNIFORM_DIMENSIONS, screensize );
        
        //CL_RefPrintf(PRINT_WARNING, "Sent dimensions %f %f.\n", screensize[0], screensize[1]);
    }
    
    {
        vec4_t local0;
        VectorSet4( local0, r_trueAnaglyphSeparation->value, r_trueAnaglyphRed->value, r_trueAnaglyphGreen->value, r_trueAnaglyphBlue->value );
        GLSL_SetUniformVec4( &tr.anaglyphShader, UNIFORM_LOCAL0, local0 );
    }
    
    FBO_Blit( hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.anaglyphShader, color, GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA );
}

void RB_TextureClean( FBO_t* hdrFbo, ivec4_t hdrBox, FBO_t* ldrFbo, ivec4_t ldrBox )
{
    vec4_t color;
    
    color[0] =
        color[1] =
            color[2] = pow( 2, r_cameraExposure->value );
    color[3] = 1.0f;
    
    GLSL_BindProgram( &tr.texturecleanShader );
    
    GL_BindToTMU( tr.fixedLevelsImage, TB_LEVELSMAP );
    
    {
        vec4_t viewInfo;
        
        float zmax = backEnd.viewParms.zFar;
        float zmin = r_znear->value;
        
        VectorSet4( viewInfo, zmax / zmin, zmax, 0.0, 0.0 );
        //VectorSet4(viewInfo, zmin, zmax, 0.0, 0.0);
        
        GLSL_SetUniformVec4( &tr.texturecleanShader, UNIFORM_VIEWINFO, viewInfo );
    }
    
    {
        vec2_t screensize;
        screensize[0] = glConfig.vidWidth;
        screensize[1] = glConfig.vidHeight;
        
        GLSL_SetUniformVec2( &tr.texturecleanShader, UNIFORM_DIMENSIONS, screensize );
        
        //CL_RefPrintf(PRINT_WARNING, "Sent dimensions %f %f.\n", screensize[0], screensize[1]);
    }
    
    /*
    cvar_t  *r_textureCleanSigma;
    cvar_t  *r_textureCleanBSigma;
    cvar_t  *r_textureCleanMSize;
    */
    {
        vec4_t local0;
        VectorSet4( local0, r_textureCleanSigma->value, r_textureCleanBSigma->value, r_textureCleanMSize->value, 0 );
        GLSL_SetUniformVec4( &tr.texturecleanShader, UNIFORM_LOCAL0, local0 );
    }
    
    //FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.texturecleanShader, color, GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA);
    FBO_Blit( hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.texturecleanShader, color, 0 );
}

void RB_ESharpening( FBO_t* hdrFbo, ivec4_t hdrBox, FBO_t* ldrFbo, ivec4_t ldrBox )
{
    vec4_t color;
    
    // bloom
    color[0] =
        color[1] =
            color[2] = pow( 2, r_cameraExposure->value );
    color[3] = 1.0f;
    
    GLSL_BindProgram( &tr.esharpeningShader );
    
    GL_BindToTMU( tr.fixedLevelsImage, TB_LEVELSMAP );
    
    //qglUseProgramObjectARB(tr.esharpeningShader.program);
    
    {
        vec4_t viewInfo;
        
        float zmax = backEnd.viewParms.zFar;
        float zmin = r_znear->value;
        
        VectorSet4( viewInfo, zmax / zmin, zmax, 0.0, 0.0 );
        //VectorSet4(viewInfo, zmin, zmax, 0.0, 0.0);
        
        GLSL_SetUniformVec4( &tr.esharpeningShader, UNIFORM_VIEWINFO, viewInfo );
    }
    
    {
        vec2_t screensize;
        screensize[0] = glConfig.vidWidth;
        screensize[1] = glConfig.vidHeight;
        
        GLSL_SetUniformVec2( &tr.esharpeningShader, UNIFORM_DIMENSIONS, screensize );
        
        //CL_RefPrintf(PRINT_WARNING, "Sent dimensions %f %f.\n", screensize[0], screensize[1]);
    }
    
    //{
    //	vec4_t local0;
    //	VectorSet4(local0, r_textureCleanSigma->value, r_textureCleanBSigma->value, 0, 0);
    //	GLSL_SetUniformVec4(&tr.texturecleanShader, UNIFORM_LOCAL0, local0);
    //}
    
    //FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.esharpeningShader, color, GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA);
    FBO_Blit( hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.esharpeningShader, color, 0 );
}


void RB_ESharpening2( FBO_t* hdrFbo, ivec4_t hdrBox, FBO_t* ldrFbo, ivec4_t ldrBox )
{
    vec4_t color;
    
    // bloom
    color[0] =
        color[1] =
            color[2] = pow( 2, r_cameraExposure->value );
    color[3] = 1.0f;
    
    GLSL_BindProgram( &tr.esharpening2Shader );
    
    GL_BindToTMU( tr.fixedLevelsImage, TB_LEVELSMAP );
    
    {
        vec4_t viewInfo;
        
        float zmax = backEnd.viewParms.zFar;
        float zmin = r_znear->value;
        
        VectorSet4( viewInfo, zmax / zmin, zmax, 0.0, 0.0 );
        
        GLSL_SetUniformVec4( &tr.esharpening2Shader, UNIFORM_VIEWINFO, viewInfo );
    }
    
    {
        vec2_t screensize;
        screensize[0] = glConfig.vidWidth;
        screensize[1] = glConfig.vidHeight;
        
        GLSL_SetUniformVec2( &tr.esharpening2Shader, UNIFORM_DIMENSIONS, screensize );
    }
    
    FBO_Blit( hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.esharpeningShader, color, 0 );
}


void RB_DOF( FBO_t* hdrFbo, ivec4_t hdrBox, FBO_t* ldrFbo, ivec4_t ldrBox )
{
    vec4_t color;
    
    // bloom
    color[0] =
        color[1] =
            color[2] = pow( 2, r_cameraExposure->value );
    color[3] = 1.0f;
    
    GLSL_BindProgram( &tr.dofShader );
    
    GL_BindToTMU( tr.fixedLevelsImage, TB_LEVELSMAP );
    
    GLSL_SetUniformInt( &tr.dofShader, UNIFORM_LEVELSMAP, TB_LEVELSMAP );
    GLSL_SetUniformInt( &tr.dofShader, UNIFORM_SCREENDEPTHMAP, TB_LIGHTMAP );
    GL_BindToTMU( tr.renderDepthImage, TB_LIGHTMAP );
    
    {
        vec2_t screensize;
        screensize[0] = glConfig.vidWidth;
        screensize[1] = glConfig.vidHeight;
        
        GLSL_SetUniformVec2( &tr.dofShader, UNIFORM_DIMENSIONS, screensize );
    }
    
    {
        vec4_t viewInfo;
        
        float zmax = backEnd.viewParms.zFar;
        float zmin = r_znear->value;
        
        VectorSet4( viewInfo, zmin, zmax, zmax / zmin, 0.0 );
        
        GLSL_SetUniformVec4( &tr.dofShader, UNIFORM_VIEWINFO, viewInfo );
    }
    
    {
        vec4_t info;
        
        info[0] = r_dof->value;
        info[1] = 0.0;//r_testvalue0->value;
        info[2] = 0.0;
        info[3] = 0.0;
        
        VectorSet4( info, info[0], info[1], info[2], info[3] );
        
        GLSL_SetUniformVec4( &tr.dofShader, UNIFORM_LOCAL0, info );
    }
    
    FBO_Blit( hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.dofShader, color, GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA );
}

void RB_Vibrancy( FBO_t* hdrFbo, ivec4_t hdrBox, FBO_t* ldrFbo, ivec4_t ldrBox )
{
    vec4_t color;
    
    // bloom
    color[0] =
        color[1] =
            color[2] = pow( 2, r_cameraExposure->value );
    color[3] = 1.0f;
    
    GLSL_BindProgram( &tr.vibrancyShader );
    
    GL_BindToTMU( tr.fixedLevelsImage, TB_LEVELSMAP );
    
    GLSL_SetUniformInt( &tr.vibrancyShader, UNIFORM_LEVELSMAP, TB_LEVELSMAP );
    
    {
        vec4_t info;
        
        info[0] = r_vibrancy->value;
        info[1] = 0.0;
        info[2] = 0.0;
        info[3] = 0.0;
        
        VectorSet4( info, info[0], info[1], info[2], info[3] );
        
        GLSL_SetUniformVec4( &tr.vibrancyShader, UNIFORM_LOCAL0, info );
    }
    
    FBO_Blit( hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.vibrancyShader, color, GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA );
}
