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
// File name:   sg_ptr.cpp
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description: post timeout restoration handling
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#include <sgame/sg_precompiled.h>

static connectionRecord_t connections[ MAX_CLIENTS ];

/*
===============
idGameLocal::CheckForUniquePTRC

Callback to detect ptrc clashes
===============
*/
bool idGameLocal::CheckForUniquePTRC( S32 code )
{
    S32 i;
    
    if( code == 0 )
    {
        return false;
    }
    
    for( i = 0; i < MAX_CLIENTS; i++ )
    {
        if( connections[i].ptrCode == code )
        {
            return false;
        }
    }
    
    return true;
}

/*
===============
idGameLocal::UpdatePTRConnection

Update the data in a connection record
===============
*/
void idGameLocal::UpdatePTRConnection( gclient_t* client )
{
    if( client && client->pers.connection )
    {
        client->pers.connection->oldClient = client - level.clients;
        client->pers.connection->clientTeam = client->pers.teamSelection;
        if( client->pers.teamSelection == TEAM_NONE )
        {
            client->pers.connection->clientCredit = client->pers.savedCredit;
        }
        else
        {
            client->pers.connection->clientCredit = client->ps.persistant[PERS_CREDIT];
        }
    }
}

/*
===============
idGameLocal::GenerateNewConnection

Generates a new connection
===============
*/
connectionRecord_t* idGameLocal::GenerateNewConnection( gclient_t* client )
{
    S32     code = 0;
    S32     i;
    
    // this should be really random
    srand( trap_Milliseconds( ) );
    
    // there is a very very small possibility that this
    // will loop infinitely
    do
    {
        code = rand( );
    }
    while( !CheckForUniquePTRC( code ) );
    
    for( i = 0; i < MAX_CLIENTS; i++ )
    {
        //found an unused slot
        if( !connections[ i ].ptrCode )
        {
            connections[ i ].ptrCode = code;
            connections[ i ].clientNum = client->ps.clientNum;
            client->pers.connection = &connections[ i ];
            UpdatePTRConnection( client );
            
            return &connections[ i ];
        }
    }
    
    return NULL;
}

/*
===============
idGameLocal::FindConnectionForCode

Finds a connection for a given code
===============
*/
connectionRecord_t* idGameLocal::FindConnectionForCode( S32 code )
{
    S32 i;
    
    if( code == 0 )
    {
        return NULL;
    }
    
    for( i = 0; i < MAX_CLIENTS; i++ )
    {
        if( connections[i].ptrCode == code )
        {
            return &connections[i];
        }
    }
    
    return NULL;
}

/*
===============
idGameLocal::ResetPTRConnections

Invalidate any existing codes
===============
*/
void idGameLocal::ResetPTRConnections( void )
{
    ::memset( connections, 0, sizeof( connectionRecord_t ) * MAX_CLIENTS );
}
