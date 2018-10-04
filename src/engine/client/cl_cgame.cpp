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
// File name:   cl_cgame.cpp
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description: client system interaction with client game
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#include <OWLib/precompiled.h>

extern botlib_export_t* botlib_export;

bool loadCamera( S32 camNum, StringEntry name );
void startCamera( S32 camNum, S32 time );
bool getCameraInfo( S32 camNum, S32 time, F32* origin, F32* angles, F32* fov );

// RF, this is only used when running a local server
//extern void SV_SendMoveSpeedsToGame( S32 entnum, UTF8* text );
//extern bool SV_GetModelInfo( S32 clientNum, UTF8* modelName, animModelInfo_t** modelInfo );

static void( *completer )( StringEntry s ) = NULL;

idCGame* cgame;
idCGame* ( *cgdllEntry )( cgameImports_t* cgimports );
static cgameImports_t exports;

/*
====================
CL_startCamera
====================
*/
void CL_startCamera( S32 camNum, S32 time )
{
    if( camNum == 0 )   // CAM_PRIMARY
    {
        cl.cameraMode = true;  //----(SA)	added
    }
    startCamera( camNum, time );
}

/*
====================
CL_stopCamera
====================
*/
void CL_stopCamera( S32 camNum )
{
    if( camNum == 0 )   // CAM_PRIMARY
    {
        cl.cameraMode = false;
    }
}

/*
====================
CL_GetGameState
====================
*/
void CL_GetGameState( gameState_t* gs )
{
    *gs = cl.gameState;
}

/*
====================
CL_GetGlconfig
====================
*/
void CL_GetGlconfig( glconfig_t* glconfig )
{
    *glconfig = cls.glconfig;
}


S32 CompleteCallback( StringEntry complete )
{
    if( completer )
    {
        completer( complete );
    }
    return 0;
}


/*
====================
CL_GetUserCmd
====================
*/
bool CL_GetUserCmd( S32 cmdNumber, usercmd_t* ucmd )
{
    // cmds[cmdNumber] is the last properly generated command
    
    // can't return anything that we haven't created yet
    if( cmdNumber > cl.cmdNumber )
    {
        Com_Error( ERR_DROP, "CL_GetUserCmd: %i >= %i", cmdNumber, cl.cmdNumber );
    }
    
    // the usercmd has been overwritten in the wrapping
    // buffer because it is too far out of date
    if( cmdNumber <= cl.cmdNumber - CMD_BACKUP )
    {
        return false;
    }
    
    *ucmd = cl.cmds[cmdNumber & CMD_MASK];
    
    return true;
}

S32 CL_GetCurrentCmdNumber( void )
{
    return cl.cmdNumber;
}


/*
====================
CL_GetParseEntityState
====================
*/
bool CL_GetParseEntityState( S32 parseEntityNumber, entityState_t* state )
{
    // can't return anything that hasn't been parsed yet
    if( parseEntityNumber >= cl.parseEntitiesNum )
    {
        Com_Error( ERR_DROP, "CL_GetParseEntityState: %i >= %i", parseEntityNumber, cl.parseEntitiesNum );
    }
    
    // can't return anything that has been overwritten in the circular buffer
    if( parseEntityNumber <= cl.parseEntitiesNum - MAX_PARSE_ENTITIES )
    {
        return false;
    }
    
    *state = cl.parseEntities[parseEntityNumber & ( MAX_PARSE_ENTITIES - 1 )];
    return true;
}

/*
====================
CL_GetCurrentSnapshotNumber
====================
*/
void CL_GetCurrentSnapshotNumber( S32* snapshotNumber, S32* serverTime )
{
    *snapshotNumber = cl.snap.messageNum;
    *serverTime = cl.snap.serverTime;
}

/*
====================
CL_GetSnapshot
====================
*/
bool CL_GetSnapshot( S32 snapshotNumber, snapshot_t* snapshot )
{
    clSnapshot_t*   clSnap;
    S32             i, count;
    
    if( snapshotNumber > cl.snap.messageNum )
    {
        Com_Error( ERR_DROP, "CL_GetSnapshot: snapshotNumber > cl.snapshot.messageNum" );
    }
    
    // if the frame has fallen out of the circular buffer, we can't return it
    if( cl.snap.messageNum - snapshotNumber >= PACKET_BACKUP )
    {
        return false;
    }
    
    // if the frame is not valid, we can't return it
    clSnap = &cl.snapshots[snapshotNumber & PACKET_MASK];
    if( !clSnap->valid )
    {
        return false;
    }
    
    // if the entities in the frame have fallen out of their
    // circular buffer, we can't return it
    if( cl.parseEntitiesNum - clSnap->parseEntitiesNum >= MAX_PARSE_ENTITIES )
    {
        return false;
    }
    
    // write the snapshot
    snapshot->snapFlags = clSnap->snapFlags;
    snapshot->serverCommandSequence = clSnap->serverCommandNum;
    snapshot->ping = clSnap->ping;
    snapshot->serverTime = clSnap->serverTime;
    memcpy( snapshot->areamask, clSnap->areamask, sizeof( snapshot->areamask ) );
    snapshot->ps = clSnap->ps;
    count = clSnap->numEntities;
    if( count > MAX_ENTITIES_IN_SNAPSHOT )
    {
        Com_DPrintf( "CL_GetSnapshot: truncated %i entities to %i\n", count, MAX_ENTITIES_IN_SNAPSHOT );
        count = MAX_ENTITIES_IN_SNAPSHOT;
    }
    snapshot->numEntities = count;
    for( i = 0; i < count; i++ )
    {
        snapshot->entities[i] = cl.parseEntities[( clSnap->parseEntitiesNum + i ) & ( MAX_PARSE_ENTITIES - 1 )];
    }
    
    // FIXME: configstring changes and server commands!!!
    
    return true;
}

/*
==============
CL_SetUserCmdValue
==============
*/
void CL_SetUserCmdValue( S32 userCmdValue, S32 flags, F32 sensitivityScale, S32 mpIdentClient )
{
    cl.cgameUserCmdValue = userCmdValue;
    cl.cgameFlags = flags;
    cl.cgameSensitivity = sensitivityScale;
    cl.cgameMpIdentClient = mpIdentClient;	// NERVE - SMF
}

/*
==================
CL_SetClientLerpOrigin
==================
*/
void CL_SetClientLerpOrigin( F32 x, F32 y, F32 z )
{
    cl.cgameClientLerpOrigin[0] = x;
    cl.cgameClientLerpOrigin[1] = y;
    cl.cgameClientLerpOrigin[2] = z;
}

/*
=====================
CL_CompleteCgameCommand
=====================
*/
void CL_CompleteCgameCommand( UTF8* args, S32 argNum )
{
    Field_CompleteCgame( argNum );
}

/*
=====================
CL_CgameCompletion
=====================
*/
void CL_CgameCompletion( void( *callback )( StringEntry s ), S32 argNum )
{
    completer = callback;
    cgame->CompleteCommand( argNum );
    completer = NULL;
}

/*
==============
CL_AddCgameCommand
==============
*/
void CL_AddCgameCommand( StringEntry cmdName )
{
    Cmd_AddCommand( cmdName, NULL );
    Cmd_SetCommandCompletionFunc( cmdName, CL_CompleteCgameCommand );
}

/*
==============
CL_CgameError
==============
*/
void CL_CgameError( StringEntry string )
{
    Com_Error( ERR_DROP, "%s", string );
}

bool CL_CGameCheckKeyExec( S32 key )
{
    if( cgvm )
    {
        return cgame->CheckExecKey( key );
    }
    else
    {
        return false;
    }
}


/*
=====================
CL_ConfigstringModified
=====================
*/
void CL_ConfigstringModified( void )
{
    UTF8*           old, *s;
    S32             i, index;
    UTF8*           dup;
    gameState_t     oldGs;
    S32             len;
    
    index = atoi( Cmd_Argv( 1 ) );
    if( index < 0 || index >= MAX_CONFIGSTRINGS )
    {
        Com_Error( ERR_DROP, "configstring > MAX_CONFIGSTRINGS" );
    }
//  s = Cmd_Argv(2);
    // get everything after "cs <num>"
    s = Cmd_ArgsFrom( 2 );
    
    old = cl.gameState.stringData + cl.gameState.stringOffsets[index];
    if( !strcmp( old, s ) )
    {
        return;					// unchanged
    }
    
    // build the new gameState_t
    oldGs = cl.gameState;
    
    memset( &cl.gameState, 0, sizeof( cl.gameState ) );
    
    // leave the first 0 for uninitialized strings
    cl.gameState.dataCount = 1;
    
    for( i = 0; i < MAX_CONFIGSTRINGS; i++ )
    {
        if( i == index )
        {
            dup = s;
        }
        else
        {
            dup = oldGs.stringData + oldGs.stringOffsets[i];
        }
        if( !dup[0] )
        {
            continue;			// leave with the default empty string
        }
        
        len = strlen( dup );
        
        if( len + 1 + cl.gameState.dataCount > MAX_GAMESTATE_CHARS )
        {
            Com_Error( ERR_DROP, "MAX_GAMESTATE_CHARS exceeded" );
        }
        
        // append it to the gameState string buffer
        cl.gameState.stringOffsets[i] = cl.gameState.dataCount;
        memcpy( cl.gameState.stringData + cl.gameState.dataCount, dup, len + 1 );
        cl.gameState.dataCount += len + 1;
    }
    
    if( index == CS_SYSTEMINFO )
    {
        // parse serverId and other cvars
        CL_SystemInfoChanged();
    }
    
}


/*
===================
CL_GetServerCommand

Set up argc/argv for the given command
===================
*/
bool CL_GetServerCommand( S32 serverCommandNumber )
{
    UTF8*           s;
    UTF8*           cmd;
    static UTF8     bigConfigString[BIG_INFO_STRING];
    S32             argc;
    
    // if we have irretrievably lost a reliable command, drop the connection
    if( serverCommandNumber <= clc.serverCommandSequence - MAX_RELIABLE_COMMANDS )
    {
        // when a demo record was started after the client got a whole bunch of
        // reliable commands then the client never got those first reliable commands
        if( clc.demoplaying )
        {
            return false;
        }
        Com_Error( ERR_DROP, "CL_GetServerCommand: a reliable command was cycled out" );
        return false;
    }
    
    if( serverCommandNumber > clc.serverCommandSequence )
    {
        Com_Error( ERR_DROP, "CL_GetServerCommand: requested a command not received" );
        return false;
    }
    
    s = clc.serverCommands[serverCommandNumber & ( MAX_RELIABLE_COMMANDS - 1 )];
    clc.lastExecutedServerCommand = serverCommandNumber;
    
    if( cl_showServerCommands->integer )
    {
        // NERVE - SMF
        Com_DPrintf( "serverCommand: %i : %s\n", serverCommandNumber, s );
    }
    
rescan:
    Cmd_TokenizeString( s );
    cmd = Cmd_Argv( 0 );
    argc = Cmd_Argc();
    
    if( !strcmp( cmd, "disconnect" ) )
    {
        // NERVE - SMF - allow server to indicate why they were disconnected
        if( argc >= 2 )
        {
            Com_Error( ERR_SERVERDISCONNECT, "Server Disconnected - %s", Cmd_Argv( 1 ) );
        }
        else
        {
            Com_Error( ERR_SERVERDISCONNECT, "Server disconnected\n" );
        }
    }
    
    if( !strcmp( cmd, "bcs0" ) )
    {
        Com_sprintf( bigConfigString, BIG_INFO_STRING, "cs %s \"%s", Cmd_Argv( 1 ), Cmd_Argv( 2 ) );
        return false;
    }
    
    if( !strcmp( cmd, "bcs1" ) )
    {
        s = Cmd_Argv( 2 );
        if( strlen( bigConfigString ) + strlen( s ) >= BIG_INFO_STRING )
        {
            Com_Error( ERR_DROP, "bcs exceeded BIG_INFO_STRING" );
        }
        strcat( bigConfigString, s );
        return false;
    }
    
    if( !strcmp( cmd, "bcs2" ) )
    {
        s = Cmd_Argv( 2 );
        if( strlen( bigConfigString ) + strlen( s ) + 1 >= BIG_INFO_STRING )
        {
            Com_Error( ERR_DROP, "bcs exceeded BIG_INFO_STRING" );
        }
        strcat( bigConfigString, s );
        strcat( bigConfigString, "\"" );
        s = bigConfigString;
        goto rescan;
    }
    
    if( !strcmp( cmd, "cs" ) )
    {
        CL_ConfigstringModified();
        // reparse the string, because CL_ConfigstringModified may have done another Cmd_TokenizeString()
        Cmd_TokenizeString( s );
        return true;
    }
    
    if( !strcmp( cmd, "map_restart" ) )
    {
        // clear notify lines and outgoing commands before passing
        // the restart to the cgame
        Con_ClearNotify();
        memset( cl.cmds, 0, sizeof( cl.cmds ) );
        return true;
    }
    
    if( !strcmp( cmd, "popup" ) )
    {
        // direct server to client popup request, bypassing cgame
        //CL_UIPopup(Cmd_Argv(1));
        //if ( cls.state == CA_ACTIVE && !clc.demoplaying ) {
        //    uiManager->SetActiveMenu( UIMENU_CLIPBOARD );
        //    Menus_OpenByName(Cmd_Argv(1));
        //}
        //return false;
    }
    
    // we may want to put a "connect to other server" command here
    
    // cgame can now act on the command
    return true;
}

// DHM - Nerve :: Copied from server to here
/*
====================
CL_SetExpectedHunkUsage

  Sets com_expectedhunkusage, so the client knows how to draw the percentage bar
====================
*/
void CL_SetExpectedHunkUsage( StringEntry mapname )
{
    S32             handle;
    UTF8*           memlistfile = "hunkusage.dat";
    UTF8*           buf;
    UTF8*           buftrav;
    UTF8*           token;
    S32             len;
    
    len = FS_FOpenFileByMode( memlistfile, &handle, FS_READ );
    if( len >= 0 )
    {
        // the file exists, so read it in, strip out the current entry for this map, and save it out, so we can append the new value
        
        buf = ( UTF8* )Z_Malloc( len + 1 );
        memset( buf, 0, len + 1 );
        
        FS_Read( ( void* )buf, len, handle );
        FS_FCloseFile( handle );
        
        // now parse the file, filtering out the current map
        buftrav = buf;
        while( ( token = COM_Parse( &buftrav ) ) != NULL && token[0] )
        {
            if( !Q_stricmp( token, ( UTF8* )mapname ) )
            {
                // found a match
                token = COM_Parse( &buftrav );	// read the size
                if( token && *token )
                {
                    // this is the usage
                    com_expectedhunkusage = atoi( token );
                    Z_Free( buf );
                    return;
                }
            }
        }
        
        Z_Free( buf );
    }
    // just set it to a negative number,so the cgame knows not to draw the percent bar
    com_expectedhunkusage = -1;
}


/*
====================
CL_CM_LoadMap

Just adds default parameters that cgame doesn't need to know about
====================
*/
void CL_CM_LoadMap( StringEntry mapname )
{
    S32             checksum;
    
    // DHM - Nerve :: If we are not running the server, then set expected usage here
    if( !com_sv_running->integer )
    {
        CL_SetExpectedHunkUsage( mapname );
    }
    else
    {
        // TTimo
        // catch here when a local server is started to avoid outdated com_errorDiagnoseIP
        Cvar_Set( "com_errorDiagnoseIP", "" );
    }
    
    collisionModelManager->LoadMap( mapname, true, &checksum );
}

/*
====================
CL_ShutdownCGame
====================
*/
void CL_ShutdownCGame( void )
{
    cls.keyCatchers &= ~KEYCATCH_CGAME;
    cls.cgameStarted = false;
    if( cgame == NULL || cgvm == NULL )
    {
        return;
    }
    cgame->Shutdown();
    cgame = NULL;
    
    Sys_UnloadDll( cgvm );
    cgvm = NULL;
}

void CL_UIClosePopup( StringEntry uiname )
{
    uiManager->KeyEvent( K_ESCAPE, true );
}

void CL_UIPopup( StringEntry uiname )
{
    if( uiname == NULL )
    {
        uiManager->SetActiveMenu( UIMENU_MAIN );
        return;
    }
    
    if( cls.state == CA_ACTIVE && !clc.demoplaying )
    {
        if( !Q_stricmp( uiname, "UIMENU_NONE" ) )
        {
            uiManager->SetActiveMenu( UIMENU_NONE );
        }
        else if( !Q_stricmp( uiname, "UIMENU_MAIN" ) )
        {
            uiManager->SetActiveMenu( UIMENU_MAIN );
        }
        else if( !Q_stricmp( uiname, "UIMENU_INGAME" ) )
        {
            uiManager->SetActiveMenu( UIMENU_INGAME );
        }
        else if( !Q_stricmp( uiname, "UIMENU_NEED_CD" ) )
        {
            uiManager->SetActiveMenu( UIMENU_NEED_CD );
        }
        else if( !Q_stricmp( uiname, "UIMENU_BAD_CD_KEY" ) )
        {
            uiManager->SetActiveMenu( UIMENU_BAD_CD_KEY );
        }
        else if( !Q_stricmp( uiname, "UIMENU_TEAM" ) )
        {
            uiManager->SetActiveMenu( UIMENU_TEAM );
        }
        else if( !Q_stricmp( uiname, "UIMENU_POSTGAME" ) )
        {
            uiManager->SetActiveMenu( UIMENU_POSTGAME );
        }
        else if( !Q_stricmp( uiname, "UIMENU_HELP" ) )
        {
            uiManager->SetActiveMenu( UIMENU_HELP );
        }
        else if( !Q_stricmp( uiname, "UIMENU_WM_QUICKMESSAGE" ) )
        {
            uiManager->SetActiveMenu( UIMENU_WM_QUICKMESSAGE );
        }
        else if( !Q_stricmp( uiname, "UIMENU_WM_QUICKMESSAGEALT" ) )
        {
            uiManager->SetActiveMenu( UIMENU_WM_QUICKMESSAGEALT );
        }
        else if( !Q_stricmp( uiname, "UIMENU_WM_FTQUICKMESSAGE" ) )
        {
            uiManager->SetActiveMenu( UIMENU_WM_FTQUICKMESSAGE );
        }
        else if( !Q_stricmp( uiname, "UIMENU_WM_TAPOUT" ) )
        {
            uiManager->SetActiveMenu( UIMENU_WM_TAPOUT );
        }
        else if( !Q_stricmp( uiname, "UIMENU_WM_TAPOUT_LMS" ) )
        {
            uiManager->SetActiveMenu( UIMENU_WM_TAPOUT_LMS );
        }
        else if( !Q_stricmp( uiname, "UIMENU_WM_AUTOUPDATE" ) )
        {
            uiManager->SetActiveMenu( UIMENU_WM_AUTOUPDATE );
        }
        else if( !Q_stricmp( uiname, "UIMENU_INGAME_MESSAGEMODE" ) )
        {
            uiManager->SetActiveMenu( UIMENU_INGAME_MESSAGEMODE );
        }
        else
        {
            uiManager->SetActiveMenu( UIMENU_MAIN );
        }
    }
}

/*
====================
CG_Key_SetCatcher
====================
*/
void CG_Key_SetCatcher( S32 catcher )
{
    Key_SetCatcher( catcher | ( Key_GetCatcher() & KEYCATCH_CONSOLE ) );
}

/*
====================
CL_PhysicsDrawDebug
====================
*/
void CL_PhysicsDrawDebug( void )
{
    physicsManager->DrawDebug();
}

/*
====================
CL_CreateExportTable
====================
*/
void CL_CreateExportTable( void )
{
    exports.Print = Com_Printf;
    exports.Error = Com_Error;
    exports.Milliseconds = Sys_Milliseconds;
    
    exports.Cvar_Register = Cvar_Register;
    exports.Cvar_Update = Cvar_Update;
    exports.Cvar_Set = Cvar_Set;
    exports.Cvar_VariableStringBuffer = Cvar_VariableStringBuffer;
    exports.Cvar_LatchedVariableStringBuffer = Cvar_LatchedVariableStringBuffer;
    
    exports.Argc = Cmd_Argc;
    exports.Argv = Cmd_ArgvBuffer;
    exports.Args = Cmd_ArgsBuffer;
    exports.LiteralArgsBuffer = Cmd_ArgsBuffer;
    
    exports.DemoState = CL_DemoState;
    exports.DemoPos = CL_DemoPos;
    
    exports.FS_FOpenFile = FS_FOpenFileByMode;
    exports.FS_Read = FS_Read2;
    exports.FS_Write = FS_Write;
    exports.FS_FCloseFile = FS_FCloseFile;
    exports.FS_GetFileList = FS_GetFileList;
    exports.FS_Delete = FS_Delete;
    exports.FS_Seek = FS_Seek;
    
    exports.SendConsoleCommand = Cbuf_AddText;
    exports.AddCommand = CL_AddCgameCommand;
    exports.Cmd_RemoveCommand = Cmd_RemoveCommand;
    exports.AddReliableCommand = CL_AddReliableCommand;
    exports.UpdateScreen = SCR_UpdateScreen;
    exports.LoadMap = CL_CM_LoadMap;
    
    exports.GetGlconfig = CL_GetGlconfig;
    exports.GetGameState = CL_GetGameState;
    exports.GetCurrentSnapshotNumber = CL_GetCurrentSnapshotNumber;
    exports.GetSnapshot = CL_GetSnapshot;
    exports.GetServerCommand = CL_GetServerCommand;
    exports.GetCurrentCmdNumber = CL_GetCurrentCmdNumber;
    exports.GetUserCmd = CL_GetUserCmd;
    exports.SetUserCmdValue = CL_SetUserCmdValue;
    exports.SetClientLerpOrigin = CL_SetClientLerpOrigin;
    exports.Hunk_MemoryRemaining = Hunk_MemoryRemaining;
    
    exports.Key_IsDown = Key_IsDown;
    exports.Key_GetCatcher = Key_GetCatcher;
    exports.Key_SetCatcher = CG_Key_SetCatcher;
    exports.Key_GetKey = Key_GetKey;
    exports.Key_GetOverstrikeMode = Key_GetOverstrikeMode;
    exports.Key_SetOverstrikeMode = Key_SetOverstrikeMode;
    
    exports.RealTime = Com_RealTime;
    
    exports.CIN_PlayCinematic = CIN_PlayCinematic;
    exports.CIN_StopCinematic = CIN_StopCinematic;
    exports.CIN_RunCinematic = CIN_RunCinematic;
    exports.CIN_DrawCinematic = CIN_DrawCinematic;
    exports.CIN_SetExtents = CIN_SetExtents;
    
    exports.loadCamera = loadCamera;
    exports.startCamera = CL_startCamera;
    exports.stopCamera = CL_stopCamera;
    
    exports.getCameraInfo = ( bool( QDECL* )( S32, S32, vec3_t(* ), vec3_t(* ), F32* ) )getCameraInfo;
    
    exports.Key_GetBindingBuf = Key_GetBindingBuf;
    exports.Key_SetBinding = Key_SetBinding;
    exports.Key_KeynumToStringBuf = Key_KeynumToStringBuf;
    exports.Key_GetBindingByString = Key_GetBindingByString;
    exports.CL_TranslateString = CL_TranslateString;
    exports.Com_GetHunkInfo = Com_GetHunkInfo;
    exports.CL_DemoName = CL_DemoName;
    exports.UI_Popup = CL_UIPopup;
    exports.UI_LimboChat = CL_AddToLimboChat;
    exports.UI_ClosePopup = CL_UIClosePopup;
    exports.Sys_SnapVector = Sys_SnapVector;
    
    exports.PC_AddGlobalDefine = botlib_export->PC_AddGlobalDefine;
    exports.PC_LoadSource = botlib_export->PC_LoadSourceHandle;
    exports.PC_FreeSource = botlib_export->PC_FreeSourceHandle;
    exports.PC_ReadToken = botlib_export->PC_ReadTokenHandle;
    exports.PC_SourceFileAndLine = botlib_export->PC_SourceFileAndLine;
    //exports.PC_UnreadLastToken = botlib_export->PC_UnreadLastTokenHandle;
    
    exports.PhysicsDrawDebug = CL_PhysicsDrawDebug;
    
    exports.renderSystem = renderSystem;
    exports.collisionModelManager = collisionModelManager;
    exports.soundSystem = soundSystem;
    
}

/*
====================
CL_UpdateLevelHunkUsage

  This updates the "hunkusage.dat" file with the current map and it's hunk usage count

  This is used for level loading, so we can show a percentage bar dependant on the amount
  of hunk memory allocated so far

  This will be slightly inaccurate if some settings like sound quality are changed, but these
  things should only account for a small variation (hopefully)
====================
*/
void CL_UpdateLevelHunkUsage( void )
{
    S32             handle;
    UTF8*           memlistfile = "hunkusage.dat";
    UTF8*           buf, *outbuf;
    UTF8*           buftrav, *outbuftrav;
    UTF8*           token;
    UTF8            outstr[256];
    S32             len, memusage;
    
    memusage = Cvar_VariableIntegerValue( "com_hunkused" ) + Cvar_VariableIntegerValue( "hunk_soundadjust" );
    
    len = FS_FOpenFileByMode( memlistfile, &handle, FS_READ );
    if( len >= 0 )
    {
        // the file exists, so read it in, strip out the current entry for this map, and save it out, so we can append the new value
        
        buf = ( UTF8* )Z_Malloc( len + 1 );
        memset( buf, 0, len + 1 );
        outbuf = ( UTF8* )Z_Malloc( len + 1 );
        memset( outbuf, 0, len + 1 );
        
        FS_Read( ( void* )buf, len, handle );
        FS_FCloseFile( handle );
        
        // now parse the file, filtering out the current map
        buftrav = buf;
        outbuftrav = outbuf;
        outbuftrav[0] = '\0';
        while( ( token = COM_Parse( &buftrav ) ) != NULL && token[0] )
        {
            if( !Q_stricmp( token, cl.mapname ) )
            {
                // found a match
                token = COM_Parse( &buftrav );	// read the size
                if( token && token[0] )
                {
                    if( atoi( token ) == memusage )
                    {
                        // if it is the same, abort this process
                        Z_Free( buf );
                        Z_Free( outbuf );
                        return;
                    }
                }
            }
            else
            {
                // send it to the outbuf
                Q_strcat( outbuftrav, len + 1, token );
                Q_strcat( outbuftrav, len + 1, " " );
                token = COM_Parse( &buftrav );	// read the size
                if( token && token[0] )
                {
                    Q_strcat( outbuftrav, len + 1, token );
                    Q_strcat( outbuftrav, len + 1, "\n" );
                }
                else
                {
                    Com_Error( ERR_DROP, "hunkusage.dat file is corrupt\n" );
                }
            }
        }
        
        handle = FS_FOpenFileWrite( memlistfile );
        if( handle < 0 )
        {
            Com_Error( ERR_DROP, "cannot create %s\n", memlistfile );
        }
        // input file is parsed, now output to the new file
        len = strlen( outbuf );
        if( FS_Write( ( void* )outbuf, len, handle ) != len )
        {
            Com_Error( ERR_DROP, "cannot write to %s\n", memlistfile );
        }
        FS_FCloseFile( handle );
        
        Z_Free( buf );
        Z_Free( outbuf );
    }
    // now append the current map to the current file
    FS_FOpenFileByMode( memlistfile, &handle, FS_APPEND );
    if( handle < 0 )
    {
        Com_Error( ERR_DROP, "cannot write to hunkusage.dat, check disk full\n" );
    }
    Com_sprintf( outstr, sizeof( outstr ), "%s %i\n", cl.mapname, memusage );
    FS_Write( outstr, strlen( outstr ), handle );
    FS_FCloseFile( handle );
    
    // now just open it and close it, so it gets copied to the pak dir
    len = FS_FOpenFileByMode( memlistfile, &handle, FS_READ );
    if( len >= 0 )
    {
        FS_FCloseFile( handle );
    }
}

/*
====================
CL_InitCGame

Should only by called by CL_StartHunkUsers
====================
*/
void CL_InitCGame( void )
{
    StringEntry     info;
    StringEntry     mapname;
    S32             t1, t2;
    
    t1 = Sys_Milliseconds();
    
    // put away the console
    Con_Close();
    
    // find the current mapname
    info = cl.gameState.stringData + cl.gameState.stringOffsets[CS_SERVERINFO];
    mapname = Info_ValueForKey( info, "mapname" );
    Com_sprintf( cl.mapname, sizeof( cl.mapname ), "maps/%s.world", mapname );
    
    // load the dll
    cgvm = Sys_LoadDll( "cgame" );
    if( !cgvm )
    {
        Com_Error( ERR_DROP, "cgvm on cgame failed" );
    }
    // Load in the entry point.
    cgdllEntry = ( idCGame * ( QDECL* )( cgameImports_t* ) )Sys_GetProcAddress( cgvm, "dllEntry" );
    if( !cgdllEntry )
    {
        Com_Error( ERR_DROP, "cgdllEntry on cgame failed" );
    }
    
    // Create the export table.
    CL_CreateExportTable();
    
    // Call the dll entry point.
    cgame = cgdllEntry( &exports );
    
    cls.state = CA_LOADING;
    
    // init for this gamestate
    // use the lastExecutedServerCommand instead of the serverCommandSequence
    // otherwise server commands sent just before a gamestate are dropped
    //bani - added clc.demoplaying, since some mods need this at init time, and drawactiveframe is too late for them
    cgame->Init( clc.serverMessageSequence, clc.lastExecutedServerCommand, clc.clientNum, clc.demoplaying );
    
    // we will send a usercmd this frame, which
    // will cause the server to send us the first snapshot
    cls.state = CA_PRIMED;
    
    t2 = Sys_Milliseconds();
    
    Com_Printf( "CL_InitCGame: %5.2f seconds\n", ( t2 - t1 ) / 1000.0 );
    
    // have the renderer touch all its images, so they are present
    // on the card even if the driver does deferred loading
    renderSystem->EndRegistration();
    
    if( !Sys_LowPhysicalMemory() )
    {
        Com_TouchMemory();
    }
    
    // clear anything that got printed
    Con_ClearNotify();
    
    // Ridah, update the memory usage file
    CL_UpdateLevelHunkUsage();
    
//  if( cl_autorecord->integer ) {
//      Cvar_Set( "g_synchronousClients", "1" );
//  }
}


/*
====================
CL_GameCommand

See if the current console command is claimed by the cgame
====================
*/
bool CL_GameCommand( void )
{
    if( !cgvm )
    {
        return false;
    }
    
    return cgame->ConsoleCommand();
}

/*
====================
CL_GameCommand

See if the current console command is claimed by the cgame
====================
*/
void CL_GameConsoleText( void )
{
    if( !cgvm )
    {
        return;
    }
    
    return cgame->ConsoleText();
}


/*
=====================
CL_CGameRendering
=====================
*/
void CL_CGameRendering( stereoFrame_t stereo )
{
    /*	static S32 x = 0;
    	if(!((++x) % 20)) {
    		Com_Printf( "numtraces: %i\n", numtraces / 20 );
    		numtraces = 0;
    	} else {
    	}*/
    
    cgame->DrawActiveFrame( cl.serverTime, stereo, clc.demoplaying );
}


/*
=================
CL_AdjustTimeDelta

Adjust the clients view of server time.

We attempt to have cl.serverTime exactly equal the server's view
of time plus the timeNudge, but with variable latencies over
the internet it will often need to drift a bit to match conditions.

Our ideal time would be to have the adjusted time approach, but not pass,
the very latest snapshot.

Adjustments are only made when a new snapshot arrives with a rational
latency, which keeps the adjustment process framerate independent and
prevents massive overadjustment during times of significant packet loss
or bursted delayed packets.
=================
*/

#define RESET_TIME  500

void CL_AdjustTimeDelta( void )
{
    S32             resetTime;
    S32             newDelta;
    S32             deltaDelta;
    
    cl.newSnapshots = false;
    
    // the delta never drifts when replaying a demo
    if( clc.demoplaying )
    {
        return;
    }
    
    // if the current time is WAY off, just correct to the current value
    if( com_sv_running->integer )
    {
        resetTime = 100;
    }
    else
    {
        resetTime = RESET_TIME;
    }
    
    newDelta = cl.snap.serverTime - cls.realtime;
    deltaDelta = abs( newDelta - cl.serverTimeDelta );
    
    if( deltaDelta > RESET_TIME )
    {
        cl.serverTimeDelta = newDelta;
        cl.oldServerTime = cl.snap.serverTime;	// FIXME: is this a problem for cgame?
        cl.serverTime = cl.snap.serverTime;
        if( cl_showTimeDelta->integer )
        {
            Com_Printf( "<RESET> " );
        }
    }
    else if( deltaDelta > 100 )
    {
        // fast adjust, cut the difference in half
        if( cl_showTimeDelta->integer )
        {
            Com_Printf( "<FAST> " );
        }
        cl.serverTimeDelta = ( cl.serverTimeDelta + newDelta ) >> 1;
    }
    else
    {
        // slow drift adjust, only move 1 or 2 msec
        
        // if any of the frames between this and the previous snapshot
        // had to be extrapolated, nudge our sense of time back a little
        // the granularity of +1 / -2 is too high for timescale modified frametimes
        if( com_timescale->value == 0 || com_timescale->value == 1 )
        {
            if( cl.extrapolatedSnapshot )
            {
                cl.extrapolatedSnapshot = false;
                cl.serverTimeDelta -= 2;
            }
            else
            {
                // otherwise, move our sense of time forward to minimize total latency
                cl.serverTimeDelta++;
            }
        }
    }
    
    if( cl_showTimeDelta->integer )
    {
        Com_Printf( "%i ", cl.serverTimeDelta );
    }
}


/*
==================
CL_FirstSnapshot
==================
*/
void CL_FirstSnapshot( void )
{
    // ignore snapshots that don't have entities
    if( cl.snap.snapFlags & SNAPFLAG_NOT_ACTIVE )
    {
        return;
    }
    cls.state = CA_ACTIVE;
    
    // set the timedelta so we are exactly on this first frame
    cl.serverTimeDelta = cl.snap.serverTime - cls.realtime;
    cl.oldServerTime = cl.snap.serverTime;
    
    clc.timeDemoBaseTime = cl.snap.serverTime;
    
    // if this is the first frame of active play,
    // execute the contents of activeAction now
    // this is to allow scripting a timedemo to start right
    // after loading
    if( cl_activeAction->string[0] )
    {
        Cbuf_AddText( cl_activeAction->string );
        Cbuf_AddText( "\n" );
        Cvar_Set( "activeAction", "" );
    }
}

/*
==================
CL_SetCGameTime
==================
*/
void CL_SetCGameTime( void )
{
    // getting a valid frame message ends the connection process
    if( cls.state != CA_ACTIVE )
    {
        if( cls.state != CA_PRIMED )
        {
            return;
        }
        if( clc.demoplaying )
        {
            // we shouldn't get the first snapshot on the same frame
            // as the gamestate, because it causes a bad time skip
            if( !clc.firstDemoFrameSkipped )
            {
                clc.firstDemoFrameSkipped = true;
                return;
            }
            CL_ReadDemoMessage();
        }
        if( cl.newSnapshots )
        {
            cl.newSnapshots = false;
            CL_FirstSnapshot();
        }
        if( cls.state != CA_ACTIVE )
        {
            return;
        }
    }
    
    // if we have gotten to this point, cl.snap is guaranteed to be valid
    if( !cl.snap.valid )
    {
        Com_Error( ERR_DROP, "CL_SetCGameTime: !cl.snap.valid" );
    }
    
    // allow pause in single player
    if( sv_paused->integer && cl_paused->integer && com_sv_running->integer )
    {
        // paused
        return;
    }
    
    if( cl.snap.serverTime < cl.oldFrameServerTime )
    {
        // Ridah, if this is a localhost, then we are probably loading a savegame
        if( !Q_stricmp( cls.servername, "localhost" ) )
        {
            // do nothing?
            CL_FirstSnapshot();
        }
        else
        {
            Com_Error( ERR_DROP, "cl.snap.serverTime < cl.oldFrameServerTime" );
        }
    }
    cl.oldFrameServerTime = cl.snap.serverTime;
    
    
    // get our current view of time
    
    if( clc.demoplaying && cl_freezeDemo->integer )
    {
        // cl_freezeDemo is used to lock a demo in place for single frame advances
        
    }
    else
    {
        // cl_timeNudge is a user adjustable cvar that allows more
        // or less latency to be added in the interest of better
        // smoothness or better responsiveness.
        S32             tn;
        
        tn = cl_timeNudge->integer;
        if( tn < -30 )
        {
            tn = -30;
        }
        else if( tn > 30 )
        {
            tn = 30;
        }
        
        cl.serverTime = cls.realtime + cl.serverTimeDelta - tn;
        
        // guarantee that time will never flow backwards, even if
        // serverTimeDelta made an adjustment or cl_timeNudge was changed
        if( cl.serverTime < cl.oldServerTime )
        {
            cl.serverTime = cl.oldServerTime;
        }
        cl.oldServerTime = cl.serverTime;
        
        // note if we are almost past the latest frame (without timeNudge),
        // so we will try and adjust back a bit when the next snapshot arrives
        if( cls.realtime + cl.serverTimeDelta >= cl.snap.serverTime - 5 )
        {
            cl.extrapolatedSnapshot = true;
        }
    }
    
    // if we have gotten new snapshots, drift serverTimeDelta
    // don't do this every frame, or a period of packet loss would
    // make a huge adjustment
    if( cl.newSnapshots )
    {
        CL_AdjustTimeDelta();
    }
    
    if( !clc.demoplaying )
    {
        return;
    }
    
    // if we are playing a demo back, we can just keep reading
    // messages from the demo file until the cgame definately
    // has valid snapshots to interpolate between
    
    // a timedemo will always use a deterministic set of time samples
    // no matter what speed machine it is run on,
    // while a normal demo may have different time samples
    // each time it is played back
    if( cl_timedemo->integer )
    {
        if( !clc.timeDemoStart )
        {
            clc.timeDemoStart = Sys_Milliseconds();
        }
        clc.timeDemoFrames++;
        cl.serverTime = clc.timeDemoBaseTime + clc.timeDemoFrames * 50;
    }
    
    while( cl.serverTime >= cl.snap.serverTime )
    {
        // feed another messag, which should change
        // the contents of cl.snap
        CL_ReadDemoMessage();
        if( cls.state != CA_ACTIVE )
        {
            Cvar_Set( "timescale", "1" );
            return;				// end of demo
        }
    }
    
}

/*
====================
CL_GetTag
====================
*/
bool CL_GetTag( S32 clientNum, UTF8* tagname, orientation_t* _or )
{
    if( !cgvm )
    {
        return false;
    }
    
    return cgame->GetTag( clientNum, tagname, _or );
}

