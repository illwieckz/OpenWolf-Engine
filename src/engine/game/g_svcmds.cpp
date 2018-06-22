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
// File name:   g_svcmds.cpp
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description: this file holds commands that can be executed by the server
//              console, but not remote clients
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#include <game/sg_precompiled.h>

/*
===================
idGameLocal::EntityList_f
===================
*/
void idGameLocal::EntityList_f( void )
{
    S32       e;
    gentity_t* check;
    
    check = g_entities;
    
    for( e = 0; e < level.num_entities; e++, check++ )
    {
        if( !check->inuse )
            continue;
            
        gameLocal.Printf( "%3i:", e );
        
        switch( check->s.eType )
        {
            case ET_GENERAL:
                gameLocal.Printf( "ET_GENERAL          " );
                break;
            case ET_PLAYER:
                gameLocal.Printf( "ET_PLAYER           " );
                break;
            case ET_ITEM:
                gameLocal.Printf( "ET_ITEM             " );
                break;
            case ET_BUILDABLE:
                gameLocal.Printf( "ET_BUILDABLE        " );
                break;
            case ET_MISSILE:
                gameLocal.Printf( "ET_MISSILE          " );
                break;
            case ET_MOVER:
                gameLocal.Printf( "ET_MOVER            " );
                break;
            case ET_BEAM:
                gameLocal.Printf( "ET_BEAM             " );
                break;
            case ET_PORTAL:
                gameLocal.Printf( "ET_PORTAL           " );
                break;
            case ET_SPEAKER:
                gameLocal.Printf( "ET_SPEAKER          " );
                break;
            case ET_PUSH_TRIGGER:
                gameLocal.Printf( "ET_PUSH_TRIGGER     " );
                break;
            case ET_TELEPORT_TRIGGER:
                gameLocal.Printf( "ET_TELEPORT_TRIGGER " );
                break;
            case ET_INVISIBLE:
                gameLocal.Printf( "ET_INVISIBLE        " );
                break;
            case ET_GRAPPLE:
                gameLocal.Printf( "ET_GRAPPLE          " );
                break;
            case ET_CORPSE:
                gameLocal.Printf( "ET_CORPSE           " );
                break;
            case ET_PARTICLE_SYSTEM:
                gameLocal.Printf( "ET_PARTICLE_SYSTEM  " );
                break;
            case ET_ANIMMAPOBJ:
                gameLocal.Printf( "ET_ANIMMAPOBJ       " );
                break;
            case ET_MODELDOOR:
                gameLocal.Printf( "ET_MODELDOOR        " );
                break;
            case ET_LIGHTFLARE:
                gameLocal.Printf( "ET_LIGHTFLARE       " );
                break;
            case ET_LEV2_ZAP_CHAIN:
                gameLocal.Printf( "ET_LEV2_ZAP_CHAIN   " );
                break;
            default:
                gameLocal.Printf( "%-3i                 ", check->s.eType );
                break;
        }
        
        if( check->classname )
            gameLocal.Printf( "%s", check->classname );
            
        gameLocal.Printf( "\n" );
    }
}

gclient_t* idGameLocal::ClientForString( UTF8* s )
{
    S32 idnum, count;
    S32 pids[ MAX_CLIENTS ];
    
    if( ( count = gameLocal.ClientNumbersFromString( s, pids, MAX_CLIENTS ) ) != 1 )
    {
        idnum = gameLocal.ClientNumberFromString( s );
        
        if( idnum == -1 )
        {
            UTF8 err[ MAX_STRING_CHARS ];
            gameLocal.MatchOnePlayer( pids, count, err, sizeof( err ) );
            gameLocal.Printf( "%s\n", err );
            return NULL;
        }
    }
    else
    {
        idnum = pids[0];
    }
    
    return &level.clients[ idnum ];
}

void idGameLocal::Svcmd_Status_f( void )
{
    S32 i;
    gclient_t* cl;
    UTF8 userinfo[ MAX_INFO_STRING ];
    
    gameLocal.Printf( "slot score ping address               rate     name\n" );
    gameLocal.Printf( "---- ----- ---- -------               ----     ----\n" );
    
    for( i = 0, cl = level.clients; i < level.maxclients; i++, cl++ )
    {
        if( cl->pers.connected == CON_DISCONNECTED )
        {
            continue;
        }
        
        gameLocal.Printf( "%-4d ", i );
        gameLocal.Printf( "%-5d ", cl->ps.persistant[ PERS_SCORE ] );
        
        if( cl->pers.connected == CON_CONNECTING )
        {
            gameLocal.Printf( "CNCT " );
        }
        else
        {
            gameLocal.Printf( "%-4d ", cl->ps.ping );
        }
        
        trap_GetUserinfo( i, userinfo, sizeof( userinfo ) );
        gameLocal.Printf( "%-21s ", Info_ValueForKey( userinfo, "ip" ) );
        gameLocal.Printf( "%-8d ", Info_ValueForKey( userinfo, "rate" ) );
        gameLocal.Printf( "%s\n", cl->pers.netname ); // Info_ValueForKey( userinfo, "name" )
    }
}

/*
===================
Svcmd_ForceTeam_f

forceteam <player> <team>
===================
*/
void idGameLocal::Svcmd_ForceTeam_f( void )
{
    gclient_t* cl;
    UTF8 str[ MAX_TOKEN_CHARS ];
    team_t team;
    
    if( trap_Argc( ) != 3 )
    {
        gameLocal.Printf( "usage: forceteam <player> <team>\n" );
        return;
    }
    
    trap_Argv( 1, str, sizeof( str ) );
    cl = gameLocal.ClientForString( str );
    
    if( !cl )
        return;
        
    trap_Argv( 2, str, sizeof( str ) );
    team = gameLocal.TeamFromString( str );
    if( team == NUM_TEAMS )
    {
        gameLocal.Printf( "forceteam: invalid team \"%s\"\n", str );
        return;
    }
    gameLocal.ChangeTeam( &g_entities[ cl - level.clients ], team );
}

/*
===================
Svcmd_LayoutSave_f

layoutsave <name>
===================
*/
void idGameLocal::Svcmd_LayoutSave_f( void )
{
    UTF8 str[ MAX_QPATH ];
    UTF8 str2[ MAX_QPATH - 4 ];
    UTF8* s;
    S32 i = 0;
    
    if( trap_Argc( ) != 2 )
    {
        gameLocal.Printf( "usage: layoutsave <name>\n" );
        return;
    }
    trap_Argv( 1, str, sizeof( str ) );
    
    // sanitize name
    s = &str[ 0 ];
    while( *s && i < sizeof( str2 ) - 1 )
    {
        if( isalnum( *s ) || *s == '-' || *s == '_' )
        {
            str2[ i++ ] = *s;
            str2[ i ] = '\0';
        }
        s++;
    }
    
    if( !str2[ 0 ] )
    {
        gameLocal.Printf( "layoutsave: invalid name \"%s\"\n", str );
        return;
    }
    
    gameLocal.LayoutSave( str2 );
}

/*
===================
Svcmd_LayoutLoad_f

layoutload [<name> [<name2> [<name3 [...]]]]

This is just a silly alias for doing:
 set g_layouts "name name2 name3"
 map_restart
===================
*/
void idGameLocal::Svcmd_LayoutLoad_f( void )
{
    UTF8 layouts[ MAX_CVAR_VALUE_STRING ];
    UTF8* s;
    
    if( trap_Argc( ) < 2 )
    {
        gameLocal.Printf( "usage: layoutload <name> ...\n" );
        return;
    }
    
    s = gameLocal.ConcatArgs( 1 );
    Q_strncpyz( layouts, s, sizeof( layouts ) );
    trap_Cvar_Set( "g_layouts", layouts );
    trap_SendConsoleCommand( EXEC_APPEND, "map_restart\n" );
    level.restarted = true;
}

void idGameLocal::Svcmd_AdmitDefeat_f( void )
{
    S32  team;
    UTF8 teamNum[ 2 ];
    
    if( trap_Argc( ) != 2 )
    {
        gameLocal.Printf( "admitdefeat: must provide a team\n" );
        return;
    }
    trap_Argv( 1, teamNum, sizeof( teamNum ) );
    team = gameLocal.TeamFromString( teamNum );
    if( team == TEAM_ALIENS )
    {
        gameLocal.TeamCommand( TEAM_ALIENS, "cp \"Hivemind Link Broken\" 1" );
        trap_SendServerCommand( -1, "print \"Alien team has admitted defeat\n\"" );
    }
    else if( team == TEAM_HUMANS )
    {
        gameLocal.TeamCommand( TEAM_HUMANS, "cp \"Life Support Terminated\" 1" );
        trap_SendServerCommand( -1, "print \"Human team has admitted defeat\n\"" );
    }
    else
    {
        gameLocal.Printf( "admitdefeat: invalid team\n" );
        return;
    }
    level.surrenderTeam = ( team_t )team;
    gameLocal.BaseSelfDestruct( ( team_t )team );
}

void idGameLocal::Svcmd_TeamWin_f( void )
{
    UTF8 cmd[ 6 ];
    trap_Argv( 0, cmd, sizeof( cmd ) );
    gameLocal.BaseSelfDestruct( gameLocal.TeamFromString( cmd ) );
}

void idGameLocal::Svcmd_Evacuation_f( void )
{
    trap_SendServerCommand( -1, "print \"Evacuation ordered\n\"" );
    level.lastWin = TEAM_NONE;
    trap_SetConfigstring( CS_WINNER, "Evacuation" );
    gameLocal.LogExit( "Evacuation." );
}

void idGameLocal::Svcmd_MapRotation_f( void )
{
    UTF8 rotationName[ MAX_QPATH ];
    
    if( trap_Argc( ) != 2 )
    {
        gameLocal.Printf( "usage: maprotation <name>\n" );
        return;
    }
    
    trap_Argv( 1, rotationName, sizeof( rotationName ) );
    if( !gameLocal.StartMapRotation( rotationName, false ) )
    {
        gameLocal.Printf( "maprotation: invalid map rotation \"%s\"\n", rotationName );
    }
}

void idGameLocal::Svcmd_TeamMessage_f( void )
{
    UTF8   teamNum[ 2 ];
    team_t team;
    
    if( trap_Argc( ) < 3 )
    {
        gameLocal.Printf( "usage: say_team <team> <message>\n" );
        return;
    }
    
    trap_Argv( 1, teamNum, sizeof( teamNum ) );
    team = gameLocal.TeamFromString( teamNum );
    
    if( team == NUM_TEAMS )
    {
        gameLocal.Printf( "say_team: invalid team \"%s\"\n", teamNum );
        return;
    }
    
    gameLocal.TeamCommand( team, va( "tchat \"console: ^5%s\"", gameLocal.ConcatArgs( 2 ) ) );
}

void idGameLocal::Svcmd_SendMessage( void )
{
    UTF8 cmd[ 5 ];
    trap_Argv( 1, cmd, sizeof( cmd ) );
    
    if( trap_Argc( ) < 2 )
    {
        gameLocal.Printf( "usage: %s <message>\n", cmd );
        return;
    }
    
    trap_SendServerCommand( -1, va( "chat \"console: ^2%s\"", gameLocal.ConcatArgs( 1 ) ) );
}

void idGameLocal::Svcmd_CenterPrint_f( void )
{
    if( trap_Argc( ) < 2 )
    {
        gameLocal.Printf( "usage: cp <message>\n" );
        return;
    }
    
    trap_SendServerCommand( -1, va( "cp \"%s\"", gameLocal.ConcatArgs( 1 ) ) );
}

void idGameLocal::Svcmd_EjectClient_f( void )
{
    UTF8* reason, name[ MAX_STRING_CHARS ];
    
    if( trap_Argc( ) < 2 )
    {
        gameLocal.Printf( "usage: eject <player|-1> <reason>\n" );
        return;
    }
    
    trap_Argv( 1, name, sizeof( name ) );
    reason = gameLocal.ConcatArgs( 2 );
    
    if( atoi( name ) == -1 )
    {
        S32 i;
        for( i = 0; i < level.maxclients; i++ )
        {
            if( level.clients[ i ].pers.connected == CON_DISCONNECTED )
                continue;
            if( level.clients[ i ].pers.localClient )
                continue;
            trap_DropClient( i, reason, 0 );
        }
    }
    else
    {
        gclient_t* cl = gameLocal.ClientForString( name );
        if( !cl )
            return;
            
        if( cl->pers.localClient )
        {
            gameLocal.Printf( "eject: cannot eject local clients\n" );
            return;
        }
        trap_DropClient( cl - level.clients, reason, 0 );
    }
}

void idGameLocal::Svcmd_DumpUser_f( void )
{
    UTF8 name[ MAX_STRING_CHARS ], userinfo[ MAX_INFO_STRING ];
    UTF8 key[ BIG_INFO_KEY ], value[ BIG_INFO_VALUE ];
    StringEntry info;
    gclient_t* cl;
    
    if( trap_Argc( ) != 2 )
    {
        gameLocal.Printf( "usage: dumpuser <player>\n" );
        return;
    }
    
    trap_Argv( 1, name, sizeof( name ) );
    cl = gameLocal.ClientForString( name );
    if( !cl )
        return;
        
    trap_GetUserinfo( cl - level.clients, userinfo, sizeof( userinfo ) );
    info = &userinfo[ 0 ];
    gameLocal.Printf( "userinfo\n--------\n" );
    //Info_Print( userinfo );
    while( 1 )
    {
        Info_NextPair( &info, key, value );
        if( !*info )
            return;
            
        gameLocal.Printf( "%-20s%s\n", key, value );
    }
}

void idGameLocal::Svcmd_PrintQueue_f( void )
{
    UTF8 team[ MAX_STRING_CHARS ];
    if( trap_Argc() != 2 )
    {
        gameLocal.Printf( "usage: printqueue <team>\n" );
        return;
    }
    trap_Argv( 1, team, sizeof( team ) );
    switch( team[0] )
    {
        case 'a':
            gameLocal.PrintSpawnQueue( &level.alienSpawnQueue );
            break;
        case 'h':
            gameLocal.PrintSpawnQueue( &level.humanSpawnQueue );
            break;
        default:
            gameLocal.Printf( "unknown team\n" );
    }
}

/*
===============
Svcmd_BotlibSetVariable_f
===============
*/
void idGameLocal::Svcmd_BotlibSetVariable_f( void )
{
    UTF8 key[MAX_TOKEN_CHARS];
    UTF8 value[MAX_TOKEN_CHARS];
    
    trap_Argv( 1, key, sizeof( key ) );
    if( !key[0] )
    {
        gameLocal.Printf( "Usage: blibset <key> <value> \n" );
        return;
    }
    
    trap_Argv( 2, value, sizeof( value ) );
    if( !value[0] )
    {
        gameLocal.Printf( "Usage: blibset <key> <value> \n" );
        return;
    }
    
    trap_BotLibVarSet( key, va( "%s", value ) );
}

// dumb wrapper for "a" and "m"
void idGameLocal::Svcmd_MessageWrapper( void )
{
    UTF8 cmd[ 2 ];
    trap_Argv( 0, cmd, sizeof( cmd ) );
    if( !Q_stricmp( cmd, "a" ) )
        gameLocal.AdminMessage_f( NULL );
    else
        gameLocal.PrivateMessage_f( NULL );
}

struct
{
    UTF8*     cmd;
    bool dedicated;
    void ( *function )( void );
} svcmds[ ] =
{
    { "entityList", false, gameLocal.EntityList_f },
    { "status", false, gameLocal.Svcmd_Status_f },
    { "forceTeam", false, gameLocal.Svcmd_ForceTeam_f },
    { "mapRotation", false, gameLocal.Svcmd_MapRotation_f },
    { "stopMapRotation", false, gameLocal.StopMapRotation },
    { "advanceMapRotation", false, gameLocal.AdvanceMapRotation },
    { "alienWin", false, gameLocal.Svcmd_TeamWin_f },
    { "humanWin", false, gameLocal.Svcmd_TeamWin_f },
    { "layoutSave", false, gameLocal.Svcmd_LayoutSave_f },
    { "layoutLoad", false, gameLocal.Svcmd_LayoutLoad_f },
    { "eject", false, gameLocal.Svcmd_EjectClient_f },
    { "dumpuser", false, gameLocal.Svcmd_DumpUser_f },
    { "admitDefeat", false, gameLocal.Svcmd_AdmitDefeat_f },
    { "evacuation", false, gameLocal.Svcmd_Evacuation_f },
    { "printqueue", false, gameLocal.Svcmd_PrintQueue_f },
    { "blibset", false, gameLocal.Svcmd_BotlibSetVariable_f },
    
    // don't handle communication commands unless dedicated
    { "say_team", true, gameLocal.Svcmd_TeamMessage_f },
    { "say", true, gameLocal.Svcmd_SendMessage },
    { "chat", true, gameLocal.Svcmd_SendMessage },
    { "cp", true, gameLocal.Svcmd_CenterPrint_f },
    { "m", true, gameLocal.Svcmd_MessageWrapper },
    { "a", true, gameLocal.Svcmd_MessageWrapper }
};

/*
=================
idGameLocal::ConsoleCommand
=================
*/
bool idGameLocal::ConsoleCommand( void )
{
    UTF8 cmd[ MAX_TOKEN_CHARS ];
    S32 i;
    
    trap_Argv( 0, cmd, sizeof( cmd ) );
    
    for( i = 0; i < sizeof( svcmds ) / sizeof( svcmds[ 0 ] ); i++ )
    {
        if( !Q_stricmp( cmd, svcmds[ i ].cmd ) )
        {
            if( svcmds[i].dedicated && !g_dedicated.integer )
            {
                return false;
            }
            
            svcmds[ i ].function( );
            return true;
        }
    }
    
    // see if this is an admin command
    if( adminLocal.AdminCmdCheck( NULL, false ) )
    {
        return true;
    }
    
    if( g_dedicated.integer )
    {
        Printf( "unknown command: %s\n", cmd );
    }
    
    return false;
}

