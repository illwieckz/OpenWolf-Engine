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
// File name:   cm_header.h
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#ifndef __CM_HEADER_H__
#define __CM_HEADER_H__

#ifndef __MATH_VECTOR_H__
#include <OWLib/math_vector.h>
#endif

/*
==============================================================================

.CM collision file format

==============================================================================
*/

#define CM_IDENT   ( ( 'D' << 24 ) + ( 'M' << 16 ) + ( 'C' << 8 ) + 'I' )
#define CM_VERSION 1

typedef struct
{
    S32			iden;
    S32			version;
    S32			checksum;
    
    S32			numBrushModels;
    S32			bmodelOffest;
    
    S32			surfaceOffset;
    
    S32			numVertexes;
    S32			vertexOffset;
} cmHeader_t;

typedef struct
{
    S32			startSurface;
    S32			numSurfaces;
} cmBrushModel_t;

typedef struct
{
    S32			startVertex;
    S32			numVertexes;
} cmBrushSurface_t;

typedef struct
{
    idVec3		xyz;
} cmBrushVertex_t;

#endif //!__CM_HEADER_H__
