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
// File name:   cl_ui.cpp
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#include <OWLib/precompiled.h>

extern botlib_export_t* botlib_export;

void*           uivm;

// ydnar: can we put this in a header, pls?
void            Key_GetBindingByString( StringEntry binding, S32* key1, S32* key2 );

/*
====================
GetClientState
====================
*/
static void GetClientState( uiClientState_t* state )
{
    state->connectPacketCount = clc.connectPacketCount;
    state->connState = cls.state;
    Q_strncpyz( state->servername, cls.servername, sizeof( state->servername ) );
    Q_strncpyz( state->updateInfoString, cls.updateInfoString, sizeof( state->updateInfoString ) );
    Q_strncpyz( state->messageString, clc.serverMessage, sizeof( state->messageString ) );
    state->clientNum = cl.snap.ps.clientNum;
}

/*
====================
LAN_LoadCachedServers
====================
*/
void LAN_LoadCachedServers()
{
    S32             size;
    fileHandle_t    fileIn;
    UTF8            filename[MAX_QPATH];
    
    cls.numglobalservers = cls.numfavoriteservers = 0;
    cls.numGlobalServerAddresses = 0;
    
    Q_strncpyz( filename, "servercache.dat", sizeof( filename ) );
    
    // Arnout: moved to mod/profiles dir
    if( FS_SV_FOpenFileRead( filename, &fileIn ) )
    {
        FS_Read( &cls.numglobalservers, sizeof( S32 ), fileIn );
        FS_Read( &cls.numfavoriteservers, sizeof( S32 ), fileIn );
        FS_Read( &size, sizeof( S32 ), fileIn );
        if( size == sizeof( cls.globalServers ) + sizeof( cls.favoriteServers ) )
        {
            FS_Read( &cls.globalServers, sizeof( cls.globalServers ), fileIn );
            FS_Read( &cls.favoriteServers, sizeof( cls.favoriteServers ), fileIn );
        }
        else
        {
            cls.numglobalservers = cls.numfavoriteservers = 0;
            cls.numGlobalServerAddresses = 0;
        }
        FS_FCloseFile( fileIn );
    }
}

/*
====================
LAN_SaveServersToCache
====================
*/
void LAN_SaveServersToCache()
{
    S32             size;
    fileHandle_t    fileOut;
    UTF8            filename[MAX_QPATH];
    
    Q_strncpyz( filename, "servercache.dat", sizeof( filename ) );
    
    // Arnout: moved to mod/profiles dir
    fileOut = FS_SV_FOpenFileWrite( filename );
    //fileOut = FS_FOpenFileWrite( filename );
    FS_Write( &cls.numglobalservers, sizeof( S32 ), fileOut );
    FS_Write( &cls.numfavoriteservers, sizeof( S32 ), fileOut );
    size = sizeof( cls.globalServers ) + sizeof( cls.favoriteServers );
    FS_Write( &size, sizeof( S32 ), fileOut );
    FS_Write( &cls.globalServers, sizeof( cls.globalServers ), fileOut );
    FS_Write( &cls.favoriteServers, sizeof( cls.favoriteServers ), fileOut );
    FS_FCloseFile( fileOut );
}


/*
====================
GetNews
====================
*/
bool GetNews( bool begin )
{
    bool finished = false;
    S32 readSize;
    static UTF8 newsFile[MAX_QPATH] = "";
    
    if( !newsFile[0] )
    {
        Q_strncpyz( newsFile,
                    FS_BuildOSPath( Cvar_VariableString( "fs_homepath" ), "", "news.dat" ),
                    MAX_QPATH );
        newsFile[MAX_QPATH - 1] = 0;
    }
    
    if( begin )   // if not already using curl, start the download
    {
        if( !clc.bWWWDl )
        {
            clc.bWWWDl = true;
            DL_BeginDownload( newsFile,
                              "http://tremulous.net/clientnews.txt", com_developer->integer );
            cls.bWWWDlDisconnected = true;
            return false;
        }
    }
    
    if( FS_SV_FOpenFileRead( newsFile, &clc.download ) )
    {
        readSize = FS_Read( clc.newsString, sizeof( clc.newsString ), clc.download );
        clc.newsString[ readSize ] = '\0';
        if( readSize > 0 )
        {
            finished = true;
            clc.bWWWDl = false;
            cls.bWWWDlDisconnected = false;
        }
    }
    FS_FCloseFile( clc.download );
    
    if( !finished )
        strcpy( clc.newsString, "Retrieving..." );
    Cvar_Set( "cl_newsString", clc.newsString );
    return finished;
}

/*
====================
LAN_ResetPings
====================
*/
static void LAN_ResetPings( S32 source )
{
    S32             count, i;
    serverInfo_t*   servers = NULL;
    
    count = 0;
    
    switch( source )
    {
        case AS_LOCAL:
            servers = &cls.localServers[0];
            count = MAX_OTHER_SERVERS;
            break;
        case AS_GLOBAL:
            servers = &cls.globalServers[0];
            count = MAX_GLOBAL_SERVERS;
            break;
        case AS_FAVORITES:
            servers = &cls.favoriteServers[0];
            count = MAX_OTHER_SERVERS;
            break;
    }
    if( servers )
    {
        for( i = 0; i < count; i++ )
        {
            servers[i].ping = -1;
        }
    }
}

/*
====================
LAN_AddServer
====================
*/
static S32 LAN_AddServer( S32 source, StringEntry name, StringEntry address )
{
    S32 max, *count, i;
    netadr_t adr;
    serverInfo_t* servers = NULL;
    max = MAX_OTHER_SERVERS;
    count = 0;
    
    switch( source )
    {
        case AS_LOCAL:
            count = &cls.numlocalservers;
            servers = &cls.localServers[0];
            break;
        case AS_GLOBAL:
            max = MAX_GLOBAL_SERVERS;
            count = &cls.numglobalservers;
            servers = &cls.globalServers[0];
            break;
        case AS_FAVORITES:
            count = &cls.numfavoriteservers;
            servers = &cls.favoriteServers[0];
            break;
    }
    if( servers && *count < max )
    {
        NET_StringToAdr( address, &adr, NA_IP );
        for( i = 0; i < *count; i++ )
        {
            if( NET_CompareAdr( servers[i].adr, adr ) )
            {
                break;
            }
        }
        if( i >= *count )
        {
            servers[*count].adr = adr;
            Q_strncpyz( servers[*count].hostName, name, sizeof( servers[*count].hostName ) );
            servers[*count].visible = true;
            ( *count )++;
            return 1;
        }
        return 0;
    }
    return -1;
}

/*
====================
LAN_RemoveServer
====================
*/
static void LAN_RemoveServer( S32 source, StringEntry addr )
{
    S32* count, i;
    serverInfo_t* servers = NULL;
    count = 0;
    switch( source )
    {
        case AS_LOCAL:
            count = &cls.numlocalservers;
            servers = &cls.localServers[0];
            break;
        case AS_GLOBAL:
            count = &cls.numglobalservers;
            servers = &cls.globalServers[0];
            break;
        case AS_FAVORITES:
            count = &cls.numfavoriteservers;
            servers = &cls.favoriteServers[0];
            break;
    }
    if( servers )
    {
        netadr_t comp;
        NET_StringToAdr( addr, &comp, NA_IP );
        for( i = 0; i < *count; i++ )
        {
            if( NET_CompareAdr( comp, servers[i].adr ) )
            {
                S32 j = i;
                while( j < *count - 1 )
                {
                    ::memcpy( &servers[j], &servers[j + 1], sizeof( servers[j] ) );
                    j++;
                }
                ( *count )--;
                break;
            }
        }
    }
}

/*
====================
LAN_GetServerCount
====================
*/
static S32 LAN_GetServerCount( S32 source )
{
    switch( source )
    {
        case AS_LOCAL:
            return cls.numlocalservers;
            break;
        case AS_GLOBAL:
            return cls.numglobalservers;
            break;
        case AS_FAVORITES:
            return cls.numfavoriteservers;
            break;
    }
    return 0;
}

/*
====================
LAN_GetLocalServerAddressString
====================
*/
static void LAN_GetServerAddressString( S32 source, S32 n, UTF8* buf, S32 buflen )
{
    switch( source )
    {
        case AS_LOCAL:
            if( n >= 0 && n < MAX_OTHER_SERVERS )
            {
                Q_strncpyz( buf, NET_AdrToStringwPort( cls.localServers[n].adr ), buflen );
                return;
            }
            break;
        case AS_GLOBAL:
            if( n >= 0 && n < MAX_GLOBAL_SERVERS )
            {
                Q_strncpyz( buf, NET_AdrToStringwPort( cls.globalServers[n].adr ), buflen );
                return;
            }
            break;
        case AS_FAVORITES:
            if( n >= 0 && n < MAX_OTHER_SERVERS )
            {
                Q_strncpyz( buf, NET_AdrToStringwPort( cls.favoriteServers[n].adr ), buflen );
                return;
            }
            break;
    }
    buf[0] = '\0';
}

/*
====================
LAN_GetServerInfo
====================
*/
static void LAN_GetServerInfo( S32 source, S32 n, UTF8* buf, S32 buflen )
{
    UTF8            info[MAX_STRING_CHARS];
    serverInfo_t*   server = NULL;
    
    info[0] = '\0';
    switch( source )
    {
        case AS_LOCAL:
            if( n >= 0 && n < MAX_OTHER_SERVERS )
            {
                server = &cls.localServers[n];
            }
            break;
        case AS_GLOBAL:
            if( n >= 0 && n < MAX_GLOBAL_SERVERS )
            {
                server = &cls.globalServers[n];
            }
            break;
        case AS_FAVORITES:
            if( n >= 0 && n < MAX_OTHER_SERVERS )
            {
                server = &cls.favoriteServers[n];
            }
            break;
    }
    if( server && buf )
    {
        buf[0] = '\0';
        Info_SetValueForKey( info, "hostname", server->hostName );
        Info_SetValueForKey( info, "serverload", va( "%i", server->load ) );
        Info_SetValueForKey( info, "mapname", server->mapName );
        Info_SetValueForKey( info, "clients", va( "%i", server->clients ) );
        Info_SetValueForKey( info, "sv_maxclients", va( "%i", server->maxClients ) );
        Info_SetValueForKey( info, "ping", va( "%i", server->ping ) );
        Info_SetValueForKey( info, "minping", va( "%i", server->minPing ) );
        Info_SetValueForKey( info, "maxping", va( "%i", server->maxPing ) );
        Info_SetValueForKey( info, "game", server->game );
        Info_SetValueForKey( info, "gametype", va( "%i", server->gameType ) );
        Info_SetValueForKey( info, "nettype", va( "%i", server->netType ) );
        Info_SetValueForKey( info, "addr", NET_AdrToStringwPort( server->adr ) );
        Info_SetValueForKey( info, "sv_allowAnonymous", va( "%i", server->allowAnonymous ) );
        Info_SetValueForKey( info, "friendlyFire", va( "%i", server->friendlyFire ) );	// NERVE - SMF
        Info_SetValueForKey( info, "maxlives", va( "%i", server->maxlives ) );	// NERVE - SMF
        Info_SetValueForKey( info, "needpass", va( "%i", server->needpass ) );	// NERVE - SMF
        Info_SetValueForKey( info, "punkbuster", va( "%i", server->punkbuster ) );	// DHM - Nerve
        Info_SetValueForKey( info, "gamename", server->gameName );	// Arnout
        Info_SetValueForKey( info, "g_antilag", va( "%i", server->antilag ) );	// TTimo
        Info_SetValueForKey( info, "weaprestrict", va( "%i", server->weaprestrict ) );
        Info_SetValueForKey( info, "balancedteams", va( "%i", server->balancedteams ) );
        Q_strncpyz( buf, info, buflen );
    }
    else
    {
        if( buf )
        {
            buf[0] = '\0';
        }
    }
}

/*
====================
LAN_GetServerPing
====================
*/
static S32 LAN_GetServerPing( S32 source, S32 n )
{
    serverInfo_t*   server = NULL;
    
    switch( source )
    {
        case AS_LOCAL:
            if( n >= 0 && n < MAX_OTHER_SERVERS )
            {
                server = &cls.localServers[n];
            }
            break;
        case AS_GLOBAL:
            if( n >= 0 && n < MAX_GLOBAL_SERVERS )
            {
                server = &cls.globalServers[n];
            }
            break;
        case AS_FAVORITES:
            if( n >= 0 && n < MAX_OTHER_SERVERS )
            {
                server = &cls.favoriteServers[n];
            }
            break;
    }
    if( server )
    {
        return server->ping;
    }
    return -1;
}

/*
====================
LAN_GetServerPtr
====================
*/
static serverInfo_t* LAN_GetServerPtr( S32 source, S32 n )
{
    switch( source )
    {
        case AS_LOCAL:
            if( n >= 0 && n < MAX_OTHER_SERVERS )
            {
                return &cls.localServers[n];
            }
            break;
        case AS_GLOBAL:
            if( n >= 0 && n < MAX_GLOBAL_SERVERS )
            {
                return &cls.globalServers[n];
            }
            break;
        case AS_FAVORITES:
            if( n >= 0 && n < MAX_OTHER_SERVERS )
            {
                return &cls.favoriteServers[n];
            }
            break;
    }
    return NULL;
}

/*
====================
LAN_CompareServers
====================
*/
static S32 LAN_CompareServers( S32 source, S32 sortKey, S32 sortDir, S32 s1, S32 s2 )
{
    S32             res;
    serverInfo_t*   server1, *server2;
    UTF8            name1[MAX_NAME_LENGTH], name2[MAX_NAME_LENGTH];
    
    server1 = LAN_GetServerPtr( source, s1 );
    server2 = LAN_GetServerPtr( source, s2 );
    if( !server1 || !server2 )
    {
        return 0;
    }
    
    res = 0;
    switch( sortKey )
    {
        case SORT_HOST:
            //% res = Q_stricmp( server1->hostName, server2->hostName );
            Q_strncpyz( name1, server1->hostName, sizeof( name1 ) );
            Q_CleanStr( name1 );
            Q_strncpyz( name2, server2->hostName, sizeof( name2 ) );
            Q_CleanStr( name2 );
            res = Q_stricmp( name1, name2 );
            break;
            
        case SORT_MAP:
            res = Q_stricmp( server1->mapName, server2->mapName );
            break;
        case SORT_CLIENTS:
            if( server1->clients < server2->clients )
            {
                res = -1;
            }
            else if( server1->clients > server2->clients )
            {
                res = 1;
            }
            else
            {
                res = 0;
            }
            break;
        case SORT_GAME:
            if( server1->gameType < server2->gameType )
            {
                res = -1;
            }
            else if( server1->gameType > server2->gameType )
            {
                res = 1;
            }
            else
            {
                res = 0;
            }
            break;
        case SORT_PING:
            if( server1->ping < server2->ping )
            {
                res = -1;
            }
            else if( server1->ping > server2->ping )
            {
                res = 1;
            }
            else
            {
                res = 0;
            }
            break;
    }
    
    if( sortDir )
    {
        if( res < 0 )
        {
            return 1;
        }
        if( res > 0 )
        {
            return -1;
        }
        return 0;
    }
    return res;
}

/*
====================
LAN_GetPingQueueCount
====================
*/
static S32 LAN_GetPingQueueCount( void )
{
    return ( CL_GetPingQueueCount() );
}

/*
====================
LAN_ClearPing
====================
*/
static void LAN_ClearPing( S32 n )
{
    CL_ClearPing( n );
}

/*
====================
LAN_GetPing
====================
*/
static void LAN_GetPing( S32 n, UTF8* buf, S32 buflen, S32* pingtime )
{
    CL_GetPing( n, buf, buflen, pingtime );
}

/*
====================
LAN_GetPingInfo
====================
*/
static void LAN_GetPingInfo( S32 n, UTF8* buf, S32 buflen )
{
    CL_GetPingInfo( n, buf, buflen );
}

/*
====================
LAN_MarkServerVisible
====================
*/
static void LAN_MarkServerVisible( S32 source, S32 n, bool visible )
{
    if( n == -1 )
    {
        S32             count = MAX_OTHER_SERVERS;
        serverInfo_t*   server = NULL;
        
        switch( source )
        {
            case AS_LOCAL:
                server = &cls.localServers[0];
                break;
            case AS_GLOBAL:
                server = &cls.globalServers[0];
                count = MAX_GLOBAL_SERVERS;
                break;
            case AS_FAVORITES:
                server = &cls.favoriteServers[0];
                break;
        }
        if( server )
        {
            for( n = 0; n < count; n++ )
            {
                server[n].visible = visible;
            }
        }
        
    }
    else
    {
        switch( source )
        {
            case AS_LOCAL:
                if( n >= 0 && n < MAX_OTHER_SERVERS )
                {
                    cls.localServers[n].visible = visible;
                }
                break;
            case AS_GLOBAL:
                if( n >= 0 && n < MAX_GLOBAL_SERVERS )
                {
                    cls.globalServers[n].visible = visible;
                }
                break;
            case AS_FAVORITES:
                if( n >= 0 && n < MAX_OTHER_SERVERS )
                {
                    cls.favoriteServers[n].visible = visible;
                }
                break;
        }
    }
}


/*
=======================
LAN_ServerIsVisible
=======================
*/
static S32 LAN_ServerIsVisible( S32 source, S32 n )
{
    switch( source )
    {
        case AS_LOCAL:
            if( n >= 0 && n < MAX_OTHER_SERVERS )
            {
                return cls.localServers[n].visible;
            }
            break;
        case AS_GLOBAL:
            if( n >= 0 && n < MAX_GLOBAL_SERVERS )
            {
                return cls.globalServers[n].visible;
            }
            break;
        case AS_FAVORITES:
            if( n >= 0 && n < MAX_OTHER_SERVERS )
            {
                return cls.favoriteServers[n].visible;
            }
            break;
    }
    return false;
}

/*
=======================
LAN_UpdateVisiblePings
=======================
*/
bool LAN_UpdateVisiblePings( S32 source )
{
    return CL_UpdateVisiblePings_f( source );
}

/*
====================
LAN_GetServerStatus
====================
*/
S32 LAN_GetServerStatus( UTF8* serverAddress, UTF8* serverStatus, S32 maxLen )
{
    return CL_ServerStatus( serverAddress, serverStatus, maxLen );
}

/*
=======================
LAN_ServerIsInFavoriteList
=======================
*/
bool LAN_ServerIsInFavoriteList( S32 source, S32 n )
{
    S32             i;
    serverInfo_t*   server = NULL;
    
    switch( source )
    {
        case AS_LOCAL:
            if( n >= 0 && n < MAX_OTHER_SERVERS )
            {
                server = &cls.localServers[n];
            }
            break;
        case AS_GLOBAL:
            if( n >= 0 && n < MAX_GLOBAL_SERVERS )
            {
                server = &cls.globalServers[n];
            }
            break;
        case AS_FAVORITES:
            if( n >= 0 && n < MAX_OTHER_SERVERS )
            {
                return true;
            }
            break;
    }
    
    if( !server )
    {
        return false;
    }
    
    for( i = 0; i < cls.numfavoriteservers; i++ )
    {
        if( NET_CompareAdr( cls.favoriteServers[i].adr, server->adr ) )
        {
            return true;
        }
    }
    
    return false;
}

/*
====================
CL_GetGlConfig
====================
*/
static void CL_GetGlconfig( glconfig_t* config )
{
    *config = cls.glconfig;
}

/*
====================
GetClipboarzdData
====================
*/
static void GetClipboardData( UTF8* buf, S32 buflen )
{
    UTF8*           cbd;
    
    cbd = Sys_GetClipboardData();
    
    if( !cbd )
    {
        *buf = 0;
        return;
    }
    
    Q_strncpyz( buf, cbd, buflen );
    
    Z_Free( cbd );
}

/*
====================
Key_KeynumToStringBuf
====================
*/
void Key_KeynumToStringBuf( S32 keynum, UTF8* buf, S32 buflen )
{
    Q_strncpyz( buf, Key_KeynumToString( keynum ), buflen );
}

/*
====================
Key_GetBindingBuf
====================
*/
void Key_GetBindingBuf( S32 keynum, UTF8* buf, S32 buflen )
{
    UTF8*           value;
    
    value = Key_GetBinding( keynum );
    if( value )
    {
        Q_strncpyz( buf, value, buflen );
    }
    else
    {
        *buf = 0;
    }
}

/*
====================
Key_GetCatcher
====================
*/
S32 Key_GetCatcher( void )
{
    return cls.keyCatchers;
}

/*
====================
Ket_SetCatcher
====================
*/
void Key_SetCatcher( S32 catcher )
{
    // NERVE - SMF - console overrides everything
    if( cls.keyCatchers & KEYCATCH_CONSOLE )
    {
        cls.keyCatchers = catcher | KEYCATCH_CONSOLE;
    }
    else
    {
        cls.keyCatchers = catcher;
    }
    
}


/*
====================
CLUI_GetCDKey
====================
*/
static void CLUI_GetCDKey( UTF8* buf, S32 buflen )
{
    cvar_t*         fs;
    
    fs = Cvar_Get( "fs_game", "", CVAR_INIT | CVAR_SYSTEMINFO );
    if( UI_usesUniqueCDKey() && fs && fs->string[0] != 0 )
    {
        memcpy( buf, &cl_cdkey[16], 16 );
        buf[16] = 0;
    }
    else
    {
        memcpy( buf, cl_cdkey, 16 );
        buf[16] = 0;
    }
}


/*
====================
CLUI_SetCDKey
====================
*/
static void CLUI_SetCDKey( UTF8* buf )
{
    cvar_t*         fs;
    
    fs = Cvar_Get( "fs_game", "", CVAR_INIT | CVAR_SYSTEMINFO );
    if( UI_usesUniqueCDKey() && fs && fs->string[0] != 0 )
    {
        memcpy( &cl_cdkey[16], buf, 16 );
        cl_cdkey[32] = 0;
        // set the flag so the fle will be written at the next opportunity
        cvar_modifiedFlags |= CVAR_ARCHIVE;
    }
    else
    {
        memcpy( cl_cdkey, buf, 16 );
        // set the flag so the fle will be written at the next opportunity
        cvar_modifiedFlags |= CVAR_ARCHIVE;
    }
}


/*
====================
GetConfigString
====================
*/
static S32 GetConfigString( S32 index, UTF8* buf, S32 size )
{
    S32             offset;
    
    if( index < 0 || index >= MAX_CONFIGSTRINGS )
    {
        return false;
    }
    
    offset = cl.gameState.stringOffsets[index];
    if( !offset )
    {
        if( size )
        {
            buf[0] = 0;
        }
        return false;
    }
    
    Q_strncpyz( buf, cl.gameState.stringData + offset, size );
    
    return true;
}

/*
====================
CL_ShutdownUI
====================
*/
void CL_ShutdownUI( void )
{
    cls.keyCatchers &= ~KEYCATCH_UI;
    cls.uiStarted = false;
    uiManager->Shutdown();
}

/*
====================
CL_InitUI
====================
*/

void CL_InitUI( void )
{
#if defined (USE_HTTP)
    //	if the session id has something in it, then assume that the browser sent it from the
    //	command line and tell ui we're already logged in.
    if( com_sessionid->string[0] )
    {
        //uiManager->Authorized(AUTHORIZE_OK);
    }
#endif
    
    uiManager->Init( cls.state >= CA_AUTHORIZING && cls.state < CA_ACTIVE );
}


bool UI_usesUniqueCDKey()
{
    return false;
}


bool UI_checkKeyExec( S32 key )
{
    if( uivm )
    {
        return uiManager->CheckExecKey( key );
    }
    else
    {
        return false;
    }
}

/*
====================
UI_GameCommand

See if the current console command is claimed by the ui
====================
*/
bool UI_GameCommand( void )
{
    if( !cls.uiStarted )
    {
        return false;
    }
    
    return uiManager->ConsoleCommand( cls.realtime );
}

//
// REMOVE THESE
//

#define PASSFLOAT(x) x

void trap_Error( StringEntry string )
{
    Com_Error( ERR_DROP, string );
}

void trap_Print( StringEntry string )
{
    Com_Printf( string );
}

S32 trap_Milliseconds( void )
{
    return Sys_Milliseconds();
}

void trap_Cvar_Register( vmCvar_t* cvar, StringEntry var_name, StringEntry value, S32 flags )
{
    Cvar_Register( cvar, var_name, value, flags );
}

void trap_Cvar_Update( vmCvar_t* cvar )
{
    Cvar_Update( cvar );
}

void trap_Cvar_Set( StringEntry var_name, StringEntry value )
{
    Cvar_Set( var_name, value );
}

F32 trap_Cvar_VariableValue( StringEntry var_name )
{
    return Cvar_VariableValue( var_name );;
}

void trap_Cvar_VariableStringBuffer( StringEntry var_name, UTF8* buffer, S32 bufsize )
{
    Cvar_VariableStringBuffer( var_name, buffer, bufsize );
}

void trap_Cvar_LatchedVariableStringBuffer( StringEntry var_name, UTF8* buffer, S32 bufsize )
{
    Cvar_VariableStringBuffer( var_name, buffer, bufsize );
}

void trap_Cvar_SetValue( StringEntry var_name, F32 value )
{
    Cvar_SetValue( var_name, PASSFLOAT( value ) );
}

void trap_Cvar_Reset( StringEntry name )
{
    Cvar_Reset( name );
}

void trap_Cvar_Create( StringEntry var_name, StringEntry var_value, S32 flags )
{
    Cvar_Get( var_name, var_value, flags );
}

void trap_Cvar_InfoStringBuffer( S32 bit, UTF8* buffer, S32 bufsize )
{
    Cvar_InfoStringBuffer( bit, buffer, bufsize );
}

S32 trap_Argc( void )
{
    return Cmd_Argc();
}

void trap_Argv( S32 n, UTF8* buffer, S32 bufferLength )
{
    Cmd_ArgvBuffer( n, buffer, bufferLength );
}

void trap_Cmd_ExecuteText( S32 exec_when, StringEntry text )
{
    Cbuf_ExecuteText( exec_when, text );
}

void trap_AddCommand( StringEntry cmdName )
{
    Cmd_AddCommand( cmdName, NULL );
}
;
S32 trap_FS_FOpenFile( StringEntry qpath, fileHandle_t* f, fsMode_t mode )
{
    return FS_FOpenFileByMode( qpath, f, mode );
}

void trap_FS_Read( void* buffer, S32 len, fileHandle_t f )
{
    FS_Read( buffer, len, f );
}

S32 trap_FS_Write( const void* buffer, S32 len, fileHandle_t f )
{
    return FS_Write( buffer, len, f );
}

void trap_FS_FCloseFile( fileHandle_t f )
{
    FS_FCloseFile( f );
}

S32 trap_FS_Delete( UTF8* filename )
{
    return FS_Delete( filename );
}

S32 trap_FS_GetFileList( StringEntry path, StringEntry extension, UTF8* listbuf, S32 bufsize )
{
    return FS_GetFileList( path, extension, listbuf, bufsize );
}

S32 trap_FS_Seek( fileHandle_t f, S64 offset, S32 origin )
{
    return FS_Seek( f, offset, origin );
}

qhandle_t trap_R_RegisterModel( StringEntry name )
{
    return renderSystem->RegisterModel( name );
}

qhandle_t trap_R_RegisterSkin( StringEntry name )
{
    return renderSystem->RegisterSkin( name );
}

qhandle_t trap_R_RegisterShaderNoMip( StringEntry name )
{
    return renderSystem->RegisterShaderNoMip( name );
}

void trap_R_ClearScene( void )
{
    renderSystem->ClearScene();
}

void trap_R_AddRefEntityToScene( const refEntity_t* re )
{
    renderSystem->AddRefEntityToScene( re );
}

void trap_R_AddPolyToScene( qhandle_t hShader , S32 numVerts, const polyVert_t* verts )
{
    renderSystem->AddPolyToScene( hShader, numVerts, verts, 1 );
}

void trap_R_AddPolysToScene( qhandle_t hShader, S32 numVerts, const polyVert_t* verts, S32 numPolys )
{
    renderSystem->AddPolyToScene( hShader, numVerts, verts, numPolys );
}

void trap_R_AddLightToScene( const vec3_t org, F32 intensity, F32 r, F32 g, F32 b )
{
    renderSystem->AddLightToScene( org, PASSFLOAT( intensity ), PASSFLOAT( r ), PASSFLOAT( g ), PASSFLOAT( b ) );
}

void trap_R_AddCoronaToScene( const vec3_t org, F32 r, F32 g, F32 b, F32 scale, S32 id, bool visible )
{
    //renderSystem->AddCoronaToScene( org, PASSFLOAT( r ), PASSFLOAT( g ), PASSFLOAT( b ), PASSFLOAT( scale ), id, visible );
}
;
void trap_R_RenderScene( const refdef_t* fd )
{
    renderSystem->RenderScene( fd );
}

void trap_R_SetColor( const F32* rgba )
{
    renderSystem->SetColor( rgba );
}

void trap_R_SetClipRegion( const F32* region )
{
    renderSystem->SetClipRegion( region );
}

void trap_R_Add2dPolys( polyVert_t* verts, S32 numverts, qhandle_t hShader )
{
    //renderSystem->Add2dPolys( verts, numverts, hShader );
}

void trap_R_DrawStretchPic( F32 x, F32 y, F32 w, F32 h, F32 s1, F32 t1, F32 s2, F32 t2, qhandle_t hShader )
{
    renderSystem->DrawStretchPic( PASSFLOAT( x ), PASSFLOAT( y ), PASSFLOAT( w ), PASSFLOAT( h ), PASSFLOAT( s1 ), PASSFLOAT( t1 ), PASSFLOAT( s2 ), PASSFLOAT( t2 ), hShader );
}

void trap_R_DrawRotatedPic( F32 x, F32 y, F32 w, F32 h, F32 s1, F32 t1, F32 s2, F32 t2, qhandle_t hShader, F32 angle )
{
    //renderSystem->DrawRotatedPic( PASSFLOAT( x ), PASSFLOAT( y ), PASSFLOAT( w ), PASSFLOAT( h ), PASSFLOAT( s1 ), PASSFLOAT( t1 ), PASSFLOAT( s2 ), PASSFLOAT( t2 ), hShader, PASSFLOAT( angle ) );
}

void trap_R_ModelBounds( clipHandle_t model, vec3_t mins, vec3_t maxs )
{
    renderSystem->ModelBounds( model, mins, maxs );
}

void trap_UpdateScreen( void )
{
    SCR_UpdateScreen();
}

S32 trap_CM_LerpTag( orientation_t* tag, clipHandle_t mod, S32 startFrame, S32 endFrame, F32 frac, StringEntry tagName )
{
    return renderSystem->LerpTag( tag, mod, startFrame, endFrame, PASSFLOAT( frac ), tagName );
}

sfxHandle_t trap_S_RegisterSound( StringEntry sample )
{
    S32 i = soundSystem->RegisterSound( sample );
#ifdef _DEBUG
    if( i == 0 )
    {
        Com_Printf( "^1Warning: Failed to load sound: %s\n", sample );
    }
#endif
    return i;
}

void trap_S_StartLocalSound( sfxHandle_t sfx, S32 channelNum )
{
    soundSystem->StartLocalSound( sfx, channelNum );
}

void trap_Key_KeynumToStringBuf( S32 keynum, UTF8* buf, S32 buflen )
{
    Key_KeynumToStringBuf( keynum, buf, buflen );
}

void trap_Key_GetBindingBuf( S32 keynum, UTF8* buf, S32 buflen )
{
    Key_GetBindingBuf( keynum, buf, buflen );
}

void trap_Key_SetBinding( S32 keynum, StringEntry binding )
{
    Key_SetBinding( keynum, binding );
}

//49.
//Key_GetBindingByString(VMA(1), VMA(2), VMA(3));
void trap_Key_KeysForBinding( StringEntry binding, S32* key1, S32* key2 )
{
    Key_GetBindingByString( binding, key1, key2 );
}

//50.
//return Key_IsDown(args[1]);
bool trap_Key_IsDown( S32 keynum )
{
    return Key_IsDown( keynum );
}

//51.
//return Key_GetOverstrikeMode();
bool trap_Key_GetOverstrikeMode( void )
{
    return Key_GetOverstrikeMode( );
}

//52.
//Key_SetOverstrikeMode(args[1]);
void trap_Key_SetOverstrikeMode( bool state )
{
    Key_SetOverstrikeMode( state );
}

//53.
//Key_ClearStates();
void trap_Key_ClearStates( void )
{
    Key_ClearStates();
}

//54.
//return Key_GetCatcher();
S32 trap_Key_GetCatcher( void )
{
    return Key_GetCatcher();
}

//55.
//Key_SetCatcher(args[1]);
void trap_Key_SetCatcher( S32 catcher )
{
    Key_SetCatcher( catcher );
}

//56.
//GetClipboardData(VMA(1), args[2]);
void trap_GetClipboardData( UTF8* buf, S32 bufsize )
{
    GetClipboardData( buf, bufsize );
}

//57.
//GetClientState(VMA(1));
void trap_GetClientState( uiClientState_t* state )
{
    GetClientState( state );
}

//58.
//CL_GetGlconfig(VMA(1));
void trap_GetGlconfig( glconfig_t* glconfig )
{
    CL_GetGlconfig( glconfig );
}

//59.
//return GetConfigString(args[1], VMA(2), args[3]);
S32 trap_GetConfigString( S32 index, UTF8* buff, S32 buffsize )
{
    return GetConfigString( index, buff, buffsize );
}

//60.
//LAN_LoadCachedServers();
void trap_LAN_LoadCachedServers( void )
{
    LAN_LoadCachedServers();
}

//61.
//LAN_SaveServersToCache();
void trap_LAN_SaveCachedServers( void )
{
    LAN_SaveServersToCache();
}

//62.
//return LAN_AddServer(args[1], VMA(2), VMA(3));
S32 trap_LAN_AddServer( S32 source, StringEntry name, StringEntry addr )
{
    return LAN_AddServer( source, name, addr );
}

//63.
//LAN_RemoveServer(args[1], VMA(2));
void trap_LAN_RemoveServer( S32 source, StringEntry addr )
{
    LAN_RemoveServer( source, addr );
}

//64.
//return LAN_GetPingQueueCount();
S32 trap_LAN_GetPingQueueCount( void )
{
    return LAN_GetPingQueueCount( );
}

//65.
//LAN_ClearPing(args[1]);
void trap_LAN_ClearPing( S32 n )
{
    LAN_ClearPing( n );
}

//66.
//LAN_GetPing(args[1], VMA(2), args[3], VMA(4));
void trap_LAN_GetPing( S32 n, UTF8* buf, S32 buflen, S32* pingtime )
{
    LAN_GetPing( n, buf, buflen, pingtime );
}

//67.
//LAN_GetPingInfo(args[1], VMA(2), args[3]);
void trap_LAN_GetPingInfo( S32 n, UTF8* buf, S32 buflen )
{
    LAN_GetPingInfo( n, buf, buflen );
}

//68.
//return LAN_GetServerCount(args[1]);
S32 trap_LAN_GetServerCount( S32 source )
{
    return LAN_GetServerCount( source );
}

//69.
//LAN_GetServerAddressString(args[1], args[2], VMA(3), args[4]);
void trap_LAN_GetServerAddressString( S32 source, S32 n, UTF8* buf, S32 buflen )
{
    LAN_GetServerAddressString( source, n, buf, buflen );
}

//70.
//LAN_GetServerInfo(args[1], args[2], VMA(3), args[4]);
void trap_LAN_GetServerInfo( S32 source, S32 n, UTF8* buf, S32 buflen )
{
    LAN_GetServerInfo( source, n, buf, buflen );
}

//71.
//return LAN_GetServerPing(args[1], args[2]);
S32 trap_LAN_GetServerPing( S32 source, S32 n )
{
    return LAN_GetServerPing( source, n );
}

//72.
//LAN_MarkServerVisible(args[1], args[2], args[3]);
void trap_LAN_MarkServerVisible( S32 source, S32 n, bool visible )
{
    LAN_MarkServerVisible( source, n, visible );
}

//73.
//return LAN_ServerIsVisible(args[1], args[2]);
S32 trap_LAN_ServerIsVisible( S32 source, S32 n )
{
    return LAN_ServerIsVisible( source, n );
}

//74.
//return LAN_UpdateVisiblePings(args[1]);
bool trap_LAN_UpdateVisiblePings( S32 source )
{
    return LAN_UpdateVisiblePings( source );
}

//75.
//LAN_ResetPings(args[1]);
void trap_LAN_ResetPings( S32 n )
{
    LAN_ResetPings( n );
}

//76.
//return LAN_GetServerStatus(VMA(1), VMA(2), args[3]);
S32 trap_LAN_ServerStatus( UTF8* serverAddress, UTF8* serverStatus, S32 maxLen )
{
    return LAN_GetServerStatus( serverAddress, serverStatus, maxLen );
}

//77.
//return LAN_ServerIsInFavoriteList(args[1], args[2]);
bool trap_LAN_ServerIsInFavoriteList( S32 source, S32 n )
{
    return LAN_ServerIsInFavoriteList( source, n );
}

//78.
//return GetNews(args[1]);
bool trap_GetNews( bool force )
{
    return GetNews( force );
}

//79.
//return LAN_CompareServers(args[1], args[2], args[3], args[4], args[5]);
S32 trap_LAN_CompareServers( S32 source, S32 sortKey, S32 sortDir, S32 s1, S32 s2 )
{
    return LAN_CompareServers( source, sortKey, sortDir, s1, s2 );
}

//80.
//return Hunk_MemoryRemaining();
S32 trap_MemoryRemaining( void )
{
    return Hunk_MemoryRemaining( );
}

//81.
//CLUI_GetCDKey(VMA(1), args[2]);
void trap_GetCDKey( UTF8* buf, S32 buflen )
{
    CLUI_GetCDKey( buf, buflen );
}


void trap_SetCDKey( UTF8* buf )
{
    CLUI_SetCDKey( buf );
}

void trap_R_RegisterFont( StringEntry fontName, S32 pointSize, fontInfo_t* font )
{
    renderSystem->RegisterFont( fontName, pointSize, font );
}

//98.
//return botlib_export->PC_AddGlobalDefine(VMA(1));
S32 trap_PC_AddGlobalDefine( UTF8* define )
{
    return botlib_export->PC_AddGlobalDefine( define );
}

//99.
//botlib_export->PC_RemoveAllGlobalDefines();
void trap_PC_RemoveAllGlobalDefines( void )
{
    return;// botlib_export->PC_RemoveAllGlobalDefines();
}

//100.
//return botlib_export->PC_LoadSourceHandle(VMA(1));
S32 trap_PC_LoadSource( StringEntry filename )
{
    return botlib_export->PC_LoadSourceHandle( filename );
}

//101.
//return botlib_export->PC_FreeSourceHandle(args[1]);
S32 trap_PC_FreeSource( S32 handle )
{
    return botlib_export->PC_FreeSourceHandle( handle );
}

//102.
//return botlib_export->PC_ReadTokenHandle(args[1], VMA(2));
S32 trap_PC_ReadToken( S32 handle, pc_token_t* pc_token )
{
    return botlib_export->PC_ReadTokenHandle( handle, pc_token );
}

//103.
//return botlib_export->PC_SourceFileAndLine(args[1], VMA(2), VMA(3));
S32 trap_PC_SourceFileAndLine( S32 handle, UTF8* filename, S32* line )
{
    return botlib_export->PC_SourceFileAndLine( handle, filename, line );
}

//104.
//botlib_export->PC_UnreadLastTokenHandle(args[1]);
void trap_PC_UnReadToken( S32 handle )
{
    return;// botlib_export->PC_UnreadLastTokenHandle(handle);
}

//105.
//S_StopBackgroundTrack();
void trap_S_StopBackgroundTrack( void )
{
    soundSystem->StopBackgroundTrack();
}

void trap_S_StartBackgroundTrack( StringEntry intro, StringEntry loop )
{
    soundSystem->StartBackgroundTrack( intro, loop );
}

//107.
//return Com_RealTime(VMA(1));
S32 trap_RealTime( qtime_t* qtime )
{
    return Com_RealTime( qtime );
}

//108.
//return CIN_PlayCinematic(VMA(1), args[2], args[3], args[4], args[5], args[6]);
S32 trap_CIN_PlayCinematic( StringEntry arg0, S32 xpos, S32 ypos, S32 width, S32 height, S32 bits )
{
    return CIN_PlayCinematic( arg0, xpos, ypos, width, height, bits );
}

//109.
//return CIN_StopCinematic(args[1]);
e_status trap_CIN_StopCinematic( S32 handle )
{
    return CIN_StopCinematic( handle );
}

//110.
//return CIN_RunCinematic(args[1]);
e_status trap_CIN_RunCinematic( S32 handle )
{
    return CIN_RunCinematic( handle );
}

//111.
//CIN_DrawCinematic(args[1]);
void trap_CIN_DrawCinematic( S32 handle )
{
    CIN_DrawCinematic( handle );
}

//112.
//CIN_SetExtents(args[1], args[2], args[3], args[4], args[5]);
void trap_CIN_SetExtents( S32 handle, S32 x, S32 y, S32 w, S32 h )
{
    CIN_SetExtents( handle, x, y, w, h );
}

//113.
//re.RemapShader(VMA(1), VMA(2), VMA(3));
void trap_R_RemapShader( StringEntry oldShader, StringEntry newShader, StringEntry timeOffset )
{
    renderSystem->RemapShader( oldShader, newShader, timeOffset );
}

//114.
//return CL_GetLimboString(args[1], VMA(2));
bool trap_GetLimboString( S32 index, UTF8* buf )
{
    return CL_GetLimboString( index, buf );
}

//115.
//CL_TranslateString(VMA(1), VMA(2));
UTF8* trap_TranslateString( StringEntry string )
{
    static UTF8     staticbuf[2][32000];
    static S32      bufcount = 0;
    UTF8*           buf;
    
    buf = staticbuf[bufcount++ % 2];
    
#ifdef LOCALIZATION_SUPPORT
    CL_TranslateString( string, buf );
#else
    Q_strncpyz( buf, string, MAX_VA_STRING );
#endif							// LOCALIZATION_SUPPORT
    return buf;
}

//116.
//CL_CheckAutoUpdate();
void trap_CheckAutoUpdate( void )
{
    CL_CheckAutoUpdate();
}

//117.
//CL_GetAutoUpdate();
void trap_GetAutoUpdate( void )
{
    CL_GetAutoUpdate();
}

//118.
//CL_OpenURL((const UTF8 *)VMA(1));
void trap_openURL( StringEntry s )
{
    CL_OpenURL( s );
}

//119.
//Com_GetHunkInfo(VMA(1), VMA(2));
void trap_GetHunkData( S32* hunkused, S32* hunkexpected )
{
    Com_GetHunkInfo( hunkused, hunkexpected );
}

S32 trap_Cvar_VariableInt( StringEntry var_name )
{
    return Cvar_VariableIntegerValue( var_name );
}

S32 trap_CrosshairPlayer( void )
{
    return cgame->CrosshairPlayer();
}

S32 trap_LastAttacker( void )
{
    return cgame->LastAttacker();
}


