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
// File name:   gui_local.h
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#ifndef __GUI_LOCAL_H__
#define __GUI_LOCAL_H__

#ifndef __Q_SHARED_H__
#include <qcommon/q_shared.h>
#endif
#ifndef __Q_SHARED_H__
#include <GPURenderer/r_types.h>
#endif
#ifndef __KEYCODES_H__
#include <GUI/keycodes.h>
#endif
#ifndef __GUI_PUBLIC_H__
#include <GUI/gui_api.h>
#endif
#ifndef __BG_PUBLIC_H__
#include <bgame/bg_public.h>
#endif
#ifndef __GUI_SHARED_H__
#include <GUI/gui_shared.h>
#endif

void UI_Report( void );
void UI_Load( void );
void UI_LoadMenus( StringEntry menuFile, bool reset );
S32 UI_AdjustTimeByGame( S32 time );
void UI_ClearScores( void );
void UI_LoadArenas( void );
void UI_ServerInfo( void );

void UI_RegisterCvars( void );
void UI_UpdateCvars( void );
void UI_DrawConnectScreen( bool overlay );

// new ui stuff
#define MAX_MAPS 128
#define MAX_PINGREQUESTS 32
#define MAX_ADDRESSLENGTH 64
#define MAX_DISPLAY_SERVERS 2048
#define MAX_SERVERSTATUS_LINES 128
#define MAX_SERVERSTATUS_TEXT 1024
#define MAX_FOUNDPLAYER_SERVERS 16
#define MAX_MODS 64
#define MAX_DEMOS 256
#define MAX_MOVIES 256
#define MAX_HELP_INFOPANES 32
#define MAX_RESOLUTIONS 32

typedef struct
{
    StringEntry mapName;
    StringEntry mapLoadName;
    StringEntry imageName;
    S32 cinematic;
    qhandle_t levelShot;
}
mapInfo;

typedef struct serverFilter_s
{
    StringEntry description;
    StringEntry basedir;
}
serverFilter_t;

typedef struct
{
    UTF8  adrstr[MAX_ADDRESSLENGTH];
    S32    start;
}
pinglist_t;


typedef struct guiServerStatus_s
{
    pinglist_t pingList[MAX_PINGREQUESTS];
    S32    numqueriedservers;
    S32    currentping;
    S32    nextpingtime;
    S32    maxservers;
    S32    refreshtime;
    S32    numServers;
    S32    sortKey;
    S32    sortDir;
    bool sorted;
    S32    lastCount;
    bool refreshActive;
    S32    currentServer;
    S32    displayServers[MAX_DISPLAY_SERVERS];
    S32    numDisplayServers;
    S32    numPlayersOnServers;
    S32    nextDisplayRefresh;
    S32    nextSortTime;
    qhandle_t currentServerPreview;
    S32    currentServerCinematic;
    S32    motdLen;
    S32    motdWidth;
    S32    motdPaintX;
    S32    motdPaintX2;
    S32    motdOffset;
    S32    motdTime;
    UTF8  motd[MAX_STRING_CHARS];
}
guiServerStatus_t;


typedef struct
{
    UTF8    adrstr[MAX_ADDRESSLENGTH];
    UTF8    name[MAX_ADDRESSLENGTH];
    S32      startTime;
    S32      serverNum;
    bool  valid;
}
pendingServer_t;

typedef struct
{
    S32 num;
    pendingServer_t server[MAX_SERVERSTATUSREQUESTS];
}
pendingServerStatus_t;

typedef struct
{
    UTF8 address[MAX_ADDRESSLENGTH];
    UTF8* lines[MAX_SERVERSTATUS_LINES][4];
    UTF8 text[MAX_SERVERSTATUS_TEXT];
    UTF8 pings[MAX_CLIENTS * 3];
    S32 numLines;
}
serverStatusInfo_t;

typedef struct
{
    StringEntry modName;
    StringEntry modDescr;
}
modInfo_t;

typedef enum
{
    INFOTYPE_TEXT,
    INFOTYPE_BUILDABLE,
    INFOTYPE_CLASS,
    INFOTYPE_WEAPON,
    INFOTYPE_UPGRADE
} infoType_t;

typedef struct
{
    StringEntry    text;
    StringEntry    cmd;
    infoType_t    type;
    union
    {
        StringEntry  text;
        buildable_t buildable;
        class_t     pclass;
        weapon_t    weapon;
        upgrade_t   upgrade;
    } v;
}
menuItem_t;

typedef struct
{
    S32 w;
    S32 h;
}
resolution_t;

typedef struct
{
    displayContextDef_t uiDC;
    
    S32 playerCount;
    S32 myTeamCount;
    S32 teamPlayerIndex;
    S32 playerRefresh;
    S32 playerIndex;
    S32 playerNumber;
    S32 myPlayerIndex;
    S32 ignoreIndex;
    UTF8 playerNames[MAX_CLIENTS][MAX_NAME_LENGTH];
    UTF8 rawPlayerNames[MAX_CLIENTS][MAX_NAME_LENGTH];
    UTF8 teamNames[MAX_CLIENTS][MAX_NAME_LENGTH];
    UTF8 rawTeamNames[MAX_CLIENTS][MAX_NAME_LENGTH];
    S32 clientNums[MAX_CLIENTS];
    S32 teamClientNums[MAX_CLIENTS];
    clientList_t ignoreList[MAX_CLIENTS];
    
    S32 mapCount;
    mapInfo mapList[MAX_MAPS];
    
    modInfo_t modList[MAX_MODS];
    S32 modCount;
    S32 modIndex;
    
    StringEntry demoList[MAX_DEMOS];
    S32 demoCount;
    S32 demoIndex;
    
    StringEntry movieList[MAX_MOVIES];
    S32 movieCount;
    S32 movieIndex;
    S32 previewMovie;
    
    menuItem_t  teamList[ 4 ];
    S32         teamCount;
    S32         teamIndex;
    
    menuItem_t  alienClassList[ 3 ];
    S32         alienClassCount;
    S32         alienClassIndex;
    
    menuItem_t  humanItemList[ 3 ];
    S32         humanItemCount;
    S32         humanItemIndex;
    
    menuItem_t  humanArmouryBuyList[ 32 ];
    S32         humanArmouryBuyCount;
    S32         humanArmouryBuyIndex;
    
    menuItem_t  humanArmourySellList[ 32 ];
    S32         humanArmourySellCount;
    S32         humanArmourySellIndex;
    
    menuItem_t  alienUpgradeList[ 16 ];
    S32         alienUpgradeCount;
    S32         alienUpgradeIndex;
    
    menuItem_t  alienBuildList[ 32 ];
    S32         alienBuildCount;
    S32         alienBuildIndex;
    
    menuItem_t  humanBuildList[ 32 ];
    S32         humanBuildCount;
    S32         humanBuildIndex;
    
    menuItem_t  helpList[ MAX_HELP_INFOPANES ];
    S32         helpCount;
    S32         helpIndex;
    
    S32         weapon;
    S32         upgrades;
    S32         credits;
    
    guiServerStatus_t serverStatus;
    
    // for the showing the status of a server
    UTF8 serverStatusAddress[MAX_ADDRESSLENGTH];
    serverStatusInfo_t serverStatusInfo;
    S32 nextServerStatusRefresh;
    
    // to retrieve the status of server to find a player
    pendingServerStatus_t pendingServerStatus;
    UTF8 findPlayerName[MAX_STRING_CHARS];
    UTF8 foundPlayerServerAddresses[MAX_FOUNDPLAYER_SERVERS][MAX_ADDRESSLENGTH];
    UTF8 foundPlayerServerNames[MAX_FOUNDPLAYER_SERVERS][MAX_ADDRESSLENGTH];
    S32 currentFoundPlayerServer;
    S32 numFoundPlayerServers;
    S32 nextFindPlayerRefresh;
    
    resolution_t  resolutions[ MAX_RESOLUTIONS ];
    S32 numResolutions;
    bool oldResolutions;
    
    bool inGameLoad;
    
    bool  chatTeam;
    bool  chatAdmins;
    bool  chatClan;
    bool  chatPrompt;
    UTF8      chatPromptCallback[ MAX_STRING_CHARS ];
    S32       chatTargetClientNum;
}
uiInfo_t;

extern uiInfo_t uiInfo;

UTF8*      UI_Cvar_VariableString( StringEntry var_name );
void      UI_SetColor( const F32* rgba );
void      UI_AdjustFrom640( F32* x, F32* y, F32* w, F32* h );
void      UI_DrawHandlePic( F32 x, F32 y, F32 w, F32 h, qhandle_t hShader );

//
// idUserInterfaceManagerLocal
//
class idUserInterfaceManagerLocal : public idUserInterfaceManager
{
public:
    virtual void Init( bool inGameLoad );
    virtual void Shutdown( void );
    
    virtual	void KeyEvent( S32 key, bool down );
    virtual void MouseEvent( S32 dx, S32 dy );
    virtual void Refresh( S32 time );
    
    virtual bool IsFullscreen( void );
    
    virtual S32 MousePosition( void );
    virtual void SetMousePosition( S32 x, S32 y );
    
    virtual void SetActiveMenu( uiMenuCommand_t menu );
    
    virtual uiMenuCommand_t GetActiveMenu( void );
    virtual bool ConsoleCommand( S32 realTime );
    
    // if !overlay, the background will be drawn, otherwise it will be
    // overlayed over whatever the cgame has drawn.
    // a GetClientState syscall will be made to get the current strings
    virtual void DrawConnectScreen( bool overlay );
    virtual bool CheckExecKey( S32 key );
    virtual bool WantsBindKeys( void );
};

extern idUserInterfaceManagerLocal uiManagerLocal;

#endif //!__GUI_LOCAL_H__

