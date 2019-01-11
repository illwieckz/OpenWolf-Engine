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
// Version:     v1.01
// Created:
// Compilers:   Visual Studio 2017, gcc 7.3.0
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
            color[2] = pow( 2, r_cameraExposure->value - autoExposure ); //exp2(r_cameraExposure->value);
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


bool RB_UpdateSunFlareVis( void )
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
            qglGetQueryObjectiv( tr.sunFlareQuery[tr.sunFlareQueryIndex], GL_QUERY_RESULT_AVAILABLE, &available );
            if( available )
                break;
        }
        
        CL_RefPrintf( PRINT_DEVELOPER, "Waited %d iterations\n", iter );
    }
    
    qglGetQueryObjectuiv( tr.sunFlareQuery[tr.sunFlareQueryIndex], GL_QUERY_RESULT, &sampleCount );
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

void RB_GaussianBlur( FBO_t* srcFbo, FBO_t* intermediateFbo, FBO_t* dstFbo, F32 spread )
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
    
    GLSL_SetUniformInt( &tr.darkexpandShader, UNIFORM_LEVELSMAP, TB_LEVELSMAP );
    GL_BindToTMU( tr.fixedLevelsImage, TB_LEVELSMAP );
    
    {
        vec2_t screensize;
        screensize[0] = glConfig.vidWidth;
        screensize[1] = glConfig.vidHeight;
        
        GLSL_SetUniformVec2( &tr.darkexpandShader, UNIFORM_DIMENSIONS, screensize );
    }
    
    FBO_Blit( hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.darkexpandShader, color, 0 );
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
    
    halfBox[0] = backEnd.viewParms.viewportX      * tr.anamorphicRenderFBOImage[0]->width / ( F32 )glConfig.vidWidth;
    halfBox[1] = backEnd.viewParms.viewportY      * tr.anamorphicRenderFBOImage[0]->height / ( F32 )glConfig.vidHeight;
    halfBox[2] = backEnd.viewParms.viewportWidth  * tr.anamorphicRenderFBOImage[0]->width / ( F32 )glConfig.vidWidth;
    halfBox[3] = backEnd.viewParms.viewportHeight * tr.anamorphicRenderFBOImage[0]->height / ( F32 )glConfig.vidHeight;
    
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
        VectorSet4( local0, 256.0, 0.0, 0.0, 0.0 );
        GLSL_SetUniformVec4( &tr.anamorphicDarkenShader, UNIFORM_LOCAL0, local0 );
    }
    
    FBO_Blit( hdrFbo, NULL, texHalfScale, tr.anamorphicRenderFBO[1], NULL, &tr.anamorphicDarkenShader, color, 0 );
    FBO_FastBlit( tr.anamorphicRenderFBO[1], NULL, tr.anamorphicRenderFBO[0], NULL, GL_COLOR_BUFFER_BIT, GL_LINEAR );
    
    //
    // Blur the new darken'ed VBO...
    //
    
    for( S32 i = 0; i < r_bloomPasses->integer; i++ )
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
                //VectorSet4(local0, (F32)width, 0.0, 0.0, 0.0);
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
        VectorSet4( local0, 1.0, 0.0, 0.0, 0.0 );
        GLSL_SetUniformVec4( &tr.anamorphicCombineShader, UNIFORM_LOCAL0, local0 );
    }
    
    FBO_Blit( hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.anamorphicCombineShader, color, GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA );
    
    //
    // Render the results now...
    //
    
    FBO_FastBlit( ldrFbo, NULL, hdrFbo, NULL, GL_COLOR_BUFFER_BIT, GL_LINEAR );
}

void RB_BloomRays( FBO_t* hdrFbo, ivec4_t hdrBox, FBO_t* ldrFbo, ivec4_t ldrBox )
{
    vec4_t color;
    
    // bloom
    color[0] =
        color[1] =
            color[2] = pow( 2, r_cameraExposure->value );
    color[3] = 1.0f;
    
    F32 glowDimensionsX, glowDimensionsY;
    
    // Use the most blurred version of glow...
    if( r_anamorphic->integer )
    {
        // We have a previously blurred glow map...
        GLSL_BindProgram( &tr.bloomRaysShader );
        
        GLSL_SetUniformInt( &tr.bloomRaysShader, UNIFORM_LEVELSMAP, TB_LEVELSMAP );
        GL_BindToTMU( hdrFbo->colorImage[0], TB_LEVELSMAP );
        
        GLSL_SetUniformInt( &tr.bloomRaysShader, UNIFORM_GLOWMAP, TB_GLOWMAP );
        
        GL_BindToTMU( tr.anamorphicRenderFBOImage[0], TB_GLOWMAP );
        glowDimensionsX = tr.anamorphicRenderFBOImage[0]->width;
        glowDimensionsY = tr.anamorphicRenderFBOImage[0]->height;
    }
    else
    {
        // We need to blur the glow map...
        //
        // Bloom X axis... (to VBO)
        //
        
        GLSL_BindProgram( &tr.anamorphicBlurShader );
        
        GLSL_SetUniformInt( &tr.anamorphicBlurShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP );
        GL_BindToTMU( tr.glowFboScaled[0]->colorImage[0], TB_DIFFUSEMAP );
        
        {
            vec2_t screensize;
            screensize[0] = tr.anamorphicRenderFBOImage[0]->width;
            screensize[1] = tr.anamorphicRenderFBOImage[0]->height;
            
            GLSL_SetUniformVec2( &tr.anamorphicBlurShader, UNIFORM_DIMENSIONS, screensize );
        }
        
        {
            vec4_t local0;
            VectorSet4( local0, 1.0, 0.0, 16.0, 0.0 );
            GLSL_SetUniformVec4( &tr.anamorphicBlurShader, UNIFORM_LOCAL0, local0 );
        }
        
        FBO_Blit( tr.glowFboScaled[0], hdrBox, NULL, tr.anamorphicRenderFBO[0], NULL, &tr.anamorphicBlurShader, color, 0 );
        
        
        GLSL_BindProgram( &tr.bloomRaysShader );
        
        GLSL_SetUniformInt( &tr.bloomRaysShader, UNIFORM_LEVELSMAP, TB_LEVELSMAP );
        GL_BindToTMU( hdrFbo->colorImage[0], TB_LEVELSMAP );
        
        GLSL_SetUniformInt( &tr.bloomRaysShader, UNIFORM_GLOWMAP, TB_GLOWMAP );
        
        GL_BindToTMU( tr.anamorphicRenderFBOImage[0], TB_GLOWMAP );
        glowDimensionsX = tr.anamorphicRenderFBOImage[0]->width;
        glowDimensionsY = tr.anamorphicRenderFBOImage[0]->height;
    }
    
    GLSL_SetUniformMat4( &tr.bloomRaysShader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection );
    
    {
        vec2_t dimensions;
        dimensions[0] = glowDimensionsX;
        dimensions[1] = glowDimensionsY;
        
        GLSL_SetUniformVec2( &tr.bloomRaysShader, UNIFORM_DIMENSIONS, dimensions );
    }
    
    {
        vec4_t viewInfo;
        F32 zmax = 2048.0;//3072.0;//backEnd.viewParms.zFar;
        F32 ymax = zmax * tan( backEnd.viewParms.fovY * M_PI / 360.0f );
        F32 xmax = zmax * tan( backEnd.viewParms.fovX * M_PI / 360.0f );
        F32 zmin = r_znear->value;
        VectorSet4( viewInfo, zmin, zmax, zmax / zmin, 0.0 );
        GLSL_SetUniformVec4( &tr.bloomRaysShader, UNIFORM_VIEWINFO, viewInfo );
    }
    
    {
        vec4_t local1;
        VectorSet4( local1, r_bloomRaysDecay->value, r_bloomRaysWeight->value, r_bloomRaysDensity->value, 0.0 );
        GLSL_SetUniformVec4( &tr.bloomRaysShader, UNIFORM_LOCAL1, local1 );
    }
    
    FBO_Blit( hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.bloomRaysShader, color, 0 );
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
    
    vec2_t screensize;
    screensize[0] = glConfig.vidWidth;
    screensize[1] = glConfig.vidHeight;
    
    GLSL_SetUniformVec2( &tr.lensflareShader, UNIFORM_DIMENSIONS, screensize );
    
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

void TR_AxisToAngles( const vec3_t axis[3], vec3_t angles )
{
    vec3_t right;
    
    // vec3_origin is the origin (0, 0, 0).
    VectorSubtract( vec3_origin, axis[1], right );
    
    if( axis[0][2] > 0.999f )
    {
        angles[PITCH] = -90.0f;
        angles[YAW] = RAD2DEG( atan2f( -right[0], right[1] ) );
        angles[ROLL] = 0.0f;
    }
    else if( axis[0][2] < -0.999f )
    {
        angles[PITCH] = 90.0f;
        angles[YAW] = RAD2DEG( atan2f( -right[0], right[1] ) );
        angles[ROLL] = 0.0f;
    }
    else
    {
        angles[PITCH] = RAD2DEG( asinf( -axis[0][2] ) );
        angles[YAW] = RAD2DEG( atan2f( axis[0][1], axis[0][0] ) );
        angles[ROLL] = RAD2DEG( atan2f( -right[2], axis[2][2] ) );
    }
}

bool TR_WorldToScreen( vec3_t worldCoord, F32* x, F32* y )
{
    S32	xcenter, ycenter;
    vec3_t local, transformed;
    vec3_t vfwd, vright, vup, viewAngles;
    
    TR_AxisToAngles( backEnd.refdef.viewaxis, viewAngles );
    
    //NOTE: did it this way because most draw functions expect virtual 640x480 coords
    //	and adjust them for current resolution
    xcenter = 640 / 2;//gives screen coords in virtual 640x480, to be adjusted when drawn
    ycenter = 480 / 2;//gives screen coords in virtual 640x480, to be adjusted when drawn
    
    VectorSubtract( worldCoord, backEnd.refdef.vieworg, local );
    
    AngleVectors( viewAngles, vfwd, vright, vup );
    
    transformed[0] = DotProduct( local, vright );
    transformed[1] = DotProduct( local, vup );
    transformed[2] = DotProduct( local, vfwd );
    
    // Make sure Z is not negative.
    if( transformed[2] < 0.01 )
    {
        //return false;
        //transformed[2] = 2.0 - transformed[2];
    }
    // Simple convert to screen coords.
    F32 xzi = xcenter / transformed[2] * ( 90.0 / backEnd.refdef.fov_x );
    F32 yzi = ycenter / transformed[2] * ( 90.0 / backEnd.refdef.fov_y );
    
    *x = ( xcenter + xzi * transformed[0] );
    *y = ( ycenter - yzi * transformed[1] );
    
    return true;
}

bool TR_InFOV( vec3_t spot, vec3_t from )
{
    return true;
    
    vec3_t	deltaVector, angles, deltaAngles;
    vec3_t	fromAnglesCopy;
    vec3_t	fromAngles;
    S32 hFOV = backEnd.refdef.fov_x * 0.5;
    S32 vFOV = backEnd.refdef.fov_y * 0.5;
    
    TR_AxisToAngles( backEnd.refdef.viewaxis, fromAngles );
    
    VectorSubtract( spot, from, deltaVector );
    vectoangles( deltaVector, angles );
    VectorCopy( fromAngles, fromAnglesCopy );
    
    deltaAngles[PITCH] = AngleDelta( fromAnglesCopy[PITCH], angles[PITCH] );
    deltaAngles[YAW] = AngleDelta( fromAnglesCopy[YAW], angles[YAW] );
    
    if( fabs( deltaAngles[PITCH] ) <= vFOV && fabs( deltaAngles[YAW] ) <= hFOV )
    {
        return true;
    }
    
    return false;
}

void Volumetric_Trace( trace_t* results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, const S32 passEntityNum, const S32 contentmask )
{
    results->entityNum = ENTITYNUM_NONE;
    collisionModelManager->BoxTrace( results, start, end, mins, maxs, 0, contentmask, TT_AABB );
    results->entityNum = results->fraction != 1.0 ? ENTITYNUM_WORLD : ENTITYNUM_NONE;
}

bool Volumetric_Visible( vec3_t from, vec3_t to, bool isSun )
{
    trace_t trace;
    
    if( isSun )
        return true;
        
    Volumetric_Trace( &trace, from, NULL, NULL, to, -1, CONTENTS_SOLID );
    
    //if (trace.fraction != 1.0 && Distance(trace.endpos, to) > 64)
    if( trace.fraction < 0.7 )
    {
        return false;
    }
    
    return true;
}

vec3_t roof;

void Volumetric_RoofHeight( vec3_t from )
{
    trace_t trace;
    vec3_t roofh;
    VectorSet( roofh, from[0] + 192, from[1], from[2] );
    Volumetric_Trace( &trace, from, NULL, NULL, roofh, -1, CONTENTS_SOLID );
    VectorSet( roof, trace.endpos[0] - 8.0, trace.endpos[1], trace.endpos[2] );
}

void WorldCoordToScreenCoord( vec3_t origin, F32* x, F32* y )
{
#if 1
    S32	xcenter, ycenter;
    vec3_t	local, transformed;
    vec3_t	vfwd, vright, vup, viewAngles;
    
    TR_AxisToAngles( backEnd.refdef.viewaxis, viewAngles );
    
    //NOTE: did it this way because most draw functions expect virtual 640x480 coords
    //	and adjust them for current resolution
    xcenter = glConfig.vidWidth/*640*/ / 2;//gives screen coords in virtual 640x480, to be adjusted when drawn
    ycenter = glConfig.vidHeight/*480*/ / 2;//gives screen coords in virtual 640x480, to be adjusted when drawn
    
    VectorSubtract( origin, backEnd.refdef.vieworg, local );
    
    AngleVectors( viewAngles, vfwd, vright, vup );
    
    transformed[0] = DotProduct( local, vright );
    transformed[1] = DotProduct( local, vup );
    transformed[2] = DotProduct( local, vfwd );
    
    // Make sure Z is not negative.
    if( transformed[2] < 0.01 )
    {
        *x = -1;
        *y = -1;
        return;
        //transformed[2] = 2.0 - transformed[2];
    }
    
    // Simple convert to screen coords.
    F32 xzi = xcenter / transformed[2] * ( 95.0/*r_testvalue0->value*//*90.0*/ / backEnd.refdef.fov_x ); // Dunno whats going on here. refdef inaccurate???!?!?!?
    F32 yzi = ycenter / transformed[2] * ( 106.0/*r_testvalue0->value*//*90.0*/ / backEnd.refdef.fov_y ); // Dunno whats going on here. refdef inaccurate???!?!?!?
    
    *x = ( xcenter + xzi * transformed[0] );
    *y = ( ycenter - yzi * transformed[1] );
    
    *x = *x / glConfig.vidWidth/*640*/;
    *y = 1.0 - ( *y / glConfig.vidHeight/*480*/ );
#else // this just sucks...
    vec4_t pos, hpos;
    VectorSet4( pos, origin[0], origin[1], origin[2], 1.0 );
    
    // project sun point
    Mat4Transform( glState.modelviewProjection, pos, hpos );
    
    // transform to UV coords
    hpos[3] = 0.5f / hpos[3];
    
    pos[0] = 0.5f + hpos[0] * hpos[3];
    pos[1] = 0.5f + hpos[1] * hpos[3];
    
    *x = pos[0];
    *y = pos[1];
#endif
}

extern vec3_t SUN_POSITION;
extern vec2_t SUN_SCREEN_POSITION;
extern bool SUN_VISIBLE;

bool RB_VolumetricLight( FBO_t* hdrFbo, ivec4_t hdrBox, FBO_t* ldrFbo, ivec4_t ldrBox )
{
    vec4_t color;
    S32 NUM_VISIBLE_LIGHTS = 0;
    S32 SUN_ID = -1;
    
    // bloom
    color[0] =
        color[1] =
            color[2] = pow( 2, r_cameraExposure->value );
    color[3] = 1.0f;
    
    //#ifndef USING_ENGINE_GLOW_LIGHTCOLORS_SEARCH
    //RB_AddGlowShaderLights();
    //#endif //USING_ENGINE_GLOW_LIGHTCOLORS_SEARCH
    
    //	backEnd.refdef.num_dlights = r_numdlights; // This is being disabled somewhere...
    //CL_RefPrint(PRINT_WARNING, "VLIGHT DEBUG: %i dlights.\n", backEnd.refdef.num_dlights);
    //CL_RefPrint(PRINT_WARNING, "VLIGHT DEBUG: %i glow positions.\n", NUM_MAP_GLOW_LOCATIONS);
    
    //CL_RefPrint(PRINT_WARNING, "VLIGHT GLOWS DEBUG: %i dlights.\n", backEnd.refdef.num_dlights);
    /*
    if ( !backEnd.refdef.num_dlights && !SUN_VISIBLE ) {
    //CL_RefPrint(PRINT_WARNING, "VLIGHT DEBUG: 0 dlights.\n");
    return qfalse;
    }*/
    
    //
    // UQ1: Now going to allow a maximum of MAX_VOLUMETRIC_LIGHTS volumetric lights on screen for FPS...
    //
    S32			NUM_CLOSE_VLIGHTS = 0;
    S32			CLOSEST_VLIGHTS[MAX_VOLUMETRIC_LIGHTS] = { 0 };
    vec2_t		CLOSEST_VLIGHTS_POSITIONS[MAX_VOLUMETRIC_LIGHTS] = { 0 };
    F32		CLOSEST_VLIGHTS_DISTANCES[MAX_VOLUMETRIC_LIGHTS] = { 0 };
    vec3_t		CLOSEST_VLIGHTS_COLORS[MAX_VOLUMETRIC_LIGHTS] = { 0 };
    
    F32 strengthMult = 1.0;
    if( r_volumelight->integer < 3 )
        strengthMult = 2.0; // because the lower samples result in less color...
        
    for( S32 l = 0; l < backEnd.refdef.num_dlights; l++ )
    {
        dlight_t*	dl = &backEnd.refdef.dlights[l];
        
        F32 x, y, distance;
        
        distance = Distance( backEnd.refdef.vieworg, dl->origin );
        
#if 0
        if( !TR_InFOV( dl->origin, backEnd.refdef.vieworg ) )
        {
            continue; // not on screen...
        }
        
        
        if( distance > 4096.0 )
        {
            continue;
        }
#endif
        
        WorldCoordToScreenCoord( dl->origin, &x, &y );
        
        if( x < 0.0 || y < 0.0 || x > 1.0 || y > 1.0 )
            continue;
            
#ifdef __VOLUME_LIGHT_TRACE__
        if( !Volumetric_Visible( backEnd.refdef.vieworg, dl->origin, qfalse ) )
        {
            // Trace to actual position failed... Try above...
            vec3_t tmpOrg;
            vec3_t eyeOrg;
            vec3_t tmpRoof;
            vec3_t eyeRoof;
            
            // Calculate ceiling heights at both positions...
            Volumetric_RoofHeight( dl->origin );
            VectorCopy( VOLUMETRIC_ROOF, tmpRoof );
            Volumetric_RoofHeight( backEnd.refdef.vieworg );
            VectorCopy( VOLUMETRIC_ROOF, eyeRoof );
            
            VectorSet( tmpOrg, tmpRoof[0], dl->origin[1], dl->origin[2] );
            VectorSet( eyeOrg, backEnd.refdef.vieworg[0], backEnd.refdef.vieworg[1], backEnd.refdef.vieworg[2] );
            if( !Volumetric_Visible( eyeOrg, tmpOrg, qfalse ) )
            {
                // Trace to above position failed... Try trace from above viewer...
                VectorSet( tmpOrg, dl->origin[0], dl->origin[1], dl->origin[2] );
                VectorSet( eyeOrg, eyeRoof[0], backEnd.refdef.vieworg[1], backEnd.refdef.vieworg[2] );
                if( !Volumetric_Visible( eyeOrg, tmpOrg, qfalse ) )
                {
                    // Trace from above viewer failed... Try trace from above, to above...
                    VectorSet( tmpOrg, tmpRoof[0], dl->origin[1], dl->origin[2] );
                    VectorSet( eyeOrg, eyeRoof[0], backEnd.refdef.vieworg[1], backEnd.refdef.vieworg[2] );
                    if( !Volumetric_Visible( eyeOrg, tmpOrg, qfalse ) )
                    {
                        // Trace from/to above viewer failed...
                        continue; // Can't see this...
                    }
                }
            }
        }
#endif //__VOLUME_LIGHT_TRACE__
        
        F32 depth = ( distance / 4096.0 );
        if( depth > 1.0 ) depth = 1.0;
        
        if( NUM_CLOSE_VLIGHTS < MAX_VOLUMETRIC_LIGHTS - 1 )
        {
            // Have free light slots for a new light...
            CLOSEST_VLIGHTS[NUM_CLOSE_VLIGHTS] = l;
            CLOSEST_VLIGHTS_POSITIONS[NUM_CLOSE_VLIGHTS][0] = x;
            CLOSEST_VLIGHTS_POSITIONS[NUM_CLOSE_VLIGHTS][1] = y;
            CLOSEST_VLIGHTS_DISTANCES[NUM_CLOSE_VLIGHTS] = depth;
            
            CLOSEST_VLIGHTS_COLORS[NUM_CLOSE_VLIGHTS][0] = dl->color[0] * strengthMult;
            CLOSEST_VLIGHTS_COLORS[NUM_CLOSE_VLIGHTS][1] = dl->color[1] * strengthMult;
            CLOSEST_VLIGHTS_COLORS[NUM_CLOSE_VLIGHTS][2] = dl->color[2] * strengthMult;
            
            NUM_CLOSE_VLIGHTS++;
            continue;
        }
        else
        {
            // See if this is closer then one of our other lights...
            S32	farthest_light = 0;
            F32	farthest_distance = 0.0;
            
            for( S32 i = 0; i < NUM_CLOSE_VLIGHTS; i++ )
            {
                // Find the most distance light in our current list to replace, if this new option is closer...
                dlight_t*	thisLight = &backEnd.refdef.dlights[CLOSEST_VLIGHTS[i]];
                F32		dist = Distance( thisLight->origin, backEnd.refdef.vieworg );
                
                if( dist > farthest_distance )
                {
                    // This one is further!
                    farthest_light = i;
                    farthest_distance = dist;
                    //break;
                }
            }
            
            if( Distance( dl->origin, backEnd.refdef.vieworg ) < farthest_distance )
            {
                // This light is closer. Replace this one in our array of closest lights...
                CLOSEST_VLIGHTS[farthest_light] = l;
                CLOSEST_VLIGHTS_POSITIONS[farthest_light][0] = x;
                CLOSEST_VLIGHTS_POSITIONS[farthest_light][1] = y;
                CLOSEST_VLIGHTS_DISTANCES[farthest_light] = depth;
                
                CLOSEST_VLIGHTS_COLORS[farthest_light][0] = dl->color[0] * strengthMult;
                CLOSEST_VLIGHTS_COLORS[farthest_light][1] = dl->color[1] * strengthMult;
                CLOSEST_VLIGHTS_COLORS[farthest_light][2] = dl->color[2] * strengthMult;
            }
        }
    }
    
    /*GLuint sampleCount = 0;
    qglGetQueryObjectuiv(tr.sunFlareVQuery[tr.sunFlareVQueryIndex], GL_QUERY_RESULT, &sampleCount);
    if (sampleCount)
    {
    CL_RefPrint(PRINT_WARNING, "SUN: %d samples.\n", sampleCount);
    SUN_VISIBLE = qtrue;
    }
    else
    {
    CL_RefPrint(PRINT_WARNING, "SUN: %d samples.\n", sampleCount);
    SUN_VISIBLE = qfalse;
    }*/
    
    if( SUN_VISIBLE )
    {
        // Add sun...
        //SUN_SCREEN_POSITION
        if( NUM_CLOSE_VLIGHTS < MAX_VOLUMETRIC_LIGHTS - 1 )
        {
            // Have free light slots for a new light...
            CLOSEST_VLIGHTS_POSITIONS[NUM_CLOSE_VLIGHTS][0] = SUN_SCREEN_POSITION[0];
            CLOSEST_VLIGHTS_POSITIONS[NUM_CLOSE_VLIGHTS][1] = SUN_SCREEN_POSITION[1];
            CLOSEST_VLIGHTS_DISTANCES[NUM_CLOSE_VLIGHTS] = 0.1;
            CLOSEST_VLIGHTS_COLORS[NUM_CLOSE_VLIGHTS][0] = backEnd.refdef.sunCol[0] * strengthMult;
            CLOSEST_VLIGHTS_COLORS[NUM_CLOSE_VLIGHTS][1] = backEnd.refdef.sunCol[1] * strengthMult;
            CLOSEST_VLIGHTS_COLORS[NUM_CLOSE_VLIGHTS][2] = backEnd.refdef.sunCol[2] * strengthMult;
            SUN_ID = NUM_CLOSE_VLIGHTS;
            NUM_CLOSE_VLIGHTS++;
        }
        else
        {
            // See if this is closer then one of our other lights...
            S32 farthest_light = 0;
            F32	farthest_distance = 0.0;
            
            for( S32 i = 0; i < NUM_CLOSE_VLIGHTS; i++ )
            {
                // Find the most distance light in our current list to replace, if this new option is closer...
                dlight_t*	thisLight = &backEnd.refdef.dlights[CLOSEST_VLIGHTS[i]];
                F32		dist = Distance( thisLight->origin, backEnd.refdef.vieworg );
                
                if( dist > farthest_distance )
                {
                    // This one is further!
                    farthest_light = i;
                    farthest_distance = dist;
                    //break;
                }
            }
            
            CLOSEST_VLIGHTS_POSITIONS[farthest_light][0] = SUN_SCREEN_POSITION[0];
            CLOSEST_VLIGHTS_POSITIONS[farthest_light][1] = SUN_SCREEN_POSITION[1];
            CLOSEST_VLIGHTS_DISTANCES[farthest_light] = 0.1;
            CLOSEST_VLIGHTS_COLORS[farthest_light][0] = backEnd.refdef.sunCol[0] * strengthMult;
            CLOSEST_VLIGHTS_COLORS[farthest_light][1] = backEnd.refdef.sunCol[1] * strengthMult;
            CLOSEST_VLIGHTS_COLORS[farthest_light][2] = backEnd.refdef.sunCol[2] * strengthMult;
            SUN_ID = farthest_light;
        }
    }
    
    //CL_RefPrint(PRINT_WARNING, "VLIGHT DEBUG: %i volume lights. Sun id is %i.\n", NUM_CLOSE_VLIGHTS, SUN_ID);
    
    // None to draw...
    if( NUM_CLOSE_VLIGHTS <= 0 )
    {
        //CL_RefPrint(PRINT_WARNING, "0 visible dlights. %i total dlights.\n", backEnd.refdef.num_dlights);
        return false;
    }
    
    
    /*for (S32 i = 0; i < NUM_CLOSE_VLIGHTS; i++)
    {
    CL_RefPrint(PRINT_WARNING, "VLIGHT DEBUG: [%i] %fx%f. Dist %f. Color %f %f %f.\n", i, CLOSEST_VLIGHTS_POSITIONS[i][0], CLOSEST_VLIGHTS_POSITIONS[i][1], CLOSEST_VLIGHTS_DISTANCES[i], CLOSEST_VLIGHTS_COLORS[i][0], CLOSEST_VLIGHTS_COLORS[i][1], CLOSEST_VLIGHTS_COLORS[i][2]);
    }*/
    
    
    S32 dlightShader = r_volumelight->integer - 1;
    
    if( r_volumelight->integer >= 4 )
    {
        dlightShader -= 3;
    }
    
    GLSL_BindProgram( &tr.volumeLightShader[dlightShader] );
    
    GLSL_SetUniformInt( &tr.volumeLightShader[dlightShader], UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP );
    GL_BindToTMU( hdrFbo->colorImage[0], TB_DIFFUSEMAP );
    
    GLSL_SetUniformInt( &tr.volumeLightShader[dlightShader], UNIFORM_SCREENDEPTHMAP, TB_LIGHTMAP );
    GL_BindToTMU( tr.renderDepthImage, TB_LIGHTMAP );
    
    GLSL_SetUniformInt( &tr.volumeLightShader[dlightShader], UNIFORM_GLOWMAP, TB_GLOWMAP );
    
    F32 glowDimensionsX, glowDimensionsY;
    
    // Use the most blurred version of glow...
    if( r_anamorphic->integer )
    {
        GL_BindToTMU( tr.anamorphicRenderFBOImage[0], TB_GLOWMAP );
        glowDimensionsX = tr.anamorphicRenderFBOImage[0]->width;
        glowDimensionsY = tr.anamorphicRenderFBOImage[0]->height;
    }
    else if( r_bloom->integer )
    {
        GL_BindToTMU( tr.bloomRenderFBOImage[0], TB_GLOWMAP );
        glowDimensionsX = tr.bloomRenderFBOImage[0]->width;
        glowDimensionsY = tr.bloomRenderFBOImage[0]->height;
    }
    else
    {
        GL_BindToTMU( tr.glowFboScaled[0]->colorImage[0], TB_GLOWMAP );
        glowDimensionsX = tr.glowFboScaled[0]->colorImage[0]->width;
        glowDimensionsY = tr.glowFboScaled[0]->colorImage[0]->height;
    }
    
    GLSL_SetUniformMat4( &tr.volumeLightShader[dlightShader], UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection );
    
    {
        vec2_t screensize;
        screensize[0] = glConfig.vidWidth;
        screensize[1] = glConfig.vidHeight;
        
        GLSL_SetUniformVec2( &tr.volumeLightShader[dlightShader], UNIFORM_DIMENSIONS, screensize );
    }
    
    {
        vec4_t viewInfo;
        
        //F32 zmax = backEnd.viewParms.zFar;
        F32 zmax = 4096.0;
        F32 zmin = r_znear->value;
        
        VectorSet4( viewInfo, zmin, zmax, zmax / zmin, ( F32 )SUN_ID );
        
        GLSL_SetUniformVec4( &tr.volumeLightShader[dlightShader], UNIFORM_VIEWINFO, viewInfo );
    }
    
    
    {
        vec4_t local0;
        VectorSet4( local0, glowDimensionsX, glowDimensionsY, r_volumeLightStrength->value, r_testvalue0->value );
        GLSL_SetUniformVec4( &tr.volumeLightShader[dlightShader], UNIFORM_LOCAL0, local0 );
    }
    
    
    GLSL_SetUniformInt( &tr.volumeLightShader[dlightShader], UNIFORM_LIGHTCOUNT, NUM_CLOSE_VLIGHTS );
    GLSL_SetUniformVec2x16( &tr.volumeLightShader[dlightShader], UNIFORM_VLIGHTPOSITIONS, CLOSEST_VLIGHTS_POSITIONS, MAX_VOLUMETRIC_LIGHTS );
    GLSL_SetUniformVec3xX( &tr.volumeLightShader[dlightShader], UNIFORM_VLIGHTCOLORS, CLOSEST_VLIGHTS_COLORS, MAX_VOLUMETRIC_LIGHTS );
    //GLSL_SetUniformFloatxX(&tr.volumeLightShader[dlightShader], UNIFORM_VLIGHTDISTANCES, CLOSEST_VLIGHTS_DISTANCES, MAX_VOLUMETRIC_LIGHTS);
    
    //#define VOLUME_LIGHT_DEBUG
    //#define VOLUME_LIGHT_SINGLE_PASS
    
    
#if !defined(VOLUME_LIGHT_DEBUG) && !defined(VOLUME_LIGHT_SINGLE_PASS)
    FBO_Blit( hdrFbo, NULL, NULL, tr.volumetricFbo, NULL, &tr.volumeLightShader[dlightShader], color, 0 );
    
    // Combine render and hbao...
    GLSL_BindProgram( &tr.volumeLightCombineShader );
    
    GLSL_SetUniformMat4( &tr.volumeLightCombineShader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection );
    GLSL_SetUniformMat4( &tr.volumeLightCombineShader, UNIFORM_MODELMATRIX, backEnd.orientation.transformMatrix );
    
    GLSL_SetUniformInt( &tr.volumeLightCombineShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP );
    GL_BindToTMU( hdrFbo->colorImage[0], TB_DIFFUSEMAP );
    
    GLSL_SetUniformInt( &tr.volumeLightCombineShader, UNIFORM_NORMALMAP, TB_NORMALMAP );
    GL_BindToTMU( tr.volumetricFBOImage, TB_NORMALMAP );
    
    vec2_t screensize;
    screensize[0] = tr.volumetricFBOImage->width;// glConfig.vidWidth * r_superSampleMultiplier->value;
    screensize[1] = tr.volumetricFBOImage->height;// glConfig.vidHeight * r_superSampleMultiplier->value;
    GLSL_SetUniformVec2( &tr.volumeLightCombineShader, UNIFORM_DIMENSIONS, screensize );
    
    FBO_Blit( hdrFbo, NULL, NULL, ldrFbo, NULL, &tr.volumeLightCombineShader, color, 0 );
#else //defined(VOLUME_LIGHT_DEBUG) || defined(VOLUME_LIGHT_SINGLE_PASS)
    FBO_Blit( hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.volumeLightShader[dlightShader], color, 0 );
#endif //defined(VOLUME_LIGHT_DEBUG) || defined(VOLUME_LIGHT_SINGLE_PASS)
    
    //CL_RefPrint(PRINT_WARNING, "%i visible dlights. %i total dlights.\n", NUM_CLOSE_VLIGHTS, backEnd.refdef.num_dlights);
    return true;
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
    
    GLSL_SetUniformMat4( &tr.hdrShader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection );
    
    GLSL_SetUniformInt( &tr.hdrShader, UNIFORM_LEVELSMAP, TB_LEVELSMAP );
    GL_BindToTMU( hdrFbo->colorImage[0], TB_LEVELSMAP );
    
    {
        vec2_t screensize;
        screensize[0] = glConfig.vidWidth;
        screensize[1] = glConfig.vidHeight;
        
        GLSL_SetUniformVec2( &tr.hdrShader, UNIFORM_DIMENSIONS, screensize );
    }
    
    FBO_Blit( hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.hdrShader, color, 0 );
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
    
    {
        vec4_t viewInfo;
        
        F32 zmax = backEnd.viewParms.zFar;
        F32 zmin = r_znear->value;
        
        VectorSet4( viewInfo, zmax / zmin, zmax, 0.0, 0.0 );
        
        GLSL_SetUniformVec4( &tr.anaglyphShader, UNIFORM_VIEWINFO, viewInfo );
    }
    
    {
        vec2_t screensize;
        screensize[0] = glConfig.vidWidth;
        screensize[1] = glConfig.vidHeight;
        
        GLSL_SetUniformVec2( &tr.anaglyphShader, UNIFORM_DIMENSIONS, screensize );
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
        vec2_t screensize;
        screensize[0] = glConfig.vidWidth;
        screensize[1] = glConfig.vidHeight;
        
        GLSL_SetUniformVec2( &tr.texturecleanShader, UNIFORM_DIMENSIONS, screensize );
    }
    
    {
        vec4_t local0;
        VectorSet4( local0, r_textureCleanSigma->value, r_textureCleanBSigma->value, r_textureCleanMSize->value, 0 );
        GLSL_SetUniformVec4( &tr.texturecleanShader, UNIFORM_LOCAL0, local0 );
    }
    
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
    
    vec2_t screensize;
    screensize[0] = glConfig.vidWidth;
    screensize[1] = glConfig.vidHeight;
    GLSL_SetUniformVec2( &tr.esharpeningShader, UNIFORM_DIMENSIONS, screensize );
    
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
    
    vec2_t screensize;
    screensize[0] = glConfig.vidWidth;
    screensize[1] = glConfig.vidHeight;
    GLSL_SetUniformVec2( &tr.esharpening2Shader, UNIFORM_DIMENSIONS, screensize );
    
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
        
        F32 zmax = backEnd.viewParms.zFar;
        F32 zmin = r_znear->value;
        
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

void RB_FXAA( FBO_t* hdrFbo, ivec4_t hdrBox, FBO_t* ldrFbo, ivec4_t ldrBox )
{
    vec4_t color;
    
    // bloom
    color[0] =
        color[1] =
            color[2] = pow( 2, r_cameraExposure->value );
    color[3] = 1.0f;
    
    GLSL_BindProgram( &tr.fxaaShader );
    GLSL_SetUniformInt( &tr.fxaaShader, UNIFORM_LEVELSMAP, TB_LEVELSMAP );
    GL_BindToTMU( tr.fixedLevelsImage, TB_LEVELSMAP );
    
    GLSL_SetUniformMat4( &tr.fxaaShader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection );
    
    {
        vec2_t screensize;
        screensize[0] = glConfig.vidWidth;
        screensize[1] = glConfig.vidHeight;
        
        GLSL_SetUniformVec2( &tr.fxaaShader, UNIFORM_DIMENSIONS, screensize );
    }
    
    FBO_Blit( hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.fxaaShader, color, 0 );
}

void RB_Bloom( FBO_t* hdrFbo, ivec4_t hdrBox, FBO_t* ldrFbo, ivec4_t ldrBox )
{
    vec4_t	color;
    ivec4_t halfBox;
    vec2_t	texScale, texHalfScale, texDoubleScale;
    
    texScale[0] = texScale[1] = 1.0f;
    texHalfScale[0] = texHalfScale[1] = texScale[0] / 2.0;
    texDoubleScale[0] = texDoubleScale[1] = texScale[0] * 2.0;
    
    // bloom
    color[0] =
        color[1] =
            color[2] = pow( 2, r_cameraExposure->value );
    color[3] = 1.0f;
    
    halfBox[0] = backEnd.viewParms.viewportX      * tr.bloomRenderFBOImage[0]->width / ( F32 )glConfig.vidWidth;
    halfBox[1] = backEnd.viewParms.viewportY      * tr.bloomRenderFBOImage[0]->height / ( F32 )glConfig.vidHeight;
    halfBox[2] = backEnd.viewParms.viewportWidth  * tr.bloomRenderFBOImage[0]->width / ( F32 )glConfig.vidWidth;
    halfBox[3] = backEnd.viewParms.viewportHeight * tr.bloomRenderFBOImage[0]->height / ( F32 )glConfig.vidHeight;
    
    //
    // Darken to VBO...
    //
    
    GLSL_BindProgram( &tr.bloomDarkenShader );
    
    GL_BindToTMU( tr.fixedLevelsImage, TB_DIFFUSEMAP );
    
    {
        vec2_t screensize;
        screensize[0] = glConfig.vidWidth;
        screensize[1] = glConfig.vidHeight;
        
        GLSL_SetUniformVec2( &tr.bloomDarkenShader, UNIFORM_DIMENSIONS, screensize );
    }
    
    {
        vec4_t local0;
        VectorSet4( local0, r_bloomDarkenPower->value, 0.0, 0.0, 0.0 );
        GLSL_SetUniformVec4( &tr.bloomDarkenShader, UNIFORM_LOCAL0, local0 );
    }
    
    FBO_Blit( hdrFbo, NULL, texHalfScale, tr.bloomRenderFBO[1], NULL, &tr.bloomDarkenShader, color, 0 );
    FBO_FastBlit( tr.bloomRenderFBO[1], NULL, tr.bloomRenderFBO[0], NULL, GL_COLOR_BUFFER_BIT, GL_LINEAR );
    
    //
    // Blur the new darken'ed VBO...
    //
    
    for( S32 i = 0; i < r_bloomPasses->integer; i++ )
    {
#ifdef ___BLOOM_AXIS_UNCOMBINED_SHADER___
        //
        // Bloom X axis... (to VBO 1)
        //
        
        GLSL_BindProgram( &tr.bloomBlurShader );
        
        GL_BindToTMU( tr.bloomRenderFBOImage[0], TB_DIFFUSEMAP );
        
        {
            vec2_t screensize;
            screensize[0] = tr.bloomRenderFBOImage[0]->width;
            screensize[1] = tr.bloomRenderFBOImage[0]->height;
            
            GLSL_SetUniformVec2( &tr.bloomBlurShader, UNIFORM_DIMENSIONS, screensize );
        }
        
        {
            vec4_t local0;
            VectorSet4( local0, 1.0, 0.0, 0.0, 0.0 );
            GLSL_SetUniformVec4( &tr.bloomBlurShader, UNIFORM_LOCAL0, local0 );
        }
        
        FBO_Blit( tr.bloomRenderFBO[0], NULL, NULL, tr.bloomRenderFBO[1], NULL, &tr.bloomBlurShader, color, 0 );
        FBO_FastBlit( tr.bloomRenderFBO[1], NULL, tr.bloomRenderFBO[0], NULL, GL_COLOR_BUFFER_BIT, GL_LINEAR );
        
        //
        // Bloom Y axis... (back to VBO 0)
        //
        
        GLSL_BindProgram( &tr.bloomBlurShader );
        
        GL_BindToTMU( tr.bloomRenderFBOImage[1], TB_DIFFUSEMAP );
        
        {
            vec2_t screensize;
            screensize[0] = tr.bloomRenderFBOImage[1]->width;
            screensize[1] = tr.bloomRenderFBOImage[1]->height;
            
            GLSL_SetUniformVec2( &tr.bloomBlurShader, UNIFORM_DIMENSIONS, screensize );
        }
        
        {
            vec4_t local0;
            VectorSet4( local0, 0.0, 1.0, 0.0, 0.0 );
            GLSL_SetUniformVec4( &tr.bloomBlurShader, UNIFORM_LOCAL0, local0 );
        }
        
        FBO_Blit( tr.bloomRenderFBO[0], NULL, NULL, tr.bloomRenderFBO[1], NULL, &tr.bloomBlurShader, color, 0 );
        FBO_FastBlit( tr.bloomRenderFBO[1], NULL, tr.bloomRenderFBO[0], NULL, GL_COLOR_BUFFER_BIT, GL_LINEAR );
#else //___BLOOM_AXIS_UNCOMBINED_SHADER___
        
        //
        // Bloom X and Y axis... (to VBO 1)
        //
        
        GLSL_BindProgram( &tr.bloomBlurShader );
        
        GL_BindToTMU( tr.bloomRenderFBOImage[0], TB_DIFFUSEMAP );
        
        {
            vec2_t screensize;
            screensize[0] = tr.bloomRenderFBOImage[0]->width;
            screensize[1] = tr.bloomRenderFBOImage[0]->height;
        
            GLSL_SetUniformVec2( &tr.bloomBlurShader, UNIFORM_DIMENSIONS, screensize );
        }
        
        {
            vec4_t local0;
            VectorSet4( local0, 0.0, 0.0, /* bloom width */ 3.0, 0.0 );
            GLSL_SetUniformVec4( &tr.bloomBlurShader, UNIFORM_LOCAL0, local0 );
        }
        
        FBO_Blit( tr.bloomRenderFBO[0], NULL, NULL, tr.bloomRenderFBO[1], NULL, &tr.bloomBlurShader, color, 0 );
        FBO_FastBlit( tr.bloomRenderFBO[1], NULL, tr.bloomRenderFBO[0], NULL, GL_COLOR_BUFFER_BIT, GL_LINEAR );
        
#endif //___BLOOM_AXIS_UNCOMBINED_SHADER___
    }
    
    //
    // Copy (and upscale) the bloom image to our full screen image...
    //
    
    FBO_Blit( tr.bloomRenderFBO[0], NULL, texDoubleScale, tr.bloomRenderFBO[2], NULL, &tr.bloomBlurShader, color, 0 );
    
    //
    // Combine the screen with the bloom'ed VBO...
    //
    
    
    GLSL_BindProgram( &tr.bloomCombineShader );
    
    GL_BindToTMU( tr.fixedLevelsImage, TB_DIFFUSEMAP );
    
    GLSL_SetUniformInt( &tr.bloomCombineShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP );
    GLSL_SetUniformInt( &tr.bloomCombineShader, UNIFORM_NORMALMAP, TB_NORMALMAP );
    
    GL_BindToTMU( tr.bloomRenderFBOImage[2], TB_NORMALMAP );
    
    {
        vec4_t local0;
        VectorSet4( local0, r_bloomScale->value, 0.0, 0.0, 0.0 );
        GLSL_SetUniformVec4( &tr.bloomCombineShader, UNIFORM_LOCAL0, local0 );
    }
    
    FBO_Blit( hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.bloomCombineShader, color, GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA );
    
    //
    // Render the results now...
    //
    
    FBO_FastBlit( ldrFbo, NULL, hdrFbo, NULL, GL_COLOR_BUFFER_BIT, GL_LINEAR );
}

void RB_SSGI( FBO_t* hdrFbo, ivec4_t hdrBox, FBO_t* ldrFbo, ivec4_t ldrBox )
{
    vec4_t	color;
    // bloom
    color[0] =
        color[1] =
            color[2] = pow( 2, r_cameraExposure->value );
    color[3] = 1.0f;
    
    {
        ivec4_t halfBox;
        vec2_t	texScale, texHalfScale, texDoubleScale;
        
        texScale[0] = texScale[1] = 1.0f;
        texHalfScale[0] = texHalfScale[1] = texScale[0] / 8.0;
        texDoubleScale[0] = texDoubleScale[1] = texScale[0] * 8.0;
        
        halfBox[0] = backEnd.viewParms.viewportX      * tr.anamorphicRenderFBOImage[0]->width / ( F32 )glConfig.vidWidth;
        halfBox[1] = backEnd.viewParms.viewportY      * tr.anamorphicRenderFBOImage[0]->height / ( F32 )glConfig.vidHeight;
        halfBox[2] = backEnd.viewParms.viewportWidth  * tr.anamorphicRenderFBOImage[0]->width / ( F32 )glConfig.vidWidth;
        halfBox[3] = backEnd.viewParms.viewportHeight * tr.anamorphicRenderFBOImage[0]->height / ( F32 )glConfig.vidHeight;
        
        //
        // Darken to VBO...
        //
        
        {
            GLSL_BindProgram( &tr.ssgiBlurShader );
            
            GL_BindToTMU( tr.fixedLevelsImage, TB_DIFFUSEMAP );
            
            {
                vec2_t screensize;
                screensize[0] = glConfig.vidWidth;
                screensize[1] = glConfig.vidHeight;
                
                GLSL_SetUniformVec2( &tr.ssgiBlurShader, UNIFORM_DIMENSIONS, screensize );
            }
            
            {
                vec4_t local0;
                VectorSet4( local0, 256.0, 0.0, 0.0, 0.0 );
                GLSL_SetUniformVec4( &tr.ssgiBlurShader, UNIFORM_LOCAL0, local0 );
            }
            
            FBO_Blit( hdrFbo, NULL, texHalfScale, tr.anamorphicRenderFBO[1], NULL, &tr.ssgiBlurShader, color, 0 );
            FBO_FastBlit( tr.anamorphicRenderFBO[1], NULL, tr.anamorphicRenderFBO[0], NULL, GL_COLOR_BUFFER_BIT, GL_LINEAR );
        }
        
        //
        // Blur the new darken'ed VBO...
        //
        
        F32 SCAN_WIDTH = r_ssgiWidth->value;
        
        //for( S32 i = 0; i < 2; i++ )
        {
            //
            // Bloom +-X axis... (to VBO 1)
            //
            
            {
                GLSL_BindProgram( &tr.ssgiBlurShader );
                
                GL_BindToTMU( tr.anamorphicRenderFBOImage[0], TB_DIFFUSEMAP );
                
                {
                    vec2_t screensize;
                    screensize[0] = tr.anamorphicRenderFBOImage[0]->width;
                    screensize[1] = tr.anamorphicRenderFBOImage[0]->height;
                    
                    GLSL_SetUniformVec2( &tr.ssgiBlurShader, UNIFORM_DIMENSIONS, screensize );
                }
                
                {
                    vec4_t local0;
                    //VectorSet4(local0, (F32)width, 0.0, 0.0, 0.0);
                    VectorSet4( local0, 1.0, 0.0, SCAN_WIDTH, 3.0 );
                    GLSL_SetUniformVec4( &tr.ssgiBlurShader, UNIFORM_LOCAL0, local0 );
                }
                
                FBO_Blit( tr.anamorphicRenderFBO[0], NULL, NULL, tr.anamorphicRenderFBO[1], NULL, &tr.ssgiBlurShader, color, 0 );
                FBO_FastBlit( tr.anamorphicRenderFBO[1], NULL, tr.anamorphicRenderFBO[0], NULL, GL_COLOR_BUFFER_BIT, GL_LINEAR );
            }
            
            //
            // Bloom +-Y axis... (to VBO 1)
            //
            
            {
                GLSL_BindProgram( &tr.ssgiBlurShader );
                
                GL_BindToTMU( tr.anamorphicRenderFBOImage[0], TB_DIFFUSEMAP );
                
                {
                    vec2_t screensize;
                    screensize[0] = tr.anamorphicRenderFBOImage[0]->width;
                    screensize[1] = tr.anamorphicRenderFBOImage[0]->height;
                    
                    GLSL_SetUniformVec2( &tr.ssgiBlurShader, UNIFORM_DIMENSIONS, screensize );
                }
                
                {
                    vec4_t local0;
                    //VectorSet4(local0, (F32)width, 0.0, 0.0, 0.0);
                    VectorSet4( local0, 0.0, 1.0, SCAN_WIDTH, 3.0 );
                    GLSL_SetUniformVec4( &tr.ssgiBlurShader, UNIFORM_LOCAL0, local0 );
                }
                
                FBO_Blit( tr.anamorphicRenderFBO[0], NULL, NULL, tr.anamorphicRenderFBO[1], NULL, &tr.ssgiBlurShader, color, 0 );
                FBO_FastBlit( tr.anamorphicRenderFBO[1], NULL, tr.anamorphicRenderFBO[0], NULL, GL_COLOR_BUFFER_BIT, GL_LINEAR );
            }
            
            //
            // Bloom XY & -X-Y axis... (to VBO 1)
            //
            
            {
                GLSL_BindProgram( &tr.ssgiBlurShader );
                
                GL_BindToTMU( tr.anamorphicRenderFBOImage[0], TB_DIFFUSEMAP );
                
                {
                    vec2_t screensize;
                    screensize[0] = tr.anamorphicRenderFBOImage[0]->width;
                    screensize[1] = tr.anamorphicRenderFBOImage[0]->height;
                    
                    GLSL_SetUniformVec2( &tr.ssgiBlurShader, UNIFORM_DIMENSIONS, screensize );
                }
                
                {
                    vec4_t local0;
                    //VectorSet4(local0, (F32)width, 0.0, 0.0, 0.0);
                    VectorSet4( local0, 1.0, 1.0, SCAN_WIDTH, 3.0 );
                    GLSL_SetUniformVec4( &tr.ssgiBlurShader, UNIFORM_LOCAL0, local0 );
                }
                
                FBO_Blit( tr.anamorphicRenderFBO[0], NULL, NULL, tr.anamorphicRenderFBO[1], NULL, &tr.ssgiBlurShader, color, 0 );
                FBO_FastBlit( tr.anamorphicRenderFBO[1], NULL, tr.anamorphicRenderFBO[0], NULL, GL_COLOR_BUFFER_BIT, GL_LINEAR );
            }
            
            //
            // Bloom -X+Y & +X-Y axis... (to VBO 1)
            //
            
            {
                GLSL_BindProgram( &tr.ssgiBlurShader );
                
                GL_BindToTMU( tr.anamorphicRenderFBOImage[0], TB_DIFFUSEMAP );
                
                {
                    vec2_t screensize;
                    screensize[0] = tr.anamorphicRenderFBOImage[0]->width;
                    screensize[1] = tr.anamorphicRenderFBOImage[0]->height;
                    
                    GLSL_SetUniformVec2( &tr.ssgiBlurShader, UNIFORM_DIMENSIONS, screensize );
                }
                
                {
                    vec4_t local0;
                    //VectorSet4(local0, (F32)width, 0.0, 0.0, 0.0);
                    VectorSet4( local0, -1.0, 1.0, SCAN_WIDTH, 3.0 );
                    GLSL_SetUniformVec4( &tr.ssgiBlurShader, UNIFORM_LOCAL0, local0 );
                }
                
                FBO_Blit( tr.anamorphicRenderFBO[0], NULL, NULL, tr.anamorphicRenderFBO[1], NULL, &tr.ssgiBlurShader, color, 0 );
                FBO_FastBlit( tr.anamorphicRenderFBO[1], NULL, tr.anamorphicRenderFBO[0], NULL, GL_COLOR_BUFFER_BIT, GL_LINEAR );
            }
        }
        
        //
        // Do a final blur pass - but this time don't mark it as a ssgi one - so that it uses darkness as well...
        //
        
        //for( S32 i = 0; i < 2; i++ )
        {
            //
            // Bloom +-X axis... (to VBO 1)
            //
            
            {
                GLSL_BindProgram( &tr.ssgiBlurShader );
                
                GL_BindToTMU( tr.anamorphicRenderFBOImage[0], TB_DIFFUSEMAP );
                
                {
                    vec2_t screensize;
                    screensize[0] = tr.anamorphicRenderFBOImage[0]->width;
                    screensize[1] = tr.anamorphicRenderFBOImage[0]->height;
                    
                    GLSL_SetUniformVec2( &tr.ssgiBlurShader, UNIFORM_DIMENSIONS, screensize );
                }
                
                {
                    vec4_t local0;
                    //VectorSet4(local0, (F32)width, 0.0, 0.0, 0.0);
                    VectorSet4( local0, 1.0, 0.0, SCAN_WIDTH, 0.0 );
                    GLSL_SetUniformVec4( &tr.ssgiBlurShader, UNIFORM_LOCAL0, local0 );
                }
                
                FBO_Blit( tr.anamorphicRenderFBO[0], NULL, NULL, tr.anamorphicRenderFBO[1], NULL, &tr.ssgiBlurShader, color, 0 );
                FBO_FastBlit( tr.anamorphicRenderFBO[1], NULL, tr.anamorphicRenderFBO[0], NULL, GL_COLOR_BUFFER_BIT, GL_LINEAR );
            }
            
            //
            // Bloom +-Y axis... (to VBO 1)
            //
            
            {
                GLSL_BindProgram( &tr.ssgiBlurShader );
                
                GL_BindToTMU( tr.anamorphicRenderFBOImage[0], TB_DIFFUSEMAP );
                
                {
                    vec2_t screensize;
                    screensize[0] = tr.anamorphicRenderFBOImage[0]->width;
                    screensize[1] = tr.anamorphicRenderFBOImage[0]->height;
                    
                    GLSL_SetUniformVec2( &tr.ssgiBlurShader, UNIFORM_DIMENSIONS, screensize );
                }
                
                {
                    vec4_t local0;
                    //VectorSet4(local0, (F32)width, 0.0, 0.0, 0.0);
                    VectorSet4( local0, 0.0, 1.0, SCAN_WIDTH, 0.0 );
                    GLSL_SetUniformVec4( &tr.ssgiBlurShader, UNIFORM_LOCAL0, local0 );
                }
                
                FBO_Blit( tr.anamorphicRenderFBO[0], NULL, NULL, tr.anamorphicRenderFBO[1], NULL, &tr.ssgiBlurShader, color, 0 );
                FBO_FastBlit( tr.anamorphicRenderFBO[1], NULL, tr.anamorphicRenderFBO[0], NULL, GL_COLOR_BUFFER_BIT, GL_LINEAR );
            }
            
            //
            // Bloom XY & -X-Y axis... (to VBO 1)
            //
            
            {
                GLSL_BindProgram( &tr.ssgiBlurShader );
                
                GL_BindToTMU( tr.anamorphicRenderFBOImage[0], TB_DIFFUSEMAP );
                
                {
                    vec2_t screensize;
                    screensize[0] = tr.anamorphicRenderFBOImage[0]->width;
                    screensize[1] = tr.anamorphicRenderFBOImage[0]->height;
                    
                    GLSL_SetUniformVec2( &tr.ssgiBlurShader, UNIFORM_DIMENSIONS, screensize );
                }
                
                {
                    vec4_t local0;
                    //VectorSet4(local0, (F32)width, 0.0, 0.0, 0.0);
                    VectorSet4( local0, 1.0, 1.0, SCAN_WIDTH, 0.0 );
                    GLSL_SetUniformVec4( &tr.ssgiBlurShader, UNIFORM_LOCAL0, local0 );
                }
                
                FBO_Blit( tr.anamorphicRenderFBO[0], NULL, NULL, tr.anamorphicRenderFBO[1], NULL, &tr.ssgiBlurShader, color, 0 );
                FBO_FastBlit( tr.anamorphicRenderFBO[1], NULL, tr.anamorphicRenderFBO[0], NULL, GL_COLOR_BUFFER_BIT, GL_LINEAR );
            }
            
            //
            // Bloom -X+Y & +X-Y axis... (to VBO 1)
            //
            
            {
                GLSL_BindProgram( &tr.ssgiBlurShader );
                
                GL_BindToTMU( tr.anamorphicRenderFBOImage[0], TB_DIFFUSEMAP );
                
                {
                    vec2_t screensize;
                    screensize[0] = tr.anamorphicRenderFBOImage[0]->width;
                    screensize[1] = tr.anamorphicRenderFBOImage[0]->height;
                    
                    GLSL_SetUniformVec2( &tr.ssgiBlurShader, UNIFORM_DIMENSIONS, screensize );
                }
                
                {
                    vec4_t local0;
                    //VectorSet4(local0, (F32)width, 0.0, 0.0, 0.0);
                    VectorSet4( local0, -1.0, 1.0, SCAN_WIDTH, 0.0 );
                    GLSL_SetUniformVec4( &tr.ssgiBlurShader, UNIFORM_LOCAL0, local0 );
                }
                
                FBO_Blit( tr.anamorphicRenderFBO[0], NULL, NULL, tr.anamorphicRenderFBO[1], NULL, &tr.ssgiBlurShader, color, 0 );
                FBO_FastBlit( tr.anamorphicRenderFBO[1], NULL, tr.anamorphicRenderFBO[0], NULL, GL_COLOR_BUFFER_BIT, GL_LINEAR );
            }
        }
        
        //
        // Copy (and upscale) the bloom image to our full screen image...
        //
        
        FBO_Blit( tr.anamorphicRenderFBO[0], NULL, texDoubleScale, tr.anamorphicRenderFBO[2], NULL, &tr.ssgiBlurShader, color, 0 );
    }
    
    //
    // Do the SSAO/SSGI...
    //
    
    GLSL_BindProgram( &tr.ssgiShader );
    
    GLSL_SetUniformInt( &tr.ssgiShader, UNIFORM_LEVELSMAP, TB_LEVELSMAP );
    GL_BindToTMU( tr.fixedLevelsImage, TB_LEVELSMAP );
    GLSL_SetUniformInt( &tr.ssgiShader, UNIFORM_SCREENDEPTHMAP, TB_LIGHTMAP );
    GL_BindToTMU( tr.renderDepthImage, TB_LIGHTMAP );
    GLSL_SetUniformInt( &tr.ssgiShader, UNIFORM_NORMALMAP, TB_NORMALMAP );
    GL_BindToTMU( tr.normalDetailedImage, TB_NORMALMAP );
    GLSL_SetUniformInt( &tr.ssgiShader, UNIFORM_GLOWMAP, TB_GLOWMAP );
    GL_BindToTMU( tr.anamorphicRenderFBOImage[2], TB_GLOWMAP );
    GLSL_SetUniformInt( &tr.ssgiShader, UNIFORM_RANDOMMAP, TB_RANDOMMAP );
    GL_BindToTMU( tr.randomImage, TB_RANDOMMAP );
    
    {
        vec2_t screensize;
        screensize[0] = glConfig.vidWidth;
        screensize[1] = glConfig.vidHeight;
        
        GLSL_SetUniformVec2( &tr.ssgiShader, UNIFORM_DIMENSIONS, screensize );
        
        //CL_RefPrintf(PRINT_WARNING, "Sent dimensions %f %f.\n", screensize[0], screensize[1]);
    }
    
    {
        vec4_t viewInfo;
        F32 zmax = backEnd.viewParms.zFar;
        F32 ymax = zmax * tan( backEnd.viewParms.fovY * M_PI / 360.0f );
        F32 xmax = zmax * tan( backEnd.viewParms.fovX * M_PI / 360.0f );
        F32 zmin = r_znear->value;;
        VectorSet4( viewInfo, zmin, zmax, zmax / zmin, 0.0 );
        GLSL_SetUniformVec4( &tr.ssgiShader, UNIFORM_VIEWINFO, viewInfo );
    }
    
    {
        vec4_t local0;
        local0[0] = r_ssgi->value;
        local0[1] = r_ssgiSamples->value;
        local0[2] = 0.0;
        local0[3] = 0.0;
        
        GLSL_SetUniformVec4( &tr.ssgiShader, UNIFORM_LOCAL0, local0 );
        
        //CL_RefPrintf(PRINT_WARNING, "Sent dimensions %f %f.\n", screensize[0], screensize[1]);
    }
    
    FBO_Blit( hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.ssgiShader, color, 0 ); //GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA);
}

void RB_TextureDetail( FBO_t* hdrFbo, ivec4_t hdrBox, FBO_t* ldrFbo, ivec4_t ldrBox )
{
    vec4_t color;
    
    // bloom
    color[0] =
        color[1] =
            color[2] = pow( 2, r_cameraExposure->value );
    color[3] = 1.0f;
    
    GLSL_BindProgram( &tr.texturedetailShader );
    
    GL_BindToTMU( hdrFbo->colorImage[0], TB_LEVELSMAP );
    
    GLSL_SetUniformMat4( &tr.texturedetailShader, UNIFORM_INVEYEPROJECTIONMATRIX, glState.invEyeProjection );
    GLSL_SetUniformInt( &tr.texturedetailShader, UNIFORM_SCREENDEPTHMAP, TB_LIGHTMAP );
    GL_BindToTMU( tr.renderDepthImage, TB_LIGHTMAP );
    {
        vec2_t screensize;
        screensize[0] = glConfig.vidWidth;
        screensize[1] = glConfig.vidHeight;
        
        GLSL_SetUniformVec2( &tr.texturedetailShader, UNIFORM_DIMENSIONS, screensize );
    }
    
    {
        vec4_t viewInfo;
        
        F32 zmax = backEnd.viewParms.zFar;
        F32 zmin = r_znear->value;
        
        VectorSet4( viewInfo, zmax / zmin, zmax, zmin, zmax );
        
        GLSL_SetUniformVec4( &tr.texturedetailShader, UNIFORM_VIEWINFO, viewInfo );
    }
    
    {
        vec4_t local0;
        VectorSet4( local0, r_texturedetailStrength->value, 0.0, 0.0, 0.0 ); // non-flicker version
        GLSL_SetUniformVec4( &tr.texturedetailShader, UNIFORM_LOCAL0, local0 );
    }
    
    FBO_Blit( hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.texturedetailShader, color, 0 ); //GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA);
}

void RB_RBM( FBO_t* hdrFbo, ivec4_t hdrBox, FBO_t* ldrFbo, ivec4_t ldrBox )
{
    vec4_t		color;
    
    // bloom
    color[0] =
        color[1] =
            color[2] = pow( 2, r_cameraExposure->value );
    color[3] = 1.0f;
    
    GLSL_BindProgram( &tr.rbmShader );
    
    GLSL_SetUniformMat4( &tr.rbmShader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection );
    GLSL_SetUniformMat4( &tr.rbmShader, UNIFORM_MODELMATRIX, backEnd.orientation.transformMatrix );
    
    GLSL_SetUniformVec3( &tr.rbmShader, UNIFORM_VIEWORIGIN, backEnd.refdef.vieworg );
    
    GLSL_SetUniformInt( &tr.rbmShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP );
    GL_BindToTMU( hdrFbo->colorImage[0], TB_DIFFUSEMAP );
    GLSL_SetUniformInt( &tr.rbmShader, UNIFORM_NORMALMAP, TB_NORMALMAP );
    GL_BindToTMU( tr.normalDetailedImage, TB_NORMALMAP );
    GLSL_SetUniformInt( &tr.rbmShader, UNIFORM_SCREENDEPTHMAP, TB_LIGHTMAP );
    GL_BindToTMU( tr.renderDepthImage, TB_LIGHTMAP );
    
    {
        vec4_t local0;
        VectorSet4( local0, r_rbmStrength->value, 0.0, 0.0, 0.0 );
        GLSL_SetUniformVec4( &tr.rbmShader, UNIFORM_LOCAL0, local0 );
    }
    
    {
        vec2_t screensize;
        screensize[0] = glConfig.vidWidth;
        screensize[1] = glConfig.vidHeight;
        
        GLSL_SetUniformVec2( &tr.rbmShader, UNIFORM_DIMENSIONS, screensize );
    }
    
    {
        vec4_t viewInfo;
        F32 zmax = backEnd.viewParms.zFar;
        F32 ymax = zmax * tan( backEnd.viewParms.fovY * M_PI / 360.0f );
        F32 xmax = zmax * tan( backEnd.viewParms.fovX * M_PI / 360.0f );
        F32 zmin = r_znear->value;
        VectorSet4( viewInfo, zmin, zmax, zmax / zmin, 0.0 );
        GLSL_SetUniformVec4( &tr.rbmShader, UNIFORM_VIEWINFO, viewInfo );
    }
    
    FBO_Blit( hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.rbmShader, color, 0 );
}

void RB_Contrast( FBO_t* src, ivec4_t srcBox, FBO_t* dst, ivec4_t dstBox )
{
    F32 brightness = 2;
    
    if( !glRefConfig.framebufferObject )
    {
        return;
    }
    
    // if cvars are at their defaults, do no processing
    if( r_brightness->value == 0.0f && r_contrast->value == 1.0f && r_gamma->value == 1.0 )
    {
        return;
    }
    GLSL_SetUniformInt( &tr.contrastShader, UNIFORM_TEXTUREMAP, TB_DIFFUSEMAP );
    GLSL_SetUniformFloat( &tr.contrastShader, UNIFORM_BRIGHTNESS, r_brightness->value );
    GLSL_SetUniformFloat( &tr.contrastShader, UNIFORM_CONTRAST, r_contrast->value );
    GLSL_SetUniformFloat( &tr.contrastShader, UNIFORM_GAMMA, r_gamma->value );
    
    FBO_FastBlit( src, srcBox, tr.screenScratchFbo, srcBox, GL_COLOR_BUFFER_BIT, GL_LINEAR );
    FBO_Blit( tr.screenScratchFbo, srcBox, NULL, dst, dstBox, &tr.contrastShader, NULL, 0 );
}

void RB_HBAO( FBO_t* hdrFbo, ivec4_t hdrBox, FBO_t* ldrFbo, ivec4_t ldrBox )
{
    vec4_t color;
    
    // bloom
    color[0] =
        color[1] =
            color[2] = pow( 2, r_cameraExposure->value );
    color[3] = 1.0f;
    
    // Generate hbao image...
    GLSL_BindProgram( &tr.hbaoShader );
    
    GLSL_SetUniformMat4( &tr.hbaoShader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection );
    GLSL_SetUniformMat4( &tr.hbaoShader, UNIFORM_INVEYEPROJECTIONMATRIX, glState.invEyeProjection );
    GLSL_SetUniformMat4( &tr.hbaoShader, UNIFORM_MODELMATRIX, backEnd.orientation.transformMatrix );
    
    GLSL_SetUniformInt( &tr.hbaoShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP );
    GL_BindToTMU( tr.fixedLevelsImage, TB_DIFFUSEMAP );
    
    GLSL_SetUniformInt( &tr.hbaoShader, UNIFORM_NORMALMAP, TB_NORMALMAP );
    GL_BindToTMU( tr.normalDetailedImage, TB_NORMALMAP );
    GLSL_SetUniformInt( &tr.hbaoShader, UNIFORM_SCREENDEPTHMAP, TB_LIGHTMAP );
    GL_BindToTMU( tr.renderDepthImage, TB_LIGHTMAP );
    
    {
        vec2_t screensize;
        screensize[0] = glConfig.vidWidth;
        screensize[1] = glConfig.vidHeight;
        
        GLSL_SetUniformVec2( &tr.hbaoShader, UNIFORM_DIMENSIONS, screensize );
        
        //CL_RefPrint(PRINT_WARNING, "Sent dimensions %f %f.\n", screensize[0], screensize[1]);
    }
    
    {
        vec4_t viewInfo;
        F32 zmax = backEnd.viewParms.zFar;
        F32 ymax = zmax * tan( backEnd.viewParms.fovY * M_PI / 360.0f );
        F32 xmax = zmax * tan( backEnd.viewParms.fovX * M_PI / 360.0f );
        F32 zmin = r_znear->value;
        //VectorSet4(viewInfo, zmax / zmin, zmax, 0.0, 0.0);
        VectorSet4( viewInfo, zmin, zmax, zmax / zmin, 0.0 );
        GLSL_SetUniformVec4( &tr.hbaoShader, UNIFORM_VIEWINFO, viewInfo );
    }
    
    //#define HBAO_DEBUG
    
#ifndef HBAO_DEBUG
    FBO_Blit( hdrFbo, hdrBox, NULL, tr.genericFbo, ldrBox, &tr.hbaoShader, color, 0 );
    
    // Combine render and hbao...
    GLSL_BindProgram( &tr.hbaoCombineShader );
    
    GLSL_SetUniformMat4( &tr.hbaoCombineShader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection );
    GLSL_SetUniformMat4( &tr.hbaoCombineShader, UNIFORM_MODELMATRIX, backEnd.orientation.transformMatrix );
    
    GL_BindToTMU( tr.fixedLevelsImage, TB_DIFFUSEMAP );
    
    GLSL_SetUniformInt( &tr.hbaoCombineShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP );
    GLSL_SetUniformInt( &tr.hbaoCombineShader, UNIFORM_NORMALMAP, TB_NORMALMAP );
    GL_BindToTMU( tr.genericFBOImage, TB_NORMALMAP );
    
    vec2_t screensize;
    screensize[0] = glConfig.vidWidth;
    screensize[1] = glConfig.vidHeight;
    GLSL_SetUniformVec2( &tr.hbaoCombineShader, UNIFORM_DIMENSIONS, screensize );
    
    FBO_Blit( hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.hbaoCombineShader, color, 0 );
#else //HBAO_DEBUG
    FBO_Blit( hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.hbaoShader, color, 0 );
#endif //HBAO_DEBUG
}

bool RB_SSS( FBO_t* hdrFbo, ivec4_t hdrBox, FBO_t* ldrFbo, ivec4_t ldrBox )
{
    vec4_t color;
    
    // bloom
    color[0] =
        color[1] =
            color[2] = pow( 2, r_cameraExposure->value );
    color[3] = 1.0f;
    
    GLSL_BindProgram( &tr.sssShader );
    
    GLSL_SetUniformMat4( &tr.sssShader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection );
    GLSL_SetUniformMat4( &tr.sssShader, UNIFORM_INVPROJECTIONMATRIX, glState.invEyeProjection );
    GLSL_SetUniformMat4( &tr.sssShader, UNIFORM_MODELMATRIX, backEnd.orientation.transformMatrix );
    
    GLSL_SetUniformInt( &tr.sssShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP );
    GL_BindToTMU( tr.fixedLevelsImage, TB_DIFFUSEMAP );
    GLSL_SetUniformInt( &tr.sssShader, UNIFORM_SCREENDEPTHMAP, TB_LIGHTMAP );
    GL_BindToTMU( tr.renderDepthImage, TB_LIGHTMAP );
    GLSL_SetUniformInt( &tr.sssShader, UNIFORM_GLOWMAP, TB_GLOWMAP );
    GL_BindToTMU( tr.glowImage, TB_GLOWMAP );
    GLSL_SetUniformInt( &tr.sssShader, UNIFORM_SPECULARMAP, TB_SPECULARMAP );
    
    {
        vec2_t screensize;
        screensize[0] = glConfig.vidWidth;
        screensize[1] = glConfig.vidHeight;
        
        GLSL_SetUniformVec2( &tr.sssShader, UNIFORM_DIMENSIONS, screensize );
    }
    
    if( SUN_VISIBLE )
    {
        // Add sun...
        //SUN_ORIGIN
        F32 x, y;
        S32	xcenter, ycenter;
        vec3_t	local, transformed;
        vec3_t	vfwd, vright, vup, viewAngles;
        
        TR_AxisToAngles( backEnd.refdef.viewaxis, viewAngles );
        
        //NOTE: did it this way because most draw functions expect virtual 640x480 coords
        //	and adjust them for current resolution
        xcenter = 640 / 2;//gives screen coords in virtual 640x480, to be adjusted when drawn
        ycenter = 480 / 2;//gives screen coords in virtual 640x480, to be adjusted when drawn
        
        VectorSubtract( SUN_POSITION, backEnd.refdef.vieworg, local );
        
        AngleVectors( viewAngles, vfwd, vright, vup );
        
        transformed[0] = DotProduct( local, vright );
        transformed[1] = DotProduct( local, vup );
        transformed[2] = DotProduct( local, vfwd );
        
        // Make sure Z is not negative.
        if( transformed[2] < 0.01 )
        {
            //return false;
            //transformed[2] = 2.0 - transformed[2];
        }
        
        // Simple convert to screen coords.
        F32 xzi = xcenter / transformed[2] * ( 90.0 / backEnd.refdef.fov_x );
        F32 yzi = ycenter / transformed[2] * ( 90.0 / backEnd.refdef.fov_y );
        
        x = ( xcenter + xzi * transformed[0] );
        y = ( ycenter - yzi * transformed[1] );
        
        vec4_t loc;
        VectorSet4( loc, x / 640, 1.0 - ( y / 480 ), 0.0, 0.0 );
        GLSL_SetUniformVec4( &tr.sssShader, UNIFORM_LOCAL1, loc );
    }
    
    {
        vec4_t viewInfo;
        F32 zmax = backEnd.viewParms.zFar;
        F32 ymax = zmax * tan( backEnd.viewParms.fovY * M_PI / 360.0f );
        F32 xmax = zmax * tan( backEnd.viewParms.fovX * M_PI / 360.0f );
        F32 zmin = r_znear->value;
        VectorSet4( viewInfo, zmin, zmax, zmax / zmin, 0.0 );
        GLSL_SetUniformVec4( &tr.sssShader, UNIFORM_VIEWINFO, viewInfo );
    }
    
    FBO_Blit( hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.sssShader, color, 0 );
    
    return true;
}

extern S32			NUM_CLOSE_LIGHTS;
extern S32			CLOSEST_LIGHTS[MAX_DEFERRED_LIGHTS];
extern vec2_t		CLOSEST_LIGHTS_SCREEN_POSITIONS[MAX_DEFERRED_LIGHTS];
extern vec3_t		CLOSEST_LIGHTS_POSITIONS[MAX_DEFERRED_LIGHTS];
extern F32		CLOSEST_LIGHTS_DISTANCES[MAX_DEFERRED_LIGHTS];
extern F32		CLOSEST_LIGHTS_HEIGHTSCALES[MAX_DEFERRED_LIGHTS];
extern vec3_t		CLOSEST_LIGHTS_COLORS[MAX_DEFERRED_LIGHTS];

extern F32		SUN_PHONG_SCALE;
extern F32		SHADOW_MINBRIGHT;
extern F32		SHADOW_MAXBRIGHT;

void RB_DeferredLighting( FBO_t* hdrFbo, ivec4_t hdrBox, FBO_t* ldrFbo, ivec4_t ldrBox )
{
    vec4_t color;
    
    // bloom
    color[0] =
        color[1] =
            color[2] = pow( 2, r_cameraExposure->value );
    color[3] = 1.0f;
    
    GLSL_BindProgram( &tr.deferredLightingShader );
    
    GLSL_SetUniformInt( &tr.deferredLightingShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP );
    GL_BindToTMU( hdrFbo->colorImage[0], TB_DIFFUSEMAP );
    
    GLSL_SetUniformInt( &tr.deferredLightingShader, UNIFORM_SCREENDEPTHMAP, TB_LIGHTMAP );
    GL_BindToTMU( tr.renderDepthImage, TB_LIGHTMAP );
    
    GLSL_SetUniformInt( &tr.deferredLightingShader, UNIFORM_NORMALMAP, TB_NORMALMAP );
    GL_BindToTMU( tr.renderNormalImage, TB_NORMALMAP );
    
    GLSL_SetUniformInt( &tr.deferredLightingShader, UNIFORM_POSITIONMAP, TB_POSITIONMAP );
    GL_BindToTMU( tr.renderPositionMapImage, TB_POSITIONMAP );
    
    if( r_ssdo->integer )
    {
        GLSL_SetUniformInt( &tr.deferredLightingShader, UNIFORM_HEIGHTMAP, TB_HEIGHTMAP );
        GL_BindToTMU( tr.ssdoImage1, TB_HEIGHTMAP );
    }
    
    //if (SHADOWS_ENABLED)
    {
        if( r_shadowBlur->integer )
        {
            GLSL_SetUniformInt( &tr.deferredLightingShader, UNIFORM_SHADOWMAP, TB_SHADOWMAP );
            GL_BindToTMU( tr.screenShadowBlurImage, TB_SHADOWMAP );
        }
        else
        {
            GLSL_SetUniformInt( &tr.deferredLightingShader, UNIFORM_SHADOWMAP, TB_SHADOWMAP );
            GL_BindToTMU( tr.screenShadowImage, TB_SHADOWMAP );
        }
    }
    
    /*for (S32 i = 0; i < NUM_CLOSE_LIGHTS; i++)
    {
    CL_RefPrint(PRINT_WARNING, "%i - %i %i %i\n", i, (int)CLOSEST_LIGHTS_POSITIONS[i][0], (int)CLOSEST_LIGHTS_POSITIONS[i][1], (int)CLOSEST_LIGHTS_POSITIONS[i][2]);
    }*/
    
    GLSL_SetUniformMat4( &tr.deferredLightingShader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection );
    
    GLSL_SetUniformInt( &tr.deferredLightingShader, UNIFORM_LIGHTCOUNT, NUM_CLOSE_LIGHTS );
    GLSL_SetUniformVec2x16( &tr.deferredLightingShader, UNIFORM_LIGHTPOSITIONS, CLOSEST_LIGHTS_SCREEN_POSITIONS, MAX_DEFERRED_LIGHTS );
    GLSL_SetUniformVec3xX( &tr.deferredLightingShader, UNIFORM_LIGHTPOSITIONS2, CLOSEST_LIGHTS_POSITIONS, MAX_DEFERRED_LIGHTS );
    GLSL_SetUniformVec3xX( &tr.deferredLightingShader, UNIFORM_LIGHTCOLORS, CLOSEST_LIGHTS_COLORS, MAX_DEFERRED_LIGHTS );
    GLSL_SetUniformFloatxX( &tr.deferredLightingShader, UNIFORM_LIGHTDISTANCES, CLOSEST_LIGHTS_DISTANCES, MAX_DEFERRED_LIGHTS );
    GLSL_SetUniformFloatxX( &tr.deferredLightingShader, UNIFORM_LIGHTHEIGHTSCALES, CLOSEST_LIGHTS_HEIGHTSCALES, MAX_DEFERRED_LIGHTS );
    
    GLSL_SetUniformVec3( &tr.deferredLightingShader, UNIFORM_VIEWORIGIN, backEnd.refdef.vieworg );
    
    
    vec3_t out;
    F32 dist = 4096.0;//backEnd.viewParms.zFar / 1.75;
    VectorMA( backEnd.refdef.vieworg, dist, backEnd.refdef.sunDir, out );
    GLSL_SetUniformVec4( &tr.deferredLightingShader, UNIFORM_PRIMARYLIGHTORIGIN, out );
    
    //GLSL_SetUniformVec4(&tr.deferredLightingShader, UNIFORM_LOCAL2,  backEnd.refdef.sunDir);
    GLSL_SetUniformVec3( &tr.deferredLightingShader, UNIFORM_PRIMARYLIGHTCOLOR, backEnd.refdef.sunCol );
    
    vec4_t local1;
    VectorSet4( local1, r_blinnPhong->value, 1.0, r_ao->integer ? 1.0 : 0.0, r_env->integer ? 1.0 : 0.0 );
    GLSL_SetUniformVec4( &tr.deferredLightingShader, UNIFORM_LOCAL1, local1 );
    
    vec4_t local2;
    VectorSet4( local2, ( r_ssdo->integer > 0 && r_ssdo->integer < 3 ) ? 1.0 : 0.0, 1.0, 0.7, 1.0 );
    GLSL_SetUniformVec4( &tr.deferredLightingShader, UNIFORM_LOCAL2, local2 );
    
    vec4_t local3;
    VectorSet4( local3, r_testshaderValue1->value, r_testshaderValue2->value, r_testshaderValue3->value, r_testshaderValue4->value );
    GLSL_SetUniformVec4( &tr.deferredLightingShader, UNIFORM_LOCAL3, local3 );
    
    {
        vec2_t screensize;
        screensize[0] = glConfig.vidWidth;
        screensize[1] = glConfig.vidHeight;
        
        GLSL_SetUniformVec2( &tr.deferredLightingShader, UNIFORM_DIMENSIONS, screensize );
    }
    
    {
        vec4_t viewInfo;
        //F32 zmax = 2048.0;
        F32 zmax = backEnd.viewParms.zFar;
        F32 ymax = zmax * tan( backEnd.viewParms.fovY * M_PI / 360.0f );
        F32 xmax = zmax * tan( backEnd.viewParms.fovX * M_PI / 360.0f );
        F32 zmin = r_znear->value;
        VectorSet4( viewInfo, zmin, zmax, zmax / zmin, backEnd.viewParms.fovX );
        GLSL_SetUniformVec4( &tr.deferredLightingShader, UNIFORM_VIEWINFO, viewInfo );
    }
    
    FBO_Blit( hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.deferredLightingShader, color, 0 );
}

void RB_ScreenSpaceReflections( FBO_t* hdrFbo, ivec4_t hdrBox, FBO_t* ldrFbo, ivec4_t ldrBox )
{
    vec4_t color;
    
    // bloom
    color[0] =
        color[1] =
            color[2] = pow( 2, r_cameraExposure->value );
    color[3] = 1.0f;
    
    GLSL_BindProgram( &tr.ssrShader );
    
    GLSL_SetUniformInt( &tr.ssrShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP );
    GL_BindToTMU( hdrFbo->colorImage[0], TB_DIFFUSEMAP );
    
    GLSL_SetUniformInt( &tr.ssrShader, UNIFORM_GLOWMAP, TB_GLOWMAP );
    
    // Use the most blurred version of glow...
    if( r_anamorphic->integer )
    {
        GL_BindToTMU( tr.anamorphicRenderFBOImage[0], TB_GLOWMAP );
    }
    else if( r_bloom->integer )
    {
        GL_BindToTMU( tr.bloomRenderFBOImage[0], TB_GLOWMAP );
    }
    else
    {
        GL_BindToTMU( tr.glowFboScaled[0]->colorImage[0], TB_GLOWMAP );
    }
    
    GLSL_SetUniformInt( &tr.ssrShader, UNIFORM_SCREENDEPTHMAP, TB_LIGHTMAP );
    GL_BindToTMU( tr.renderDepthImage, TB_LIGHTMAP );
    
    
    GLSL_SetUniformMat4( &tr.ssrShader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection );
    
    vec4_t local1;
    VectorSet4( local1, r_ssr->integer, r_sse->integer, r_ssrStrength->value, r_sseStrength->value );
    GLSL_SetUniformVec4( &tr.ssrShader, UNIFORM_LOCAL1, local1 );
    
    vec4_t local3;
    VectorSet4( local3, r_testshaderValue1->value, r_testshaderValue2->value, r_testshaderValue3->value, r_testshaderValue4->value );
    GLSL_SetUniformVec4( &tr.ssrShader, UNIFORM_LOCAL3, local3 );
    
    {
        vec2_t screensize;
        screensize[0] = glConfig.vidWidth;
        screensize[1] = glConfig.vidHeight;
        
        GLSL_SetUniformVec2( &tr.ssrShader, UNIFORM_DIMENSIONS, screensize );
    }
    
    {
        vec4_t viewInfo;
        //F32 zmax = 2048.0;
        F32 zmax = backEnd.viewParms.zFar;
        F32 ymax = zmax * tan( backEnd.viewParms.fovY * M_PI / 360.0f );
        F32 xmax = zmax * tan( backEnd.viewParms.fovX * M_PI / 360.0f );
        F32 zmin = r_znear->value;
        VectorSet4( viewInfo, zmin, zmax, zmax / zmin, backEnd.viewParms.fovX );
        GLSL_SetUniformVec4( &tr.ssrShader, UNIFORM_VIEWINFO, viewInfo );
    }
    
    //FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.ssrShader, color, 0);
    
    FBO_Blit( hdrFbo, NULL, NULL, tr.genericFbo2, NULL, &tr.ssrShader, color, 0 );
    
    // Combine render and hbao...
    GLSL_BindProgram( &tr.ssrCombineShader );
    
    GLSL_SetUniformMat4( &tr.ssrCombineShader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection );
    GLSL_SetUniformMat4( &tr.ssrCombineShader, UNIFORM_MODELMATRIX, backEnd.orientation.transformMatrix );
    
    GLSL_SetUniformInt( &tr.ssrCombineShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP );
    GL_BindToTMU( hdrFbo->colorImage[0], TB_DIFFUSEMAP );
    
    GLSL_SetUniformInt( &tr.ssrCombineShader, UNIFORM_NORMALMAP, TB_NORMALMAP );
    GL_BindToTMU( tr.genericFBO2Image, TB_NORMALMAP );
    
    vec2_t screensize;
    screensize[0] = tr.genericFBO2Image->width;// glConfig.vidWidth * r_superSampleMultiplier->value;
    screensize[1] = tr.genericFBO2Image->height;// glConfig.vidHeight * r_superSampleMultiplier->value;
    GLSL_SetUniformVec2( &tr.ssrCombineShader, UNIFORM_DIMENSIONS, screensize );
    
    FBO_Blit( hdrFbo, NULL, NULL, ldrFbo, NULL, &tr.ssrCombineShader, color, 0 );
}

F32 mix( F32 x, F32 y, F32 a )
{
    return ( 1 - a ) * x + a * y;
}

void RB_DepthToNormal( FBO_t* hdrFbo, ivec4_t hdrBox, FBO_t* ldrFbo, ivec4_t ldrBox )
{
    vec4_t		color;
    
    // bloom
    color[0] =
        color[1] =
            color[2] = pow( 2, r_cameraExposure->value );
    color[3] = 1.0f;
    
    GLSL_BindProgram( &tr.depthToNormalShader );
    
    GLSL_SetUniformMat4( &tr.depthToNormalShader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection );
    
    GLSL_SetUniformVec3( &tr.depthToNormalShader, UNIFORM_VIEWORIGIN, backEnd.refdef.vieworg );
    
    GLSL_SetUniformInt( &tr.depthToNormalShader, UNIFORM_SCREENDEPTHMAP, TB_LIGHTMAP );
    GL_BindToTMU( tr.renderDepthImage, TB_LIGHTMAP );
    
    {
        vec2_t screensize;
        screensize[0] = glConfig.vidWidth;
        screensize[1] = glConfig.vidHeight;
        
        GLSL_SetUniformVec2( &tr.depthToNormalShader, UNIFORM_DIMENSIONS, screensize );
    }
    
    {
        vec4_t viewInfo;
        F32 zmax = backEnd.viewParms.zFar;
        F32 ymax = zmax * tan( backEnd.viewParms.fovY * M_PI / 360.0f );
        F32 xmax = zmax * tan( backEnd.viewParms.fovX * M_PI / 360.0f );
        F32 zmin = r_znear->value;
        VectorSet4( viewInfo, zmin, zmax, zmax / zmin, 0.0 );
        GLSL_SetUniformVec4( &tr.depthToNormalShader, UNIFORM_VIEWINFO, viewInfo );
    }
    
    FBO_Blit( tr.ssdoFbo2, hdrBox, NULL, tr.screenPureNormalFbo, ldrBox, &tr.depthToNormalShader, color, 0 );
}

void RB_SSDO( FBO_t* hdrFbo, ivec4_t hdrBox, FBO_t* ldrFbo, ivec4_t ldrBox )
{
    vec4_t color;
    
    // bloom
    color[0] =
        color[1] =
            color[2] = pow( 2, r_cameraExposure->value );
    color[3] = 1.0f;
    
    //
    // Generate occlusion map...
    //
    
    GLSL_BindProgram( &tr.ssdoShader );
    
    GLSL_SetUniformMat4( &tr.ssdoShader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection );
    
    GLSL_SetUniformInt( &tr.ssdoShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP );
    GL_BindToTMU( hdrFbo->colorImage[0], TB_DIFFUSEMAP );
    
    GLSL_SetUniformInt( &tr.ssdoShader, UNIFORM_POSITIONMAP, TB_POSITIONMAP );
    GL_BindToTMU( tr.renderPositionMapImage, TB_POSITIONMAP );
    
    GLSL_SetUniformInt( &tr.ssdoShader, UNIFORM_NORMALMAP, TB_NORMALMAP );
    GL_BindToTMU( tr.screenPureNormalImage, TB_NORMALMAP );
    
    GLSL_SetUniformInt( &tr.ssdoShader, UNIFORM_SCREENDEPTHMAP, TB_LIGHTMAP );
    GL_BindToTMU( tr.renderDepthImage, TB_LIGHTMAP );
    
    GLSL_SetUniformInt( &tr.ssdoShader, UNIFORM_DELUXEMAP, TB_DELUXEMAP );
    GL_BindToTMU( tr.ssdoNoiseImage, TB_DELUXEMAP );
    
    GLSL_SetUniformVec3( &tr.ssdoShader, UNIFORM_VIEWORIGIN, backEnd.refdef.vieworg );
    
    vec4_t viewInfo;
    F32 zmax = backEnd.viewParms.zFar;
    //F32 ymax = zmax * tan(backEnd.viewParms.fovY * M_PI / 360.0f);
    //F32 xmax = zmax * tan(backEnd.viewParms.fovX * M_PI / 360.0f);
    
    vec2_t screensize;
    screensize[0] = glConfig.vidWidth;
    screensize[1] = glConfig.vidHeight;
    GLSL_SetUniformVec2( &tr.ssdoShader, UNIFORM_DIMENSIONS, screensize );
    
    // tan(RATIO*FOVY*0.5f),tan(FOVY*0.5f));
    F32 ratio = screensize[0] / screensize[1];
    F32 xmax = tan( backEnd.viewParms.fovX * ratio * 0.5 );
    F32 ymax = tan( backEnd.viewParms.fovY * 0.5 );
    F32 zmin = r_znear->value;
    VectorSet4( viewInfo, zmin, zmax, zmax / zmin, 0.0 );
    GLSL_SetUniformVec4( &tr.ssdoShader, UNIFORM_VIEWINFO, viewInfo );
    
    vec3_t out;
    F32 dist = 4096.0;//backEnd.viewParms.zFar / 1.75;
    VectorMA( backEnd.refdef.vieworg, dist, backEnd.refdef.sunDir, out );
    GLSL_SetUniformVec4( &tr.ssdoShader, UNIFORM_PRIMARYLIGHTORIGIN, out );
    
    vec4_t local0;
    VectorSet4( local0, screensize[0] / tr.random2KImage[0]->width, screensize[1] / tr.random2KImage[0]->height, r_ssdoBaseRadius->value, r_ssdoMaxOcclusionDist->value );
    GLSL_SetUniformVec4( &tr.ssdoShader, UNIFORM_LOCAL0, local0 );
    
    vec4_t local1;
    VectorSet4( local1, xmax, ymax, r_testvalue0->value, r_testvalue1->value );
    GLSL_SetUniformVec4( &tr.ssdoShader, UNIFORM_LOCAL1, local1 );
    
    //FBO_Blit(hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.ssdoShader, color, 0);
    FBO_Blit( hdrFbo, hdrBox, NULL, tr.ssdoFbo1, ldrBox, &tr.ssdoShader, color, 0 );
    
    //
    // Blur Occlusion Map...
    //
    
    GLSL_BindProgram( &tr.ssdoBlurShader );
    
    GLSL_SetUniformMat4( &tr.ssdoBlurShader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection );
    
    GLSL_SetUniformInt( &tr.ssdoBlurShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP );
    GL_BindToTMU( hdrFbo->colorImage[0], TB_DIFFUSEMAP );
    
    GLSL_SetUniformInt( &tr.ssdoBlurShader, UNIFORM_POSITIONMAP, TB_POSITIONMAP );
    GL_BindToTMU( tr.renderPositionMapImage, TB_POSITIONMAP );
    
    GLSL_SetUniformInt( &tr.ssdoBlurShader, UNIFORM_NORMALMAP, TB_NORMALMAP );
    GL_BindToTMU( tr.screenPureNormalImage, TB_NORMALMAP );
    
    GLSL_SetUniformInt( &tr.ssdoBlurShader, UNIFORM_SCREENDEPTHMAP, TB_LIGHTMAP );
    GL_BindToTMU( tr.renderDepthImage, TB_LIGHTMAP );
    
    GLSL_SetUniformVec3( &tr.ssdoBlurShader, UNIFORM_VIEWORIGIN, backEnd.refdef.vieworg );
    GLSL_SetUniformVec2( &tr.ssdoBlurShader, UNIFORM_DIMENSIONS, screensize );
    GLSL_SetUniformVec4( &tr.ssdoBlurShader, UNIFORM_VIEWINFO, viewInfo );
    GLSL_SetUniformVec4( &tr.ssdoBlurShader, UNIFORM_PRIMARYLIGHTORIGIN, out );
    
    // X
    GLSL_SetUniformInt( &tr.ssdoBlurShader, UNIFORM_DELUXEMAP, TB_DELUXEMAP );
    GL_BindToTMU( tr.ssdoImage1, TB_DELUXEMAP );
    
    VectorSet4( local0, 1.0, 0.0, 0.0, 0.0 );
    GLSL_SetUniformVec4( &tr.ssdoBlurShader, UNIFORM_LOCAL0, local0 );
    
    FBO_Blit( tr.ssdoFbo1, hdrBox, NULL, tr.ssdoFbo2, ldrBox, &tr.ssdoBlurShader, color, 0 );
    
    // Y
    GLSL_SetUniformInt( &tr.ssdoBlurShader, UNIFORM_DELUXEMAP, TB_DELUXEMAP );
    GL_BindToTMU( tr.ssdoImage2, TB_DELUXEMAP );
    
    VectorSet4( local0, 0.0, 1.0, 0.0, 0.0 );
    GLSL_SetUniformVec4( &tr.ssdoBlurShader, UNIFORM_LOCAL0, local0 );
    
    FBO_Blit( tr.ssdoFbo2, hdrBox, NULL, tr.ssdoFbo1, ldrBox, &tr.ssdoBlurShader, color, 0 );
    
    if( r_ssdo->integer == 2 )
    {
        // X
        GLSL_SetUniformInt( &tr.ssdoBlurShader, UNIFORM_DELUXEMAP, TB_DELUXEMAP );
        GL_BindToTMU( tr.ssdoImage1, TB_DELUXEMAP );
        
        VectorSet4( local0, 1.0, 0.0, 0.0, 0.0 );
        GLSL_SetUniformVec4( &tr.ssdoBlurShader, UNIFORM_LOCAL0, local0 );
        
        FBO_Blit( tr.ssdoFbo1, hdrBox, NULL, tr.ssdoFbo2, ldrBox, &tr.ssdoBlurShader, color, 0 );
        
        // Y
        GLSL_SetUniformInt( &tr.ssdoBlurShader, UNIFORM_DELUXEMAP, TB_DELUXEMAP );
        GL_BindToTMU( tr.ssdoImage2, TB_DELUXEMAP );
        
        VectorSet4( local0, 0.0, 1.0, 0.0, 0.0 );
        GLSL_SetUniformVec4( &tr.ssdoBlurShader, UNIFORM_LOCAL0, local0 );
        
        FBO_Blit( tr.ssdoFbo2, hdrBox, NULL, tr.ssdoFbo1, ldrBox, &tr.ssdoBlurShader, color, 0 );
    }
    
    if( r_ssdo->integer == 3 )
        FBO_FastBlit( tr.ssdoFbo1, NULL, ldrFbo, NULL, GL_COLOR_BUFFER_BIT, GL_NEAREST );
}

void RB_ShowDepth( FBO_t* hdrFbo, ivec4_t hdrBox, FBO_t* ldrFbo, ivec4_t ldrBox )
{
    vec4_t color;
    
    // bloom
    color[0] =
        color[1] =
            color[2] = pow( 2, r_cameraExposure->value );
    color[3] = 1.0f;
    
    GLSL_BindProgram( &tr.showDepthShader );
    
    GLSL_SetUniformInt( &tr.showDepthShader, UNIFORM_DIFFUSEMAP, TB_DIFFUSEMAP );
    GL_BindToTMU( hdrFbo->colorImage[0], TB_DIFFUSEMAP );
    
    GLSL_SetUniformInt( &tr.showDepthShader, UNIFORM_SCREENDEPTHMAP, TB_LIGHTMAP );
    GL_BindToTMU( tr.renderDepthImage, TB_LIGHTMAP );
    
    GLSL_SetUniformMat4( &tr.showDepthShader, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection );
    
    {
        vec4_t viewInfo;
        //F32 zmax = 2048.0;
        F32 zmax = backEnd.viewParms.zFar;
        F32 ymax = zmax * tan( backEnd.viewParms.fovY * M_PI / 360.0f );
        F32 xmax = zmax * tan( backEnd.viewParms.fovX * M_PI / 360.0f );
        F32 zmin = r_znear->value;
        VectorSet4( viewInfo, zmin, zmax, zmax / zmin, backEnd.viewParms.fovX );
        GLSL_SetUniformVec4( &tr.showDepthShader, UNIFORM_VIEWINFO, viewInfo );
    }
    
    FBO_Blit( hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &tr.showDepthShader, color, 0 );
}

