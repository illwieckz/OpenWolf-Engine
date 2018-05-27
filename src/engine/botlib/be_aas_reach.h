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
// File name:   be_aas_reach.h
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#ifndef __BE_AAS_REACH_H__
#define __BE_AAS_REACH_H__

#ifdef AASINTERN
//initialize calculating the reachabilities
void AAS_InitReachability( void );
//continue calculating the reachabilities
S32 AAS_ContinueInitReachability( F32 time );
//
S32 AAS_BestReachableLinkArea( aas_link_t* areas );
#endif //AASINTERN

//returns true if the are has reachabilities to other areas
S32 AAS_AreaReachability( S32 areanum );
//returns the best reachable area and goal origin for a bounding box at the given origin
S32 AAS_BestReachableArea( vec3_t origin, vec3_t mins, vec3_t maxs, vec3_t goalorigin );
//returns the best jumppad area from which the bbox at origin is reachable
S32 AAS_BestReachableFromJumpPadArea( vec3_t origin, vec3_t mins, vec3_t maxs );
//returns the next reachability using the given model
S32 AAS_NextModelReachability( S32 num, S32 modelnum );
//returns the total area of the ground faces of the given area
F32 AAS_AreaGroundFaceArea( S32 areanum );
//returns true if the area is crouch only
S32 AAS_AreaCrouch( S32 areanum );
//returns true if a player can swim in this area
S32 AAS_AreaSwim( S32 areanum );
//returns true if the area is filled with a liquid
S32 AAS_AreaLiquid( S32 areanum );
//returns true if the area contains lava
S32 AAS_AreaLava( S32 areanum );
//returns true if the area contains slime
S32 AAS_AreaSlime( S32 areanum );
//returns true if the area has one or more ground faces
S32 AAS_AreaGrounded( S32 areanum );
//returns true if the area has one or more ladder faces
S32 AAS_AreaLadder( S32 areanum );
//returns true if the area is a jump pad
S32 AAS_AreaJumpPad( S32 areanum );
//returns true if the area is donotenter
S32 AAS_AreaDoNotEnter( S32 areanum );

#endif //!__BE_AAS_REACH_H__