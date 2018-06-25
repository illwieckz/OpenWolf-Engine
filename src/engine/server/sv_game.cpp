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
// File name:   sv_game.cpp
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description: interface to the game dll
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#include <OWLIb/precompiled.h>

botlib_export_t* botlib_export;

idGame* game;
idGame* ( *gameDllEntry )( gameImports_t* gimports );

static gameImports_t exports;

/*
==================
SV_GameError
==================
*/
void SV_GameError( StringEntry string )
{
    Com_Error( ERR_DROP, "%s", string );
}

/*
==================
SV_GamePrint
==================
*/
void SV_GamePrint( StringEntry string )
{
    Com_Printf( "%s", string );
}

// these functions must be used instead of pointer arithmetic, because
// the game allocates gentities with private information after the server shared part
/*
==================
SV_NumForGentity
==================
*/
S32 SV_NumForGentity( sharedEntity_t* ent )
{
    S32 num;
    
    num = ( ( U8* ) ent - ( U8* ) sv.gentities ) / sv.gentitySize;
    
    return num;
}

/*
==================
SV_GentityNum
==================
*/
sharedEntity_t* SV_GentityNum( S32 num )
{
    sharedEntity_t* ent;
    
    ent = ( sharedEntity_t* )( ( U8* ) sv.gentities + sv.gentitySize * ( num ) );
    
    return ent;
}

/*
==================
SV_GentityNum
==================
*/
playerState_t*  SV_GameClientNum( S32 num )
{
    playerState_t*  ps;
    
    ps = ( playerState_t* )( ( U8* ) sv.gameClients + sv.gameClientSize * ( num ) );
    
    return ps;
}

/*
==================
SV_SvEntityForGentity
==================
*/
svEntity_t* SV_SvEntityForGentity( sharedEntity_t* gEnt )
{
    if( !gEnt || gEnt->s.number < 0 || gEnt->s.number >= MAX_GENTITIES )
    {
        Com_Error( ERR_DROP, "SV_SvEntityForGentity: bad gEnt" );
    }
    return &sv.svEntities[gEnt->s.number];
}

/*
==================
SV_GEntityForSvEntity
==================
*/
sharedEntity_t* SV_GEntityForSvEntity( svEntity_t* svEnt )
{
    S32 num;
    
    num = svEnt - sv.svEntities;
    return SV_GentityNum( num );
}

/*
===============
SV_GameSendServerCommand

Sends a command string to a client
===============
*/
void SV_GameSendServerCommand( S32 clientNum, StringEntry text )
{
    if( clientNum == -1 )
    {
        SV_SendServerCommand( NULL, "%s", text );
    }
    else
    {
        if( clientNum < 0 || clientNum >= sv_maxclients->integer )
        {
            return;
        }
        SV_SendServerCommand( svs.clients + clientNum, "%s", text );
    }
}


/*
===============
SV_GameDropClient

Disconnects the client with a message
===============
*/
void SV_GameDropClient( S32 clientNum, StringEntry reason, S32 length )
{
    if( clientNum < 0 || clientNum >= sv_maxclients->integer )
    {
        return;
    }
    SV_DropClient( svs.clients + clientNum, reason );
    if( length )
    {
        SV_TempBanNetAddress( svs.clients[clientNum].netchan.remoteAddress, length );
    }
}

/*
=================
SV_SetBrushModel

sets mins and maxs for inline bmodels
=================
*/
void SV_SetBrushModel( sharedEntity_t* ent, StringEntry name )
{
    clipHandle_t    h;
    vec3_t          mins, maxs;
    
    if( !name )
    {
        Com_Error( ERR_DROP, "SV_SetBrushModel: NULL" );
    }
    
    if( name[0] != '*' )
    {
        Com_Error( ERR_DROP, "SV_SetBrushModel: %s isn't a brush model", name );
    }
    
    ent->s.modelindex = atoi( name + 1 );
    
    h = collisionModelManager->InlineModel( ent->s.modelindex );
    collisionModelManager->ModelBounds( h, mins, maxs );
    VectorCopy( mins, ent->r.mins );
    VectorCopy( maxs, ent->r.maxs );
    ent->r.bmodel = true;
    
    ent->r.contents = -1;		// we don't know exactly what is in the brushes
    
    //SV_LinkEntity( ent );			// FIXME: remove
}



/*
=================
SV_inPVS

Also checks portalareas so that doors block sight
=================
*/
bool SV_inPVS( const vec3_t p1, const vec3_t p2 )
{
    S32             leafnum, cluster, area1, area2;
    U8*           mask;
    
    leafnum = collisionModelManager->PointLeafnum( p1 );
    cluster = collisionModelManager->LeafCluster( leafnum );
    area1 = collisionModelManager->LeafArea( leafnum );
    mask = collisionModelManager->ClusterPVS( cluster );
    
    leafnum = collisionModelManager->PointLeafnum( p2 );
    cluster = collisionModelManager->LeafCluster( leafnum );
    area2 = collisionModelManager->LeafArea( leafnum );
    if( mask && ( !( mask[cluster >> 3] & ( 1 << ( cluster & 7 ) ) ) ) )
    {
        return false;
    }
    if( !collisionModelManager->AreasConnected( area1, area2 ) )
    {
        return false; // a door blocks sight
    }
    return true;
}


/*
=================
SV_inPVSIgnorePortals

Does NOT check portalareas
=================
*/
bool SV_inPVSIgnorePortals( const vec3_t p1, const vec3_t p2 )
{
    S32             leafnum, cluster, area1, area2;
    U8*           mask;
    
    leafnum = collisionModelManager->PointLeafnum( p1 );
    cluster = collisionModelManager->LeafCluster( leafnum );
    area1 = collisionModelManager->LeafArea( leafnum );
    mask = collisionModelManager->ClusterPVS( cluster );
    
    leafnum = collisionModelManager->PointLeafnum( p2 );
    cluster = collisionModelManager->LeafCluster( leafnum );
    area2 = collisionModelManager->LeafArea( leafnum );
    
    if( mask && ( !( mask[cluster >> 3] & ( 1 << ( cluster & 7 ) ) ) ) )
    {
        return false;
    }
    
    return true;
}


/*
========================
SV_AdjustAreaPortalState
========================
*/
void SV_AdjustAreaPortalState( sharedEntity_t* ent, bool open )
{
    svEntity_t* svEnt;
    
    svEnt = SV_SvEntityForGentity( ent );
    if( svEnt->areanum2 == -1 )
    {
        return;
    }
    collisionModelManager->AdjustAreaPortalState( svEnt->areanum, svEnt->areanum2, open );
}


/*
==================
SV_GameAreaEntities
==================
*/
bool SV_EntityContact( const vec3_t mins, const vec3_t maxs, const sharedEntity_t* gEnt, traceType_t type )
{
    const F32*		origin, *angles;
    clipHandle_t	ch;
    trace_t			trace;
    
    // check for exact collision
    origin = gEnt->r.currentOrigin;
    angles = gEnt->r.currentAngles;
    
    ch = SV_ClipHandleForEntity( gEnt );
    collisionModelManager->TransformedBoxTrace( &trace, vec3_origin, vec3_origin, mins, maxs, ch, -1, origin, angles, type );
    
    return trace.startsolid;
}

/*
===============
SV_GetServerinfo
===============
*/
void SV_GetServerinfo( UTF8* buffer, S32 bufferSize )
{
    if( bufferSize < 1 )
    {
        Com_Error( ERR_DROP, "SV_GetServerinfo: bufferSize == %i", bufferSize );
    }
    Q_strncpyz( buffer, Cvar_InfoString( CVAR_SERVERINFO | CVAR_SERVERINFO_NOUPDATE ), bufferSize );
}

/*
===============
SV_LocateGameData
===============
*/
void SV_LocateGameData( sharedEntity_t* gEnts, S32 numGEntities, S32 sizeofGEntity_t, playerState_t* clients, S32 sizeofGameClient )
{
    sv.gentities = gEnts;
    sv.gentitySize = sizeofGEntity_t;
    sv.num_entities = numGEntities;
    
    sv.gameClients = clients;
    sv.gameClientSize = sizeofGameClient;
}

/*
===============
SV_GetUsercmd
===============
*/
void SV_GetUsercmd( S32 clientNum, usercmd_t* cmd )
{
    if( clientNum < 0 || clientNum >= sv_maxclients->integer )
    {
        Com_Error( ERR_DROP, "SV_GetUsercmd: bad clientNum:%i", clientNum );
    }
    *cmd = svs.clients[clientNum].lastUsercmd;
}

// taken from cl_main.c
#define MAX_RCON_MESSAGE 1024

/*
===============
SV_UpdateSharedConfig
===============
*/
void SV_UpdateSharedConfig( U32 port, StringEntry rconpass )
{
    UTF8 		message[MAX_RCON_MESSAGE];
    netadr_t	to;
    
    message[0] = -1;
    message[1] = -1;
    message[2] = -1;
    message[3] = -1;
    message[4] = 0;
    
    Q_strcat( message, MAX_RCON_MESSAGE, "rcon " );
    
    Q_strcat( message, MAX_RCON_MESSAGE, rconpass );
    Q_strcat( message, MAX_RCON_MESSAGE, " !readconfig" );
    NET_StringToAdr( "127.0.0.1", &to, NA_UNSPEC );
    to.port = BigShort( port );
    
    NET_SendPacket( NS_SERVER, strlen( message ) + 1, message, to );
}

//==============================================

/*
===============
SV_GetEntityToken
===============
*/
bool SV_GetEntityToken( UTF8* buffer, S32 bufferSize )
{
    StringEntry  s;
    
    s = COM_Parse( &sv.entityParsePoint );
    Q_strncpyz( buffer, s, bufferSize );
    if( !sv.entityParsePoint && !s[0] )
    {
        return false;
    }
    else
    {
        return true;
    }
}

//
// SV_PhysicsSetGravity
//
void SV_PhysicsSetGravity( const idVec3& gravity )
{
    physicsManager->SetGravity( gravity );
}

//
// SV_AllocTraceModel
//
idTraceModel* SV_AllocTraceModel( void )
{
    return physicsManager->AllocTraceModel();
}

//
// SV_ResetPhysics
//
void SV_ResetPhysics( void )
{
    physicsManager->Reset();
}

// SV_BotGetUserCommand
//
void SV_BotGetUserCommand( S32 clientNum, usercmd_t* ucmd )
{
    SV_ClientThink( &svs.clients[clientNum], ucmd );
}

/*
====================
SV_InitExportTable
====================
*/
void SV_InitExportTable( void )
{
    exports.Printf = Com_Printf;
    exports.Error = Com_Error;;
    exports.Milliseconds = Sys_Milliseconds;
    exports.Cvar_Register = Cvar_Register;
    exports.Cvar_Update = Cvar_Update;
    exports.Cvar_Set = Cvar_Set;;
    exports.Cvar_VariableIntegerValue = Cvar_VariableIntegerValue;
    exports.Cvar_VariableStringBuffer = Cvar_VariableStringBuffer;
    exports.Cvar_LatchedVariableStringBuffer = Cvar_LatchedVariableStringBuffer;
    exports.Argc = Cmd_Argc;
    exports.Argv = Cmd_ArgvBuffer;
    exports.SendConsoleCommand = Cbuf_ExecuteText;
    exports.FS_FOpenFile = FS_FOpenFileByMode;
    exports.FS_Read = FS_Read2;
    exports.FS_Write = FS_Write;
    exports.FS_Rename = FS_Rename;
    exports.FS_FCloseFile = FS_FCloseFile;
    exports.FS_GetFileList = FS_GetFileList;
    exports.LocateGameData = SV_LocateGameData;
    exports.DropClient = SV_GameDropClient;
    exports.SendServerCommand = SV_GameSendServerCommand;
    exports.LinkEntity = SV_LinkEntity;
    exports.UnlinkEntity = SV_UnlinkEntity;
    exports.EntitiesInBox = SV_AreaEntities;
    exports.EntityContact = SV_EntityContact;
    exports.Trace = SV_Trace;
    exports.PointContents = SV_PointContents;
    exports.SetBrushModel = SV_SetBrushModel;
    exports.InPVS = SV_inPVS;
    exports.InPVSIgnorePortals = SV_inPVSIgnorePortals;
    exports.SetConfigstring = SV_SetConfigstring;
    exports.GetConfigstring = SV_GetConfigstring;
    exports.SetConfigstringRestrictions = SV_SetConfigstringRestrictions;
    exports.SetUserinfo = SV_SetUserinfo;
    exports.GetUserinfo = SV_GetUserinfo;
    exports.GetServerinfo = SV_GetServerinfo;
    exports.AdjustAreaPortalState = SV_AdjustAreaPortalState;
    exports.UpdateSharedConfig = SV_UpdateSharedConfig;
    exports.BotAllocateClient = SV_BotAllocateClient;
    exports.BotFreeClient = SV_BotFreeClient;
    exports.BotGetSnapshotEntity = SV_BotGetSnapshotEntity;
    exports.BotGetConsoleMessage = SV_BotGetConsoleMessage;
    exports.GetUsercmd = SV_GetUsercmd;
    exports.DebugPolygonCreate = BotImport_DebugPolygonCreate;
    exports.DebugPolygonDelete = BotImport_DebugPolygonDelete;
    exports.RealTime = Com_RealTime;
    exports.Sys_SnapVector = Sys_SnapVector;
    exports.MasterGameStat = SV_MasterGameStat;
    exports.AddCommand = Cmd_AddCommand;
    exports.RemoveCommand = Cmd_RemoveCommand;
    exports.GetTag = SV_GetTag;
    exports.LoadTag = SV_LoadTag;
    exports.BotLibSetup = SV_BotLibSetup;
    exports.BotLibShutdown = SV_BotLibShutdown;
    exports.GetEntityToken = SV_GetEntityToken;
    exports.BotGetUserCommand = SV_BotGetUserCommand;
    exports.PhysicsSetGravity = SV_PhysicsSetGravity;
    exports.AllocTraceModel = SV_AllocTraceModel;
    exports.ResetPhysics = SV_ResetPhysics;
    
    exports.botlib = botlib_export;
    exports.collisionModelManager = collisionModelManager;
#ifndef DEDICATED
    exports.soundSystem = soundSystem;
#endif
    exports.databaseSystem = databaseSystem;
}

/*
===============
SV_ShutdownGameProgs

Called every time a map changes
===============
*/
void SV_ShutdownGameProgs( void )
{
    if( !gvm || game == NULL )
    {
        return;
    }
    game->Shutdown( false );
    game = NULL;
    
    Sys_UnloadDll( gvm );
    gvm = NULL;
    if( sv_newGameShlib->string[0] )
    {
        FS_Rename( sv_newGameShlib->string, "game" DLL_EXT );
        Cvar_Set( "sv_newGameShlib", "" );
    }
}

/*
==================
SV_InitGameVM

Called for both a full init and a restart
==================
*/
static void SV_InitGameVM( bool restart )
{
    S32 i;
    
    // start the entity parsing at the beginning
    sv.entityParsePoint = collisionModelManager->EntityString();
    
    // clear all gentity pointers that might still be set from
    // a previous level
    for( i = 0; i < sv_maxclients->integer; i++ )
    {
        svs.clients[i].gentity = NULL;
    }
    
    // use the current msec count for a random seed
    // init for this gamestate
    game->Init( svs.time, Com_Milliseconds(), restart );
}

/*
===================
SV_RestartGameProgs

Called on a map_restart, but not on a normal map change
===================
*/
void SV_RestartGameProgs( void )
{
    if( !gvm )
    {
        return;
    }
    game->Shutdown( true );
    
    SV_InitGameVM( true );
}


/*
===============
SV_InitGameProgs

Called on a normal map change, not on a map_restart
===============
*/
void SV_InitGameProgs( void )
{
    sv.num_tagheaders = 0;
    sv.num_tags = 0;
    
    // load the dll or bytecode
    gvm = Sys_LoadDll( "sgame" );
    if( !gvm )
    {
        Com_Error( ERR_FATAL, "VM_Create on game failed" );
    }
    
    // Get the entry point.
    gameDllEntry = ( idGame * ( __cdecl* )( gameImports_t* ) )Sys_GetProcAddress( gvm, "dllEntry" );
    if( !gameDllEntry )
    {
        Com_Error( ERR_FATAL, "gameDllEntry on game failed.\n" );
    }
    
    // Init the export table.
    SV_InitExportTable();
    
    game = gameDllEntry( &exports );
    
    SV_InitGameVM( false );
}


/*
====================
SV_GameCommand

See if the current console command is claimed by the game
====================
*/
bool SV_GameCommand( void )
{
    if( sv.state != SS_GAME )
    {
        return false;
    }
    
    return game->ConsoleCommand();
}

/*
====================
SV_GameIsSinglePlayer
====================
*/
bool SV_GameIsSinglePlayer( void )
{
    return ( bool )( com_gameInfo.spGameTypes & ( 1 << g_gameType->integer ) );
}

/*
====================
SV_GameIsCoop

	This is a modified SinglePlayer, no savegame capability for example
====================
*/
bool SV_GameIsCoop( void )
{
    return ( bool )( com_gameInfo.coopGameTypes & ( 1 << g_gameType->integer ) );
}

/*
====================
SV_GetTag
return false if unable to retrieve tag information for this client
====================
*/
extern bool CL_GetTag( S32 clientNum, UTF8* tagname, orientation_t* or );

bool SV_GetTag( S32 clientNum, S32 tagFileNumber, UTF8* tagname, orientation_t* or )
{
    S32 i;
    
    if( tagFileNumber > 0 && tagFileNumber <= sv.num_tagheaders )
    {
        for( i = sv.tagHeadersExt[tagFileNumber - 1].start; i < sv.tagHeadersExt[tagFileNumber - 1].start + sv.tagHeadersExt[tagFileNumber - 1].count; i++ )
        {
            if( !Q_stricmp( sv.tags[i].name, tagname ) )
            {
                VectorCopy( sv.tags[i].origin, or ->origin );
                VectorCopy( sv.tags[i].axis[0], or ->axis[0] );
                VectorCopy( sv.tags[i].axis[1], or ->axis[1] );
                VectorCopy( sv.tags[i].axis[2], or ->axis[2] );
                return true;
            }
        }
    }
    
    // Gordon: lets try and remove the inconsitancy between ded/non-ded servers...
    // Gordon: bleh, some code in clientthink_real really relies on this working on player models...
#ifndef DEDICATED				// TTimo: dedicated only binary defines DEDICATED
    if( com_dedicated->integer )
    {
        return false;
    }
    
    return CL_GetTag( clientNum, tagname, or );
#else
    return false;
#endif
}
