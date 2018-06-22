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
// File name:   cg_api.h
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#ifndef __CG_API_H__
#define __CG_API_H__

#ifndef __Q_SHARED_H__
#include <qcommon/q_shared.h>
#endif
#ifndef __R_TYPES_H__
#include <GPURenderer/r_types.h>
#endif
#ifndef __BG_PUBLIC_H__
#include <bgame/bg_public.h>
#endif

#define CGAME_IMPORT_API_VERSION 5
#define CAM_PRIMARY 0

#define CMD_BACKUP 64
#define CMD_MASK ( CMD_BACKUP - 1 )
// allow a lot of command backups for very fast systems
// multiple commands may be combined into a single packet, so this
// needs to be larger than PACKET_BACKUP

#define MAX_ENTITIES_IN_SNAPSHOT    512

// snapshots are a view of the server at a given time

// Snapshots are generated at regular time intervals by the server,
// but they may not be sent if a client's rate level is exceeded, or
// they may be dropped by the network.
typedef struct
{
    S32 snapFlags;	// SNAPFLAG_RATE_DELAYED, etc
    S32 ping;
    
    S32 serverTime;	// server time the message is valid for (in msec)
    
    U8 areamask[MAX_MAP_AREA_BYTES];	// portalarea visibility bits
    
    playerState_t ps;			// complete information about the current player at this time
    
    S32 numEntities;	// all of the entities that need to be presented
    entityState_t entities[MAX_ENTITIES_IN_SNAPSHOT];	// at the time of this snapshot
    
    S32 numServerCommands;	// text based server commands to execute when this
    S32 serverCommandSequence;	// snapshot becomes current
} snapshot_t;

typedef enum cgameEvent_e
{
    CGAME_EVENT_NONE,
    CGAME_EVENT_GAMEVIEW,
    CGAME_EVENT_SPEAKEREDITOR,
    CGAME_EVENT_CAMPAIGNBREIFING,
    CGAME_EVENT_DEMO,
    CGAME_EVENT_FIRETEAMMSG,
    CGAME_EVENT_MULTIVIEW
} cgameEvent_t;

struct cgameImports_t
{
    void( *Print )( StringEntry fmt, ... );
    void( *Error )( S32 level, StringEntry fmt, ... );
    S32( *Milliseconds )( void );
    void( *Cvar_Register )( vmCvar_t* vmCvar, StringEntry varName, StringEntry defaultValue, S32 flags );
    void( *Cvar_Update )( vmCvar_t* vmCvar );
    void( *Cvar_Set )( StringEntry var_name, StringEntry value );
    void( *Cvar_VariableStringBuffer )( StringEntry var_name, UTF8* buffer, S32 bufsize );
    S32( *Argc )( void );
    void( *Argv )( S32 n, UTF8* buffer, S32 bufferLength );
    void( *Args )( UTF8* buffer, S32 bufferLength );
    S32( *FS_FOpenFile )( StringEntry qpath, fileHandle_t* f, fsMode_t mode );
    S32( *FS_Read )( void* buffer, S32 len, fileHandle_t f );
    S32( *FS_Write )( const void* buffer, S32 len, fileHandle_t f );
    void( *FS_FCloseFile )( fileHandle_t f );
    void( *SendConsoleCommand )( StringEntry text );
    void( *AddCommand )( StringEntry cmdName );
    void( *RemoveCommand )( StringEntry cmdName );
    void( *SendClientCommand )( StringEntry s );
    void( *UpdateScreen )( void );
    void( *GetGlconfig )( glconfig_t* glconfig );
    void( *SetClientLerpOrigin )( F32 x, F32 y, F32 z );
    void( *GetGameState )( gameState_t* gamestate );
    void( *GetCurrentSnapshotNumber )( S32* snapshotNumber, S32* serverTime );
    bool( *GetSnapshot )( S32 snapshotNumber, snapshot_t* snapshot );
    bool( *GetServerCommand )( S32 serverCommandNumber );
    S32( *GetCurrentCmdNumber )( void );
    bool( *GetUserCmd )( S32 cmdNumber, usercmd_t* ucmd );
    void( *SetUserCmdValue )( S32 stateValue, S32 holdableValue, F32 sensitivityScale, S32 cld );
    S32( *MemoryRemaining )( void );
    bool( *loadCamera )( S32 camNum, StringEntry name );
    void( *startCamera )( S32 camNum, S32 time );
    void( *stopCamera )( S32 camNum );
    bool( *getCameraInfo )( S32 camNum, S32 time, vec3_t* origin, vec3_t* angles, F32* fov );
    void ( *LoadMap )( StringEntry mapname );
    bool( *Key_IsDown )( S32 keynum );
    S32( *Key_GetCatcher )( void );
    void( *Key_SetCatcher )( S32 catcher );
    S32( *Key_GetKey )( StringEntry binding );
    S32( *PC_AddGlobalDefine )( UTF8* define );
    S32( *PC_LoadSource )( StringEntry filename );
    S32( *PC_FreeSource )( S32 handle );
    S32( *PC_ReadToken )( S32 handle, pc_token_t* pc_token );
    S32( *PC_SourceFileAndLine )( S32 handle, UTF8* filename, S32* line );
    void ( *PC_UnreadLastToken )( S32 handle );
    S32( *RealTime )( qtime_t* qtime );
    
    // this returns a handle.  arg0 is the name in the format "idlogo.roq", set arg1 to NULL, alteredstates to qfalse (do not alter gamestate)
    S32( *CIN_PlayCinematic )( StringEntry arg0, S32 xpos, S32 ypos, S32 width, S32 height, S32 bits );
    
    // stops playing the cinematic and ends it.  should always return FMV_EOF
    // cinematics must be stopped in reverse order of when they are started
    e_status( *CIN_StopCinematic )( S32 handle );
    
    // will run a frame of the cinematic but will not draw it.  Will return FMV_EOF if the end of the cinematic has been reached.
    e_status( *CIN_RunCinematic )( S32 handle );
    
    // draws the current frame
    void( *CIN_DrawCinematic )( S32 handle );
    
    // allows you to resize the animation dynamically
    void( *CIN_SetExtents )( S32 handle, S32 x, S32 y, S32 w, S32 h );
    void( *UI_Popup )( StringEntry arg0 );
    void( *UI_LimboChat )( StringEntry arg0 );
    void( *UI_ClosePopup )( StringEntry arg0 );
    void( *Sys_SnapVector )( F32* v );
    void ( *Cmd_RemoveCommand )( StringEntry cmd_name );
    void ( *AddReliableCommand )( StringEntry cmd );
    void ( *Cvar_LatchedVariableStringBuffer )( StringEntry var_name, UTF8* buffer, S32 bufsize );
    S32( *FS_Delete )( UTF8* filename );
    S32( *FS_GetFileList )( StringEntry path, StringEntry extension, UTF8* listbuf, S32 bufsize );
    void ( *CL_TranslateString )( StringEntry string, UTF8* dest_buffer );
    void ( *CL_DemoName )( UTF8* buffer, S32 size );
    void ( *Com_GetHunkInfo )( S32* hunkused, S32* hunkexpected );
    S32( *DemoPos )( void );
    void ( *DemoName )( UTF8* buffer, S32 size );
    demoState_t ( *DemoState )( void );
    S32( *FS_Seek )( fileHandle_t f, S64 offset, S32 origin );
    S32( *Hunk_MemoryRemaining )( void );
    void ( *Key_GetBindingBuf )( S32 keynum, UTF8* buf, S32 buflen );
    void ( *Key_KeynumToStringBuf )( S32 keynum, UTF8* buf, S32 buflen );
    void ( *Key_GetBindingByString )( StringEntry binding, S32* key1, S32* key2 );
    void ( *Key_SetBinding )( S32 keynum, StringEntry binding );
    void ( *LiteralArgsBuffer )( UTF8* buffer, S32 bufferLength );
    bool ( *Key_GetOverstrikeMode )( void );
    void ( *Key_SetOverstrikeMode )( bool state );
    S32( *FS_FOpenFileByMode )( StringEntry qpath, fileHandle_t* f, fsMode_t mode );
    void( *PhysicsDrawDebug )( void );
    S32( *CompleteCallback )( StringEntry complete );
    
    idRenderSystem* renderSystem;
    idCollisionModelManager* collisionModelManager;
    idSoundSystem* soundSystem;
};

class idCGame
{
public:
    virtual void Init( S32 serverMessageNum, S32 serverCommandSequence, S32 clientNum, bool demoPlayback ) = 0;
    // called when the level loads or when the renderer is restarted
    // all media should be registered at this time
    // cgame will display loading status by calling SCR_Update, which
    // will call CG_DrawInformation during the loading process
    // reliableCommandSequence will be 0 on fresh loads, but higher for
    // demos, tourney restarts, or vid_restarts
    
    virtual void Shutdown() = 0;
    // oportunity to flush and close any open files
    
    virtual bool ConsoleCommand() = 0;
    // a console command has been issued locally that is not recognized by the
    // main game system.
    // use Cmd_Argc() / Cmd_Argv() to read the command, return false if the
    // command is not known to the game
    
    virtual void DrawActiveFrame( S32 serverTime, stereoFrame_t stereoView, bool demoPlayback ) = 0;
    // Generates and draws a game scene and status information at the given time.
    // If demoPlayback is set, local movement prediction will not be enabled
    
    virtual void ConsoleText( void ) = 0;
    //	pass text that has been printed to the console to cgame
    //	use Cmd_Argc() / Cmd_Argv() to read it
    
    virtual S32 CrosshairPlayer( void ) = 0;
    
    virtual S32 LastAttacker( void ) = 0;
    
    virtual void KeyEvent( S32 key, bool down ) = 0;
    
    virtual void MouseEvent( S32 dx, S32 dy ) = 0;
    
    virtual void EventHandling( S32 type, bool fForced ) = 0;
    
    virtual bool GetTag( S32 clientNum, UTF8* tagname, orientation_t* or ) = 0;
    
    virtual bool CheckExecKey( S32 key ) = 0;
    
    virtual bool WantsBindKeys( void ) = 0;
    
    virtual void CompleteCommand( S32 argNum ) = 0;
    // will callback on all availible completions
    // use Cmd_Argc() / Cmd_Argv() to read the command
};

extern idCGame* cgame;

void trap_Print( StringEntry fmt );
void trap_Error( StringEntry fmt );
S32 trap_Milliseconds( void );
void trap_Cvar_Register( vmCvar_t* vmCvar, StringEntry varName, StringEntry defaultValue, S32 flags );
void trap_Cvar_Update( vmCvar_t* vmCvar );
void trap_Cvar_Set( StringEntry var_name, StringEntry value );
void trap_Cvar_VariableStringBuffer( StringEntry var_name, UTF8* buffer, S32 bufsize );
void trap_Cvar_LatchedVariableStringBuffer( StringEntry var_name, UTF8* buffer, S32 bufsize );
S32 trap_Argc( void );
void trap_Argv( S32 n, UTF8* buffer, S32 bufferLength );
void trap_Args( UTF8* buffer, S32 bufferLength );
void trap_LiteralArgs( UTF8* buffer, S32 bufferLength );
S32 trap_GetDemoState( void );
S32 trap_GetDemoPos( void );
S32 trap_FS_FOpenFile( StringEntry qpath, fileHandle_t* f, fsMode_t mode );
void trap_FS_Read( void* buffer, S32 len, fileHandle_t f );
S32 trap_FS_Write( const void* buffer, S32 len, fileHandle_t f );
void trap_FS_FCloseFile( fileHandle_t f );
S32 trap_FS_GetFileList( StringEntry path, StringEntry extension, UTF8* listbuf, S32 bufsize );
S32 trap_FS_Delete( UTF8* filename );
void trap_SendConsoleCommand( StringEntry text );
void trap_AddCommand( StringEntry cmdName );
void trap_RemoveCommand( StringEntry cmdName );
void trap_SendClientCommand( StringEntry s );
void trap_UpdateScreen( void );
void trap_CM_LoadMap( StringEntry mapname );
S32 trap_CM_NumInlineModels( void );
clipHandle_t trap_CM_InlineModel( S32 index );
clipHandle_t trap_CM_TempBoxModel( const vec3_t mins, const vec3_t maxs );
clipHandle_t trap_CM_TempCapsuleModel( const vec3_t mins, const vec3_t maxs );
S32 trap_CM_PointContents( const vec3_t p, clipHandle_t model );
S32 trap_CM_TransformedPointContents( const vec3_t p, clipHandle_t model, const vec3_t origin, const vec3_t angles );
void trap_CM_BoxTrace( trace_t* results, const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs, clipHandle_t model, S32 brushmask );
void trap_CM_TransformedBoxTrace( trace_t* results, const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs, clipHandle_t model, S32 brushmask, const vec3_t origin, const vec3_t angles );
void trap_CM_CapsuleTrace( trace_t* results, const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs, clipHandle_t model, S32 brushmask );
void trap_CM_TransformedCapsuleTrace( trace_t* results, const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs, clipHandle_t model, S32 brushmask, const vec3_t origin, const vec3_t angles );
void trap_CM_BiSphereTrace( trace_t* results, const vec3_t start, const vec3_t end, F32 startRad, F32 endRad, clipHandle_t model, S32 mask );
void trap_CM_TransformedBiSphereTrace( trace_t* results, const vec3_t start, const vec3_t end, F32 startRad, F32 endRad, clipHandle_t model, S32 mask, const vec3_t origin );
S32 trap_CM_MarkFragments( S32 numPoints, const vec3_t* points, const vec3_t projection, S32 maxPoints, vec3_t pointBuffer, S32 maxFragments, markFragment_t* fragmentBuffer );
void trap_R_ProjectDecal( qhandle_t hShader, S32 numPoints, vec3_t* points, vec4_t projection, vec4_t color, S32 lifeTime, S32 fadeTime );
void trap_R_ClearDecals( void );
void trap_S_StartSound( vec3_t origin, S32 entityNum, S32 entchannel, sfxHandle_t sfx );
void trap_S_StartSoundVControl( vec3_t origin, S32 entityNum, S32 entchannel, sfxHandle_t sfx, S32 volume );
void trap_S_StartSoundEx( vec3_t origin, S32 entityNum, S32 entchannel, sfxHandle_t sfx, S32 flags );
void trap_S_StartSoundExVControl( vec3_t origin, S32 entityNum, S32 entchannel, sfxHandle_t sfx, S32 flags, S32 volume );
void trap_S_StartLocalSound( sfxHandle_t sfx, S32 channelNum );
void trap_S_ClearLoopingSounds( void );
void trap_S_ClearSounds( bool killmusic, bool clearMusic );
void trap_S_AddLoopingSound( const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx, S32 volume, S32 soundTime );
void trap_S_AddRealLoopingSound( const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx, S32 range, S32 volume, S32 soundTime );
void trap_S_StopStreamingSound( S32 entityNum );
void trap_S_StopLoopingSound( S32 entityNum );
void trap_S_UpdateEntityPosition( S32 entityNum, const vec3_t origin );
S32 trap_S_GetVoiceAmplitude( S32 entityNum );
void trap_S_Respatialize( S32 entityNum, const vec3_t origin, vec3_t axis[3], S32 inwater );
S32 trap_S_GetSoundLength( sfxHandle_t sfx );
S32 trap_S_GetCurrentSoundTime( void );
void trap_S_StartBackgroundTrack( StringEntry intro, StringEntry loop, S32 fadeupTime );
void trap_S_FadeBackgroundTrack( F32 targetvol, S32 time, S32 num );
void trap_S_FadeAllSound( F32 targetvol, S32 time, bool stopsounds );
S32 trap_S_StartStreamingSound( StringEntry intro, StringEntry loop, S32 entnum, S32 channel, S32 attenuation );
void trap_R_LoadWorldMap( StringEntry mapname );
qhandle_t trap_R_RegisterModel( StringEntry name );
qhandle_t trap_R_RegisterSkin( StringEntry name );
bool trap_R_GetSkinModel( qhandle_t skinid, StringEntry type, UTF8* name );
qhandle_t trap_R_GetShaderFromModel( qhandle_t modelid, S32 surfnum, S32 withlightmap );
qhandle_t trap_R_RegisterShader( StringEntry name );
void trap_R_RegisterFont( StringEntry fontName, S32 pointSize, fontInfo_t* font );
qhandle_t trap_R_RegisterShaderNoMip( StringEntry name );
qhandle_t trap_R_RegisterShaderLightAttenuation( StringEntry name );
void trap_R_ClearScene( void );
void trap_R_AddRefEntityToScene( const refEntity_t* re );
void trap_R_AddPolyToScene( qhandle_t hShader, S32 numVerts, const polyVert_t* verts );
void trap_R_AddPolysToScene( qhandle_t hShader, S32 numVerts, const polyVert_t* verts, S32 numPolys );
void trap_R_AddLightToScene( const vec3_t org, F32 intensity, F32 r, F32 g, F32 b );
void trap_GS_FS_Seek( fileHandle_t f, S64 offset, fsOrigin_t origin );
void trap_R_AddCoronaToScene( const vec3_t org, F32 r, F32 g, F32 b, F32 scale, S32 id, bool visible );
void trap_R_SetFog( S32 fogvar, S32 var1, S32 var2, F32 r, F32 g, F32 b, F32 density );
void trap_R_SetGlobalFog( bool restore, S32 duration, F32 r, F32 g, F32 b, F32 depthForOpaque );
void trap_R_RenderScene( const refdef_t* fd );
void trap_R_RestoreViewParms();
void trap_R_SetColor( const F32* rgba );
void trap_R_SetClipRegion( const F32* region );
void trap_R_DrawStretchPic( F32 x, F32 y, F32 w, F32 h, F32 s1, F32 t1, F32 s2, F32 t2, qhandle_t hShader );
void trap_R_DrawRotatedPic( F32 x, F32 y, F32 w, F32 h, F32 s1, F32 t1, F32 s2, F32 t2, qhandle_t hShader, F32 angle );
void trap_R_DrawStretchPicGradient( F32 x, F32 y, F32 w, F32 h, F32 s1, F32 t1, F32 s2, F32 t2, qhandle_t hShader, const F32* gradientColor, S32 gradientType );
void trap_R_Add2dPolys( polyVert_t* verts, S32 numverts, qhandle_t hShader );
void trap_R_ModelBounds( clipHandle_t model, vec3_t mins, vec3_t maxs );
S32 trap_R_LerpTag( orientation_t* tag, clipHandle_t mod, S32 startFrame, S32 endFrame, F32 frac, StringEntry tagName );
void trap_GetGlconfig( glconfig_t* glconfig );
void trap_GetGameState( gameState_t* gamestate );
void trap_GetCurrentSnapshotNumber( S32* snapshotNumber, S32* serverTime );
bool trap_GetSnapshot( S32 snapshotNumber, snapshot_t* snapshot );
bool trap_GetServerCommand( S32 serverCommandNumber );
S32 trap_GetCurrentCmdNumber( void );
bool trap_GetUserCmd( S32 cmdNumber, usercmd_t* ucmd );
void trap_SetUserCmdValue( S32 stateValue, S32 flags, F32 sensitivityScale, S32 mpIdentClient );
void trap_SetClientLerpOrigin( F32 x, F32 y, F32 z );
S32 trap_MemoryRemaining( void );
bool trap_Key_IsDown( S32 keynum );
S32 trap_Key_GetCatcher( void );
void trap_Key_SetCatcher( S32 catcher );
S32 trap_Key_GetKey( StringEntry binding );
bool trap_Key_GetOverstrikeMode( void );
void trap_Key_SetOverstrikeMode( bool state );
S32 trap_PC_AddGlobalDefine( UTF8* define );
S32 trap_PC_LoadSource( StringEntry filename );
S32 trap_PC_FreeSource( S32 handle );
S32 trap_PC_ReadToken( S32 handle, pc_token_t* pc_token );
S32 trap_PC_SourceFileAndLine( S32 handle, UTF8* filename, S32* line );
void trap_PC_UnReadToken( S32 handle );
void trap_S_StopBackgroundTrack( void );
S32 trap_RealTime( qtime_t* qtime );
void trap_SnapVector( F32* v );
S32 trap_CIN_PlayCinematic( StringEntry arg0, S32 xpos, S32 ypos, S32 width, S32 height, S32 bits );
e_status trap_CIN_StopCinematic( S32 handle );
e_status trap_CIN_RunCinematic( S32 handle );
void trap_CIN_DrawCinematic( S32 handle );
void trap_CIN_SetExtents( S32 handle, S32 x, S32 y, S32 w, S32 h );
void trap_R_RemapShader( StringEntry oldShader, StringEntry newShader, StringEntry timeOffset );
bool trap_loadCamera( S32 camNum, StringEntry name );
void trap_startCamera( S32 camNum, S32 time );
void trap_stopCamera( S32 camNum );
bool trap_getCameraInfo( S32 camNum, S32 time, vec3_t* origin, vec3_t* angles, F32* fov );
bool trap_GetEntityToken( UTF8* buffer, S32 bufferSize );
void trap_UI_Popup( StringEntry arg0 );
void trap_UI_ClosePopup( StringEntry arg0 );
void trap_Key_GetBindingBuf( S32 keynum, UTF8* buf, S32 buflen );
void trap_Key_SetBinding( S32 keynum, StringEntry binding );
void trap_Key_KeynumToStringBuf( S32 keynum, UTF8* buf, S32 buflen );
void trap_Key_KeysForBinding( StringEntry binding, S32* key1, S32* key2 );
void trap_CG_TranslateString( StringEntry string, UTF8* buf );
bool trap_R_inPVS( const vec3_t p1, const vec3_t p2 );
void trap_GetHunkData( S32* hunkused, S32* hunkexpected );
void trap_PumpEventLoop( void );
bool trap_R_LoadDynamicShader( StringEntry shadername, StringEntry shadertext );
void trap_R_RenderToTexture( S32 textureid, S32 x, S32 y, S32 w, S32 h );
S32 trap_R_GetTextureId( StringEntry name );
void trap_R_Finish( void );
void trap_GetDemoName( UTF8* buffer, S32 size );
S32 trap_R_LightForPoint( vec3_t point, vec3_t ambientLight, vec3_t directedLight, vec3_t lightDir );
qhandle_t trap_R_RegisterAnimation( StringEntry name );
void trap_TranslateString( StringEntry string, UTF8* buf );
sfxHandle_t trap_S_RegisterSound( StringEntry sample, bool compressed );
void trap_PhysicsDrawDebug( void );
qhandle_t trap_R_LoadAnim( qhandle_t modelIndex, StringEntry name );
S32 trap_S_SoundDuration( sfxHandle_t handle );

#endif //!__CG_API_H__
