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
// File name:   bg_local.h
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:  local definitions for the bg (both games) files
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#ifndef __BG_LOCAL_H__
#define __BG_LOCAL_H__

#ifndef __BG_PUBLIC_H__
#include <bgame/bg_public.h>
#endif

#define MIN_WALK_NORMAL 0.7f   // can't walk on very steep slopes

#define STEPSIZE    18

#define TIMER_LAND        130
#define TIMER_GESTURE     (34*66+50)
#define TIMER_ATTACK      500 //nonsegmented models

#define OVERCLIP    1.001f

#define FALLING_THRESHOLD -900.0f //what vertical speed to start falling sound at

// all of the locals will be zeroed before each
// pmove, just to make damn sure we don't have
// any differences when running on client or server
typedef struct
{
    vec3_t forward, right, up;
    F32 frametime;
    S32 msec;
    bool walking;
    bool groundPlane;
    bool ladder;
    trace_t groundTrace;
    F32 impactSpeed;
    vec3_t previous_origin;
    vec3_t previous_velocity;
    S32 previous_waterlevel;
} pml_t;

extern pmove_t* pm;
extern pml_t pml;

// movement parameters
extern F32 pm_stopspeed;
extern F32 pm_duckScale;
extern F32 pm_swimScale;
extern F32 pm_wadeScale;

extern F32 pm_accelerate;
extern F32 pm_airaccelerate;
extern F32 pm_wateraccelerate;
extern F32 pm_flyaccelerate;

extern F32 pm_friction;
extern F32 pm_waterfriction;
extern F32 pm_flightfriction;

extern S32 c_pmove;

class idCGameLocal;

//
// idGame
//
class idBothGamesLocal : public bgGame
{
public:
    virtual const buildableAttributes_t* BuildableByName( StringEntry name );
    virtual const buildableAttributes_t* BuildableByEntityName( StringEntry name );
    virtual const buildableAttributes_t* Buildable( buildable_t buildable );
    virtual bool BuildableAllowedInStage( buildable_t buildable, stage_t stage );
    virtual buildableConfig_t* BuildableConfig( buildable_t buildable );
    virtual void BuildableBoundingBox( buildable_t buildable, vec3_t mins, vec3_t maxs );
    virtual bool ParseBuildableFile( StringEntry filename, buildableConfig_t* bc );
    virtual void InitBuildableConfigs( void );
    virtual const classAttributes_t* ClassByName( StringEntry name );
    virtual const classAttributes_t* Class( class_t _class );
    virtual bool ClassAllowedInStage( class_t _class, stage_t stage );
    virtual classConfig_t* ClassConfig( class_t _class );
    virtual void ClassBoundingBox( class_t _class, vec3_t mins, vec3_t maxs, vec3_t cmaxs, vec3_t dmins, vec3_t dmaxs );
    virtual bool ClassHasAbility( class_t _class, S32 ability );
    virtual S32 ClassCanEvolveFromTo( class_t fclass, class_t tclass, S32 credits, S32 stage, S32 cost );
    virtual bool AlienCanEvolve( class_t pclass, S32 credits, S32 stage );
    virtual bool ParseClassFile( StringEntry filename, classConfig_t* cc );
    virtual void InitClassConfigs( void );
    virtual const weaponAttributes_t* WeaponByName( StringEntry name );
    virtual const weaponAttributes_t* Weapon( weapon_t weapon );
    virtual bool WeaponAllowedInStage( weapon_t weapon, stage_t stage );
    virtual const upgradeAttributes_t* UpgradeByName( StringEntry name );
    virtual const upgradeAttributes_t* Upgrade( upgrade_t upgrade );
    virtual bool UpgradeAllowedInStage( upgrade_t upgrade, stage_t stage );
    virtual void EvaluateTrajectory( const trajectory_t* tr, S32 atTime, vec3_t result );
    virtual void EvaluateTrajectoryDelta( const trajectory_t* tr, S32 atTime, vec3_t result );
    virtual StringEntry EventName( S32 num );
    virtual void AddPredictableEventToPlayerstate( S32 newEvent, S32 eventParm, playerState_t* ps );
    virtual void PlayerStateToEntityState( playerState_t* ps, entityState_t* s, bool snap );
    virtual void PlayerStateToEntityStateExtraPolate( playerState_t* ps, entityState_t* s, S32 time, bool snap );
    virtual bool WeaponIsFull( weapon_t weapon, S32 stats[], S32 ammo, S32 clips );
    virtual bool InventoryContainsWeapon( S32 weapon, S32 stats[] );
    virtual S32 CalculateSlotsForInventory( S32 stats[] );
    virtual void AddUpgradeToInventory( S32 item, S32 stats[] );
    virtual void RemoveUpgradeFromInventory( S32 item, S32 stats[] );
    virtual bool InventoryContainsUpgrade( S32 item, S32 stats[] );
    virtual void ActivateUpgrade( S32 item, S32 stats[] );
    virtual void DeactivateUpgrade( S32 item, S32 stats[] );
    virtual bool UpgradeIsActive( S32 item, S32 stats[] );
    virtual bool RotateAxis( vec3_t surfNormal, vec3_t inAxis[3], vec3_t outAxis[3], bool inverse, bool ceiling );
    virtual void GetClientNormal( const playerState_t* ps, vec3_t normal );
    virtual void PositionBuildableRelativeToPlayer( const playerState_t* ps, const vec3_t mins, const vec3_t maxs, void( *trace )( trace_t*, const vec3_t, const vec3_t, const vec3_t, const vec3_t, S32, S32 ), vec3_t outOrigin, vec3_t outAngles, trace_t* tr );
    virtual S32 GetValueOfPlayer( playerState_t* ps );
    virtual S32 PlayerPoisonCloudTime( playerState_t* ps );
    virtual weapon_t GetPlayerWeapon( playerState_t* ps );
    virtual bool PlayerCanChangeWeapon( playerState_t* ps );
    virtual F32 atof_neg( UTF8* token, bool allowNegative );
    virtual S32 atoi_neg( UTF8* token, bool allowNegative );
    virtual void ParseCSVEquipmentList( StringEntry string, weapon_t* weapons, S32 weaponsSize, upgrade_t* upgrades, S32 upgradesSize );
    virtual void ParseCSVClassList( StringEntry string, class_t* classes, S32 classesSize );
    virtual void ParseCSVBuildableList( StringEntry string, buildable_t* buildables, S32 buildablesSize );
    virtual void InitAllowedGameElements( void );
    virtual bool WeaponIsAllowed( weapon_t weapon );
    virtual bool UpgradeIsAllowed( upgrade_t upgrade );
    virtual bool ClassIsAllowed( class_t _class );
    virtual bool BuildableIsAllowed( buildable_t buildable );
    virtual bool ClientListTest( clientList_t* list, S32 clientNum );
    virtual void ClientListAdd( clientList_t* list, S32 clientNum );
    virtual void ClientListRemove( clientList_t* list, S32 clientNum );
    virtual UTF8* ClientListString( clientList_t* list );
    virtual void ClientListParse( clientList_t* list, StringEntry s );
    virtual weapon_t PrimaryWeapon( S32 stats[] );
    virtual S32 LoadEmoticons( UTF8 names[][MAX_EMOTICON_NAME_LEN], S32 widths[] );
    virtual UTF8* TeamName( team_t team );
    virtual bool SlideMove( bool gravity );
    virtual void StepEvent( vec3_t from, vec3_t to, vec3_t normal );
    virtual bool StepSlideMove( bool gravity, bool predictive );
    virtual bool PredictStepMove( void );
    virtual void AddEvent( S32 newEvent );
    virtual void AddTouchEnt( S32 entityNum );
    virtual void StartTorsoAnim( S32 anim );
    virtual void StartLegsAnim( S32 anim );
    virtual void ContinueLegsAnim( S32 anim );
    virtual void ContinueTorsoAnim( S32 anim );
    virtual void ForceLegsAnim( S32 anim );
    virtual void ClipVelocity( vec3_t in, vec3_t normal, vec3_t out, F32 overbounce );
    virtual void Friction( void );
    virtual void Accelerate( vec3_t wishdir, F32 wishspeed, F32 accel );
    virtual F32 CmdScale( usercmd_t* cmd );
    virtual void SetMovementDir( void );
    virtual void CheckCharge( void );
    virtual bool CheckPounce( void );
    virtual bool CheckWallJump( void );
    virtual bool CheckJump( void );
    virtual bool CheckWaterJump( void );
    virtual bool CheckDodge( void );
    virtual void WaterJumpMove( void );
    virtual void WaterMove( void );
    virtual void JetPackMove( void );
    virtual void FlyMove( void );
    virtual void AirMove( void );
    virtual void ClimbMove( void );
    virtual void WalkMove( void );
    virtual void LadderMove( void );
    virtual void CheckLadder( void );
    virtual void DeadMove( void );
    virtual void NoclipMove( void );
    virtual S32 FootstepForSurface( void );
    virtual void CrashLand( void );
    virtual S32 CorrectAllSolid( trace_t* trace );
    virtual void GroundTraceMissed( void );
    virtual void GroundClimbTrace( void );
    virtual void GroundTrace( void );
    virtual void SetWaterLevel( void );
    virtual void SetViewheight( void );
    virtual void CheckDuck( void );
    virtual void Footsteps( void );
    virtual void WaterEvents( void );
    virtual void BeginWeaponChange( S32 weapon );
    virtual void FinishWeaponChange( void );
    virtual void TorsoAnimation( void );
    virtual void Weapon( void );
    virtual void Animate( void );
    virtual void DropTimers( void );
    virtual void UpdateViewAngles( playerState_t* ps, const usercmd_t* cmd );
    virtual void PmoveSingle( pmove_t* pmove );
    virtual void Pmove( pmove_t* pmove );
    virtual void VoiceParseError( fileHandle_t handle, UTF8* err );
    virtual voice_t* VoiceList( void );
    virtual bool VoiceParseTrack( S32 handle, voiceTrack_t* voiceTrack );
    virtual voiceTrack_t* VoiceParseCommand( S32 handle );
    virtual voiceCmd_t* VoiceParse( UTF8* name );
    virtual voice_t* VoiceInit( void );
    virtual void PrintVoices( voice_t* voices, S32 debugLevel );
    virtual voice_t* VoiceByName( voice_t* head, UTF8* name );
    virtual voiceCmd_t* VoiceCmdFind( voiceCmd_t* head, UTF8* name, S32* cmdNum );
    virtual voiceCmd_t* VoiceCmdByNum( voiceCmd_t* head, S32 num );
    virtual voiceTrack_t* VoiceTrackByNum( voiceTrack_t* head, S32 num );
    virtual voiceTrack_t* VoiceTrackFind( voiceTrack_t* head, team_t team, class_t _class, weapon_t weapon, S32 enthusiasm, S32* trackNum );
    virtual void* Alloc( S32 size );
    virtual void Free( void* ptr );
    virtual void InitMemory( void );
    virtual void DefragmentMemory( void );
    
};

extern idBothGamesLocal bgLocal;

#endif // !__BG_LOCAL_H__