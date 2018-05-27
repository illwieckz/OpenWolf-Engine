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
// File name:   be_ea.h
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#ifndef __BG_EA_H__
#define __BG_EA_H__

//ClientCommand elementary actions
void EA_Say( S32 client, UTF8* str );
void EA_SayTeam( S32 client, UTF8* str );
void EA_Command( S32 client, UTF8* command );

void EA_Action( S32 client, S32 action );
void EA_Crouch( S32 client );
void EA_Walk( S32 client );
void EA_MoveUp( S32 client );
void EA_MoveDown( S32 client );
void EA_MoveForward( S32 client );
void EA_MoveBack( S32 client );
void EA_MoveLeft( S32 client );
void EA_MoveRight( S32 client );
void EA_Attack( S32 client );
void EA_Respawn( S32 client );
void EA_Talk( S32 client );
void EA_Gesture( S32 client );
void EA_Use( S32 client );

//regular elementary actions
void EA_SelectWeapon( S32 client, S32 weapon );
void EA_Jump( S32 client );
void EA_DelayedJump( S32 client );
void EA_Move( S32 client, vec3_t dir, F32 speed );
void EA_View( S32 client, vec3_t viewangles );

//send regular input to the server
void EA_EndRegular( S32 client, F32 thinktime );
void EA_GetInput( S32 client, F32 thinktime, bot_input_t* input );
void EA_ResetInput( S32 client );
//setup and shutdown routines
S32 EA_Setup( void );
void EA_Shutdown( void );

#endif //!__BG_EA_H__
