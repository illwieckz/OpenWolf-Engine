////////////////////////////////////////////////////////////////////////////////////////
// Copyright(C) 1999 - 2010 id Software LLC, a ZeniMax Media company.
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
// File name:   cm_model.h
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#ifndef __CM_MODEL_H__
#define __CM_MODEL_H__

//
// idCollisionShape_t
//
struct idCollisionShape_t
{
    S32 numVertexes;
    idVec3*	xyz;
};

//
// idCollisionModel
//
class idCollisionModel
{
public:
    void			Init( cmHeader_t* cm );
    idCollisionShape_t* GetBrushShape( S32 brushNum );
private:
    cmBrushModel_t*		GetModel( cmHeader_t* cm, S32 modelNum );
    cmBrushSurface_t*	GetSurface( cmHeader_t* cm, S32 surfaceNum );
    cmBrushVertex_t*		GetVertex( cmHeader_t* cm, S32 vertexNum );
    idCollisionShape_t*	cm_shapes;
    idVec3*				cm_shape_vertexes;
};

ID_INLINE cmBrushModel_t* idCollisionModel::GetModel( cmHeader_t* cm, S32 modelNum )
{
    return ( cmBrushModel_t* )( ( U8* )cm + cm->bmodelOffest + ( sizeof( cmBrushModel_t ) * modelNum ) );
}

ID_INLINE cmBrushSurface_t* idCollisionModel::GetSurface( cmHeader_t* cm, S32 surfaceNum )
{
    return ( cmBrushSurface_t* )( ( U8* )cm + cm->surfaceOffset + ( sizeof( cmBrushSurface_t ) * surfaceNum ) );
}

ID_INLINE cmBrushVertex_t* idCollisionModel::GetVertex( cmHeader_t* cm, S32 vertexNum )
{
    return ( cmBrushVertex_t* )( ( U8* )cm + cm->vertexOffset + ( sizeof( cmBrushVertex_t ) * vertexNum ) );
}

#endif //!__CM_MODEL_H__