//////////////////////////////////////////////////////////////////////////////////////
//
//  This file is part of OWEngine source code.
//  Copyright (C) 2011 JV Software
//  Copyright (C) 2016 Dusan Jocic <dusanjocic@msn.com>
//
//  This file is part of the OWEngine single player GPL Source Code.
//
//  OWengine Source Code is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  OWEngine Source Code is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with OWEngine Source Code. If not, see <http://www.gnu.org/licenses/>.
//
//  In addition, the OWEngine Source Code is also subject to certain additional terms.
//  You should have received a copy of these additional terms immediately following
//  the terms and conditions of the GNU General Public License which accompanied the
//  OWEngine Source Code. If not, please request a copy in writing from id Software
//  at the address below.
//
//  If you have questions concerning this license or the applicable additional terms,
//  you may contact in writing id Software LLC, c/o ZeniMax Media Inc.,
//  Suite 120, Rockville, Maryland 20850 USA.
//
// -------------------------------------------------------------------------
//  File name:   cmfile.cpp
//  Version:     v1.01
//  Created:
//  Compilers:   Visual Studio 2015, gcc 7.3.0
//  Description:
// -------------------------------------------------------------------------
//  History:
//
//////////////////////////////////////////////////////////////////////////////////////

#include <OWLib/math_vector.h>
#include <OWLib/math_quaternion.h>
#include <OWLib/util_list.h>

//#define vec2_t idVec2

#include "q3map2.h"

#include <cm/cm_traceModel.h>
#include <cm/cm_header.h>
#include <API/physics_api.h>

// Nasty, ugly, shitty...life sucks.
class idCollisionModelManager* collisionModelManager;

#if 0
#undef qboolean
#undef qfalse
#undef qtrue

typedef enum
{
    qfalse,
    qtrue
} qboolean;
#endif

idList<cmBrushVertex_t> vertexes;
idList<cmBrushSurface_t> brushSurfs;
idList<cmBrushModel_t> brushModels;

typedef struct model_s model_t;
void* R_GetSurfaceForModel( model_t* model, int surfacenum )
{
    return NULL;
}
void* R_GetVertexesForModel( model_t* model, int startVertex )
{
    return NULL;
}
short* R_GetIndexesForModel( model_t* model, int startIndex )
{
    return NULL;
}
int R_GetModelSurfaceCount( model_t* model )
{
    return NULL;
}
void FS_FreeFile( void* ) { }
int FS_ReadFile( const char*, void** )
{
    return -1;
};
void Hunk_FreeTempMemory( void* ) { }

/*
=================
WriteCollisionModel
=================
*/
void WriteCollisionModel( const char* bspfile )
{
    for( int i = 1; i < numBSPModels; i++ )
    {
        bspModel_t* bspModel = &bspModels[i];
        cmBrushModel_t cmModel;
        
        cmModel.startSurface = brushSurfs.Num();
        cmModel.numSurfaces = bspModel->numBSPSurfaces;
        for( int s = bspModel->firstBSPSurface; s < bspModel->firstBSPSurface + bspModel->numBSPSurfaces; s++ )
        {
            bspDrawSurface_t* surface = &bspDrawSurfaces[s];
            cmBrushSurface_t brushSurf;
            
            brushSurf.startVertex = vertexes.Num();
            brushSurf.numVertexes = 0;
            //for( int c = surface->firstVert; c < surface->firstVert + surface->numVerts; c++ ) {
            for( int c = surface->firstIndex; c < surface->firstIndex + surface->numIndexes; c++ )
            {
                cmBrushVertex_t vertex;
                bspDrawVert_t* drawVert = &bspDrawVerts[ surface->firstVert + bspDrawIndexes[c] ];
                
                vertex.xyz.set( drawVert->xyz[0], drawVert->xyz[1], drawVert->xyz[2] );
                
                brushSurf.numVertexes++;
                vertexes.Append( vertex );
            }
            
            brushSurfs.Append( brushSurf );
        }
        
        brushModels.Append( cmModel );
    }
    
    printf( "...%d bmodels, %d brushSurfs, %d vertexes \n", brushModels.Num(), brushSurfs.Num(), vertexes.Num() );
    
    FILE* file = fopen( bspfile, "wb" );
    if( !file )
    {
        printf( "WARNING: WriteCollisionModel failed to open file for writing. \n" );
        return;
    }
    
    cmHeader_t header;
    
    header.iden = CM_IDENT;
    header.version = CM_VERSION;
    header.numBrushModels = brushModels.Num();
    header.numVertexes = vertexes.Num();
    
    fwrite( &header, sizeof( cmHeader_t ), 1, file );
    
    header.bmodelOffest = ftell( file );
    fwrite( &brushModels[0], sizeof( cmBrushModel_t ), brushModels.Num(), file );
    
    header.surfaceOffset = ftell( file );
    fwrite( &brushSurfs[0], sizeof( cmBrushSurface_t ), brushSurfs.Num(), file );
    
    header.vertexOffset = ftell( file );
    fwrite( &vertexes[0], sizeof( cmBrushVertex_t ), vertexes.Num(), file );
    
    fseek( file, 0, SEEK_SET );
    fwrite( &header, sizeof( cmHeader_t ), 1, file );
    
    brushModels.Clear();
    brushSurfs.Clear();
    vertexes.Clear();
    
    fclose( file );
}

/*
=================
BuildCollisionModelFromBSP
=================
*/
qboolean BuildCollisionModelFromBSP( const char* bspfile )
{
    char source[1024], cmfilepath[1024], bulletfilepath[1024];
    Sys_Printf( "-------- BuildCollisionModelFromBSP -----------\n" );
    
    strcpy( source, ExpandArg( bspfile ) );
    StripExtension( source );
    
    strcpy( cmfilepath, source );
    strcpy( bulletfilepath, source );
    
    DefaultExtension( source, ".world" );
    DefaultExtension( cmfilepath, ".cm" );
    DefaultExtension( bulletfilepath, ".bullet" );
    
    Sys_Printf( "Loading %s\n", source );
    
    // Load the BSP file.
    LoadBSPFile( source );
    
    /* parse bsp entities */
    ParseEntities();
    
    physicsManager->Init();
    idPhysicsAbstractBSP bsp;
    
    bsp.numShaders = numBSPShaders;
    bsp.shaders = &bspShaders[0];
    
    bsp.numLeafs = numBSPLeafs;
    bsp.leafs = &bspLeafs[0];
    
    bsp.numPlanes = numBSPPlanes;
    bsp.planes = &bspPlanes[0];
    
    bsp.numNodes = numBSPNodes;
    bsp.nodes = &bspNodes[0];
    
    bsp.numLeafSurfaces = numBSPLeafSurfaces;
    bsp.leafSurfaces = &bspLeafSurfaces[0];
    
    bsp.numLeafBrushes = numBSPLeafBrushes;
    bsp.leafBrushes = &bspLeafBrushes[0];
    
    bsp.numBrushes = numBSPBrushes;
    bsp.brushes = &bspBrushes[0];
    
    bsp.numBrushSides = numBSPBrushSides;
    bsp.brushSides = &bspBrushSides[0];
    
    printf( "Creating Collision Model from internal BSP...\n" );
    physicsManager->CreateCollisionModelFromAbstractBSP( &bsp );
    
    printf( "Writing Bullet File...\n" );
    physicsManager->WriteBulletPhysicsFile( bulletfilepath );
    
    printf( "Writing CM File...\n" );
    // Write the extra information we need for collision detection.
    WriteCollisionModel( cmfilepath );
    
    return qtrue;
}
