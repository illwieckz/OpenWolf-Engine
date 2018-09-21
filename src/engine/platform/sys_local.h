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
// File name:   sys_loadlib.h
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#ifndef __Q_SHARED_H__
#include <qcommon/q_shared.h>
#endif
#ifndef __QCOMMON_H__
#include <qcommon/qcommon.h>
#endif

// Require a minimum version of SDL
#define MINSDL_MAJOR 1
#define MINSDL_MINOR 2
#define MINSDL_PATCH 10

// Input subsystem
void IN_Init( void );
void IN_Frame( void );
void IN_Shutdown( void );
void IN_Restart( void );

// Console
void CON_Shutdown( void );
void CON_Init( void );
UTF8* CON_Input( void );
void CON_Print( StringEntry message );
StringEntry Con_GetText( S32 console );

U32 CON_LogSize( void );
U32 CON_LogWrite( StringEntry in );
U32 CON_LogRead( UTF8* out, U32 outSize );

#ifdef MACOS_X
UTF8* Sys_StripAppBundle( UTF8* pwd );
#endif

void Sys_GLimpSafeInit( void );
void Sys_GLimpInit( void );
void Sys_PlatformInit( void );
void Sys_SigHandler( S32 signal );
void Sys_ErrorDialog( StringEntry error );
void Sys_AnsiColorPrint( StringEntry msg );

S32 Sys_PID( void );
bool Sys_PIDIsRunning( S32 pid );

