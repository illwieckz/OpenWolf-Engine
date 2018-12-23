////////////////////////////////////////////////////////////////////////////////////////
// Copyright(C) 2011-2018 Dusan Jocic <dusanjocic@msn.com>
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
// File name:   sgame_api.h
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2017, gcc 7.3.0
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#ifndef __SG_API_H__
#define __SG_API_H__

#ifndef __Q_SHARED_H__
#include <qcommon/q_shared.h>
#endif
#ifndef __BOTLIB_H__
#include <botlib/botlib.h>
#endif

typedef void ( *xcommand_t )( void );

#define GAME_API_VERSION 1

#define SVF_NOCLIENT                  0x00000001
#define SVF_CLIENTMASK                0x00000002
#define SVF_VISDUMMY                  0x00000004
#define SVF_BOT                       0x00000008
#define SVF_POW                       0x00000010 // ignored by the engine
#define SVF_BROADCAST                 0x00000020
#define SVF_PORTAL                    0x00000040
#define SVF_BLANK                     0x00000080 // ignored by the engine
#define SVF_NOFOOTSTEPS               0x00000100 // ignored by the engine
#define SVF_CAPSULE                   0x00000200
#define SVF_VISDUMMY_MULTIPLE         0x00000400
#define SVF_SINGLECLIENT              0x00000800
#define SVF_NOSERVERINFO              0x00001000 // only meaningful for entities numbered in [0..MAX_CLIENTS)
#define SVF_NOTSINGLECLIENT           0x00002000
#define SVF_IGNOREBMODELEXTENTS       0x00004000
#define SVF_SELF_PORTAL               0x00008000
#define SVF_SELF_PORTAL_EXCLUSIVE     0x00010000
#define SVF_RIGID_BODY                0x00020000 // ignored by the engine
#define SVF_USE_CURRENT_ORIGIN        0x00040000 // ignored by the engine

#ifdef GAMEDLL
typedef struct gclient_s gclient_t;
typedef struct gentity_s gentity_t;
#endif

//
// system functions provided by the main engine
//
struct gameImports_t
{
    void( *Printf )( StringEntry fmt, ... );
    void( *Error )( S32 level, StringEntry fmt, ... );
    
    void( *Endgame )( void );
    
    S32( *Milliseconds )( void );
    S32( *Argc )( void );
    
    void( *Argv )( S32 n, UTF8* buffer, S32 bufferLength );
    
    void( *SendConsoleCommand )( S32 exec_when, StringEntry text );
    
    
    void( *SetConfigstring )( S32 num, StringEntry string );
    void( *GetConfigstring )( S32 num, UTF8* buffer, S32 bufferSize );
    void( *GetUserinfo )( S32 num, UTF8* buffer, S32 bufferSize );
    void( *SetUserinfo )( S32 num, StringEntry buffer );
    
    S32( *RealTime )( qtime_t* qtime );
    
    void( *SetConfigstringRestrictions )( S32 index, const clientList_t* clientList );
    void ( *AddCommand )( StringEntry cmd_name, xcommand_t function );
    void ( *RemoveCommand )( StringEntry cmd_name );
    
    S32( *LoadTag )( StringEntry mod_name );
    void( *MasterGameStat )( StringEntry data );
    void( *Sys_SnapVector )( F32* v );
    
    botlib_export_t* botlib;
    idDatabaseSystem* databaseSystem;
    idSoundSystem* soundSystem;
    idCollisionModelManager* collisionModelManager;
    idFileSystem* fileSystem;
    idCVarSystem* cvarSystem;
    idServerBotSystem* serverBotSystem;
    idServerGameSystem* serverGameSystem;
    idServerWorldSystem* serverWorldSystem;
};

void trap_Print( StringEntry fmt );
void trap_Error( StringEntry fmt );
S32 trap_Milliseconds( void );
void trap_Cvar_Register( vmCvar_t* cvar, StringEntry var_name, StringEntry value, S32 flags );
void trap_Cvar_Set( StringEntry var_name, StringEntry value );
void trap_Cvar_Update( vmCvar_t* cvar );
S32 trap_Cvar_VariableIntegerValue( StringEntry var_name );
void trap_Cvar_VariableStringBuffer( StringEntry var_name, UTF8* buffer, S32 bufsize );
void trap_Cvar_LatchedVariableStringBuffer( StringEntry var_name, UTF8* buffer, S32 bufsize );
S32 trap_Argc( void );
void trap_Argv( S32 n, UTF8* buffer, S32 bufferLength );
void trap_SendConsoleCommand( S32 exec_when, StringEntry text );
S32 trap_FS_FOpenFile( StringEntry qpath, fileHandle_t* f, fsMode_t mode );
void trap_FS_Read( void* buffer, S32 len, fileHandle_t f );
S32 trap_FS_Write( const void* buffer, S32 len, fileHandle_t f );
S32 trap_FS_Rename( StringEntry from, StringEntry to );
void trap_FS_FCloseFile( fileHandle_t f );
S32 trap_FS_GetFileList( StringEntry path, StringEntry extension, UTF8* listbuf, S32 bufsize );
void trap_LocateGameData( gentity_t* gEnts, S32 numGEntities, S32 sizeofGEntity_t, playerState_t* clients, S32 sizeofGClient );
void trap_DropClient( S32 clientNum, StringEntry reason, S32 length );
void trap_SendServerCommand( S32 clientNum, StringEntry text );
void trap_SetConfigstring( S32 num, StringEntry string );
void trap_LinkEntity( gentity_t* ent );
void trap_UnlinkEntity( gentity_t* ent );
S32 trap_EntitiesInBox( const vec3_t mins, const vec3_t maxs, S32* list, S32 maxcount );
bool trap_EntityContact( const vec3_t mins, const vec3_t maxs, const gentity_t* ent );
bool trap_EntityContactCapsule( const vec3_t mins, const vec3_t maxs, const gentity_t* ent );
void trap_Trace( trace_t* results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, S32 passEntityNum, S32 contentmask );
void trap_TraceNoEnts( trace_t* results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, S32 passEntityNum, S32 contentmask );
void trap_TraceCapsule( trace_t* results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, S32 passEntityNum, S32 contentmask );
void trap_TraceCapsuleNoEnts( trace_t* results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, S32 passEntityNum, S32 contentmask );
S32 trap_PointContents( const vec3_t point, S32 passEntityNum );
void trap_SetBrushModel( gentity_t* ent, StringEntry name );
bool trap_InPVS( const vec3_t p1, const vec3_t p2 );
bool trap_InPVSIgnorePortals( const vec3_t p1, const vec3_t p2 );
void trap_SetConfigstringRestrictions( S32 num, const clientList_t* clientList );
void trap_GetConfigstring( S32 num, UTF8* buffer, S32 bufferSize );
void trap_SetConfigstringRestrictions( S32 num, const clientList_t* clientList );
void trap_SetUserinfo( S32 num, StringEntry buffer );
void trap_GetUserinfo( S32 num, UTF8* buffer, S32 bufferSize );
void trap_GetServerinfo( UTF8* buffer, S32 bufferSize );
void trap_AdjustAreaPortalState( gentity_t* ent, bool open );
bool trap_AreasConnected( S32 area1, S32 area2 );
void trap_UpdateSharedConfig( U32 port, StringEntry rconpass );
S32 trap_BotAllocateClient( S32 clientNum );
void trap_BotFreeClient( S32 clientNum );
void trap_GetUsercmd( S32 clientNum, usercmd_t* cmd );
bool trap_GetEntityToken( UTF8* buffer, S32 bufferSize );
S32 trap_RealTime( qtime_t* qtime );
void trap_SnapVector( F32* v );
void trap_SendGameStat( StringEntry data );
void trap_AddCommand( StringEntry cmdName );
void trap_RemoveCommand( StringEntry cmdName );
bool trap_GetTag( S32 clientNum, S32 tagFileNumber, UTF8* tagName, orientation_t* ori );
bool trap_LoadTag( StringEntry filename );
sfxHandle_t trap_RegisterSound( StringEntry sample );
S32 trap_GetSoundLength( sfxHandle_t sfxHandle );
S32 trap_BotLibSetup( void );
S32 trap_BotLibVarSet( UTF8* var_name, UTF8* value );
S32 trap_BotLibVarSet( UTF8* var_name, UTF8* value );
S32 trap_BotLibVarGet( UTF8* var_name, UTF8* value, S32 size );
void trap_EA_Say( S32 client, UTF8* str );
void trap_EA_SayTeam( S32 client, UTF8* str );
void trap_EA_Command( S32 client, UTF8* command );
void trap_EA_Action( S32 client, S32 action );
void trap_EA_Gesture( S32 client );
void trap_EA_Talk( S32 client );
void trap_EA_Attack( S32 client );
void trap_EA_Use( S32 client );
void trap_EA_Respawn( S32 client );
void trap_EA_Crouch( S32 client );
void trap_EA_MoveUp( S32 client );
void trap_EA_MoveDown( S32 client );
void trap_EA_MoveForward( S32 client );
void trap_EA_MoveBack( S32 client );
void trap_EA_MoveLeft( S32 client );
void trap_EA_MoveRight( S32 client );
void trap_EA_SelectWeapon( S32 client, S32 weapon );
void trap_EA_Jump( S32 client );
void trap_EA_DelayedJump( S32 client );
void trap_EA_Move( S32 client, vec3_t dir, F32 speed );
void trap_EA_View( S32 client, vec3_t viewangles );
void trap_EA_EndRegular( S32 client, F32 thinktime );
void trap_EA_GetInput( S32 client, F32 thinktime, bot_input_s* input );
void trap_EA_ResetInput( S32 client );
S32 trap_BotLibLoadMap( StringEntry mapname );
F32 trap_AAS_Time( void );
S32 trap_AAS_Initialized( void );
S32 trap_AAS_PointAreaNum( vec3_t point );
S32 trap_AAS_AreaReachability( S32 areanum );
S32 trap_AAS_TraceAreas( vec3_t start, vec3_t end, S32* areas, vec3_t* points, S32 maxareas );
S32 trap_AAS_AreaTravelTimeToGoalArea( S32 areanum, vec3_t origin, S32 goalareanum, S32 travelflags );
void trap_AAS_EntityInfo( S32 entnum, aas_entityinfo_s* info );
S32 trap_BotLibStartFrame( F32 time );
S32 trap_BotLibVarSet( UTF8* var_name, UTF8* value );
S32 trap_BotLibVarGet( UTF8* var_name, UTF8* value, S32 size );
S32 trap_BotLibUpdateEntity( S32 ent, bot_entitystate_t* bue );
S32 trap_BotGetServerCommand( S32 clientNum, UTF8* message, S32 size );
void trap_BotUserCommand( S32 clientNum, usercmd_t* ucmd );
void trap_BotMoveToGoal( bot_moveresult_s* result, S32 movestate, bot_goal_s* goal, S32 travelflags );
void trap_BotInitMoveState( S32 handle, bot_initmove_s* initmove );
S32 trap_BotAllocMoveState( void );
void trap_BotFreeMoveState( S32 handle );
S32 trap_BotMovementViewTarget( S32 movestate, bot_goal_s* goal, S32 travelflags, F32 lookahead, vec3_t target );
S32 trap_BotPredictVisiblePosition( vec3_t origin, S32 areanum, bot_goal_s* goal, S32 travelflags, vec3_t target );
S32 trap_BotAllocChatState( void );
S32	trap_AAS_PointContents( vec3_t point );
void trap_AAS_PresenceTypeBoundingBox( S32 presencetype, vec3_t mins, vec3_t maxs );
void trap_BotEnterChat( S32 chatstate, S32 client, S32 sendto );
void trap_BotInitialChat( S32 chatstate, UTF8* type, S32 mcontext, UTF8* var0, UTF8* var1, UTF8* var2, UTF8* var3, UTF8* var4, UTF8* var5, UTF8* var6, UTF8* var7 );
S32	trap_BotNumInitialChats( S32 chatstate, UTF8* type );
S32 trap_BotChatLength( S32 chatstate );
S32 trap_Characteristic_BInteger( S32 character, S32 index, S32 min, S32 max );
F32 trap_Characteristic_BFloat( S32 character, S32 index, F32 min, F32 max );
S32	trap_AAS_PredictClientMovement( aas_clientmove_s* move, S32 entnum, vec3_t origin, S32 presencetype, S32 onground, vec3_t velocity, vec3_t cmdmove, S32 cmdframes, S32 maxframes, F32 frametime, S32 stopevent, S32 stopareanum, S32 visualize );
void trap_BotGetWeaponInfo( S32 weaponstate, S32 weapon, weaponinfo_s* weaponinfo );
S32	trap_BotMoveInDirection( S32 movestate, vec3_t dir, F32 speed, S32 type );
S32	trap_BotAllocWeaponState( void );
void trap_Characteristic_String( S32 character, S32 index, UTF8* buf, S32 size );
S32	trap_BotLoadCharacter( UTF8* charfile, S32 skill );
S32	trap_BotLoadChatFile( S32 chatstate, UTF8* chatfile, UTF8* chatname );
void trap_BotSetChatName( S32 chatstate, UTF8* name, S32 client );
S32 trap_BotLibShutdown( void );
S32 trap_AAS_AlternativeRouteGoals( vec3_t start, S32 startareanum, vec3_t goal, S32 goalareanum, S32 travelflags, aas_altroutegoal_s* altroutegoals, S32 maxaltroutegoals, S32 type );
void trap_PhysicsSetGravity( const idVec3& gravity );
idTraceModel* trap_AllocTraceModel( void );
void trap_ResetPhysics( void );
void trap_BotAddAvoidSpot( S32 movestate, vec3_t origin, F32 radius, S32 type );
void trap_BotFreeChatState( S32 handle );
S32	trap_BotGetSnapshotEntity( S32 clientNum, S32 sequence );
void trap_BotFreeWeaponState( S32 weaponstate );
void trap_BotFreeCharacter( S32 character );
S32 trap_AAS_Swimming( vec3_t origin );
S32 trap_BotTouchingGoal( vec3_t origin, bot_goal_s* goal );
S32 trap_BotItemGoalInVisButNotVisible( S32 viewer, vec3_t eye, vec3_t viewangles, bot_goal_s* goal );
void trap_BotSetAvoidGoalTime( S32 goalstate, S32 number, F32 avoidtime );
S32 trap_AAS_BBoxAreas( vec3_t absmins, vec3_t absmaxs, S32* areas, S32 maxareas );
void trap_BotResetAvoidReach( S32 movestate );
void trap_BotResetMoveState( S32 movestate );
void trap_BotResetGoalState( S32 goalstate );
void trap_BotResetWeaponState( S32 weaponstate );
void trap_BotResetAvoidGoals( S32 goalstate );
void trap_BotQueueConsoleMessage( S32 chatstate, S32 type, UTF8* message );
bool trap_Database_RunQuery( UTF8* query );
void trap_Database_FinishQuery( void );
bool trap_Database_FetchRow( void );
void trap_Database_FetchfieldbyID( S32 id, UTF8* buffer, S32 len );
void trap_Database_FetchFieldByName( StringEntry name, UTF8* buffer, S32 len );
sfxHandle_t trap_S_RegisterSound( StringEntry sample );
S32 trap_S_SoundDuration( sfxHandle_t handle );

//
// idGame
//
class idGame
{
public:
    virtual void Init( S32 levelTime, S32 randomSeed, S32 restart ) = 0;
    virtual void Shutdown( S32 restart ) = 0;
    virtual UTF8* ClientConnect( S32 clientNum, bool firstTime, bool isBot ) = 0;
    virtual void ClientBegin( S32 clientNum ) = 0;
    virtual void ClientThink( S32 clientNum ) = 0;
    virtual void ClientUserinfoChanged( S32 clientNum ) = 0;
    virtual void ClientDisconnect( S32 clientNum ) = 0;
    virtual void ClientCommand( S32 clientNum ) = 0;
    virtual void RunFrame( S32 levelTime ) = 0;
    virtual bool ConsoleCommand( void ) = 0;
    virtual S32	BotAIStartFrame( S32 time ) = 0;
    virtual bool SnapshotCallback( S32 entityNum, S32 clientNum ) = 0;
    virtual bool BotVisibleFromPos( vec3_t srcorigin, S32 srcnum, vec3_t destorigin, S32 destent, bool dummy ) = 0;
    virtual bool BotCheckAttackAtPos( S32 entnum, S32 enemy, vec3_t pos, bool ducking, bool allowHitWorld ) = 0;
};

extern idGame* game;

#endif // !__SG_API_H__
