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
// File name:   be_ass_main.h
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#ifndef __BE_AAS_MAIN_H__
#define __BE_AAS_MAIN_H__

#ifndef __BE_AAS_DEF_H__
#include <botlib/be_aas_def.h>
#endif

#ifdef AASINTERN

extern aas_t aasworld;

//AAS error message
void AAS_Error( UTF8* fmt, ... );
//set AAS initialized
void AAS_SetInitialized( void );
//setup AAS with the given number of entities and clients
S32 AAS_Setup( void );
//shutdown AAS
void AAS_Shutdown( void );
//start a new map
S32 AAS_LoadMap( StringEntry mapname );
//start a new time frame
S32 AAS_StartFrame( F32 time );
#endif //AASINTERN

//returns true if AAS is initialized
S32 AAS_Initialized( void );
//returns true if the AAS file is loaded
S32 AAS_Loaded( void );
//returns the model name from the given index
UTF8* AAS_ModelFromIndex( S32 index );
//returns the index from the given model name
S32 AAS_IndexFromModel( UTF8* modelname );
//returns the current time
F32 AAS_Time( void );
//
void AAS_ProjectPointOntoVector( vec3_t point, vec3_t vStart, vec3_t vEnd, vec3_t vProj );

#endif //__BE_AAS_MAIN_H__