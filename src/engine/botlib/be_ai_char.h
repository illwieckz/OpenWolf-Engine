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
// File name:   be_ai_char.h
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#ifndef __BE_AI_CHAR_H__
#define __BE_AI_CHAR_H__

//loads a bot character from a file
S32 BotLoadCharacter( UTF8* charfile, F32 skill );
//frees a bot character
void BotFreeCharacter( S32 character );
//returns a F32 characteristic
F32 Characteristic_Float( S32 character, S32 index );
//returns a bounded F32 characteristic
F32 Characteristic_BFloat( S32 character, S32 index, F32 min, F32 max );
//returns an integer characteristic
S32 Characteristic_Integer( S32 character, S32 index );
//returns a bounded integer characteristic
S32 Characteristic_BInteger( S32 character, S32 index, S32 min, S32 max );
//returns a string characteristic
void Characteristic_String( S32 character, S32 index, UTF8* buf, S32 size );
//free cached bot characters
void BotShutdownCharacters( void );

#endif //!__BE_AI_CHAR_H__
