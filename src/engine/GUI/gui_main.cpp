////////////////////////////////////////////////////////////////////////////////////////
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
// File name:   gui_main.cpp
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#include <OWLib/precompiled.h>

extern bool g_waitingForKey;
extern bool g_editingField;
extern itemDef_t* g_editItem;

uiInfo_t uiInfo;

static UTF8 screenshots[ 1024 ][ MAX_QPATH ];
static S32 current_screen = 0, maxscreens = 0;

static StringEntry MonthAbbrev[ ] =
{
    "Jan", "Feb", "Mar",
    "Apr", "May", "Jun",
    "Jul", "Aug", "Sep",
    "Oct", "Nov", "Dec"
};

static StringEntry netSources[ ] =
{
    "LAN",
    "Mplayer",
    "Internet",
    "Favorites"
};

static const S32 numNetSources = sizeof( netSources ) / sizeof( StringEntry );

static StringEntry netnames[ ] =
{
    "???",
    "UDP",
    "IPX",
    NULL
};

/*
================
cvars
================
*/

typedef struct
{
    vmCvar_t*  vmCvar;
    UTF8*    cvarName;
    UTF8*    defaultString;
    S32     cvarFlags;
}

cvarTable_t;

vmCvar_t  ui_browserShowFull;
vmCvar_t  ui_browserShowEmpty;

vmCvar_t  ui_dedicated;
vmCvar_t  ui_netSource;
vmCvar_t  ui_selectedMap;
vmCvar_t  ui_lastServerRefresh_0;
vmCvar_t  ui_lastServerRefresh_1;
vmCvar_t  ui_lastServerRefresh_2;
vmCvar_t  ui_lastServerRefresh_3;
vmCvar_t  ui_lastServerRefresh_0_time;
vmCvar_t  ui_lastServerRefresh_1_time;
vmCvar_t  ui_lastServerRefresh_2_time;
vmCvar_t  ui_lastServerRefresh_3_time;
vmCvar_t  ui_smallFont;
vmCvar_t  ui_bigFont;
vmCvar_t  ui_findPlayer;
vmCvar_t  ui_serverStatusTimeOut;
vmCvar_t  ui_textWrapCache;
vmCvar_t  ui_developer;
vmCvar_t  ui_screen;
vmCvar_t  ui_screens;
vmCvar_t  ui_screenname;

vmCvar_t  ui_winner;

vmCvar_t  ui_emoticons;

vmCvar_t ui_autoredirect;
vmCvar_t ui_profile;

static cvarTable_t    cvarTable[ ] =
{
    { &ui_browserShowFull, "ui_browserShowFull", "1", CVAR_ARCHIVE },
    { &ui_browserShowEmpty, "ui_browserShowEmpty", "1", CVAR_ARCHIVE },
    
    { &ui_dedicated, "ui_dedicated", "0", CVAR_ARCHIVE },
    { &ui_netSource, "ui_netSource", "0", CVAR_ARCHIVE },
    { &ui_selectedMap, "ui_selectedMap", "0", CVAR_ARCHIVE },
    { &ui_lastServerRefresh_0, "ui_lastServerRefresh_0", "", CVAR_ARCHIVE},
    { &ui_lastServerRefresh_1, "ui_lastServerRefresh_1", "", CVAR_ARCHIVE},
    { &ui_lastServerRefresh_2, "ui_lastServerRefresh_2", "", CVAR_ARCHIVE},
    { &ui_lastServerRefresh_3, "ui_lastServerRefresh_3", "", CVAR_ARCHIVE},
    { &ui_lastServerRefresh_0, "ui_lastServerRefresh_0_time", "", CVAR_ARCHIVE},
    { &ui_lastServerRefresh_1, "ui_lastServerRefresh_1_time", "", CVAR_ARCHIVE},
    { &ui_lastServerRefresh_2, "ui_lastServerRefresh_2_time", "", CVAR_ARCHIVE},
    { &ui_lastServerRefresh_3, "ui_lastServerRefresh_3_time", "", CVAR_ARCHIVE},
    { &ui_smallFont, "ui_smallFont", "0.2", CVAR_ARCHIVE},
    { &ui_bigFont, "ui_bigFont", "0.5", CVAR_ARCHIVE},
    { &ui_findPlayer, "ui_findPlayer", "", CVAR_ARCHIVE},
    { &ui_serverStatusTimeOut, "ui_serverStatusTimeOut", "7000", CVAR_ARCHIVE},
    { &ui_textWrapCache, "ui_textWrapCache", "1", CVAR_ARCHIVE },
    { &ui_developer, "ui_developer", "0", CVAR_ARCHIVE | CVAR_CHEAT },
    { &ui_emoticons, "cg_emoticons", "0", CVAR_LATCH | CVAR_ARCHIVE },
    { &ui_screen, "ui_screen", "0", CVAR_ROM },
    { &ui_screens, "ui_screens", "0", CVAR_ROM },
    { &ui_screenname, "ui_screenname", "", CVAR_ROM },
};

static S32    cvarTableSize = sizeof( cvarTable ) / sizeof( cvarTable[0] );
extern itemDef_t* g_bindItem;

idUserInterfaceManagerLocal uiManagerLocal;
idUserInterfaceManager* uiManager = &uiManagerLocal;

void AssetCache( void )
{
    S32 i;
    
    uiInfo.uiDC.Assets.gradientBar = trap_R_RegisterShaderNoMip( ASSET_GRADIENTBAR );
    uiInfo.uiDC.Assets.scrollBar = trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR );
    uiInfo.uiDC.Assets.scrollBarArrowDown = trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWDOWN );
    uiInfo.uiDC.Assets.scrollBarArrowUp = trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWUP );
    uiInfo.uiDC.Assets.scrollBarArrowLeft = trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWLEFT );
    uiInfo.uiDC.Assets.scrollBarArrowRight = trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWRIGHT );
    uiInfo.uiDC.Assets.scrollBarThumb = trap_R_RegisterShaderNoMip( ASSET_SCROLL_THUMB );
    uiInfo.uiDC.Assets.sliderBar = trap_R_RegisterShaderNoMip( ASSET_SLIDER_BAR );
    uiInfo.uiDC.Assets.sliderThumb = trap_R_RegisterShaderNoMip( ASSET_SLIDER_THUMB );
    uiInfo.uiDC.Assets.cornerIn = trap_R_RegisterShaderNoMip( ASSET_CORNERIN );
    uiInfo.uiDC.Assets.cornerOut = trap_R_RegisterShaderNoMip( ASSET_CORNEROUT );
    
    if( ui_emoticons.integer )
    {
        //uiInfo.uiDC.Assets.emoticonCount = bggame->LoadEmoticons( uiInfo.uiDC.Assets.emoticons, uiInfo.uiDC.Assets.emoticonWidths );
    }
    else
        uiInfo.uiDC.Assets.emoticonCount = 0;
        
    for( i = 0; i < uiInfo.uiDC.Assets.emoticonCount; i++ )
    {
        uiInfo.uiDC.Assets.emoticonShaders[ i ] = trap_R_RegisterShaderNoMip(
                    va( "emoticons/%s_%dx1.tga", uiInfo.uiDC.Assets.emoticons[ i ],
                        uiInfo.uiDC.Assets.emoticonWidths[ i ] ) );
    }
}

void UI_DrawSides( F32 x, F32 y, F32 w, F32 h, F32 size )
{
    UI_AdjustFrom640( &x, &y, &w, &h );
    size *= uiInfo.uiDC.xscale;
    trap_R_DrawStretchPic( x, y, size, h, 0, 0, 0, 0, uiInfo.uiDC.whiteShader );
    trap_R_DrawStretchPic( x + w - size, y, size, h, 0, 0, 0, 0, uiInfo.uiDC.whiteShader );
}

void UI_DrawTopBottom( F32 x, F32 y, F32 w, F32 h, F32 size )
{
    UI_AdjustFrom640( &x, &y, &w, &h );
    size *= uiInfo.uiDC.yscale;
    trap_R_DrawStretchPic( x, y, w, size, 0, 0, 0, 0, uiInfo.uiDC.whiteShader );
    trap_R_DrawStretchPic( x, y + h - size, w, size, 0, 0, 0, 0, uiInfo.uiDC.whiteShader );
}

static void UI_DrawCorners( F32 x, F32 y, F32 w, F32 h, F32 size, qhandle_t pic )
{
    F32 hs, vs;
    UI_AdjustFrom640( &x, &y, &w, &h );
    hs = size * uiInfo.uiDC.xscale;
    vs = size * uiInfo.uiDC.yscale;
    
    trap_R_DrawStretchPic( x, y, hs, vs, 0, 0, 0.5, 0.5, pic );
    trap_R_DrawStretchPic( x, y + h - vs, hs, vs, 0, 0.5, 0.5, 1, pic );
    trap_R_DrawStretchPic( x + w - hs, y, hs, vs, 0.5, 0, 1, 0.5, pic );
    trap_R_DrawStretchPic( x + w - hs, y + h - vs, hs, vs, 0.5, 0.5, 1, 1, pic );
}

/*
================
UI_DrawRect

Coordinates are 640*480 virtual values
=================
*/
void UI_DrawRect( F32 x, F32 y, F32 width, F32 height, F32 size, const F32* color )
{
    trap_R_SetColor( color );
    
    UI_DrawTopBottom( x, y, width, height, size );
    UI_DrawSides( x, y, width, height, size );
    
    trap_R_SetColor( NULL );
}

/*
================
UI_DrawRoundedRect

Coordinates are 640*480 virtual values
=================
*/
void UI_DrawRoundedRect( F32 x, F32 y, F32 width, F32 height, F32 size, const F32* color )
{
    trap_R_SetColor( color );
    
    UI_DrawTopBottom( x + size * 4, y, width - size * 8, height, size );
    UI_DrawSides( x, y + size * 4, width, height - size * 8, size );
    UI_DrawCorners( x, y, width, height, size * 4, uiInfo.uiDC.Assets.cornerOut );
    
    trap_R_SetColor( NULL );
}

/*
================
UI_FillRect

Coordinates are 640*480 virtual values
=================
*/
void UI_FillRect( F32 x, F32 y, F32 width, F32 height, const F32* color )
{
    trap_R_SetColor( color );
    
    UI_AdjustFrom640( &x, &y, &width, &height );
    trap_R_DrawStretchPic( x, y, width, height, 0, 0, 0, 0, uiInfo.uiDC.whiteShader );
    
    trap_R_SetColor( NULL );
}

/*
================
UI_FillRoundedRect

Coordinates are 640*480 virtual values
=================
*/
void UI_FillRoundedRect( F32 x, F32 y, F32 width, F32 height, F32 size, const F32* color )
{
    UI_FillRect( x, y + size * 3, width, height - size * 6, color );
    
    trap_R_SetColor( color );
    
    UI_DrawTopBottom( x + size * 3, y, width - size * 6, height, size * 3 );
    UI_DrawCorners( x - size, y - size, width + size * 2, height + size * 2, size * 4, uiInfo.uiDC.Assets.cornerIn );
    
    trap_R_SetColor( NULL );
}

/*
==================
UI_InsertServerIntoDisplayList
==================
*/
static void UI_InsertServerIntoDisplayList( S32 num, S32 position )
{
    S32 i;
    
    if( position < 0 || position > uiInfo.serverStatus.numDisplayServers )
        return;
        
    //
    uiInfo.serverStatus.numDisplayServers++;
    
    for( i = uiInfo.serverStatus.numDisplayServers; i > position; i-- )
        uiInfo.serverStatus.displayServers[i] = uiInfo.serverStatus.displayServers[i - 1];
        
    uiInfo.serverStatus.displayServers[position] = num;
}

/*
==================
UI_RemoveServerFromDisplayList
==================
*/
static void UI_RemoveServerFromDisplayList( S32 num )
{
    S32 i, j;
    
    for( i = 0; i < uiInfo.serverStatus.numDisplayServers; i++ )
    {
        if( uiInfo.serverStatus.displayServers[i] == num )
        {
            uiInfo.serverStatus.numDisplayServers--;
            
            for( j = i; j < uiInfo.serverStatus.numDisplayServers; j++ )
                uiInfo.serverStatus.displayServers[j] = uiInfo.serverStatus.displayServers[j + 1];
                
            return;
        }
    }
}

/*
==================
UI_BinaryServerInsertion
==================
*/
static void UI_BinaryServerInsertion( S32 num )
{
    S32 mid, offset, res, len;
    
    // use binary search to insert server
    len = uiInfo.serverStatus.numDisplayServers;
    mid = len;
    offset = 0;
    res = 0;
    
    while( mid > 0 )
    {
        mid = len >> 1;
        //
        res = trap_LAN_CompareServers( ui_netSource.integer, uiInfo.serverStatus.sortKey,
                                       uiInfo.serverStatus.sortDir, num, uiInfo.serverStatus.displayServers[offset + mid] );
        // if equal
        
        if( res == 0 )
        {
            UI_InsertServerIntoDisplayList( num, offset + mid );
            return;
        }
        
        // if larger
        else if( res == 1 )
        {
            offset += mid;
            len -= mid;
        }
        
        // if smaller
        else
            len -= mid;
    }
    
    if( res == 1 )
        offset++;
        
    UI_InsertServerIntoDisplayList( num, offset );
}

typedef struct
{
    UTF8* name, *altName;
}

serverStatusCvar_t;

serverStatusCvar_t serverStatusCvars[] =
{
    {"sv_hostname", "Name"},
    {"Address", ""},
    {"gamename", "Game name"},
    {"mapname", "Map"},
    {"version", ""},
    {"protocol", ""},
    {"timelimit", ""},
    {NULL, NULL}
};

/*
==================
UI_SortServerStatusInfo
==================
*/
static void UI_SortServerStatusInfo( serverStatusInfo_t* info )
{
    S32 i, j, index;
    UTF8* tmp1, *tmp2;
    
    index = 0;
    
    for( i = 0; serverStatusCvars[i].name; i++ )
    {
        for( j = 0; j < info->numLines; j++ )
        {
            if( !info->lines[j][1] || info->lines[j][1][0] )
                continue;
                
            if( !Q_stricmp( serverStatusCvars[i].name, info->lines[j][0] ) )
            {
                // swap lines
                tmp1 = info->lines[index][0];
                tmp2 = info->lines[index][3];
                info->lines[index][0] = info->lines[j][0];
                info->lines[index][3] = info->lines[j][3];
                info->lines[j][0] = tmp1;
                info->lines[j][3] = tmp2;
                //
                
                if( strlen( serverStatusCvars[i].altName ) )
                    info->lines[index][0] = serverStatusCvars[i].altName;
                    
                index++;
            }
        }
    }
}

/*
==================
UI_GetServerStatusInfo
==================
*/
static S32 UI_GetServerStatusInfo( UTF8* serverAddress, serverStatusInfo_t* info )
{
    UTF8* p, *score, *ping, *name;
    S32 i, len;
    
    if( !info )
    {
        trap_LAN_ServerStatus( serverAddress, NULL, 0 );
        return false;
    }
    
    memset( info, 0, sizeof( *info ) );
    
    if( trap_LAN_ServerStatus( serverAddress, info->text, sizeof( info->text ) ) )
    {
        Q_strncpyz( info->address, serverAddress, sizeof( info->address ) );
        p = info->text;
        info->numLines = 0;
        info->lines[info->numLines][0] = "Address";
        info->lines[info->numLines][1] = "";
        info->lines[info->numLines][2] = "";
        info->lines[info->numLines][3] = info->address;
        info->numLines++;
        // get the cvars
        
        while( p && *p )
        {
            p = strchr( p, '\\' );
            
            if( !p ) break;
            
            *p++ = '\0';
            
            if( *p == '\\' )
                break;
                
            info->lines[info->numLines][0] = p;
            info->lines[info->numLines][1] = "";
            info->lines[info->numLines][2] = "";
            
            p = strchr( p, '\\' );
            
            if( !p ) break;
            
            *p++ = '\0';
            
            info->lines[info->numLines][3] = p;
            info->numLines++;
            
            if( info->numLines >= MAX_SERVERSTATUS_LINES )
                break;
        }
        
        // get the player list
        if( info->numLines < MAX_SERVERSTATUS_LINES - 3 )
        {
            // empty line
            info->lines[info->numLines][0] = "";
            info->lines[info->numLines][1] = "";
            info->lines[info->numLines][2] = "";
            info->lines[info->numLines][3] = "";
            info->numLines++;
            // header
            info->lines[info->numLines][0] = "num";
            info->lines[info->numLines][1] = "score";
            info->lines[info->numLines][2] = "ping";
            info->lines[info->numLines][3] = "name";
            info->numLines++;
            // parse players
            i = 0;
            len = 0;
            
            while( p && *p )
            {
                if( *p == '\\' )
                    *p++ = '\0';
                    
                if( !p )
                    break;
                    
                score = p;
                
                p = strchr( p, ' ' );
                
                if( !p )
                    break;
                    
                *p++ = '\0';
                
                ping = p;
                
                p = strchr( p, ' ' );
                
                if( !p )
                    break;
                    
                *p++ = '\0';
                
                name = p;
                
                Com_sprintf( &info->pings[len], sizeof( info->pings ) - len, "%d", i );
                
                info->lines[info->numLines][0] = &info->pings[len];
                
                len += strlen( &info->pings[len] ) + 1;
                
                info->lines[info->numLines][1] = score;
                info->lines[info->numLines][2] = ping;
                info->lines[info->numLines][3] = name;
                info->numLines++;
                
                if( info->numLines >= MAX_SERVERSTATUS_LINES )
                    break;
                    
                p = strchr( p, '\\' );
                
                if( !p )
                    break;
                    
                *p++ = '\0';
                
                //
                i++;
            }
        }
        
        UI_SortServerStatusInfo( info );
        return true;
    }
    
    return false;
}

/*
==================
stristr
==================
*/
static UTF8* stristr( UTF8* str, UTF8* charset )
{
    S32 i;
    
    while( *str )
    {
        for( i = 0; charset[i] && str[i]; i++ )
            if( toupper( charset[i] ) != toupper( str[i] ) ) break;
            
        if( !charset[i] ) return str;
        
        str++;
    }
    
    return NULL;
}

/*
==================
UI_BuildFindPlayerList
==================
*/
static void UI_FeederSelection( F32 feederID, S32 index );

static void UI_BuildFindPlayerList( bool force )
{
    static S32 numFound, numTimeOuts;
    S32 i, j, k, resend;
    serverStatusInfo_t info;
    UTF8 name[MAX_NAME_LENGTH + 2];
    UTF8 infoString[MAX_STRING_CHARS];
    bool duplicate;
    
    if( !force )
    {
        if( !uiInfo.nextFindPlayerRefresh || uiInfo.nextFindPlayerRefresh > uiInfo.uiDC.realTime )
            return;
    }
    else
    {
        memset( &uiInfo.pendingServerStatus, 0, sizeof( uiInfo.pendingServerStatus ) );
        uiInfo.numFoundPlayerServers = 0;
        uiInfo.currentFoundPlayerServer = 0;
        trap_Cvar_VariableStringBuffer( "ui_findPlayer", uiInfo.findPlayerName, sizeof( uiInfo.findPlayerName ) );
        Q_CleanStr( uiInfo.findPlayerName );
        // should have a string of some length
        
        if( !strlen( uiInfo.findPlayerName ) )
        {
            uiInfo.nextFindPlayerRefresh = 0;
            return;
        }
        
        // set resend time
        resend = ui_serverStatusTimeOut.integer / 2 - 10;
        
        if( resend < 50 )
            resend = 50;
            
        trap_Cvar_Set( "cl_serverStatusResendTime", va( "%d", resend ) );
        // reset all server status requests
        trap_LAN_ServerStatus( NULL, NULL, 0 );
        //
        uiInfo.numFoundPlayerServers = 1;
        Com_sprintf( uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers - 1],
                     sizeof( uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers - 1] ),
                     "searching %d...", uiInfo.pendingServerStatus.num );
        numFound = 0;
        numTimeOuts++;
    }
    
    for( i = 0; i < MAX_SERVERSTATUSREQUESTS; i++ )
    {
        // if this pending server is valid
        
        if( uiInfo.pendingServerStatus.server[i].valid )
        {
            // try to get the server status for this server
            
            if( UI_GetServerStatusInfo( uiInfo.pendingServerStatus.server[i].adrstr, &info ) )
            {
                //
                numFound++;
                // parse through the server status lines
                
                for( j = 0; j < info.numLines; j++ )
                {
                    // should have ping info
                    
                    if( !info.lines[j][2] || !info.lines[j][2][0] )
                        continue;
                        
                    // clean string first
                    Q_strncpyz( name, info.lines[j][3], sizeof( name ) );
                    
                    Q_CleanStr( name );
                    
                    duplicate = false;
                    
                    for( k = 0; k < uiInfo.numFoundPlayerServers - 1; k++ )
                    {
                        if( Q_strncmp( uiInfo.foundPlayerServerAddresses[ k ],
                                       uiInfo.pendingServerStatus.server[ i ].adrstr,
                                       MAX_ADDRESSLENGTH ) == 0 )
                            duplicate = true;
                    }
                    
                    // if the player name is a substring
                    if( stristr( name, uiInfo.findPlayerName ) && !duplicate )
                    {
                        // add to found server list if we have space (always leave space for a line with the number found)
                        
                        if( uiInfo.numFoundPlayerServers < MAX_FOUNDPLAYER_SERVERS - 1 )
                        {
                            //
                            Q_strncpyz( uiInfo.foundPlayerServerAddresses[uiInfo.numFoundPlayerServers - 1],
                                        uiInfo.pendingServerStatus.server[i].adrstr,
                                        sizeof( uiInfo.foundPlayerServerAddresses[0] ) );
                            Q_strncpyz( uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers - 1],
                                        uiInfo.pendingServerStatus.server[i].name,
                                        sizeof( uiInfo.foundPlayerServerNames[0] ) );
                            uiInfo.numFoundPlayerServers++;
                        }
                        else
                        {
                            // can't add any more so we're done
                            uiInfo.pendingServerStatus.num = uiInfo.serverStatus.numDisplayServers;
                        }
                    }
                }
                
                Com_sprintf( uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers - 1],
                             sizeof( uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers - 1] ),
                             "searching %d/%d...", numFound, uiInfo.pendingServerStatus.num );
                // retrieved the server status so reuse this spot
                uiInfo.pendingServerStatus.server[i].valid = false;
            }
        }
        
        // if empty pending slot or timed out
        if( !uiInfo.pendingServerStatus.server[i].valid ||
                uiInfo.pendingServerStatus.server[i].startTime < uiInfo.uiDC.realTime - ui_serverStatusTimeOut.integer )
        {
            if( uiInfo.pendingServerStatus.server[i].valid )
                numTimeOuts++;
                
            // reset server status request for this address
            UI_GetServerStatusInfo( uiInfo.pendingServerStatus.server[i].adrstr, NULL );
            
            // reuse pending slot
            uiInfo.pendingServerStatus.server[i].valid = false;
            
            // if we didn't try to get the status of all servers in the main browser yet
            if( uiInfo.pendingServerStatus.num < uiInfo.serverStatus.numDisplayServers )
            {
                uiInfo.pendingServerStatus.server[i].startTime = uiInfo.uiDC.realTime;
                trap_LAN_GetServerAddressString( ui_netSource.integer,
                                                 uiInfo.serverStatus.displayServers[uiInfo.pendingServerStatus.num],
                                                 uiInfo.pendingServerStatus.server[i].adrstr,
                                                 sizeof( uiInfo.pendingServerStatus.server[i].adrstr ) );
                                                 
                trap_LAN_GetServerInfo( ui_netSource.integer,
                                        uiInfo.serverStatus.displayServers[uiInfo.pendingServerStatus.num],
                                        infoString, sizeof( infoString ) );
                                        
                Q_strncpyz( uiInfo.pendingServerStatus.server[i].name,
                            Info_ValueForKey( infoString, "hostname" ),
                            sizeof( uiInfo.pendingServerStatus.server[0].name ) );
                            
                uiInfo.pendingServerStatus.server[i].valid = true;
                uiInfo.pendingServerStatus.num++;
                Com_sprintf( uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers - 1],
                             sizeof( uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers - 1] ),
                             "searching %d/%d...", numFound, uiInfo.pendingServerStatus.num );
            }
        }
    }
    
    for( i = 0; i < MAX_SERVERSTATUSREQUESTS; i++ )
    {
        if( uiInfo.pendingServerStatus.server[i].valid )
            break;
    }
    
    // if still trying to retrieve server status info
    if( i < MAX_SERVERSTATUSREQUESTS )
        uiInfo.nextFindPlayerRefresh = uiInfo.uiDC.realTime + 25;
    else
    {
        // add a line that shows the number of servers found
        
        if( !uiInfo.numFoundPlayerServers )
        {
            Com_sprintf( uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers - 1],
                         sizeof( uiInfo.foundPlayerServerAddresses[0] ), "no servers found" );
        }
        else
        {
            Com_sprintf( uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers - 1],
                         sizeof( uiInfo.foundPlayerServerAddresses[0] ),
                         "%d server%s found with player %s", uiInfo.numFoundPlayerServers - 1,
                         uiInfo.numFoundPlayerServers == 2 ? "" : "s", uiInfo.findPlayerName );
        }
        
        uiInfo.nextFindPlayerRefresh = 0;
        // show the server status info for the selected server
        UI_FeederSelection( FEEDER_FINDPLAYER, uiInfo.currentFoundPlayerServer );
    }
}

/*
==================
UI_BuildServerStatus
==================
*/
static void UI_BuildServerStatus( bool force )
{
    if( uiInfo.nextFindPlayerRefresh )
        return;
        
    if( !force )
    {
        if( !uiInfo.nextServerStatusRefresh || uiInfo.nextServerStatusRefresh > uiInfo.uiDC.realTime )
            return;
    }
    else
    {
        Menu_SetFeederSelection( NULL, FEEDER_SERVERSTATUS, 0, NULL );
        uiInfo.serverStatusInfo.numLines = 0;
        // reset all server status requests
        trap_LAN_ServerStatus( NULL, NULL, 0 );
    }
    
    if( uiInfo.serverStatus.currentServer < 0 ||
            uiInfo.serverStatus.currentServer > uiInfo.serverStatus.numDisplayServers ||
            uiInfo.serverStatus.numDisplayServers == 0 )
        return;
        
    if( UI_GetServerStatusInfo( uiInfo.serverStatusAddress, &uiInfo.serverStatusInfo ) )
    {
        uiInfo.nextServerStatusRefresh = 0;
        UI_GetServerStatusInfo( uiInfo.serverStatusAddress, NULL );
    }
    else
        uiInfo.nextServerStatusRefresh = uiInfo.uiDC.realTime + 500;
}

/*
==================
UI_BuildServerDisplayList
==================
*/
static void UI_BuildServerDisplayList( bool force )
{
    S32 i, count, clients, maxClients, ping, len, visible;
    UTF8 info[MAX_STRING_CHARS];
    static S32 numinvisible;
    
    if( !( force || uiInfo.uiDC.realTime > uiInfo.serverStatus.nextDisplayRefresh ) )
        return;
        
    // if we shouldn't reset
    if( force == 2 )
        force = 0;
        
    // do motd updates here too
    trap_Cvar_VariableStringBuffer( "cl_motdString", uiInfo.serverStatus.motd, sizeof( uiInfo.serverStatus.motd ) );
    
    len = strlen( uiInfo.serverStatus.motd );
    
    if( len != uiInfo.serverStatus.motdLen )
    {
        uiInfo.serverStatus.motdLen = len;
        uiInfo.serverStatus.motdWidth = -1;
    }
    
    if( force )
    {
        numinvisible = 0;
        // clear number of displayed servers
        uiInfo.serverStatus.numDisplayServers = 0;
        uiInfo.serverStatus.numPlayersOnServers = 0;
        // set list box index to zero
        Menu_SetFeederSelection( NULL, FEEDER_SERVERS, 0, NULL );
        // mark all servers as visible so we store ping updates for them
        trap_LAN_MarkServerVisible( ui_netSource.integer, -1, true );
    }
    
    // get the server count (comes from the master)
    count = trap_LAN_GetServerCount( ui_netSource.integer );
    
    if( count == -1 || ( ui_netSource.integer == AS_LOCAL && count == 0 ) )
    {
        // still waiting on a response from the master
        uiInfo.serverStatus.numDisplayServers = 0;
        uiInfo.serverStatus.numPlayersOnServers = 0;
        uiInfo.serverStatus.nextDisplayRefresh = uiInfo.uiDC.realTime + 500;
        return;
    }
    
    visible = false;
    
    for( i = 0; i < count; i++ )
    {
        // if we already got info for this server
        
        if( !trap_LAN_ServerIsVisible( ui_netSource.integer, i ) )
            continue;
            
        visible = true;
        // get the ping for this server
        ping = trap_LAN_GetServerPing( ui_netSource.integer, i );
        
        if( ping > 0 || ui_netSource.integer == AS_FAVORITES )
        {
            trap_LAN_GetServerInfo( ui_netSource.integer, i, info, MAX_STRING_CHARS );
            
            clients = atoi( Info_ValueForKey( info, "clients" ) );
            uiInfo.serverStatus.numPlayersOnServers += clients;
            
            if( ui_browserShowEmpty.integer == 0 )
            {
                if( clients == 0 )
                {
                    trap_LAN_MarkServerVisible( ui_netSource.integer, i, false );
                    continue;
                }
            }
            
            if( ui_browserShowFull.integer == 0 )
            {
                maxClients = atoi( Info_ValueForKey( info, "sv_maxclients" ) );
                
                if( clients == maxClients )
                {
                    trap_LAN_MarkServerVisible( ui_netSource.integer, i, false );
                    continue;
                }
            }
            
            // make sure we never add a favorite server twice
            if( ui_netSource.integer == AS_FAVORITES )
                UI_RemoveServerFromDisplayList( i );
                
            // insert the server into the list
            UI_BinaryServerInsertion( i );
            
            // done with this server
            if( ping > 0 )
            {
                trap_LAN_MarkServerVisible( ui_netSource.integer, i, false );
                numinvisible++;
            }
        }
    }
    
    uiInfo.serverStatus.refreshtime = uiInfo.uiDC.realTime;
    
    // if there were no servers visible for ping updates
    
    if( !visible )
    {
//    UI_StopServerRefresh();
//    uiInfo.serverStatus.nextDisplayRefresh = 0;
    }
}


/*
=================
UI_StopServerRefresh
=================
*/
static void UI_StopServerRefresh( void )
{
    S32 count;
    
    if( !uiInfo.serverStatus.refreshActive )
    {
        // not currently refreshing
        return;
    }
    
    uiInfo.serverStatus.refreshActive = false;
    /*Com_Printf( "%d servers listed in browser with %d players.\n",
                uiInfo.serverStatus.numDisplayServers,
                uiInfo.serverStatus.numPlayersOnServers );*/
    count = trap_LAN_GetServerCount( ui_netSource.integer );
    
    if( count - uiInfo.serverStatus.numDisplayServers > 0 )
    {
        /*Com_Printf( "%d servers not listed due to packet loss or pings higher than %d\n",
                    count - uiInfo.serverStatus.numDisplayServers,
                    ( S32 ) trap_Cvar_VariableValue( "cl_maxPing" ) );*/
    }
    
}

/*
=================
UI_DoServerRefresh
=================
*/
static void UI_DoServerRefresh( void )
{
    bool wait = false;
    
    if( !uiInfo.serverStatus.refreshActive )
        return;
        
    if( ui_netSource.integer != AS_FAVORITES )
    {
        if( ui_netSource.integer == AS_LOCAL )
        {
            if( !trap_LAN_GetServerCount( ui_netSource.integer ) )
                wait = true;
        }
        else
        {
            if( trap_LAN_GetServerCount( ui_netSource.integer ) < 0 )
                wait = true;
        }
    }
    
    if( uiInfo.uiDC.realTime < uiInfo.serverStatus.refreshtime )
    {
        if( wait )
            return;
    }
    
    // if still trying to retrieve pings
    if( trap_LAN_UpdateVisiblePings( ui_netSource.integer ) )
        uiInfo.serverStatus.refreshtime = uiInfo.uiDC.realTime + 1000;
    else if( !wait )
    {
        // get the last servers in the list
        UI_BuildServerDisplayList( 2 );
        // stop the refresh
        UI_StopServerRefresh();
    }
    
    //
    UI_BuildServerDisplayList( false );
}

/*
=================
UI_UpdatePendingPings
=================
*/
static void UI_UpdatePendingPings( void )
{
    trap_LAN_ResetPings( ui_netSource.integer );
    uiInfo.serverStatus.refreshActive = true;
    uiInfo.serverStatus.refreshtime = uiInfo.uiDC.realTime + 1000;
    
}

/*
=================
UI_StartServerRefresh
=================
*/
static void UI_StartServerRefresh( bool full )
{
    S32   i;
    UTF8*  ptr;
    S32   time;
    qtime_t q;
    
    time = trap_RealTime( &q );
    trap_Cvar_Set( va( "ui_lastServerRefresh_%i_time", ui_netSource.integer ),
                   va( "%i", time ) );
    trap_Cvar_Set( va( "ui_lastServerRefresh_%i", ui_netSource.integer ),
                   va( "%s-%i, %i at %i:%02i", MonthAbbrev[q.tm_mon],
                       q.tm_mday, 1900 + q.tm_year, q.tm_hour, q.tm_min ) );
                       
    if( !full )
    {
        UI_UpdatePendingPings();
        return;
    }
    
    uiInfo.serverStatus.refreshActive = true;
    uiInfo.serverStatus.nextDisplayRefresh = uiInfo.uiDC.realTime + 1000;
    // clear number of displayed servers
    uiInfo.serverStatus.numDisplayServers = 0;
    uiInfo.serverStatus.numPlayersOnServers = 0;
    // mark all servers as visible so we store ping updates for them
    trap_LAN_MarkServerVisible( ui_netSource.integer, -1, true );
    // reset all the pings
    trap_LAN_ResetPings( ui_netSource.integer );
    //
    
    if( ui_netSource.integer == AS_LOCAL )
    {
        trap_Cmd_ExecuteText( EXEC_APPEND, "localservers\n" );
        uiInfo.serverStatus.refreshtime = uiInfo.uiDC.realTime + 1000;
        return;
    }
    
    uiInfo.serverStatus.refreshtime = uiInfo.uiDC.realTime + 5000;
    
    if( ui_netSource.integer == AS_GLOBAL )
    {
        if( ui_netSource.integer == AS_GLOBAL )
            i = 0;
        else
            i = 1;
            
        ptr = UI_Cvar_VariableString( "debug_protocol" );
        
        if( strlen( ptr ) )
            trap_Cmd_ExecuteText( EXEC_APPEND, va( "globalservers %d %s full empty\n", i, ptr ) );
        else
        {
            trap_Cmd_ExecuteText( EXEC_APPEND, va( "globalservers %d %d full empty\n", i,
                                                   ( S32 )trap_Cvar_VariableValue( "protocol" ) ) );
        }
    }
}

S32 frameCount = 0;
S32 startTime;

#define UI_FPS_FRAMES 4
void idUserInterfaceManagerLocal::Refresh( S32 realtime )
{
    static S32 index;
    static S32  previousTimes[UI_FPS_FRAMES];
    
    //if( !( trap_Key_GetCatcher() & KEYCATCH_UI ) ) {
    //  return;
    //}
    
    uiInfo.uiDC.frameTime = realtime - uiInfo.uiDC.realTime;
    uiInfo.uiDC.realTime = realtime;
    
    previousTimes[index % UI_FPS_FRAMES] = uiInfo.uiDC.frameTime;
    index++;
    
    if( index > UI_FPS_FRAMES )
    {
        S32 i, total;
        // average multiple frames together to smooth changes out a bit
        total = 0;
        
        for( i = 0 ; i < UI_FPS_FRAMES ; i++ )
            total += previousTimes[i];
            
        if( !total )
            total = 1;
            
        uiInfo.uiDC.FPS = 1000 * UI_FPS_FRAMES / total;
    }
    
    UI_UpdateCvars();
    
    if( Menu_Count() > 0 )
    {
        // paint all the menus
        Menu_PaintAll();
        // refresh server browser list
        UI_DoServerRefresh();
        // refresh server status
        UI_BuildServerStatus( false );
        // refresh find player list
        UI_BuildFindPlayerList( false );
    }
    
    // draw cursor
    UI_SetColor( NULL );
    
    if( Menu_Count() > 0 ) // && !trap_Cvar_VariableValue("ui_hideCursor") )
    {
        UI_DrawHandlePic( uiInfo.uiDC.cursorx - ( 16.0f * uiInfo.uiDC.aspectScale ), uiInfo.uiDC.cursory - 16.0f,
                          32.0f * uiInfo.uiDC.aspectScale, 32.0f, uiInfo.uiDC.Assets.cursor );
    }
}

/*
=================
idUserInterfaceManagerLocal::Shutdown
=================
*/
void idUserInterfaceManagerLocal::Shutdown( void )
{
    trap_LAN_SaveCachedServers();
}

bool Asset_Parse( S32 handle )
{
    pc_token_t token;
    StringEntry tempStr;
    
    if( !trap_PC_ReadToken( handle, &token ) )
        return false;
        
    if( Q_stricmp( token.string, "{" ) != 0 )
        return false;
        
    while( 1 )
    {
        memset( &token, 0, sizeof( pc_token_t ) );
        
        if( !trap_PC_ReadToken( handle, &token ) )
            return false;
            
        if( Q_stricmp( token.string, "}" ) == 0 )
            return true;
            
        // font
        if( Q_stricmp( token.string, "font" ) == 0 )
        {
            S32 pointSize;
            
            if( !PC_String_Parse( handle, &tempStr ) || !PC_Int_Parse( handle, &pointSize ) )
                return false;
                
            trap_R_RegisterFont( tempStr, pointSize, &uiInfo.uiDC.Assets.textFont );
            uiInfo.uiDC.Assets.fontRegistered = true;
            continue;
        }
        
        if( Q_stricmp( token.string, "smallFont" ) == 0 )
        {
            S32 pointSize;
            
            if( !PC_String_Parse( handle, &tempStr ) || !PC_Int_Parse( handle, &pointSize ) )
                return false;
                
            trap_R_RegisterFont( tempStr, pointSize, &uiInfo.uiDC.Assets.smallFont );
            continue;
        }
        
        if( Q_stricmp( token.string, "bigFont" ) == 0 )
        {
            S32 pointSize;
            
            if( !PC_String_Parse( handle, &tempStr ) || !PC_Int_Parse( handle, &pointSize ) )
                return false;
                
            trap_R_RegisterFont( tempStr, pointSize, &uiInfo.uiDC.Assets.bigFont );
            continue;
        }
        
        
        // gradientbar
        if( Q_stricmp( token.string, "gradientbar" ) == 0 )
        {
            if( !PC_String_Parse( handle, &tempStr ) )
                return false;
                
            uiInfo.uiDC.Assets.gradientBar = trap_R_RegisterShaderNoMip( tempStr );
            continue;
        }
        
        // enterMenuSound
        if( Q_stricmp( token.string, "menuEnterSound" ) == 0 )
        {
            if( !PC_String_Parse( handle, &tempStr ) )
                return false;
                
            uiInfo.uiDC.Assets.menuEnterSound = trap_S_RegisterSound( tempStr );
            continue;
        }
        
        // exitMenuSound
        if( Q_stricmp( token.string, "menuExitSound" ) == 0 )
        {
            if( !PC_String_Parse( handle, &tempStr ) )
                return false;
                
            uiInfo.uiDC.Assets.menuExitSound = trap_S_RegisterSound( tempStr );
            continue;
        }
        
        // itemFocusSound
        if( Q_stricmp( token.string, "itemFocusSound" ) == 0 )
        {
            if( !PC_String_Parse( handle, &tempStr ) )
                return false;
                
            uiInfo.uiDC.Assets.itemFocusSound = trap_S_RegisterSound( tempStr );
            continue;
        }
        
        // menuBuzzSound
        if( Q_stricmp( token.string, "menuBuzzSound" ) == 0 )
        {
            if( !PC_String_Parse( handle, &tempStr ) )
                return false;
                
            uiInfo.uiDC.Assets.menuBuzzSound = trap_S_RegisterSound( tempStr );
            continue;
        }
        
        if( Q_stricmp( token.string, "cursor" ) == 0 )
        {
            if( !PC_String_Parse( handle, &uiInfo.uiDC.Assets.cursorStr ) )
                return false;
                
            uiInfo.uiDC.Assets.cursor = trap_R_RegisterShaderNoMip( uiInfo.uiDC.Assets.cursorStr );
            continue;
        }
        
        if( Q_stricmp( token.string, "fadeClamp" ) == 0 )
        {
            if( !PC_Float_Parse( handle, &uiInfo.uiDC.Assets.fadeClamp ) )
                return false;
                
            continue;
        }
        
        if( Q_stricmp( token.string, "fadeCycle" ) == 0 )
        {
            if( !PC_Int_Parse( handle, &uiInfo.uiDC.Assets.fadeCycle ) )
                return false;
                
            continue;
        }
        
        if( Q_stricmp( token.string, "fadeAmount" ) == 0 )
        {
            if( !PC_Float_Parse( handle, &uiInfo.uiDC.Assets.fadeAmount ) )
                return false;
                
            continue;
        }
        
        if( Q_stricmp( token.string, "shadowX" ) == 0 )
        {
            if( !PC_Float_Parse( handle, &uiInfo.uiDC.Assets.shadowX ) )
                return false;
                
            continue;
        }
        
        if( Q_stricmp( token.string, "shadowY" ) == 0 )
        {
            if( !PC_Float_Parse( handle, &uiInfo.uiDC.Assets.shadowY ) )
                return false;
                
            continue;
        }
        
        if( Q_stricmp( token.string, "shadowColor" ) == 0 )
        {
            if( !PC_Color_Parse( handle, &uiInfo.uiDC.Assets.shadowColor ) )
                return false;
                
            uiInfo.uiDC.Assets.shadowFadeClamp = uiInfo.uiDC.Assets.shadowColor[3];
            continue;
        }
        
    }
    
    return false;
}

void UI_Report( void )
{
    String_Report();
}

void UI_ParseMenu( StringEntry menuFile )
{
    S32 handle;
    pc_token_t token;
    
    handle = trap_PC_LoadSource( menuFile );
    
    if( !handle )
        return;
        
    while( 1 )
    {
        memset( &token, 0, sizeof( pc_token_t ) );
        
        if( !trap_PC_ReadToken( handle, &token ) )
            break;
            
        //if( Q_stricmp( token, "{" ) ) {
        //  Com_Printf( "Missing { in menu file\n" );
        //  break;
        //}
        
        //if( menuCount == MAX_MENUS ) {
        //  Com_Printf( "Too many menus!\n" );
        //  break;
        //}
        
        if( token.string[0] == '}' )
            break;
            
        if( Q_stricmp( token.string, "assetGlobalDef" ) == 0 )
        {
            if( Asset_Parse( handle ) )
                continue;
            else
                break;
        }
        
        if( Q_stricmp( token.string, "menudef" ) == 0 )
        {
            // start a new menu
            Menu_New( handle );
        }
    }
    
    trap_PC_FreeSource( handle );
}

bool Load_Menu( S32 handle )
{
    pc_token_t token;
    
    if( !trap_PC_ReadToken( handle, &token ) )
        return false;
        
    if( token.string[0] != '{' )
        return false;
        
    while( 1 )
    {
        if( !trap_PC_ReadToken( handle, &token ) )
            return false;
            
        if( token.string[0] == 0 )
            return false;
            
        if( token.string[0] == '}' )
            return true;
            
        UI_ParseMenu( token.string );
    }
    
    return false;
}

void UI_LoadMenus( StringEntry menuFile, bool reset )
{
    pc_token_t token;
    S32 handle;
    S32 start;
    
    start = trap_Milliseconds();
    
    handle = trap_PC_LoadSource( menuFile );
    
    if( !handle )
        trap_Error( va( S_COLOR_RED "menu list '%s' not found, unable to continue!\n", menuFile ) );
        
    if( reset )
        Menu_Reset();
        
    while( 1 )
    {
        if( !trap_PC_ReadToken( handle, &token ) )
            break;
            
        if( token.string[0] == 0 || token.string[0] == '}' )
            break;
            
        if( token.string[0] == '}' )
            break;
            
        if( Q_stricmp( token.string, "loadmenu" ) == 0 )
        {
            if( Load_Menu( handle ) )
                continue;
            else
                break;
        }
    }
    
    //Com_Printf( "ui menu file '%s' loaded in %d msec\n", menuFile, trap_Milliseconds() - start );
    
    trap_PC_FreeSource( handle );
}

void UI_LoadHelp( StringEntry helpFile )
{
    pc_token_t token;
    S32 handle, start;
    UTF8 title[ 32 ], buffer[ 1024 ];
    
    start = trap_Milliseconds();
    
    handle = trap_PC_LoadSource( helpFile );
    if( !handle )
    {
        Com_Printf( S_COLOR_YELLOW "WARNING: help file '%s' not found!\n",
                    helpFile );
        return;
    }
    
    if( !trap_PC_ReadToken( handle, &token ) ||
            token.string[0] == 0 || token.string[0] != '{' )
    {
        Com_Printf( S_COLOR_YELLOW "WARNING: help file '%s' does not start with "
                    "'{'\n", helpFile );
        return;
    }
    
    uiInfo.helpCount = 0;
    title[ 0 ] = 0;
    while( 1 )
    {
        if( !trap_PC_ReadToken( handle, &token ) ||
                token.string[0] == 0 || token.string[0] == '}' )
            break;
            
        if( token.string[0] == '{' )
        {
            buffer[ 0 ] = 0;
            Q_strcat( buffer, sizeof( buffer ), title );
            Q_strcat( buffer, sizeof( buffer ), "\n\n" );
            while( 1 )
            {
                if( !trap_PC_ReadToken( handle, &token ) ||
                        token.string[0] == 0 || token.string[0] == '}' )
                    break;
                Q_strcat( buffer, sizeof( buffer ), token.string );
            }
            uiInfo.helpList[ uiInfo.helpCount ].text = String_Alloc( title );
            uiInfo.helpList[ uiInfo.helpCount ].v.text = String_Alloc( buffer );
            uiInfo.helpList[ uiInfo.helpCount ].type = INFOTYPE_TEXT;
            uiInfo.helpCount++;
            title[ 0 ] = 0;
        }
        else
            Q_strcat( title, sizeof( title ), token.string );
    }
    
    trap_PC_FreeSource( handle );
    
    /*Com_Printf( "ui help file '%s' loaded in %d msec (%d infopanes)\n",
                helpFile, trap_Milliseconds() - start, uiInfo.helpCount );*/
}

void UI_Load( void )
{
    UTF8 lastName[1024];
    menuDef_t* menu = Menu_GetFocused();
    
    if( menu && menu->window.name )
        strcpy( lastName, menu->window.name );
        
    String_Init();
    
    UI_LoadMenus( "gui/menus.txt", true );
    UI_LoadMenus( "gui/ingame.txt", false );
    UI_LoadMenus( "gui/tremulous.txt", false );
    UI_LoadHelp( "gui/help.txt" );
    Menus_CloseAll( true );
    Menus_ActivateByName( lastName );
}

/*
===============
UI_GetCurrentStage
===============
*/
static stage_t UI_GetCurrentStage( void )
{
    UTF8    buffer[ MAX_TOKEN_CHARS ];
    stage_t stage;
    
    trap_Cvar_VariableStringBuffer( "ui_stage", buffer, sizeof( buffer ) );
    sscanf( buffer, "%d", ( S32* ) & stage );
    
    return stage;
}

/*
===============
UI_DrawInfoPane
===============
*/
static void UI_DrawInfoPane( menuItem_t* item, rectDef_t* rect, F32 text_x, F32 text_y,
                             F32 scale, S32 textalign, S32 textvalign, vec4_t color, S32 textStyle )
{
    S32         value = 0;
    StringEntry  s = "";
    UTF8*        string = "";
    
    S32         _class, credits;
    UTF8        ui_currentClass[ MAX_STRING_CHARS ];
    
    trap_Cvar_VariableStringBuffer( "ui_currentClass", ui_currentClass, MAX_STRING_CHARS );
    
    sscanf( ui_currentClass, "%d %d", &_class, &credits );
    
    switch( item->type )
    {
        case INFOTYPE_TEXT:
            s = item->v.text;
            break;
            
        case INFOTYPE_CLASS:
            value = ( bggame->ClassCanEvolveFromTo( ( class_t )_class, item->v.pclass, credits,
                                                    UI_GetCurrentStage(), 0 ) +
                      ALIEN_CREDITS_PER_FRAG - 1 ) / ALIEN_CREDITS_PER_FRAG;
                      
            if( value < 1 )
            {
                s = va( "%s\n\n%s",
                        bggame->ClassConfig( item->v.pclass )->humanName,
                        bggame->Class( item->v.pclass )->info );
            }
            else
            {
                s = va( "%s\n\n%s\n\nFrags: %d",
                        bggame->ClassConfig( item->v.pclass )->humanName,
                        bggame->Class( item->v.pclass )->info,
                        value );
            }
            
            break;
            
        case INFOTYPE_WEAPON:
            value = bggame->Weapon( item->v.weapon )->price;
            
            if( value == 0 )
            {
                s = va( "%s\n\n%s\n\nCredits: Free",
                        bggame->Weapon( item->v.weapon )->humanName,
                        bggame->Weapon( item->v.weapon )->info );
            }
            else
            {
                s = va( "%s\n\n%s\n\nCredits: %d",
                        bggame->Weapon( item->v.weapon )->humanName,
                        bggame->Weapon( item->v.weapon )->info,
                        value );
            }
            
            break;
            
        case INFOTYPE_UPGRADE:
            value = bggame->Upgrade( item->v.upgrade )->price;
            
            if( value == 0 )
            {
                s = va( "%s\n\n%s\n\nCredits: Free",
                        bggame->Upgrade( item->v.upgrade )->humanName,
                        bggame->Upgrade( item->v.upgrade )->info );
            }
            else
            {
                s = va( "%s\n\n%s\n\nCredits: %d",
                        bggame->Upgrade( item->v.upgrade )->humanName,
                        bggame->Upgrade( item->v.upgrade )->info,
                        value );
            }
            
            break;
            
        case INFOTYPE_BUILDABLE:
            value = bggame->Buildable( item->v.buildable )->buildPoints;
            
            switch( bggame->Buildable( item->v.buildable )->team )
            {
                case TEAM_ALIENS:
                    string = "Sentience";
                    break;
                    
                case TEAM_HUMANS:
                    string = "Power";
                    break;
                    
                default:
                    break;
            }
            
            if( value == 0 )
            {
                s = va( "%s\n\n%s",
                        bggame->Buildable( item->v.buildable )->humanName,
                        bggame->Buildable( item->v.buildable )->info );
            }
            else
            {
                s = va( "%s\n\n%s\n\n%s: %d",
                        bggame->Buildable( item->v.buildable )->humanName,
                        bggame->Buildable( item->v.buildable )->info,
                        string, value );
            }
            
            break;
    }
    
    UI_DrawTextBlock( rect, text_x, text_y, color, scale,
                      textalign, textvalign, textStyle, s );
}


static void UI_DrawServerMapPreview( rectDef_t* rect, F32 scale, vec4_t color )
{
    if( uiInfo.serverStatus.currentServerCinematic >= 0 )
    {
        trap_CIN_RunCinematic( uiInfo.serverStatus.currentServerCinematic );
        trap_CIN_SetExtents( uiInfo.serverStatus.currentServerCinematic, rect->x, rect->y, rect->w, rect->h );
        trap_CIN_DrawCinematic( uiInfo.serverStatus.currentServerCinematic );
    }
    else if( uiInfo.serverStatus.currentServerPreview > 0 )
        UI_DrawHandlePic( rect->x, rect->y, rect->w, rect->h, uiInfo.serverStatus.currentServerPreview );
    else
        UI_DrawHandlePic( rect->x, rect->y, rect->w, rect->h, trap_R_RegisterShaderNoMip( "gfx/2d/load_screen" ) );
}

static void UI_DrawSelectedMapPreview( rectDef_t* rect, F32 scale, vec4_t color )
{
    S32 map = ui_selectedMap.integer;
    
    if( map < 0 || map > uiInfo.mapCount )
    {
        ui_selectedMap.integer = 0;
        trap_Cvar_Set( "ui_selectedMap", "0" );
        map = 0;
    }
    
    if( uiInfo.mapList[map].cinematic >= -1 )
    {
        if( uiInfo.mapList[map].cinematic == -1 )
            uiInfo.mapList[map].cinematic = trap_CIN_PlayCinematic( va( "%s.roq", uiInfo.mapList[map].mapLoadName ),
                                            0, 0, 0, 0, ( CIN_loop | CIN_silent ) );
                                            
        if( uiInfo.mapList[map].cinematic >= 0 )
        {
            trap_CIN_RunCinematic( uiInfo.mapList[map].cinematic );
            trap_CIN_SetExtents( uiInfo.mapList[map].cinematic, rect->x, rect->y, rect->w, rect->h );
            trap_CIN_DrawCinematic( uiInfo.mapList[map].cinematic );
        }
        else
            uiInfo.mapList[map].cinematic = -2;
    }
    else
    {
        if( uiInfo.mapList[map].levelShot == -1 )
            uiInfo.mapList[map].levelShot = trap_R_RegisterShaderNoMip( uiInfo.mapList[map].imageName );
            
        if( uiInfo.mapList[map].levelShot > 0 )
            UI_DrawHandlePic( rect->x, rect->y, rect->w, rect->h, uiInfo.mapList[map].levelShot );
        else
            UI_DrawHandlePic( rect->x, rect->y, rect->w, rect->h, trap_R_RegisterShaderNoMip( "gfx/2d/load_screen" ) );
    }
}

static void UI_DrawSelectedMapName( rectDef_t* rect, F32 scale, vec4_t color, S32 textStyle )
{
    S32 map = ui_selectedMap.integer;
    
    if( map >= 0 && map < uiInfo.mapCount )
        UI_Text_Paint( rect->x, rect->y, scale, color, uiInfo.mapList[map].mapName, 0, 0, textStyle );
}

static StringEntry UI_OwnerDrawText( S32 ownerDraw )
{
    StringEntry s = NULL;
    
    switch( ownerDraw )
    {
        case UI_NETSOURCE:
            if( ui_netSource.integer < 0 || ui_netSource.integer >= numNetSources )
                ui_netSource.integer = 0;
                
            s = netSources[ui_netSource.integer];
            break;
            
        case UI_KEYBINDSTATUS:
            if( Display_KeyBindPending() )
                s = "Waiting for new key... Press ESCAPE to cancel";
            else
                s = "Press ENTER or CLICK to change, Press BACKSPACE to clear";
                
            break;
            
        case UI_SERVERREFRESHDATE:
            if( uiInfo.serverStatus.refreshActive )
            {
#define MAX_DOTS 5
                S32 numServers = trap_LAN_GetServerCount( ui_netSource.integer );
                S32 numDots = ( uiInfo.uiDC.realTime / 500 ) % ( MAX_DOTS + 1 );
                UTF8 dots[ MAX_DOTS + 1 ];
                S32 i;
                
                for( i = 0; i < numDots; i++ )
                    dots[ i ] = '.';
                    
                dots[ i ] = '\0';
                
                s = numServers < 0 ? va( "Waiting for response%s", dots ) :
                    va( "Getting info for %d servers (ESC to cancel)%s", numServers, dots );
            }
            else
                s = va( "Refresh Time: %s", UI_Cvar_VariableString( va( "ui_lastServerRefresh_%i", ui_netSource.integer ) ) );
                
            break;
            
        case UI_SERVERMOTD:
            s = uiInfo.serverStatus.motd;
            break;
            
        default:
            break;
    }
    
    return s;
}

static S32 UI_OwnerDrawWidth( S32 ownerDraw, F32 scale )
{
    StringEntry s = NULL;
    
    switch( ownerDraw )
    {
        case UI_NETSOURCE:
        case UI_KEYBINDSTATUS:
        case UI_SERVERREFRESHDATE:
        case UI_SERVERMOTD:
            s = UI_OwnerDrawText( ownerDraw );
            break;
            
        default:
            break;
    }
    
    if( s )
        return UI_Text_Width( s, scale, 0 );
        
    return 0;
}

/*
===============
UI_BuildPlayerList
===============
*/
static void UI_BuildPlayerList( void )
{
    uiClientState_t cs;
    S32   n, count, team, team2, playerTeamNumber;
    UTF8  info[MAX_INFO_STRING];
    
    trap_GetClientState( &cs );
    trap_GetConfigString( CS_PLAYERS + cs.clientNum, info, MAX_INFO_STRING );
    uiInfo.playerNumber = cs.clientNum;
    team = atoi( Info_ValueForKey( info, "t" ) );
    trap_GetConfigString( CS_SERVERINFO, info, sizeof( info ) );
    count = atoi( Info_ValueForKey( info, "sv_maxclients" ) );
    uiInfo.playerCount = 0;
    uiInfo.myTeamCount = 0;
    uiInfo.myPlayerIndex = 0;
    playerTeamNumber = 0;
    
    for( n = 0; n < count; n++ )
    {
        trap_GetConfigString( CS_PLAYERS + n, info, MAX_INFO_STRING );
        
        if( info[0] )
        {
            bggame->ClientListParse( &uiInfo.ignoreList[ uiInfo.playerCount ],
                                     Info_ValueForKey( info, "ig" ) );
            Q_strncpyz( uiInfo.rawPlayerNames[uiInfo.playerCount],
                        Info_ValueForKey( info, "n" ), MAX_NAME_LENGTH );
            Q_strncpyz( uiInfo.playerNames[uiInfo.playerCount],
                        Info_ValueForKey( info, "n" ), MAX_NAME_LENGTH );
            Q_CleanStr( uiInfo.playerNames[uiInfo.playerCount] );
            uiInfo.clientNums[uiInfo.playerCount] = n;
            
            if( n == uiInfo.playerNumber )
                uiInfo.myPlayerIndex = uiInfo.playerCount;
                
            uiInfo.playerCount++;
            
            team2 = atoi( Info_ValueForKey( info, "t" ) );
            
            if( team2 == team )
            {
                Q_strncpyz( uiInfo.rawTeamNames[uiInfo.myTeamCount],
                            Info_ValueForKey( info, "n" ), MAX_NAME_LENGTH );
                Q_strncpyz( uiInfo.teamNames[uiInfo.myTeamCount],
                            Info_ValueForKey( info, "n" ), MAX_NAME_LENGTH );
                Q_CleanStr( uiInfo.teamNames[uiInfo.myTeamCount] );
                uiInfo.teamClientNums[uiInfo.myTeamCount] = n;
                
                if( uiInfo.playerNumber == n )
                    playerTeamNumber = uiInfo.myTeamCount;
                    
                uiInfo.myTeamCount++;
            }
        }
    }
}

static void UI_DrawGLInfo( rectDef_t* rect, F32 scale, S32 textalign, S32 textvalign,
                           vec4_t color, S32 textStyle, F32 text_x, F32 text_y )
{
    UTF8      buffer[ 4096 ];
    
    Com_sprintf( buffer, sizeof( buffer ), "VENDOR: %s\nVERSION: %s\n"
                 "PIXELFORMAT: color(%d-bits) Z(%d-bits) stencil(%d-bits)\n%s",
                 uiInfo.uiDC.glconfig.vendor_string, uiInfo.uiDC.glconfig.renderer_string,
                 uiInfo.uiDC.glconfig.colorBits, uiInfo.uiDC.glconfig.depthBits,
                 uiInfo.uiDC.glconfig.stencilBits, uiInfo.uiDC.glconfig.extensions_string );
                 
    UI_DrawTextBlock( rect, text_x, text_y, color, scale,
                      textalign, textvalign, textStyle, buffer );
}

static void UI_DrawCredits( rectDef_t* rect, F32 scale, S32 textalign, S32 textvalign,
                            vec4_t color, S32 textStyle, F32 text_x, F32 text_y )
{
    UI_DrawTextBlock( rect, text_x, text_y, color, scale, textalign, textvalign, textStyle,
                      "^2~~=[ ^4TremZ Core Development Team ^2]=~~\n\n" );
}

static void UI_DrawScreen( rectDef_t* rect, S32 modifier )
{
    S32 shotNumber;
    
    shotNumber = current_screen + modifier;
    
    if( shotNumber < 0 || shotNumber >= maxscreens )
        return;
        
    UI_DrawHandlePic( rect->x, rect->y, rect->w, rect->h,
                      trap_R_RegisterShaderNoMip( va( "screenshots/%s", screenshots[ shotNumber ] ) ) );
}

// FIXME: table drive
//
static void UI_OwnerDraw( F32 x, F32 y, F32 w, F32 h,
                          F32 text_x, F32 text_y, S32 ownerDraw,
                          S32 ownerDrawFlags, S32 align,
                          S32 textalign, S32 textvalign, F32 special,
                          F32 scale, vec4_t foreColor, vec4_t backColor,
                          qhandle_t shader, S32 textStyle )
{
    rectDef_t       rect;
    
    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;
    
    switch( ownerDraw )
    {
        case UI_TEAMINFOPANE:
            UI_DrawInfoPane( &uiInfo.teamList[ uiInfo.teamIndex ],
                             &rect, text_x, text_y, scale, textalign, textvalign, foreColor, textStyle );
            break;
            
        case UI_ACLASSINFOPANE:
            UI_DrawInfoPane( &uiInfo.alienClassList[ uiInfo.alienClassIndex ],
                             &rect, text_x, text_y, scale, textalign, textvalign, foreColor, textStyle );
            break;
            
        case UI_AUPGRADEINFOPANE:
            UI_DrawInfoPane( &uiInfo.alienUpgradeList[ uiInfo.alienUpgradeIndex ],
                             &rect, text_x, text_y, scale, textalign, textvalign, foreColor, textStyle );
            break;
            
        case UI_HITEMINFOPANE:
            UI_DrawInfoPane( &uiInfo.humanItemList[ uiInfo.humanItemIndex ],
                             &rect, text_x, text_y, scale, textalign, textvalign, foreColor, textStyle );
            break;
            
        case UI_HBUYINFOPANE:
            UI_DrawInfoPane( &uiInfo.humanArmouryBuyList[ uiInfo.humanArmouryBuyIndex ],
                             &rect, text_x, text_y, scale, textalign, textvalign, foreColor, textStyle );
            break;
            
        case UI_HSELLINFOPANE:
            UI_DrawInfoPane( &uiInfo.humanArmourySellList[ uiInfo.humanArmourySellIndex ],
                             &rect, text_x, text_y, scale, textalign, textvalign, foreColor, textStyle );
            break;
            
        case UI_ABUILDINFOPANE:
            UI_DrawInfoPane( &uiInfo.alienBuildList[ uiInfo.alienBuildIndex ],
                             &rect, text_x, text_y, scale, textalign, textvalign, foreColor, textStyle );
            break;
            
        case UI_HBUILDINFOPANE:
            UI_DrawInfoPane( &uiInfo.humanBuildList[ uiInfo.humanBuildIndex ],
                             &rect, text_x, text_y, scale, textalign, textvalign, foreColor, textStyle );
            break;
            
        case UI_HELPINFOPANE:
            UI_DrawInfoPane( &uiInfo.helpList[ uiInfo.helpIndex ],
                             &rect, text_x, text_y, scale, textalign, textvalign, foreColor, textStyle );
            break;
            
        case UI_NETMAPPREVIEW:
            UI_DrawServerMapPreview( &rect, scale, foreColor );
            break;
            
        case UI_SELECTEDMAPPREVIEW:
            UI_DrawSelectedMapPreview( &rect, scale, foreColor );
            break;
            
        case UI_SELECTEDMAPNAME:
            UI_DrawSelectedMapName( &rect, scale, foreColor, textStyle );
            break;
            
        case UI_GLINFO:
            UI_DrawGLInfo( &rect, scale, textalign, textvalign, foreColor, textStyle, text_x, text_y );
            break;
            
        case UI_CREDITS:
            UI_DrawCredits( &rect, scale, textalign, textvalign, foreColor, textStyle, text_x, text_y );
            break;
            
        case UI_SCREEN:
            UI_DrawScreen( &rect, special );
            break;
            
        default:
            break;
    }
    
}

static bool UI_OwnerDrawVisible( S32 flags )
{
    bool vis = true;
    uiClientState_t cs;
    team_t          team;
    UTF8            info[ MAX_INFO_STRING ];
    
    trap_GetClientState( &cs );
    trap_GetConfigString( CS_PLAYERS + cs.clientNum, info, MAX_INFO_STRING );
    team = ( team_t )atoi( Info_ValueForKey( info, "t" ) );
    
    
    while( flags )
    {
        if( flags & UI_SHOW_NOTSPECTATING )
        {
            if( team == TEAM_NONE )
                vis = false;
                
            flags &= ~UI_SHOW_NOTSPECTATING;
        }
        
        if( flags & UI_SHOW_VOTEACTIVE )
        {
            if( !trap_Cvar_VariableValue( "ui_voteActive" ) )
                vis = false;
                
            flags &= ~UI_SHOW_VOTEACTIVE;
        }
        
        if( flags & UI_SHOW_CANVOTE )
        {
            if( trap_Cvar_VariableValue( "ui_voteActive" ) )
                vis = false;
                
            flags &= ~UI_SHOW_CANVOTE;
        }
        
        if( flags & UI_SHOW_TEAMVOTEACTIVE )
        {
            if( team == TEAM_ALIENS )
            {
                if( !trap_Cvar_VariableValue( "ui_alienTeamVoteActive" ) )
                    vis = false;
            }
            else if( team == TEAM_HUMANS )
            {
                if( !trap_Cvar_VariableValue( "ui_humanTeamVoteActive" ) )
                    vis = false;
            }
            
            flags &= ~UI_SHOW_TEAMVOTEACTIVE;
        }
        
        if( flags & UI_SHOW_CANTEAMVOTE )
        {
            if( team == TEAM_ALIENS )
            {
                if( trap_Cvar_VariableValue( "ui_alienTeamVoteActive" ) )
                    vis = false;
            }
            else if( team == TEAM_HUMANS )
            {
                if( trap_Cvar_VariableValue( "ui_humanTeamVoteActive" ) )
                    vis = false;
            }
            
            flags &= ~UI_SHOW_CANTEAMVOTE;
        }
        
        if( flags & UI_SHOW_FAVORITESERVERS )
        {
            // this assumes you only put this type of display flag on something showing in the proper context
            
            if( ui_netSource.integer != AS_FAVORITES )
                vis = false;
                
            flags &= ~UI_SHOW_FAVORITESERVERS;
        }
        
        if( flags & UI_SHOW_NOTFAVORITESERVERS )
        {
            // this assumes you only put this type of display flag on something showing in the proper context
            
            if( ui_netSource.integer == AS_FAVORITES )
                vis = false;
                
            flags &= ~UI_SHOW_NOTFAVORITESERVERS;
        }
        else
            flags = 0;
    }
    
    return vis;
}

static bool UI_NetSource_HandleKey( S32 flags, F32* special, S32 key )
{
    if( key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER )
    {
        if( ui_netSource.integer >= numNetSources )
            ui_netSource.integer = 0;
        else if( ui_netSource.integer < 0 )
            ui_netSource.integer = numNetSources - 1;
            
        UI_BuildServerDisplayList( true );
        
        if( ui_netSource.integer != AS_GLOBAL )
            UI_StartServerRefresh( true );
            
        trap_Cvar_Set( "ui_netSource", va( "%d", ui_netSource.integer ) );
        return true;
    }
    
    return false;
}

static bool UI_OwnerDrawHandleKey( S32 ownerDraw, S32 flags, F32* special, S32 key )
{
    switch( ownerDraw )
    {
        case UI_NETSOURCE:
            UI_NetSource_HandleKey( flags, special, key );
            break;
            
        default:
            break;
    }
    
    return false;
}


/*
=================
UI_ServersQsortCompare
=================
*/
static S32 UI_ServersQsortCompare( const void* arg1, const void* arg2 )
{
    return trap_LAN_CompareServers( ui_netSource.integer, uiInfo.serverStatus.sortKey,
                                    uiInfo.serverStatus.sortDir, *( S32* )arg1, *( S32* )arg2 );
}


/*
=================
UI_ServersSort
=================
*/
void UI_ServersSort( S32 column, bool force )
{
    if( !force )
    {
        if( uiInfo.serverStatus.sortKey == column )
            return;
    }
    
    uiInfo.serverStatus.sortKey = column;
    qsort( &uiInfo.serverStatus.displayServers[0], uiInfo.serverStatus.numDisplayServers,
           sizeof( S32 ), UI_ServersQsortCompare );
}

/*
===============
UI_LoadTeams
===============
*/
static void UI_LoadTeams( void )
{
    uiInfo.teamCount = 4;
    
    uiInfo.teamList[ 0 ].text = String_Alloc( "Aliens" );
    uiInfo.teamList[ 0 ].cmd = String_Alloc( "cmd team aliens\n" );
    uiInfo.teamList[ 0 ].type = INFOTYPE_TEXT;
    uiInfo.teamList[ 0 ].v.text =
        "The Alien Team\n\n"
        "The Aliens' strengths are in movement and the ability to "
        "quickly construct new bases quickly. They possess a range "
        "of abilities including basic melee attacks, movement-"
        "crippling poisons and more.";
        
    uiInfo.teamList[ 1 ].text = String_Alloc( "Humans" );
    uiInfo.teamList[ 1 ].cmd = String_Alloc( "cmd team humans\n" );
    uiInfo.teamList[ 1 ].type = INFOTYPE_TEXT;
    uiInfo.teamList[ 1 ].v.text =
        "The Human Team\n\n"
        "The humans are the masters of technology. Although their "
        "bases take long to construct, their automated defense "
        "ensures they stay built. A wide range of upgrades and "
        "weapons are available to the humans, each contributing "
        "to eradicate the alien threat.";
        
    uiInfo.teamList[ 2 ].text = String_Alloc( "Spectate" );
    uiInfo.teamList[ 2 ].cmd = String_Alloc( "cmd team spectate\n" );
    uiInfo.teamList[ 2 ].type = INFOTYPE_TEXT;
    uiInfo.teamList[ 2 ].v.text = "Watch the game without playing.";
    
    uiInfo.teamList[ 3 ].text = String_Alloc( "Auto select" );
    uiInfo.teamList[ 3 ].cmd = String_Alloc( "cmd team auto\n" );
    uiInfo.teamList[ 3 ].type = INFOTYPE_TEXT;
    uiInfo.teamList[ 3 ].v.text = "Join the team with the least players.";
}

/*
===============
UI_AddClass
===============
*/

static void UI_AddClass( class_t _class )
{
    uiInfo.alienClassList[ uiInfo.alienClassCount ].text =
    
        String_Alloc( bggame->ClassConfig( _class )->humanName );
    uiInfo.alienClassList[ uiInfo.alienClassCount ].cmd =
    
        String_Alloc( va( "cmd class %s\n", bggame->Class( _class )->name ) );
    uiInfo.alienClassList[ uiInfo.alienClassCount ].type = INFOTYPE_CLASS;
    
    uiInfo.alienClassList[ uiInfo.alienClassCount ].v.pclass = _class;
    
    uiInfo.alienClassCount++;
}

/*
===============
UI_LoadAlienClasses
===============
*/
static void UI_LoadAlienClasses( void )
{
    uiInfo.alienClassCount = 0;
    
    if( bggame->ClassIsAllowed( PCL_ALIEN_LEVEL0 ) )
        UI_AddClass( PCL_ALIEN_LEVEL0 );
        
    if( bggame->ClassIsAllowed( PCL_ALIEN_BUILDER0_UPG ) &&
            bggame->ClassAllowedInStage( PCL_ALIEN_BUILDER0_UPG, UI_GetCurrentStage( ) ) )
        UI_AddClass( PCL_ALIEN_BUILDER0_UPG );
    else if( bggame->ClassIsAllowed( PCL_ALIEN_BUILDER0 ) )
        UI_AddClass( PCL_ALIEN_BUILDER0 );
}

/*
===============
UI_AddItem
===============
*/
static void UI_AddItem( weapon_t weapon )
{
    uiInfo.humanItemList[ uiInfo.humanItemCount ].text =
        String_Alloc( bggame->Weapon( weapon )->humanName );
    uiInfo.humanItemList[ uiInfo.humanItemCount ].cmd =
        String_Alloc( va( "cmd class %s\n", bggame->Weapon( weapon )->name ) );
    uiInfo.humanItemList[ uiInfo.humanItemCount ].type = INFOTYPE_WEAPON;
    uiInfo.humanItemList[ uiInfo.humanItemCount ].v.weapon = weapon;
    
    uiInfo.humanItemCount++;
}

/*
===============
UI_LoadHumanItems
===============
*/
static void UI_LoadHumanItems( void )
{
    uiInfo.humanItemCount = 0;
    
    if( bggame->WeaponIsAllowed( WP_MACHINEGUN ) )
        UI_AddItem( WP_MACHINEGUN );
    if( bggame->WeaponIsAllowed( WP_HBUILD ) )
        UI_AddItem( WP_HBUILD );
}

/*
===============
UI_ParseCarriageList
===============
*/
static void UI_ParseCarriageList( void )
{
    UTF8 buffer[ MAX_CVAR_VALUE_STRING ];
    
    trap_Cvar_VariableStringBuffer( "ui_carriage", buffer, sizeof( buffer ) );
    
    sscanf( buffer, "%d %d %d", &uiInfo.weapon, &uiInfo.upgrades, &uiInfo.credits );
}

/*
===============
UI_LoadHumanArmouryBuys
===============
*/
static void UI_LoadHumanArmouryBuys( void )
{
    S32 i, i2, j = 0;
    stage_t stage = UI_GetCurrentStage( );
    S32 slots = 0;
    
    UI_ParseCarriageList( );
    
    for( i = UP_NONE + 1; i < UP_NUM_UPGRADES; i++ )
    {
        if( uiInfo.upgrades & ( 1 << i ) )
            slots |= bggame->Upgrade( ( upgrade_t )i )->slots;
    }
    
    uiInfo.humanArmouryBuyCount = 0;
    
    for( i = WP_NONE + 1; i < WP_NUM_WEAPONS; i++ )
    {
        if( bggame->Weapon( ( weapon_t )i )->team == TEAM_HUMANS &&
                bggame->Weapon( ( weapon_t )i )->purchasable &&
                bggame->WeaponAllowedInStage( ( weapon_t )i, stage ) &&
                bggame->WeaponIsAllowed( ( weapon_t )i ) &&
                i != uiInfo.weapon )
        {
            UTF8 buffer[ MAX_STRING_CHARS ] = "";
            S32 price = bggame->Weapon( ( weapon_t )i )->price;
            if( uiInfo.weapon != WP_NONE && uiInfo.weapon != WP_BLASTER )
            {
                Com_sprintf( buffer, sizeof( buffer ), "cmd sell %s;", bggame->Weapon( ( weapon_t )uiInfo.weapon )->name );
                price -= bggame->Weapon( ( weapon_t )uiInfo.weapon )->price;
            }
            uiInfo.humanArmouryBuyList[ j ].text = String_Alloc( price <= uiInfo.credits ?
                                                   bggame->Weapon( ( weapon_t )i )->humanName : va( "^1%s", bggame->Weapon( ( weapon_t )i )->humanName ) );
            Com_sprintf( buffer, sizeof( buffer ), "%scmd buy %s\n", buffer, bggame->Weapon( ( weapon_t )i )->name );
            uiInfo.humanArmouryBuyList[ j ].cmd = String_Alloc( buffer );
            uiInfo.humanArmouryBuyList[ j ].type = INFOTYPE_WEAPON;
            uiInfo.humanArmouryBuyList[ j ].v.weapon = ( weapon_t )i;
            
            j++;
            
            uiInfo.humanArmouryBuyCount++;
        }
    }
    
    for( i = UP_NONE + 1; i < UP_NUM_UPGRADES; i++ )
    {
        if( bggame->Upgrade( ( upgrade_t )i )->team == TEAM_HUMANS &&
                bggame->Upgrade( ( upgrade_t )i )->purchasable &&
                bggame->UpgradeAllowedInStage( ( upgrade_t )i, stage ) &&
                bggame->UpgradeIsAllowed( ( upgrade_t )i ) &&
                !( uiInfo.upgrades & ( 1 << i ) ) )
        {
            UTF8 buffer[ MAX_STRING_CHARS ] = "";
            S32 price = bggame->Upgrade( ( upgrade_t )i )->price;
            for( i2 = UP_NONE + 1; i2 < UP_NUM_UPGRADES; i2++ )
            {
                if( ( uiInfo.upgrades & ( 1 << i2 ) ) &&
                        ( bggame->Upgrade( ( upgrade_t )i2 )->slots & bggame->Upgrade( ( upgrade_t )i )->slots ) )
                {
                    Com_sprintf( buffer, sizeof( buffer ), "%scmd sell %s;", buffer, bggame->Upgrade( ( upgrade_t )i2 )->name );
                    price -= bggame->Upgrade( ( upgrade_t )i2 )->price;
                }
            }
            uiInfo.humanArmouryBuyList[ j ].text = String_Alloc( price <= uiInfo.credits ?
                                                   bggame->Upgrade( ( upgrade_t )i )->humanName : va( "^1%s", bggame->Upgrade( ( upgrade_t )i )->humanName ) );
            Com_sprintf( buffer, sizeof( buffer ), "%scmd buy %s\n", buffer, bggame->Upgrade( ( upgrade_t )i )->name );
            uiInfo.humanArmouryBuyList[ j ].cmd = String_Alloc( buffer );
            uiInfo.humanArmouryBuyList[ j ].type = INFOTYPE_UPGRADE;
            uiInfo.humanArmouryBuyList[ j ].v.upgrade = ( upgrade_t )i;
            
            j++;
            
            uiInfo.humanArmouryBuyCount++;
        }
    }
}

/*
===============
UI_LoadHumanArmourySells
===============
*/
static void UI_LoadHumanArmourySells( void )
{
    S32 i, j = 0;
    
    uiInfo.humanArmourySellCount = 0;
    UI_ParseCarriageList( );
    
    if( uiInfo.weapon != WP_NONE )
    {
        uiInfo.humanArmourySellList[ j ].text = String_Alloc( bggame->Weapon( ( weapon_t )uiInfo.weapon )->humanName );
        uiInfo.humanArmourySellList[ j ].cmd =
            String_Alloc( va( "cmd sell %s\n", bggame->Weapon( ( weapon_t )uiInfo.weapon )->name ) );
        uiInfo.humanArmourySellList[ j ].type = INFOTYPE_WEAPON;
        uiInfo.humanArmourySellList[ j ].v.weapon = ( weapon_t )uiInfo.weapon;
        
        j++;
        
        uiInfo.humanArmourySellCount++;
    }
    
    for( i = UP_NONE + 1; i < UP_NUM_UPGRADES; i++ )
    {
        if( uiInfo.upgrades & ( 1 << i ) )
        {
            uiInfo.humanArmourySellList[ j ].text = String_Alloc( bggame->Upgrade( ( upgrade_t )i )->humanName );
            uiInfo.humanArmourySellList[ j ].cmd =
                String_Alloc( va( "cmd sell %s\n", bggame->Upgrade( ( upgrade_t )i )->name ) );
            uiInfo.humanArmourySellList[ j ].type = INFOTYPE_UPGRADE;
            uiInfo.humanArmourySellList[ j ].v.upgrade = ( upgrade_t )i;
            
            j++;
            
            uiInfo.humanArmourySellCount++;
        }
    }
}

/*
===============
UI_ArmouryRefreshCb
===============
*/
static void UI_ArmouryRefreshCb( void* data )
{
    S32 oldWeapon  = uiInfo.weapon;
    S32 oldUpgrades = uiInfo.upgrades;
    
    UI_ParseCarriageList( );
    
    if( uiInfo.weapon != oldWeapon || uiInfo.upgrades != oldUpgrades )
    {
        UI_LoadHumanArmouryBuys( );
        UI_LoadHumanArmourySells( );
        UI_RemoveCaptureFunc( );
    }
}

/*
===============
UI_LoadAlienUpgrades
===============
*/
static void UI_LoadAlienUpgrades( void )
{
    S32     i, j = 0;
    
    S32     _class, credits;
    UTF8    ui_currentClass[ MAX_STRING_CHARS ];
    stage_t stage = UI_GetCurrentStage( );
    
    trap_Cvar_VariableStringBuffer( "ui_currentClass", ui_currentClass, MAX_STRING_CHARS );
    
    sscanf( ui_currentClass, "%d %d", &_class, &credits );
    
    uiInfo.alienUpgradeCount = 0;
    
    for( i = PCL_NONE + 1; i < PCL_NUM_CLASSES; i++ )
    {
        if( bggame->ClassCanEvolveFromTo( ( class_t )_class, ( class_t )i, credits, stage, 0 ) >= 0 )
        {
            uiInfo.alienUpgradeList[ j ].text = String_Alloc( bggame->ClassConfig( ( class_t )i )->humanName );
            uiInfo.alienUpgradeList[ j ].cmd =
                String_Alloc( va( "cmd class %s\n", bggame->Class( ( class_t )i )->name ) );
            uiInfo.alienUpgradeList[ j ].type = INFOTYPE_CLASS;
            uiInfo.alienUpgradeList[ j ].v.pclass = ( class_t )i;
            
            j++;
            
            uiInfo.alienUpgradeCount++;
        }
    }
}

/*
===============
UI_LoadAlienBuilds
===============
*/
static void UI_LoadAlienBuilds( void )
{
    S32     i, j = 0;
    stage_t stage;
    
    UI_ParseCarriageList( );
    stage = UI_GetCurrentStage( );
    
    uiInfo.alienBuildCount = 0;
    
    for( i = BA_NONE + 1; i < BA_NUM_BUILDABLES; i++ )
    {
        if( bggame->Buildable( ( buildable_t )i )->team == TEAM_ALIENS &&
                ( bggame->Buildable( ( buildable_t )i )->buildWeapon & ( 1 << uiInfo.weapon ) ) &&
                bggame->BuildableAllowedInStage( ( buildable_t )i, stage ) &&
                bggame->BuildableIsAllowed( ( buildable_t )i ) )
        {
            uiInfo.alienBuildList[ j ].text =
                String_Alloc( bggame->Buildable( ( buildable_t )i )->humanName );
            uiInfo.alienBuildList[ j ].cmd =
                String_Alloc( va( "cmd build %s\n", bggame->Buildable( ( buildable_t )i )->name ) );
            uiInfo.alienBuildList[ j ].type = INFOTYPE_BUILDABLE;
            uiInfo.alienBuildList[ j ].v.buildable = ( buildable_t )i;
            
            j++;
            
            uiInfo.alienBuildCount++;
        }
    }
}

/*
===============
UI_LoadHumanBuilds
===============
*/
static void UI_LoadHumanBuilds( void )
{
    S32     i, j = 0;
    stage_t stage;
    
    UI_ParseCarriageList( );
    stage = UI_GetCurrentStage( );
    
    uiInfo.humanBuildCount = 0;
    
    for( i = BA_NONE + 1; i < BA_NUM_BUILDABLES; i++ )
    {
        if( bggame->Buildable( ( buildable_t )i )->team == TEAM_HUMANS &&
                ( bggame->Buildable( ( buildable_t )i )->buildWeapon & ( 1 << uiInfo.weapon ) ) &&
                bggame->BuildableAllowedInStage( ( buildable_t )i, stage ) &&
                bggame->BuildableIsAllowed( ( buildable_t )i ) )
        {
            uiInfo.humanBuildList[ j ].text =
                String_Alloc( bggame->Buildable( ( buildable_t )i )->humanName );
            uiInfo.humanBuildList[ j ].cmd =
                String_Alloc( va( "cmd build %s\n", bggame->Buildable( ( buildable_t )i )->name ) );
            uiInfo.humanBuildList[ j ].type = INFOTYPE_BUILDABLE;
            uiInfo.humanBuildList[ j ].v.buildable = ( buildable_t )i;
            
            j++;
            
            uiInfo.humanBuildCount++;
        }
    }
}

/*
===============
UI_LoadMods
===============
*/
static void UI_LoadMods( void )
{
    S32   numdirs;
    UTF8  dirlist[2048];
    UTF8*  dirptr;
    UTF8*  descptr;
    S32   i;
    S32   dirlen;
    
    uiInfo.modCount = 0;
    numdirs = trap_FS_GetFileList( "$modlist", "", dirlist, sizeof( dirlist ) );
    dirptr  = dirlist;
    
    for( i = 0; i < numdirs; i++ )
    {
        dirlen = strlen( dirptr ) + 1;
        descptr = dirptr + dirlen;
        uiInfo.modList[uiInfo.modCount].modName = String_Alloc( dirptr );
        uiInfo.modList[uiInfo.modCount].modDescr = String_Alloc( descptr );
        dirptr += dirlen + strlen( descptr ) + 1;
        uiInfo.modCount++;
        
        if( uiInfo.modCount >= MAX_MODS )
            break;
    }
    
}


/*
===============
UI_LoadMovies
===============
*/
static void UI_LoadMovies( void )
{
    UTF8  movielist[4096];
    UTF8*  moviename;
    S32   i, len;
    
    uiInfo.movieCount = trap_FS_GetFileList( "video", "roq", movielist, 4096 );
    
    if( uiInfo.movieCount )
    {
        if( uiInfo.movieCount > MAX_MOVIES )
            uiInfo.movieCount = MAX_MOVIES;
            
        moviename = movielist;
        
        for( i = 0; i < uiInfo.movieCount; i++ )
        {
            len = strlen( moviename );
            
            if( !Q_stricmp( moviename +  len - 4, ".roq" ) )
                moviename[len - 4] = '\0';
                
            Q_strupr( moviename );
            uiInfo.movieList[i] = String_Alloc( moviename );
            moviename += len + 1;
        }
    }
    
}



/*
===============
UI_LoadDemos
===============
*/
static void UI_LoadDemos( void )
{
    UTF8  demolist[4096];
    UTF8 demoExt[32];
    UTF8*  demoname;
    S32   i, len;
    
    Com_sprintf( demoExt, sizeof( demoExt ), "dm_%d", ( S32 )trap_Cvar_VariableValue( "protocol" ) );
    
    uiInfo.demoCount = trap_FS_GetFileList( "demos", demoExt, demolist, 4096 );
    
    Com_sprintf( demoExt, sizeof( demoExt ), ".dm_%d", ( S32 )trap_Cvar_VariableValue( "protocol" ) );
    
    if( uiInfo.demoCount )
    {
        if( uiInfo.demoCount > MAX_DEMOS )
            uiInfo.demoCount = MAX_DEMOS;
            
        demoname = demolist;
        
        for( i = 0; i < uiInfo.demoCount; i++ )
        {
            len = strlen( demoname );
            
            if( !Q_stricmp( demoname +  len - strlen( demoExt ), demoExt ) )
                demoname[len - strlen( demoExt )] = '\0';
                
            uiInfo.demoList[i] = String_Alloc( demoname );
            demoname += len + 1;
        }
    }
    
}

bool idUserInterfaceManagerLocal::CheckExecKey( S32 key )
{
    menuDef_t* menu = Menu_GetFocused();
    
    if( g_editingField )
    {
        return true;
    }
    
    if( key > 256 )
    {
        return false;
    }
    
#if 0
    if( !menu )
    {
        if( cl_bypassMouseInput.integer )
        {
            if( !trap_Key_GetCatcher() )
            {
                trap_Cvar_Set( "cl_bypassMouseInput", "0" );
            }
        }
        return false;
    }
    
    if( menu->onKey[key] )
    {
        return true;
    }
#endif
    
    return false;
}
static void UI_Update( StringEntry name )
{
    S32 val = trap_Cvar_VariableValue( name );
    
    if( Q_stricmp( name, "ui_SetName" ) == 0 )
        trap_Cvar_Set( "name", UI_Cvar_VariableString( "ui_Name" ) );
    else if( Q_stricmp( name, "ui_setRate" ) == 0 )
    {
        F32 rate = trap_Cvar_VariableValue( "rate" );
        
        if( rate >= 5000 )
        {
            trap_Cvar_Set( "cl_maxpackets", "30" );
            trap_Cvar_Set( "cl_packetdup", "1" );
        }
        else if( rate >= 4000 )
        {
            trap_Cvar_Set( "cl_maxpackets", "15" );
            trap_Cvar_Set( "cl_packetdup", "2" );   // favor less prediction errors when there's packet loss
        }
        else
        {
            trap_Cvar_Set( "cl_maxpackets", "15" );
            trap_Cvar_Set( "cl_packetdup", "1" );   // favor lower bandwidth
        }
    }
    else if( Q_stricmp( name, "ui_GetName" ) == 0 )
        trap_Cvar_Set( "ui_Name", UI_Cvar_VariableString( "name" ) );
    else if( Q_stricmp( name, "r_colorbits" ) == 0 )
    {
        switch( val )
        {
            case 0:
                trap_Cvar_SetValue( "r_depthbits", 0 );
                trap_Cvar_SetValue( "r_stencilbits", 0 );
                break;
                
            case 16:
                trap_Cvar_SetValue( "r_depthbits", 16 );
                trap_Cvar_SetValue( "r_stencilbits", 0 );
                break;
                
            case 32:
                trap_Cvar_SetValue( "r_depthbits", 24 );
                break;
        }
    }
    else if( Q_stricmp( name, "r_lodbias" ) == 0 )
    {
        switch( val )
        {
            case 0:
                trap_Cvar_SetValue( "r_subdivisions", 4 );
                break;
                
            case 1:
                trap_Cvar_SetValue( "r_subdivisions", 12 );
                break;
                
            case 2:
                trap_Cvar_SetValue( "r_subdivisions", 20 );
                break;
        }
    }
    else if( Q_stricmp( name, "ui_glCustom" ) == 0 )
    {
        switch( val )
        {
            case 0: // high quality
                trap_Cvar_SetValue( "r_subdivisions", 4 );
                trap_Cvar_SetValue( "r_vertexlight", 0 );
                trap_Cvar_SetValue( "r_lodbias", 0 );
                trap_Cvar_SetValue( "r_colorbits", 32 );
                trap_Cvar_SetValue( "r_depthbits", 24 );
                trap_Cvar_SetValue( "r_picmip", 0 );
                trap_Cvar_SetValue( "r_texturebits", 32 );
                trap_Cvar_SetValue( "r_fastSky", 0 );
                trap_Cvar_SetValue( "r_inGameVideo", 1 );
                trap_Cvar_SetValue( "cg_shadows", 1 );
                trap_Cvar_SetValue( "cg_brassTime", 2500 );
                trap_Cvar_SetValue( "cg_bounceParticles", 1 );
                trap_Cvar_Set( "r_texturemode", "GL_LINEAR_MIPMAP_LINEAR" );
                break;
                
            case 1: // normal
                trap_Cvar_SetValue( "r_subdivisions", 12 );
                trap_Cvar_SetValue( "r_vertexlight", 0 );
                trap_Cvar_SetValue( "r_lodbias", 0 );
                trap_Cvar_SetValue( "r_colorbits", 0 );
                trap_Cvar_SetValue( "r_depthbits", 24 );
                trap_Cvar_SetValue( "r_picmip", 1 );
                trap_Cvar_SetValue( "r_texturebits", 0 );
                trap_Cvar_SetValue( "r_fastSky", 0 );
                trap_Cvar_SetValue( "r_inGameVideo", 1 );
                trap_Cvar_SetValue( "cg_brassTime", 2500 );
                trap_Cvar_Set( "r_texturemode", "GL_LINEAR_MIPMAP_LINEAR" );
                trap_Cvar_SetValue( "cg_shadows", 0 );
                trap_Cvar_SetValue( "cg_bounceParticles", 0 );
                break;
                
            case 2: // fast
                trap_Cvar_SetValue( "r_subdivisions", 8 );
                trap_Cvar_SetValue( "r_vertexlight", 0 );
                trap_Cvar_SetValue( "r_lodbias", 1 );
                trap_Cvar_SetValue( "r_colorbits", 0 );
                trap_Cvar_SetValue( "r_depthbits", 0 );
                trap_Cvar_SetValue( "r_picmip", 1 );
                trap_Cvar_SetValue( "r_texturebits", 0 );
                trap_Cvar_SetValue( "cg_shadows", 0 );
                trap_Cvar_SetValue( "r_fastSky", 1 );
                trap_Cvar_SetValue( "r_inGameVideo", 0 );
                trap_Cvar_SetValue( "cg_brassTime", 0 );
                trap_Cvar_SetValue( "cg_bounceParticles", 0 );
                trap_Cvar_Set( "r_texturemode", "GL_LINEAR_MIPMAP_NEAREST" );
                break;
                
            case 3: // fastest
                trap_Cvar_SetValue( "r_subdivisions", 20 );
                trap_Cvar_SetValue( "r_vertexlight", 1 );
                trap_Cvar_SetValue( "r_lodbias", 2 );
                trap_Cvar_SetValue( "r_colorbits", 16 );
                trap_Cvar_SetValue( "r_depthbits", 16 );
                trap_Cvar_SetValue( "r_picmip", 2 );
                trap_Cvar_SetValue( "r_texturebits", 16 );
                trap_Cvar_SetValue( "cg_shadows", 0 );
                trap_Cvar_SetValue( "cg_brassTime", 0 );
                trap_Cvar_SetValue( "r_fastSky", 1 );
                trap_Cvar_SetValue( "r_inGameVideo", 0 );
                trap_Cvar_SetValue( "cg_bounceParticles", 0 );
                trap_Cvar_Set( "r_texturemode", "GL_LINEAR_MIPMAP_NEAREST" );
                break;
        }
    }
    else if( Q_stricmp( name, "ui_mousePitch" ) == 0 )
    {
        if( val == 0 )
            trap_Cvar_SetValue( "m_pitch", 0.022f );
        else
            trap_Cvar_SetValue( "m_pitch", -0.022f );
    }
}

void UI_ScreenChange( UTF8** args )
{
    static S32 saved_index = 0;
    StringEntry string;
    S32 i, modifier;
    UTF8 buffer[ 8192 ];
    
    // Reload the screenshots list
    maxscreens = trap_FS_GetFileList( "screenshots", ".jpg", buffer, sizeof( buffer ) );
    if( maxscreens > 1024 )
        maxscreens = 1024;
    string = buffer;
    for( i = 0; i < maxscreens; i++ )
    {
        Q_strncpyz( screenshots[ i ], string, sizeof( screenshots[ i ] ) );
        string += strlen( string ) + 1;
    }
    
    // Also load TGAs
    maxscreens += trap_FS_GetFileList( "screenshots", ".tga", buffer, sizeof( buffer ) );
    if( maxscreens > 1024 )
        maxscreens = 1024;
    trap_Cvar_SetValue( "ui_screens", maxscreens );
    string = buffer;
    for( ; i < maxscreens; i++ )
    {
        Q_strncpyz( screenshots[ i ], string, sizeof( screenshots[ i ] ) );
        string += strlen( string ) + 1;
    }
    
    // Sort the list
    qsort( screenshots, maxscreens, MAX_QPATH, ( S32(* )( const void*, const void* ) )strcmp );
    
    // Get the modifier & find the screen index
    if( String_Parse( args, &string ) )
    {
        if( string[0] == '+' )
        {
            if( Int_Parse( args, &modifier ) )
                current_screen += modifier;
        }
        else if( string[0] == '-' )
        {
            if( Int_Parse( args, &modifier ) )
                current_screen -= modifier;
        }
        else if( string[0] == '=' )
        {
            if( Int_Parse( args, &modifier ) )
                current_screen = modifier;
        }
        // Hack for saving the index when switching from multiview to detail view
        else if( string[0] == '?' )
            saved_index = current_screen;
        else if( string[0] == '!' )
            current_screen = saved_index;
    }
    
    // We don't want it to go below 0 or above the max number of screens
    if( current_screen >= maxscreens )
    {
        current_screen -= modifier;
        if( current_screen >= maxscreens )
            current_screen = maxscreens - 1;
    }
    else if( current_screen < 0 )
    {
        current_screen += modifier;
        if( current_screen < 0 )
            current_screen = 0;
    }
    
    trap_Cvar_SetValue( "ui_screen", current_screen );
    trap_Cvar_Set( "ui_screenname", screenshots[ current_screen ] );
}

//FIXME: lookup table
static void UI_RunMenuScript( UTF8** args )
{
    StringEntry name, name2;
    UTF8 buff[1024];
    StringEntry cmd;
    
    if( String_Parse( args, &name ) )
    {
        if( Q_stricmp( name, "StartServer" ) == 0 )
        {
            trap_Cvar_SetValue( "dedicated", Com_Clamp( 0, 2, ui_dedicated.integer ) );
            trap_Cmd_ExecuteText( EXEC_APPEND, va( "wait ; wait ; map %s\n",
                                                   uiInfo.mapList[ui_selectedMap.integer].mapLoadName ) );
        }
        else if( Q_stricmp( name, "resetDefaults" ) == 0 )
        {
            trap_Cmd_ExecuteText( EXEC_APPEND, "exec default.cfg\n" );
            trap_Cmd_ExecuteText( EXEC_APPEND, "cvar_restart\n" );
            Controls_SetDefaults();
            trap_Cvar_Set( "com_introPlayed", "1" );
            trap_Cmd_ExecuteText( EXEC_APPEND, "vid_restart\n" );
        }
        else if( Q_stricmp( name, "loadArenas" ) == 0 )
        {
            UI_LoadArenas();
            Menu_SetFeederSelection( NULL, FEEDER_MAPS, 0, "createserver" );
        }
        else if( Q_stricmp( name, "loadServerInfo" ) == 0 )
            UI_ServerInfo();
        else if( Q_stricmp( name, "saveControls" ) == 0 )
            Controls_SetConfig( true );
        else if( Q_stricmp( name, "loadControls" ) == 0 )
            Controls_GetConfig();
        else if( Q_stricmp( name, "clearError" ) == 0 )
            trap_Cvar_Set( "com_errorMessage", "" );
        //else if( Q_stricmp( name, "downloadIgnore" ) == 0 )
        //    trap_Cvar_Set( "cl_downloadPrompt", va( "%d", DLP_IGNORE ) );
        //else if( Q_stricmp( name, "downloadCURL" ) == 0 )
        //    trap_Cvar_Set( "cl_downloadPrompt", va( "%d", DLP_CURL ) );
        //else if( Q_stricmp( name, "downloadUDP" ) == 0 )
        //    trap_Cvar_Set( "cl_downloadPrompt", va( "%d", DLP_UDP ) );
        else if( Q_stricmp( name, "RefreshServers" ) == 0 )
        {
            UI_StartServerRefresh( true );
            UI_BuildServerDisplayList( true );
        }
        else if( Q_stricmp( name, "InitServerList" ) == 0 )
        {
            S32 time = trap_RealTime( NULL );
            S32 last;
            S32 sortColumn;
            
            // set up default sorting
            
            if( !uiInfo.serverStatus.sorted && Int_Parse( args, &sortColumn ) )
            {
                uiInfo.serverStatus.sortKey = sortColumn;
                uiInfo.serverStatus.sortDir = 0;
            }
            
            // refresh if older than 3 days or if list is empty
            last = atoi( UI_Cvar_VariableString( va( "ui_lastServerRefresh_%i_time",
                                                 ui_netSource.integer ) ) );
                                                 
            if( trap_LAN_GetServerCount( ui_netSource.integer ) < 1 ||
                    ( time - last ) > 3600 )
            {
                UI_StartServerRefresh( true );
                UI_BuildServerDisplayList( true );
            }
        }
        else if( Q_stricmp( name, "RefreshFilter" ) == 0 )
        {
            UI_StartServerRefresh( false );
            UI_BuildServerDisplayList( true );
        }
        else if( Q_stricmp( name, "LoadDemos" ) == 0 )
            UI_LoadDemos();
        else if( Q_stricmp( name, "LoadMovies" ) == 0 )
            UI_LoadMovies();
        else if( Q_stricmp( name, "LoadMods" ) == 0 )
            UI_LoadMods();
        else if( Q_stricmp( name, "LoadTeams" ) == 0 )
            UI_LoadTeams( );
        else if( Q_stricmp( name, "JoinTeam" ) == 0 )
        {
            if( ( cmd = uiInfo.teamList[ uiInfo.teamIndex ].cmd ) )
                trap_Cmd_ExecuteText( EXEC_APPEND, cmd );
        }
        else if( Q_stricmp( name, "LoadHumanItems" ) == 0 )
            UI_LoadHumanItems( );
        else if( Q_stricmp( name, "SpawnWithHumanItem" ) == 0 )
        {
            if( ( cmd = uiInfo.humanItemList[ uiInfo.humanItemIndex ].cmd ) )
                trap_Cmd_ExecuteText( EXEC_APPEND, cmd );
        }
        else if( Q_stricmp( name, "LoadAlienClasses" ) == 0 )
            UI_LoadAlienClasses( );
        else if( Q_stricmp( name, "SpawnAsAlienClass" ) == 0 )
        {
            if( ( cmd = uiInfo.alienClassList[ uiInfo.alienClassIndex ].cmd ) )
                trap_Cmd_ExecuteText( EXEC_APPEND, cmd );
        }
        else if( Q_stricmp( name, "LoadHumanArmouryBuys" ) == 0 )
            UI_LoadHumanArmouryBuys( );
        else if( Q_stricmp( name, "BuyFromArmoury" ) == 0 )
        {
            if( ( cmd = uiInfo.humanArmouryBuyList[ uiInfo.humanArmouryBuyIndex ].cmd ) )
                trap_Cmd_ExecuteText( EXEC_APPEND, cmd );
                
            UI_InstallCaptureFunc( UI_ArmouryRefreshCb, NULL, 1000 );
        }
        else if( Q_stricmp( name, "LoadHumanArmourySells" ) == 0 )
            UI_LoadHumanArmourySells( );
        else if( Q_stricmp( name, "SellToArmoury" ) == 0 )
        {
            if( ( cmd = uiInfo.humanArmourySellList[ uiInfo.humanArmourySellIndex ].cmd ) )
                trap_Cmd_ExecuteText( EXEC_APPEND, cmd );
                
            UI_InstallCaptureFunc( UI_ArmouryRefreshCb, NULL, 1000 );
        }
        else if( Q_stricmp( name, "LoadAlienUpgrades" ) == 0 )
        {
            UI_LoadAlienUpgrades( );
            
            //disallow the menu if it would be empty
            
            if( uiInfo.alienUpgradeCount <= 0 )
                Menus_CloseAll( false );
        }
        else if( Q_stricmp( name, "UpgradeToNewClass" ) == 0 )
        {
            if( ( cmd = uiInfo.alienUpgradeList[ uiInfo.alienUpgradeIndex ].cmd ) )
                trap_Cmd_ExecuteText( EXEC_APPEND, cmd );
        }
        else if( Q_stricmp( name, "LoadAlienBuilds" ) == 0 )
            UI_LoadAlienBuilds( );
        else if( Q_stricmp( name, "BuildAlienBuildable" ) == 0 )
        {
            if( ( cmd = uiInfo.alienBuildList[ uiInfo.alienBuildIndex ].cmd ) )
                trap_Cmd_ExecuteText( EXEC_APPEND, cmd );
        }
        else if( Q_stricmp( name, "LoadHumanBuilds" ) == 0 )
            UI_LoadHumanBuilds( );
        else if( Q_stricmp( name, "BuildHumanBuildable" ) == 0 )
        {
            if( ( cmd = uiInfo.humanBuildList[ uiInfo.humanBuildIndex ].cmd ) )
                trap_Cmd_ExecuteText( EXEC_APPEND, cmd );
        }
        else if( Q_stricmp( name, "PTRCRestore" ) == 0 )
        {
            S32           len;
            UTF8          text[ 16 ];
            fileHandle_t  f;
            UTF8          command[ 32 ];
            
            // load the file
            len = trap_FS_FOpenFile( "ptrc.cfg", &f, FS_READ );
            
            if( len > 0 && ( len < sizeof( text ) - 1 ) )
            {
                trap_FS_Read( text, len, f );
                text[ len ] = 0;
                
                Com_sprintf( command, 32, "ptrcrestore %s", text );
                
                trap_Cmd_ExecuteText( EXEC_APPEND, command );
            }
            if( len > -1 )
                trap_FS_FCloseFile( f );
        }
        else if( Q_stricmp( name, "Say" ) == 0 )
        {
            UTF8 buffer[ MAX_CVAR_VALUE_STRING ];
            trap_Cvar_VariableStringBuffer( "ui_sayBuffer", buffer, sizeof( buffer ) );
            
            if( !buffer[ 0 ] )
            {
            }
            else if( uiInfo.chatTargetClientNum != -1 )
                trap_Cmd_ExecuteText( EXEC_APPEND, va( "tell %i \"%s\"\n", uiInfo.chatTargetClientNum, buffer ) );
            else if( uiInfo.chatTeam )
                trap_Cmd_ExecuteText( EXEC_APPEND, va( "say_team \"%s\"\n", buffer ) );
            else if( uiInfo.chatAdmins )
                trap_Cmd_ExecuteText( EXEC_APPEND, va( "say_admins \"%s\"\n", buffer ) );
            else if( uiInfo.chatClan )
            {
                UTF8 clantagDecolored[ 32 ];
                trap_Cvar_VariableStringBuffer( "cl_clantag", clantagDecolored, sizeof( clantagDecolored ) );
                Q_CleanStr( clantagDecolored );
                if( strlen( clantagDecolored ) > 2 && strlen( clantagDecolored ) < 11 )
                    trap_Cmd_ExecuteText( EXEC_APPEND, va( "m \"%s\" \"%s\"\n", clantagDecolored, buffer ) );
                else
                    Com_Printf( "^3Error: Your clantag has to be between 3 and 10 chars long. Current value is:^7 %s^7\n", clantagDecolored );
            }
            else if( uiInfo.chatPrompt )
                trap_Cmd_ExecuteText( EXEC_APPEND, va( "vstr \"%s\"\n", uiInfo.chatPromptCallback ) );
            else
                trap_Cmd_ExecuteText( EXEC_APPEND, va( "say \"%s\"\n", buffer ) );
        }
        else if( Q_stricmp( name, "playMovie" ) == 0 )
        {
            if( uiInfo.previewMovie >= 0 )
                trap_CIN_StopCinematic( uiInfo.previewMovie );
                
            trap_Cmd_ExecuteText( EXEC_APPEND, va( "cinematic %s.roq 2\n", uiInfo.movieList[uiInfo.movieIndex] ) );
        }
        else if( Q_stricmp( name, "RunMod" ) == 0 )
        {
            trap_Cvar_Set( "fs_game", uiInfo.modList[uiInfo.modIndex].modName );
            trap_Cmd_ExecuteText( EXEC_APPEND, "vid_restart;" );
        }
        else if( Q_stricmp( name, "RunDemo" ) == 0 )
            trap_Cmd_ExecuteText( EXEC_APPEND, va( "demo %s\n", uiInfo.demoList[uiInfo.demoIndex] ) );
        else if( Q_stricmp( name, "Tremulous" ) == 0 )
        {
            trap_Cvar_Set( "fs_game", "" );
            trap_Cmd_ExecuteText( EXEC_APPEND, "vid_restart;" );
        }
        else if( Q_stricmp( name, "closeJoin" ) == 0 )
        {
            if( uiInfo.serverStatus.refreshActive )
            {
                UI_StopServerRefresh();
                uiInfo.serverStatus.nextDisplayRefresh = 0;
                uiInfo.nextServerStatusRefresh = 0;
                uiInfo.nextFindPlayerRefresh = 0;
                UI_BuildServerDisplayList( true );
            }
            else
            {
                Menus_CloseByName( "joinserver" );
                Menus_ActivateByName( "main" );
            }
        }
        else if( Q_stricmp( name, "StopRefresh" ) == 0 )
        {
            UI_StopServerRefresh();
            uiInfo.serverStatus.nextDisplayRefresh = 0;
            uiInfo.nextServerStatusRefresh = 0;
            uiInfo.nextFindPlayerRefresh = 0;
        }
        else if( Q_stricmp( name, "UpdateFilter" ) == 0 )
        {
            if( ui_netSource.integer == AS_LOCAL )
                UI_StartServerRefresh( true );
                
            UI_BuildServerDisplayList( true );
            UI_FeederSelection( FEEDER_SERVERS, 0 );
        }
        else if( Q_stricmp( name, "ServerStatus" ) == 0 )
        {
            trap_LAN_GetServerAddressString( ui_netSource.integer,
                                             uiInfo.serverStatus.displayServers[uiInfo.serverStatus.currentServer],
                                             uiInfo.serverStatusAddress, sizeof( uiInfo.serverStatusAddress ) );
            UI_BuildServerStatus( true );
        }
        else if( Q_stricmp( name, "FoundPlayerServerStatus" ) == 0 )
        {
            Q_strncpyz( uiInfo.serverStatusAddress,
                        uiInfo.foundPlayerServerAddresses[uiInfo.currentFoundPlayerServer],
                        sizeof( uiInfo.serverStatusAddress ) );
            UI_BuildServerStatus( true );
            Menu_SetFeederSelection( NULL, FEEDER_FINDPLAYER, 0, NULL );
        }
        else if( Q_stricmp( name, "FindPlayer" ) == 0 )
        {
            UI_BuildFindPlayerList( true );
            // clear the displayed server status info
            uiInfo.serverStatusInfo.numLines = 0;
            Menu_SetFeederSelection( NULL, FEEDER_FINDPLAYER, 0, NULL );
        }
        else if( Q_stricmp( name, "JoinServer" ) == 0 )
        {
            if( uiInfo.serverStatus.currentServer >= 0 &&
                    uiInfo.serverStatus.currentServer < uiInfo.serverStatus.numDisplayServers )
            {
                trap_LAN_GetServerAddressString( ui_netSource.integer,
                                                 uiInfo.serverStatus.displayServers[uiInfo.serverStatus.currentServer],
                                                 buff, 1024 );
                trap_Cmd_ExecuteText( EXEC_APPEND, va( "connect %s\n", buff ) );
            }
        }
        else if( Q_stricmp( name, "FoundPlayerJoinServer" ) == 0 )
        {
            if( uiInfo.currentFoundPlayerServer >= 0 &&
                    uiInfo.currentFoundPlayerServer < uiInfo.numFoundPlayerServers )
            {
                trap_Cmd_ExecuteText( EXEC_APPEND, va( "connect %s\n",
                                                       uiInfo.foundPlayerServerAddresses[uiInfo.currentFoundPlayerServer] ) );
            }
        }
        else if( Q_stricmp( name, "Quit" ) == 0 )
            trap_Cmd_ExecuteText( EXEC_APPEND, "quit" );
        else if( Q_stricmp( name, "Leave" ) == 0 )
        {
            trap_Cmd_ExecuteText( EXEC_APPEND, "disconnect\n" );
            trap_Key_SetCatcher( KEYCATCH_UI );
            Menus_CloseAll( true );
            Menus_ActivateByName( "main" );
        }
        else if( Q_stricmp( name, "ServerSort" ) == 0 )
        {
            S32 sortColumn;
            
            if( Int_Parse( args, &sortColumn ) )
            {
                // if same column we're already sorting on then flip the direction
                
                if( sortColumn == uiInfo.serverStatus.sortKey )
                    uiInfo.serverStatus.sortDir = !uiInfo.serverStatus.sortDir;
                    
                // make sure we sort again
                UI_ServersSort( sortColumn, true );
                
                uiInfo.serverStatus.sorted = true;
            }
        }
        else if( Q_stricmp( name, "closeingame" ) == 0 )
        {
            trap_Key_SetCatcher( trap_Key_GetCatcher() & ~KEYCATCH_UI );
            trap_Key_ClearStates();
            trap_Cvar_Set( "cl_paused", "0" );
            Menus_CloseAll( false );
        }
        else if( Q_stricmp( name, "voteMap" ) == 0 )
        {
            if( ui_selectedMap.integer >= 0 && ui_selectedMap.integer < uiInfo.mapCount )
            {
                trap_Cmd_ExecuteText( EXEC_APPEND, va( "callvote map %s\n",
                                                       uiInfo.mapList[ui_selectedMap.integer].mapLoadName ) );
            }
        }
        else if( Q_stricmp( name, "voteKick" ) == 0 )
        {
            if( uiInfo.playerIndex >= 0 && uiInfo.playerIndex < uiInfo.playerCount )
            {
                trap_Cmd_ExecuteText( EXEC_APPEND, va( "callvote kick %d\n",
                                                       uiInfo.clientNums[ uiInfo.playerIndex ] ) );
            }
        }
        else if( Q_stricmp( name, "voteMute" ) == 0 )
        {
            if( uiInfo.playerIndex >= 0 && uiInfo.playerIndex < uiInfo.playerCount )
            {
                trap_Cmd_ExecuteText( EXEC_APPEND, va( "callvote mute %d\n",
                                                       uiInfo.clientNums[ uiInfo.playerIndex ] ) );
            }
        }
        else if( Q_stricmp( name, "voteUnMute" ) == 0 )
        {
            if( uiInfo.playerIndex >= 0 && uiInfo.playerIndex < uiInfo.playerCount )
            {
                trap_Cmd_ExecuteText( EXEC_APPEND, va( "callvote unmute %d\n",
                                                       uiInfo.clientNums[ uiInfo.playerIndex ] ) );
            }
        }
        else if( Q_stricmp( name, "voteTeamKick" ) == 0 )
        {
            if( uiInfo.teamPlayerIndex >= 0 && uiInfo.teamPlayerIndex < uiInfo.myTeamCount )
            {
                trap_Cmd_ExecuteText( EXEC_APPEND, va( "callteamvote kick %d\n",
                                                       uiInfo.teamClientNums[ uiInfo.teamPlayerIndex ] ) );
            }
        }
        else if( Q_stricmp( name, "voteTeamDenyBuild" ) == 0 )
        {
            if( uiInfo.teamPlayerIndex >= 0 && uiInfo.teamPlayerIndex < uiInfo.myTeamCount )
            {
                trap_Cmd_ExecuteText( EXEC_APPEND, va( "callteamvote denybuild %d\n",
                                                       uiInfo.teamClientNums[ uiInfo.teamPlayerIndex ] ) );
            }
        }
        else if( Q_stricmp( name, "voteTeamAllowBuild" ) == 0 )
        {
            if( uiInfo.teamPlayerIndex >= 0 && uiInfo.teamPlayerIndex < uiInfo.myTeamCount )
            {
                trap_Cmd_ExecuteText( EXEC_APPEND, va( "callteamvote allowbuild %d\n",
                                                       uiInfo.teamClientNums[ uiInfo.teamPlayerIndex ] ) );
            }
        }
        else if( Q_stricmp( name, "addFavorite" ) == 0 )
        {
            if( ui_netSource.integer != AS_FAVORITES )
            {
                UTF8 name[MAX_NAME_LENGTH];
                UTF8 addr[MAX_NAME_LENGTH];
                S32 res;
                
                trap_LAN_GetServerInfo( ui_netSource.integer,
                                        uiInfo.serverStatus.displayServers[uiInfo.serverStatus.currentServer],
                                        buff, MAX_STRING_CHARS );
                name[0] = addr[0] = '\0';
                Q_strncpyz( name,  Info_ValueForKey( buff, "hostname" ), MAX_NAME_LENGTH );
                Q_strncpyz( addr,  Info_ValueForKey( buff, "addr" ), MAX_NAME_LENGTH );
                
                if( strlen( name ) > 0 && strlen( addr ) > 0 )
                {
                    res = trap_LAN_AddServer( AS_FAVORITES, name, addr );
                    
                    if( res == 0 )
                    {
                        // server already in the list
                        Com_Printf( "Favorite already in list\n" );
                    }
                    else if( res == -1 )
                    {
                        // list full
                        Com_Printf( "Favorite list full\n" );
                    }
                    else
                    {
                        // successfully added
                        Com_Printf( "Added favorite server %s\n", addr );
                    }
                }
            }
        }
        else if( Q_stricmp( name, "deleteFavorite" ) == 0 )
        {
            if( ui_netSource.integer == AS_FAVORITES )
            {
                UTF8 addr[MAX_NAME_LENGTH];
                trap_LAN_GetServerInfo( ui_netSource.integer,
                                        uiInfo.serverStatus.displayServers[uiInfo.serverStatus.currentServer],
                                        buff, MAX_STRING_CHARS );
                addr[0] = '\0';
                Q_strncpyz( addr,  Info_ValueForKey( buff, "addr" ), MAX_NAME_LENGTH );
                
                if( strlen( addr ) > 0 )
                    trap_LAN_RemoveServer( AS_FAVORITES, addr );
            }
        }
        else if( Q_stricmp( name, "createFavorite" ) == 0 )
        {
            if( ui_netSource.integer == AS_FAVORITES )
            {
                UTF8 name[MAX_NAME_LENGTH];
                UTF8 addr[MAX_NAME_LENGTH];
                S32 res;
                
                name[0] = addr[0] = '\0';
                Q_strncpyz( name,  UI_Cvar_VariableString( "ui_favoriteName" ), MAX_NAME_LENGTH );
                Q_strncpyz( addr,  UI_Cvar_VariableString( "ui_favoriteAddress" ), MAX_NAME_LENGTH );
                
                if( strlen( name ) > 0 && strlen( addr ) > 0 )
                {
                    res = trap_LAN_AddServer( AS_FAVORITES, name, addr );
                    
                    if( res == 0 )
                    {
                        // server already in the list
                        Com_Printf( "Favorite already in list\n" );
                    }
                    else if( res == -1 )
                    {
                        // list full
                        Com_Printf( "Favorite list full\n" );
                    }
                    else
                    {
                        // successfully added
                        Com_Printf( "Added favorite server %s\n", addr );
                    }
                }
            }
        }
        else if( Q_stricmp( name, "glCustom" ) == 0 )
            trap_Cvar_Set( "ui_glCustom", "4" );
        else if( Q_stricmp( name, "update" ) == 0 )
        {
            if( String_Parse( args, &name2 ) )
                UI_Update( name2 );
        }
        else if( Q_stricmp( name, "InitIgnoreList" ) == 0 )
            UI_BuildPlayerList();
        else if( Q_stricmp( name, "ToggleIgnore" ) == 0 )
        {
            if( uiInfo.ignoreIndex >= 0 && uiInfo.ignoreIndex < uiInfo.playerCount )
            {
                if( bggame->ClientListTest( &uiInfo.ignoreList[ uiInfo.myPlayerIndex ],
                                            uiInfo.clientNums[ uiInfo.ignoreIndex ] ) )
                {
                    bggame->ClientListRemove( &uiInfo.ignoreList[ uiInfo.myPlayerIndex ],
                                              uiInfo.clientNums[ uiInfo.ignoreIndex ] );
                    trap_Cmd_ExecuteText( EXEC_APPEND, va( "unignore %i\n",
                                                           uiInfo.clientNums[ uiInfo.ignoreIndex ] ) );
                }
                else
                {
                    bggame->ClientListAdd( &uiInfo.ignoreList[ uiInfo.myPlayerIndex ],
                                           uiInfo.clientNums[ uiInfo.ignoreIndex ] );
                    trap_Cmd_ExecuteText( EXEC_APPEND, va( "ignore %i\n",
                                                           uiInfo.clientNums[ uiInfo.ignoreIndex ] ) );
                }
            }
        }
        else if( Q_stricmp( name, "IgnorePlayer" ) == 0 )
        {
            if( uiInfo.ignoreIndex >= 0 && uiInfo.ignoreIndex < uiInfo.playerCount )
            {
                if( !bggame->ClientListTest( &uiInfo.ignoreList[ uiInfo.myPlayerIndex ],
                                             uiInfo.clientNums[ uiInfo.ignoreIndex ] ) )
                {
                    bggame->ClientListAdd( &uiInfo.ignoreList[ uiInfo.myPlayerIndex ],
                                           uiInfo.clientNums[ uiInfo.ignoreIndex ] );
                    trap_Cmd_ExecuteText( EXEC_APPEND, va( "ignore %i\n",
                                                           uiInfo.clientNums[ uiInfo.ignoreIndex ] ) );
                }
            }
        }
        else if( Q_stricmp( name, "UnIgnorePlayer" ) == 0 )
        {
            if( uiInfo.ignoreIndex >= 0 && uiInfo.ignoreIndex < uiInfo.playerCount )
            {
                if( bggame->ClientListTest( &uiInfo.ignoreList[ uiInfo.myPlayerIndex ],
                                            uiInfo.clientNums[ uiInfo.ignoreIndex ] ) )
                {
                    bggame->ClientListRemove( &uiInfo.ignoreList[ uiInfo.myPlayerIndex ],
                                              uiInfo.clientNums[ uiInfo.ignoreIndex ] );
                    trap_Cmd_ExecuteText( EXEC_APPEND, va( "unignore %i\n",
                                                           uiInfo.clientNums[ uiInfo.ignoreIndex ] ) );
                }
            }
        }
        else if( Q_stricmp( name, "ScreenChange" ) == 0 )
        {
            UI_ScreenChange( args );
        }
        else
            Com_Printf( "unknown UI script %s\n", name );
    }
}

/*
==================
UI_FeederCount
==================
*/
static S32 UI_FeederCount( F32 feederID )
{
    if( feederID == FEEDER_CINEMATICS )
        return uiInfo.movieCount;
    else if( feederID == FEEDER_MAPS )
        return uiInfo.mapCount;
    else if( feederID == FEEDER_SERVERS )
        return uiInfo.serverStatus.numDisplayServers;
    else if( feederID == FEEDER_SERVERSTATUS )
        return uiInfo.serverStatusInfo.numLines;
    else if( feederID == FEEDER_FINDPLAYER )
        return uiInfo.numFoundPlayerServers;
    else if( feederID == FEEDER_PLAYER_LIST )
    {
        if( uiInfo.uiDC.realTime > uiInfo.playerRefresh )
        {
            uiInfo.playerRefresh = uiInfo.uiDC.realTime + 3000;
            UI_BuildPlayerList();
        }
        
        return uiInfo.playerCount;
    }
    else if( feederID == FEEDER_TEAM_LIST )
    {
        if( uiInfo.uiDC.realTime > uiInfo.playerRefresh )
        {
            uiInfo.playerRefresh = uiInfo.uiDC.realTime + 3000;
            UI_BuildPlayerList();
        }
        
        return uiInfo.myTeamCount;
    }
    else if( feederID == FEEDER_IGNORE_LIST )
        return uiInfo.playerCount;
    else if( feederID == FEEDER_HELP_LIST )
        return uiInfo.helpCount;
    else if( feederID == FEEDER_MODS )
        return uiInfo.modCount;
    else if( feederID == FEEDER_DEMOS )
        return uiInfo.demoCount;
    else if( feederID == FEEDER_TREMTEAMS )
        return uiInfo.teamCount;
    else if( feederID == FEEDER_TREMHUMANITEMS )
        return uiInfo.humanItemCount;
    else if( feederID == FEEDER_TREMALIENCLASSES )
        return uiInfo.alienClassCount;
    else if( feederID == FEEDER_TREMHUMANARMOURYBUY )
        return uiInfo.humanArmouryBuyCount;
    else if( feederID == FEEDER_TREMHUMANARMOURYSELL )
        return uiInfo.humanArmourySellCount;
    else if( feederID == FEEDER_TREMALIENUPGRADE )
        return uiInfo.alienUpgradeCount;
    else if( feederID == FEEDER_TREMALIENBUILD )
        return uiInfo.alienBuildCount;
    else if( feederID == FEEDER_TREMHUMANBUILD )
        return uiInfo.humanBuildCount;
    else if( feederID == FEEDER_RESOLUTIONS )
        return uiInfo.numResolutions;
        
    return 0;
}

static StringEntry UI_SelectedMap( S32 index, S32* actual )
{
    S32 i, c;
    c = 0;
    *actual = 0;
    
    for( i = 0; i < uiInfo.mapCount; i++ )
    {
        if( c == index )
        {
            *actual = i;
            return uiInfo.mapList[i].mapName;
        }
        else
            c++;
    }
    
    return "";
}

static StringEntry UI_FeederItemText( F32 feederID, S32 index, S32 column, qhandle_t* handle )
{
    static UTF8 info[MAX_STRING_CHARS];
    static UTF8 hostname[1024];
    static UTF8 clientBuff[32];
    static UTF8 resolution[MAX_STRING_CHARS];
    static S32 lastColumn = -1;
    static S32 lastTime = 0;
    
    if( handle )
        *handle = -1;
        
    if( feederID == FEEDER_MAPS )
    {
        S32 actual;
        return UI_SelectedMap( index, &actual );
    }
    else if( feederID == FEEDER_SERVERS )
    {
        if( index >= 0 && index < uiInfo.serverStatus.numDisplayServers )
        {
            S32 ping;
            
            if( lastColumn != column || lastTime > uiInfo.uiDC.realTime + 5000 )
            {
                trap_LAN_GetServerInfo( ui_netSource.integer, uiInfo.serverStatus.displayServers[index],
                                        info, MAX_STRING_CHARS );
                lastColumn = column;
                lastTime = uiInfo.uiDC.realTime;
            }
            
            ping = atoi( Info_ValueForKey( info, "ping" ) );
            
            if( ping == -1 )
            {
                // if we ever see a ping that is out of date, do a server refresh
                // UI_UpdatePendingPings();
            }
            
            switch( column )
            {
                case SORT_HOST:
                    if( ping <= 0 )
                        return Info_ValueForKey( info, "addr" );
                    else
                    {
                        if( ui_netSource.integer == AS_LOCAL )
                        {
                            Com_sprintf( hostname, sizeof( hostname ), "%s [%s]",
                                         Info_ValueForKey( info, "hostname" ),
                                         netnames[atoi( Info_ValueForKey( info, "nettype" ) )] );
                            return hostname;
                        }
                        else
                        {
                            UTF8* text;
                            
                            Com_sprintf( hostname, sizeof( hostname ), "%s", Info_ValueForKey( info, "hostname" ) );
                            
                            // Strip leading whitespace
                            text = hostname;
                            
                            while( *text != '\0' && *text == ' ' )
                                text++;
                                
                            return text;
                        }
                    }
                    
                case SORT_MAP:
                    return Info_ValueForKey( info, "mapname" );
                    
                case SORT_CLIENTS:
                    Com_sprintf( clientBuff, sizeof( clientBuff ), "%s (%s)",
                                 Info_ValueForKey( info, "clients" ), Info_ValueForKey( info, "sv_maxclients" ) );
                    return clientBuff;
                    
                case SORT_PING:
                    if( ping <= 0 )
                        return "...";
                    else
                        return Info_ValueForKey( info, "ping" );
            }
        }
    }
    else if( feederID == FEEDER_SERVERSTATUS )
    {
        if( index >= 0 && index < uiInfo.serverStatusInfo.numLines )
        {
            if( column >= 0 && column < 4 )
                return uiInfo.serverStatusInfo.lines[index][column];
        }
    }
    else if( feederID == FEEDER_FINDPLAYER )
    {
        if( index >= 0 && index < uiInfo.numFoundPlayerServers )
        {
            //return uiInfo.foundPlayerServerAddresses[index];
            return uiInfo.foundPlayerServerNames[index];
        }
    }
    else if( feederID == FEEDER_PLAYER_LIST )
    {
        if( index >= 0 && index < uiInfo.playerCount )
            return uiInfo.playerNames[index];
    }
    else if( feederID == FEEDER_TEAM_LIST )
    {
        if( index >= 0 && index < uiInfo.myTeamCount )
            return uiInfo.teamNames[index];
    }
    else if( feederID == FEEDER_IGNORE_LIST )
    {
        if( index >= 0 && index < uiInfo.playerCount )
        {
            switch( column )
            {
                case 1:
                    // am I ignoring him
                    return ( bggame->ClientListTest( &uiInfo.ignoreList[ uiInfo.myPlayerIndex ],
                                                     uiInfo.clientNums[ index ] ) ) ? "X" : "";
                                                     
                case 2:
                    // is he ignoring me
                    return ( bggame->ClientListTest( &uiInfo.ignoreList[ index ],
                                                     uiInfo.playerNumber ) ) ? "X" : "";
                                                     
                default:
                    return uiInfo.playerNames[index];
            }
        }
    }
    else if( feederID == FEEDER_HELP_LIST )
    {
        if( index >= 0 && index < uiInfo.helpCount )
            return uiInfo.helpList[ index ].text;
    }
    else if( feederID == FEEDER_MODS )
    {
        if( index >= 0 && index < uiInfo.modCount )
        {
            if( uiInfo.modList[index].modDescr && *uiInfo.modList[index].modDescr )
                return uiInfo.modList[index].modDescr;
            else
                return uiInfo.modList[index].modName;
        }
    }
    else if( feederID == FEEDER_CINEMATICS )
    {
        if( index >= 0 && index < uiInfo.movieCount )
            return uiInfo.movieList[index];
    }
    else if( feederID == FEEDER_DEMOS )
    {
        if( index >= 0 && index < uiInfo.demoCount )
            return uiInfo.demoList[index];
    }
    else if( feederID == FEEDER_TREMTEAMS )
    {
        if( index >= 0 && index < uiInfo.teamCount )
            return uiInfo.teamList[ index ].text;
    }
    else if( feederID == FEEDER_TREMHUMANITEMS )
    {
        if( index >= 0 && index < uiInfo.humanItemCount )
            return uiInfo.humanItemList[ index ].text;
    }
    else if( feederID == FEEDER_TREMALIENCLASSES )
    {
        if( index >= 0 && index < uiInfo.alienClassCount )
            return uiInfo.alienClassList[ index ].text;
    }
    else if( feederID == FEEDER_TREMHUMANARMOURYBUY )
    {
        if( index >= 0 && index < uiInfo.humanArmouryBuyCount )
            return uiInfo.humanArmouryBuyList[ index ].text;
    }
    else if( feederID == FEEDER_TREMHUMANARMOURYSELL )
    {
        if( index >= 0 && index < uiInfo.humanArmourySellCount )
            return uiInfo.humanArmourySellList[ index ].text;
    }
    else if( feederID == FEEDER_TREMALIENUPGRADE )
    {
        if( index >= 0 && index < uiInfo.alienUpgradeCount )
            return uiInfo.alienUpgradeList[ index ].text;
    }
    else if( feederID == FEEDER_TREMALIENBUILD )
    {
        if( index >= 0 && index < uiInfo.alienBuildCount )
            return uiInfo.alienBuildList[ index ].text;
    }
    else if( feederID == FEEDER_TREMHUMANBUILD )
    {
        if( index >= 0 && index < uiInfo.humanBuildCount )
            return uiInfo.humanBuildList[ index ].text;
    }
    else if( feederID == FEEDER_RESOLUTIONS )
    {
        if( uiInfo.oldResolutions )
        {
            S32 mode = trap_Cvar_VariableValue( "r_mode" );
            if( mode < 0 || mode >= uiInfo.numResolutions )
            {
                Com_sprintf( resolution, sizeof( resolution ), "Custom (%dx%d)",
                             ( S32 )trap_Cvar_VariableValue( "r_customWidth" ),
                             ( S32 )trap_Cvar_VariableValue( "r_customHeight" ) );
            }
            else
            {
                Com_sprintf( resolution, sizeof( resolution ), "%dx%d",
                             uiInfo.resolutions[ mode ].w,
                             uiInfo.resolutions[ mode ].h );
            }
            return resolution;
        }
        else
        {
            S32 i;
            S32 w = trap_Cvar_VariableValue( "r_width" );
            S32 h = trap_Cvar_VariableValue( "r_height" );
            
            for( i = 0; i < uiInfo.numResolutions; i++ )
            {
                if( w == uiInfo.resolutions[ i ].w && h == uiInfo.resolutions[ i ].h )
                {
                    Com_sprintf( resolution, sizeof( resolution ), "%dx%d", w, h );
                    return resolution;
                }
            }
            
            Com_sprintf( resolution, sizeof( resolution ), "Custom (%dx%d)", w, h );
            return resolution;
        }
    }
    
    return "";
}


static qhandle_t UI_FeederItemImage( F32 feederID, S32 index )
{
    if( feederID == FEEDER_MAPS )
    {
        S32 actual;
        UI_SelectedMap( index, &actual );
        index = actual;
        
        if( index >= 0 && index < uiInfo.mapCount )
        {
            if( uiInfo.mapList[index].levelShot == -1 )
                uiInfo.mapList[index].levelShot = trap_R_RegisterShaderNoMip( uiInfo.mapList[index].imageName );
                
            return uiInfo.mapList[index].levelShot;
        }
    }
    
    return 0;
}

static void UI_FeederSelection( F32 feederID, S32 index )
{
    static UTF8 info[MAX_STRING_CHARS];
    
    if( feederID == FEEDER_MAPS )
    {
        S32 actual, map;
        map = ui_selectedMap.integer;
        
        if( uiInfo.mapList[map].cinematic >= 0 )
        {
            trap_CIN_StopCinematic( uiInfo.mapList[map].cinematic );
            uiInfo.mapList[map].cinematic = -1;
        }
        
        UI_SelectedMap( index, &actual );
        
        ui_selectedMap.integer = actual;
        trap_Cvar_Set( "ui_selectedMap", va( "%d", actual ) );
        uiInfo.mapList[ui_selectedMap.integer].cinematic =
            trap_CIN_PlayCinematic( va( "%s.roq", uiInfo.mapList[ui_selectedMap.integer].mapLoadName ),
                                    0, 0, 0, 0, ( CIN_loop | CIN_silent ) );
    }
    else if( feederID == FEEDER_SERVERS )
    {
        StringEntry mapName = NULL;
        uiInfo.serverStatus.currentServer = index;
        trap_LAN_GetServerInfo( ui_netSource.integer, uiInfo.serverStatus.displayServers[index],
                                info, MAX_STRING_CHARS );
        uiInfo.serverStatus.currentServerPreview =
            trap_R_RegisterShaderNoMip( va( "levelshots/%s", Info_ValueForKey( info, "mapname" ) ) );
            
        if( uiInfo.serverStatus.currentServerCinematic >= 0 )
        {
            trap_CIN_StopCinematic( uiInfo.serverStatus.currentServerCinematic );
            uiInfo.serverStatus.currentServerCinematic = -1;
        }
        
        mapName = Info_ValueForKey( info, "mapname" );
        
        if( mapName && *mapName )
        {
            uiInfo.serverStatus.currentServerCinematic =
                trap_CIN_PlayCinematic( va( "%s.roq", mapName ), 0, 0, 0, 0, ( CIN_loop | CIN_silent ) );
        }
    }
    else if( feederID == FEEDER_SERVERSTATUS )
    {
    }
    else if( feederID == FEEDER_FINDPLAYER )
    {
        uiInfo.currentFoundPlayerServer = index;
        //
        
        if( index < uiInfo.numFoundPlayerServers - 1 )
        {
            // build a new server status for this server
            Q_strncpyz( uiInfo.serverStatusAddress,
                        uiInfo.foundPlayerServerAddresses[uiInfo.currentFoundPlayerServer],
                        sizeof( uiInfo.serverStatusAddress ) );
            Menu_SetFeederSelection( NULL, FEEDER_SERVERSTATUS, 0, NULL );
            UI_BuildServerStatus( true );
        }
    }
    else if( feederID == FEEDER_PLAYER_LIST )
        uiInfo.playerIndex = index;
    else if( feederID == FEEDER_TEAM_LIST )
        uiInfo.teamPlayerIndex = index;
    else if( feederID == FEEDER_IGNORE_LIST )
        uiInfo.ignoreIndex = index;
    else if( feederID == FEEDER_HELP_LIST )
        uiInfo.helpIndex = index;
    else if( feederID == FEEDER_MODS )
        uiInfo.modIndex = index;
    else if( feederID == FEEDER_CINEMATICS )
    {
        uiInfo.movieIndex = index;
        
        if( uiInfo.previewMovie >= 0 )
            trap_CIN_StopCinematic( uiInfo.previewMovie );
            
        uiInfo.previewMovie = -1;
    }
    else if( feederID == FEEDER_DEMOS )
        uiInfo.demoIndex = index;
    else if( feederID == FEEDER_TREMTEAMS )
        uiInfo.teamIndex = index;
    else if( feederID == FEEDER_TREMHUMANITEMS )
        uiInfo.humanItemIndex = index;
    else if( feederID == FEEDER_TREMALIENCLASSES )
        uiInfo.alienClassIndex = index;
    else if( feederID == FEEDER_TREMHUMANARMOURYBUY )
        uiInfo.humanArmouryBuyIndex = index;
    else if( feederID == FEEDER_TREMHUMANARMOURYSELL )
        uiInfo.humanArmourySellIndex = index;
    else if( feederID == FEEDER_TREMALIENUPGRADE )
        uiInfo.alienUpgradeIndex = index;
    else if( feederID == FEEDER_TREMALIENBUILD )
        uiInfo.alienBuildIndex = index;
    else if( feederID == FEEDER_TREMHUMANBUILD )
        uiInfo.humanBuildIndex = index;
    else if( feederID == FEEDER_RESOLUTIONS )
    {
        if( uiInfo.oldResolutions )
            trap_Cvar_Set( "r_mode", va( "%d", index ) );
        else
        {
            trap_Cvar_Set( "r_width", va( "%d", uiInfo.resolutions[ index ].w ) );
            trap_Cvar_Set( "r_height", va( "%d", uiInfo.resolutions[ index ].h ) );
        }
    }
}

static S32 UI_FeederInitialise( F32 feederID )
{
    if( feederID == FEEDER_RESOLUTIONS )
    {
        if( uiInfo.oldResolutions )
            return trap_Cvar_VariableValue( "r_mode" );
        else
        {
            S32 i;
            S32 w = trap_Cvar_VariableValue( "r_width" );
            S32 h = trap_Cvar_VariableValue( "r_height" );
            
            for( i = 0; i < uiInfo.numResolutions; i++ )
            {
                if( w == uiInfo.resolutions[ i ].w && h == uiInfo.resolutions[ i ].h )
                    return i;
            }
        }
    }
    
    return 0;
}

static void UI_Pause( bool b )
{
    if( b )
    {
        // pause the game and set the ui keycatcher
        trap_Cvar_Set( "cl_paused", "1" );
        trap_Key_SetCatcher( KEYCATCH_UI );
    }
    else
    {
        // unpause the game and clear the ui keycatcher
        trap_Key_SetCatcher( trap_Key_GetCatcher() & ~KEYCATCH_UI );
        trap_Key_ClearStates();
        trap_Cvar_Set( "cl_paused", "0" );
    }
}

static S32 UI_PlayCinematic( StringEntry name, F32 x, F32 y, F32 w, F32 h )
{
    return trap_CIN_PlayCinematic( name, x, y, w, h, ( CIN_loop | CIN_silent ) );
}

static void UI_StopCinematic( S32 handle )
{
    if( handle >= 0 )
        trap_CIN_StopCinematic( handle );
    else
    {
        handle = abs( handle );
        
        if( handle == UI_NETMAPCINEMATIC )
        {
            if( uiInfo.serverStatus.currentServerCinematic >= 0 )
            {
                trap_CIN_StopCinematic( uiInfo.serverStatus.currentServerCinematic );
                uiInfo.serverStatus.currentServerCinematic = -1;
            }
        }
    }
}

static void UI_DrawCinematic( S32 handle, F32 x, F32 y, F32 w, F32 h )
{
    trap_CIN_SetExtents( handle, x, y, w, h );
    trap_CIN_DrawCinematic( handle );
}

static void UI_RunCinematicFrame( S32 handle )
{
    trap_CIN_RunCinematic( handle );
}

static F32 UI_GetValue( S32 ownerDraw )
{
    return 0.0f;
}

/*
=================
UI_ParseResolutions
=================
*/
void UI_ParseResolutions( void )
{
    UTF8        buf[ MAX_STRING_CHARS ];
    UTF8        w[ 16 ], h[ 16 ];
    UTF8*        p;
    StringEntry  out;
    UTF8*        s = NULL;
    
    trap_Cvar_VariableStringBuffer( "r_availableModes", buf, sizeof( buf ) );
    if( buf[0] )
    {
        p = buf;
        uiInfo.oldResolutions = false;
    }
    else
    {
        p = "320x240 400x300 512x384 640x480 800x600 960x720 1024x768 1152x864 1280x1024 1600x1200 2048x1536 856x480";
        uiInfo.oldResolutions = true;
    }
    uiInfo.numResolutions = 0;
    
    while( String_Parse( &p, &out ) )
    {
        Q_strncpyz( w, out, sizeof( w ) );
        s = strchr( w, 'x' );
        if( !s )
            return;
            
        *s++ = '\0';
        Q_strncpyz( h, s, sizeof( h ) );
        
        uiInfo.resolutions[ uiInfo.numResolutions ].w = atoi( w );
        uiInfo.resolutions[ uiInfo.numResolutions ].h = atoi( h );
        uiInfo.numResolutions++;
    }
}

/*
=================
idUserInterfaceManagerLocal::Init
=================
*/
void idUserInterfaceManagerLocal::Init( bool inGameLoad )
{
    S32 start;
    
    bggame->InitClassConfigs( );
    bggame->InitAllowedGameElements( );
    
    uiInfo.inGameLoad = inGameLoad;
    
    UI_RegisterCvars();
    UI_InitMemory();
    
    // cache redundant calulations
    trap_GetGlconfig( &uiInfo.uiDC.glconfig );
    
    // for 640x480 virtualized screen
    uiInfo.uiDC.yscale = uiInfo.uiDC.glconfig.vidHeight * ( 1.0f / 480.0f );
    uiInfo.uiDC.xscale = uiInfo.uiDC.glconfig.vidWidth * ( 1.0f / 640.0f );
    
    // wide screen
    uiInfo.uiDC.aspectScale = ( ( 640.0f * uiInfo.uiDC.glconfig.vidHeight ) /
                                ( 480.0f * uiInfo.uiDC.glconfig.vidWidth ) );
                                
    uiInfo.uiDC.registerShaderNoMip = &trap_R_RegisterShaderNoMip;
    uiInfo.uiDC.setColor = &UI_SetColor;
    uiInfo.uiDC.drawHandlePic = &UI_DrawHandlePic;
    uiInfo.uiDC.drawStretchPic = &trap_R_DrawStretchPic;
    uiInfo.uiDC.registerModel = &trap_R_RegisterModel;
    uiInfo.uiDC.modelBounds = &trap_R_ModelBounds;
    uiInfo.uiDC.fillRect = &UI_FillRect;
    uiInfo.uiDC.drawRect = &UI_DrawRect;
    uiInfo.uiDC.fillRoundedRect = &UI_FillRoundedRect;
    uiInfo.uiDC.drawRoundedRect = &UI_DrawRoundedRect;
    uiInfo.uiDC.drawSides = &UI_DrawSides;
    uiInfo.uiDC.drawTopBottom = &UI_DrawTopBottom;
    uiInfo.uiDC.clearScene = &trap_R_ClearScene;
    uiInfo.uiDC.drawSides = &UI_DrawSides;
    uiInfo.uiDC.addRefEntityToScene = &trap_R_AddRefEntityToScene;
    uiInfo.uiDC.renderScene = &trap_R_RenderScene;
    uiInfo.uiDC.registerFont = &trap_R_RegisterFont;
    uiInfo.uiDC.ownerDrawItem = &UI_OwnerDraw;
    uiInfo.uiDC.getValue = &UI_GetValue;
    uiInfo.uiDC.ownerDrawVisible = &UI_OwnerDrawVisible;
    uiInfo.uiDC.runScript = &UI_RunMenuScript;
    uiInfo.uiDC.setCVar = trap_Cvar_Set;
    uiInfo.uiDC.getCVarString = trap_Cvar_VariableStringBuffer;
    uiInfo.uiDC.getCVarValue = trap_Cvar_VariableValue;
    uiInfo.uiDC.setOverstrikeMode = &trap_Key_SetOverstrikeMode;
    uiInfo.uiDC.getOverstrikeMode = &trap_Key_GetOverstrikeMode;
    uiInfo.uiDC.startLocalSound = &trap_S_StartLocalSound;
    uiInfo.uiDC.ownerDrawHandleKey = &UI_OwnerDrawHandleKey;
    uiInfo.uiDC.feederCount = &UI_FeederCount;
    uiInfo.uiDC.feederItemImage = &UI_FeederItemImage;
    uiInfo.uiDC.feederItemText = &UI_FeederItemText;
    uiInfo.uiDC.feederSelection = &UI_FeederSelection;
    uiInfo.uiDC.feederInitialise = &UI_FeederInitialise;
    uiInfo.uiDC.setBinding = &trap_Key_SetBinding;
    uiInfo.uiDC.getBindingBuf = &trap_Key_GetBindingBuf;
    uiInfo.uiDC.keynumToStringBuf = &trap_Key_KeynumToStringBuf;
    uiInfo.uiDC.executeText = &trap_Cmd_ExecuteText;
    uiInfo.uiDC.Error = &Com_Error;
    uiInfo.uiDC.Print = &Com_Printf;
    uiInfo.uiDC.Pause = &UI_Pause;
    uiInfo.uiDC.ownerDrawWidth = &UI_OwnerDrawWidth;
    uiInfo.uiDC.ownerDrawText = &UI_OwnerDrawText;
    uiInfo.uiDC.registerSound = &trap_S_RegisterSound;
    uiInfo.uiDC.startBackgroundTrack = &trap_S_StartBackgroundTrack;
    uiInfo.uiDC.stopBackgroundTrack = &trap_S_StopBackgroundTrack;
    uiInfo.uiDC.playCinematic = &UI_PlayCinematic;
    uiInfo.uiDC.stopCinematic = &UI_StopCinematic;
    uiInfo.uiDC.drawCinematic = &UI_DrawCinematic;
    uiInfo.uiDC.runCinematicFrame = &UI_RunCinematicFrame;
    uiInfo.uiDC.getFileList = &trap_FS_GetFileList;
    
    Init_Display( &uiInfo.uiDC );
    
    String_Init();
    
    uiInfo.uiDC.whiteShader = trap_R_RegisterShaderNoMip( "white" );
    
    AssetCache();
    
    start = trap_Milliseconds();
    
    UI_LoadMenus( "gui/menus.txt", true );
    UI_LoadMenus( "gui/ingame.txt", false );
    UI_LoadMenus( "gui/tremulous.txt", false );
    UI_LoadHelp( "gui/help.txt" );
    
    Menus_CloseAll( true );
    
    trap_LAN_LoadCachedServers();
    
    // sets defaults for ui temp cvars
    trap_Cvar_Set( "ui_mousePitch", ( trap_Cvar_VariableValue( "m_pitch" ) >= 0 ) ? "0" : "1" );
    
    uiInfo.serverStatus.currentServerCinematic = -1;
    uiInfo.previewMovie = -1;
    
    trap_Cvar_Register( NULL, "debug_protocol", "", 0 );
    
    UI_ParseResolutions( );
}


/*
=================
idUserInterfaceManagerLocal::KeyEvent
=================
*/
void idUserInterfaceManagerLocal::KeyEvent( S32 key, bool down )
{
    if( Menu_Count() > 0 )
    {
        menuDef_t* menu = Menu_GetFocused();
        
        if( menu )
        {
            if( key == K_ESCAPE && down && !Menus_AnyFullScreenVisible() )
                Menus_CloseAll( true );
            else
                Menu_HandleKey( menu, key, down );
        }
        else
        {
            trap_Key_SetCatcher( trap_Key_GetCatcher() & ~KEYCATCH_UI );
            trap_Key_ClearStates();
            trap_Cvar_Set( "cl_paused", "0" );
        }
    }
}

/*
=================
idUserInterfaceManagerLocal::MouseEvent
=================
*/
void idUserInterfaceManagerLocal::MouseEvent( S32 dx, S32 dy )
{
    // update mouse screen position
    uiInfo.uiDC.cursorx += ( dx * uiInfo.uiDC.aspectScale );
    
    if( uiInfo.uiDC.cursorx < 0 )
        uiInfo.uiDC.cursorx = 0;
    else if( uiInfo.uiDC.cursorx > SCREEN_WIDTH )
        uiInfo.uiDC.cursorx = SCREEN_WIDTH;
        
    uiInfo.uiDC.cursory += dy;
    
    if( uiInfo.uiDC.cursory < 0 )
        uiInfo.uiDC.cursory = 0;
    else if( uiInfo.uiDC.cursory > SCREEN_HEIGHT )
        uiInfo.uiDC.cursory = SCREEN_HEIGHT;
        
    uiInfo.uiDC.cursordx = dx * uiInfo.uiDC.aspectScale;
    uiInfo.uiDC.cursordy = dy;
    
    if( Menu_Count( ) > 0 )
        Display_MouseMove( NULL, uiInfo.uiDC.cursorx, uiInfo.uiDC.cursory );
}

static uiMenuCommand_t menutype = UIMENU_NONE;

uiMenuCommand_t idUserInterfaceManagerLocal::GetActiveMenu( void )
{
    return menutype;
}

/*
=================
idUserInterfaceManagerLocal::MousePosition
=================
*/
S32 idUserInterfaceManagerLocal::MousePosition( void )
{
    return ( S32 )( F32 )rint( ( F32 )uiInfo.uiDC.cursorx ) |
           ( S32 )( F32 )rint( ( F32 )uiInfo.uiDC.cursory ) << 16;
}

/*
=================
idUserInterfaceManagerLocal::SetMousePosition
=================
*/
void idUserInterfaceManagerLocal::SetMousePosition( S32 x, S32 y )
{
    uiInfo.uiDC.cursordx = x - uiInfo.uiDC.cursorx;
    uiInfo.uiDC.cursordy = y - uiInfo.uiDC.cursory;
    uiInfo.uiDC.cursorx = x;
    uiInfo.uiDC.cursory = y;
    
    if( Menu_Count( ) > 0 )
        Display_MouseMove( NULL, uiInfo.uiDC.cursorx, uiInfo.uiDC.cursory );
}

#define MISSING_FILES_MSG "The following packs are missing:"

/*
=================
idUserInterfaceManagerLocal::SetActiveMenu
=================
*/
void idUserInterfaceManagerLocal::SetActiveMenu( uiMenuCommand_t menu )
{
    UTF8 buf[4096]; // com_errorMessage can go up to 4096
    UTF8* missing_files;
    
    // this should be the ONLY way the menu system is brought up
    // enusure minumum menu data is cached
    
    if( Menu_Count() > 0 )
    {
        vec3_t v;
        v[0] = v[1] = v[2] = 0;
        
        menutype = menu;
        
        switch( menu )
        {
            case UIMENU_NONE:
                trap_Key_SetCatcher( trap_Key_GetCatcher() & ~KEYCATCH_UI );
                trap_Key_ClearStates();
                trap_Cvar_Set( "cl_paused", "0" );
                Menus_CloseAll( true );
                
                return;
                
            case UIMENU_MAIN:
                trap_Key_SetCatcher( KEYCATCH_UI );
                Menus_CloseAll( true );
                Menus_ActivateByName( "backgroundmusic" );
                // makes sure it doesn't get restarted every time you reach the main menu
                if( !cl_profile->string[0] )
                {
                    //Menus_ActivateByName( "profilelogin", qtrue );
                    // FIXME: initial profile popup
                    // FIXED: handled in opener now
                    Menus_ActivateByName( "main" );
                }
                else
                {
                    Menus_ActivateByName( "main" );
                }
                
                trap_Cvar_VariableStringBuffer( "com_errorMessage", buf, sizeof( buf ) );
                
                if( ( *buf ) && ( Q_stricmp( buf, ";" ) ) )
                {
                    trap_Cvar_Set( "ui_connecting", "0" );
                    if( !Q_stricmpn( buf, "Invalid password", 16 ) )
                    {
                        trap_Cvar_Set( "com_errorMessage", trap_TranslateString( buf ) );
                        Menus_ActivateByName( "popupPassword" );
                    }
                    else if( strlen( buf ) > 5 && !Q_stricmpn( buf, "OW://", 5 ) )
                    {
                        Q_strncpyz( buf, buf + 5, sizeof( buf ) );
                        Com_Printf( "Server is full, redirect to: %s\n", buf );
                        switch( ui_autoredirect.integer )
                        {
                                //auto-redirect
                            case 1:
                                trap_Cvar_Set( "com_errorMessage", "" );
                                trap_Cmd_ExecuteText( EXEC_APPEND, va( "connect %s\n", buf ) );
                                break;
                                //prompt (default)
                            default:
                                trap_Cvar_Set( "com_errorMessage", buf );
                                Menus_ActivateByName( "popupServerRedirect" );
                                break;
                        }
                    }
                    else
                    {
                        trap_Cvar_Set( "com_errorMessage", trap_TranslateString( buf ) );
                        if( UI_Cvar_VariableString( "com_errorDiagnoseIP" )[0] )
                        {
                            missing_files = UI_Cvar_VariableString( "com_missingFiles" );
                            if( missing_files[0] )
                            {
                                trap_Cvar_Set( "com_errorMessage",
                                               va( "%s\n\n%s\n%s",
                                                   UI_Cvar_VariableString( "com_errorMessage" ),
                                                   trap_TranslateString( MISSING_FILES_MSG ),
                                                   missing_files ) );
                            }
                        }
                    }
                }
                
                return;
                
            case UIMENU_TEAM:
                trap_Key_SetCatcher( KEYCATCH_UI );
                Menus_ActivateByName( "team" );
                return;
                
            case UIMENU_INGAME:
                trap_Cvar_Set( "cl_paused", "1" );
                trap_Key_SetCatcher( KEYCATCH_UI );
                UI_BuildPlayerList();
                Menu_SetFeederSelection( NULL, FEEDER_PLAYER_LIST, 0, NULL );
                Menus_CloseAll( true );
                //trap_Cvar_Set( "authLevel", "0" ); // just used for testing...
                Menus_ActivateByName( "ingame" );
                return;
                
            case UIMENU_WM_AUTOUPDATE:
                return;
                
            default:
                return;
        }
    }
}

bool idUserInterfaceManagerLocal::IsFullscreen( void )
{
    return Menus_AnyFullScreenVisible( );
}



static connstate_t  lastConnState;
static UTF8     lastLoadingText[MAX_INFO_VALUE];

static void UI_ReadableSize( UTF8* buf, S32 bufsize, S32 value )
{
    if( value > 1024 * 1024 * 1024 )
    {
        // gigs
        Com_sprintf( buf, bufsize, "%d", value / ( 1024 * 1024 * 1024 ) );
        Com_sprintf( buf + strlen( buf ), bufsize - strlen( buf ), ".%02d GB",
                     ( value % ( 1024 * 1024 * 1024 ) ) * 100 / ( 1024 * 1024 * 1024 ) );
    }
    else if( value > 1024 * 1024 )
    {
        // megs
        Com_sprintf( buf, bufsize, "%d", value / ( 1024 * 1024 ) );
        Com_sprintf( buf + strlen( buf ), bufsize - strlen( buf ), ".%02d MB",
                     ( value % ( 1024 * 1024 ) ) * 100 / ( 1024 * 1024 ) );
    }
    else if( value > 1024 )
    {
        // kilos
        Com_sprintf( buf, bufsize, "%d KB", value / 1024 );
    }
    else
    {
        // bytes
        Com_sprintf( buf, bufsize, "%d bytes", value );
    }
}

// Assumes time is in msec
static void UI_PrintTime( UTF8* buf, S32 bufsize, S32 time )
{
    time /= 1000;  // change to seconds
    
    if( time > 3600 )
    {
        // in the hours range
        Com_sprintf( buf, bufsize, "%d hr %d min", time / 3600, ( time % 3600 ) / 60 );
    }
    else if( time > 60 )
    {
        // mins
        Com_sprintf( buf, bufsize, "%d min %d sec", time / 60, time % 60 );
    }
    else
    {
        // secs
        Com_sprintf( buf, bufsize, "%d sec", time );
    }
}

// FIXME: move to ui_shared.c?
void Text_PaintCenter( F32 x, F32 y, F32 scale, vec4_t color, StringEntry text, F32 adjust )
{
    S32 len = UI_Text_Width( text, scale, 0 );
    UI_Text_Paint( x - len / 2, y, scale, color, text, 0, 0, ITEM_TEXTSTYLE_SHADOWEDMORE );
}

void Text_PaintCenter_AutoWrapped( F32 x, F32 y, F32 xmax, F32 ystep, F32 scale, vec4_t color, StringEntry str, F32 adjust )
{
    S32 width;
    UTF8* s1, *s2, *s3;
    UTF8 c_bcp;
    UTF8 buf[1024];
    
    if( !str || str[0] == '\0' )
        return;
        
    Q_strncpyz( buf, str, sizeof( buf ) );
    
    s1 = s2 = s3 = buf;
    
    while( 1 )
    {
        do
            s3++;
        while( *s3 != ' ' && *s3 != '\0' );
        
        c_bcp = *s3;
        
        *s3 = '\0';
        
        width = UI_Text_Width( s1, scale, 0 );
        
        *s3 = c_bcp;
        
        if( width > xmax )
        {
            if( s1 == s2 )
            {
                // fuck, don't have a clean cut, we'll overflow
                s2 = s3;
            }
            
            *s2 = '\0';
            Text_PaintCenter( x, y, scale, color, s1, adjust );
            y += ystep;
            
            if( c_bcp == '\0' )
            {
                // that was the last word
                // we could start a new loop, but that wouldn't be much use
                // even if the word is too long, we would overflow it (see above)
                // so just print it now if needed
                s2++;
                
                if( *s2 != '\0' ) // if we are printing an overflowing line we have s2 == s3
                    Text_PaintCenter( x, y, scale, color, s2, adjust );
                    
                break;
            }
            
            s2++;
            s1 = s2;
            s3 = s2;
        }
        else
        {
            s2 = s3;
            
            if( c_bcp == '\0' ) // we reached the end
            {
                Text_PaintCenter( x, y, scale, color, s1, adjust );
                break;
            }
        }
    }
}


static void UI_DisplayDownloadInfo( StringEntry downloadName, F32 centerPoint, F32 yStart, F32 scale )
{
    static UTF8 dlText[]  = "Downloading:";
    static UTF8 etaText[] = "Estimated time left:";
    static UTF8 xferText[]  = "Transfer rate:";
    
    S32 downloadSize, downloadCount, downloadTime;
    UTF8 dlSizeBuf[64], totalSizeBuf[64], xferRateBuf[64], dlTimeBuf[64];
    S32 xferRate;
    S32 leftWidth;
    StringEntry s;
    
    downloadSize = trap_Cvar_VariableValue( "cl_downloadSize" );
    downloadCount = trap_Cvar_VariableValue( "cl_downloadCount" );
    downloadTime = trap_Cvar_VariableValue( "cl_downloadTime" );
    
    leftWidth = 320;
    
    UI_SetColor( colorWhite );
    Text_PaintCenter( centerPoint, yStart + 112, scale, colorWhite, dlText, 0 );
    Text_PaintCenter( centerPoint, yStart + 192, scale, colorWhite, etaText, 0 );
    Text_PaintCenter( centerPoint, yStart + 248, scale, colorWhite, xferText, 0 );
    
    if( downloadSize > 0 )
        s = va( "%s (%d%%)", downloadName, ( S32 )( ( F32 )downloadCount * 100.0f / downloadSize ) );
    else
        s = downloadName;
        
    Text_PaintCenter( centerPoint, yStart + 136, scale, colorWhite, s, 0 );
    
    UI_ReadableSize( dlSizeBuf,   sizeof dlSizeBuf,   downloadCount );
    UI_ReadableSize( totalSizeBuf,  sizeof totalSizeBuf,  downloadSize );
    
    if( downloadCount < 4096 || !downloadTime )
    {
        Text_PaintCenter( leftWidth, yStart + 216, scale, colorWhite, "estimating", 0 );
        Text_PaintCenter( leftWidth, yStart + 160, scale, colorWhite, va( "(%s of %s copied)", dlSizeBuf, totalSizeBuf ), 0 );
    }
    else
    {
        if( ( uiInfo.uiDC.realTime - downloadTime ) / 1000 )
            xferRate = downloadCount / ( ( uiInfo.uiDC.realTime - downloadTime ) / 1000 );
        else
            xferRate = 0;
            
        UI_ReadableSize( xferRateBuf, sizeof xferRateBuf, xferRate );
        
        // Extrapolate estimated completion time
        
        if( downloadSize && xferRate )
        {
            S32 n = downloadSize / xferRate; // estimated time for entire d/l in secs
            
            // We do it in K (/1024) because we'd overflow around 4MB
            UI_PrintTime( dlTimeBuf, sizeof dlTimeBuf,
                          ( n - ( ( ( downloadCount / 1024 ) * n ) / ( downloadSize / 1024 ) ) ) * 1000 );
                          
            Text_PaintCenter( leftWidth, yStart + 216, scale, colorWhite, dlTimeBuf, 0 );
            Text_PaintCenter( leftWidth, yStart + 160, scale, colorWhite, va( "(%s of %s copied)", dlSizeBuf, totalSizeBuf ), 0 );
        }
        else
        {
            Text_PaintCenter( leftWidth, yStart + 216, scale, colorWhite, "estimating", 0 );
            
            if( downloadSize )
                Text_PaintCenter( leftWidth, yStart + 160, scale, colorWhite, va( "(%s of %s copied)", dlSizeBuf, totalSizeBuf ), 0 );
            else
                Text_PaintCenter( leftWidth, yStart + 160, scale, colorWhite, va( "(%s copied)", dlSizeBuf ), 0 );
        }
        
        if( xferRate )
            Text_PaintCenter( leftWidth, yStart + 272, scale, colorWhite, va( "%s/Sec", xferRateBuf ), 0 );
    }
}

/*
========================
idUserInterfaceManagerLocal::DrawConnectScreen
========================
*/
void idUserInterfaceManagerLocal::DrawConnectScreen( bool overlay )
{
    UTF8*      s;
    uiClientState_t cstate;
    UTF8      info[MAX_INFO_VALUE];
    UTF8 text[256];
    F32 centerPoint, yStart, scale;
    
    menuDef_t* menu = Menus_FindByName( "Connect" );
    
    
    if( !overlay && menu )
        Menu_Paint( menu, true );
        
    if( !overlay )
    {
        centerPoint = 320;
        yStart = 130;
        scale = 0.5f;
    }
    else
    {
        centerPoint = 320;
        yStart = 32;
        scale = 0.6f;
        return;
    }
    
    // see what information we should display
    trap_GetClientState( &cstate );
    
    info[0] = '\0';
    
    if( trap_GetConfigString( CS_SERVERINFO, info, sizeof( info ) ) )
        Text_PaintCenter( centerPoint, yStart, scale, colorWhite, va( "Loading %s", Info_ValueForKey( info, "mapname" ) ), 0 );
        
    if( !Q_stricmp( cstate.servername, "localhost" ) )
        Text_PaintCenter( centerPoint, yStart + 48, scale, colorWhite,
                          "Starting up...", ITEM_TEXTSTYLE_SHADOWEDMORE );
    else
    {
        Com_sprintf( text, sizeof( text ), "Connecting to %s", cstate.servername );
        Text_PaintCenter( centerPoint, yStart + 48, scale, colorWhite, text , ITEM_TEXTSTYLE_SHADOWEDMORE );
    }
    
    
    // display global MOTD at bottom
    Text_PaintCenter( centerPoint, 600, scale, colorWhite, Info_ValueForKey( cstate.updateInfoString, "motd" ), 0 );
    
    // print any server info (server full, bad version, etc)
    if( cstate.connState < CA_CONNECTED )
        Text_PaintCenter( centerPoint, yStart + 176, scale, colorWhite, cstate.messageString, 0 );
        
    if( lastConnState > cstate.connState )
        lastLoadingText[0] = '\0';
        
    lastConnState = cstate.connState;
    
    switch( cstate.connState )
    {
        case CA_CONNECTING:
            s = va( "Awaiting connection...%i", cstate.connectPacketCount );
            break;
            
        case CA_CHALLENGING:
            s = va( "Awaiting challenge...%i", cstate.connectPacketCount );
            break;
            
        case CA_CONNECTED:
        {
            UTF8 downloadName[MAX_INFO_VALUE];
            S32 prompt = trap_Cvar_VariableValue( "cl_downloadPrompt" );
            
            if( prompt )
            {
                Com_Printf( "Opening download prompt...\n" );
                trap_Key_SetCatcher( KEYCATCH_UI );
                Menus_ActivateByName( "download_popmenu" );
                trap_Cvar_Set( "cl_downloadPrompt", "0" );
            }
            
            trap_Cvar_VariableStringBuffer( "cl_downloadName", downloadName, sizeof( downloadName ) );
            
            if( *downloadName )
            {
                UI_DisplayDownloadInfo( downloadName, centerPoint, yStart, scale );
                return;
            }
        }
        
        s = "Awaiting gamestate...";
        break;
        
        case CA_LOADING:
            return;
            
        case CA_PRIMED:
            return;
            
        default:
            return;
    }
    
    
    if( Q_stricmp( cstate.servername, "localhost" ) )
        Text_PaintCenter( centerPoint, yStart + 80, scale, colorWhite, s, 0 );
        
    // password required / connection rejected information goes here
}

/*
=================
UI_RegisterCvars
=================
*/
void UI_RegisterCvars( void )
{
    S32     i;
    cvarTable_t* cv;
    
    for( i = 0, cv = cvarTable ; i < cvarTableSize ; i++, cv++ )
        trap_Cvar_Register( cv->vmCvar, cv->cvarName, cv->defaultString, cv->cvarFlags );
        
    // use ui messagemode
    trap_Cvar_Set( "ui_useMessagemode", "1" );
}

/*
=================
UI_UpdateCvars
=================
*/
void UI_UpdateCvars( void )
{
    S32     i;
    cvarTable_t* cv;
    
    for( i = 0, cv = cvarTable ; i < cvarTableSize ; i++, cv++ )
        trap_Cvar_Update( cv->vmCvar );
}

bool idUserInterfaceManagerLocal::WantsBindKeys( void )
{
    return ( g_waitingForKey && g_bindItem ) ? true : false;
}

