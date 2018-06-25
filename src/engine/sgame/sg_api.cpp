////////////////////////////////////////////////////////////////////////////////////////
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
// File name:   sg_api.cpp
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#include <sgame/sg_precompiled.h>

gameImports_t* imports;
idSoundSystem* soundSystem;
botlib_export_t* botlib;
idCollisionModelManager* collisionModelManager;
idDatabaseSystem* databaseSystem;

//
// dllEntry
//
idGame* dllEntry( gameImports_t* gimports )
{
    imports = gimports;
    
    soundSystem = imports->soundSystem;
    botlib = imports->botlib;
    collisionModelManager = imports->collisionModelManager;
    databaseSystem = imports->databaseSystem;
    return game;
}

#define PASSFLOAT(x) x

void trap_Print( StringEntry fmt )
{
    imports->Printf( fmt );
}

void trap_Error( StringEntry fmt )
{
    imports->Error( ERR_DROP, fmt );
}

S32 trap_Milliseconds( void )
{
    return imports->Milliseconds();
}

void trap_Cvar_Register( vmCvar_t* cvar, StringEntry var_name, StringEntry value, S32 flags )
{
    imports->Cvar_Register( cvar, var_name, value, flags );
}

void trap_Cvar_Update( vmCvar_t* cvar )
{
    imports->Cvar_Update( cvar );
}

void trap_Cvar_Set( StringEntry var_name, StringEntry value )
{
    imports->Cvar_Set( var_name, value );
}

S32 trap_Cvar_VariableIntegerValue( StringEntry var_name )
{
    return imports->Cvar_VariableIntegerValue( var_name );
}

void trap_Cvar_VariableStringBuffer( StringEntry var_name, UTF8* buffer, S32 bufsize )
{
    imports->Cvar_VariableStringBuffer( var_name, buffer, bufsize );
}

void trap_Cvar_LatchedVariableStringBuffer( StringEntry var_name, UTF8* buffer, S32 bufsize )
{
    imports->Cvar_VariableStringBuffer( var_name, buffer, bufsize );
}

S32 trap_Argc( void )
{
    return imports->Argc();
}

void trap_Argv( S32 n, UTF8* buffer, S32 bufferLength )
{
    imports->Argv( n, buffer, bufferLength );
}

void trap_SendConsoleCommand( S32 exec_when, StringEntry text )
{
    imports->SendConsoleCommand( exec_when, text );
}

S32 trap_FS_FOpenFile( StringEntry qpath, fileHandle_t* f, fsMode_t mode )
{
    return imports->FS_FOpenFile( qpath, f, mode );
}

void trap_FS_Read( void* buffer, S32 len, fileHandle_t f )
{
    imports->FS_Read( buffer, len, f );
}

S32 trap_FS_Write( const void* buffer, S32 len, fileHandle_t f )
{
    return imports->FS_Write( buffer, len, f );
}

S32 trap_FS_Rename( StringEntry from, StringEntry to )
{
    imports->FS_Rename( from, to );
    return 0;
}

void trap_FS_FCloseFile( fileHandle_t f )
{
    imports->FS_FCloseFile( f );
}

S32 trap_FS_GetFileList( StringEntry path, StringEntry extension, UTF8* listbuf, S32 bufsize )
{
    return imports->FS_GetFileList( path, extension, listbuf, bufsize );
}

void trap_LocateGameData( gentity_t* gEnts, S32 numGEntities, S32 sizeofGEntity_t, playerState_t* clients, S32 sizeofGClient )
{
    imports->LocateGameData( gEnts, numGEntities, sizeofGEntity_t, clients, sizeofGClient );
}

void trap_DropClient( S32 clientNum, StringEntry reason, S32 length )
{
    imports->DropClient( clientNum, reason, length );
}

void trap_SendServerCommand( S32 clientNum, StringEntry text )
{
    if( strlen( text ) > 1022 )
    {
        gameLocal.LogPrintf( "%s: trap_SendServerCommand( %d, ... ) length exceeds 1022.\n", PRODUCT_VERSION, clientNum );
        gameLocal.LogPrintf( "%s: text [%.950s]... truncated\n", PRODUCT_VERSION, text );
        return;
    }
    imports->SendServerCommand( clientNum, text );
}

void trap_LinkEntity( gentity_t* ent )
{
    imports->LinkEntity( ent );
}

void trap_UnlinkEntity( gentity_t* ent )
{
    imports->UnlinkEntity( ent );
}

S32 trap_EntitiesInBox( const vec3_t mins, const vec3_t maxs, S32* list, S32 maxcount )
{
    return imports->EntitiesInBox( mins, maxs, list, maxcount );
}

bool trap_EntityContact( const vec3_t mins, const vec3_t maxs, const gentity_t* ent )
{
    return imports->EntityContact( mins, maxs, ( sharedEntity_t* )ent, TT_AABB );
}

bool trap_EntityContactCapsule( const vec3_t mins, const vec3_t maxs, const gentity_t* ent )
{
    return imports->EntityContact( mins, maxs, ( sharedEntity_t* )ent, TT_CAPSULE );
}

void trap_Trace( trace_t* results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, S32 passEntityNum, S32 contentmask )
{
    imports->Trace( results, start, mins, maxs, end, passEntityNum, contentmask, TT_AABB );
}

void trap_TraceNoEnts( trace_t* results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, S32 passEntityNum, S32 contentmask )
{
    imports->Trace( results, start, mins, maxs, end, -2, contentmask, TT_AABB );
}

void trap_TraceCapsule( trace_t* results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, S32 passEntityNum, S32 contentmask )
{
    imports->Trace( results, start, mins, maxs, end, passEntityNum, contentmask, TT_CAPSULE );
}

void trap_TraceCapsuleNoEnts( trace_t* results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, S32 passEntityNum, S32 contentmask )
{
    imports->Trace( results, start, mins, maxs, end, -2, contentmask, TT_CAPSULE );
}

S32 trap_PointContents( const vec3_t point, S32 passEntityNum )
{
    return imports->PointContents( point, passEntityNum );
}

void trap_SetBrushModel( gentity_t* ent, StringEntry name )
{
    imports->SetBrushModel( ent, name );
}

bool trap_InPVS( const vec3_t p1, const vec3_t p2 )
{
    return imports->InPVS( p1, p2 );
}

bool trap_InPVSIgnorePortals( const vec3_t p1, const vec3_t p2 )
{
    return imports->InPVSIgnorePortals( p1, p2 );
}

void trap_SetConfigstring( S32 num, StringEntry string )
{
    imports->SetConfigstring( num, string );
}

void trap_GetConfigstring( S32 num, UTF8* buffer, S32 bufferSize )
{
    imports->GetConfigstring( num, buffer, bufferSize );
}

void trap_SetConfigstringRestrictions( S32 num, const clientList_t* clientList )
{
    imports->SetConfigstringRestrictions( num, clientList );
}

void trap_SetUserinfo( S32 num, StringEntry buffer )
{
    imports->SetUserinfo( num, buffer );
}

void trap_GetUserinfo( S32 num, UTF8* buffer, S32 bufferSize )
{
    imports->GetUserinfo( num, buffer, bufferSize );
}

void trap_GetServerinfo( UTF8* buffer, S32 bufferSize )
{
    imports->GetServerinfo( buffer, bufferSize );
}

void trap_AdjustAreaPortalState( gentity_t* ent, bool open )
{
    imports->AdjustAreaPortalState( ent, open );
}

bool trap_AreasConnected( S32 area1, S32 area2 )
{
    return imports->collisionModelManager->AreasConnected( area1, area2 );
}

void trap_UpdateSharedConfig( U32 port, StringEntry rconpass )
{
    imports->UpdateSharedConfig( port, rconpass );
}

S32 trap_BotAllocateClient( S32 clientNum )
{
    return imports->BotAllocateClient( clientNum );
}

void trap_BotFreeClient( S32 clientNum )
{
    imports->BotFreeClient( clientNum );
}

void trap_GetUsercmd( S32 clientNum, usercmd_t* cmd )
{
    imports->GetUsercmd( clientNum, cmd );
}

bool trap_GetEntityToken( UTF8* buffer, S32 bufferSize )
{
    return imports->GetEntityToken( buffer, bufferSize );
}

S32 trap_DebugPolygonCreate( S32 color, S32 numPoints, vec3_t* points )
{
    return imports->DebugPolygonCreate( color, numPoints, points );
}

void trap_DebugPolygonDelete( S32 id )
{
    imports->DebugPolygonDelete( id );
}

S32 trap_RealTime( qtime_t* qtime )
{
    return imports->RealTime( qtime );
}

void trap_SnapVector( F32* v )
{
    imports->Sys_SnapVector( v );
}

void trap_SendGameStat( StringEntry data )
{
    imports->MasterGameStat( data );
}

void trap_AddCommand( StringEntry cmdName )
{
    imports->AddCommand( cmdName, NULL );
}

void trap_RemoveCommand( StringEntry cmdName )
{
    imports->RemoveCommand( cmdName );
}

bool trap_GetTag( S32 clientNum, S32 tagFileNumber, UTF8* tagname, orientation_t* or )
{
    return imports->GetTag( clientNum, tagFileNumber, tagname, or );
}

bool trap_LoadTag( StringEntry filename )
{
    return imports->LoadTag( filename );
}

sfxHandle_t trap_RegisterSound( StringEntry sample, bool compressed )
{
    return imports->soundSystem->RegisterSound( sample, compressed );
}

S32 trap_GetSoundLength( sfxHandle_t sfxHandle )
{
    return imports->soundSystem->GetSoundLength( sfxHandle );
}

S32 trap_BotLibSetup( void )
{
    return imports->BotLibSetup();
}

S32 trap_BotLibShutdown( void )
{
    return imports->BotLibShutdown();
}

S32 trap_BotLibVarSet( UTF8* var_name, UTF8* value )
{
    return imports->botlib->BotLibVarSet( var_name, value );
}

S32 trap_BotLibVarGet( UTF8* var_name, UTF8* value, S32 size )
{
    return imports->botlib->BotLibVarGet( var_name, value, size );
}

S32 trap_BotLibDefine( UTF8* string )
{
    return imports->botlib->PC_AddGlobalDefine( string );
}

S32 trap_PC_LoadSource( StringEntry filename )
{
    return imports->botlib->PC_LoadSourceHandle( filename );
}

S32 trap_PC_FreeSource( S32 handle )
{
    return imports->botlib->PC_FreeSourceHandle( handle );
}

S32 trap_PC_ReadToken( S32 handle, pc_token_t* pc_token )
{
    return imports->botlib->PC_ReadTokenHandle( handle, pc_token );
}

S32 trap_PC_SourceFileAndLine( S32 handle, UTF8* filename, S32* line )
{
    return imports->botlib->PC_SourceFileAndLine( handle, filename, line );
}

S32 trap_BotLibStartFrame( F32 time )
{
    return imports->botlib->BotLibStartFrame( PASSFLOAT( time ) );
}

S32 trap_BotLibLoadMap( StringEntry mapname )
{
    return imports->botlib->BotLibLoadMap( mapname );
}

S32 trap_BotLibUpdateEntity( S32 ent, bot_entitystate_t* bue )
{
    return imports->botlib->BotLibUpdateEntity( ent, bue );
}

S32 trap_BotLibTest( S32 parm0, UTF8* parm1, vec3_t parm2, vec3_t parm3 )
{
    return imports->botlib->Test( parm0, parm1, parm2, parm3 );
}

S32 trap_BotGetSnapshotEntity( S32 clientNum, S32 sequence )
{
    return imports->BotGetSnapshotEntity( clientNum, sequence );
}

S32 trap_BotGetServerCommand( S32 clientNum, UTF8* message, S32 size )
{
    return imports->BotGetConsoleMessage( clientNum, message, size );
}

void trap_BotUserCommand( S32 clientNum, usercmd_t* ucmd )
{
    imports->BotGetUserCommand( clientNum, ucmd );
}

void trap_AAS_EntityInfo( S32 entnum, aas_entityinfo_s* info )
{
    imports->botlib->aas.AAS_EntityInfo( entnum, info );
}

S32 trap_AAS_Initialized( void )
{
    return imports->botlib->aas.AAS_Initialized();
}

void trap_AAS_PresenceTypeBoundingBox( S32 presencetype, vec3_t mins, vec3_t maxs )
{
    imports->botlib->aas.AAS_PresenceTypeBoundingBox( presencetype, mins, maxs );
}

F32 trap_AAS_Time( void )
{
    return imports->botlib->aas.AAS_Time();
}

S32 trap_AAS_PointAreaNum( vec3_t point )
{
    return imports->botlib->aas.AAS_PointAreaNum( point );
}

S32 trap_AAS_TraceAreas( vec3_t start, vec3_t end, S32* areas, vec3_t* points, S32 maxareas )
{
    return imports->botlib->aas.AAS_TraceAreas( start, end, areas, points, maxareas );
}

S32 trap_AAS_BBoxAreas( vec3_t absmins, vec3_t absmaxs, S32* areas, S32 maxareas )
{
    return imports->botlib->aas.AAS_BBoxAreas( absmins, absmaxs, areas, maxareas );
}

S32 trap_AAS_PointContents( vec3_t point )
{
    return imports->botlib->aas.AAS_PointContents( point );
}

S32 trap_AAS_NextBSPEntity( S32 ent )
{
    return imports->botlib->aas.AAS_NextBSPEntity( ent );
}

S32 trap_AAS_ValueForBSPEpairKey( S32 ent, UTF8* key, UTF8* value, S32 size )
{
    return imports->botlib->aas.AAS_ValueForBSPEpairKey( ent, key, value, size );
}

S32 trap_AAS_VectorForBSPEpairKey( S32 ent, UTF8* key, vec3_t v )
{
    return imports->botlib->aas.AAS_VectorForBSPEpairKey( ent, key, v );
}

S32 trap_AAS_FloatForBSPEpairKey( S32 ent, UTF8* key, F32* value )
{
    return imports->botlib->aas.AAS_FloatForBSPEpairKey( ent, key, value );
}

S32 trap_AAS_IntForBSPEpairKey( S32 ent, UTF8* key, S32* value )
{
    return imports->botlib->aas.AAS_IntForBSPEpairKey( ent, key, value );
}

S32 trap_AAS_AreaReachability( S32 areanum )
{
    return imports->botlib->aas.AAS_AreaReachability( areanum );
}

S32 trap_AAS_AreaTravelTimeToGoalArea( S32 areanum, vec3_t origin, S32 goalareanum, S32 travelflags )
{
    return imports->botlib->aas.AAS_AreaTravelTimeToGoalArea( areanum, origin, goalareanum, travelflags );
}

S32 trap_AAS_Swimming( vec3_t origin )
{
    return imports->botlib->aas.AAS_Swimming( origin );
}

S32 trap_AAS_PredictClientMovement( struct aas_clientmove_s* move, S32 entnum, vec3_t origin, S32 presencetype, S32 onground, vec3_t velocity, vec3_t cmdmove, S32 cmdframes, S32 maxframes, F32 frametime, S32 stopevent, S32 stopareanum, S32 visualize )
{
    return imports->botlib->aas.AAS_PredictClientMovement( move, entnum, origin, presencetype, onground, velocity, cmdmove, cmdframes, maxframes, PASSFLOAT( frametime ), stopevent, stopareanum, visualize );
}

void trap_EA_Say( S32 client, UTF8* str )
{
    imports->botlib->ea.EA_Say( client, str );
}

void trap_EA_SayTeam( S32 client, UTF8* str )
{
    imports->botlib->ea.EA_SayTeam( client, str );
}

void trap_EA_Gesture( S32 client )
{
    imports->botlib->ea.EA_Gesture( client );
}

void trap_EA_Command( S32 client, UTF8* command )
{
    imports->botlib->ea.EA_Command( client, command );
}

void trap_EA_Action( S32 client, S32 action )
{
    imports->botlib->ea.EA_Action( client, action );
}

void trap_EA_SelectWeapon( S32 client, S32 weapon )
{
    imports->botlib->ea.EA_SelectWeapon( client, weapon );
}

void trap_EA_Talk( S32 client )
{
    imports->botlib->ea.EA_Talk( client );
}

void trap_EA_Attack( S32 client )
{
    imports->botlib->ea.EA_Attack( client );
}

void trap_EA_Use( S32 client )
{
    imports->botlib->ea.EA_Use( client );
}

void trap_EA_Respawn( S32 client )
{
    imports->botlib->ea.EA_Respawn( client );
}

void trap_EA_Jump( S32 client )
{
    imports->botlib->ea.EA_Jump( client );
}

void trap_EA_DelayedJump( S32 client )
{
    imports->botlib->ea.EA_DelayedJump( client );
}

void trap_EA_Crouch( S32 client )
{
    imports->botlib->ea.EA_Crouch( client );
}

void trap_EA_MoveUp( S32 client )
{
    imports->botlib->ea.EA_MoveUp( client );
}

void trap_EA_MoveDown( S32 client )
{
    imports->botlib->ea.EA_MoveDown( client );
}

void trap_EA_MoveForward( S32 client )
{
    imports->botlib->ea.EA_MoveForward( client );
}

void trap_EA_MoveBack( S32 client )
{
    imports->botlib->ea.EA_MoveBack( client );
}

void trap_EA_MoveLeft( S32 client )
{
    imports->botlib->ea.EA_MoveLeft( client );
}

void trap_EA_MoveRight( S32 client )
{
    imports->botlib->ea.EA_MoveRight( client );
}

void trap_EA_Move( S32 client, vec3_t dir, F32 speed )
{
    imports->botlib->ea.EA_Move( client, dir, PASSFLOAT( speed ) );
}

void trap_EA_View( S32 client, vec3_t viewangles )
{
    imports->botlib->ea.EA_View( client, viewangles );
}

void trap_EA_EndRegular( S32 client, F32 thinktime )
{
    imports->botlib->ea.EA_EndRegular( client, PASSFLOAT( thinktime ) );
}

void trap_EA_GetInput( S32 client, F32 thinktime, bot_input_s* input )
{
    imports->botlib->ea.EA_GetInput( client, PASSFLOAT( thinktime ), input );
}

void trap_EA_ResetInput( S32 client )
{
    imports->botlib->ea.EA_ResetInput( client );
}

S32 trap_BotLoadCharacter( UTF8* charfile, S32 skill )
{
    return imports->botlib->ai.BotLoadCharacter( charfile, skill );
}

void trap_BotFreeCharacter( S32 character )
{
    imports->botlib->ai.BotFreeCharacter( character );
}

F32 trap_Characteristic_Float( S32 character, S32 index )
{
    return imports->botlib->ai.Characteristic_Float( character, index );
}

F32 trap_Characteristic_BFloat( S32 character, S32 index, F32 min, F32 max )
{
    return imports->botlib->ai.Characteristic_BFloat( character, index, min, max );
}

S32 trap_Characteristic_Integer( S32 character, S32 index )
{
    return imports->botlib->ai.Characteristic_Integer( character, index );
}

S32 trap_Characteristic_BInteger( S32 character, S32 index, S32 min, S32 max )
{
    return imports->botlib->ai.Characteristic_BInteger( character, index, min, max );
}

void trap_Characteristic_String( S32 character, S32 index, UTF8* buf, S32 size )
{
    imports->botlib->ai.Characteristic_String( character, index, buf, size );
}

S32 trap_BotAllocChatState( void )
{
    return imports->botlib->ai.BotAllocChatState( );
}

void trap_BotFreeChatState( S32 handle )
{
    imports->botlib->ai.BotFreeChatState( handle );
}

void trap_BotQueueConsoleMessage( S32 chatstate, S32 type, UTF8* message )
{
    imports->botlib->ai.BotQueueConsoleMessage( chatstate, type, message );
}

void trap_BotRemoveConsoleMessage( S32 chatstate, S32 handle )
{
    imports->botlib->ai.BotRemoveConsoleMessage( chatstate, handle );
}

S32 trap_BotNextConsoleMessage( S32 chatstate, bot_consolemessage_s* cm )
{
    return imports->botlib->ai.BotNextConsoleMessage( chatstate, cm );
}

S32 trap_BotNumConsoleMessages( S32 chatstate )
{
    return imports->botlib->ai.BotNumConsoleMessages( chatstate );
}

void trap_BotInitialChat( S32 chatstate, UTF8* type, S32 mcontext, UTF8* var0, UTF8* var1, UTF8* var2, UTF8* var3, UTF8* var4, UTF8* var5, UTF8* var6, UTF8* var7 )
{
    imports->botlib->ai.BotInitialChat( chatstate, type, mcontext, var0, var1, var2, var3, var4, var5, var6, var7 );
}

S32	trap_BotNumInitialChats( S32 chatstate, UTF8* type )
{
    return imports->botlib->ai.BotNumInitialChats( chatstate, type );
}

S32 trap_BotReplyChat( S32 chatstate, UTF8* message, S32 mcontext, S32 vcontext, UTF8* var0, UTF8* var1, UTF8* var2, UTF8* var3, UTF8* var4, UTF8* var5, UTF8* var6, UTF8* var7 )
{
    return imports->botlib->ai.BotReplyChat( chatstate, message, mcontext, vcontext, var0, var1, var2, var3, var4, var5, var6, var7 );
}

S32 trap_BotChatLength( S32 chatstate )
{
    return imports->botlib->ai.BotChatLength( chatstate );
}

void trap_BotEnterChat( S32 chatstate, S32 client, S32 sendto )
{
    imports->botlib->ai.BotEnterChat( chatstate, client, sendto );
}

void trap_BotGetChatMessage( S32 chatstate, UTF8* buf, S32 size )
{
    imports->botlib->ai.BotGetChatMessage( chatstate, buf, size );
}

S32 trap_StringContains( UTF8* str1, UTF8* str2, S32 casesensitive )
{
    return imports->botlib->ai.StringContains( str1, str2, casesensitive );
}

S32 trap_BotFindMatch( UTF8* str, void /* struct bot_match_s */ *match, U64 context )
{
    return imports->botlib->ai.BotFindMatch( str, ( struct bot_match_s* )match, context );
}

void trap_BotMatchVariable( void /* struct bot_match_s */ *match, S32 variable, UTF8* buf, S32 size )
{
    imports->botlib->ai.BotMatchVariable( ( struct bot_match_s* )match, variable, buf, size );
}

void trap_UnifyWhiteSpaces( UTF8* string )
{
    imports->botlib->ai.UnifyWhiteSpaces( string );
}

void trap_BotReplaceSynonyms( UTF8* string, U64 context )
{
    imports->botlib->ai.BotReplaceSynonyms( string, context );
}

S32 trap_BotLoadChatFile( S32 chatstate, UTF8* chatfile, UTF8* chatname )
{
    return imports->botlib->ai.BotLoadChatFile( chatstate, chatfile, chatname );
}

void trap_BotSetChatGender( S32 chatstate, S32 gender )
{
    imports->botlib->ai.BotSetChatGender( chatstate, gender );
}

void trap_BotSetChatName( S32 chatstate, UTF8* name, S32 client )
{
    imports->botlib->ai.BotSetChatName( chatstate, name, client );
}

void trap_BotResetGoalState( S32 goalstate )
{
    imports->botlib->ai.BotResetGoalState( goalstate );
}

void trap_BotResetAvoidGoals( S32 goalstate )
{
    imports->botlib->ai.BotResetAvoidGoals( goalstate );
}

void trap_BotRemoveFromAvoidGoals( S32 goalstate, S32 number )
{
    imports->botlib->ai.BotRemoveFromAvoidGoals( goalstate, number );
}

void trap_BotPushGoal( S32 goalstate, void /* struct bot_goal_s */ *goal )
{
    imports->botlib->ai.BotPushGoal( goalstate, ( bot_goal_s* )goal );
}

void trap_BotPopGoal( S32 goalstate )
{
    imports->botlib->ai.BotPopGoal( goalstate );
}

void trap_BotEmptyGoalStack( S32 goalstate )
{
    imports->botlib->ai.BotEmptyGoalStack( goalstate );
}

void trap_BotDumpAvoidGoals( S32 goalstate )
{
    imports->botlib->ai.BotDumpAvoidGoals( goalstate );
}

void trap_BotDumpGoalStack( S32 goalstate )
{
    imports->botlib->ai.BotDumpGoalStack( goalstate );
}

void trap_BotGoalName( S32 number, UTF8* name, S32 size )
{
    imports->botlib->ai.BotGoalName( number, name, size );
}

S32 trap_BotGetTopGoal( S32 goalstate, void /* struct bot_goal_s */ *goal )
{
    return imports->botlib->ai.BotGetTopGoal( goalstate, ( bot_goal_s* )goal );
}

S32 trap_BotGetSecondGoal( S32 goalstate, void /* struct bot_goal_s */ *goal )
{
    return imports->botlib->ai.BotGetSecondGoal( goalstate, ( bot_goal_s* )goal );
}

S32 trap_BotChooseLTGItem( S32 goalstate, vec3_t origin, S32* inventory, S32 travelflags )
{
    return imports->botlib->ai.BotChooseLTGItem( goalstate, origin, inventory, travelflags );
}

S32 trap_BotChooseNBGItem( S32 goalstate, vec3_t origin, S32* inventory, S32 travelflags, void /* struct bot_goal_s */ *ltg, F32 maxtime )
{
    return imports->botlib->ai.BotChooseNBGItem( goalstate, origin, inventory, travelflags, ( bot_goal_s* )ltg, maxtime );
}

S32 trap_BotTouchingGoal( vec3_t origin, bot_goal_s* goal )
{
    return imports->botlib->ai.BotTouchingGoal( origin, goal );
}

S32 trap_BotItemGoalInVisButNotVisible( S32 viewer, vec3_t eye, vec3_t viewangles, bot_goal_s* goal )
{
    return imports->botlib->ai.BotItemGoalInVisButNotVisible( viewer, eye, viewangles, ( struct bot_goal_s* )goal );
}

S32 trap_BotGetLevelItemGoal( S32 index, UTF8* classname, void /* struct bot_goal_s */ *goal )
{
    return imports->botlib->ai.BotGetLevelItemGoal( index, classname, ( struct bot_goal_s* )goal );
}

S32 trap_BotGetNextCampSpotGoal( S32 num, void /* struct bot_goal_s */ *goal )
{
    return imports->botlib->ai.BotGetNextCampSpotGoal( num, ( struct bot_goal_s* )goal );
}

S32 trap_BotGetMapLocationGoal( UTF8* name, void /* struct bot_goal_s */ *goal )
{
    return imports->botlib->ai.BotGetMapLocationGoal( ( UTF8* )name, ( struct bot_goal_s* )goal );
}

F32 trap_BotAvoidGoalTime( S32 goalstate, S32 number )
{
    return imports->botlib->ai.BotAvoidGoalTime( goalstate, number );
}

void trap_BotInitLevelItems( void )
{
    imports->botlib->ai.BotInitLevelItems( );
}

void trap_BotUpdateEntityItems( void )
{
    imports->botlib->ai.BotUpdateEntityItems( );
}

S32 trap_BotLoadItemWeights( S32 goalstate, UTF8* filename )
{
    return imports->botlib->ai.BotLoadItemWeights( goalstate, filename );
}

void trap_BotFreeItemWeights( S32 goalstate )
{
    imports->botlib->ai.BotFreeItemWeights( goalstate );
}

void trap_BotInterbreedGoalFuzzyLogic( S32 parent1, S32 parent2, S32 child )
{
    imports->botlib->ai.BotInterbreedGoalFuzzyLogic( parent1, parent2, child );
}

void trap_BotSaveGoalFuzzyLogic( S32 goalstate, UTF8* filename )
{
    imports->botlib->ai.BotSaveGoalFuzzyLogic( goalstate, filename );
}

void trap_BotMutateGoalFuzzyLogic( S32 goalstate, F32 range )
{
    imports->botlib->ai.BotMutateGoalFuzzyLogic( goalstate, range );
}

S32 trap_BotAllocGoalState( S32 state )
{
    return imports->botlib->ai.BotAllocGoalState( state );
}

void trap_BotFreeGoalState( S32 handle )
{
    imports->botlib->ai.BotFreeGoalState( handle );
}

void trap_BotResetMoveState( S32 movestate )
{
    imports->botlib->ai.BotResetMoveState( movestate );
}

void trap_BotMoveToGoal( bot_moveresult_s* result, S32 movestate, bot_goal_s* goal, S32 travelflags )
{
    imports->botlib->ai.BotMoveToGoal( result, movestate, goal, travelflags );
}

S32 trap_BotMoveInDirection( S32 movestate, vec3_t dir, F32 speed, S32 type )
{
    return imports->botlib->ai.BotMoveInDirection( movestate, dir, speed, type );
}

void trap_BotResetAvoidReach( S32 movestate )
{
    imports->botlib->ai.BotResetAvoidReach( movestate );
}

void trap_BotResetLastAvoidReach( S32 movestate )
{
    imports->botlib->ai.BotResetLastAvoidReach( movestate );
}

S32 trap_BotReachabilityArea( vec3_t origin, S32 testground )
{
    return imports->botlib->ai.BotReachabilityArea( origin, testground );
}

S32 trap_BotMovementViewTarget( S32 movestate, bot_goal_s* goal, S32 travelflags, F32 lookahead, vec3_t target )
{
    return imports->botlib->ai.BotMovementViewTarget( movestate, goal, travelflags, lookahead, target );
}

S32 trap_BotPredictVisiblePosition( vec3_t origin, S32 areanum, bot_goal_s* goal, S32 travelflags, vec3_t target )
{
    return imports->botlib->ai.BotPredictVisiblePosition( origin, areanum, goal, travelflags, target );
}

S32 trap_BotAllocMoveState( void )
{
    return imports->botlib->ai.BotAllocMoveState();
}

void trap_BotFreeMoveState( S32 handle )
{
    imports->botlib->ai.BotFreeMoveState( handle );
}

void trap_BotInitMoveState( S32 handle, bot_initmove_s* initmove )
{
    imports->botlib->ai.BotInitMoveState( handle, initmove );
}

S32 trap_BotChooseBestFightWeapon( S32 weaponstate, S32* inventory )
{
    return imports->botlib->ai.BotChooseBestFightWeapon( weaponstate, inventory );
}

void trap_BotGetWeaponInfo( S32 weaponstate, S32 weapon, weaponinfo_s* weaponinfo )
{
    imports->botlib->ai.BotGetWeaponInfo( weaponstate, weapon, weaponinfo );
}

S32 trap_BotLoadWeaponWeights( S32 weaponstate, UTF8* filename )
{
    return imports->botlib->ai.BotLoadWeaponWeights( weaponstate, filename );
}

S32 trap_BotAllocWeaponState( void )
{
    return imports->botlib->ai.BotAllocWeaponState();
}

void trap_BotFreeWeaponState( S32 weaponstate )
{
    imports->botlib->ai.BotFreeWeaponState( weaponstate );
}

void trap_BotResetWeaponState( S32 weaponstate )
{
    imports->botlib->ai.BotResetWeaponState( weaponstate );
}

S32 trap_GeneticParentsAndChildSelection( S32 numranks, F32* ranks, S32* parent1, S32* parent2, S32* child )
{
    return imports->botlib->ai.GeneticParentsAndChildSelection( numranks, ranks, parent1, parent2, child );
}

S32 trap_AAS_AlternativeRouteGoals( vec3_t start, S32 startareanum, vec3_t goal, S32 goalareanum, S32 travelflags, aas_altroutegoal_s* altroutegoals, S32 maxaltroutegoals, S32 type )
{
    return imports->botlib->aas.AAS_AlternativeRouteGoals( start, startareanum, goal, goalareanum, travelflags, altroutegoals, maxaltroutegoals, type );
}

void trap_PhysicsSetGravity( const idVec3& gravity )
{
    imports->PhysicsSetGravity( gravity );
}

idTraceModel* trap_AllocTraceModel( void )
{
    return imports->AllocTraceModel();
}

void trap_ResetPhysics( void )
{
    imports->ResetPhysics();
}

void trap_BotSetAvoidGoalTime( S32 goalstate, S32 number, F32 avoidtime )
{
    imports->botlib->ai.BotSetAvoidGoalTime( goalstate, number, avoidtime );
}

void trap_BotAddAvoidSpot( S32 movestate, vec3_t origin, F32 radius, S32 type )
{
    imports->botlib->ai.BotAddAvoidSpot( movestate, origin, radius, type );
}

bool trap_Database_RunQuery( UTF8* query )
{
    return imports->databaseSystem->Runquery( query );
}

void trap_Database_FinishQuery( void )
{
    imports->databaseSystem->FinishQuery();
}

bool trap_Database_FetchRow( void )
{
    return imports->databaseSystem->FetchRow();
}

void trap_Database_FetchfieldbyID( S32 id, UTF8* buffer, S32 len )
{
    imports->databaseSystem->FetchFieldbyID( id, buffer, len );
}

void trap_Database_FetchFieldByName( StringEntry name, UTF8* buffer, S32 len )
{
    imports->databaseSystem->FetchFieldByName( name, buffer, len );
}

S32 trap_S_SoundDuration( sfxHandle_t handle )
{
    return imports->soundSystem->SoundDuration( handle );
}

sfxHandle_t trap_S_RegisterSound( StringEntry sample, bool compressed )
{
    return soundSystem->RegisterSound( sample, compressed );
}
