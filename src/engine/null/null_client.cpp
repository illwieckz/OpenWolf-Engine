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
// File name:   null_client.cpp
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description: all other sound mixing is portable
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#ifdef DEDICATED
#include <null/null_precompiled.h>
#endif

cvar_t*         cl_shownet;

// TTimo: win32 dedicated
cvar_t*         cl_language;

void CL_Shutdown( void )
{
}

void CL_Init( void )
{
    cl_shownet = Cvar_Get( "cl_shownet", "0", CVAR_TEMP );
    // TTimo: localisation, prolly not any use in dedicated / null client
    cl_language = Cvar_Get( "cl_language", "0", CVAR_ARCHIVE );
}

void CL_MouseEvent( int dx, int dy, int time )
{
}

void Key_WriteBindings( fileHandle_t f )
{
}

void CL_Frame( int msec )
{
}

void CL_PacketEvent( netadr_t from, msg_t* msg )
{
}

void CL_CharEvent( int key )
{
}

void CL_Disconnect( bool showMainMenu )
{
}

void CL_MapLoading( void )
{
}

bool CL_GameCommand( void )
{
    return false;				// bk001204 - non-void
}

void CL_KeyEvent( int key, int down, U32 time )
{
}

bool UI_GameCommand( void )
{
    return false;
}

void CL_ForwardCommandToServer( StringEntry string )
{
}

void CL_ConsolePrint( char* txt )
{
}

void CL_JoystickEvent( int axis, int value, int time )
{
}

void CL_InitKeyCommands( void )
{
}

void CL_CDDialog( void )
{
}

void CL_FlushMemory( void )
{
}

void CL_StartHunkUsers( bool rendererOnly )
{
}

// bk001119 - added new dummy for sv_init.c
void CL_ShutdownAll( void )
{
};

// bk001208 - added new dummy (RC4)
bool CL_CDKeyValidate( StringEntry key, StringEntry checksum )
{
    return true;
}

// TTimo added for win32 dedicated
void Key_ClearStates( void )
{
}

// Dushan
StringEntry Con_GetText( int console )
{
    return NULL;
}

