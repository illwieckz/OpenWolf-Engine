////////////////////////////////////////////////////////////////////////////////////////
// Copyright(C) 2011 JV Software
// Copyright(C) 2011 - 2018 Dusan Jocic <dusanjocic@msn.com>
//
// This file is part of the OpenWolf GPL Source Code.
// OpenWolf Source Code is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// OpenWolf Source Code is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with OpenWolf Source Code.  If not, see <http://www.gnu.org/licenses/>.
//
// In addition, the OpenWolf Source Code is also subject to certain additional terms.
// You should have received a copy of these additional terms immediately following the
// terms and conditions of the GNU General Public License which accompanied the
// OpenWolf Source Code. If not, please request a copy in writing from id Software
// at the address below.
//
// If you have questions concerning this license or the applicable additional terms,
// you may contact in writing id Software LLC, c/o ZeniMax Media Inc.,
// Suite 120, Rockville, Maryland 20850 USA.
//
// -------------------------------------------------------------------------------------
// File name:   cm_model.cpp
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#include <OWLIb/precompiled.h>

/*
===============
idCollisionModel::Init
===============
*/
void idCollisionModel::Init( cmHeader_t* cm )
{
#ifndef Q3MAP2
    cm_shapes = ( idCollisionShape_t* )Hunk_Alloc( cm->numBrushModels * sizeof( cmBrushModel_t ), h_low );
    cm_shape_vertexes = ( idVec3* )Hunk_Alloc( cm->numVertexes * sizeof( idVec3 ), h_low );
    
    memcpy( cm_shape_vertexes, GetVertex( cm, 0 ), cm->numVertexes * sizeof( idVec3 ) );
    
    idCollisionShape_t* shape = cm_shapes;
    
    idVec3* startVertex = cm_shape_vertexes;
    for( S32 i = 0; i < cm->numBrushModels; i++, shape++ )
    {
        cmBrushModel_t* model = GetModel( cm, i );
        shape->xyz = startVertex;
        shape->numVertexes = 0;
        for( S32 c = 0; c < model->numSurfaces; c++ )
        {
            cmBrushSurface_t* surface = GetSurface( cm, model->startSurface + c );
            
            //shape->xyz = cm_shape_vertexes + surface->startVertex;
            shape->numVertexes += surface->numVertexes;
        }
        
        startVertex += shape->numVertexes;
    }
#endif
}

/*
===============
idCollisionModel::GetBrushShape
===============
*/
idCollisionShape_t* idCollisionModel::GetBrushShape( S32 brushNum )
{
    return &cm_shapes[ brushNum - 1 ];
}