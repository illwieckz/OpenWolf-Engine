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
// File name:   cg_api.cpp
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#include <OWLIb/precompiled.h>

#define PASSFLOAT(x) x

cgameImports_t* imports;
idSoundSystem* soundSystem;
idRenderSystem* renderSystem;
idCollisionModelManager* collisionModelManager;

idCGame* dllEntry( cgameImports_t* cgimports )
{
    imports = cgimports;
    
    soundSystem = imports->soundSystem;
    renderSystem = imports->renderSystem;
    collisionModelManager = imports->collisionModelManager;
    return cgame;
}

void trap_Print( StringEntry fmt )
{
    imports->Print( fmt );
}

void trap_Error( StringEntry fmt )
{
    imports->Error( ERR_DROP, fmt );
}

S32 trap_Milliseconds( void )
{
    return imports->Milliseconds();
}

void trap_Cvar_Register( vmCvar_t* vmCvar, StringEntry varName, StringEntry defaultValue, S32 flags )
{
    imports->Cvar_Register( vmCvar, varName, defaultValue, flags );
}

void trap_Cvar_Update( vmCvar_t* vmCvar )
{
    imports->Cvar_Update( vmCvar );
}

void trap_Cvar_Set( StringEntry var_name, StringEntry value )
{
    imports->Cvar_Set( var_name, value );
}

void trap_Cvar_VariableStringBuffer( StringEntry var_name, UTF8* buffer, S32 bufsize )
{
    imports->Cvar_VariableStringBuffer( var_name, buffer, bufsize );
}

void trap_Cvar_LatchedVariableStringBuffer( StringEntry var_name, UTF8* buffer, S32 bufsize )
{
    imports->Cvar_LatchedVariableStringBuffer( var_name, buffer, bufsize );
}

S32 trap_Argc( void )
{
    return imports->Argc();
}

void trap_Argv( S32 n, UTF8* buffer, S32 bufferLength )
{
    imports->Argv( n, buffer, bufferLength );
}

void trap_Args( UTF8* buffer, S32 bufferLength )
{
    imports->Args( buffer, bufferLength );
}

void trap_LiteralArgs( UTF8* buffer, S32 bufferLength )
{
    imports->LiteralArgsBuffer( buffer, bufferLength );
}

S32 trap_GetDemoState( void )
{
    return imports->DemoState();
}

S32 trap_GetDemoPos( void )
{
    return imports->DemoPos();
}

S32 trap_FS_FOpenFile( StringEntry qpath, fileHandle_t* f, fsMode_t mode )
{
    return imports->FS_FOpenFile( qpath, f, mode );
}

void trap_FS_Read( void* buffer, S32 len, fileHandle_t f )
{
    imports->FS_Read( buffer, len, f );
}

void trap_FS_Write( const void* buffer, S32 len, fileHandle_t f )
{
    imports->FS_Write( buffer, len, f );
}

void trap_FS_FCloseFile( fileHandle_t f )
{
    imports->FS_FCloseFile( f );
}

S32 trap_FS_GetFileList( StringEntry path, StringEntry extension, UTF8* listbuf, S32 bufsize )
{
    return imports->FS_GetFileList( path, extension, listbuf, bufsize );
}

S32 trap_FS_Delete( UTF8* filename )
{
    return imports->FS_Delete( filename );
}

void trap_SendConsoleCommand( StringEntry text )
{
    imports->SendConsoleCommand( text );
}

void trap_AddCommand( StringEntry cmdName )
{
    imports->AddCommand( cmdName );
}

void trap_RemoveCommand( StringEntry cmdName )
{
    imports->Cmd_RemoveCommand( cmdName );
}

void trap_SendClientCommand( StringEntry s )
{
    imports->AddReliableCommand( s );
}

void trap_UpdateScreen( void )
{
    imports->UpdateScreen();
}

void trap_CM_LoadMap( StringEntry mapname )
{
    imports->LoadMap( mapname );
}

S32 trap_CM_NumInlineModels( void )
{
    return imports->collisionModelManager->NumInlineModels( );
}

clipHandle_t trap_CM_InlineModel( S32 index )
{
    return imports->collisionModelManager->InlineModel( index );
}

clipHandle_t trap_CM_TempBoxModel( const vec3_t mins, const vec3_t maxs )
{
    return imports->collisionModelManager->TempBoxModel( mins, maxs, false );
}

clipHandle_t trap_CM_TempCapsuleModel( const vec3_t mins, const vec3_t maxs )
{
    return imports->collisionModelManager->TempBoxModel( mins, maxs, true );
}

S32 trap_CM_PointContents( const vec3_t p, clipHandle_t model )
{
    return imports->collisionModelManager->PointContents( p, model );
}

S32 trap_CM_TransformedPointContents( const vec3_t p, clipHandle_t model, const vec3_t origin, const vec3_t angles )
{
    return imports->collisionModelManager->TransformedPointContents( p, model, origin, angles );
}

void trap_CM_BoxTrace( trace_t* results, const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs, clipHandle_t model, S32 brushmask )
{
    imports->collisionModelManager->BoxTrace( results, start, end, mins, maxs, model, brushmask, TT_AABB );
}

void trap_CM_TransformedBoxTrace( trace_t* results, const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs, clipHandle_t model, S32 brushmask, const vec3_t origin, const vec3_t angles )
{
    imports->collisionModelManager->TransformedBoxTrace( results, start, end, mins, maxs, model, brushmask, origin, angles, TT_AABB );
}

void trap_CM_CapsuleTrace( trace_t* results, const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs, clipHandle_t model, S32 brushmask )
{
    imports->collisionModelManager->BoxTrace( results, start, end, mins, maxs, model, brushmask, TT_CAPSULE );
}

void trap_CM_TransformedCapsuleTrace( trace_t* results, const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs, clipHandle_t model, S32 brushmask, const vec3_t origin, const vec3_t angles )
{
    imports->collisionModelManager->TransformedBoxTrace( results, start, end, mins, maxs, model, brushmask, origin, angles, TT_CAPSULE );
}

void trap_CM_BiSphereTrace( trace_t* results, const vec3_t start, const vec3_t end, F32 startRad, F32 endRad, clipHandle_t model, S32 mask )
{
    imports->collisionModelManager->BiSphereTrace( results, start, end, PASSFLOAT( startRad ), PASSFLOAT( endRad ), model, mask );
}

void trap_CM_TransformedBiSphereTrace( trace_t* results, const vec3_t start, const vec3_t end, F32 startRad, F32 endRad, clipHandle_t model, S32 mask, const vec3_t origin )
{
    imports->collisionModelManager->TransformedBiSphereTrace( results, start, end, PASSFLOAT( startRad ), PASSFLOAT( endRad ), model, mask, origin );
}

S32 trap_CM_MarkFragments( S32 numPoints, const vec3_t* points, const vec3_t projection, S32 maxPoints, vec3_t pointBuffer, S32 maxFragments, markFragment_t* fragmentBuffer )
{
    return imports->renderSystem->MarkFragments( numPoints, points, projection, maxPoints, pointBuffer, maxFragments, fragmentBuffer );
}

void trap_R_ProjectDecal( qhandle_t hShader, S32 numPoints, vec3_t* points, vec4_t projection, vec4_t color, S32 lifeTime, S32 fadeTime )
{
    //imports->renderSystem->ProjectDecal( hShader, numPoints, points, projection, color, lifeTime, fadeTime );
}

void trap_R_ClearDecals( void )
{
    //imports->renderSystem->ClearDecals();
}

void trap_S_StartSound( vec3_t origin, S32 entityNum, S32 entchannel, sfxHandle_t sfx )
{
    imports->soundSystem->StartSound( origin, entityNum, entchannel, sfx, 127 );
}

void trap_S_StartSoundVControl( vec3_t origin, S32 entityNum, S32 entchannel, sfxHandle_t sfx, S32 volume )
{
    imports->soundSystem->StartSound( origin, entityNum, entchannel, sfx, volume );
}

void trap_S_StartSoundEx( vec3_t origin, S32 entityNum, S32 entchannel, sfxHandle_t sfx, S32 flags )
{
    imports->soundSystem->StartSoundEx( origin, entityNum, entchannel, sfx, flags, 127 );
}

void trap_S_StartSoundExVControl( vec3_t origin, S32 entityNum, S32 entchannel, sfxHandle_t sfx, S32 flags, S32 volume )
{
    imports->soundSystem->StartSoundEx( origin, entityNum, entchannel, sfx, flags, volume );
}

void trap_S_StartLocalSound( sfxHandle_t sfx, S32 channelNum )
{
    imports->soundSystem->StartLocalSound( sfx, channelNum, 127 );
}

void trap_S_ClearLoopingSounds( void )
{
    imports->soundSystem->ClearLoopingSounds();
}

void trap_S_ClearSounds( bool killmusic, bool clearMusic )
{
    imports->soundSystem->ClearSounds( killmusic, clearMusic );
}

void trap_S_AddLoopingSound( const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx, S32 volume, S32 soundTime )
{
    imports->soundSystem->AddLoopingSound( origin, velocity, 1250, sfx, volume, soundTime );
}

void trap_S_AddRealLoopingSound( const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx, S32 range, S32 volume, S32 soundTime )
{
    imports->soundSystem->AddRealLoopingSound( origin, velocity, range, sfx, volume, soundTime );
}

void trap_S_StopStreamingSound( S32 entityNum )
{
    imports->soundSystem->StopStreamingSound( entityNum );
}

void trap_S_StopLoopingSound( S32 entityNum )
{
    imports->soundSystem->StopLoopingSound( entityNum );
}

void trap_S_UpdateEntityPosition( S32 entityNum, const vec3_t origin )
{
    imports->soundSystem->UpdateEntityPosition( entityNum, origin );
}

S32 trap_S_GetVoiceAmplitude( S32 entityNum )
{
    return imports->soundSystem->GetVoiceAmplitude( entityNum );
}

void trap_S_Respatialize( S32 entityNum, const vec3_t origin, vec3_t axis[3], S32 inwater )
{
    imports->soundSystem->Respatialize( entityNum, origin, axis, inwater );
}

S32 trap_S_GetSoundLength( sfxHandle_t sfx )
{
    return imports->soundSystem->GetSoundLength( sfx );
}

S32 trap_S_GetCurrentSoundTime( void )
{
    return imports->soundSystem->GetCurrentSoundTime();
}

void trap_S_StartBackgroundTrack( StringEntry intro, StringEntry loop, S32 fadeupTime )
{
    imports->soundSystem->StartBackgroundTrack( intro, loop, fadeupTime );
}

void trap_S_FadeBackgroundTrack( F32 targetvol, S32 time, S32 num )
{
    soundSystem->FadeStreamingSound( PASSFLOAT( targetvol ), time, num );
}

void trap_S_FadeAllSound( F32 targetvol, S32 time, bool stopsounds )
{
    imports->soundSystem->FadeAllSounds( PASSFLOAT( targetvol ), time, stopsounds );
}

S32 trap_S_StartStreamingSound( StringEntry intro, StringEntry loop, S32 entnum, S32 channel, S32 attenuation )
{
    return imports->soundSystem->StartStreamingSound( intro, loop, entnum, channel, attenuation );
}

void trap_R_LoadWorldMap( StringEntry mapname )
{
    //CG_DrawInformation(true);
    
    trap_PumpEventLoop();
    imports->renderSystem->LoadWorld( mapname );
}

qhandle_t trap_R_RegisterModel( StringEntry name )
{
    //CG_DrawInformation(true);
    
    trap_PumpEventLoop();
    return imports->renderSystem->RegisterModel( name );
}

qhandle_t trap_R_RegisterSkin( StringEntry name )
{
    //CG_DrawInformation(true);
    
    //trap_PumpEventLoop();
    return imports->renderSystem->RegisterSkin( name );
}

bool trap_R_GetSkinModel( qhandle_t skinid, StringEntry type, UTF8* name )
{
    return 0;// imports->renderSystem->GetSkinModel(skinid, type, name);
}

qhandle_t trap_R_GetShaderFromModel( qhandle_t modelid, S32 surfnum, S32 withlightmap )
{
    return 0;//imports->renderSystem->GetShaderFromModel( modelid, surfnum, withlightmap );
}

qhandle_t trap_R_RegisterShader( StringEntry name )
{
    //CG_DrawInformation(true);
    
    //trap_PumpEventLoop();
    return imports->renderSystem->RegisterShader( name );
}

void trap_R_RegisterFont( StringEntry fontName, S32 pointSize, fontInfo_t* font )
{
    //CG_DrawInformation(true);
    
    //trap_PumpEventLoop();
    imports->renderSystem->RegisterFont( fontName, pointSize, font );
}

qhandle_t trap_R_RegisterShaderNoMip( StringEntry name )
{
    //CG_DrawInformation(true);
    
    //trap_PumpEventLoop();
    return imports->renderSystem->RegisterShaderNoMip( name );
}

qhandle_t trap_R_RegisterShaderLightAttenuation( StringEntry name )
{
    return 0;//imports->renderSystem->RegisterShaderLightAttenuation( name );
}

void trap_R_ClearScene( void )
{
    imports->renderSystem->ClearScene();
}

void trap_R_AddRefEntityToScene( const refEntity_t* re )
{
    imports->renderSystem->AddRefEntityToScene( re );
}

void trap_R_AddPolyToScene( qhandle_t hShader, S32 numVerts, const polyVert_t* verts )
{
    imports->renderSystem->AddPolyToScene( hShader, numVerts, verts, 1 );
}

void trap_R_AddPolysToScene( qhandle_t hShader, S32 numVerts, const polyVert_t* verts, S32 numPolys )
{
    imports->renderSystem->AddPolyToScene( hShader, numVerts, verts, numPolys );
}

//void trap_R_AddPolyBufferToScene( polyBuffer_t* pPolyBuffer )
//{
//    imports->renderSystem->AddPolyBufferToScene( pPolyBuffer );
//}

void trap_R_AddLightToScene( const vec3_t org, F32 intensity, F32 r, F32 g, F32 b )
{
    imports->renderSystem->AddLightToScene( org, PASSFLOAT( intensity ), PASSFLOAT( r ), PASSFLOAT( g ), PASSFLOAT( b ) );
}

void trap_GS_FS_Seek( fileHandle_t f, S64 offset, fsOrigin_t origin )
{
    imports->FS_Seek( f, offset, origin );
}

void trap_R_AddCoronaToScene( const vec3_t org, F32 r, F32 g, F32 b, F32 scale, S32 id, bool visible )
{
    //imports->renderSystem->AddCoronaToScene( org, PASSFLOAT( r ), PASSFLOAT( g ), PASSFLOAT( b ), PASSFLOAT( scale ), id, visible );
}

void trap_R_SetFog( S32 fogvar, S32 var1, S32 var2, F32 r, F32 g, F32 b, F32 density )
{
    //imports->renderSystem->SetFog( fogvar, var1, var2, PASSFLOAT( r ), PASSFLOAT( g ), PASSFLOAT( b ), PASSFLOAT( density ) );
}

void trap_R_SetGlobalFog( bool restore, S32 duration, F32 r, F32 g, F32 b, F32 depthForOpaque )
{
    //imports->renderSystem->SetGlobalFog( restore, duration, PASSFLOAT( r ), PASSFLOAT( g ), PASSFLOAT( b ), PASSFLOAT( depthForOpaque ) );
}

void trap_R_RenderScene( const refdef_t* fd )
{
    imports->renderSystem->RenderScene( fd );
}

//void trap_R_SaveViewParms()
//{
//    imports->renderSystem->SaveViewParms();
//}

void trap_R_RestoreViewParms()
{
    //imports->renderSystem->RestoreViewParms();
}

void trap_R_SetColor( const F32* rgba )
{
    imports->renderSystem->SetColor( rgba );
}

void trap_R_SetClipRegion( const F32* region )
{
    imports->renderSystem->SetClipRegion( region );
}

void trap_R_DrawStretchPic( F32 x, F32 y, F32 w, F32 h, F32 s1, F32 t1, F32 s2, F32 t2, qhandle_t hShader )
{
    imports->renderSystem->DrawStretchPic( PASSFLOAT( x ), PASSFLOAT( y ), PASSFLOAT( w ), PASSFLOAT( h ), PASSFLOAT( s1 ), PASSFLOAT( t1 ), PASSFLOAT( s2 ), PASSFLOAT( t2 ), hShader );
}

void trap_R_DrawRotatedPic( F32 x, F32 y, F32 w, F32 h, F32 s1, F32 t1, F32 s2, F32 t2, qhandle_t hShader, F32 angle )
{
    //imports->renderSystem->DrawRotatedPic( PASSFLOAT( x ), PASSFLOAT( y ), PASSFLOAT( w ), PASSFLOAT( h ), PASSFLOAT( s1 ), PASSFLOAT( t1 ), PASSFLOAT( s2 ), PASSFLOAT( t2 ), hShader, PASSFLOAT( angle ) );
}

void trap_R_DrawStretchPicGradient( F32 x, F32 y, F32 w, F32 h, F32 s1, F32 t1, F32 s2, F32 t2, qhandle_t hShader, const F32* gradientColor, S32 gradientType )
{
    //imports->renderSystem->DrawStretchPicGradient( PASSFLOAT( x ), PASSFLOAT( y ), PASSFLOAT( w ), PASSFLOAT( h ), PASSFLOAT( s1 ), PASSFLOAT( t1 ), PASSFLOAT( s2 ), PASSFLOAT( t2 ), hShader, gradientColor, gradientType );
}

void trap_R_Add2dPolys( polyVert_t* verts, S32 numverts, qhandle_t hShader )
{
    //imports->renderSystem->Add2dPolys( verts, numverts, hShader );
}

void trap_R_ModelBounds( clipHandle_t model, vec3_t mins, vec3_t maxs )
{
    imports->renderSystem->ModelBounds( model, mins, maxs );
}

S32 trap_R_LerpTag( orientation_t* tag, clipHandle_t mod, S32 startFrame, S32 endFrame, F32 frac, StringEntry tagName )
{
    return imports->renderSystem->LerpTag( tag, mod, startFrame, endFrame, PASSFLOAT( frac ), tagName );
}

void trap_GetGlconfig( glconfig_t* glconfig )
{
    imports->GetGlconfig( glconfig );
}

void trap_GetGameState( gameState_t* gamestate )
{
    imports->GetGameState( gamestate );
}

void trap_GetCurrentSnapshotNumber( S32* snapshotNumber, S32* serverTime )
{
    imports->GetCurrentSnapshotNumber( snapshotNumber, serverTime );
}

bool trap_GetSnapshot( S32 snapshotNumber, snapshot_t* snapshot )
{
    return imports->GetSnapshot( snapshotNumber, snapshot );
}

bool trap_GetServerCommand( S32 serverCommandNumber )
{
    return imports->GetServerCommand( serverCommandNumber );
}

S32 trap_GetCurrentCmdNumber( void )
{
    return imports->GetCurrentCmdNumber();
}

bool trap_GetUserCmd( S32 cmdNumber, usercmd_t* ucmd )
{
    return imports->GetUserCmd( cmdNumber, ucmd );
}

void trap_SetUserCmdValue( S32 stateValue, S32 flags, F32 sensitivityScale, S32 mpIdentClient )
{
    imports->SetUserCmdValue( stateValue, flags, PASSFLOAT( sensitivityScale ), mpIdentClient );
}

void trap_SetClientLerpOrigin( F32 x, F32 y, F32 z )
{
    imports->SetClientLerpOrigin( PASSFLOAT( x ), PASSFLOAT( y ), PASSFLOAT( z ) );
}

S32 trap_MemoryRemaining( void )
{
    return imports->Hunk_MemoryRemaining();
}

bool trap_Key_IsDown( S32 keynum )
{
    return imports->Key_IsDown( keynum );
}

S32 trap_Key_GetCatcher( void )
{
    return imports->Key_GetCatcher();
}

void trap_Key_SetCatcher( S32 catcher )
{
    imports->Key_SetCatcher( catcher );
}

S32 trap_Key_GetKey( StringEntry binding )
{
    return imports->Key_GetKey( binding );
}

bool trap_Key_GetOverstrikeMode( void )
{
    return imports->Key_GetOverstrikeMode();
}

void trap_Key_SetOverstrikeMode( bool state )
{
    imports->Key_SetOverstrikeMode( state );
}

S32 trap_PC_AddGlobalDefine( UTF8* define )
{
    return imports->PC_AddGlobalDefine( define );
}

S32 trap_PC_LoadSource( StringEntry filename )
{
    return imports->PC_LoadSource( filename );
}

S32 trap_PC_FreeSource( S32 handle )
{
    return imports->PC_FreeSource( handle );
}

S32 trap_PC_ReadToken( S32 handle, pc_token_t* pc_token )
{
    return imports->PC_ReadToken( handle, pc_token );
}

S32 trap_PC_SourceFileAndLine( S32 handle, UTF8* filename, S32* line )
{
    return imports->PC_SourceFileAndLine( handle, filename, line );
}

void trap_PC_UnReadToken( S32 handle )
{
    return imports->PC_UnreadLastToken( handle );
}

void trap_S_StopBackgroundTrack( void )
{
    imports->soundSystem->StopBackgroundTrack();
}

S32 trap_RealTime( qtime_t* qtime )
{
    return imports->RealTime( qtime );
}

void trap_SnapVector( F32* v )
{
    imports->Sys_SnapVector( v );
}

S32 trap_CIN_PlayCinematic( StringEntry arg0, S32 xpos, S32 ypos, S32 width, S32 height, S32 bits )
{
    return imports->CIN_PlayCinematic( arg0, xpos, ypos, width, height, bits );
}

e_status trap_CIN_StopCinematic( S32 handle )
{
    return imports->CIN_StopCinematic( handle );
}

e_status trap_CIN_RunCinematic( S32 handle )
{
    return imports->CIN_RunCinematic( handle );
}

void trap_CIN_DrawCinematic( S32 handle )
{
    imports->CIN_DrawCinematic( handle );
}

void trap_CIN_SetExtents( S32 handle, S32 x, S32 y, S32 w, S32 h )
{
    imports->CIN_SetExtents( handle, x, y, w, h );
}

void trap_R_RemapShader( StringEntry oldShader, StringEntry newShader, StringEntry timeOffset )
{
    imports->renderSystem->RemapShader( oldShader, newShader, timeOffset );
}

bool trap_loadCamera( S32 camNum, StringEntry name )
{
    return imports->loadCamera( camNum, name );
}

void trap_startCamera( S32 camNum, S32 time )
{
    imports->startCamera( camNum, time );
}

void trap_stopCamera( S32 camNum )
{
    imports->stopCamera( camNum );
}

bool trap_getCameraInfo( S32 camNum, S32 time, vec3_t* origin, vec3_t* angles, F32* fov )
{
    return imports->getCameraInfo( camNum, time, origin, angles, fov );
}

bool trap_GetEntityToken( UTF8* buffer, S32 bufferSize )
{
    return imports->renderSystem->GetEntityToken( buffer, bufferSize );
}

void trap_UI_Popup( StringEntry arg0 )
{
    imports->UI_Popup( arg0 );
}

void trap_UI_ClosePopup( StringEntry arg0 )
{
    imports->UI_ClosePopup( arg0 );
}

void trap_Key_GetBindingBuf( S32 keynum, UTF8* buf, S32 buflen )
{
    imports->Key_GetBindingBuf( keynum, buf, buflen );
}

void trap_Key_SetBinding( S32 keynum, StringEntry binding )
{
    imports->Key_SetBinding( keynum, binding );
}

void trap_Key_KeynumToStringBuf( S32 keynum, UTF8* buf, S32 buflen )
{
    imports->Key_KeynumToStringBuf( keynum, buf, buflen );
}

void trap_Key_KeysForBinding( StringEntry binding, S32* key1, S32* key2 )
{
    imports->Key_GetBindingByString( binding, key1, key2 );
}

void trap_CG_TranslateString( StringEntry string, UTF8* buf )
{
    imports->CL_TranslateString( string, buf );
}

bool trap_R_inPVS( const vec3_t p1, const vec3_t p2 )
{
    return imports->renderSystem->inPVS( p1, p2 );
}

void trap_GetHunkData( S32* hunkused, S32* hunkexpected )
{
    imports->Com_GetHunkInfo( hunkused, hunkexpected );
}

void trap_PumpEventLoop( void )
{
    /*if(!cgs.initing) {
    	return;
    }
    syscall(CG_PUMPEVENTLOOP);*/
}

bool trap_R_LoadDynamicShader( StringEntry shadername, StringEntry shadertext )
{
    return 0;// imports->renderSystem->LoadDynamicShader(shadername, shadertext);
}

void trap_R_RenderToTexture( S32 textureid, S32 x, S32 y, S32 w, S32 h )
{
    //imports->renderSystem->RenderToTexture( textureid, x, y, w, h );
}

S32 trap_R_GetTextureId( StringEntry name )
{
    return 0;// imports->renderSystem->GetTextureId(name);
}

void trap_R_Finish( void )
{
    //imports->renderSystem->Finish();
}

void trap_GetDemoName( UTF8* buffer, S32 size )
{
    imports->CL_DemoName( buffer, size );
}

S32 trap_R_LightForPoint( vec3_t point, vec3_t ambientLight, vec3_t directedLight, vec3_t lightDir )
{
    return imports->renderSystem->LightForPoint( point, ambientLight, directedLight, lightDir );
}

#if 0 //defined(USE_REFENTITY_ANIMATIONSYSTEM)
qhandle_t trap_R_RegisterAnimation( StringEntry name )
{
    return imports->renderSystem->RegisterAnimation( name );
}

S32 trap_R_CheckSkeleton( refSkeleton_t* skel, qhandle_t hModel, qhandle_t hAnim )
{
    return imports->renderSystem->CheckSkeleton( skel, hModel, hAnim );
}

S32 trap_R_BuildSkeleton( refSkeleton_t* skel, qhandle_t anim, S32 startFrame, S32 endFrame, F32 frac, bool clearOrigin )
{
    return;// imports->renderSystem->BuildSkeleton( skel, anim, startFrame, endFrame, PASSFLOAT( frac ), clearOrigin );
}

S32 trap_R_BlendSkeleton( refSkeleton_t* skel, const refSkeleton_t* blend, F32 frac )
{
    return;// imports->renderSystem->BlendSkeleton( skel, blend, PASSFLOAT( frac ) );
}

S32 trap_R_BoneIndex( qhandle_t hModel, StringEntry boneName )
{
    return;// imports->renderSystem->BoneIndex( hModel, boneName );
}

S32 trap_R_AnimNumFrames( qhandle_t hAnim )
{
    return;// imports->renderSystem->AnimNumFrames( hAnim );
}

S32 trap_R_AnimFrameRate( qhandle_t hAnim )
{
    return;//imports->renderSystem->AnimFrameRate( hAnim );
}
#endif

void trap_TranslateString( StringEntry string, UTF8* buf )
{
    imports->CL_TranslateString( string, buf );
}

sfxHandle_t trap_S_RegisterSound( StringEntry sample, bool compressed )
{
    return soundSystem->RegisterSound( sample, compressed );
}

void trap_PhysicsDrawDebug( void )
{
    imports->PhysicsDrawDebug();
}

//void trap_R_AddRefLightToScene( const refLight_t* light )
//{
//    imports->renderSystem->AddRefLightToScene( light );
//}

S32 trap_S_SoundDuration( sfxHandle_t handle )
{
    return imports->soundSystem->SoundDuration( handle );
}
