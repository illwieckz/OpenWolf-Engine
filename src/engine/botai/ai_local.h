////////////////////////////////////////////////////////////////////////////
// Copyright(C) 1999 - 2005 Id Software, Inc.
// Copyright(C) 2000 - 2006 Tim Angus
// Copyright(C) 2018 Dusan Jocic <dusanjocic@msn.com>
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
// -------------------------------------------------------------------------
// File name:   ai_local.h
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////

#ifndef __AI_MAIN_H__
#define __AI_MAIN_H__

#ifndef __BOTLIB_H__
#include <botlib/botlib.h>
#endif
#ifndef __BE_AAS_H__
#include <botlib/be_aas.h>
#endif
#ifndef __BE_AI_GOAL_H__
#include <botlib/be_ai_goal.h>
#endif
#ifndef __BE_AI_MOVE_H__
#include <botlib/be_ai_move.h>
#endif
#ifndef __BE_AI_WEAP_H__
#include <botlib/be_ai_weap.h>
#endif
#ifndef __BE_AI_CHAT_H__
#include <botlib/be_ai_chat.h>
#endif
#ifndef __CHARS_H__
#include <game/chars.h>
#endif

#define MAX_ITEMS 256
#define MAX_STATESWITCHES 5

extern F32 floattime;
#define FloatTime() floattime

//bot flags
#define BFL_STRAFERIGHT				1	//strafe to the right
#define BFL_ATTACKED				2	//bot has attacked last ai frame
#define BFL_ATTACKJUMPED			4	//bot jumped during attack last frame
#define BFL_AIMATENEMY				8	//bot aimed at the enemy this frame
#define BFL_AVOIDRIGHT				16	//avoid obstacles by going to the right
#define BFL_IDEALVIEWSET			32	//bot has ideal view angles set
#define BFL_FIGHTSUICIDAL			64	//bot is in a suicidal fight

#define IDEAL_ATTACKDIST			140

//copied from the aas file header
#define PRESENCE_NONE				1
#define PRESENCE_NORMAL				2
#define PRESENCE_CROUCH				4

//long term goal types
#define LTG_TEAMHELP             1  //help a team mate
#define LTG_TEAMACCOMPANY        2  //accompany a team mate
#define LTG_DEFENDKEYAREA        3  //defend a key area
#define LTG_GETFLAG              4  //get the enemy flag
#define LTG_RUSHBASE             5  //rush to the base
#define LTG_RETURNFLAG           6  //return the flag
#define LTG_CAMP                 7  //camp somewhere
#define LTG_CAMPORDER            8  //ordered to camp somewhere
#define LTG_PATROL               9  //patrol
#define LTG_GETITEM             10  //get an item
#define LTG_KILL                11  //kill someone
#define LTG_HARVEST             12  //harvest skulls
#define LTG_ATTACKENEMYBASE     13  //attack the enemy base
#define LTG_MAKELOVE_UNDER      14
#define LTG_MAKELOVE_ONTOP      15

//patrol flags
#define PATROL_LOOP         1
#define PATROL_REVERSE      2
#define PATROL_BACK         4

//goal flag, see ../botlib/be_ai_goal.h for the other GFL_*
#define GFL_AIR             128

// Neural network
#define MAX_NODESWITCHES    50
#define MAX_WAYPOINTS       128

//some maxs
#define MAX_FILEPATH        144

//bot settings
typedef struct bot_settings_s
{
    UTF8 characterfile[MAX_FILEPATH];
    F32 skill;
    UTF8 team[MAX_FILEPATH];
} bot_settings_t;

//check points
typedef struct bot_waypoint_s
{
    S32 inuse;
    UTF8 name[32];
    bot_goal_t goal;
    struct bot_waypoint_s* next, *prev;
} bot_waypoint_t;

// print types
typedef enum
{
    BPMSG,
    BPERROR,
    BPDEBUG
} botprint_t;

// inventory
typedef enum
{
    BI_HEALTH,
    BI_CREDITS,		// evos for alien bots
    BI_WEAPON,
    BI_AMMO,
    BI_CLIPS,
    BI_STAMINA,		// == boost time for aliens?
    BI_GRENADE,
    BI_MEDKIT,
    BI_JETPACK,
    BI_BATTPACK,
    BI_LARMOR,
    BI_HELMET,
    BI_BSUIT,
    BI_SIZE,
    BI_CLASS
} bot_inventory_t;

typedef enum
{
    NEXT_STATE_NONE,
    NEXT_STATE_LOS,     // Go to next state when u can see the goal
    NEXT_STATE_DISTANCE //Go to the next state when u are close enough to the goal.
} nextstatetype_t;

#define MAX_ACTIVATESTACK    8
#define MAX_ACTIVATEAREAS   32

typedef struct bot_activategoal_s
{
    S32 inuse;                        // true if this state is used by a bot client
    bot_goal_t goal;                  // goal to activate (buttons etc.)
    F32 time;                         // time to activate something
    F32 start_time;                   // time starting to activate something
    F32 justused_time;                // time the goal was used
    S32 shoot;                        // true if bot has to shoot to activate
    S32 weapon;                       // weapon to be used for activation
    vec3_t target;                    // target to shoot at to activate something
    vec3_t origin;                    // origin of the blocking entity to activate
    S32 areas[MAX_ACTIVATEAREAS];     // routing areas disabled by blocking entity
    S32 numareas;                     // number of disabled routing areas
    S32 areasdisabled;                // true if the areas are disabled for the routing
    struct bot_activategoal_s* next;  // next activate goal on stack
} bot_activategoal_t;

//bot state
typedef struct bot_state_s
{
    S32 inuse;                                              // true if this state is used by a bot client
    S32 botthink_residual;                                  // residual for the bot thinks
    S32 client;                                             // client number of the bot
    S32 entitynum;                                          // entity number of the bot
    playerState_t cur_ps;                                   // current player state
    S32 last_eFlags;                                        // last ps flags
    usercmd_t lastucmd;                                     // usercmd from last frame
    S32 entityeventTime[1024];                              // last entity event time
    bot_settings_t settings;                                // several bot settings
    S32( *ainode )( struct bot_state_s* bs );               // current AI node
    F32 thinktime;                                          // time the bot thinks this frame
    vec3_t origin;                                          // origin of the bot
    vec3_t velocity;                                        // velocity of the bot
    S32 presencetype;                                       // presence type of the bot
    vec3_t eye;                                             // eye coordinates of the bot
    S32 areanum;                                            // the number of the area the bot is in
    S32 inventory[MAX_ITEMS];                               // string with items amounts the bot has
    S32 tfl;                                                // the travel flags the bot uses
    S32 flags;                                              // several flags
    S32 respawn_wait;                                       // wait until respawned
    S32 lasthealth;                                         // health value previous frame
    S32 lastkilledplayer;                                   // last killed player
    S32 lastkilledby;                                       // player that last killed this bot
    S32 botdeathtype;                                       // the death type of the bot
    S32 enemydeathtype;                                     // the death type of the enemy
    S32 botsuicide;                                         // true when the bot suicides
    S32 enemysuicide;                                       // true when the enemy of the bot suicides
    S32 setupcount;                                         // true when the bot has just been setup
    S32 map_restart;                                        // true when the map is being restarted
    S32 entergamechat;                                      // true when the bot used an enter game chat
    S32 num_deaths;                                         // number of time this bot died
    S32 num_kills;                                          // number of kills of this bot
    S32 revenge_enemy;                                      // the revenge enemy
    S32 revenge_kills;                                      // number of kills the enemy made
    S32 lastframe_health;                                   // health value the last frame
    S32 lasthitcount;                                       // number of hits last frame
    S32 chatto;                                             // chat to all or team
    F32 walker;                                             // walker charactertic
    F32 ltime;                                              // local bot time
    F32 entergame_time;                                     // time the bot entered the game
    F32 ltg_time;                                           // long term goal time
    F32 nbg_time;                                           // nearby goal time
    F32 respawn_time;                                       // time the bot takes to respawn
    F32 respawnchat_time;                                   // time the bot started a chat during respawn
    F32 chase_time;                                         // time the bot will chase the enemy
    F32 enemyvisible_time;                                  // time the enemy was last visible
    F32 check_time;                                         // time to check for nearby items
    F32 stand_time;                                         // time the bot is standing still
    F32 lastchat_time;                                      // time the bot last selected a chat
    F32 kamikaze_time;                                      // time to check for kamikaze usage
    F32 invulnerability_time;                               // time to check for invulnerability usage
    F32 standfindenemy_time;                                // time to find enemy while standing
    F32 attackstrafe_time;                                  // time the bot is strafing in one dir
    F32 attackcrouch_time;                                  // time the bot will stop crouching
    F32 attackchase_time;                                   // time the bot chases during actual attack
    F32 attackjump_time;                                    // time the bot jumped during attack
    F32 enemysight_time;                                    // time before reacting to enemy
    F32 enemydeath_time;                                    // time the enemy died
    F32 enemyposition_time;                                 // time the position and velocity of the enemy were stored
    F32 defendaway_time;                                    // time away while defending
    F32 defendaway_range;                                   // max travel time away from defend area
    F32 rushbaseaway_time;                                  // time away from rushing to the base
    F32 attackaway_time;                                    // time away from attacking the enemy base
    F32 harvestaway_time;                                   // time away from harvesting
    F32 ctfroam_time;                                       // time the bot is roaming in ctf
    F32 killedenemy_time;                                   // time the bot killed the enemy
    F32 arrive_time;                                        // time arrived (at companion)
    F32 lastair_time;                                       // last time the bot had air
    F32 teleport_time;                                      // last time the bot teleported
    F32 camp_time;                                          // last time camped
    F32 camp_range;                                         // camp range
    F32 weaponchange_time;                                  // time the bot started changing weapons
    F32 firethrottlewait_time;                              // amount of time to wait
    F32 firethrottleshoot_time;                             // amount of time to shoot
    F32 notblocked_time;                                    // last time the bot was not blocked
    F32 blockedbyavoidspot_time;                            // time blocked by an avoid spot
    F32 predictobstacles_time;                              // last time the bot predicted obstacles
    S32 predictobstacles_goalareanum;                       // last goal areanum the bot predicted obstacles for
    vec3_t aimtarget;
    vec3_t enemyvelocity;                                   // enemy velocity 0.5 secs ago during battle
    vec3_t enemyorigin;                                     // enemy origin 0.5 secs ago during battle
    S32 kamikazebody;                                       // kamikaze body
    S32 numproxmines;
    S32 character;                                          // the bot character
    S32 ms;                                                 // move state of the bot
    S32 gs;                                                 // goal state of the bot
    S32 cs;                                                 // chat state of the bot
    S32 ws;                                                 // weapon state of the bot
    S32 enemy;                                              // enemy entity number
    S32 lastenemyareanum;                                   // last reachability area the enemy was in
    vec3_t lastenemyorigin;                                 // last origin of the enemy in the reachability area
    S32 weaponnum;                                          // current weapon number
    vec3_t viewangles;                                      // current view angles
    vec3_t ideal_viewangles;                                // ideal view angles
    vec3_t viewanglespeed;
    S32 ltgtype;                                            // long term goal type
    S32 teammate;                                           // team mate involved in this team goal
    S32 decisionmaker;                                      // player who decided to go for this goal
    S32 ordered;                                            // true if ordered to do something
    F32 order_time;                                         // time ordered to do something
    S32 owndecision_time;                                   // time the bot made it's own decision
    bot_goal_t teamgoal;                                    // the team goal
    bot_goal_t altroutegoal;                                // alternative route goal
    F32 reachedaltroutegoal_time;                           // time the bot reached the alt route goal
    F32 teammessage_time;                                   // time to message team mates what the bot is doing
    F32 teamgoal_time;                                      // time to stop helping team mate
    F32 teammatevisible_time;                               // last time the team mate was NOT visible
    S32 teamtaskpreference;                                 // team task preference
    S32 lastgoal_decisionmaker;
    S32 lastgoal_ltgtype;
    S32 lastgoal_teammate;
    bot_goal_t lastgoal_teamgoal;
    S32 lead_teammate;                                      // team mate the bot is leading
    bot_goal_t lead_teamgoal;                               // team goal while leading
    F32 lead_time;                                          // time leading someone
    F32 leadvisible_time;                                   // last time the team mate was visible
    F32 leadmessage_time;                                   // last time a messaged was sent to the team mate
    F32 leadbackup_time;                                    // time backing up towards team mate
    UTF8 teamleader[32];                                    // netname of the team leader
    F32 askteamleader_time;                                 // time asked for team leader
    F32 becometeamleader_time;                              // time the bot will become the team leader
    F32 teamgiveorders_time;                                // time to give team orders
    F32 lastflagcapture_time;                               // last time a flag was captured
    S32 numteammates;                                       // number of team mates
    S32 redflagstatus;                                      // 0 = at base, 1 = not at base
    S32 blueflagstatus;                                     // 0 = at base, 1 = not at base
    S32 neutralflagstatus;                                  // 0 = at base, 1 = our team has flag, 2 = enemy team has flag, 3 = enemy team dropped the flag
    S32 flagstatuschanged;                                  // flag status changed
    S32 forceorders;                                        // true if forced to give orders
    S32 flagcarrier;                                        // team mate carrying the enemy flag
    S32 ctfstrategy;                                        // ctf strategy
    UTF8 subteam[32];                                       // sub team name
    F32 formation_dist;                                     // formation team mate intervening space
    UTF8 formation_teammate[16];                            // netname of the team mate the bot uses for relative positioning
    F32 formation_angle;                                    // angle relative to the formation team mate
    vec3_t formation_dir;                                   // the direction the formation is moving in
    vec3_t formation_origin;                                // origin the bot uses for relative positioning
    bot_goal_t formation_goal;                              // formation goal
    bot_activategoal_t* activatestack;                      // first activate goal on the stack
    bot_activategoal_t activategoalheap[MAX_ACTIVATESTACK]; // activate goal heap
    bot_waypoint_t* checkpoints;                            // check points
    bot_waypoint_t* patrolpoints;                           // patrol points
    bot_waypoint_t* curpatrolpoint;                         // current patrol point the bot is going for
    S32 patrolflags;                                        // patrol flags
    F32 lastheardtime;                                      // Time we last ''heard'' something
    vec3_t lastheardorigin;                                 // Origin of it
    S32 team;
    gentity_t* enemyent;
    gentity_t* ent;
    UTF8 hudinfo[MAX_INFO_STRING];                          // for bot_developer hud debug text
    F32 buyammo_time;
    F32 buygun_time;
    F32 findenemy_time;                                     // time the bot last looked for a new enemy
    S32 state;
    S32 prevstate;
    S32 statecycles;
    bot_goal_t goal;
} bot_state_t;

extern bot_waypoint_t botai_waypoints[MAX_WAYPOINTS];
extern bot_waypoint_t* botai_freewaypoints;
extern S32 max_bspmodelindex;                               // maximum BSP model index

//
// idBotLocal
//
class idBotLocal
{
public:
    UTF8* EasyClientName( S32 client, UTF8* buf, S32 size );
    S32 BotReachedGoal( bot_state_t* bs, bot_goal_t* goal );
    void BotSetEntityNumForGoalWithModel( bot_goal_t* goal, S32 eType, UTF8* modelname );
    bool TeamPlayIsOn( void );
    void BotInitWaypoints( void );
    void BotFreeWaypoints( bot_waypoint_t* wp );
    bot_waypoint_t* BotFindWayPoint( bot_waypoint_t* waypoints, UTF8* name );
    bot_waypoint_t* BotCreateWayPoint( UTF8* name, vec3_t origin, S32 areanum );
    bool EntityIsShooting( aas_entityinfo_t* entinfo );
    bool EntityIsChatting( aas_entityinfo_t* entinfo );
    bool BotIsDead( bot_state_t* bs );
    bool BotInLavaOrSlime( bot_state_t* bs );
    UTF8* ClientName( S32 client, UTF8* name, S32 size );
    UTF8* ClientSkin( S32 client, UTF8* skin, S32 size );
    F32 AngleDifference( F32 ang1, F32 ang2 );
    F32 BotChangeViewAngle( F32 angle, F32 ideal_angle, F32 speed );
    void BotChangeViewAngles( bot_state_t* bs, F32 thinktime );
    S32 BotPointAreaNum( vec3_t origin );
    void Bot_Print( botprint_t type, UTF8* fmt, ... );
    void BotAddInfo( bot_state_t* bs, UTF8* key, UTF8* value );
    S32 BotAI_GetClientState( S32 clientNum, playerState_t* state );
    void BotSetupForMovement( bot_state_t* bs );
    S32 BotSameTeam( bot_state_t* bs, S32 entnum );
    bool BotIsAlive( bot_state_t* bs );
    bool BotIntermission( bot_state_t* bs );
    S32 CheckAreaForGoal( vec3_t origin, vec3_t bestorigin );
    bool CheckReachability( bot_goal_t* goal );
    void TraceDownToGround( bot_state_t* bs, vec3_t origin, vec3_t out );
    void OrgToGoal( vec3_t org, bot_goal_t* goal );
    S32 GetWalkingDist( bot_state_t* bs, vec3_t origin );
    bool BotGoalForBuildable( bot_state_t* bs, bot_goal_t* goal, S32 bclass );
    static bool Nullcheckfuct( bot_state_t* bs, S32 entnum, F32 dist );
    bool BotGoalForClosestBuildable( bot_state_t* bs, bot_goal_t* goal, S32 bclass, bool( *check )( bot_state_t* bs, S32 entnum, F32 dist ) );
    S32 BotFindEnemy( bot_state_t* bs, S32 curenemy );
    bool BotGoalForEnemy( bot_state_t* bs, bot_goal_t* goal );
    bool BotGoalForNearestEnemy( bot_state_t* bs, bot_goal_t* goal );
    bool BotInFieldOfVision( vec3_t viewangles, F32 fov, vec3_t angles );
    F32 BotEntityVisible( S32 viewer, vec3_t eye, vec3_t viewangles, F32 fov, S32 ent );
    F32 BotEntityDistance( bot_state_t* bs, S32 ent );
    void BotAI_Trace( bsp_trace_t* bsptrace, vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, S32 passent, S32 contentmask );
    bool EntityIsInvisible( aas_entityinfo_t* entinfo );
    bool AI_BuildableWalkingRange( bot_state_t* bs, S32 r, buildable_t buildable );
    bool EntityIsDead( aas_entityinfo_t* entinfo );
    S32 ClientOnSameTeamFromName( bot_state_t* bs, UTF8* name );
    void BotSetTeleportTime( bot_state_t* bs );
    bool BotIsObserver( bot_state_t* bs );
    void BotShowViewAngles( bot_state_t* bs );
    void BotAI_Print( S32 type, UTF8* fmt, ... );
    void BotInputToUserCommand( bot_input_t* bi, usercmd_t* ucmd, S32 delta_angles[3], S32 time );
    void BotUpdateInput( bot_state_t* bs, S32 time, S32 elapsed_time );
    S32 BotAI_GetEntityState( S32 entityNum, entityState_t* state );
    S32 BotAI_GetSnapshotEntity( S32 clientNum, S32 sequence, entityState_t* state );
    void BotCheckEvents( bot_state_t* bs, entityState_t* state );
    void BotCheckSnapshot( bot_state_t* bs );
    UTF8* BotGetMenuText( S32 menu, S32 arg );
    S32 BotAI( S32 client, F32 thinktime );
    void BotCheckDeath( S32 target, S32 attacker, S32 mod );
    void BotScheduleBotThink( void );
    S32 BotAISetupClient( S32 client, struct bot_settings_s* settings, bool restart );
    S32 BotAIShutdownClient( S32 client, bool restart );
    void BotResetState( bot_state_t* bs );
    S32 BotAILoadMap( S32 restart );
    S32 BotInitLibrary( void );
    S32 BotAISetup( S32 restart );
    S32 BotAIShutdown( S32 restart );
    void BotEntityInfo( S32 entnum, aas_entityinfo_t* info );
    void BotRandomMove( bot_state_t* bs, bot_moveresult_t* moveresult );
    void BotAIBlocked( bot_state_t* bs, bot_moveresult_t* moveresult, S32 activate );
    S32 NumBots( void );
    S32 Bot_TargetValue( bot_state_t* bs, S32 i );
    S32 Bot_FindTarget( bot_state_t* bs );
    void BotBeginIntermission( void );
    void BotResetStateSwitches( void );
    void HBotDumpStateSwitches( bot_state_t* bs );
    void HBotRecordStateSwitch( bot_state_t* bs, UTF8* node, UTF8* str, UTF8* s );
    void HBotAimAtEnemy( bot_state_t* bs, S32 enemy );
    void HBotCheckAttack( bot_state_t* bs, S32 attackentity );
    void BBotCheckClientAttack( bot_state_t* bs );
    bool HBotHealthOK( bot_state_t* bs );
    void HBotCheckReload( bot_state_t* bs );
    void HBotCheckRespawn( bot_state_t* bs );
    void HBotUpdateInventory( bot_state_t* bs );
    void BuyAmmo( bot_state_t* bs );
    bool HBotEquipOK( bot_state_t* bs );
    void HBotShop( bot_state_t* bs );
    bool HBotAttack2( bot_state_t* bs );
    bool HBotFindEnemy( bot_state_t* bs );
    void HBotAvoid( bot_state_t* bs );
    void HBotStrafe( bot_state_t* bs );
    void HBotEnterAttack( bot_state_t* bs, UTF8* s );
    bool HBotAttack( bot_state_t* bs );
    static bool CheckMedi( bot_state_t* bs, S32 entnum, F32 dist );
    void HBotEnterHeal( bot_state_t* bs, UTF8* s );
    bool HBotHeal( bot_state_t* bs );
    void HBotEnterGear( bot_state_t* bs, UTF8* s );
    bool HBotGear( bot_state_t* bs );
    void HBotEnterSpawn( bot_state_t* bs, UTF8* s );
    bool HBotSpawn( bot_state_t* bs );
    void HBotEnterChat( bot_state_t* bs );
    bool HBotChat( bot_state_t* bs );
    bool HBotRunState( bot_state_t* bs );
    void BotHumanAI( bot_state_t* bs, F32 thinktime );
    void BotAI_BotInitialChat( bot_state_t* bs, UTF8* type, ... );
    bool EntityCarriesFlag( aas_entityinfo_t* entinfo );
    S32 BotNumActivePlayers( void );
    S32 BotIsFirstInRankings( bot_state_t* bs );
    S32 BotIsLastInRankings( bot_state_t* bs );
    UTF8* BotFirstClientInRankings( void );
    UTF8* BotLastClientInRankings( void );
    UTF8* BotRandomOpponentName( bot_state_t* bs );
    UTF8* BotMapTitle( void );
    UTF8* BotWeaponNameForMeansOfDeath( S32 mod );
    UTF8* BotRandomWeaponName( void );
    S32 BotVisibleEnemies( bot_state_t* bs );
    S32 BotValidChatPosition( bot_state_t* bs );
    S32 BotChat_EnterGame( bot_state_t* bs );
    S32 BotChat_ExitGame( bot_state_t* bs );
    S32 BotChat_StartLevel( bot_state_t* bs );
    S32 BotChat_EndLevel( bot_state_t* bs );
    S32 BotChat_Death( bot_state_t* bs );
    S32 BotChat_Kill( bot_state_t* bs );
    S32 BotChat_EnemySuicide( bot_state_t* bs );
    S32 BotChat_HitTalking( bot_state_t* bs );
    S32 BotChat_HitNoDeath( bot_state_t* bs );
    S32 BotChat_HitNoKill( bot_state_t* bs );
    S32 BotChat_Random( bot_state_t* bs );
    F32 BotChatTime( bot_state_t* bs );
    void BotChatTest( bot_state_t* bs );
    F32 ABotFindWidth( S32 weapon );
    F32 ABotFindRange( S32 weapon );
    void ABotAimAtEnemy( bot_state_t* bs );
    void ABotCheckAttack( bot_state_t* bs );
    bool HumansNearby( bot_state_t* bs );
    bool ABotClassOK( bot_state_t* bs );
    void ABotEvolve( bot_state_t* bs );
    void ABotCheckRespawn( bot_state_t* bs );
    void ABotUpdateInventory( bot_state_t* bs );
    bool ABotFindEnemy( bot_state_t* bs );
    bool ABotAttack( bot_state_t* bs );
    bool ABotEvo( bot_state_t* bs );
    bool ABotSpawn( bot_state_t* bs );
    bool ABotRunState( bot_state_t* bs );
    void BotAlienAI( bot_state_t* bs, F32 thinktime );
    
    friend class idGameLocal;
    friend class idAdminLocal;
};

extern idBotLocal botLocal;

#endif //__AI_MAIN_H__
