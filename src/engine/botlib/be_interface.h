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
// File name:   be_interface.h
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#ifndef __BE_INTERFACE_H__
#define __BE_INTERFACE_H__

//#define DEBUG			//debug code
#define RANDOMIZE		//randomize bot behaviour

//FIXME: get rid of this global structure
typedef struct botlib_globals_s
{
    S32 botlibsetup;						//true when the bot library has been setup
    S32 maxentities;						//maximum number of entities
    S32 maxclients;							//maximum number of clients
    F32 time;								//the global time
#ifdef DEBUG
    bool debug;							//true if debug is on
    S32 goalareanum;
    vec3_t goalorigin;
    S32 runai;
#endif
} botlib_globals_t;


extern botlib_globals_t botlibglobals;
extern botlib_import_t botimport;
extern S32 bot_developer;					//true if developer is on

//
S32 Sys_MilliSeconds( void );

#endif //!__BE_INTERFACE_H__
