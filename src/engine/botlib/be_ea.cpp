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
// File name:   be_ea.cpp
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description: elementary actions
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#include <OWLib/precompiled.h>

#define MAX_USERMOVE				400
#define MAX_COMMANDARGUMENTS		10
#define ACTION_JUMPEDLASTFRAME		128

bot_input_t* botinputs;

//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void EA_Say( S32 client, UTF8* str )
{
    botimport.BotClientCommand( client, va( "say %s", str ) );
} //end of the function EA_Say
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void EA_SayTeam( S32 client, UTF8* str )
{
    botimport.BotClientCommand( client, va( "say_team %s", str ) );
} //end of the function EA_SayTeam
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void EA_Tell( S32 client, S32 clientto, UTF8* str )
{
    botimport.BotClientCommand( client, va( "tell %d, %s", clientto, str ) );
} //end of the function EA_SayTeam
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void EA_UseItem( S32 client, S32* it )
{
    botimport.BotClientCommand( client, va( "use %s", it ) );
} //end of the function EA_UseItem
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void EA_DropItem( S32 client, UTF8* it )
{
    botimport.BotClientCommand( client, va( "drop %s", it ) );
} //end of the function EA_DropItem
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void EA_UseInv( S32 client, UTF8* inv )
{
    botimport.BotClientCommand( client, va( "invuse %s", inv ) );
} //end of the function EA_UseInv
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void EA_DropInv( S32 client, UTF8* inv )
{
    botimport.BotClientCommand( client, va( "invdrop %s", inv ) );
} //end of the function EA_DropInv
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void EA_Gesture( S32 client )
{
    bot_input_t* bi;
    
    bi = &botinputs[client];
    
    bi->actionflags |= ACTION_GESTURE;
} //end of the function EA_Gesture
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void EA_Command( S32 client, UTF8* command )
{
    botimport.BotClientCommand( client, command );
} //end of the function EA_Command
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void EA_SelectWeapon( S32 client, S32 weapon )
{
    bot_input_t* bi;
    
    bi = &botinputs[client];
    
    bi->weapon = weapon;
} //end of the function EA_SelectWeapon
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void EA_Attack( S32 client )
{
    bot_input_t* bi;
    
    bi = &botinputs[client];
    
    bi->actionflags |= ACTION_ATTACK;
} //end of the function EA_Attack
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void EA_Talk( S32 client )
{
    bot_input_t* bi;
    
    bi = &botinputs[client];
    
    bi->actionflags |= ACTION_TALK;
} //end of the function EA_Talk
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void EA_Use( S32 client )
{
    bot_input_t* bi;
    
    bi = &botinputs[client];
    
    bi->actionflags |= ACTION_USE;
} //end of the function EA_Use
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void EA_Respawn( S32 client )
{
    bot_input_t* bi;
    
    bi = &botinputs[client];
    
    bi->actionflags |= ACTION_RESPAWN;
} //end of the function EA_Respawn
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void EA_Jump( S32 client )
{
    bot_input_t* bi;
    
    bi = &botinputs[client];
    
    if( bi->actionflags & ACTION_JUMPEDLASTFRAME )
    {
        bi->actionflags &= ~ACTION_JUMP;
    } //end if
    else
    {
        bi->actionflags |= ACTION_JUMP;
    } //end if
} //end of the function EA_Jump
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void EA_DelayedJump( S32 client )
{
    bot_input_t* bi;
    
    bi = &botinputs[client];
    
    if( bi->actionflags & ACTION_JUMPEDLASTFRAME )
    {
        bi->actionflags &= ~ACTION_DELAYEDJUMP;
    } //end if
    else
    {
        bi->actionflags |= ACTION_DELAYEDJUMP;
    } //end if
} //end of the function EA_DelayedJump
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void EA_Crouch( S32 client )
{
    bot_input_t* bi;
    
    bi = &botinputs[client];
    
    bi->actionflags |= ACTION_CROUCH;
} //end of the function EA_Crouch
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void EA_Walk( S32 client )
{
    bot_input_t* bi;
    
    bi = &botinputs[client];
    
    bi->actionflags |= ACTION_WALK;
} //end of the function EA_Walk
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void EA_Action( S32 client, S32 action )
{
    bot_input_t* bi;
    
    bi = &botinputs[client];
    
    bi->actionflags |= action;
} //end of function EA_Action
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void EA_MoveUp( S32 client )
{
    bot_input_t* bi;
    
    bi = &botinputs[client];
    
    bi->actionflags |= ACTION_MOVEUP;
} //end of the function EA_MoveUp
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void EA_MoveDown( S32 client )
{
    bot_input_t* bi;
    
    bi = &botinputs[client];
    
    bi->actionflags |= ACTION_MOVEDOWN;
} //end of the function EA_MoveDown
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void EA_MoveForward( S32 client )
{
    bot_input_t* bi;
    
    bi = &botinputs[client];
    
    bi->actionflags |= ACTION_MOVEFORWARD;
} //end of the function EA_MoveForward
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void EA_MoveBack( S32 client )
{
    bot_input_t* bi;
    
    bi = &botinputs[client];
    
    bi->actionflags |= ACTION_MOVEBACK;
} //end of the function EA_MoveBack
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void EA_MoveLeft( S32 client )
{
    bot_input_t* bi;
    
    bi = &botinputs[client];
    
    bi->actionflags |= ACTION_MOVELEFT;
} //end of the function EA_MoveLeft
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void EA_MoveRight( S32 client )
{
    bot_input_t* bi;
    
    bi = &botinputs[client];
    
    bi->actionflags |= ACTION_MOVERIGHT;
} //end of the function EA_MoveRight
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void EA_Move( S32 client, vec3_t dir, F32 speed )
{
    bot_input_t* bi;
    
    bi = &botinputs[client];
    
    VectorCopy( dir, bi->dir );
    //cap speed
    if( speed > MAX_USERMOVE ) speed = MAX_USERMOVE;
    else if( speed < -MAX_USERMOVE ) speed = -MAX_USERMOVE;
    bi->speed = speed;
} //end of the function EA_Move
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void EA_View( S32 client, vec3_t viewangles )
{
    bot_input_t* bi;
    
    bi = &botinputs[client];
    
    VectorCopy( viewangles, bi->viewangles );
} //end of the function EA_View
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void EA_EndRegular( S32 client, F32 thinktime )
{
    /*
    	bot_input_t *bi;
    	S32 jumped = false;
    
    	bi = &botinputs[client];
    
    	bi->actionflags &= ~ACTION_JUMPEDLASTFRAME;
    
    	bi->thinktime = thinktime;
    	botimport.BotInput(client, bi);
    
    	bi->thinktime = 0;
    	VectorClear(bi->dir);
    	bi->speed = 0;
    	jumped = bi->actionflags & ACTION_JUMP;
    	bi->actionflags = 0;
    	if (jumped) bi->actionflags |= ACTION_JUMPEDLASTFRAME;
    */
} //end of the function EA_EndRegular
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void EA_GetInput( S32 client, F32 thinktime, bot_input_t* input )
{
    bot_input_t* bi;
//	S32 jumped = false;

    bi = &botinputs[client];
    
//	bi->actionflags &= ~ACTION_JUMPEDLASTFRAME;

    bi->thinktime = thinktime;
    ::memcpy( input, bi, sizeof( bot_input_t ) );
    
    /*
    bi->thinktime = 0;
    VectorClear(bi->dir);
    bi->speed = 0;
    jumped = bi->actionflags & ACTION_JUMP;
    bi->actionflags = 0;
    if (jumped) bi->actionflags |= ACTION_JUMPEDLASTFRAME;
    */
} //end of the function EA_GetInput
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void EA_ResetInput( S32 client )
{
    bot_input_t* bi;
    S32 jumped = false;
    
    bi = &botinputs[client];
    bi->actionflags &= ~ACTION_JUMPEDLASTFRAME;
    
    bi->thinktime = 0;
    VectorClear( bi->dir );
    bi->speed = 0;
    jumped = bi->actionflags & ACTION_JUMP;
    bi->actionflags = 0;
    if( jumped ) bi->actionflags |= ACTION_JUMPEDLASTFRAME;
} //end of the function EA_ResetInput
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
S32 EA_Setup( void )
{
    //initialize the bot inputs
    botinputs = ( bot_input_t* ) GetClearedHunkMemory(
                    botlibglobals.maxclients * sizeof( bot_input_t ) );
    return BLERR_NOERROR;
} //end of the function EA_Setup
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void EA_Shutdown( void )
{
    FreeMemory( botinputs );
    botinputs = NULL;
} //end of the function EA_Shutdown
