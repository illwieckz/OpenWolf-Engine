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
// File name:   be_ass_entity.h
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#ifndef __BE_AAS_ENTITY_H__
#define __BE_AAS_ENTITY_H__

#ifdef AASINTERN
//invalidates all entity infos
void AAS_InvalidateEntities( void );
//unlink not updated entities
void AAS_UnlinkInvalidEntities( void );
//resets the entity AAS and BSP links (sets areas and leaves pointers to NULL)
void AAS_ResetEntityLinks( void );
//updates an entity
S32 AAS_UpdateEntity( S32 ent, bot_entitystate_t* state );
//gives the entity data used for collision detection
void AAS_EntityBSPData( S32 entnum, bsp_entdata_t* entdata );
#endif //AASINTERN

//returns the size of the entity bounding box in mins and maxs
void AAS_EntitySize( S32 entnum, vec3_t mins, vec3_t maxs );
//returns the BSP model number of the entity
S32 AAS_EntityModelNum( S32 entnum );
//returns the origin of an entity with the given model number
S32 AAS_OriginOfMoverWithModelNum( S32 modelnum, vec3_t origin );
//returns the best reachable area the entity is situated in
S32 AAS_BestReachableEntityArea( S32 entnum );
//returns the info of the given entity
void AAS_EntityInfo( S32 entnum, aas_entityinfo_t* info );
//returns the next entity
S32 AAS_NextEntity( S32 entnum );
//returns the origin of the entity
void AAS_EntityOrigin( S32 entnum, vec3_t origin );
//returns the entity type
S32 AAS_EntityType( S32 entnum );
//returns the model index of the entity
S32 AAS_EntityModelindex( S32 entnum );

#endif //!__BE_AAS_ENTITY_H__
