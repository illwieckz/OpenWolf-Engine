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
// File name:   be_aas_bsp.h
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#ifndef __BE_AAS_BSP_H__
#define __BE_AAS_BSP_H__

#ifdef AASINTERN

//loads the given BSP file
S32 AAS_LoadBSPFile( void );
//dump the loaded BSP data
void AAS_DumpBSPData( void );
//unlink the given entity from the bsp tree leaves
void AAS_UnlinkFromBSPLeaves( bsp_link_t* leaves );
//link the given entity to the bsp tree leaves of the given model
bsp_link_t* AAS_BSPLinkEntity( vec3_t absmins, vec3_t absmaxs, S32 entnum, S32 modelnum );

//calculates collision with given entity
bool AAS_EntityCollision( S32 entnum, vec3_t start, vec3_t boxmins, vec3_t boxmaxs, vec3_t end, S32 contentmask, bsp_trace_t* trace );
//for debugging
void AAS_PrintFreeBSPLinks( UTF8* str );
//
#endif //AASINTERN

#define MAX_EPAIRKEY		128

//trace through the world
bsp_trace_t AAS_Trace(	vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, S32 passent, S32 contentmask );
//returns the contents at the given point
S32 AAS_PointContents( vec3_t point );
//returns true when p2 is in the PVS of p1
bool AAS_inPVS( vec3_t p1, vec3_t p2 );
//returns true when p2 is in the PHS of p1
bool AAS_inPHS( vec3_t p1, vec3_t p2 );
//returns true if the given areas are connected
bool AAS_AreasConnected( S32 area1, S32 area2 );
//creates a list with entities totally or partly within the given box
S32 AAS_BoxEntities( vec3_t absmins, vec3_t absmaxs, S32* list, S32 maxcount );
//gets the mins, maxs and origin of a BSP model
void AAS_BSPModelMinsMaxsOrigin( S32 modelnum, vec3_t angles, vec3_t mins, vec3_t maxs, vec3_t origin );
//handle to the next bsp entity
S32 AAS_NextBSPEntity( S32 ent );
//return the value of the BSP epair key
S32 AAS_ValueForBSPEpairKey( S32 ent, UTF8* key, UTF8* value, S32 size );
//get a vector for the BSP epair key
S32 AAS_VectorForBSPEpairKey( S32 ent, UTF8* key, vec3_t v );
//get a F32 for the BSP epair key
S32 AAS_FloatForBSPEpairKey( S32 ent, UTF8* key, F32* value );
//get an integer for the BSP epair key
S32 AAS_IntForBSPEpairKey( S32 ent, UTF8* key, S32* value );

#endif //!__BE_AAS_BSP_H__
