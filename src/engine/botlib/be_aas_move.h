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
// File name:   be_aas_move.h
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#ifndef __BE_AAS_MOVE_H__
#define __BE_AAS_MOVE_H__

#ifdef AASINTERN
extern aas_settings_t aassettings;
#endif //AASINTERN

//movement prediction
S32 AAS_PredictClientMovement( struct aas_clientmove_s* move, S32 entnum, vec3_t origin, S32 presencetype, S32 onground, vec3_t velocity, vec3_t cmdmove, S32 cmdframes, S32 maxframes, F32 frametime, S32 stopevent, S32 stopareanum, S32 visualize );
//predict movement until bounding box is hit
S32 AAS_ClientMovementHitBBox( struct aas_clientmove_s* move, S32 entnum, vec3_t origin, S32 presencetype, S32 onground, vec3_t velocity, vec3_t cmdmove, S32 cmdframes, S32 maxframes, F32 frametime, vec3_t mins, vec3_t maxs, S32 visualize );
//returns true if on the ground at the given origin
S32 AAS_OnGround( vec3_t origin, S32 presencetype, S32 passent );
//returns true if swimming at the given origin
S32 AAS_Swimming( vec3_t origin );
//returns the jump reachability run start point
void AAS_JumpReachRunStart( struct aas_reachability_s* reach, vec3_t runstart );
//returns true if against a ladder at the given origin
S32 AAS_AgainstLadder( vec3_t origin );
//rocket jump Z velocity when rocket-jumping at origin
F32 AAS_RocketJumpZVelocity( vec3_t origin );
//bfg jump Z velocity when bfg-jumping at origin
F32 AAS_BFGJumpZVelocity( vec3_t origin );
//calculates the horizontal velocity needed for a jump and returns true this velocity could be calculated
S32 AAS_HorizontalVelocityForJump( F32 zvel, vec3_t start, vec3_t end, F32* velocity );
//
void AAS_SetMovedir( vec3_t angles, vec3_t movedir );
//
S32 AAS_DropToFloor( vec3_t origin, vec3_t mins, vec3_t maxs );
//
void AAS_InitSettings( void );

#endif //!__BE_AAS_MOVE_H__