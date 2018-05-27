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
// File name:   g_maprotation.cpp
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description: the map rotation system
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#include <game/g_local.h>

static mapRotations_t mapRotations;

/*
===============
idGameLocal::MapExists

Check if a map exists
===============
*/
bool idGameLocal::MapExists( UTF8* name )
{
    return trap_FS_FOpenFile( va( "maps/%s.bsp", name ), NULL, FS_READ );
}

/*
===============
idGameLocal::RotationExists

Check if a rotation exists
===============
*/
bool idGameLocal::RotationExists( UTF8* name )
{
    S32 i;
    
    for( i = 0; i < mapRotations.numRotations; i++ )
    {
        if( Q_strncmp( mapRotations.rotations[ i ].name, name, MAX_QPATH ) == 0 )
            return true;
    }
    
    return false;
}

/*
===============
idGameLocal::ParseCommandSection

Parse a map rotation command section
===============
*/
bool idGameLocal::ParseMapCommandSection( mapRotationEntry_t* mre, UTF8** text_p )
{
    UTF8* token;
    
    // read optional parameters
    while( 1 )
    {
        token = COM_Parse( text_p );
        
        if( !token )
            break;
            
        if( !Q_stricmp( token, "" ) )
            return false;
            
        if( !Q_stricmp( token, "}" ) )
            return true; //reached the end of this command section
            
        if( !Q_stricmp( token, "layouts" ) )
        {
            token = COM_ParseExt( text_p, false );
            mre->layouts[ 0 ] = '\0';
            while( token && token[ 0 ] != 0 )
            {
                Q_strcat( mre->layouts, sizeof( mre->layouts ), token );
                Q_strcat( mre->layouts, sizeof( mre->layouts ), " " );
                token = COM_ParseExt( text_p, false );
            }
            continue;
        }
        
        if( mre->numCmds == MAX_MAP_COMMANDS )
        {
            Printf( S_COLOR_RED "ERROR: maximum number of map commands (%d) reached\n", MAX_MAP_COMMANDS );
            return false;
        }
        
        Q_strncpyz( mre->postCmds[ mre->numCmds ], token, sizeof( mre->postCmds[ 0 ] ) );
        Q_strcat( mre->postCmds[ mre->numCmds ], sizeof( mre->postCmds[ 0 ] ), " " );
        
        token = COM_ParseExt( text_p, false );
        
        while( token && token[ 0 ] != 0 )
        {
            Q_strcat( mre->postCmds[ mre->numCmds ], sizeof( mre->postCmds[ 0 ] ), token );
            Q_strcat( mre->postCmds[ mre->numCmds ], sizeof( mre->postCmds[ 0 ] ), " " );
            token = COM_ParseExt( text_p, false );
        }
        mre->numCmds++;
    }
    
    return false;
}

/*
===============
G_ParseMapRotation

Parse a map rotation section
===============
*/
bool idGameLocal::ParseMapRotation( mapRotation_t* mr, UTF8** text_p )
{
    UTF8* token;
    bool mnSet = false;
    mapRotationEntry_t* mre = NULL;
    mapRotationCondition_t*  mrc;
    
    // read optional parameters
    while( 1 )
    {
        token = COM_Parse( text_p );
        
        if( !token )
            break;
            
        if( !Q_stricmp( token, "" ) )
            return false;
            
        if( !Q_stricmp( token, "{" ) )
        {
            if( !mnSet )
            {
                Printf( S_COLOR_RED "ERROR: map settings section with no name\n" );
                return false;
            }
            
            if( !ParseMapCommandSection( mre, text_p ) )
            {
                Printf( S_COLOR_RED "ERROR: failed to parse map command section\n" );
                return false;
            }
            
            mnSet = false;
            continue;
        }
        else if( !Q_stricmp( token, "goto" ) )
        {
            token = COM_Parse( text_p );
            
            if( !token )
                break;
                
            if( mre->numConditions == MAX_MAP_ROTATION_CONDS )
            {
                Printf( S_COLOR_RED "ERROR: maximum number of conditions for one map (%d) reached\n", MAX_MAP_ROTATION_CONDS );
                return false;
            }
            
            mrc = &mre->conditions[ mre->numConditions++ ];
            mrc->unconditional = true;
            Q_strncpyz( mrc->dest, token, sizeof( mrc->dest ) );
            
            continue;
        }
        else if( !Q_stricmp( token, "if" ) )
        {
            token = COM_Parse( text_p );
            
            if( !token )
                break;
                
            if( mre->numConditions == MAX_MAP_ROTATION_CONDS )
            {
                Printf( S_COLOR_RED "ERROR: maximum number of conditions for one map (%d) reached\n", MAX_MAP_ROTATION_CONDS );
                return false;
            }
            
            mrc = &mre->conditions[ mre->numConditions++ ];
            
            if( !Q_stricmp( token, "numClients" ) )
            {
                mrc->lhs = MCV_NUMCLIENTS;
                
                token = COM_Parse( text_p );
                
                if( !token )
                    break;
                    
                if( !Q_stricmp( token, "<" ) )
                    mrc->op = MCO_LT;
                else if( !Q_stricmp( token, ">" ) )
                    mrc->op = MCO_GT;
                else if( !Q_stricmp( token, "=" ) )
                    mrc->op = MCO_EQ;
                else
                {
                    Printf( S_COLOR_RED "ERROR: invalid operator in expression: %s\n", token );
                    return false;
                }
                
                token = COM_Parse( text_p );
                
                if( !token )
                    break;
                    
                mrc->numClients = atoi( token );
            }
            else if( !Q_stricmp( token, "lastWin" ) )
            {
                mrc->lhs = MCV_LASTWIN;
                
                token = COM_Parse( text_p );
                
                if( !token )
                    break;
                    
                if( !Q_stricmp( token, "aliens" ) )
                    mrc->lastWin = TEAM_ALIENS;
                else if( !Q_stricmp( token, "humans" ) )
                    mrc->lastWin = TEAM_HUMANS;
                else
                {
                    Printf( S_COLOR_RED "ERROR: invalid right hand side in expression: %s\n", token );
                    return false;
                }
            }
            else if( !Q_stricmp( token, "random" ) )
                mrc->lhs = MCV_RANDOM;
            else
            {
                Printf( S_COLOR_RED "ERROR: invalid left hand side in expression: %s\n", token );
                return false;
            }
            
            token = COM_Parse( text_p );
            
            if( !token )
                break;
                
            mrc->unconditional = false;
            Q_strncpyz( mrc->dest, token, sizeof( mrc->dest ) );
            
            continue;
        }
        else if( !Q_stricmp( token, "}" ) )
            return true; //reached the end of this map rotation
            
        if( mr->numMaps == MAX_MAP_ROTATION_MAPS )
        {
            Printf( S_COLOR_RED "ERROR: maximum number of maps in one rotation (%d) reached\n", MAX_MAP_ROTATION_MAPS );
            return false;
        }
        
        mre = &mr->maps[ mr->numMaps++ ];
        
        Q_strncpyz( mre->name, token, sizeof( mre->name ) );
        mnSet = true;
    }
    
    return false;
}

/*
===============
idGameLocal::ParseMapRotationFile

Load the map rotations from a map rotation file
===============
*/
bool idGameLocal::ParseMapRotationFile( StringEntry fileName )
{
    UTF8* text_p;
    S32 i, j, k, len;
    UTF8* token;
    UTF8 text[ 20000 ];
    UTF8 mrName[ MAX_QPATH ];
    bool mrNameSet = false;
    fileHandle_t  f;
    
    // load the file
    len = trap_FS_FOpenFile( fileName, &f, FS_READ );
    if( len < 0 )
        return false;
        
    if( len == 0 || len >= sizeof( text ) - 1 )
    {
        trap_FS_FCloseFile( f );
        Printf( S_COLOR_RED "ERROR: map rotation file %s is %s\n", fileName, len == 0 ? "empty" : "too long" );
        return false;
    }
    
    trap_FS_Read( text, len, f );
    text[ len ] = 0;
    trap_FS_FCloseFile( f );
    
    // parse the text
    text_p = text;
    
    // read optional parameters
    while( 1 )
    {
        token = COM_Parse( &text_p );
        
        if( !token )
            break;
            
        if( !Q_stricmp( token, "" ) )
            break;
            
        if( !Q_stricmp( token, "{" ) )
        {
            if( mrNameSet )
            {
                //check for name space clashes
                for( i = 0; i < mapRotations.numRotations; i++ )
                {
                    if( !Q_stricmp( mapRotations.rotations[ i ].name, mrName ) )
                    {
                        Printf( S_COLOR_RED "ERROR: a map rotation is already named %s\n", mrName );
                        return false;
                    }
                }
                
                if( mapRotations.numRotations == MAX_MAP_ROTATIONS )
                {
                    Printf( S_COLOR_RED "ERROR: maximum number of map rotations (%d) reached\n",
                            MAX_MAP_ROTATIONS );
                    return false;
                }
                
                Q_strncpyz( mapRotations.rotations[ mapRotations.numRotations ].name, mrName, MAX_QPATH );
                
                if( !ParseMapRotation( &mapRotations.rotations[ mapRotations.numRotations ], &text_p ) )
                {
                    Printf( S_COLOR_RED "ERROR: %s: failed to parse map rotation %s\n", fileName, mrName );
                    return false;
                }
                
                mapRotations.numRotations++;
                
                //start parsing map rotations again
                mrNameSet = false;
                
                continue;
            }
            else
            {
                Printf( S_COLOR_RED "ERROR: unnamed map rotation\n" );
                return false;
            }
        }
        
        if( !mrNameSet )
        {
            Q_strncpyz( mrName, token, sizeof( mrName ) );
            mrNameSet = true;
        }
        else
        {
            Printf( S_COLOR_RED "ERROR: map rotation already named\n" );
            return false;
        }
    }
    
    for( i = 0; i < mapRotations.numRotations; i++ )
    {
        for( j = 0; j < mapRotations.rotations[ i ].numMaps; j++ )
        {
            if( !MapExists( mapRotations.rotations[ i ].maps[ j ].name ) )
            {
                Printf( S_COLOR_RED "ERROR: map \"%s\" doesn't exist\n",
                        mapRotations.rotations[ i ].maps[ j ].name );
                return false;
            }
            
            for( k = 0; k < mapRotations.rotations[ i ].maps[ j ].numConditions; k++ )
            {
                if( !MapExists( mapRotations.rotations[ i ].maps[ j ].conditions[ k ].dest ) &&
                        !RotationExists( mapRotations.rotations[ i ].maps[ j ].conditions[ k ].dest ) )
                {
                    Printf( S_COLOR_RED "ERROR: conditional destination \"%s\" doesn't exist\n", mapRotations.rotations[ i ].maps[ j ].conditions[ k ].dest );
                    return false;
                }
                
            }
            
        }
    }
    
    return true;
}

/*
===============
idGameLocal::PrintRotations

Print the parsed map rotations
===============
*/
void idGameLocal::PrintRotations( void )
{
    S32 i, j, k;
    
    Printf( "Map rotations as parsed:\n\n" );
    
    for( i = 0; i < mapRotations.numRotations; i++ )
    {
        Printf( "rotation: %s\n{\n", mapRotations.rotations[ i ].name );
        
        for( j = 0; j < mapRotations.rotations[ i ].numMaps; j++ )
        {
            Printf( "  map: %s\n  {\n", mapRotations.rotations[ i ].maps[ j ].name );
            
            for( k = 0; k < mapRotations.rotations[ i ].maps[ j ].numCmds; k++ )
            {
                Printf( "    command: %s\n",
                        mapRotations.rotations[ i ].maps[ j ].postCmds[ k ] );
            }
            
            Printf( "  }\n" );
            
            for( k = 0; k < mapRotations.rotations[ i ].maps[ j ].numConditions; k++ )
            {
                Printf( "  conditional: %s\n",
                        mapRotations.rotations[ i ].maps[ j ].conditions[ k ].dest );
            }
            
        }
        
        Printf( "}\n" );
    }
    
    Printf( "Total memory used: %d bytes\n", sizeof( mapRotations ) );
}

/*
===============
idGameLocal::GetCurrentMapArray

Fill a static array with the current map of each rotation
===============
*/
S32* idGameLocal::GetCurrentMapArray( void )
{
    static S32  currentMap[ MAX_MAP_ROTATIONS ];
    S32         i = 0;
    UTF8        text[ MAX_MAP_ROTATIONS * 2 ];
    UTF8*        text_p, *token;
    
    Q_strncpyz( text, g_currentMap.string, sizeof( text ) );
    
    text_p = text;
    
    while( 1 )
    {
        token = COM_Parse( &text_p );
        
        if( !token )
            break;
            
        if( !Q_stricmp( token, "" ) )
            break;
            
        currentMap[ i++ ] = atoi( token );
    }
    
    return currentMap;
}

/*
===============
idGameLocal::SetCurrentMap

Set the current map in some rotation
===============
*/
void idGameLocal::SetCurrentMap( S32 currentMap, S32 rotation )
{
    UTF8  text[ MAX_MAP_ROTATIONS * 2 ] = { 0 };
    S32*   p = GetCurrentMapArray( );
    S32   i;
    
    p[ rotation ] = currentMap;
    
    for( i = 0; i < mapRotations.numRotations; i++ )
        Q_strcat( text, sizeof( text ), va( "%d ", p[ i ] ) );
        
    trap_Cvar_Set( "g_currentMap", text );
    trap_Cvar_Update( &g_currentMap );
}

/*
===============
G_GetCurrentMap

Return the current map in some rotation
===============
*/
S32 idGameLocal::GetCurrentMap( S32 rotation )
{
    S32* p = GetCurrentMapArray( );
    
    return p[ rotation ];
}

/*
===============
idGameLocal::IssueMapChange

Send commands to the server to actually change the map
===============
*/
void idGameLocal::IssueMapChange( S32 rotation )
{
    S32 i;
    S32 map = GetCurrentMap( rotation );
    UTF8 cmd[ MAX_TOKEN_CHARS ];
    
    // allow a manually defined g_layouts setting to override the maprotation
    if( !g_layouts.string[ 0 ] && mapRotations.rotations[ rotation ].maps[ map ].layouts[ 0 ] )
    {
        trap_Cvar_Set( "g_layouts", mapRotations.rotations[ rotation ].maps[ map ].layouts );
    }
    
    trap_SendConsoleCommand( EXEC_APPEND, va( "map %s\n", mapRotations.rotations[ rotation ].maps[ map ].name ) );
    
    // load up map defaults if g_mapConfigs is set
    MapConfigs( mapRotations.rotations[ rotation ].maps[ map ].name );
    
    for( i = 0; i < mapRotations.rotations[ rotation ].maps[ map ].numCmds; i++ )
    {
        Q_strncpyz( cmd, mapRotations.rotations[ rotation ].maps[ map ].postCmds[ i ], sizeof( cmd ) );
        Q_strcat( cmd, sizeof( cmd ), "\n" );
        trap_SendConsoleCommand( EXEC_APPEND, cmd );
    }
}

/*
===============
idGameLocal::ResolveConditionDestination

Resolve the destination of some condition
===============
*/
mapConditionType_t idGameLocal::ResolveConditionDestination( S32* n, UTF8* name )
{
    S32 i;
    
    //search the current rotation first...
    for( i = 0; i < mapRotations.rotations[ g_currentMapRotation.integer ].numMaps; i++ )
    {
        if( !Q_stricmp( mapRotations.rotations[ g_currentMapRotation.integer ].maps[ i ].name, name ) )
        {
            *n = i;
            return MCT_MAP;
        }
    }
    
    //...then search the rotation names
    for( i = 0; i < mapRotations.numRotations; i++ )
    {
        if( !Q_stricmp( mapRotations.rotations[ i ].name, name ) )
        {
            *n = i;
            return MCT_ROTATION;
        }
    }
    
    return MCT_ERR;
}

/*
===============
G_EvaluateMapCondition

Evaluate a map condition
===============
*/
bool idGameLocal::EvaluateMapCondition( mapRotationCondition_t* mrc )
{
    switch( mrc->lhs )
    {
        case MCV_RANDOM:
            return rand( ) & 1;
            break;
            
        case MCV_NUMCLIENTS:
            switch( mrc->op )
            {
                case MCO_LT:
                    return level.numConnectedClients < mrc->numClients;
                    break;
                    
                case MCO_GT:
                    return level.numConnectedClients > mrc->numClients;
                    break;
                    
                case MCO_EQ:
                    return level.numConnectedClients == mrc->numClients;
                    break;
            }
            break;
            
        case MCV_LASTWIN:
            return level.lastWin == mrc->lastWin;
            break;
            
        default:
        case MCV_ERR:
            Printf( S_COLOR_RED "ERROR: malformed map switch condition\n" );
            break;
    }
    
    return false;
}

/*
===============
idGameLocal::AdvanceMapRotation

Increment the current map rotation
===============
*/
void idGameLocal::AdvanceMapRotation( void )
{
    mapRotation_t* mr;
    mapRotationEntry_t* mre;
    mapRotationCondition_t* mrc;
    S32 currentRotation, currentMap, nextMap, i, n;
    mapConditionType_t mct;
    
    if( ( currentRotation = g_currentMapRotation.integer ) == NOT_ROTATING )
        return;
        
    currentMap = GetCurrentMap( currentRotation );
    
    mr = &mapRotations.rotations[ currentRotation ];
    mre = &mr->maps[ currentMap ];
    nextMap = ( currentMap + 1 ) % mr->numMaps;
    
    for( i = 0; i < mre->numConditions; i++ )
    {
        mrc = &mre->conditions[ i ];
        
        if( mrc->unconditional || EvaluateMapCondition( mrc ) )
        {
            mct = ResolveConditionDestination( &n, mrc->dest );
            
            switch( mct )
            {
                case MCT_MAP:
                    nextMap = n;
                    break;
                    
                case MCT_ROTATION:
                    //need to increment the current map before changing the rotation
                    //or you get infinite loops with some conditionals
                    SetCurrentMap( nextMap, currentRotation );
                    StartMapRotation( mrc->dest, true );
                    return;
                    
                default:
                case MCT_ERR:
                    Printf( S_COLOR_YELLOW "WARNING: map switch destination could not be resolved: %s\n", mrc->dest );
                    break;
            }
        }
    }
    
    SetCurrentMap( nextMap, currentRotation );
    IssueMapChange( currentRotation );
}

/*
===============
idGameLocal::StartMapRotation

Switch to a new map rotation
===============
*/
bool idGameLocal::StartMapRotation( UTF8* name, bool changeMap )
{
    S32 i;
    
    for( i = 0; i < mapRotations.numRotations; i++ )
    {
        if( !Q_stricmp( mapRotations.rotations[ i ].name, name ) )
        {
            trap_Cvar_Set( "g_currentMapRotation", va( "%d", i ) );
            trap_Cvar_Update( &g_currentMapRotation );
            
            if( changeMap )
            {
                IssueMapChange( i );
            }
            break;
        }
    }
    
    if( i == mapRotations.numRotations )
    {
        return false;
    }
    else
    {
        return true;
    }
}

/*
===============
idGameLocal::StopMapRotation

Stop the current map rotation
===============
*/
void idGameLocal::StopMapRotation( void )
{
    trap_Cvar_Set( "g_currentMapRotation", va( "%d", NOT_ROTATING ) );
    trap_Cvar_Update( &g_currentMapRotation );
}

/*
===============
idGameLocal::MapRotationActive

Test if any map rotation is currently active
===============
*/
bool idGameLocal::MapRotationActive( void )
{
    return ( g_currentMapRotation.integer != NOT_ROTATING );
}

/*
===============
idGameLocal::InitMapRotations

Load and intialise the map rotations
===============
*/
void idGameLocal::InitMapRotations( void )
{
    StringEntry fileName = "maprotation.cfg";
    
    //load the file if it exists
    if( trap_FS_FOpenFile( fileName, NULL, FS_READ ) )
    {
        if( !ParseMapRotationFile( fileName ) )
        {
            Printf( S_COLOR_RED "ERROR: failed to parse %s file\n", fileName );
        }
    }
    else
    {
        Printf( "%s file not found.\n", fileName );
    }
    
    if( g_currentMapRotation.integer == NOT_ROTATING )
    {
        if( g_initialMapRotation.string[ 0 ] != 0 )
        {
            StartMapRotation( g_initialMapRotation.string, false );
            
            trap_Cvar_Set( "g_initialMapRotation", "" );
            trap_Cvar_Update( &g_initialMapRotation );
        }
    }
}
