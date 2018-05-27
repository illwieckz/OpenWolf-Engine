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
// File name:   botlib.h
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#ifndef __BOTLIB_H__
#define __BOTLIB_H__

#define	BOTLIB_API_VERSION		2

struct aas_clientmove_s;
struct aas_entityinfo_s;
struct aas_areainfo_s;
struct aas_altroutegoal_s;
struct aas_predictroute_s;
struct bot_consolemessage_s;
struct bot_match_s;
struct bot_goal_s;
struct bot_moveresult_s;
struct bot_initmove_s;
struct weaponinfo_s;

#define BOTFILESBASEFOLDER		"botfiles"
//debug line colors
#define LINECOLOR_NONE			-1
#define LINECOLOR_RED			1//0xf2f2f0f0L
#define LINECOLOR_GREEN			2//0xd0d1d2d3L
#define LINECOLOR_BLUE			3//0xf3f3f1f1L
#define LINECOLOR_YELLOW		4//0xdcdddedfL
#define LINECOLOR_ORANGE		5//0xe0e1e2e3L

//Print types
#define PRT_MESSAGE				1
#define PRT_WARNING				2
#define PRT_ERROR				3
#define PRT_FATAL				4
#define PRT_EXIT				5

//console message types
#define CMS_NORMAL				0
#define CMS_CHAT				1

//botlib error codes
#define BLERR_NOERROR					0	//no error
#define BLERR_LIBRARYNOTSETUP			1	//library not setup
#define BLERR_INVALIDENTITYNUMBER		2	//invalid entity number
#define BLERR_NOAASFILE					3	//no AAS file available
#define BLERR_CANNOTOPENAASFILE			4	//cannot open AAS file
#define BLERR_WRONGAASFILEID			5	//incorrect AAS file id
#define BLERR_WRONGAASFILEVERSION		6	//incorrect AAS file version
#define BLERR_CANNOTREADAASLUMP			7	//cannot read AAS file lump
#define BLERR_CANNOTLOADICHAT			8	//cannot load initial chats
#define BLERR_CANNOTLOADITEMWEIGHTS		9	//cannot load item weights
#define BLERR_CANNOTLOADITEMCONFIG		10	//cannot load item config
#define BLERR_CANNOTLOADWEAPONWEIGHTS	11	//cannot load weapon weights
#define BLERR_CANNOTLOADWEAPONCONFIG	12	//cannot load weapon config

//action flags
#define ACTION_ATTACK			0x0000001
#define ACTION_USE				0x0000002
#define ACTION_RESPAWN			0x0000008
#define ACTION_JUMP				0x0000010
#define ACTION_MOVEUP			0x0000020
#define ACTION_CROUCH			0x0000080
#define ACTION_MOVEDOWN			0x0000100
#define ACTION_MOVEFORWARD		0x0000200
#define ACTION_MOVEBACK			0x0000800
#define ACTION_MOVELEFT			0x0001000
#define ACTION_MOVERIGHT		0x0002000
#define ACTION_DELAYEDJUMP		0x0008000
#define ACTION_TALK				0x0010000
#define ACTION_GESTURE			0x0020000
#define ACTION_WALK				0x0080000
#define ACTION_AFFIRMATIVE		0x0100000
#define ACTION_NEGATIVE			0x0200000
#define ACTION_GETFLAG			0x0800000
#define ACTION_GUARDBASE		0x1000000
#define ACTION_PATROL			0x2000000
#define ACTION_FOLLOWME			0x8000000

//the bot input, will be converted to an usercmd_t
typedef struct bot_input_s
{
    F32 thinktime;		//time since last output (in seconds)
    vec3_t dir;				//movement direction
    F32 speed;			//speed in the range [0, 400]
    vec3_t viewangles;		//the view angles
    S32 actionflags;		//one of the ACTION_? flags
    S32 weapon;				//weapon to use
} bot_input_t;

#ifndef BSPTRACE

#define BSPTRACE

//bsp_trace_t hit surface
typedef struct bsp_surface_s
{
    UTF8 name[16];
    S32 flags;
    S32 value;
} bsp_surface_t;

//remove the bsp_trace_s structure definition l8r on
//a trace is returned when a box is swept through the world
typedef struct bsp_trace_s
{
    bool		allsolid;	// if true, plane is not valid
    bool		startsolid;	// if true, the initial point was in a solid area
    F32			fraction;	// time completed, 1.0 = didn't hit anything
    vec3_t			endpos;		// final position
    cplane_t		plane;		// surface normal at impact
    F32			exp_dist;	// expanded plane distance
    S32				sidenum;	// number of the brush side hit
    bsp_surface_t	surface;	// the hit point surface
    S32				contents;	// contents on other side of surface hit
    S32				ent;		// number of entity hit
} bsp_trace_t;

#endif	// BSPTRACE

//entity state
typedef struct bot_entitystate_s
{
    S32		type;			// entity type
    S32		flags;			// entity flags
    vec3_t	origin;			// origin of the entity
    vec3_t	angles;			// angles of the model
    vec3_t	old_origin;		// for lerping
    vec3_t	mins;			// bounding box minimums
    vec3_t	maxs;			// bounding box maximums
    S32		groundent;		// ground entity
    S32		solid;			// solid type
    S32		modelindex;		// model used
    S32		modelindex2;	// weapons, CTF flags, etc
    S32		frame;			// model frame number
    S32		event;			// impulse events -- muzzle flashes, footsteps, etc
    S32		eventParm;		// even parameter
    S32		powerups;		// bit flags
    S32		weapon;			// determines weapon and flash model, etc
    S32		legsAnim;		// mask off ANIM_TOGGLEBIT
    S32		torsoAnim;		// mask off ANIM_TOGGLEBIT
} bot_entitystate_t;

//bot AI library exported functions
typedef struct botlib_import_s
{
    //print messages from the bot library
    void	( * Print )( S32 type, UTF8* fmt, ... );
    //trace a bbox through the world
    void	( *Trace )( bsp_trace_t* trace, vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, S32 passent, S32 contentmask );
    //trace a bbox against a specific entity
    void	( *EntityTrace )( bsp_trace_t* trace, vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, S32 entnum, S32 contentmask );
    //retrieve the contents at the given point
    S32( *PointContents )( vec3_t point );
    //check if the point is in potential visible sight
    S32( *inPVS )( vec3_t p1, vec3_t p2 );
    //retrieve the BSP entity data lump
    UTF8*		( *BSPEntityData )( void );
    //
    void	( *BSPModelMinsMaxsOrigin )( S32 modelnum, vec3_t angles, vec3_t mins, vec3_t maxs, vec3_t origin );
    //send a bot client command
    void	( *BotClientCommand )( S32 client, UTF8* command );
    //memory allocation
    void*		( *GetMemory )( S32 size );		// allocate from Zone
    void	( *FreeMemory )( void* ptr );		// free memory from Zone
    S32( *AvailableMemory )( void );		// available Zone memory
    void*		( *HunkAlloc )( S32 size );		// allocate from hunk
    //file system access
    S32( *FS_FOpenFile )( StringEntry qpath, fileHandle_t* file, fsMode_t mode );
    S32( *FS_Read )( void* buffer, S32 len, fileHandle_t f );
    S32( *FS_Write )( const void* buffer, S32 len, fileHandle_t f );
    void	( *FS_FCloseFile )( fileHandle_t f );
    S32( *FS_Seek )( fileHandle_t f, S64 offset, S32 origin );
    //debug visualisation stuff
    S32( *DebugLineCreate )( void );
    void	( *DebugLineDelete )( S32 line );
    void	( *DebugLineShow )( S32 line, vec3_t start, vec3_t end, S32 color );
    //
    S32( *DebugPolygonCreate )( S32 color, S32 numPoints, vec3_t* points );
    void	( *DebugPolygonDelete )( S32 id );
    // Ridah, Cast AI stuff
    bool( *BotVisibleFromPos )( vec3_t srcpos, S32 srcnum, vec3_t destpos, S32 destnum, bool updateVisPos );
    bool( *BotCheckAttackAtPos )( S32 entnum, S32 enemy, vec3_t pos, bool ducking, bool allowHitWorld );
} botlib_import_t;

typedef struct aas_export_s
{
    //-----------------------------------
    // be_aas_entity.h
    //-----------------------------------
    void	( *AAS_EntityInfo )( S32 entnum, struct aas_entityinfo_s* info );
    //-----------------------------------
    // be_aas_main.h
    //-----------------------------------
    S32( *AAS_Initialized )( void );
    void	( *AAS_PresenceTypeBoundingBox )( S32 presencetype, vec3_t mins, vec3_t maxs );
    F32( *AAS_Time )( void );
    //--------------------------------------------
    // be_aas_sample.c
    //--------------------------------------------
    S32( *AAS_PointAreaNum )( vec3_t point );
    S32( *AAS_PointReachabilityAreaIndex )( vec3_t point );
    S32( *AAS_TraceAreas )( vec3_t start, vec3_t end, S32* areas, vec3_t* points, S32 maxareas );
    S32( *AAS_BBoxAreas )( vec3_t absmins, vec3_t absmaxs, S32* areas, S32 maxareas );
    S32( *AAS_AreaInfo )( S32 areanum, struct aas_areainfo_s* info );
    //--------------------------------------------
    // be_aas_bspq3.c
    //--------------------------------------------
    S32( *AAS_PointContents )( vec3_t point );
    S32( *AAS_NextBSPEntity )( S32 ent );
    S32( *AAS_ValueForBSPEpairKey )( S32 ent, UTF8* key, UTF8* value, S32 size );
    S32( *AAS_VectorForBSPEpairKey )( S32 ent, UTF8* key, vec3_t v );
    S32( *AAS_FloatForBSPEpairKey )( S32 ent, UTF8* key, F32* value );
    S32( *AAS_IntForBSPEpairKey )( S32 ent, UTF8* key, S32* value );
    //--------------------------------------------
    // be_aas_reach.c
    //--------------------------------------------
    S32( *AAS_AreaReachability )( S32 areanum );
    //--------------------------------------------
    // be_aas_route.c
    //--------------------------------------------
    S32( *AAS_AreaTravelTimeToGoalArea )( S32 areanum, vec3_t origin, S32 goalareanum, S32 travelflags );
    S32( *AAS_EnableRoutingArea )( S32 areanum, S32 enable );
    S32( *AAS_PredictRoute )( struct aas_predictroute_s* route, S32 areanum, vec3_t origin,
                              S32 goalareanum, S32 travelflags, S32 maxareas, S32 maxtime,
                              S32 stopevent, S32 stopcontents, S32 stoptfl, S32 stopareanum );
    //--------------------------------------------
    // be_aas_altroute.c
    //--------------------------------------------
    S32( *AAS_AlternativeRouteGoals )( vec3_t start, S32 startareanum, vec3_t goal, S32 goalareanum, S32 travelflags,
                                       struct aas_altroutegoal_s* altroutegoals, S32 maxaltroutegoals,
                                       S32 type );
    //--------------------------------------------
    // be_aas_move.c
    //--------------------------------------------
    S32( *AAS_Swimming )( vec3_t origin );
    S32( *AAS_PredictClientMovement )( struct aas_clientmove_s* move,
                                       S32 entnum, vec3_t origin,
                                       S32 presencetype, S32 onground,
                                       vec3_t velocity, vec3_t cmdmove,
                                       S32 cmdframes,
                                       S32 maxframes, F32 frametime,
                                       S32 stopevent, S32 stopareanum, S32 visualize );
} aas_export_t;

typedef struct ea_export_s
{
    //ClientCommand elementary actions
    void	( *EA_Command )( S32 client, UTF8* command );
    void	( *EA_Say )( S32 client, UTF8* str );
    void	( *EA_SayTeam )( S32 client, UTF8* str );
    //
    void	( *EA_Action )( S32 client, S32 action );
    void	( *EA_Gesture )( S32 client );
    void	( *EA_Talk )( S32 client );
    void	( *EA_Attack )( S32 client );
    void	( *EA_Use )( S32 client );
    void	( *EA_Respawn )( S32 client );
    void	( *EA_MoveUp )( S32 client );
    void	( *EA_MoveDown )( S32 client );
    void	( *EA_MoveForward )( S32 client );
    void	( *EA_MoveBack )( S32 client );
    void	( *EA_MoveLeft )( S32 client );
    void	( *EA_MoveRight )( S32 client );
    void	( *EA_Crouch )( S32 client );
    
    void	( *EA_SelectWeapon )( S32 client, S32 weapon );
    void	( *EA_Jump )( S32 client );
    void	( *EA_DelayedJump )( S32 client );
    void	( *EA_Move )( S32 client, vec3_t dir, F32 speed );
    void	( *EA_View )( S32 client, vec3_t viewangles );
    //send regular input to the server
    void	( *EA_EndRegular )( S32 client, F32 thinktime );
    void	( *EA_GetInput )( S32 client, F32 thinktime, bot_input_t* input );
    void	( *EA_ResetInput )( S32 client );
} ea_export_t;

typedef struct ai_export_s
{
    //-----------------------------------
    // be_ai_char.h
    //-----------------------------------
    S32( *BotLoadCharacter )( UTF8* charfile, F32 skill );
    void	( *BotFreeCharacter )( S32 character );
    F32( *Characteristic_Float )( S32 character, S32 index );
    F32( *Characteristic_BFloat )( S32 character, S32 index, F32 min, F32 max );
    S32( *Characteristic_Integer )( S32 character, S32 index );
    S32( *Characteristic_BInteger )( S32 character, S32 index, S32 min, S32 max );
    void	( *Characteristic_String )( S32 character, S32 index, UTF8* buf, S32 size );
    //-----------------------------------
    // be_ai_chat.h
    //-----------------------------------
    S32( *BotAllocChatState )( void );
    void	( *BotFreeChatState )( S32 handle );
    void	( *BotQueueConsoleMessage )( S32 chatstate, S32 type, UTF8* message );
    void	( *BotRemoveConsoleMessage )( S32 chatstate, S32 handle );
    S32( *BotNextConsoleMessage )( S32 chatstate, struct bot_consolemessage_s* cm );
    S32( *BotNumConsoleMessages )( S32 chatstate );
    void	( *BotInitialChat )( S32 chatstate, UTF8* type, S32 mcontext, UTF8* var0, UTF8* var1, UTF8* var2, UTF8* var3, UTF8* var4, UTF8* var5, UTF8* var6, UTF8* var7 );
    S32( *BotNumInitialChats )( S32 chatstate, UTF8* type );
    S32( *BotReplyChat )( S32 chatstate, UTF8* message, S32 mcontext, S32 vcontext, UTF8* var0, UTF8* var1, UTF8* var2, UTF8* var3, UTF8* var4, UTF8* var5, UTF8* var6, UTF8* var7 );
    S32( *BotChatLength )( S32 chatstate );
    void	( *BotEnterChat )( S32 chatstate, S32 client, S32 sendto );
    void	( *BotGetChatMessage )( S32 chatstate, UTF8* buf, S32 size );
    S32( *StringContains )( UTF8* str1, UTF8* str2, S32 casesensitive );
    S32( *BotFindMatch )( UTF8* str, struct bot_match_s* match, U64 context );
    void	( *BotMatchVariable )( struct bot_match_s* match, S32 variable, UTF8* buf, S32 size );
    void	( *UnifyWhiteSpaces )( UTF8* string );
    void	( *BotReplaceSynonyms )( UTF8* string, U64 context );
    S32( *BotLoadChatFile )( S32 chatstate, UTF8* chatfile, UTF8* chatname );
    void	( *BotSetChatGender )( S32 chatstate, S32 gender );
    void	( *BotSetChatName )( S32 chatstate, UTF8* name, S32 client );
    //-----------------------------------
    // be_ai_goal.h
    //-----------------------------------
    void	( *BotResetGoalState )( S32 goalstate );
    void	( *BotResetAvoidGoals )( S32 goalstate );
    void	( *BotRemoveFromAvoidGoals )( S32 goalstate, S32 number );
    void	( *BotPushGoal )( S32 goalstate, struct bot_goal_s* goal );
    void	( *BotPopGoal )( S32 goalstate );
    void	( *BotEmptyGoalStack )( S32 goalstate );
    void	( *BotDumpAvoidGoals )( S32 goalstate );
    void	( *BotDumpGoalStack )( S32 goalstate );
    void	( *BotGoalName )( S32 number, UTF8* name, S32 size );
    S32( *BotGetTopGoal )( S32 goalstate, struct bot_goal_s* goal );
    S32( *BotGetSecondGoal )( S32 goalstate, struct bot_goal_s* goal );
    S32( *BotChooseLTGItem )( S32 goalstate, vec3_t origin, S32* inventory, S32 travelflags );
    S32( *BotChooseNBGItem )( S32 goalstate, vec3_t origin, S32* inventory, S32 travelflags,
                              struct bot_goal_s* ltg, F32 maxtime );
    S32( *BotTouchingGoal )( vec3_t origin, struct bot_goal_s* goal );
    S32( *BotItemGoalInVisButNotVisible )( S32 viewer, vec3_t eye, vec3_t viewangles, struct bot_goal_s* goal );
    S32( *BotGetLevelItemGoal )( S32 index, UTF8* classname, struct bot_goal_s* goal );
    S32( *BotGetNextCampSpotGoal )( S32 num, struct bot_goal_s* goal );
    S32( *BotGetMapLocationGoal )( UTF8* name, struct bot_goal_s* goal );
    F32( *BotAvoidGoalTime )( S32 goalstate, S32 number );
    void	( *BotSetAvoidGoalTime )( S32 goalstate, S32 number, F32 avoidtime );
    void	( *BotInitLevelItems )( void );
    void	( *BotUpdateEntityItems )( void );
    S32( *BotLoadItemWeights )( S32 goalstate, UTF8* filename );
    void	( *BotFreeItemWeights )( S32 goalstate );
    void	( *BotInterbreedGoalFuzzyLogic )( S32 parent1, S32 parent2, S32 child );
    void	( *BotSaveGoalFuzzyLogic )( S32 goalstate, UTF8* filename );
    void	( *BotMutateGoalFuzzyLogic )( S32 goalstate, F32 range );
    S32( *BotAllocGoalState )( S32 client );
    void	( *BotFreeGoalState )( S32 handle );
    //-----------------------------------
    // be_ai_move.h
    //-----------------------------------
    void	( *BotResetMoveState )( S32 movestate );
    void	( *BotMoveToGoal )( struct bot_moveresult_s* result, S32 movestate, struct bot_goal_s* goal, S32 travelflags );
    S32( *BotMoveInDirection )( S32 movestate, vec3_t dir, F32 speed, S32 type );
    void	( *BotResetAvoidReach )( S32 movestate );
    void	( *BotResetLastAvoidReach )( S32 movestate );
    S32( *BotReachabilityArea )( vec3_t origin, S32 testground );
    S32( *BotMovementViewTarget )( S32 movestate, struct bot_goal_s* goal, S32 travelflags, F32 lookahead, vec3_t target );
    S32( *BotPredictVisiblePosition )( vec3_t origin, S32 areanum, struct bot_goal_s* goal, S32 travelflags, vec3_t target );
    S32( *BotAllocMoveState )( void );
    void	( *BotFreeMoveState )( S32 handle );
    void	( *BotInitMoveState )( S32 handle, struct bot_initmove_s* initmove );
    void	( *BotAddAvoidSpot )( S32 movestate, vec3_t origin, F32 radius, S32 type );
    //-----------------------------------
    // be_ai_weap.h
    //-----------------------------------
    S32( *BotChooseBestFightWeapon )( S32 weaponstate, S32* inventory );
    void	( *BotGetWeaponInfo )( S32 weaponstate, S32 weapon, struct weaponinfo_s* weaponinfo );
    S32( *BotLoadWeaponWeights )( S32 weaponstate, UTF8* filename );
    S32( *BotAllocWeaponState )( void );
    void	( *BotFreeWeaponState )( S32 weaponstate );
    void	( *BotResetWeaponState )( S32 weaponstate );
    //-----------------------------------
    // be_ai_gen.h
    //-----------------------------------
    S32( *GeneticParentsAndChildSelection )( S32 numranks, F32* ranks, S32* parent1, S32* parent2, S32* child );
} ai_export_t;

//bot AI library imported functions
typedef struct botlib_export_s
{
    //Area Awareness System functions
    aas_export_t aas;
    //Elementary Action functions
    ea_export_t ea;
    //AI functions
    ai_export_t ai;
    //setup the bot library, returns BLERR_
    S32( *BotLibSetup )( void );
    //shutdown the bot library, returns BLERR_
    S32( *BotLibShutdown )( void );
    //sets a library variable returns BLERR_
    S32( *BotLibVarSet )( UTF8* var_name, UTF8* value );
    //gets a library variable returns BLERR_
    S32( *BotLibVarGet )( UTF8* var_name, UTF8* value, S32 size );
    
    //sets a C-like define returns BLERR_
    S32( *PC_AddGlobalDefine )( UTF8* string );
    S32( *PC_LoadSourceHandle )( StringEntry filename );
    S32( *PC_FreeSourceHandle )( S32 handle );
    S32( *PC_ReadTokenHandle )( S32 handle, pc_token_t* pc_token );
    S32( *PC_SourceFileAndLine )( S32 handle, UTF8* filename, S32* line );
    
    //start a frame in the bot library
    S32( *BotLibStartFrame )( F32 time );
    //load a new map in the bot library
    S32( *BotLibLoadMap )( StringEntry mapname );
    //entity updates
    S32( *BotLibUpdateEntity )( S32 ent, bot_entitystate_t* state );
    //just for testing
    S32( *Test )( S32 parm0, UTF8* parm1, vec3_t parm2, vec3_t parm3 );
} botlib_export_t;

//linking of bot library
botlib_export_t* GetBotLibAPI( S32 apiVersion, botlib_import_t* import );

/* Library variables:

name:						default:			module(s):			description:

"basedir"					""					l_utils.c			base directory
"gamedir"					""					l_utils.c			game directory
"cddir"						""					l_utils.c			CD directory

"log"						"0"					l_log.c				enable/disable creating a log file
"maxclients"				"4"					be_interface.c		maximum number of clients
"maxentities"				"1024"				be_interface.c		maximum number of entities
"bot_developer"				"0"					be_interface.c		bot developer mode

"phys_friction"				"6"					be_aas_move.c		ground friction
"phys_stopspeed"			"100"				be_aas_move.c		stop speed
"phys_gravity"				"800"				be_aas_move.c		gravity value
"phys_waterfriction"		"1"					be_aas_move.c		water friction
"phys_watergravity"			"400"				be_aas_move.c		gravity in water
"phys_maxvelocity"			"320"				be_aas_move.c		maximum velocity
"phys_maxwalkvelocity"		"320"				be_aas_move.c		maximum walk velocity
"phys_maxcrouchvelocity"	"100"				be_aas_move.c		maximum crouch velocity
"phys_maxswimvelocity"		"150"				be_aas_move.c		maximum swim velocity
"phys_walkaccelerate"		"10"				be_aas_move.c		walk acceleration
"phys_airaccelerate"		"1"					be_aas_move.c		air acceleration
"phys_swimaccelerate"		"4"					be_aas_move.c		swim acceleration
"phys_maxstep"				"18"				be_aas_move.c		maximum step height
"phys_maxsteepness"			"0.7"				be_aas_move.c		maximum floor steepness
"phys_maxbarrier"			"32"				be_aas_move.c		maximum barrier height
"phys_maxwaterjump"			"19"				be_aas_move.c		maximum waterjump height
"phys_jumpvel"				"270"				be_aas_move.c		jump z velocity
"phys_falldelta5"			"40"				be_aas_move.c
"phys_falldelta10"			"60"				be_aas_move.c
"rs_waterjump"				"400"				be_aas_move.c
"rs_teleport"				"50"				be_aas_move.c
"rs_barrierjump"			"100"				be_aas_move.c
"rs_startcrouch"			"300"				be_aas_move.c
"rs_startgrapple"			"500"				be_aas_move.c
"rs_startwalkoffledge"		"70"				be_aas_move.c
"rs_startjump"				"300"				be_aas_move.c
"rs_rocketjump"				"500"				be_aas_move.c
"rs_bfgjump"				"500"				be_aas_move.c
"rs_jumppad"				"250"				be_aas_move.c
"rs_aircontrolledjumppad"	"300"				be_aas_move.c
"rs_funcbob"				"300"				be_aas_move.c
"rs_startelevator"			"50"				be_aas_move.c
"rs_falldamage5"			"300"				be_aas_move.c
"rs_falldamage10"			"500"				be_aas_move.c
"rs_maxjumpfallheight"		"450"				be_aas_move.c

"max_aaslinks"				"4096"				be_aas_sample.c		maximum links in the AAS
"max_routingcache"			"4096"				be_aas_route.c		maximum routing cache size in KB
"forceclustering"			"0"					be_aas_main.c		force recalculation of clusters
"forcereachability"			"0"					be_aas_main.c		force recalculation of reachabilities
"forcewrite"				"0"					be_aas_main.c		force writing of aas file
"aasoptimize"				"0"					be_aas_main.c		enable aas optimization
"sv_mapChecksum"			"0"					be_aas_main.c		BSP file checksum
"bot_visualizejumppads"		"0"					be_aas_reach.c		visualize jump pads

"bot_reloadcharacters"		"0"					-					reload bot character files
"ai_gametype"				"0"					be_ai_goal.c		game type
"droppedweight"				"1000"				be_ai_goal.c		additional dropped item weight
"weapindex_rocketlauncher"	"5"					be_ai_move.c		rl weapon index for rocket jumping
"weapindex_bfg10k"			"9"					be_ai_move.c		bfg weapon index for bfg jumping
"weapindex_grapple"			"10"				be_ai_move.c		grapple weapon index for grappling
"entitytypemissile"			"3"					be_ai_move.c		ET_MISSILE
"offhandgrapple"			"0"					be_ai_move.c		enable off hand grapple hook
"cmd_grappleon"				"grappleon"			be_ai_move.c		command to activate off hand grapple
"cmd_grappleoff"			"grappleoff"		be_ai_move.c		command to deactivate off hand grapple
"itemconfig"				"items.c"			be_ai_goal.c		item configuration file
"weaponconfig"				"weapons.c"			be_ai_weap.c		weapon configuration file
"synfile"					"syn.c"				be_ai_chat.c		file with synonyms
"rndfile"					"rnd.c"				be_ai_chat.c		file with random strings
"matchfile"					"match.c"			be_ai_chat.c		file with match strings
"nochat"					"0"					be_ai_chat.c		disable chats
"max_messages"				"1024"				be_ai_chat.c		console message heap size
"max_weaponinfo"			"32"				be_ai_weap.c		maximum number of weapon info
"max_projectileinfo"		"32"				be_ai_weap.c		maximum number of projectile info
"max_iteminfo"				"256"				be_ai_goal.c		maximum number of item info
"max_levelitems"			"256"				be_ai_goal.c		maximum number of level items

*/

//some functions from g_bot.c

#endif //!__BOTLIB_H__
