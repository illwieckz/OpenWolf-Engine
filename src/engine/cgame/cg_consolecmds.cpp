////////////////////////////////////////////////////////////////////////////////////////
// Copyright(C) 1999 - 2005 Id Software, Inc.
// Copyright(C) 2000 - 2006 Tim Angus
// Copyright(C) 2011 - 2018 Dusan Jocic <dusanjocic@msn.com>
//
// This file is part of OpenWolf.
//
// OpenWolf is free software; you can redistribute it
// and / or modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of the License,
// or (at your option) any later version.
//
// OpenWolf is distributed in the hope that it will be
// useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with OpenWolf; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110 - 1301  USA
//
// -------------------------------------------------------------------------------------
// File name:   cg_consolecmds.cpp
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description: text commands typed in at the local console, or
//              executed by a key binding
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#include <cgame/cg_local.h>

/*
=================
SizeUp_f

Keybinding command
=================
*/
void idCGameLocal::SizeUp_f( void )
{
    trap_Cvar_Set( "cg_viewsize", va( "%i", ( S32 )( cg_viewsize.integer + 10 ) ) );
}


/*
=================
SizeDown_f

Keybinding command
=================
*/
void idCGameLocal::SizeDown_f( void )
{
    trap_Cvar_Set( "cg_viewsize", va( "%i", ( S32 )( cg_viewsize.integer - 10 ) ) );
}


/*
=============
Viewpos_f

Debugging command to print the current position
=============
*/
void idCGameLocal::Viewpos_f( void )
{
    Printf( "(%i %i %i) : %i\n", ( S32 )cg.refdef.vieworg[ 0 ],
            ( S32 )cg.refdef.vieworg[ 1 ], ( S32 )cg.refdef.vieworg[ 2 ],
            ( S32 )cg.refdefViewAngles[ YAW ] );
}

bool idCGameLocal::RequestScores( void )
{
    if( cg.scoresRequestTime + 2000 < cg.time )
    {
        // the scores are more than two seconds out of data,
        // so request new ones
        cg.scoresRequestTime = cg.time;
        trap_SendClientCommand( "score\n" );
        
        return true;
    }
    else
        return false;
}

extern menuDef_t* menuScoreboard;

void idCGameLocal::scrollScoresDown_f( void )
{
    if( menuScoreboard && cg.scoreBoardShowing )
    {
        Menu_ScrollFeeder( menuScoreboard, FEEDER_ALIENTEAM_LIST, true );
        Menu_ScrollFeeder( menuScoreboard, FEEDER_HUMANTEAM_LIST, true );
    }
}


void idCGameLocal::scrollScoresUp_f( void )
{
    if( menuScoreboard && cg.scoreBoardShowing )
    {
        Menu_ScrollFeeder( menuScoreboard, FEEDER_ALIENTEAM_LIST, false );
        Menu_ScrollFeeder( menuScoreboard, FEEDER_HUMANTEAM_LIST, false );
    }
}

void idCGameLocal::ScoresDown_f( void )
{
    if( !cg.showScores )
    {
        Menu_SetFeederSelection( menuScoreboard, FEEDER_ALIENTEAM_LIST, 0, NULL );
        Menu_SetFeederSelection( menuScoreboard, FEEDER_HUMANTEAM_LIST, 0, NULL );
    }
    
    if( RequestScores( ) )
    {
        // leave the current scores up if they were already
        // displayed, but if this is the first hit, clear them out
        if( !cg.showScores )
        {
            if( cg_debugRandom.integer )
                Printf( "ScoresDown_f: scores out of date\n" );
                
            cg.showScores = true;
            cg.numScores = 0;
        }
    }
    else
    {
        // show the cached contents even if they just pressed if it
        // is within two seconds
        cg.showScores = true;
    }
}

void idCGameLocal::ScoresUp_f( void )
{
    if( cg.showScores )
    {
        cg.showScores = false;
        cg.scoreFadeTime = cg.time;
    }
}

void idCGameLocal::TellTarget_f( void )
{
    S32   clientNum;
    UTF8  command[ 128 ];
    UTF8  message[ 128 ];
    
    clientNum = cgameLocal.CrosshairPlayer( );
    if( clientNum == -1 )
        return;
        
    trap_Args( message, 128 );
    Com_sprintf( command, 128, "tell %i %s", clientNum, message );
    trap_SendClientCommand( command );
}

void idCGameLocal::TellAttacker_f( void )
{
    S32   clientNum;
    UTF8  command[ 128 ];
    UTF8  message[ 128 ];
    
    clientNum = cgameLocal.LastAttacker( );
    if( clientNum == -1 )
        return;
        
    trap_Args( message, 128 );
    Com_sprintf( command, 128, "tell %i %s", clientNum, message );
    trap_SendClientCommand( command );
}

void idCGameLocal::SquadMark_f( void )
{
    centity_t* cent;
    vec3_t end;
    trace_t trace;
    
    // Find the player we are looking at
    VectorMA( cg.refdef.vieworg, 131072, cg.refdef.viewaxis[ 0 ], end );
    Trace( &trace, cg.refdef.vieworg, NULL, NULL, end, cg.snap->ps.clientNum, CONTENTS_SOLID | CONTENTS_BODY );
    
    if( trace.entityNum >= MAX_CLIENTS )
        return;
        
    // Only mark teammates
    cent = cg_entities + trace.entityNum;
    if( cent->currentState.eType != ET_PLAYER || cgs.clientinfo[ trace.entityNum ].team != cg.snap->ps.stats[ STAT_TEAM ] )
        return;
        
    cent->pe.squadMarked = !cent->pe.squadMarked;
}

static consoleCommand_t commands[ ] =
{
    { "testgun", cgameLocal.TestGun_f },
    { "testmodel", cgameLocal.TestModel_f },
    { "nextframe", cgameLocal.TestModelNextFrame_f },
    { "prevframe", cgameLocal.TestModelPrevFrame_f },
    { "nextskin", cgameLocal.TestModelNextSkin_f },
    { "prevskin", cgameLocal.TestModelPrevSkin_f },
    { "viewpos", cgameLocal.Viewpos_f },
    { "+scores", cgameLocal.ScoresDown_f },
    { "-scores", cgameLocal.ScoresUp_f },
    { "scoresUp", cgameLocal.scrollScoresUp_f },
    { "scoresDown", cgameLocal.scrollScoresDown_f },
    { "sizeup", cgameLocal.SizeUp_f },
    { "sizedown", cgameLocal.SizeDown_f },
    { "weapnext", cgameLocal.NextWeapon_f },
    { "weapprev", cgameLocal.PrevWeapon_f },
    { "weapon", cgameLocal.Weapon_f },
    { "tell_target", cgameLocal.TellTarget_f },
    { "tell_attacker", cgameLocal.TellAttacker_f },
    { "testPS", cgameLocal.TestPS_f },
    { "destroyTestPS", cgameLocal.DestroyTestPS_f },
    { "testTS", cgameLocal.TestTS_f },
    { "destroyTestTS", cgameLocal.DestroyTestTS_f },
    { "reloadhud", cgameLocal.LoadHudMenu },
    { "squadmark", cgameLocal.SquadMark_f },
};


/*
=================
idCGameLocal::ConsoleCommand

The string has been tokenized and can be retrieved with
Cmd_Argc() / Cmd_Argv()
=================
*/
bool idCGameLocal::ConsoleCommand( void )
{
    StringEntry  cmd;
    S32         i;
    
    cmd = Argv( 0 );
    
    for( i = 0; i < sizeof( commands ) / sizeof( commands[ 0 ] ); i++ )
    {
        if( !Q_stricmp( cmd, commands[ i ].cmd ) )
        {
            commands[ i ].function( );
            return true;
        }
    }
    
    return false;
}


/*
=================
CG_InitConsoleCommands

Let the client system know about all of our commands
so it can perform tab completion
=================
*/
void idCGameLocal::InitConsoleCommands( void )
{
    S32   i;
    
    for( i = 0 ; i < sizeof( commands ) / sizeof( commands[ 0 ] ) ; i++ )
        trap_AddCommand( commands[ i ].cmd );
        
    //
    // the game server will interpret these commands, which will be automatically
    // forwarded to the server after they are not recognized locally
    //
    trap_AddCommand( "kill" );
    trap_AddCommand( "messagemode" );
    trap_AddCommand( "messagemode2" );
    trap_AddCommand( "messagemode3" );
    trap_AddCommand( "messagemode4" );
    trap_AddCommand( "messagemode5" );
    trap_AddCommand( "messagemode6" );
    trap_AddCommand( "prompt" );
    trap_AddCommand( "say" );
    trap_AddCommand( "say_team" );
    trap_AddCommand( "vsay" );
    trap_AddCommand( "vsay_team" );
    trap_AddCommand( "vsay_local" );
    trap_AddCommand( "m" );
    trap_AddCommand( "mt" );
    trap_AddCommand( "tell" );
    trap_AddCommand( "give" );
    trap_AddCommand( "god" );
    trap_AddCommand( "notarget" );
    trap_AddCommand( "noclip" );
    trap_AddCommand( "team" );
    trap_AddCommand( "follow" );
    trap_AddCommand( "levelshot" );
    trap_AddCommand( "setviewpos" );
    trap_AddCommand( "callvote" );
    trap_AddCommand( "vote" );
    trap_AddCommand( "callteamvote" );
    trap_AddCommand( "teamvote" );
    trap_AddCommand( "class" );
    trap_AddCommand( "build" );
    trap_AddCommand( "buy" );
    trap_AddCommand( "sell" );
    trap_AddCommand( "reload" );
    trap_AddCommand( "boost" );
    trap_AddCommand( "itemact" );
    trap_AddCommand( "itemdeact" );
    trap_AddCommand( "itemtoggle" );
    trap_AddCommand( "destroy" );
    trap_AddCommand( "deconstruct" );
    trap_AddCommand( "ignore" );
    trap_AddCommand( "unignore" );
}
/*
=================
idCGameLocal::CompleteCommand

The command has been tokenized and can be retrieved with
Cmd_Argc() / Cmd_Argv()
=================
*/
void idCGameLocal::CompleteCommand( S32 argNum )
{
    StringEntry  cmd;
    S32 i;
    
    cmd = Argv( 0 );
    
    for( i = 0; i < sizeof( commands ) / sizeof( commands[ 0 ] ); i++ )
    {
        if( !Q_stricmp( cmd, commands[ i ].cmd ) )
        {
            commands[ i ].completer( );
            return;
        }
    }
}
