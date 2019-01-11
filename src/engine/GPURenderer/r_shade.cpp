////////////////////////////////////////////////////////////////////////////////////////
// Copyright(C) 1999 - 2005 Id Software, Inc.
// Copyright(C) 2000 - 2013 Darklegion Development
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
// File name:   r_shade.cpp
// Version:     v1.01
// Created:
// Compilers:   Visual Studio 2017, gcc 7.3.0
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#include <OWLib/precompiled.h>

#if idppc_altivec && !defined(__APPLE__)
#include <altivec.h>
#endif

/*

  THIS ENTIRE FILE IS BACK END

  This file deals with applying shaders to surface data in the tess struct.
*/

/*
==================
R_DrawElements
==================
*/

void R_DrawElements( S32 numIndexes, U32 firstIndex, bool tesselation )
{
    if( r_tesselation->integer && tesselation )
    {
        S32 MaxPatchVertices = 0;
        qglGetIntegerv( GL_MAX_PATCH_VERTICES, &MaxPatchVertices );
        //CL_RefPrintf( PRINT_ALL, "Max supported patch vertices %d\n", MaxPatchVertices);
        qglPatchParameteri( GL_PATCH_VERTICES, 3 );
        qglDrawElements( GL_PATCHES, numIndexes, GL_INDEX_TYPE, BUFFER_OFFSET( firstIndex * sizeof( U32 ) ) );
    }
    else
    {
        qglDrawElements( GL_TRIANGLES, numIndexes, GL_INDEX_TYPE, BUFFER_OFFSET( firstIndex * sizeof( U32 ) ) );
    }
}

/*
=============================================================

SURFACE SHADERS

=============================================================
*/

shaderCommands_t	tess;


/*
=================
R_BindAnimatedImageToTMU

=================
*/
static void R_BindAnimatedImageToTMU( textureBundle_t* bundle, S32 tmu )
{
    S64 index;
    F64	v;
    
    if( bundle->isVideoMap )
    {
        CIN_RunCinematic( bundle->videoMapHandle );
        CIN_UploadCinematic( bundle->videoMapHandle );
        GL_BindToTMU( tr.scratchImage[bundle->videoMapHandle], tmu );
        return;
    }
    
    if( bundle->numImageAnimations <= 1 )
    {
        GL_BindToTMU( bundle->image[0], tmu );
        return;
    }
    
    // it is necessary to do this messy calc to make sure animations line up
    // exactly with waveforms of the same frequency
    v = tess.shaderTime * bundle->imageAnimationSpeed;
    index = v;
    
    if( index  < 0 )
    {
        index = 0;	// may happen with shader time offsets
    }
    index %= bundle->numImageAnimations;
    
    GL_BindToTMU( bundle->image[ index ], tmu );
}


/*
================
DrawTris

Draws triangle outlines for debugging
================
*/
static void DrawTris( shaderCommands_t* input )
{
    GL_BindToTMU( tr.whiteImage, TB_COLORMAP );
    
    GL_State( GLS_POLYMODE_LINE | GLS_DEPTHMASK_TRUE );
    glDepthRange( 0, 0 );
    
    {
        shaderProgram_t* sp = &tr.textureColorShader;
        vec4_t color;
        
        GLSL_BindProgram( sp );
        
        GLSL_SetUniformMat4( sp, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection );
        VectorSet4( color, 1, 1, 1, 1 );
        GLSL_SetUniformVec4( sp, UNIFORM_COLOR, color );
        GLSL_SetUniformInt( sp, UNIFORM_ALPHATEST, 0 );
        
        R_DrawElements( input->numIndexes, input->firstIndex, false );
    }
    
    glDepthRange( 0, 1 );
}


/*
================
DrawNormals

Draws vertex normals for debugging
================
*/
static void DrawNormals( shaderCommands_t* input )
{
    //FIXME: implement this
}

/*
==============
RB_BeginSurface

We must set some things up before beginning any tesselation,
because a surface may be forced to perform a RB_End due
to overflow.
==============
*/
void RB_BeginSurface( shader_t* shader, S32 fogNum, S32 cubemapIndex )
{
    shader_t* state = ( shader->remappedShader ) ? shader->remappedShader : shader;
    
    tess.numIndexes = 0;
    tess.firstIndex = 0;
    tess.numVertexes = 0;
    tess.shader = state;
    tess.fogNum = fogNum;
    tess.cubemapIndex = cubemapIndex;
    tess.dlightBits = 0;		// will be OR'd in by surface functions
    tess.pshadowBits = 0;       // will be OR'd in by surface functions
    tess.xstages = state->stages;
    tess.numPasses = state->numUnfoggedPasses;
    tess.currentStageIteratorFunc = state->optimalStageIteratorFunc;
    tess.useInternalVao = true;
    tess.useCacheVao = false;
    
    tess.shaderTime = backEnd.refdef.floatTime - tess.shader->timeOffset;
    if( tess.shader->clampTime && tess.shaderTime >= tess.shader->clampTime )
    {
        tess.shaderTime = tess.shader->clampTime;
    }
    
    if( backEnd.viewParms.flags & VPF_SHADOWMAP )
    {
        tess.currentStageIteratorFunc = RB_StageIteratorGeneric;
    }
}



extern F32 EvalWaveForm( const waveForm_t* wf );
extern F32 EvalWaveFormClamped( const waveForm_t* wf );


static void ComputeTexMods( shaderStage_t* pStage, S32 bundleNum, F32* outMatrix, F32* outOffTurb )
{
    S32 tm;
    F32 matrix[6], currentmatrix[6];
    textureBundle_t* bundle = &pStage->bundle[bundleNum];
    
    matrix[0] = 1.0f;
    matrix[2] = 0.0f;
    matrix[4] = 0.0f;
    matrix[1] = 0.0f;
    matrix[3] = 1.0f;
    matrix[5] = 0.0f;
    
    currentmatrix[0] = 1.0f;
    currentmatrix[2] = 0.0f;
    currentmatrix[4] = 0.0f;
    currentmatrix[1] = 0.0f;
    currentmatrix[3] = 1.0f;
    currentmatrix[5] = 0.0f;
    
    outMatrix[0] = 1.0f;
    outMatrix[2] = 0.0f;
    outMatrix[1] = 0.0f;
    outMatrix[3] = 1.0f;
    
    outOffTurb[0] = 0.0f;
    outOffTurb[1] = 0.0f;
    outOffTurb[2] = 0.0f;
    outOffTurb[3] = 0.0f;
    
    for( tm = 0; tm < bundle->numTexMods ; tm++ )
    {
        switch( bundle->texMods[tm].type )
        {
        
            case TMOD_NONE:
                tm = TR_MAX_TEXMODS;		// break out of for loop
                break;
                
            case TMOD_TURBULENT:
                RB_CalcTurbulentFactors( &bundle->texMods[tm].wave, &outOffTurb[2], &outOffTurb[3] );
                break;
                
            case TMOD_ENTITY_TRANSLATE:
                RB_CalcScrollTexMatrix( backEnd.currentEntity->e.shaderTexCoord, matrix );
                break;
                
            case TMOD_SCROLL:
                RB_CalcScrollTexMatrix( bundle->texMods[tm].scroll,
                                        matrix );
                break;
                
            case TMOD_SCALE:
                RB_CalcScaleTexMatrix( bundle->texMods[tm].scale,
                                       matrix );
                break;
                
            case TMOD_STRETCH:
                RB_CalcStretchTexMatrix( &bundle->texMods[tm].wave,
                                         matrix );
                break;
                
            case TMOD_TRANSFORM:
                RB_CalcTransformTexMatrix( &bundle->texMods[tm],
                                           matrix );
                break;
                
            case TMOD_ROTATE:
                RB_CalcRotateTexMatrix( bundle->texMods[tm].rotateSpeed,
                                        matrix );
                break;
                
            default:
                Com_Error( ERR_DROP, "ERROR: unknown texmod '%d' in shader '%s'", bundle->texMods[tm].type, tess.shader->name );
                break;
        }
        
        switch( bundle->texMods[tm].type )
        {
            case TMOD_NONE:
            case TMOD_TURBULENT:
            default:
                break;
                
            case TMOD_ENTITY_TRANSLATE:
            case TMOD_SCROLL:
            case TMOD_SCALE:
            case TMOD_STRETCH:
            case TMOD_TRANSFORM:
            case TMOD_ROTATE:
                outMatrix[0] = matrix[0] * currentmatrix[0] + matrix[2] * currentmatrix[1];
                outMatrix[1] = matrix[1] * currentmatrix[0] + matrix[3] * currentmatrix[1];
                
                outMatrix[2] = matrix[0] * currentmatrix[2] + matrix[2] * currentmatrix[3];
                outMatrix[3] = matrix[1] * currentmatrix[2] + matrix[3] * currentmatrix[3];
                
                outOffTurb[0] = matrix[0] * currentmatrix[4] + matrix[2] * currentmatrix[5] + matrix[4];
                outOffTurb[1] = matrix[1] * currentmatrix[4] + matrix[3] * currentmatrix[5] + matrix[5];
                
                currentmatrix[0] = outMatrix[0];
                currentmatrix[1] = outMatrix[1];
                currentmatrix[2] = outMatrix[2];
                currentmatrix[3] = outMatrix[3];
                currentmatrix[4] = outOffTurb[0];
                currentmatrix[5] = outOffTurb[1];
                break;
        }
    }
}


static void ComputeDeformValues( S32* deformGen, vec5_t deformParams )
{
    // u_DeformGen
    *deformGen = DGEN_NONE;
    if( !ShaderRequiresCPUDeforms( tess.shader ) )
    {
        deformStage_t*  ds;
        
        // only support the first one
        ds = &tess.shader->deforms[0];
        
        switch( ds->deformation )
        {
            case DEFORM_WAVE:
                *deformGen = ds->deformationWave.func;
                
                deformParams[0] = ds->deformationWave.base;
                deformParams[1] = ds->deformationWave.amplitude;
                deformParams[2] = ds->deformationWave.phase;
                deformParams[3] = ds->deformationWave.frequency;
                deformParams[4] = ds->deformationSpread;
                break;
                
            case DEFORM_BULGE:
                *deformGen = DGEN_BULGE;
                
                deformParams[0] = 0;
                deformParams[1] = ds->bulgeHeight; // amplitude
                deformParams[2] = ds->bulgeWidth;  // phase
                deformParams[3] = ds->bulgeSpeed;  // frequency
                deformParams[4] = 0;
                break;
                
            default:
                break;
        }
    }
}

#define __MERGE_DLIGHTS__

//F32 DLIGHT_SIZE_MULTIPLIER = 5.0;
F32 DLIGHT_SIZE_MULTIPLIER = 2.5;

#define __SINGLE_PASS__

static void ProjectDlightTexture( void )
{
    S32		l;
    vec5_t deformParams;
    S32 deformGen;
    
    if( !backEnd.refdef.num_dlights )
    {
        return;
    }
    
    //#define MAX_SHADER_DLIGHTS 8
#define MAX_SHADER_DLIGHTS 2
    
    ComputeDeformValues( &deformGen, deformParams );
    
#ifdef __SINGLE_PASS__
    
    S32 NUM_PASSES = ( backEnd.refdef.num_dlights / MAX_SHADER_DLIGHTS ) + 1;
    
    for( S32 i = 0; i < NUM_PASSES; i++ )
    {
        S32 START_POS = i * MAX_SHADER_DLIGHTS;
        
        backEnd.pc.c_dlightDraws++;
        
        shaderProgram_t* sp = &tr.dlightShader[deformGen == DGEN_NONE ? 0 : 1];
        
        GLSL_BindProgram( sp );
        
        GLSL_SetUniformMat4( sp, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection );
        
        GLSL_SetUniformFloat( sp, UNIFORM_VERTEXLERP, glState.vertexAttribsInterpolation );
        
        GLSL_SetUniformInt( sp, UNIFORM_DEFORMGEN, deformGen );
        if( deformGen != DGEN_NONE )
        {
            GLSL_SetUniformFloat5( sp, UNIFORM_DEFORMPARAMS, deformParams );
            GLSL_SetUniformFloat( sp, UNIFORM_TIME, tess.shaderTime );
        }
        
        for( l = START_POS; l < backEnd.refdef.num_dlights && l - START_POS < MAX_SHADER_DLIGHTS; l++ )
        {
            vec3_t		origin;
            F32		    scale;
            F32		    radius;
            dlight_t*	dl;
            vec4_t		vector;
            
            //if ( !( tess.dlightBits & ( 1 << l ) ) ) {
            //	continue;	// this surface definately doesn't have any of this light
            //}
            
            dl = &backEnd.refdef.dlights[l];
            VectorCopy( dl->transformed, origin );
            radius = dl->radius * DLIGHT_SIZE_MULTIPLIER;
            scale = 1.0f / radius;
            
            vector[0] = ( dl->color[0] );
            vector[1] = ( dl->color[1] );
            vector[2] = ( dl->color[2] );
            vector[3] = 0.2f;
            GLSL_SetUniformVec4( sp, UNIFORM_LIGHTCOLOR + ( l - START_POS ), vector );
            
            vector[0] = origin[0];
            vector[1] = origin[1];
            vector[2] = origin[2];
            vector[3] = scale;
            GLSL_SetUniformVec4( sp, UNIFORM_LIGHTORIGIN + ( l - START_POS ), vector );
        }
        
        GL_BindToTMU( tr.dlightImage, TB_COLORMAP );
        
        // include GLS_DEPTHFUNC_EQUAL so alpha tested surfaces don't add light
        // where they aren't rendered
        //if ( dl->additive ) {
        //	GL_State( GLS_ATEST_GT_0 | GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE | GLS_DEPTHFUNC_EQUAL );
        //}
        //else {
        GL_State( GLS_ATEST_GT_0 | GLS_SRCBLEND_DST_COLOR | GLS_DSTBLEND_ONE | GLS_DEPTHFUNC_EQUAL );
        //}
        
        R_DrawElements( tess.numIndexes, tess.firstIndex, false );
        
        backEnd.pc.c_totalIndexes += tess.numIndexes;
        backEnd.pc.c_dlightIndexes += tess.numIndexes;
        backEnd.pc.c_dlightVertexes += tess.numVertexes;
    }
    
#else //!__SINGLE_PASS__
#ifndef __MERGE_DLIGHTS__
    
    for( l = 0; l < backEnd.refdef.num_dlights; l++ )
    {
        vec3_t origin;
        F32	scale;
        F32	radius;
        dlight_t*	dl;
        shaderProgram_t* sp;
        vec4_t vector;
    
        //if ( !( tess.dlightBits & ( 1 << l ) ) ) {
        //	continue;	// this surface definately doesn't have any of this light
        //}
    
        dl = &backEnd.refdef.dlights[l];
        VectorCopy( dl->transformed, origin );
        radius = dl->radius * DLIGHT_SIZE_MULTIPLIER;
        scale = 1.0f / radius;
    
        sp = &tr.dlightShader[deformGen == DGEN_NONE ? 0 : 1];
    
        backEnd.pc.c_dlightDraws++;
    
        GLSL_BindProgram( sp );
    
        GLSL_SetUniformMat4( sp, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection );
    
        GLSL_SetUniformFloat( sp, UNIFORM_VERTEXLERP, glState.vertexAttribsInterpolation );
    
        GLSL_SetUniformInt( sp, UNIFORM_DEFORMGEN, deformGen );
        if( deformGen != DGEN_NONE )
        {
            GLSL_SetUniformFloat5( sp, UNIFORM_DEFORMPARAMS, deformParams );
            GLSL_SetUniformFloat( sp, UNIFORM_TIME, tess.shaderTime );
        }
    
        vector[0] = ( dl->color[0] );
        vector[1] = ( dl->color[1] );
        vector[2] = ( dl->color[2] );
        vector[3] = 0.2f;
        GLSL_SetUniformVec4( sp, UNIFORM_LIGHTCOLOR, vector );
    
        vector[0] = origin[0];
        vector[1] = origin[1];
        vector[2] = origin[2];
        vector[3] = scale;
        GLSL_SetUniformVec4( sp, UNIFORM_LIGHTORIGIN, vector );
    
        GLSL_SetUniformFloat( sp, UNIFORM_LIGHTRADIUS, dl->radius );
    
        /*
        {
        vec2_t screensize;
        screensize[0] = tr.dlightImage->width;
        screensize[1] = tr.dlightImage->height;
    
        GLSL_SetUniformVec2(sp, UNIFORM_DIMENSIONS, screensize);
        }
        */
    
        GL_BindToTMU( tr.dlightImage, TB_COLORMAP );
    
        // include GLS_DEPTHFUNC_EQUAL so alpha tested surfaces don't add light
        // where they aren't rendered
        if( dl->additive )
        {
            GL_State( GLS_ATEST_GT_0 | GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE | GLS_DEPTHFUNC_EQUAL );
        }
        else
        {
            GL_State( GLS_ATEST_GT_0 | GLS_SRCBLEND_DST_COLOR | GLS_DSTBLEND_ONE | GLS_DEPTHFUNC_EQUAL );
        }
    
        R_DrawElements( tess.numIndexes, tess.firstIndex );
    
        backEnd.pc.c_totalIndexes += tess.numIndexes;
        backEnd.pc.c_dlightIndexes += tess.numIndexes;
        backEnd.pc.c_dlightVertexes += tess.numVertexes;
    }
#else // __MERGE_DLIGHTS__
    
    {
        bool	    SHOULD_MERGE[256];
        bool	    COMPLETED_MERGE[256];
        dlight_t	MERGED_DLIGHTS[256];
        S32			MERGED_DLIGHT_COUNT[256];
        S32			NUM_MERGED_DLIGHTS = 0;
        S32			j = 0;
    
        ::memset( &SHOULD_MERGE, false, sizeof( bool ) * 256 );
        ::memset( &COMPLETED_MERGE, false, sizeof( bool ) * 256 );
        ::memset( &MERGED_DLIGHT_COUNT, 0, sizeof( S32 ) * 256 );
    
        for( l = 0; l < backEnd.refdef.num_dlights; l++ )
        {
            dlight_t* dl = &backEnd.refdef.dlights[l];
    
            // Start search for mergeable lights at the next light from this one...
            for( j = l + 1; j < backEnd.refdef.num_dlights; j++ )
            {
                dlight_t* dl2 = &backEnd.refdef.dlights[j];
    
                if( Distance( dl2->origin, dl->origin ) <= dl->radius * 2.0 )
                {
                    SHOULD_MERGE[j] = true;
                }
            }
        }
    
        // Add all lights that should not be merged with another...
        for( l = 0; l < backEnd.refdef.num_dlights; l++ )
        {
            if( !SHOULD_MERGE[l] )
            {
                dlight_t*	dl;
    
                // Copy this dlight to our list...
                ::memcpy( &MERGED_DLIGHTS[NUM_MERGED_DLIGHTS], &backEnd.refdef.dlights[l], sizeof( dlight_t ) );
                MERGED_DLIGHT_COUNT[NUM_MERGED_DLIGHTS]++;
    
                dl = &MERGED_DLIGHTS[NUM_MERGED_DLIGHTS];
    
                // And merge any lights close enough with this one...
                for( j = l; j < backEnd.refdef.num_dlights; j++ )
                {
                    dlight_t* dl2 = &backEnd.refdef.dlights[j];
    
                    if( !SHOULD_MERGE[j] || COMPLETED_MERGE[j] ) continue;
    
                    if( Distance( dl2->origin, dl->origin ) <= dl->radius )
                    {
                        // Merge these two...
                        dl->color[0] += dl2->color[0];
                        dl->color[1] += dl2->color[1];
                        dl->color[2] += dl2->color[2];
    
                        // TODO: Move the light origin...
    
                        // mark this light as merged...
                        COMPLETED_MERGE[j] = true;
                        // increase counter of how many lights have been added for this merged light...
                        MERGED_DLIGHT_COUNT[NUM_MERGED_DLIGHTS]++;
                    }
                }
    
                NUM_MERGED_DLIGHTS++;
            }
        }
    
        // Finish up by adjusting merged lights color and radius...
        for( l = 0; l < NUM_MERGED_DLIGHTS; l++ )
        {
            dlight_t*	dl = &MERGED_DLIGHTS[l];
    
            // Average out the colors...
            dl->color[0] /= MERGED_DLIGHT_COUNT[l];
            dl->color[1] /= MERGED_DLIGHT_COUNT[l];
            dl->color[2] /= MERGED_DLIGHT_COUNT[l];
    
            // Increase the radius...
            dl->radius *= ( MERGED_DLIGHT_COUNT[l] );
        }
    
        //CL_RefPrintf(PRINT_ALL, "%i dlights were merged into %i dlights.\n", backEnd.refdef.num_dlights, NUM_MERGED_DLIGHTS);
    
        // Now display the merged lights...
        for( l = 0; l < NUM_MERGED_DLIGHTS; l++ )
        {
            shaderProgram_t* sp;
            vec4_t vector;
            dlight_t*	dl = &MERGED_DLIGHTS[l];
    
            VectorCopy( dl->transformed, origin );
            radius = dl->radius * DLIGHT_SIZE_MULTIPLIER;
            scale = 1.0f / radius;
    
            sp = &tr.dlightShader[deformGen == DGEN_NONE ? 0 : 1];
    
            backEnd.pc.c_dlightDraws++;
    
            GLSL_BindProgram( sp );
    
            GLSL_SetUniformMat4( sp, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection );
    
            GLSL_SetUniformFloat( sp, UNIFORM_VERTEXLERP, glState.vertexAttribsInterpolation );
    
            GLSL_SetUniformInt( sp, UNIFORM_DEFORMGEN, deformGen );
            if( deformGen != DGEN_NONE )
            {
                GLSL_SetUniformFloat5( sp, UNIFORM_DEFORMPARAMS, deformParams );
                GLSL_SetUniformFloat( sp, UNIFORM_TIME, tess.shaderTime );
            }
    
            vector[0] = ( dl->color[0] );
            vector[1] = ( dl->color[1] );
            vector[2] = ( dl->color[2] );
            vector[3] = 0.2f;
            GLSL_SetUniformVec4( sp, UNIFORM_LIGHTCOLOR, vector );
    
            vector[0] = origin[0];
            vector[1] = origin[1];
            vector[2] = origin[2];
            vector[3] = scale;
            GLSL_SetUniformVec4( sp, UNIFORM_LIGHTORIGIN, vector );
    
            GLSL_SetUniformFloat( sp, UNIFORM_LIGHTRADIUS, dl->radius );
    
            GL_BindToTMU( tr.dlightImage, TB_COLORMAP );
    
            // include GLS_DEPTHFUNC_EQUAL so alpha tested surfaces don't add light
            // where they aren't rendered
            if( dl->additive )
            {
                GL_State( GLS_ATEST_GT_0 | GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE | GLS_DEPTHFUNC_EQUAL );
            }
            else
            {
                GL_State( GLS_ATEST_GT_0 | GLS_SRCBLEND_DST_COLOR | GLS_DSTBLEND_ONE | GLS_DEPTHFUNC_EQUAL );
            }
    
            R_DrawElements( tess.numIndexes, tess.firstIndex );
    
            backEnd.pc.c_totalIndexes += tess.numIndexes;
            backEnd.pc.c_dlightIndexes += tess.numIndexes;
            backEnd.pc.c_dlightVertexes += tess.numVertexes;
        }
    }
#endif //__MERGE_DLIGHTS__
#endif //__SINGLE_PASS__
}

static void ComputeShaderColors( shaderStage_t* pStage, vec4_t baseColor, vec4_t vertColor, S32 blend )
{
    bool isBlend = ( ( blend & GLS_SRCBLEND_BITS ) == GLS_SRCBLEND_DST_COLOR )
                   || ( ( blend & GLS_SRCBLEND_BITS ) == GLS_SRCBLEND_ONE_MINUS_DST_COLOR )
                   || ( ( blend & GLS_DSTBLEND_BITS ) == GLS_DSTBLEND_SRC_COLOR )
                   || ( ( blend & GLS_DSTBLEND_BITS ) == GLS_DSTBLEND_ONE_MINUS_SRC_COLOR );
                   
    bool is2DDraw = backEnd.currentEntity == &backEnd.entity2D;
    
    F32 overbright = ( isBlend || is2DDraw ) ? 1.0f : ( F32 )( 1 << tr.overbrightBits );
    
    fog_t* fog;
    
    baseColor[0] =
        baseColor[1] =
            baseColor[2] =
                baseColor[3] = 1.0f;
                
    vertColor[0] =
        vertColor[1] =
            vertColor[2] =
                vertColor[3] = 0.0f;
                
    //
    // rgbGen
    //
    switch( pStage->rgbGen )
    {
        case CGEN_EXACT_VERTEX:
        case CGEN_EXACT_VERTEX_LIT:
            baseColor[0] =
                baseColor[1] =
                    baseColor[2] =
                        baseColor[3] = 0.0f;
                        
            vertColor[0] =
                vertColor[1] =
                    vertColor[2] = overbright;
            vertColor[3] = 1.0f;
            break;
        case CGEN_CONST:
            baseColor[0] = pStage->constantColor[0] / 255.0f;
            baseColor[1] = pStage->constantColor[1] / 255.0f;
            baseColor[2] = pStage->constantColor[2] / 255.0f;
            baseColor[3] = pStage->constantColor[3] / 255.0f;
            break;
        case CGEN_VERTEX:
        case CGEN_VERTEX_LIT:
            baseColor[0] =
                baseColor[1] =
                    baseColor[2] =
                        baseColor[3] = 0.0f;
                        
            vertColor[0] =
                vertColor[1] =
                    vertColor[2] =
                        vertColor[3] = 1.0f;
            break;
        case CGEN_ONE_MINUS_VERTEX:
            baseColor[0] =
                baseColor[1] =
                    baseColor[2] = 1.0f;
                    
            vertColor[0] =
                vertColor[1] =
                    vertColor[2] = -1.0f;
            break;
        case CGEN_FOG:
            fog = tr.world->fogs + tess.fogNum;
            
            baseColor[0] = ( ( U8* )( &fog->colorInt ) )[0] / 255.0f;
            baseColor[1] = ( ( U8* )( &fog->colorInt ) )[1] / 255.0f;
            baseColor[2] = ( ( U8* )( &fog->colorInt ) )[2] / 255.0f;
            baseColor[3] = ( ( U8* )( &fog->colorInt ) )[3] / 255.0f;
            break;
        case CGEN_WAVEFORM:
            baseColor[0] =
                baseColor[1] =
                    baseColor[2] = RB_CalcWaveColorSingle( &pStage->rgbWave );
            break;
        case CGEN_ENTITY:
            if( backEnd.currentEntity )
            {
                baseColor[0] = ( ( U8* )backEnd.currentEntity->e.shaderRGBA )[0] / 255.0f;
                baseColor[1] = ( ( U8* )backEnd.currentEntity->e.shaderRGBA )[1] / 255.0f;
                baseColor[2] = ( ( U8* )backEnd.currentEntity->e.shaderRGBA )[2] / 255.0f;
                baseColor[3] = ( ( U8* )backEnd.currentEntity->e.shaderRGBA )[3] / 255.0f;
            }
            break;
        case CGEN_ONE_MINUS_ENTITY:
            if( backEnd.currentEntity )
            {
                baseColor[0] = 1.0f - ( ( U8* )backEnd.currentEntity->e.shaderRGBA )[0] / 255.0f;
                baseColor[1] = 1.0f - ( ( U8* )backEnd.currentEntity->e.shaderRGBA )[1] / 255.0f;
                baseColor[2] = 1.0f - ( ( U8* )backEnd.currentEntity->e.shaderRGBA )[2] / 255.0f;
                baseColor[3] = 1.0f - ( ( U8* )backEnd.currentEntity->e.shaderRGBA )[3] / 255.0f;
            }
            break;
        case CGEN_IDENTITY:
        case CGEN_LIGHTING_DIFFUSE:
            baseColor[0] =
                baseColor[1] =
                    baseColor[2] = overbright;
            break;
        case CGEN_IDENTITY_LIGHTING:
        case CGEN_BAD:
            break;
    }
    
    //
    // alphaGen
    //
    switch( pStage->alphaGen )
    {
        case AGEN_SKIP:
            break;
        case AGEN_CONST:
            baseColor[3] = pStage->constantColor[3] / 255.0f;
            vertColor[3] = 0.0f;
            break;
        case AGEN_WAVEFORM:
            baseColor[3] = RB_CalcWaveAlphaSingle( &pStage->alphaWave );
            vertColor[3] = 0.0f;
            break;
        case AGEN_ENTITY:
            if( backEnd.currentEntity )
            {
                baseColor[3] = ( ( U8* )backEnd.currentEntity->e.shaderRGBA )[3] / 255.0f;
            }
            vertColor[3] = 0.0f;
            break;
        case AGEN_ONE_MINUS_ENTITY:
            if( backEnd.currentEntity )
            {
                baseColor[3] = 1.0f - ( ( U8* )backEnd.currentEntity->e.shaderRGBA )[3] / 255.0f;
            }
            vertColor[3] = 0.0f;
            break;
        case AGEN_VERTEX:
            baseColor[3] = 0.0f;
            vertColor[3] = 1.0f;
            break;
        case AGEN_ONE_MINUS_VERTEX:
            baseColor[3] = 1.0f;
            vertColor[3] = -1.0f;
            break;
        case AGEN_IDENTITY:
        case AGEN_LIGHTING_SPECULAR:
        case AGEN_PORTAL:
            // Done entirely in vertex program
            baseColor[3] = 1.0f;
            vertColor[3] = 0.0f;
            break;
        case AGEN_NORMALZFADE:
            baseColor[3] = pStage->constantColor[3] / 255.0f;
            if( backEnd.currentEntity && backEnd.currentEntity->e.hModel )
            {
                baseColor[3] *= ( ( U8* )backEnd.currentEntity->e.shaderRGBA )[3] / 255.0f;
            }
            vertColor[3] = 0.0f;
            break;
    }
    
    // FIXME: find some way to implement this.
#if 0
    // if in greyscale rendering mode turn all color values into greyscale.
    if( r_greyscale->integer )
    {
        S32 scale;
        
        for( i = 0; i < tess.numVertexes; i++ )
        {
            scale = ( tess.svars.colors[i][0] + tess.svars.colors[i][1] + tess.svars.colors[i][2] ) / 3;
            tess.svars.colors[i][0] = tess.svars.colors[i][1] = tess.svars.colors[i][2] = scale;
        }
    }
#endif
}


static void ComputeFogValues( vec4_t fogDistanceVector, vec4_t fogDepthVector, F32* eyeT )
{
    // from RB_CalcFogTexCoords()
    fog_t*  fog;
    vec3_t  local;
    
    if( !tess.fogNum )
        return;
        
    fog = tr.world->fogs + tess.fogNum;
    
    VectorSubtract( backEnd.orientation.origin, backEnd.viewParms.orientation.origin, local );
    fogDistanceVector[0] = -backEnd.orientation.modelMatrix[2];
    fogDistanceVector[1] = -backEnd.orientation.modelMatrix[6];
    fogDistanceVector[2] = -backEnd.orientation.modelMatrix[10];
    fogDistanceVector[3] = DotProduct( local, backEnd.viewParms.orientation.axis[0] );
    
    // scale the fog vectors based on the fog's thickness
    VectorScale4( fogDistanceVector, fog->tcScale, fogDistanceVector );
    
    // rotate the gradient vector for this orientation
    if( fog->hasSurface )
    {
        fogDepthVector[0] = fog->surface[0] * backEnd.orientation.axis[0][0] +
                            fog->surface[1] * backEnd.orientation.axis[0][1] + fog->surface[2] * backEnd.orientation.axis[0][2];
        fogDepthVector[1] = fog->surface[0] * backEnd.orientation.axis[1][0] +
                            fog->surface[1] * backEnd.orientation.axis[1][1] + fog->surface[2] * backEnd.orientation.axis[1][2];
        fogDepthVector[2] = fog->surface[0] * backEnd.orientation.axis[2][0] +
                            fog->surface[1] * backEnd.orientation.axis[2][1] + fog->surface[2] * backEnd.orientation.axis[2][2];
        fogDepthVector[3] = -fog->surface[3] + DotProduct( backEnd.orientation.origin, fog->surface );
        
        *eyeT = DotProduct( backEnd.orientation.viewOrigin, fogDepthVector ) + fogDepthVector[3];
    }
    else
    {
        *eyeT = 1;	// non-surface fog always has eye inside
    }
}


static void ComputeFogColorMask( shaderStage_t* pStage, vec4_t fogColorMask )
{
    switch( pStage->adjustColorsForFog )
    {
        case ACFF_MODULATE_RGB:
            fogColorMask[0] =
                fogColorMask[1] =
                    fogColorMask[2] = 1.0f;
            fogColorMask[3] = 0.0f;
            break;
        case ACFF_MODULATE_ALPHA:
            fogColorMask[0] =
                fogColorMask[1] =
                    fogColorMask[2] = 0.0f;
            fogColorMask[3] = 1.0f;
            break;
        case ACFF_MODULATE_RGBA:
            fogColorMask[0] =
                fogColorMask[1] =
                    fogColorMask[2] =
                        fogColorMask[3] = 1.0f;
            break;
        default:
            fogColorMask[0] =
                fogColorMask[1] =
                    fogColorMask[2] =
                        fogColorMask[3] = 0.0f;
            break;
    }
}

static void ForwardDlight( void )
{
    S32		l;
    //vec3_t	origin;
    //F32	scale;
    F32	radius;
    
    S32 deformGen;
    vec5_t deformParams;
    
    vec4_t fogDistanceVector, fogDepthVector = { 0, 0, 0, 0 };
    F32 eyeT = 0;
    
    shaderCommands_t* input = &tess;
    shaderStage_t* pStage = tess.xstages[tess.shader->lightingStage];
    
    if( !pStage )
    {
        return;
    }
    
    if( !backEnd.refdef.num_dlights )
    {
        return;
    }
    
    ComputeDeformValues( &deformGen, deformParams );
    
    ComputeFogValues( fogDistanceVector, fogDepthVector, &eyeT );
    
    for( l = 0; l < backEnd.refdef.num_dlights; l++ )
    {
        dlight_t*	dl;
        shaderProgram_t* sp;
        vec4_t vector;
        vec4_t texMatrix;
        vec4_t texOffTurb;
        
        if( !( tess.dlightBits & ( 1 << l ) ) )
        {
            continue;	// this surface definately doesn't have any of this light
        }
        
        dl = &backEnd.refdef.dlights[l];
        //VectorCopy( dl->transformed, origin );
        radius = dl->radius;
        //scale = 1.0f / radius;
        
        //if (pStage->glslShaderGroup == tr.lightallShader)
        {
            S32 index = pStage->glslShaderIndex;
            
            index &= ~LIGHTDEF_LIGHTTYPE_MASK;
            index |= LIGHTDEF_USE_LIGHT_VECTOR;
            
            sp = &tr.lightallShader[index];
        }
        
        backEnd.pc.c_lightallDraws++;
        
        GLSL_BindProgram( sp );
        
        GLSL_SetUniformMat4( sp, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection );
        GLSL_SetUniformVec3( sp, UNIFORM_VIEWORIGIN, backEnd.viewParms.orientation.origin );
        GLSL_SetUniformVec3( sp, UNIFORM_LOCALVIEWORIGIN, backEnd.orientation.viewOrigin );
        
        GLSL_SetUniformFloat( sp, UNIFORM_VERTEXLERP, glState.vertexAttribsInterpolation );
        
        GLSL_SetUniformInt( sp, UNIFORM_DEFORMGEN, deformGen );
        if( deformGen != DGEN_NONE )
        {
            GLSL_SetUniformFloat5( sp, UNIFORM_DEFORMPARAMS, deformParams );
            GLSL_SetUniformFloat( sp, UNIFORM_TIME, tess.shaderTime );
        }
        
        if( input->fogNum )
        {
            vec4_t fogColorMask;
            
            GLSL_SetUniformVec4( sp, UNIFORM_FOGDISTANCE, fogDistanceVector );
            GLSL_SetUniformVec4( sp, UNIFORM_FOGDEPTH, fogDepthVector );
            GLSL_SetUniformFloat( sp, UNIFORM_FOGEYET, eyeT );
            
            ComputeFogColorMask( pStage, fogColorMask );
            
            GLSL_SetUniformVec4( sp, UNIFORM_FOGCOLORMASK, fogColorMask );
        }
        
        {
            vec4_t baseColor;
            vec4_t vertColor;
            
            ComputeShaderColors( pStage, baseColor, vertColor, GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE );
            
            GLSL_SetUniformVec4( sp, UNIFORM_BASECOLOR, baseColor );
            GLSL_SetUniformVec4( sp, UNIFORM_VERTCOLOR, vertColor );
        }
        
        if( pStage->alphaGen == AGEN_PORTAL )
        {
            GLSL_SetUniformFloat( sp, UNIFORM_PORTALRANGE, tess.shader->portalRange );
        }
        
        GLSL_SetUniformInt( sp, UNIFORM_COLORGEN, pStage->rgbGen );
        GLSL_SetUniformInt( sp, UNIFORM_ALPHAGEN, pStage->alphaGen );
        
        GLSL_SetUniformVec3( sp, UNIFORM_DIRECTEDLIGHT, dl->color );
        
        VectorSet( vector, 0, 0, 0 );
        GLSL_SetUniformVec3( sp, UNIFORM_AMBIENTLIGHT, vector );
        
        VectorCopy( dl->origin, vector );
        vector[3] = 1.0f;
        GLSL_SetUniformVec4( sp, UNIFORM_LIGHTORIGIN, vector );
        
        GLSL_SetUniformFloat( sp, UNIFORM_LIGHTRADIUS, radius );
        
        GLSL_SetUniformVec4( sp, UNIFORM_NORMALSCALE, pStage->normalScale );
        GLSL_SetUniformVec4( sp, UNIFORM_SPECULARSCALE, pStage->specularScale );
        
        // include GLS_DEPTHFUNC_EQUAL so alpha tested surfaces don't add light
        // where they aren't rendered
        GL_State( GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE | GLS_DEPTHFUNC_EQUAL );
        GLSL_SetUniformInt( sp, UNIFORM_ALPHATEST, 0 );
        
        GLSL_SetUniformMat4( sp, UNIFORM_MODELMATRIX, backEnd.orientation.transformMatrix );
        
        if( pStage->bundle[TB_DIFFUSEMAP].image[0] )
            R_BindAnimatedImageToTMU( &pStage->bundle[TB_DIFFUSEMAP], TB_DIFFUSEMAP );
            
        // bind textures that are sampled and used in the glsl shader, and
        // bind whiteImage to textures that are sampled but zeroed in the glsl shader
        //
        // alternatives:
        //  - use the last bound texture
        //     -> costs more to sample a higher res texture then throw out the result
        //  - disable texture sampling in glsl shader with #ifdefs, as before
        //     -> increases the number of shaders that must be compiled
        //
        
        if( pStage->bundle[TB_NORMALMAP].image[0] )
        {
            R_BindAnimatedImageToTMU( &pStage->bundle[TB_NORMALMAP], TB_NORMALMAP );
        }
        else if( r_normalMapping->integer )
            GL_BindToTMU( tr.whiteImage, TB_NORMALMAP );
            
        if( pStage->bundle[TB_SPECULARMAP].image[0] )
        {
            R_BindAnimatedImageToTMU( &pStage->bundle[TB_SPECULARMAP], TB_SPECULARMAP );
        }
        else if( r_specularMapping->integer )
            GL_BindToTMU( tr.whiteImage, TB_SPECULARMAP );
            
        {
            vec4_t enableTextures;
            
            VectorSet4( enableTextures, 0.0f, 0.0f, 0.0f, 0.0f );
            GLSL_SetUniformVec4( sp, UNIFORM_ENABLETEXTURES, enableTextures );
        }
        
        if( r_dlightMode->integer >= 2 )
            GL_BindToTMU( tr.shadowCubemaps[l], TB_SHADOWMAP );
            
        ComputeTexMods( pStage, TB_DIFFUSEMAP, texMatrix, texOffTurb );
        GLSL_SetUniformVec4( sp, UNIFORM_DIFFUSETEXMATRIX, texMatrix );
        GLSL_SetUniformVec4( sp, UNIFORM_DIFFUSETEXOFFTURB, texOffTurb );
        
        GLSL_SetUniformInt( sp, UNIFORM_TCGEN0, pStage->bundle[0].tcGen );
        
        //
        // draw
        //
        
        R_DrawElements( input->numIndexes, input->firstIndex, false );
        
        backEnd.pc.c_totalIndexes += tess.numIndexes;
        backEnd.pc.c_dlightIndexes += tess.numIndexes;
        backEnd.pc.c_dlightVertexes += tess.numVertexes;
    }
}


static void ProjectPshadowVBOGLSL( void )
{
    S32		l;
    vec3_t	origin;
    F32	radius;
    
    S32 deformGen;
    vec5_t deformParams;
    
    shaderCommands_t* input = &tess;
    
    if( !backEnd.refdef.num_pshadows )
    {
        return;
    }
    
    ComputeDeformValues( &deformGen, deformParams );
    
    for( l = 0; l < backEnd.refdef.num_pshadows; l++ )
    {
        pshadow_t*	ps;
        shaderProgram_t* sp;
        vec4_t vector;
        
        if( !( tess.pshadowBits & ( 1 << l ) ) )
        {
            continue;	// this surface definately doesn't have any of this shadow
        }
        
        ps = &backEnd.refdef.pshadows[l];
        VectorCopy( ps->lightOrigin, origin );
        radius = ps->lightRadius;
        
        sp = &tr.pshadowShader;
        
        GLSL_BindProgram( sp );
        
        GLSL_SetUniformMat4( sp, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection );
        
        VectorCopy( origin, vector );
        vector[3] = 1.0f;
        GLSL_SetUniformVec4( sp, UNIFORM_LIGHTORIGIN, vector );
        
        VectorScale( ps->lightViewAxis[0], 1.0f / ps->viewRadius, vector );
        GLSL_SetUniformVec3( sp, UNIFORM_LIGHTFORWARD, vector );
        
        VectorScale( ps->lightViewAxis[1], 1.0f / ps->viewRadius, vector );
        GLSL_SetUniformVec3( sp, UNIFORM_LIGHTRIGHT, vector );
        
        VectorScale( ps->lightViewAxis[2], 1.0f / ps->viewRadius, vector );
        GLSL_SetUniformVec3( sp, UNIFORM_LIGHTUP, vector );
        
        GLSL_SetUniformFloat( sp, UNIFORM_LIGHTRADIUS, radius );
        
        // include GLS_DEPTHFUNC_EQUAL so alpha tested surfaces don't add light
        // where they aren't rendered
        GL_State( GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA | GLS_DEPTHFUNC_EQUAL );
        GLSL_SetUniformInt( sp, UNIFORM_ALPHATEST, 0 );
        
        GL_BindToTMU( tr.pshadowMaps[l], TB_DIFFUSEMAP );
        
        //
        // draw
        //
        
        R_DrawElements( input->numIndexes, input->firstIndex, false );
        
        backEnd.pc.c_totalIndexes += tess.numIndexes;
        //backEnd.pc.c_dlightIndexes += tess.numIndexes;
    }
}


/*
===================
RB_FogPass

Blends a fog texture on top of everything else
===================
*/
static void RB_FogPass( void )
{
    fog_t*		fog;
    vec4_t  color;
    vec4_t	fogDistanceVector, fogDepthVector = {0, 0, 0, 0};
    F32	eyeT = 0;
    shaderProgram_t* sp;
    
    S32 deformGen;
    vec5_t deformParams;
    
    ComputeDeformValues( &deformGen, deformParams );
    
    {
        S32 index = 0;
        
        if( deformGen != DGEN_NONE )
            index |= FOGDEF_USE_DEFORM_VERTEXES;
            
        if( glState.vertexAnimation )
            index |= FOGDEF_USE_VERTEX_ANIMATION;
            
        sp = &tr.fogShader[index];
    }
    
    backEnd.pc.c_fogDraws++;
    
    GLSL_BindProgram( sp );
    
    fog = tr.world->fogs + tess.fogNum;
    
    GLSL_SetUniformMat4( sp, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection );
    
    GLSL_SetUniformFloat( sp, UNIFORM_VERTEXLERP, glState.vertexAttribsInterpolation );
    
    GLSL_SetUniformInt( sp, UNIFORM_DEFORMGEN, deformGen );
    if( deformGen != DGEN_NONE )
    {
        GLSL_SetUniformFloat5( sp, UNIFORM_DEFORMPARAMS, deformParams );
        GLSL_SetUniformFloat( sp, UNIFORM_TIME, tess.shaderTime );
    }
    
    color[0] = ( ( U8* )( &fog->colorInt ) )[0] / 255.0f;
    color[1] = ( ( U8* )( &fog->colorInt ) )[1] / 255.0f;
    color[2] = ( ( U8* )( &fog->colorInt ) )[2] / 255.0f;
    color[3] = ( ( U8* )( &fog->colorInt ) )[3] / 255.0f;
    GLSL_SetUniformVec4( sp, UNIFORM_COLOR, color );
    
    ComputeFogValues( fogDistanceVector, fogDepthVector, &eyeT );
    
    GLSL_SetUniformVec4( sp, UNIFORM_FOGDISTANCE, fogDistanceVector );
    GLSL_SetUniformVec4( sp, UNIFORM_FOGDEPTH, fogDepthVector );
    GLSL_SetUniformFloat( sp, UNIFORM_FOGEYET, eyeT );
    
    if( tess.shader->fogPass == FP_EQUAL )
    {
        GL_State( GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA | GLS_DEPTHFUNC_EQUAL );
    }
    else
    {
        GL_State( GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA );
    }
    GLSL_SetUniformInt( sp, UNIFORM_ALPHATEST, 0 );
    
    R_DrawElements( tess.numIndexes, tess.firstIndex, false );
}

static U32 RB_CalcShaderVertexAttribs( shaderCommands_t* input )
{
    U32 vertexAttribs = input->shader->vertexAttribs;
    
    if( glState.vertexAnimation )
    {
        vertexAttribs |= ATTR_POSITION2;
        if( vertexAttribs & ATTR_NORMAL )
        {
            vertexAttribs |= ATTR_NORMAL2;
            vertexAttribs |= ATTR_TANGENT2;
        }
    }
    
    return vertexAttribs;
}

S32 overlaySwayTime = 0;
bool overlaySwayDown = false;
F32 overlaySway = 0.0;

void RB_AdvanceOverlaySway( void )
{
    if( overlaySwayTime > CL_ScaledMilliseconds() )
        return;
        
    if( overlaySwayDown )
    {
        overlaySway -= 0.00016;
        
        if( overlaySway < 0.0 )
        {
            overlaySway += 0.00032;
            overlaySwayDown = false;
        }
    }
    else
    {
        overlaySway += 0.00016;
        
        if( overlaySway > 0.0016 )
        {
            overlaySway -= 0.00032;
            overlaySwayDown = true;
        }
    }
    
    overlaySwayTime = CL_ScaledMilliseconds() + 50;
}

void RB_SetMaterialBasedProperties( shaderProgram_t* sp, shaderStage_t* pStage )
{
    vec4_t	local1, local3, local4, local5;
    F32	specularScale = 1.0;
    F32	materialType = 0.0;
    F32   parallaxScale = 1.0;
    F32	cubemapScale = 0.0;
    F32	isMetalic = 0.0;
    F32	useSteepParallax = 0.0;
    F32	hasOverlay = 0.0;
    F32	doSway = 0.0;
    F32	hasSteepMap = 0.0;
    
    if( pStage->bundle[TB_OVERLAYMAP].overlayLoaded
            && pStage->hasRealOverlayMap
            && pStage->bundle[TB_OVERLAYMAP].image[0] != tr.whiteImage )
    {
        hasOverlay = 1.0;
    }
    
    if( pStage->bundle[TB_STEEPMAP].steepMapLoaded
            && pStage->hasRealSteepMap
            && pStage->bundle[TB_STEEPMAP].image[0] != tr.blackImage )
    {
        hasSteepMap = 1.0;
    }
    
    if( pStage->isWater )
    {
        specularScale = 1.5;
        materialType = ( F32 )MATERIAL_WATER;
        parallaxScale = 2.0;
    }
    else
    {
        switch( tess.shader->surfaceFlags & MATERIAL_MASK )
        {
            case MATERIAL_WATER:			// 13			// light covering of water on a surface
                specularScale = 1.0;
                cubemapScale = 1.5;
                materialType = ( F32 )MATERIAL_WATER;
                parallaxScale = 2.0;
                break;
            case MATERIAL_SHORTGRASS:		// 5			// manicured lawn
                specularScale = 0.53;
                cubemapScale = 0.0;
                materialType = ( F32 )MATERIAL_SHORTGRASS;
                parallaxScale = 2.5;
                break;
            case MATERIAL_LONGGRASS:		// 6			// long jungle grass
                specularScale = 0.5;
                cubemapScale = 0.0;
                materialType = ( F32 )MATERIAL_LONGGRASS;
                parallaxScale = 3.0;
                break;
            case MATERIAL_SAND:				// 8			// sandy beach
                specularScale = 0.0;
                cubemapScale = 0.0;
                materialType = ( F32 )MATERIAL_SAND;
                parallaxScale = 2.5;
                break;
            case MATERIAL_CARPET:			// 27			// lush carpet
                specularScale = 0.0;
                cubemapScale = 0.0;
                materialType = ( F32 )MATERIAL_CARPET;
                parallaxScale = 2.5;
                break;
            case MATERIAL_GRAVEL:			// 9			// lots of small stones
                specularScale = 0.0;
                cubemapScale = 0.0;
                materialType = ( F32 )MATERIAL_GRAVEL;
                parallaxScale = 3.0;
                break;
            case MATERIAL_ROCK:				// 23			//
                specularScale = 0.0;
                cubemapScale = 0.0;
                materialType = ( F32 )MATERIAL_ROCK;
                parallaxScale = 3.0;
                useSteepParallax = 1.0;
                break;
            case MATERIAL_TILES:			// 26			// tiled floor
                specularScale = 0.86;
                cubemapScale = 0.9;
                materialType = ( F32 )MATERIAL_TILES;
                parallaxScale = 2.5;
                useSteepParallax = 1.0;
                break;
            case MATERIAL_SOLIDWOOD:		// 1			// freshly cut timber
                specularScale = 0.0;
                cubemapScale = 0.0;
                materialType = ( F32 )MATERIAL_SOLIDWOOD;
                parallaxScale = 2.5;
                //useSteepParallax = 1.0;
                break;
            case MATERIAL_HOLLOWWOOD:		// 2			// termite infested creaky wood
                specularScale = 0.0;
                cubemapScale = 0.0;
                materialType = ( F32 )MATERIAL_HOLLOWWOOD;
                parallaxScale = 2.5;
                //useSteepParallax = 1.0;
                break;
            case MATERIAL_SOLIDMETAL:		// 3			// solid girders
                specularScale = 0.92;
                cubemapScale = 0.92;
                materialType = ( F32 )MATERIAL_SOLIDMETAL;
                parallaxScale = 0.005;
                isMetalic = 1.0;
                break;
            case MATERIAL_HOLLOWMETAL:		// 4			// hollow metal machines -- Used for weapons to force lower parallax...
                specularScale = 0.92;
                cubemapScale = 0.92;
                materialType = ( F32 )MATERIAL_HOLLOWMETAL;
                parallaxScale = 2.0;
                isMetalic = 1.0;
                break;
            case MATERIAL_DRYLEAVES:		// 19			// dried up leaves on the floor
                specularScale = 0.0;
                cubemapScale = 0.0;
                materialType = ( F32 )MATERIAL_DRYLEAVES;
                parallaxScale = 0.0;
                //useSteepParallax = 1.0;
                break;
            case MATERIAL_GREENLEAVES:		// 20			// fresh leaves still on a tree
                specularScale = 0.75;
                cubemapScale = 0.0;
                materialType = ( F32 )MATERIAL_GREENLEAVES;
                parallaxScale = 0.0; // GreenLeaves should NEVER be parallaxed.. It's used for surfaces with an alpha channel and parallax screws it up...
                //useSteepParallax = 1.0;
                break;
            case MATERIAL_FABRIC:			// 21			// Cotton sheets
                specularScale = 0.48;
                cubemapScale = 0.0;
                materialType = ( F32 )MATERIAL_FABRIC;
                parallaxScale = 2.5;
                break;
            case MATERIAL_CANVAS:			// 22			// tent material
                specularScale = 0.45;
                cubemapScale = 0.0;
                materialType = ( F32 )MATERIAL_CANVAS;
                parallaxScale = 2.5;
                break;
            case MATERIAL_MARBLE:			// 12			// marble floors
                specularScale = 0.86;
                cubemapScale = 1.0;
                materialType = ( F32 )MATERIAL_MARBLE;
                parallaxScale = 2.0;
                //useSteepParallax = 1.0;
                break;
            case MATERIAL_SNOW:				// 14			// freshly laid snow
                specularScale = 0.65;
                cubemapScale = 0.0;
                materialType = ( F32 )MATERIAL_SNOW;
                parallaxScale = 3.0;
                useSteepParallax = 1.0;
                break;
            case MATERIAL_MUD:				// 17			// wet soil
                specularScale = 0.0;
                cubemapScale = 0.0;
                materialType = ( F32 )MATERIAL_MUD;
                parallaxScale = 3.0;
                useSteepParallax = 1.0;
                break;
            case MATERIAL_DIRT:				// 7			// hard mud
                specularScale = 0.0;
                cubemapScale = 0.0;
                materialType = ( F32 )MATERIAL_DIRT;
                parallaxScale = 3.0;
                useSteepParallax = 1.0;
                break;
            case MATERIAL_CONCRETE:			// 11			// hardened concrete pavement
                specularScale = 0.3;
                cubemapScale = 0.0;
                materialType = ( F32 )MATERIAL_CONCRETE;
                parallaxScale = 3.0;
                //useSteepParallax = 1.0;
                break;
            case MATERIAL_FLESH:			// 16			// hung meat, corpses in the world
                specularScale = 0.2;
                cubemapScale = 0.0;
                materialType = ( F32 )MATERIAL_FLESH;
                parallaxScale = 1.0;
                break;
            case MATERIAL_RUBBER:			// 24			// hard tire like rubber
                specularScale = 0.0;
                cubemapScale = 0.0;
                materialType = ( F32 )MATERIAL_RUBBER;
                parallaxScale = 1.0;
                break;
            case MATERIAL_PLASTIC:			// 25			//
                specularScale = 0.88;
                cubemapScale = 0.5;
                materialType = ( F32 )MATERIAL_PLASTIC;
                parallaxScale = 1.0;
                break;
            case MATERIAL_PLASTER:			// 28			// drywall style plaster
                specularScale = 0.4;
                cubemapScale = 0.0;
                materialType = ( F32 )MATERIAL_PLASTER;
                parallaxScale = 2.0;
                break;
            case MATERIAL_SHATTERGLASS:		// 29			// glass with the Crisis Zone style shattering
                specularScale = 0.88;
                cubemapScale = 1.0;
                materialType = ( F32 )MATERIAL_SHATTERGLASS;
                parallaxScale = 1.0;
                break;
            case MATERIAL_ARMOR:			// 30			// body armor
                specularScale = 0.4;
                cubemapScale = 2.0;
                materialType = ( F32 )MATERIAL_ARMOR;
                parallaxScale = 2.0;
                isMetalic = 1.0;
                break;
            case MATERIAL_ICE:				// 15			// packed snow/solid ice
                specularScale = 0.9;
                cubemapScale = 0.8;
                parallaxScale = 2.0;
                materialType = ( F32 )MATERIAL_ICE;
                useSteepParallax = 1.0;
                break;
            case MATERIAL_GLASS:			// 10			//
                specularScale = 0.95;
                cubemapScale = 1.0;
                materialType = ( F32 )MATERIAL_GLASS;
                parallaxScale = 1.0;
                break;
            case MATERIAL_BPGLASS:			// 18			// bulletproof glass
                specularScale = 0.93;
                cubemapScale = 0.93;
                materialType = ( F32 )MATERIAL_BPGLASS;
                parallaxScale = 1.0;
                break;
            case MATERIAL_COMPUTER:			// 31			// computers/electronic equipment
                specularScale = 0.92;
                cubemapScale = 0.92;
                materialType = ( F32 )MATERIAL_COMPUTER;
                parallaxScale = 2.0;
                break;
            default:
                specularScale = 0.0;
                cubemapScale = 0.0;
                materialType = ( F32 )0.0;
                parallaxScale = 1.0;
                break;
        }
    }
    
    // Shader overrides material...
    if( pStage->cubeMapScale > 0.0 )
    {
        cubemapScale = pStage->cubeMapScale;
    }
    
    bool realNormalMap = false;
    
    if( pStage->bundle[TB_NORMALMAP].image[0] )
    {
        realNormalMap = true;
    }
    
    if( pStage->bundle[TB_DIFFUSEMAP].image[0] )
    {
        doSway = 0.7;
    }
    
    VectorSet4( local1, parallaxScale, ( F32 )pStage->hasSpecular, specularScale, materialType );
    GLSL_SetUniformVec4( sp, UNIFORM_LOCAL1, local1 );
    GLSL_SetUniformVec4( sp, UNIFORM_LOCAL2, pStage->subsurfaceExtinctionCoefficient );
    VectorSet4( local3, pStage->subsurfaceRimScalar, pStage->subsurfaceMaterialThickness, pStage->subsurfaceSpecularPower, cubemapScale );
    GLSL_SetUniformVec4( sp, UNIFORM_LOCAL3, local3 );
    VectorSet4( local4, ( F32 )realNormalMap, isMetalic, 0.0/*(F32)pStage->hasRealSubsurfaceMap*/, doSway );
    GLSL_SetUniformVec4( sp, UNIFORM_LOCAL4, local4 );
    VectorSet4( local5, hasOverlay, overlaySway, r_blinnPhong->value, hasSteepMap );
    GLSL_SetUniformVec4( sp, UNIFORM_LOCAL5, local5 );
    
    if( backEnd.viewParms.targetFbo == tr.renderCubeFbo )
    {
        VectorSet4( local5, 0.0, 0.0, 0.0, 0.0 );
        GLSL_SetUniformVec4( sp, UNIFORM_LOCAL5, local5 );
    }
    else
    {
        VectorSet4( local5, r_imageBasedLighting->value, 0.0, 0.0, 0.0 );
        GLSL_SetUniformVec4( sp, UNIFORM_LOCAL5, local5 );
    }
    
    if( r_sunlightSpecular->integer )
    {
        vec4_t local6;
        VectorSet4( local6, 1.0, 0.0, 0.0, 0 );
        GLSL_SetUniformVec4( sp, UNIFORM_LOCAL6, local6 );
    }
    else
    {
        vec4_t local6;
        VectorSet4( local6, 0.0, 0.0, 0.0, 0 );
        GLSL_SetUniformVec4( sp, UNIFORM_LOCAL6, local6 );
    }
    
    //GLSL_SetUniformFloat(sp, UNIFORM_TIME, tess.shaderTime);
    GLSL_SetUniformFloat( sp, UNIFORM_TIME, backEnd.refdef.floatTime );
}

void RB_SetStageImageDimensions( shaderProgram_t* sp, shaderStage_t* pStage )
{
    vec2_t dimensions;
    
    if( !pStage->bundle[0].image[0] )
    {
        pStage->bundle[0].image[0] = tr.whiteImage; // argh!
    }
    
    dimensions[0] = pStage->bundle[0].image[0]->width;
    dimensions[1] = pStage->bundle[0].image[0]->height;
    
    if( pStage->bundle[TB_DIFFUSEMAP].image[0] )
    {
        dimensions[0] = pStage->bundle[TB_DIFFUSEMAP].image[0]->width;
        dimensions[1] = pStage->bundle[TB_DIFFUSEMAP].image[0]->height;
    }
    else if( pStage->bundle[TB_NORMALMAP].image[0] )
    {
        dimensions[0] = pStage->bundle[TB_NORMALMAP].image[0]->width;
        dimensions[1] = pStage->bundle[TB_NORMALMAP].image[0]->height;
    }
    else if( pStage->bundle[TB_SPECULARMAP].image[0] )
    {
        dimensions[0] = pStage->bundle[TB_SPECULARMAP].image[0]->width;
        dimensions[1] = pStage->bundle[TB_SPECULARMAP].image[0]->height;
    }
    else if( pStage->bundle[TB_SUBSURFACEMAP].image[0] )
    {
        dimensions[0] = pStage->bundle[TB_SUBSURFACEMAP].image[0]->width;
        dimensions[1] = pStage->bundle[TB_SUBSURFACEMAP].image[0]->height;
    }
    else if( pStage->bundle[TB_OVERLAYMAP].image[0] )
    {
        dimensions[0] = pStage->bundle[TB_OVERLAYMAP].image[0]->width;
        dimensions[1] = pStage->bundle[TB_OVERLAYMAP].image[0]->height;
    }
    else if( pStage->bundle[TB_STEEPMAP].image[0] )
    {
        dimensions[0] = pStage->bundle[TB_STEEPMAP].image[0]->width;
        dimensions[1] = pStage->bundle[TB_STEEPMAP].image[0]->height;
    }
    
    GLSL_SetUniformVec2( sp, UNIFORM_DIMENSIONS, dimensions );
}

bool RB_ShouldUseTesselation( S32 materialType )
{
    /*if ( materialType == MATERIAL_SHORTGRASS
    	|| materialType == MATERIAL_LONGGRASS
    	|| materialType == MATERIAL_SAND
    	|| materialType == MATERIAL_ROCK
    	|| materialType == MATERIAL_ICE)*/
    return true;
    
    return false;
}

F32 RB_GetTesselationAlphaLevel( S32 materialType )
{
    F32 tessAlphaLevel = r_tesselationAlpha->value;
    
    switch( materialType )
    {
        case MATERIAL_WATER:			// 13			// light covering of water on a surface
            tessAlphaLevel = r_tesselationAlpha->value;
            break;
        case MATERIAL_SHORTGRASS:		// 5			// manicured lawn
            tessAlphaLevel = r_tesselationAlpha->value;
            break;
        case MATERIAL_LONGGRASS:		// 6			// long jungle grass
            tessAlphaLevel = r_tesselationAlpha->value;
            break;
        case MATERIAL_SAND:				// 8			// sandy beach
            tessAlphaLevel = r_tesselationAlpha->value * 0.1;
            break;
        case MATERIAL_CARPET:			// 27			// lush carpet
            tessAlphaLevel = r_tesselationAlpha->value * 0.3;
            break;
        case MATERIAL_GRAVEL:			// 9			// lots of small stones
            tessAlphaLevel = r_tesselationAlpha->value * 0.5;
            break;
        case MATERIAL_ROCK:				// 23			//
            tessAlphaLevel = r_tesselationAlpha->value;
            break;
        case MATERIAL_TILES:			// 26			// tiled floor
            tessAlphaLevel = r_tesselationAlpha->value * 0.3;
            break;
        case MATERIAL_SOLIDWOOD:		// 1			// freshly cut timber
            tessAlphaLevel = r_tesselationAlpha->value;
            break;
        case MATERIAL_HOLLOWWOOD:		// 2			// termite infested creaky wood
            tessAlphaLevel = r_tesselationAlpha->value;
            break;
        case MATERIAL_SOLIDMETAL:		// 3			// solid girders
            tessAlphaLevel = r_tesselationAlpha->value * 0.3;
            break;
        case MATERIAL_HOLLOWMETAL:		// 4			// hollow metal machines -- Used for weapons to force lower parallax and high reflection...
            tessAlphaLevel = r_tesselationAlpha->value * 0.2;
            break;
        case MATERIAL_DRYLEAVES:		// 19			// dried up leaves on the floor
            tessAlphaLevel = r_tesselationAlpha->value;
            break;
        case MATERIAL_GREENLEAVES:		// 20			// fresh leaves still on a tree
            tessAlphaLevel = r_tesselationAlpha->value;
            break;
        case MATERIAL_FABRIC:			// 21			// Cotton sheets
            tessAlphaLevel = r_tesselationAlpha->value;
            break;
        case MATERIAL_CANVAS:			// 22			// tent material
            tessAlphaLevel = r_tesselationAlpha->value;
            break;
        case MATERIAL_MARBLE:			// 12			// marble floors
            tessAlphaLevel = r_tesselationAlpha->value * 0.5;
            break;
        case MATERIAL_SNOW:				// 14			// freshly laid snow
            tessAlphaLevel = r_tesselationAlpha->value;
            break;
        case MATERIAL_MUD:				// 17			// wet soil
            tessAlphaLevel = r_tesselationAlpha->value;
            break;
        case MATERIAL_DIRT:				// 7			// hard mud
            tessAlphaLevel = r_tesselationAlpha->value;
            break;
        case MATERIAL_CONCRETE:			// 11			// hardened concrete pavement
            tessAlphaLevel = r_tesselationAlpha->value * 0.3;
            break;
        case MATERIAL_FLESH:			// 16			// hung meat, corpses in the world
            tessAlphaLevel = r_tesselationAlpha->value;
            break;
        case MATERIAL_RUBBER:			// 24			// hard tire like rubber
            tessAlphaLevel = r_tesselationAlpha->value;
            break;
        case MATERIAL_PLASTIC:			// 25			//
            tessAlphaLevel = r_tesselationAlpha->value * 0.5;
            break;
        case MATERIAL_PLASTER:			// 28			// drywall style plaster
            tessAlphaLevel = r_tesselationAlpha->value * 0.3;
            break;
        case MATERIAL_SHATTERGLASS:		// 29			// glass with the Crisis Zone style shattering
            tessAlphaLevel = r_tesselationAlpha->value * 0.1;
            break;
        case MATERIAL_ARMOR:			// 30			// body armor
            tessAlphaLevel = r_tesselationAlpha->value;
            break;
        case MATERIAL_ICE:				// 15			// packed snow/solid ice
            tessAlphaLevel = r_tesselationAlpha->value;
            break;
        case MATERIAL_GLASS:			// 10			//
            tessAlphaLevel = r_tesselationAlpha->value * 0.1;
            break;
        case MATERIAL_BPGLASS:			// 18			// bulletproof glass
            tessAlphaLevel = r_tesselationAlpha->value * 0.1;
            break;
        case MATERIAL_COMPUTER:			// 31			// computers/electronic equipment
            tessAlphaLevel = r_tesselationAlpha->value * 0.1;
            break;
        default:
            break;
    }
    
    return tessAlphaLevel;
}

F32 RB_GetTesselationInnerLevel( S32 materialType )
{
    F32 tessInnerLevel = 1.0;
    
    switch( materialType )
    {
        case MATERIAL_WATER:			// 13			// light covering of water on a surface
            tessInnerLevel = r_tesselationLevel->value;
            break;
        case MATERIAL_SHORTGRASS:		// 5			// manicured lawn
            tessInnerLevel = CLAMP( r_tesselationLevel->value, 1.0, 2.25 );
            break;
        case MATERIAL_LONGGRASS:		// 6			// long jungle grass
            tessInnerLevel = CLAMP( r_tesselationLevel->value, 1.0, 2.25 );
            break;
        case MATERIAL_SAND:				// 8			// sandy beach
            tessInnerLevel = CLAMP( r_tesselationLevel->value * 0.1, 1.0, 2.25 );
            break;
        case MATERIAL_CARPET:			// 27			// lush carpet
            tessInnerLevel = CLAMP( r_tesselationLevel->value * 0.3, 1.0, 2.25 );
            break;
        case MATERIAL_GRAVEL:			// 9			// lots of small stones
            tessInnerLevel = CLAMP( r_tesselationLevel->value, 1.0, 2.25 );
            break;
        case MATERIAL_ROCK:				// 23			//
            tessInnerLevel = r_tesselationLevel->value;
            break;
        case MATERIAL_TILES:			// 26			// tiled floor
            tessInnerLevel = CLAMP( r_tesselationLevel->value * 0.3, 1.0, 2.25 );
            break;
        case MATERIAL_SOLIDWOOD:		// 1			// freshly cut timber
            tessInnerLevel = r_tesselationLevel->value;
            break;
        case MATERIAL_HOLLOWWOOD:		// 2			// termite infested creaky wood
            tessInnerLevel = r_tesselationLevel->value;
            break;
        case MATERIAL_SOLIDMETAL:		// 3			// solid girders
            tessInnerLevel = CLAMP( r_tesselationLevel->value * 0.5, 1.0, 2.25 );
            break;
        case MATERIAL_HOLLOWMETAL:		// 4			// hollow metal machines -- Used for weapons to force lower parallax and high reflection...
            tessInnerLevel = CLAMP( r_tesselationLevel->value * 0.3, 1.0, 2.25 );
            break;
        case MATERIAL_DRYLEAVES:		// 19			// dried up leaves on the floor
            tessInnerLevel = CLAMP( r_tesselationLevel->value, 1.0, 2.25 );
            break;
        case MATERIAL_GREENLEAVES:		// 20			// fresh leaves still on a tree
            tessInnerLevel = CLAMP( r_tesselationLevel->value, 1.0, 2.25 );
            break;
        case MATERIAL_FABRIC:			// 21			// Cotton sheets
            tessInnerLevel = r_tesselationLevel->value;
            break;
        case MATERIAL_CANVAS:			// 22			// tent material
            tessInnerLevel = CLAMP( r_tesselationLevel->value, 1.0, 2.25 );
            break;
        case MATERIAL_MARBLE:			// 12			// marble floors
            tessInnerLevel = CLAMP( r_tesselationLevel->value * 0.5, 1.0, 2.25 );
            break;
        case MATERIAL_SNOW:				// 14			// freshly laid snow
            tessInnerLevel = r_tesselationLevel->value;
            break;
        case MATERIAL_MUD:				// 17			// wet soil
            tessInnerLevel = CLAMP( r_tesselationLevel->value, 1.0, 2.25 );
            break;
        case MATERIAL_DIRT:				// 7			// hard mud
            tessInnerLevel = CLAMP( r_tesselationLevel->value, 1.0, 2.25 );
            break;
        case MATERIAL_CONCRETE:			// 11			// hardened concrete pavement
            tessInnerLevel = CLAMP( r_tesselationLevel->value * 0.3, 1.0, 2.25 );
            break;
        case MATERIAL_FLESH:			// 16			// hung meat, corpses in the world
            tessInnerLevel = r_tesselationLevel->value;
            break;
        case MATERIAL_RUBBER:			// 24			// hard tire like rubber
            tessInnerLevel = CLAMP( r_tesselationLevel->value, 1.0, 2.25 );
            break;
        case MATERIAL_PLASTIC:			// 25			//
            tessInnerLevel = CLAMP( r_tesselationLevel->value * 0.5, 1.0, 2.25 );
            break;
        case MATERIAL_PLASTER:			// 28			// drywall style plaster
            tessInnerLevel = CLAMP( r_tesselationLevel->value * 0.3, 1.0, 2.25 );
            break;
        case MATERIAL_SHATTERGLASS:		// 29			// glass with the Crisis Zone style shattering
            tessInnerLevel = 1.0;
            break;
        case MATERIAL_ARMOR:			// 30			// body armor
            tessInnerLevel = r_tesselationLevel->value;
            break;
        case MATERIAL_ICE:				// 15			// packed snow/solid ice
            tessInnerLevel = r_tesselationLevel->value;
            break;
        case MATERIAL_GLASS:			// 10			//
            tessInnerLevel = 1.0;
            break;
        case MATERIAL_BPGLASS:			// 18			// bulletproof glass
            tessInnerLevel = 1.0;
            break;
        case MATERIAL_COMPUTER:			// 31			// computers/electronic equipment
            tessInnerLevel = 1.0;
            break;
        default:
            break;
    }
    
    if( tessInnerLevel < 1.0 ) tessInnerLevel = 1.0;
    
    return tessInnerLevel;
}

F32 RB_GetTesselationOuterLevel( S32 materialType )
{
    F32 tessOuterLevel = 1.0;
    
    switch( materialType )
    {
        case MATERIAL_WATER:			// 13			// light covering of water on a surface
            tessOuterLevel = 1.0;
            break;
        case MATERIAL_SHORTGRASS:		// 5			// manicured lawn
            tessOuterLevel = r_tesselationLevel->value;
            break;
        case MATERIAL_LONGGRASS:		// 6			// long jungle grass
            tessOuterLevel = r_tesselationLevel->value;
            break;
        case MATERIAL_SAND:				// 8			// sandy beach
            tessOuterLevel = r_tesselationLevel->value * 0.1;
            break;
        case MATERIAL_CARPET:			// 27			// lush carpet
            tessOuterLevel = 1.0;
            break;
        case MATERIAL_GRAVEL:			// 9			// lots of small stones
            tessOuterLevel = r_tesselationLevel->value;
            break;
        case MATERIAL_ROCK:				// 23			//
            tessOuterLevel = r_tesselationLevel->value;
            break;
        case MATERIAL_TILES:			// 26			// tiled floor
            tessOuterLevel = 1.0;
            break;
        case MATERIAL_SOLIDWOOD:		// 1			// freshly cut timber
            tessOuterLevel = r_tesselationLevel->value;
            break;
        case MATERIAL_HOLLOWWOOD:		// 2			// termite infested creaky wood
            tessOuterLevel = r_tesselationLevel->value;
            break;
        case MATERIAL_SOLIDMETAL:		// 3			// solid girders
            tessOuterLevel = r_tesselationLevel->value * 0.5;
            break;
        case MATERIAL_HOLLOWMETAL:		// 4			// hollow metal machines -- Used for weapons to force lower parallax and high reflection...
            tessOuterLevel = 1.0;
            break;
        case MATERIAL_DRYLEAVES:		// 19			// dried up leaves on the floor
            tessOuterLevel = 1.0;
            break;
        case MATERIAL_GREENLEAVES:		// 20			// fresh leaves still on a tree
            tessOuterLevel = 1.0;
            break;
        case MATERIAL_FABRIC:			// 21			// Cotton sheets
            tessOuterLevel = r_tesselationLevel->value;
            break;
        case MATERIAL_CANVAS:			// 22			// tent material
            tessOuterLevel = r_tesselationLevel->value;
            break;
        case MATERIAL_MARBLE:			// 12			// marble floors
            tessOuterLevel = 1.0;
            break;
        case MATERIAL_SNOW:				// 14			// freshly laid snow
            tessOuterLevel = r_tesselationLevel->value;
            break;
        case MATERIAL_MUD:				// 17			// wet soil
            tessOuterLevel = r_tesselationLevel->value;
            break;
        case MATERIAL_DIRT:				// 7			// hard mud
            tessOuterLevel = r_tesselationLevel->value;
            break;
        case MATERIAL_CONCRETE:			// 11			// hardened concrete pavement
            tessOuterLevel = 1.0;
            break;
        case MATERIAL_FLESH:			// 16			// hung meat, corpses in the world
            tessOuterLevel = r_tesselationLevel->value;
            break;
        case MATERIAL_RUBBER:			// 24			// hard tire like rubber
            tessOuterLevel = 1.0;
            break;
        case MATERIAL_PLASTIC:			// 25			//
            tessOuterLevel = r_tesselationLevel->value * 0.5;
            break;
        case MATERIAL_PLASTER:			// 28			// drywall style plaster
            tessOuterLevel = 1.0;
            break;
        case MATERIAL_SHATTERGLASS:		// 29			// glass with the Crisis Zone style shattering
            tessOuterLevel = 1.0;
            break;
        case MATERIAL_ARMOR:			// 30			// body armor
            tessOuterLevel = r_tesselationLevel->value;
            break;
        case MATERIAL_ICE:				// 15			// packed snow/solid ice
            tessOuterLevel = r_tesselationLevel->value;
            break;
        case MATERIAL_GLASS:			// 10			//
            tessOuterLevel = 1.0;
            break;
        case MATERIAL_BPGLASS:			// 18			// bulletproof glass
            tessOuterLevel = 1.0;
            break;
        case MATERIAL_COMPUTER:			// 31			// computers/electronic equipment
            tessOuterLevel = 1.0;
            break;
        default:
            break;
    }
    
    if( tessOuterLevel < 1.0 ) tessOuterLevel = 1.0;
    
    return tessOuterLevel;
}
bool MATRIX_UPDATE = true;
bool CLOSE_LIGHTS_UPDATE = true;

matrix_t MATRIX_TRANS, MATRIX_MODEL, MATRIX_MVP, MATRIX_INVTRANS, MATRIX_NORMAL, MATRIX_VP, MATRIX_INVMV;

void RB_UpdateMatrixes( void )
{
    if( !MATRIX_UPDATE ) return;
    
    // Calculate some matrixes that rend2 doesn't seem to have (or have correct)...
    Mat4Translation( backEnd.viewParms.orientation.origin, MATRIX_TRANS );
    Mat4Multiply( backEnd.viewParms.world.modelMatrix, MATRIX_TRANS, MATRIX_MODEL );
    Mat4Multiply( backEnd.viewParms.projectionMatrix, MATRIX_MODEL, MATRIX_MVP );
    Mat4Multiply( backEnd.viewParms.projectionMatrix, backEnd.viewParms.world.modelMatrix, MATRIX_VP );
    
    Mat4SimpleInverse( MATRIX_TRANS, MATRIX_INVTRANS );
    Mat4SimpleInverse( backEnd.viewParms.projectionMatrix, MATRIX_NORMAL );
    
    
    //GLSL_SetUniformMatrix16(sp, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection);
    //GLSL_SetUniformMatrix16(sp, UNIFORM_INVEYEPROJECTIONMATRIX, glState.invEyeProjection);
    Mat4SimpleInverse( glState.modelviewProjection, glState.invEyeProjection );
    Mat4SimpleInverse( MATRIX_MODEL, MATRIX_INVMV );
    
    MATRIX_UPDATE = false;
}

S32			NUM_CLOSE_LIGHTS = 0;
S32			CLOSEST_LIGHTS[MAX_DEFERRED_LIGHTS] = { 0 };
vec3_t		CLOSEST_LIGHTS_POSITIONS[MAX_DEFERRED_LIGHTS] = { 0 };
vec2_t		CLOSEST_LIGHTS_SCREEN_POSITIONS[MAX_DEFERRED_LIGHTS];
F32		CLOSEST_LIGHTS_DISTANCES[MAX_DEFERRED_LIGHTS] = { 0 };
F32		CLOSEST_LIGHTS_HEIGHTSCALES[MAX_DEFERRED_LIGHTS] = { 0 };
vec3_t		CLOSEST_LIGHTS_COLORS[MAX_DEFERRED_LIGHTS] = { 0 };

extern void WorldCoordToScreenCoord( vec3_t origin, F32* x, F32* y );
extern bool Volumetric_Visible( vec3_t from, vec3_t to, bool isSun );

void RB_UpdateCloseLights( void )
{
    if( !CLOSE_LIGHTS_UPDATE ) return; // Already done for this frame...
    
    NUM_CLOSE_LIGHTS = 0;
    
    for( S32 l = 0; l < backEnd.refdef.num_dlights; l++ )
    {
        dlight_t* dl = &backEnd.refdef.dlights[l];
        
        if( dl->color[0] < 0.0 && dl->color[1] < 0.0 && dl->color[2] < 0.0 )
        {
            // Surface glow light... But has no color assigned...
            continue;
        }
        
        F32 distance = Distance( tr.refdef.vieworg, dl->origin );
        /*bool skip = qfalse;
        
        for (S32 i = 0; i < NUM_CLOSE_LIGHTS; i++)
        {// Find the most distance light in our current list to replace, if this new option is closer...
        dlight_t *thisLight = &backEnd.refdef.dlights[CLOSEST_LIGHTS[i]];
        F32 dist = Distance(thisLight->origin, dl->origin);
        if (dist < 128.0)
        {
        skip = qtrue;
        break;
        }
        }
        
        if (skip)
        {
        continue;
        }*/
        
        vec3_t from;
        VectorCopy( tr.refdef.vieworg, from );
        from[2] += 48.0;
        if( !Volumetric_Visible( tr.refdef.vieworg, dl->origin, false ) )
        {
            continue;
        }
        
        if( NUM_CLOSE_LIGHTS < MAX_LIGHTALL_DLIGHTS )
        {
            // Have free light slots for a new light...
            F32 x, y;
            WorldCoordToScreenCoord( dl->origin, &x, &y );
            
            if( x < 0.0 || y < 0.0 || x > 1.0 || y > 1.0 )
                continue;
                
            CLOSEST_LIGHTS[NUM_CLOSE_LIGHTS] = l;
            VectorCopy( dl->origin, CLOSEST_LIGHTS_POSITIONS[NUM_CLOSE_LIGHTS] );
            CLOSEST_LIGHTS_DISTANCES[NUM_CLOSE_LIGHTS] = dl->radius;
            CLOSEST_LIGHTS_HEIGHTSCALES[NUM_CLOSE_LIGHTS] = dl->heightScale;
            CLOSEST_LIGHTS_COLORS[NUM_CLOSE_LIGHTS][0] = dl->color[0];
            CLOSEST_LIGHTS_COLORS[NUM_CLOSE_LIGHTS][1] = dl->color[1];
            CLOSEST_LIGHTS_COLORS[NUM_CLOSE_LIGHTS][2] = dl->color[2];
            CLOSEST_LIGHTS_SCREEN_POSITIONS[NUM_CLOSE_LIGHTS][0] = x;
            CLOSEST_LIGHTS_SCREEN_POSITIONS[NUM_CLOSE_LIGHTS][1] = y;
            NUM_CLOSE_LIGHTS++;
            continue;
        }
        else
        {
            // See if this is closer then one of our other lights...
            S32 farthest_light = 0;
            F32 farthest_distance = 0.0;
            
            for( S32 i = 0; i < NUM_CLOSE_LIGHTS; i++ )
            {
                // Find the most distance light in our current list to replace, if this new option is closer...
                dlight_t*	thisLight = &backEnd.refdef.dlights[CLOSEST_LIGHTS[i]];
                F32 dist = Distance( thisLight->origin, tr.refdef.vieworg );
                
                if( dist > farthest_distance )
                {
                    // This one is further!
                    farthest_light = i;
                    farthest_distance = dist;
                    break;
                }
            }
            
            if( Distance( dl->origin, tr.refdef.vieworg ) < farthest_distance )
            {
                // This light is closer. Replace this one in our array of closest lights...
                F32 x, y;
                WorldCoordToScreenCoord( dl->origin, &x, &y );
                
                if( x < 0.0 || y < 0.0 || x > 1.0 || y > 1.0 )
                {
                    continue;
                }
                
                CLOSEST_LIGHTS[farthest_light] = l;
                VectorCopy( dl->origin, CLOSEST_LIGHTS_POSITIONS[farthest_light] );
                CLOSEST_LIGHTS_DISTANCES[farthest_light] = dl->radius;
                CLOSEST_LIGHTS_HEIGHTSCALES[farthest_light] = dl->heightScale;
                CLOSEST_LIGHTS_COLORS[farthest_light][0] = dl->color[0];
                CLOSEST_LIGHTS_COLORS[farthest_light][1] = dl->color[1];
                CLOSEST_LIGHTS_COLORS[farthest_light][2] = dl->color[2];
                CLOSEST_LIGHTS_SCREEN_POSITIONS[farthest_light][0] = x;
                CLOSEST_LIGHTS_SCREEN_POSITIONS[farthest_light][1] = y;
            }
        }
    }
    
    for( S32 i = 0; i < NUM_CLOSE_LIGHTS; i++ )
    {
        if( CLOSEST_LIGHTS_DISTANCES[i] < 0.0 )
        {
            // Remove volume light markers...
            CLOSEST_LIGHTS_DISTANCES[i] = -CLOSEST_LIGHTS_DISTANCES[i];
        }
        
        // Double the range on all lights...
        CLOSEST_LIGHTS_DISTANCES[i] *= 2.0;
    }
    
    //CL_RefPrintf(PRINT_ALL, "Found %i close lights this frame.\n", NUM_CLOSE_LIGHTS);
    
    CLOSE_LIGHTS_UPDATE = false;
}

static void RB_IterateStagesGeneric( shaderCommands_t* input )
{
    S32 stage;
    vec4_t fogDistanceVector, fogDepthVector = { 0, 0, 0, 0 };
    F32 eyeT = 0;
    S32 deformGen;
    vec5_t deformParams;
    
    bool renderToCubemap = tr.renderCubeFbo && glState.currentFBO == tr.renderCubeFbo;
    
    ComputeDeformValues( &deformGen, deformParams );
    
    ComputeFogValues( fogDistanceVector, fogDepthVector, &eyeT );
    
    S32 NUM_CLOSE_LIGHTS = 0;
    S32 CLOSEST_LIGHTS[MAX_LIGHTALL_DLIGHTS] = { 0 };
    vec3_t CLOSEST_LIGHTS_POSITIONS[MAX_LIGHTALL_DLIGHTS] = { 0 };
    F32 CLOSEST_LIGHTS_DISTANCES[MAX_LIGHTALL_DLIGHTS] = { 0 };
    vec3_t CLOSEST_LIGHTS_COLORS[MAX_LIGHTALL_DLIGHTS] = { 0 };
    
    for( S32 l = 0; l < backEnd.refdef.num_dlights; l++ )
    {
        dlight_t*	dl = &backEnd.refdef.dlights[l];
        
        F32 distance = Distance( backEnd.refdef.vieworg, dl->origin );
        
        if( NUM_CLOSE_LIGHTS < MAX_LIGHTALL_DLIGHTS )
        {
            // Have free light slots for a new light...
            CLOSEST_LIGHTS[NUM_CLOSE_LIGHTS] = l;
            VectorCopy( dl->origin, CLOSEST_LIGHTS_POSITIONS[NUM_CLOSE_LIGHTS] );
            CLOSEST_LIGHTS_DISTANCES[NUM_CLOSE_LIGHTS] = dl->radius;
            CLOSEST_LIGHTS_COLORS[NUM_CLOSE_LIGHTS][0] = dl->color[0];
            CLOSEST_LIGHTS_COLORS[NUM_CLOSE_LIGHTS][1] = dl->color[1];
            CLOSEST_LIGHTS_COLORS[NUM_CLOSE_LIGHTS][2] = dl->color[2];
            NUM_CLOSE_LIGHTS++;
            continue;
        }
        else
        {
            // See if this is closer then one of our other lights...
            S32 farthest_light = 0;
            F32	farthest_distance = 0.0;
            
            for( S32 i = 0; i < NUM_CLOSE_LIGHTS; i++ )
            {
                // Find the most distance light in our current list to replace, if this new option is closer...
                dlight_t* thisLight = &backEnd.refdef.dlights[CLOSEST_LIGHTS[i]];
                F32	 dist = Distance( thisLight->origin, backEnd.refdef.vieworg );
                
                if( dist > farthest_distance )
                {
                    // This one is further!
                    farthest_light = i;
                    farthest_distance = dist;
                    break;
                }
            }
            
            if( Distance( dl->origin, backEnd.refdef.vieworg ) < farthest_distance )
            {
                // This light is closer. Replace this one in our array of closest lights...
                CLOSEST_LIGHTS[farthest_light] = l;
                VectorCopy( dl->origin, CLOSEST_LIGHTS_POSITIONS[farthest_light] );
                CLOSEST_LIGHTS_DISTANCES[farthest_light] = dl->radius;
                CLOSEST_LIGHTS_COLORS[farthest_light][0] = dl->color[0];
                CLOSEST_LIGHTS_COLORS[farthest_light][1] = dl->color[1];
                CLOSEST_LIGHTS_COLORS[farthest_light][2] = dl->color[2];
            }
        }
    }
    
    for( S32 i = 0; i < NUM_CLOSE_LIGHTS; i++ )
    {
        if( CLOSEST_LIGHTS_DISTANCES[i] < 0.0 )
        {
            // Remove volume light markers...
            CLOSEST_LIGHTS_DISTANCES[i] = -CLOSEST_LIGHTS_DISTANCES[i];
        }
        
        // Double the range on all lights...
        CLOSEST_LIGHTS_DISTANCES[i] *= 2.0;
    }
    
    // Calculate some matrixes that rend2 doesn't seem to have (or have correct)...
    matrix_t trans, model, mvp, invTrans, normalMatrix;
    
    Mat4Translation( backEnd.viewParms.orientation.origin, trans );
    Mat4Multiply( backEnd.viewParms.world.modelMatrix, trans, model );
    Mat4Multiply( backEnd.viewParms.projectionMatrix, model, mvp );
    
    Mat4SimpleInverse( trans, invTrans );
    Mat4SimpleInverse( backEnd.viewParms.projectionMatrix, normalMatrix );
    
    for( stage = 0; stage < MAX_SHADER_STAGES; stage++ )
    {
        shaderStage_t* pStage = input->xstages[stage];
        shaderProgram_t* sp;
        vec4_t texMatrix;
        vec4_t texOffTurb;
        
        if( !pStage )
        {
            break;
        }
        
        if( backEnd.depthFill )
        {
            if( pStage->glslShaderGroup == tr.lightallShader )
            {
                S32 index = 0;
                
                if( pStage->stateBits & GLS_ATEST_BITS )
                {
                    index |= LIGHTDEF_USE_TCGEN_AND_TCMOD;
                }
                
                sp = &pStage->glslShaderGroup[index];
            }
            else
            {
                S32 shaderAttribs = 0;
                
                if( tess.shader->numDeforms && !ShaderRequiresCPUDeforms( tess.shader ) )
                {
                    shaderAttribs |= GENERICDEF_USE_DEFORM_VERTEXES;
                }
                
                if( glState.vertexAnimation )
                {
                    shaderAttribs |= GENERICDEF_USE_VERTEX_ANIMATION;
                }
                
                if( pStage->stateBits & GLS_ATEST_BITS )
                {
                    shaderAttribs |= GENERICDEF_USE_TCGEN_AND_TCMOD;
                }
                
                sp = &tr.genericShader[shaderAttribs];
            }
        }
        else if( pStage->glslShaderGroup == tr.lightallShader )
        {
            S32 index = pStage->glslShaderIndex;
            
            if( backEnd.currentEntity && backEnd.currentEntity != &tr.worldEntity )
            {
                index |= LIGHTDEF_ENTITY;
            }
            
            if( r_sunlightMode->integer >= 2 && ( backEnd.viewParms.flags & VPF_USESUNLIGHT ) )
            {
                index |= LIGHTDEF_LIGHTTYPE_MASK;
                index |= LIGHTDEF_USE_SHADOWMAP;
            }
            
            if( r_lightmap->integer && ( ( index & LIGHTDEF_LIGHTTYPE_MASK ) == LIGHTDEF_USE_LIGHTMAP ) )
            {
                index = LIGHTDEF_USE_TCGEN_AND_TCMOD;
            }
            
            sp = &pStage->glslShaderGroup[index];
            
            backEnd.pc.c_lightallDraws++;
        }
        else
        {
            sp = GLSL_GetGenericShaderProgram( stage );
            
            backEnd.pc.c_genericDraws++;
        }
        
        GLSL_BindProgram( sp );
        
        if( pStage->isWater )
        {
            if( stage > 0 )
            {
                break;
            }
            
            sp = &tr.waterShader;
            pStage->glslShaderGroup = &tr.waterShader;
            GLSL_BindProgram( sp );
            GLSL_SetUniformFloat( sp, UNIFORM_TIME, tess.shaderTime );
            vec4_t loc0;
            
            VectorSet4( loc0, ( F32 )2.0, 0, 0, 0 ); // force it to use the old water fx...
            
            GLSL_SetUniformVec4( sp, UNIFORM_LOCAL0, loc0 );
            RB_SetMaterialBasedProperties( sp, pStage );
            
            GLSL_SetUniformInt( sp, UNIFORM_RANDOMMAP, TB_RANDOMMAP );
            GL_BindToTMU( tr.randomImage, TB_RANDOMMAP );
            GLSL_SetUniformInt( sp, UNIFORM_SCREENDEPTHMAP, TB_LEVELSMAP );
            GL_BindToTMU( tr.renderDepthImage, TB_LEVELSMAP );
        }
        else
        {
            if( !sp || !sp->program )
            {
                pStage->glslShaderGroup = tr.lightallShader;
                sp = &pStage->glslShaderGroup[0];
            }
            
            RB_SetMaterialBasedProperties( sp, pStage );
            
            GLSL_BindProgram( sp );
        }
        
        if( r_proceduralSun->integer && tess.shader == tr.sunShader )
        {
            // Special case for procedural sun...
            sp = &tr.sunPassShader;
            GLSL_BindProgram( sp );
            GLSL_SetUniformFloat( sp, UNIFORM_TIME, tess.shaderTime );
        }
        
        RB_SetMaterialBasedProperties( sp, pStage );
        
        RB_SetStageImageDimensions( sp, pStage );
        
        GLSL_SetUniformMat4( sp, UNIFORM_VIEWPROJECTIONMATRIX, backEnd.viewParms.projectionMatrix );
        GLSL_SetUniformMat4( sp, UNIFORM_MODELMATRIX, backEnd.orientation.transformMatrix );
        GLSL_SetUniformMat4( sp, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection );
        GLSL_SetUniformMat4( sp, UNIFORM_INVEYEPROJECTIONMATRIX, glState.invEyeProjection );
        
        {
            matrix_t trans, model, mvp, invTrans, normalMatrix, vp;
            
            Mat4Translation( backEnd.viewParms.orientation.origin, trans );
            Mat4Multiply( backEnd.viewParms.world.modelMatrix, trans, model );
            Mat4Multiply( backEnd.viewParms.projectionMatrix, model, mvp );
            //Mat4Multiply(backEnd.viewParms.projectionMatrix, trans, vp);
            Mat4Multiply( backEnd.viewParms.projectionMatrix, backEnd.viewParms.world.modelMatrix, vp );
            
            GLSL_SetUniformMat4( sp, UNIFORM_PROJECTIONMATRIX, glState.projection/*backEnd.viewParms.projectionMatrix*/ );
            GLSL_SetUniformMat4( sp, UNIFORM_MODELVIEWMATRIX, model ); //backEnd.viewParms.world.modelMatrix);
            GLSL_SetUniformMat4( sp, UNIFORM_VIEWMATRIX, trans );
        }
        
        GLSL_SetUniformVec3( sp, UNIFORM_VIEWORIGIN, backEnd.viewParms.orientation.origin );
        GLSL_SetUniformVec3( sp, UNIFORM_LOCALVIEWORIGIN, backEnd.orientation.viewOrigin );
        
        if( pStage->normalScale[0] == 0 && pStage->normalScale[1] == 0 && pStage->normalScale[2] == 0 )
        {
            vec4_t normalScale;
            VectorSet4( normalScale, r_baseNormalX->value, r_baseNormalY->value, 1.0f, r_baseParallax->value );
            GLSL_SetUniformVec4( sp, UNIFORM_NORMALSCALE, normalScale );
        }
        else
        {
            GLSL_SetUniformVec4( sp, UNIFORM_NORMALSCALE, pStage->normalScale );
        }
        
        GLSL_SetUniformFloat( sp, UNIFORM_VERTEXLERP, glState.vertexAttribsInterpolation );
        
        GLSL_SetUniformInt( sp, UNIFORM_DEFORMGEN, deformGen );
        if( deformGen != DGEN_NONE )
        {
            GLSL_SetUniformFloat5( sp, UNIFORM_DEFORMPARAMS, deformParams );
            GLSL_SetUniformFloat( sp, UNIFORM_TIME, tess.shaderTime );
        }
        
        if( input->fogNum )
        {
            GLSL_SetUniformVec4( sp, UNIFORM_FOGDISTANCE, fogDistanceVector );
            GLSL_SetUniformVec4( sp, UNIFORM_FOGDEPTH, fogDepthVector );
            GLSL_SetUniformFloat( sp, UNIFORM_FOGEYET, eyeT );
        }
        
        GL_State( pStage->stateBits );
        if( ( pStage->stateBits & GLS_ATEST_BITS ) == GLS_ATEST_GT_0 )
        {
            GLSL_SetUniformInt( sp, UNIFORM_ALPHATEST, 1 );
        }
        else if( ( pStage->stateBits & GLS_ATEST_BITS ) == GLS_ATEST_LT_80 )
        {
            GLSL_SetUniformInt( sp, UNIFORM_ALPHATEST, 2 );
        }
        else if( ( pStage->stateBits & GLS_ATEST_BITS ) == GLS_ATEST_GE_80 )
        {
            GLSL_SetUniformInt( sp, UNIFORM_ALPHATEST, 3 );
        }
        else
        {
            GLSL_SetUniformInt( sp, UNIFORM_ALPHATEST, 0 );
        }
        
        
        {
            vec4_t baseColor;
            vec4_t vertColor;
            
            ComputeShaderColors( pStage, baseColor, vertColor, pStage->stateBits );
            
            GLSL_SetUniformVec4( sp, UNIFORM_BASECOLOR, baseColor );
            GLSL_SetUniformVec4( sp, UNIFORM_VERTCOLOR, vertColor );
        }
        
        if( pStage->rgbGen == CGEN_LIGHTING_DIFFUSE )
        {
            vec4_t vec;
            
            VectorScale( backEnd.currentEntity->ambientLight, 1.0f / 255.0f, vec );
            GLSL_SetUniformVec3( sp, UNIFORM_AMBIENTLIGHT, vec );
            
            VectorScale( backEnd.currentEntity->directedLight, 1.0f / 255.0f, vec );
            GLSL_SetUniformVec3( sp, UNIFORM_DIRECTEDLIGHT, vec );
            
            VectorCopy( backEnd.currentEntity->lightDir, vec );
            vec[3] = 0.0f;
            GLSL_SetUniformVec4( sp, UNIFORM_LIGHTORIGIN, vec );
            GLSL_SetUniformVec3( sp, UNIFORM_MODELLIGHTDIR, backEnd.currentEntity->modelLightDir );
            
            GLSL_SetUniformFloat( sp, UNIFORM_LIGHTRADIUS, 0.0f );
        }
        
        if( pStage->alphaGen == AGEN_PORTAL )
        {
            GLSL_SetUniformFloat( sp, UNIFORM_PORTALRANGE, tess.shader->portalRange );
        }
        else if( pStage->alphaGen == AGEN_NORMALZFADE )
        {
            F32 lowest, highest;
            
            lowest = pStage->zFadeBounds[0];
            if( lowest == -1000 )     // use entity alpha
            {
                lowest = backEnd.currentEntity->e.shaderTime;
            }
            highest = pStage->zFadeBounds[1];
            if( highest == -1000 )    // use entity alpha
            {
                highest = backEnd.currentEntity->e.shaderTime;
            }
            
            GLSL_SetUniformFloat( sp, UNIFORM_ZFADELOWEST, lowest );
            GLSL_SetUniformFloat( sp, UNIFORM_ZFADEHIGHEST, highest );
        }
        
        GLSL_SetUniformInt( sp, UNIFORM_COLORGEN, pStage->rgbGen );
        GLSL_SetUniformInt( sp, UNIFORM_ALPHAGEN, pStage->alphaGen );
        
        if( input->fogNum )
        {
            vec4_t fogColorMask;
            
            ComputeFogColorMask( pStage, fogColorMask );
            
            GLSL_SetUniformVec4( sp, UNIFORM_FOGCOLORMASK, fogColorMask );
        }
        
        if( r_lightmap->integer )
        {
            vec4_t v;
            VectorSet4( v, 1.0f, 0.0f, 0.0f, 1.0f );
            GLSL_SetUniformVec4( sp, UNIFORM_DIFFUSETEXMATRIX, v );
            VectorSet4( v, 0.0f, 0.0f, 0.0f, 0.0f );
            GLSL_SetUniformVec4( sp, UNIFORM_DIFFUSETEXOFFTURB, v );
            
            GLSL_SetUniformInt( sp, UNIFORM_TCGEN0, TCGEN_LIGHTMAP );
        }
        else
        {
            ComputeTexMods( pStage, TB_DIFFUSEMAP, texMatrix, texOffTurb );
            GLSL_SetUniformVec4( sp, UNIFORM_DIFFUSETEXMATRIX, texMatrix );
            GLSL_SetUniformVec4( sp, UNIFORM_DIFFUSETEXOFFTURB, texOffTurb );
            
            GLSL_SetUniformInt( sp, UNIFORM_TCGEN0, pStage->bundle[0].tcGen );
            if( pStage->bundle[0].tcGen == TCGEN_VECTOR )
            {
                vec3_t vec;
                
                VectorCopy( pStage->bundle[0].tcGenVectors[0], vec );
                GLSL_SetUniformVec3( sp, UNIFORM_TCGEN0VECTOR0, vec );
                VectorCopy( pStage->bundle[0].tcGenVectors[1], vec );
                GLSL_SetUniformVec3( sp, UNIFORM_TCGEN0VECTOR1, vec );
            }
        }
        
        GLSL_SetUniformMat4( sp, UNIFORM_MODELMATRIX, backEnd.orientation.transformMatrix );
        
        GLSL_SetUniformVec4( sp, UNIFORM_NORMALSCALE, pStage->normalScale );
        
        {
            vec4_t specularScale;
            Vector4Copy( pStage->specularScale, specularScale );
            
            if( renderToCubemap )
            {
                // force specular to nonmetal if rendering cubemaps
                if( r_pbr->integer )
                    specularScale[1] = 0.0f;
            }
            
            GLSL_SetUniformVec4( sp, UNIFORM_SPECULARSCALE, specularScale );
        }
        
        if( pStage->bundle[TB_STEEPMAP].image[0] )
        {
            //ri->Printf(PRINT_WARNING, "Image bound to steep map %i x %i.\n", pStage->bundle[TB_STEEPMAP].image[0]->width, pStage->bundle[TB_STEEPMAP].image[0]->height);
            R_BindAnimatedImageToTMU( &pStage->bundle[TB_STEEPMAP], TB_STEEPMAP );
        }
        else
        {
            GL_BindToTMU( tr.whiteImage, TB_STEEPMAP );
        }
        
        if( pStage->bundle[TB_SUBSURFACEMAP].image[0] )
        {
            R_BindAnimatedImageToTMU( &pStage->bundle[TB_SUBSURFACEMAP], TB_SUBSURFACEMAP );
        }
        else
        {
            GL_BindToTMU( tr.whiteImage, TB_SUBSURFACEMAP );
        }
        
        if( pStage->bundle[TB_OVERLAYMAP].image[0] )
        {
            R_BindAnimatedImageToTMU( &pStage->bundle[TB_OVERLAYMAP], TB_OVERLAYMAP );
        }
        else
        {
            GL_BindToTMU( tr.blackImage, TB_OVERLAYMAP );
        }
        
        if( !backEnd.depthFill )
        {
            if( r_sunlightMode->integer && ( r_sunlightSpecular->integer || ( backEnd.viewParms.flags & VPF_USESUNLIGHT ) ) )
            {
                if( backEnd.viewParms.flags & VPF_USESUNLIGHT )
                {
                    GL_BindToTMU( tr.screenShadowImage, TB_SHADOWMAP );
                }
                else
                {
                    GL_BindToTMU( tr.whiteImage, TB_SHADOWMAP );
                }
                
                GLSL_SetUniformVec3( sp, UNIFORM_PRIMARYLIGHTAMBIENT, backEnd.refdef.sunAmbCol );
                GLSL_SetUniformVec3( sp, UNIFORM_PRIMARYLIGHTCOLOR, backEnd.refdef.sunCol );
                GLSL_SetUniformVec4( sp, UNIFORM_PRIMARYLIGHTORIGIN, backEnd.refdef.sunDir );
                
                
                GLSL_SetUniformInt( sp, UNIFORM_LIGHTCOUNT, NUM_CLOSE_LIGHTS );
                GLSL_SetUniformVec3x16( sp, UNIFORM_LIGHTPOSITIONS2, CLOSEST_LIGHTS_POSITIONS, MAX_LIGHTALL_DLIGHTS );
                GLSL_SetUniformVec3x16( sp, UNIFORM_LIGHTCOLORS, CLOSEST_LIGHTS_COLORS, MAX_LIGHTALL_DLIGHTS );
                GLSL_SetUniformFloatx16( sp, UNIFORM_LIGHTDISTANCES, CLOSEST_LIGHTS_DISTANCES, MAX_LIGHTALL_DLIGHTS );
            }
        }
        
        GL_BindToTMU( tr.whiteImage, TB_NORMALMAP );
        
        //
        // do multitexture
        //
        if( backEnd.depthFill )
        {
            if( !( pStage->stateBits & GLS_ATEST_BITS ) )
                GL_BindToTMU( tr.whiteImage, TB_COLORMAP );
            else if( pStage->bundle[TB_COLORMAP].image[0] != 0 )
                R_BindAnimatedImageToTMU( &pStage->bundle[TB_COLORMAP], TB_COLORMAP );
        }
        else if( pStage->glslShaderGroup == tr.lightallShader )
        {
            S32 i;
            vec4_t enableTextures;
            
            if( r_sunlightMode->integer && ( backEnd.viewParms.flags & VPF_USESUNLIGHT ) && ( pStage->glslShaderIndex & LIGHTDEF_LIGHTTYPE_MASK ) )
            {
                // FIXME: screenShadowImage is NULL if no framebuffers
                if( tr.screenShadowImage )
                    GL_BindToTMU( tr.screenShadowImage, TB_SHADOWMAP );
                GLSL_SetUniformVec3( sp, UNIFORM_PRIMARYLIGHTAMBIENT, backEnd.refdef.sunAmbCol );
                if( r_pbr->integer )
                {
                    vec3_t color;
                    
                    color[0] = backEnd.refdef.sunCol[0] * backEnd.refdef.sunCol[0];
                    color[1] = backEnd.refdef.sunCol[1] * backEnd.refdef.sunCol[1];
                    color[2] = backEnd.refdef.sunCol[2] * backEnd.refdef.sunCol[2];
                    GLSL_SetUniformVec3( sp, UNIFORM_PRIMARYLIGHTCOLOR, color );
                }
                else
                {
                    GLSL_SetUniformVec3( sp, UNIFORM_PRIMARYLIGHTCOLOR, backEnd.refdef.sunCol );
                }
                GLSL_SetUniformVec4( sp, UNIFORM_PRIMARYLIGHTORIGIN, backEnd.refdef.sunDir );
            }
            
            VectorSet4( enableTextures, 0, 0, 0, 0 );
            if( ( r_lightmap->integer == 1 || r_lightmap->integer == 2 ) && pStage->bundle[TB_LIGHTMAP].image[0] )
            {
                for( i = 0; i < NUM_TEXTURE_BUNDLES; i++ )
                {
                    if( i == TB_COLORMAP )
                        R_BindAnimatedImageToTMU( &pStage->bundle[TB_LIGHTMAP], i );
                    else
                        GL_BindToTMU( tr.whiteImage, i );
                }
            }
            else if( r_lightmap->integer == 3 && pStage->bundle[TB_DELUXEMAP].image[0] )
            {
                for( i = 0; i < NUM_TEXTURE_BUNDLES; i++ )
                {
                    if( i == TB_COLORMAP )
                        R_BindAnimatedImageToTMU( &pStage->bundle[TB_DELUXEMAP], i );
                    else
                        GL_BindToTMU( tr.whiteImage, i );
                }
            }
            else
            {
                bool light = ( pStage->glslShaderIndex & LIGHTDEF_LIGHTTYPE_MASK ) != 0;
                bool fastLight = !( r_normalMapping->integer || r_specularMapping->integer );
                
                if( pStage->bundle[TB_DIFFUSEMAP].image[0] )
                    R_BindAnimatedImageToTMU( &pStage->bundle[TB_DIFFUSEMAP], TB_DIFFUSEMAP );
                    
                if( pStage->bundle[TB_LIGHTMAP].image[0] )
                    R_BindAnimatedImageToTMU( &pStage->bundle[TB_LIGHTMAP], TB_LIGHTMAP );
                    
                // bind textures that are sampled and used in the glsl shader, and
                // bind whiteImage to textures that are sampled but zeroed in the glsl shader
                //
                // alternatives:
                //  - use the last bound texture
                //     -> costs more to sample a higher res texture then throw out the result
                //  - disable texture sampling in glsl shader with #ifdefs, as before
                //     -> increases the number of shaders that must be compiled
                //
                if( ( light || ( pStage->isWater ) || pStage->hasRealNormalMap || pStage->hasSpecular || pStage->hasRealSubsurfaceMap || pStage->hasRealOverlayMap || pStage->hasRealSteepMap ) && !fastLight )
                {
                    if( pStage->bundle[TB_NORMALMAP].image[0] )
                    {
                        R_BindAnimatedImageToTMU( &pStage->bundle[TB_NORMALMAP], TB_NORMALMAP );
                        enableTextures[0] = 1.0f;
                    }
                    else if( r_normalMapping->integer )
                    {
                        GL_BindToTMU( tr.whiteImage, TB_NORMALMAP );
                    }
                    
                    if( pStage->bundle[TB_DELUXEMAP].image[0] )
                    {
                        R_BindAnimatedImageToTMU( &pStage->bundle[TB_DELUXEMAP], TB_DELUXEMAP );
                        enableTextures[1] = 1.0f;
                    }
                    else if( r_deluxeMapping->integer )
                    {
                        GL_BindToTMU( tr.whiteImage, TB_DELUXEMAP );
                    }
                    
                    if( pStage->bundle[TB_SPECULARMAP].image[0] )
                    {
                        R_BindAnimatedImageToTMU( &pStage->bundle[TB_SPECULARMAP], TB_SPECULARMAP );
                        enableTextures[2] = 1.0f;
                    }
                    else if( r_specularMapping->integer )
                    {
                        GL_BindToTMU( tr.whiteImage, TB_SPECULARMAP );
                    }
                }
                
                if( input->cubemapIndex )
                    enableTextures[3] = ( r_cubeMapping->integer && !( tr.viewParms.flags & VPF_NOCUBEMAPS ) && input->cubemapIndex ) ? 1.0f : 0.0f;
                else
                    enableTextures[3] = 0.0f;
            }
            
            GLSL_SetUniformVec4( sp, UNIFORM_ENABLETEXTURES, enableTextures );
        }
        else if( pStage->bundle[TB_LIGHTMAP].image[0] != 0 )
        {
            R_BindAnimatedImageToTMU( &pStage->bundle[0], 0 );
            R_BindAnimatedImageToTMU( &pStage->bundle[1], 1 );
        }
        else
        {
            //
            // set state
            //
            R_BindAnimatedImageToTMU( &pStage->bundle[0], 0 );
        }
        
        //
        // testing cube map
        //
        if( !( tr.viewParms.flags & VPF_NOCUBEMAPS ) && input->cubemapIndex && r_cubeMapping->integer )
        {
            vec4_t vec;
            cubemap_t* cubemap = &tr.cubemaps[input->cubemapIndex - 1];
            
            // FIXME: cubemap image could be NULL if cubemap isn't renderer or loaded
            if( cubemap->image )
                GL_BindToTMU( cubemap->image, TB_CUBEMAP );
                
            VectorSubtract( cubemap->origin, backEnd.viewParms.orientation.origin, vec );
            vec[3] = 1.0f;
            
            F32 dist = Distance( tr.refdef.vieworg, cubemap->origin );
            F32 mult = r_cubemapCullFalloffMult->value - ( r_cubemapCullFalloffMult->value * 0.04 );
            
            if( dist < r_cubemapCullRange->value )
            {
                // In range for full effect...
                GLSL_SetUniformFloat( sp, UNIFORM_CUBEMAPSTRENGTH, 1.0 );
            }
            else if( dist >= r_cubemapCullRange->value && dist < r_cubemapCullRange->value * mult )
            {
                // Further scale the strength of the cubemap by the fade-out distance...
                F32 extraDist =		dist - r_cubemapCullRange->value;
                F32 falloffDist =	( r_cubemapCullRange->value * mult ) - r_cubemapCullRange->value;
                F32 strength =	( falloffDist - extraDist ) / falloffDist;
                
                strength = CLAMP( strength, 0.0, 1.0 );
                GLSL_SetUniformFloat( sp, UNIFORM_CUBEMAPSTRENGTH, strength );
            }
            else
            {
                // Out of range completely...
                GLSL_SetUniformFloat( sp, UNIFORM_CUBEMAPSTRENGTH, 0.0 );
            }
            
            VectorScale4( vec, 1.0f / cubemap->parallaxRadius, vec );
            
            GLSL_SetUniformVec4( sp, UNIFORM_CUBEMAPINFO, vec );
        }
        
        bool tesselation = false;
        
        if( r_tesselation->integer && sp->tesselation )
        {
            tesselation = true;
            
            F32 tessInner = RB_GetTesselationInnerLevel( tess.shader->surfaceFlags & MATERIAL_MASK );
            F32 tessOuter = tessInner;//RB_GetTesselationOuterLevel(tess.shader->surfaceFlags & MATERIAL_MASK);
            F32 tessAlpha = RB_GetTesselationAlphaLevel( tess.shader->surfaceFlags & MATERIAL_MASK );
            
            if( tessInner <= 1.0 )
            {
                tessAlpha = 0.1;
            }
            
            vec4_t l10;
            VectorSet4( l10, tessAlpha, tessInner, tessOuter, 0.0 );
            GLSL_SetUniformVec4( sp, UNIFORM_LOCAL1, l10 );
        }
        
        //
        // draw
        //
        R_DrawElements( input->numIndexes, input->firstIndex, tesselation );
        
        // allow skipping out to show just lightmaps during development
        if( r_lightmap->integer && ( pStage->bundle[0].isLightmap || pStage->bundle[1].isLightmap ) )
        {
            break;
        }
        
        if( backEnd.depthFill )
            break;
    }
}

static void RB_RenderShadowmap( shaderCommands_t* input )
{
    S32 deformGen;
    vec5_t deformParams;
    
    ComputeDeformValues( &deformGen, deformParams );
    
    {
        shaderProgram_t* sp = &tr.shadowmapShader;
        
        vec4_t vector;
        
        GLSL_BindProgram( sp );
        
        GLSL_SetUniformMat4( sp, UNIFORM_MODELVIEWPROJECTIONMATRIX, glState.modelviewProjection );
        
        GLSL_SetUniformMat4( sp, UNIFORM_MODELMATRIX, backEnd.orientation.transformMatrix );
        
        GLSL_SetUniformFloat( sp, UNIFORM_VERTEXLERP, glState.vertexAttribsInterpolation );
        
        GLSL_SetUniformInt( sp, UNIFORM_DEFORMGEN, deformGen );
        if( deformGen != DGEN_NONE )
        {
            GLSL_SetUniformFloat5( sp, UNIFORM_DEFORMPARAMS, deformParams );
            GLSL_SetUniformFloat( sp, UNIFORM_TIME, tess.shaderTime );
        }
        
        VectorCopy( backEnd.viewParms.orientation.origin, vector );
        vector[3] = 1.0f;
        GLSL_SetUniformVec4( sp, UNIFORM_LIGHTORIGIN, vector );
        GLSL_SetUniformFloat( sp, UNIFORM_LIGHTRADIUS, backEnd.viewParms.zFar );
        
        GL_State( 0 );
        GLSL_SetUniformInt( sp, UNIFORM_ALPHATEST, 0 );
        
        //
        // do multitexture
        //
        //if ( pStage->glslShaderGroup )
        {
            //
            // draw
            //
            
            R_DrawElements( input->numIndexes, input->firstIndex, false );
        }
    }
}

//#define ___OLD_DLIGHT_CODE___

/*
** RB_StageIteratorGeneric
*/
void RB_StageIteratorGeneric( void )
{
    shaderCommands_t* input;
    U32 vertexAttribs = 0;
    
    input = &tess;
    
    if( !input->numVertexes || !input->numIndexes )
    {
        return;
    }
    
    if( tess.useInternalVao )
    {
        RB_DeformTessGeometry();
    }
    
    vertexAttribs = RB_CalcShaderVertexAttribs( input );
    
    if( tess.useInternalVao )
    {
        RB_UpdateTessVao( vertexAttribs );
    }
    else
    {
        backEnd.pc.c_staticVaoDraws++;
    }
    
    //
    // log this call
    //
    if( r_logFile->integer )
    {
        // don't just call LogComment, or we will get
        // a call to va() every frame!
        GLimp_LogComment( ( UTF8* )va( "--- RB_StageIteratorGeneric( %s ) ---\n", tess.shader->name ) );
    }
    
    //
    // set face culling appropriately
    //
    if( input->shader->cullType == CT_TWO_SIDED )
    {
        GL_Cull( CT_TWO_SIDED );
    }
    else
    {
        bool cullFront = ( input->shader->cullType == CT_FRONT_SIDED );
        
        if( backEnd.viewParms.flags & VPF_DEPTHSHADOW )
            cullFront = !cullFront;
            
        if( backEnd.viewParms.isMirror )
            cullFront = !cullFront;
            
        if( backEnd.currentEntity && backEnd.currentEntity->mirrored )
            cullFront = !cullFront;
            
        if( cullFront )
            GL_Cull( CT_FRONT_SIDED );
        else
            GL_Cull( CT_BACK_SIDED );
    }
    
    // set polygon offset if necessary
    if( input->shader->polygonOffset )
    {
        glEnable( GL_POLYGON_OFFSET_FILL );
    }
    
    //
    // Set up any special shaders needed for this surface/contents type...
    //
    if( ( tess.shader->contentFlags & CONTENTS_WATER ) )
    {
        if( input->xstages[0]->isWater != true ) // In case it is already set, no need looping more then once on the same shader...
            for( S32 stage = 0; stage < MAX_SHADER_STAGES; stage++ )
                if( input->xstages[stage] )
                    input->xstages[stage]->isWater = true;
    }
    
    
    //
    // render depth if in depthfill mode
    //
    if( backEnd.depthFill )
    {
        /*if (tr.currentEntity && tr.currentEntity != &tr.worldEntity)
        {
        if (backEnd.currentEntity->e.ignoreCull)
        {
        //model_t	*model = R_GetModelByHandle(backEnd.currentEntity->e.hModel);
        //if (model)
        //	CL_RefPrintf(PRINT_WARNING, "Cull ignored on model type: %i. name: %s.\n", model->type, model->name);
        //else
        //	CL_RefPrintf(PRINT_WARNING, "Cull ignored on unknown.\n");
        if (input->shader->polygonOffset)
        {
        qglDisable(GL_POLYGON_OFFSET_FILL);
        }
        
        //if (model->type == MOD_BRUSH || model->data.bmodel || !tr.currentModel)
        return;
        }
        else if (tr.currentEntity->e.reType == RT_MODEL)
        {
        model_t	*model = R_GetModelByHandle(backEnd.currentEntity->e.hModel);
        
        if (model->type == MOD_BRUSH || model->data.bmodel || !tr.currentModel)
        return;
        }
        }*/
        
        RB_IterateStagesGeneric( input );
        
        //
        // reset polygon offset
        //
        if( input->shader->polygonOffset )
        {
            glDisable( GL_POLYGON_OFFSET_FILL );
        }
        
        return;
    }
    
    //
    // render shadowmap if in shadowmap mode
    //
    if( backEnd.viewParms.flags & VPF_SHADOWMAP )
    {
        if( input->shader->sort == SS_OPAQUE )
        {
            RB_RenderShadowmap( input );
        }
        
        //
        // reset polygon offset
        //
        if( input->shader->polygonOffset )
        {
            glDisable( GL_POLYGON_OFFSET_FILL );
        }
        
        return;
    }
    
    //
    //
    // call shader function
    //
    RB_IterateStagesGeneric( input );
    
#ifdef ___OLD_DLIGHT_CODE___
    //
    // pshadows!
    //
    if( r_shadows->integer == 4 && tess.pshadowBits && tess.shader->sort <= SS_OPAQUE && !( tess.shader->surfaceFlags & ( SURF_NODLIGHT | SURF_SKY ) ) )
    {
        ProjectPshadowVBOGLSL();
    }
    
    //
    // now do any dynamic lighting needed
    //
    if( tess.dlightBits && tess.shader->lightingStage >= 0 )
    {
        if( r_dlightMode->integer )
        {
            ForwardDlight();
        }
        else
        {
            ProjectDlightTexture();
        }
    }
#else //!___OLD_DLIGHT_CODE___
    //
    // now do any dynamic lighting needed. UQ1: A generic method to rule them all... A SANE real world style lighting with a blacklist - not a whitelist!
    //
    if( !( tess.shader->surfaceFlags & ( /*SURF_NODLIGHT |*/ SURF_SKY ) ) )
        //if( !( tess.shader->surfaceFlags & ( SURF_NODLIGHT | SURF_SKY ) ) && tess.dlightBits && tess.shader->sort <= SS_OPAQUE )
    {
        switch( int( tess.shader->sort ) )
        {
            case SS_PORTAL:
            case SS_ENVIRONMENT: // is this really always a skybox???
            case SS_SEE_THROUGH:
                //case SS_FOG: // hmm... these??? i sorta like the idea of lighting up fog particles myself...
            case SS_BLEND0:
            case SS_BLEND1:
            case SS_BLEND2:
            case SS_BLEND3:
            case SS_BLEND6:
                break;
            default:
                if( r_dlightMode->integer >= 2 )
                {
                    ForwardDlight();
                }
                else
                {
                    ProjectDlightTexture();
                }
    
                //
                // pshadows!
                //
                if( r_shadows->integer == 4 && tess.pshadowBits && !( tess.shader->surfaceFlags & ( /*SURF_NODLIGHT |*/ SURF_SKY ) ) )
                    ProjectPshadowVBOGLSL();
                break;
        }
    }
#endif //___OLD_DLIGHT_CODE___
    
    //
    // now do fog
    //
    if( tess.fogNum && tess.shader->fogPass )
    {
        RB_FogPass();
    }
    
    //
    // reset polygon offset
    //
    if( input->shader->polygonOffset )
    {
        glDisable( GL_POLYGON_OFFSET_FILL );
    }
}

/*
** RB_EndSurface
*/
void RB_EndSurface( void )
{
    shaderCommands_t* input;
    
    input = &tess;
    
    if( input->numIndexes == 0 || input->numVertexes == 0 )
    {
        return;
    }
    
    if( input->indexes[SHADER_MAX_INDEXES - 1] != 0 )
    {
        Com_Error( ERR_DROP, "RB_EndSurface() - SHADER_MAX_INDEXES hit" );
    }
    if( input->xyz[SHADER_MAX_VERTEXES - 1][0] != 0 )
    {
        Com_Error( ERR_DROP, "RB_EndSurface() - SHADER_MAX_VERTEXES hit" );
    }
    
    if( tess.shader == tr.shadowShader )
    {
        RB_ShadowTessEnd();
        return;
    }
    
    // for debugging of sort order issues, stop rendering after a given sort value
    if( r_debugSort->integer && r_debugSort->integer < tess.shader->sort )
    {
        return;
    }
    
    if( tess.useCacheVao )
    {
        // upload indexes now
        VaoCache_Commit();
    }
    
    //
    // update performance counters
    //
    backEnd.pc.c_shaders++;
    backEnd.pc.c_vertexes += tess.numVertexes;
    backEnd.pc.c_indexes += tess.numIndexes;
    backEnd.pc.c_totalIndexes += tess.numIndexes * tess.numPasses;
    
    //
    // call off to shader specific tess end function
    //
    tess.currentStageIteratorFunc();
    
    //
    // draw debugging stuff
    //
    if( r_showtris->integer )
    {
        DrawTris( input );
    }
    if( r_shownormals->integer )
    {
        DrawNormals( input );
    }
    // clear shader so we can tell we don't have any unclosed surfaces
    tess.numIndexes = 0;
    tess.numVertexes = 0;
    tess.firstIndex = 0;
    
    GLimp_LogComment( "----------\n" );
}
