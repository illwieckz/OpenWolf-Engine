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
// File name:   be_aas_sample.h
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#ifndef __BE_AAS_SAMPLE_H__
#define __BE_AAS_SAMPLE_H__

#ifdef AASINTERN
void AAS_InitAASLinkHeap( void );
void AAS_InitAASLinkedEntities( void );
void AAS_FreeAASLinkHeap( void );
void AAS_FreeAASLinkedEntities( void );
aas_face_t* AAS_AreaGroundFace( S32 areanum, vec3_t point );
aas_face_t* AAS_TraceEndFace( aas_trace_t* trace );
aas_plane_t* AAS_PlaneFromNum( S32 planenum );
aas_link_t* AAS_AASLinkEntity( vec3_t absmins, vec3_t absmaxs, S32 entnum );
aas_link_t* AAS_LinkEntityClientBBox( vec3_t absmins, vec3_t absmaxs, S32 entnum, S32 presencetype );
bool AAS_PointInsideFace( S32 facenum, vec3_t point, F32 epsilon );
bool AAS_InsideFace( aas_face_t* face, vec3_t pnormal, vec3_t point, F32 epsilon );
void AAS_UnlinkFromAreas( aas_link_t* areas );
#endif //AASINTERN

//returns the mins and maxs of the bounding box for the given presence type
void AAS_PresenceTypeBoundingBox( S32 presencetype, vec3_t mins, vec3_t maxs );
//returns the cluster the area is in (negative portal number if the area is a portal)
S32 AAS_AreaCluster( S32 areanum );
//returns the presence type(s) of the area
S32 AAS_AreaPresenceType( S32 areanum );
//returns the presence type(s) at the given point
S32 AAS_PointPresenceType( vec3_t point );
//returns the result of the trace of a client bbox
aas_trace_t AAS_TraceClientBBox( vec3_t start, vec3_t end, S32 presencetype, S32 passent );
//stores the areas the trace went through and returns the number of passed areas
S32 AAS_TraceAreas( vec3_t start, vec3_t end, S32* areas, vec3_t* points, S32 maxareas );
//returns the areas the bounding box is in
S32 AAS_BBoxAreas( vec3_t absmins, vec3_t absmaxs, S32* areas, S32 maxareas );
//return area information
S32 AAS_AreaInfo( S32 areanum, aas_areainfo_t* info );
//returns the area the point is in
S32 AAS_PointAreaNum( vec3_t point );
//
S32 AAS_PointReachabilityAreaIndex( vec3_t point );
//returns the plane the given face is in
void AAS_FacePlane( S32 facenum, vec3_t normal, F32* dist );

#endif //!__BE_AAS_SAMPLE_H__
