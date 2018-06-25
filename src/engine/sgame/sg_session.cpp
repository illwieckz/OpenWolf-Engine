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
// File name:   sg_session.cpp
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#include <sgame/sg_precompiled.h>

/*
=======================================================================
SESSION DATA

Session data is the only data that stays persistant across level loads
and tournament restarts.
=======================================================================
*/

/*
================
idGameLocal::WriteClientSessionData

Called on game shutdown
================
*/
void idGameLocal::WriteClientSessionData( gclient_t* client )
{
    StringEntry  s;
    StringEntry  var;
    
    s = va( "%i %i %i %i %s",
            client->sess.spectatorTime,
            client->sess.spectatorState,
            client->sess.spectatorClient,
            client->sess.restartTeam,
            Com_ClientListString( &client->sess.ignoreList )
          );
          
    var = va( "session%i", ( S32 )( client - level.clients ) );
    
    trap_Cvar_Set( var, s );
}

/*
================
idGameLocal::ReadSessionData

Called on a reconnect
================
*/
void idGameLocal::ReadSessionData( gclient_t* client )
{
    UTF8        s[MAX_STRING_CHARS];
    StringEntry  var;
    S32         spectatorState;
    S32         restartTeam;
    UTF8        ignorelist[17];
    
    var = va( "session%i", ( S32 )( client - level.clients ) );
    trap_Cvar_VariableStringBuffer( var, s, sizeof( s ) );
    
    sscanf( s, "%i %i %i %i %16s",
            &client->sess.spectatorTime,
            &spectatorState,
            &client->sess.spectatorClient,
            &restartTeam,
            ignorelist
          );
          
    client->sess.spectatorState = ( spectatorState_t )spectatorState;
    client->sess.restartTeam = ( team_t )restartTeam;
    Com_ClientListParse( &client->sess.ignoreList, ignorelist );
}


/*
================
idGameLocal::InitSessionData

Called on a first-time connect
================
*/
void idGameLocal::InitSessionData( gclient_t* client, UTF8* userinfo )
{
    clientSession_t*  sess;
    StringEntry      value;
    
    sess = &client->sess;
    
    // initial team determination
    value = Info_ValueForKey( userinfo, "team" );
    if( value[0] == 's' )
    {
        // a willing spectator, not a waiting-in-line
        sess->spectatorState = SPECTATOR_FREE;
    }
    else
    {
        if( g_maxGameClients.integer > 0 && level.numNonSpectatorClients >= g_maxGameClients.integer )
        {
            sess->spectatorState = SPECTATOR_FREE;
        }
        else
        {
            sess->spectatorState = SPECTATOR_NOT;
        }
    }
    
    sess->restartTeam = TEAM_NONE;
    sess->spectatorState = SPECTATOR_FREE;
    sess->spectatorTime = level.time;
    sess->spectatorClient = -1;
    ::memset( &sess->ignoreList, 0, sizeof( sess->ignoreList ) );
    
    WriteClientSessionData( client );
}


/*
==================
idGameLocal::WriteSessionData
==================
*/
void idGameLocal::WriteSessionData( void )
{
    S32    i;
    
    for( i = 0 ; i < level.maxclients ; i++ )
    {
        if( level.clients[i].pers.connected == CON_CONNECTED )
        {
            WriteClientSessionData( &level.clients[i] );
        }
    }
    
    // write values for sv_maxclients and sv_democlients because they invalidate session data
    trap_Cvar_Set( "session", va( "%i %i", trap_Cvar_VariableIntegerValue( "sv_maxclients" ), trap_Cvar_VariableIntegerValue( "sv_democlients" ) ) );
}
