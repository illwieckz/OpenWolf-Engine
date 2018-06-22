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
// File name:   g_local.cpp
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description: Local definitions for game module
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#ifndef __G_LOCAL_H__
#define __G_LOCAL_H__

#ifndef __Q_SHARED_H__
#include <qcommon/q_shared.h>
#endif
#ifndef __BG_PUBLIC_H__
#include <bgame/bg_public.h>
#endif
#ifndef __G_API_H__
#include <server/g_api.h>
#endif
#ifndef __R_TYPES_H__
#include <GPURenderer/r_types.h>
#endif
#ifndef __SPLINES_H__
#include <OWLib/splines.h>
#endif
#ifndef __BE_AAS_H__
#include <botlib/be_aas.h>
#endif

typedef struct gclient_s gclient_t;

#ifndef __G_ADMIN_H__
#include <game/g_admin.h>
#endif

//==================================================================

#define INFINITE 1000000

#define FRAMETIME 100         // msec
#define CARNAGE_REWARD_TIME 3000
#define REWARD_SPRITE_TIME 2000

#define INTERMISSION_DELAY_TIME 1000
#define SP_INTERMISSION_DELAY_TIME 5000

// gentity->flags
#define FL_GODMODE        0x00000010
#define FL_NOTARGET       0x00000020
#define FL_TEAMSLAVE      0x00000400  // not the first on the team
#define FL_NO_KNOCKBACK   0x00000800
#define FL_DROPPED_ITEM   0x00001000
#define FL_NO_BOTS        0x00002000  // spawn point not for bot use
#define FL_NO_HUMANS      0x00004000  // spawn point just for bots
#define FL_FORCE_GESTURE  0x00008000  // spawn point just for bots

// movers are things like doors, plats, buttons, etc
typedef enum
{
    MOVER_POS1,
    MOVER_POS2,
    MOVER_1TO2,
    MOVER_2TO1,
    
    ROTATOR_POS1,
    ROTATOR_POS2,
    ROTATOR_1TO2,
    ROTATOR_2TO1,
    
    MODEL_POS1,
    MODEL_POS2,
    MODEL_1TO2,
    MODEL_2TO1
} moverState_t;

#define SP_PODIUM_MODEL   "models/mapobjects/podium/podium4.md3"

//============================================================================

struct gentity_s
{
    entityState_t     s;        // communicated by server to clients
    entityShared_t    r;        // shared by both the server system and game
    
    // DO NOT MODIFY ANYTHING ABOVE THIS, THE SERVER
    // EXPECTS THE FIELDS IN THAT ORDER!
    //================================
    
    struct gclient_s*  client;        // NULL if not a client
    
    bool              inuse;
    
    UTF8*             classname;     // set in QuakeEd
    S32               spawnflags;     // set in QuakeEd
    
    bool              neverFree;      // if true, FreeEntity will only unlink
    // bodyque uses this
    
    S32               flags;          // FL_* variables
    
    UTF8*             model;
    UTF8*             model2;
    S32               freetime;       // level.time when the object was freed
    
    S32               eventTime;      // events will be cleared EVENT_VALID_MSEC after set
    bool              freeAfterEvent;
    bool              unlinkAfterEvent;
    
    bool              physicsObject;  // if true, it can be pushed by movers and fall off edges
    // all game items are physicsObjects,
    F32               physicsBounce;  // 1.0 = continuous bounce, 0.0 = no bounce
    S32               clipmask;       // brushes with this content value will be collided against
    // when moving.  items and corpses do not collide against
    // players, for instance
    
    // movers
    moverState_t      moverState;
    S32               soundPos1;
    S32               sound1to2;
    S32               sound2to1;
    S32               soundPos2;
    S32               soundLoop;
    gentity_t*        parent;
    gentity_t*        nextTrain;
    gentity_t*        prevTrain;
    vec3_t            pos1, pos2;
    F32               rotatorAngle;
    gentity_t*        clipBrush;     // clipping brush for model doors
    
    UTF8*             message;
    
    S32               timestamp;      // body queue sinking, etc
    
    F32               angle;          // set in editor, -1 = up, -2 = down
    UTF8*             target;
    UTF8*             targetname;
    UTF8*             team;
    UTF8*             targetShaderName;
    UTF8*             targetShaderNewName;
    gentity_t*        target_ent;
    
    F32               speed;
    F32               lastSpeed;      // used by trains that have been restarted
    vec3_t            movedir;
    
    // acceleration evaluation
    bool              evaluateAcceleration;
    vec3_t            oldVelocity;
    vec3_t            acceleration;
    vec3_t            oldAccel;
    vec3_t            jerk;
    
    S32               nextthink;
    void ( *think )( gentity_t* self );
    void ( *reached )( gentity_t* self );             // movers call this when hitting endpoint
    void ( *blocked )( gentity_t* self, gentity_t* other );
    void ( *touch )( gentity_t* self, gentity_t* other, trace_t* trace );
    void ( *use )( gentity_t* self, gentity_t* other, gentity_t* activator );
    void ( *pain )( gentity_t* self, gentity_t* attacker, S32 damage );
    void ( *die )( gentity_t* self, gentity_t* inflictor, gentity_t* attacker, S32 damage, S32 mod );
    
    S32               pain_debounce_time;
    S32               fly_sound_debounce_time;  // wind tunnel
    S32               last_move_time;
    
    S32               health;
    S32               lastHealth; // currently only used for overmind
    
    bool              takedamage;
    
    S32               damage;
    S32               splashDamage; // quad will increase this without increasing radius
    S32               splashRadius;
    S32               methodOfDeath;
    S32               splashMethodOfDeath;
    
    S32               count;
    
    gentity_t*        chain;
    gentity_t*        enemy;
    gentity_t*        activator;
    gentity_t*        teamchain;   // next entity in team
    gentity_t*        teammaster;  // master of the team
    
    S32               watertype;
    S32               waterlevel;
    
    S32               noise_index;
    
    // timing variables
    F32               wait;
    F32               random;
    
    team_t            stageTeam;
    stage_t           stageStage;
    
    team_t            buildableTeam;      // buildable item team
    gentity_t*        parentNode;        // for creep and defence/spawn dependencies
    bool              active;             // for power repeater, but could be useful elsewhere
    bool              locked;             // used for turret tracking
    bool              powered;            // for human buildables
    S32               builtBy;            // clientNum of person that built this
    gentity_t*        overmindNode;      // controlling overmind
    S32               dcc;                // controlled by how many dccs
    bool              spawned;            // whether or not this buildable has finished spawning
    S32               shrunkTime;         // time when a barricade shrunk or zero
    S32               buildTime;          // when this buildable was built
    S32               animTime;           // last animation change
    S32               time1000;           // timer evaluated every second
    bool              deconstruct;        // deconstruct if no BP left
    S32               deconstructTime;    // time at which structure marked
    S32               overmindAttackTimer;
    S32               overmindDyingTimer;
    S32               overmindSpawnsTimer;
    S32               nextPhysicsTime;    // buildables don't need to check what they're sitting on
    // every single frame.. so only do it periodically
    S32               clientSpawnTime;    // the time until this spawn can spawn a client
    bool              lev1Grabbed;        // for turrets interacting with lev1s
    S32               lev1GrabTime;       // for turrets interacting with lev1s
    
    S32               credits[ MAX_CLIENTS ];     // human credits for each client
    bool              creditsHash[ MAX_CLIENTS ]; // track who has claimed credit
    S32               killedBy;                   // clientNum of killer
    
    gentity_t*        targeted;          // true if the player is currently a valid target of a turret
    vec3_t            turretAim;          // aim vector for turrets
    vec3_t            turretAimRate;      // track turn speed for norfenturrets
    S32               turretSpinupTime;   // spinup delay for norfenturrets
    
    vec4_t            animation;          // animated map objects
    
    gentity_t*        builder;           // occupant of this hovel
    
    bool              nonSegModel;        // this entity uses a nonsegmented player model
    
    buildable_t       bTriggers[ BA_NUM_BUILDABLES ]; // which buildables are triggers
    class_t           cTriggers[ PCL_NUM_CLASSES ];   // which classes are triggers
    weapon_t          wTriggers[ WP_NUM_WEAPONS ];    // which weapons are triggers
    upgrade_t         uTriggers[ UP_NUM_UPGRADES ];   // which upgrades are triggers
    
    S32               triggerGravity;                 // gravity for this trigger
    
    S32               suicideTime;                    // when the client will suicide
    
    S32               lastDamageTime;
    S32               nextRegenTime;
    
    bool              zapping;                        // adv maurader is zapping
    bool              wasZapping;                     // adv maurader was zapping
    S32               zapTargets[ LEVEL2_AREAZAP_MAX_TARGETS ];
    F32               zapDmg;                         // keep track of damage
    
    bool              ownerClear;                     // used for missle tracking
    
    bool              pointAgainstWorld;              // don't use the bbox for map collisions
    S32               mass;
};

typedef enum
{
    CON_DISCONNECTED,
    CON_CONNECTING,
    CON_CONNECTED
} clientConnected_t;

// the auto following clients don't follow a specific client
// number, but instead follow the first two active players
#define FOLLOW_ACTIVE1  -1
#define FOLLOW_ACTIVE2  -2

// client data that stays across multiple levels or tournament restarts
// this is achieved by writing all the data to cvar strings at game shutdown
// time and reading them back at connection time.  Anything added here
// MUST be dealt with in idGameLocal::InitSessionData() / idGameLocal::ReadSessionData() / idGameLocal::WriteSessionData()
typedef struct
{
    S32               spectatorTime;    // for determining next-in-line to play
    spectatorState_t  spectatorState;
    S32               spectatorClient;  // for chasecam and follow mode
    S32               wins, losses;     // tournament stats
    bool              teamLeader;       // true when this client is a team leader
    team_t            restartTeam;      // for !restart keepteams and !restart switchteams
    clientList_t      ignoreList;
} clientSession_t;

// data to store details of clients that have abnormally disconnected
typedef struct connectionRecord_s
{
    S32       clientNum;
    S32       oldClient;
    team_t    clientTeam;
    S32       clientCredit;
    
    S32       ptrCode;
} connectionRecord_t;

// client data that stays across multiple respawns, but is cleared
// on each level change or team change at ClientBegin()
typedef struct
{
    clientConnected_t   connected;
    usercmd_t           cmd;                // we would lose angles if not persistant
    bool                localClient;        // true if "ip" info key is "localhost"
    bool                initialSpawn;       // the first spawn should be at a cool location
    bool                stickySpec;         // don't stop spectating a player after they get killed
    bool                pmoveFixed;         //
    UTF8                netname[ MAX_NAME_LENGTH ];
    S32                 maxHealth;          // for handicapping
    S32                 enterTime;          // level.time the client entered the game
    S32                 location;           // player locations
    S32                 voteCount;          // to prevent people from constantly calling votes
    bool                teamInfo;           // send team overlay updates?
    F32                 flySpeed;           // for spectator/noclip moves
    
    class_t             classSelection;     // player class (copied to ent->client->ps.stats[ STAT_CLASS ] once spawned)
    F32                 evolveHealthFraction;
    weapon_t            humanItemSelection; // humans have a starting item
    team_t              teamSelection;      // player team (copied to ps.stats[ STAT_TEAM ])
    
    S32                 teamChangeTime;     // level.time of last team change
    bool                joinedATeam;        // used to tell when a PTR code is valid
    connectionRecord_t*  connection;
    
    S32                 nameChangeTime;
    S32                 nameChanges;
    
    // used to save persistant[] values while in SPECTATOR_FOLLOW mode
    S32                 savedCredit;
    
    // votes
    bool                vote;
    bool                teamVote;
    
    // flood protection
    S32                 floodDemerits;
    S32                 floodTime;
    
    vec3_t              lastDeathLocation;
    UTF8                guid[ 33 ];
    UTF8                ip[ 40 ];
    bool                muted;
    bool                denyBuild;
    bool                demoClient;
    S32                 adminLevel;
    UTF8                voice[ MAX_VOICE_NAME_LEN ];
    bool                useUnlagged;
    g_admin_admin_t*    admin;
    S32                 pubkey_authenticated; // -1 = does not have pubkey, 0 = not authenticated, 1 = authenticated
    S32                 cl_pubkeyID;
    UTF8                pubkey_msg[RSA_STRING_LENGTH];
    UTF8                connect_name[MAX_NAME_LENGTH]; // Name of client before admin was removed with pubkey
    S32				    mysqlid;
} clientPersistant_t;

#define MAX_UNLAGGED_MARKERS 10
typedef struct unlagged_s
{
    vec3_t      origin;
    vec3_t      mins;
    vec3_t      maxs;
    bool    used;
} unlagged_t;

// this structure is cleared on each ClientSpawn(),
// except for 'client->pers' and 'client->sess'
struct gclient_s
{
    // ps MUST be the first element, because the server expects it
    playerState_t       ps;       // communicated by server to clients
    
    // exported into pmove, but not communicated to clients
    pmoveExt_t          pmext;
    
    // the rest of the structure is private to game
    clientPersistant_t  pers;
    clientSession_t     sess;
    
    bool                readyToExit;    // wishes to leave the intermission
    
    bool                noclip;
    
    S32                 lastCmdTime;    // level.time of last usercmd_t, for EF_CONNECTION
    // we can't just use pers.lastCommand.time, because
    // of the g_sycronousclients case
    S32                 buttons;
    S32                 oldbuttons;
    S32                 latched_buttons;
    
    vec3_t              oldOrigin;
    
    // sum up damage over an entire frame, so
    // shotgun blasts give a single big kick
    S32                 damage_armor;     // damage absorbed by armor
    S32                 damage_blood;     // damage taken out of health
    S32                 damage_knockback; // impact damage
    vec3_t              damage_from;      // origin for vector calculation
    bool                damage_fromWorld; // if true, don't use the damage_from vector
    
    //
    S32                 lastkilled_client;// last client that this client killed
    S32                 lasthurt_client;  // last client that damaged this client
    S32                 lasthurt_mod;     // type of damage the client did
    
    // timers
    S32                 respawnTime;      // can respawn when time > this
    S32                 inactivityTime;   // kick players when time > this
    bool                inactivityWarning;// true if the five seoond warning has been given
    S32                 rewardTime;       // clear the EF_AWARD_IMPRESSIVE, etc when time > this
    S32                 boostedTime;      // last time we touched a booster
    
    S32                 airOutTime;
    
    S32                 lastKillTime;     // for multiple kill rewards
    
    bool                fireHeld;         // used for hook
    bool                fire2Held;        // used for alt fire
    gentity_t*           hook;            // grapple hook if out
    
    S32                 switchTeamTime;   // time the player switched teams
    
    S32                 time100;          // timer for 100ms interval events
    S32                 time1000;         // timer for one second interval events
    
    UTF8*                areabits;
    
    gentity_t*           hovel;
    
    S32                 lastPoisonTime;
    S32                 poisonImmunityTime;
    gentity_t*          lastPoisonClient;
    S32                 lastPoisonCloudedTime;
    S32                 grabExpiryTime;
    S32                 lastLockTime;
    S32                 lastSlowTime;
    S32                 lastMedKitTime;
    S32                 medKitHealthToRestore;
    S32                 medKitIncrementTime;
    S32                 lastCreepSlowTime;    // time until creep can be removed
    
    bool                charging;
    
    vec3_t              hovelOrigin;          // player origin before entering hovel
    
    S32                 lastFlameBall;        // s.number of the last flame ball fired
    
    unlagged_t          unlaggedHist[ MAX_UNLAGGED_MARKERS ];
    unlagged_t          unlaggedBackup;
    unlagged_t          unlaggedCalc;
    S32                 unlaggedTime;
    
    F32                 voiceEnthusiasm;
    UTF8                lastVoiceCmd[ MAX_VOICE_CMD_LEN ];
    
    S32                 lcannonStartTime;
    
    S32                 lastCrushTime;        // Tyrant crush
};

typedef struct spawnQueue_s
{
    S32 clients[ MAX_CLIENTS ];
    S32 front, back;
} spawnQueue_t;

#define QUEUE_PLUS1(x)  (((x)+1)%MAX_CLIENTS)
#define QUEUE_MINUS1(x) (((x)+MAX_CLIENTS-1)%MAX_CLIENTS)

#define MAX_DAMAGE_REGION_TEXT    8192
#define MAX_DAMAGE_REGIONS 16

// store locational damage regions
typedef struct damageRegion_s
{
    UTF8 name[ 32 ];
    F32 area, modifier, minHeight, maxHeight;
    S32 minAngle, maxAngle;
    bool crouch;
} damageRegion_t;

// demo commands
typedef enum
{
    DC_SERVER_COMMAND = -1,
    DC_CLIENT_SET = 0,
    DC_CLIENT_REMOVE,
    DC_SET_STAGE
} demoCommand_t;

//status of the warning of certain events
typedef enum
{
    TW_NOT = 0,
    TW_IMMINENT,
    TW_PASSED
} timeWarning_t;

//
// this structure is cleared as each map is entered
//
#define MAX_SPAWN_VARS      64
#define MAX_SPAWN_VARS_CHARS  4096

typedef struct
{
    struct gclient_s*  clients;   // [maxclients]
    
    struct gentity_s*  gentities;
    S32               gentitySize;
    S32               num_entities;   // current number, <= MAX_GENTITIES
    
    S32               warmupTime;     // restart match at this time
    
    fileHandle_t      logFile;
    
    // store latched cvars here that we want to get at often
    S32               maxclients;
    
    S32               framenum;
    S32               time;                         // in msec
    S32               previousTime;                 // so movers can back up when blocked
    S32               frameMsec;                    // trap_Milliseconds() at end frame
    
    S32               startTime;                    // level.time the map was started
    
    S32               teamScores[ NUM_TEAMS ];
    S32               lastTeamLocationTime;         // last time of client team location update
    
    bool              newSession;                   // don't use any old session data, because
    // we changed gametype
    bool              restarted;                    // waiting for a map_restart to fire
    
    S32               numConnectedClients;
    S32               numNonSpectatorClients;       // includes connecting clients
    S32               numPlayingClients;            // connected, non-spectators
    S32               sortedClients[MAX_CLIENTS];   // sorted by score
    
    S32               snd_fry;                      // sound index for standing in lava
    
    S32               warmupModificationCount;      // for detecting if g_warmup is changed
    
    // voting state
    UTF8              voteString[MAX_STRING_CHARS];
    UTF8              voteDisplayString[MAX_STRING_CHARS];
    S32               voteTime;                     // level.time vote was called
    S32               votePassThreshold;            // need at least this percent to pass
    S32               voteExecuteTime;              // time the vote is executed
    S32               voteYes;
    S32               voteNo;
    S32               numVotingClients;             // set by CalculateRanks
    
    // team voting state
    UTF8              teamVoteString[ 2 ][ MAX_STRING_CHARS ];
    UTF8              teamVoteDisplayString[ 2 ][ MAX_STRING_CHARS ];
    S32               teamVoteTime[ 2 ];            // level.time vote was called
    S32               teamVoteYes[ 2 ];
    S32               teamVoteNo[ 2 ];
    S32               numteamVotingClients[ 2 ];    // set by CalculateRanks
    
    // spawn variables
    bool              spawning;                     // the G_Spawn*() functions are valid
    S32               numSpawnVars;
    UTF8*             spawnVars[ MAX_SPAWN_VARS ][ 2 ];  // key / value pairs
    S32               numSpawnVarChars;
    UTF8              spawnVarChars[ MAX_SPAWN_VARS_CHARS ];
    
    // intermission state
    S32               intermissionQueued;           // intermission was qualified, but
    // wait INTERMISSION_DELAY_TIME before
    // actually going there so the last
    // frag can be watched.  Disable future
    // kills during this delay
    S32               intermissiontime;             // time the intermission was started
    UTF8*             changemap;
    bool              readyToExit;                  // at least one client wants to exit
    S32               exitTime;
    vec3_t            intermission_origin;          // also used for spectator spawns
    vec3_t            intermission_angle;
    
    bool              locationLinked;               // target_locations get linked
    gentity_t*        locationHead;                // head of the location list
    
    S32               numAlienSpawns;
    S32               numHumanSpawns;
    
    S32               numAlienClients;
    S32               numHumanClients;
    
    F32               averageNumAlienClients;
    S32               numAlienSamples;
    F32               averageNumHumanClients;
    S32               numHumanSamples;
    
    S32               numLiveAlienClients;
    S32               numLiveHumanClients;
    
    S32               alienBuildPoints;
    S32               alienBuildPointQueue;
    S32               alienNextQueueTime;
    S32               humanBuildPoints;
    S32               humanBuildPointQueue;
    S32               humanNextQueueTime;
    
    gentity_t*        markedBuildables[ MAX_GENTITIES ];
    S32               numBuildablesForRemoval;
    
    S32               alienKills;
    S32               humanKills;
    
    bool              reactorPresent;
    bool              overmindPresent;
    bool              overmindMuted;
    
    S32               humanBaseAttackTimer;
    
    team_t            lastWin;
    
    bool              suddenDeath;
    S32               suddenDeathBeginTime;
    timeWarning_t     suddenDeathWarning;
    timeWarning_t     timelimitWarning;
    
    spawnQueue_t      alienSpawnQueue;
    spawnQueue_t      humanSpawnQueue;
    
    S32               alienStage2Time;
    S32               alienStage3Time;
    S32               humanStage2Time;
    S32               humanStage3Time;
    
    S32               alienStagedownCredits;    //credits at the time the opposing team
    S32               humanStagedownCredits;   //became vulnerable to a stage-down
    
    
    bool              uncondAlienWin;
    bool              uncondHumanWin;
    bool              alienTeamLocked;
    bool              humanTeamLocked;
    
    S32               unlaggedIndex;
    S32               unlaggedTimes[ MAX_UNLAGGED_MARKERS ];
    
    UTF8              layout[ MAX_QPATH ];
    
    team_t            surrenderTeam;
    
    voice_t*          voices;
    
    UTF8              emoticons[ MAX_EMOTICONS ][ MAX_EMOTICON_NAME_LEN ];
    S32               emoticonCount;
    S32               numBots;
    demoState_t       demoState;
} level_locals_t;

#define CMD_CHEAT         0x0001
#define CMD_CHEAT_TEAM    0x0002 // is a cheat when used on a team
#define CMD_MESSAGE       0x0004 // sends message to others (skip when muted)
#define CMD_TEAM          0x0008 // must be on a team
#define CMD_SPEC          0x0010 // must be a spectator
#define CMD_ALIEN         0x0020
#define CMD_HUMAN         0x0040
#define CMD_LIVING        0x0080
#define CMD_INTERMISSION  0x0100 // valid during intermission

typedef struct
{
    UTF8* cmdName;
    S32  cmdFlags;
    void ( *cmdHandler )( gentity_t* ent );
} commands_t;

#define MAX_ALIEN_BBOX  25

typedef enum
{
    IBE_NONE,
    
    IBE_NOOVERMIND,
    IBE_ONEOVERMIND,
    IBE_NOALIENBP,
    IBE_SPWNWARN, // not currently used
    IBE_NOCREEP,
    IBE_ONEHOVEL,
    IBE_HOVELEXIT,
    
    IBE_ONEREACTOR,
    IBE_NOPOWERHERE,
    IBE_TNODEWARN, // not currently used
    IBE_RPTNOREAC,
    IBE_RPTPOWERHERE,
    IBE_NOHUMANBP,
    IBE_NODCC,
    
    IBE_NORMAL, // too steep
    IBE_NOROOM,
    IBE_PERMISSION,
    IBE_LASTSPAWN,
    
    IBE_MAXERRORS
} itemBuildError_t;

// damage flags
#define DAMAGE_RADIUS         0x00000001  // damage was indirect
#define DAMAGE_NO_ARMOR       0x00000002  // armour does not protect from this damage
#define DAMAGE_NO_KNOCKBACK   0x00000004  // do not affect velocity, just view angles
#define DAMAGE_NO_PROTECTION  0x00000008  // armor, shields, invulnerability, and godmode have no effect
#define DAMAGE_NO_LOCDAMAGE   0x00000010  // do not apply locational damage

#define MAX_ZAP_TARGETS LEVEL2_AREAZAP_MAX_TARGETS

typedef struct zap_s
{
    bool           used;
    gentity_t*     creator;
    gentity_t*     targets[ MAX_ZAP_TARGETS ];
    S32            numTargets;
    S32            timeToLive;
    S32            damageUsed;
    gentity_t*     effectChannel;
} zap_t;

#define MAX_MAP_ROTATIONS       16
#define MAX_MAP_ROTATION_MAPS   64
#define MAX_MAP_COMMANDS        16
#define MAX_MAP_ROTATION_CONDS  4
#define NOT_ROTATING           -1

typedef enum
{
    MCV_ERR,
    MCV_RANDOM,
    MCV_NUMCLIENTS,
    MCV_LASTWIN
} mapConditionVariable_t;

typedef enum
{
    MCO_LT,
    MCO_EQ,
    MCO_GT
} mapConditionOperator_t;

typedef enum
{
    MCT_ERR,
    MCT_MAP,
    MCT_ROTATION
} mapConditionType_t;

typedef struct mapRotationCondition_s
{
    UTF8                    dest[ MAX_QPATH ];
    bool                    unconditional;
    mapConditionVariable_t  lhs;
    mapConditionOperator_t  op;
    S32                     numClients;
    team_t                  lastWin;
} mapRotationCondition_t;

typedef struct mapRotationEntry_s
{
    UTF8                    name[ MAX_QPATH ];
    UTF8                    postCmds[ MAX_MAP_COMMANDS ][ MAX_STRING_CHARS ];
    UTF8                    layouts[ MAX_CVAR_VALUE_STRING ];
    S32                     numCmds;
    mapRotationCondition_t  conditions[ MAX_MAP_ROTATION_CONDS ];
    S32                     numConditions;
} mapRotationEntry_t;

typedef struct mapRotation_s
{
    UTF8                name[ MAX_QPATH ];
    mapRotationEntry_t  maps[ MAX_MAP_ROTATION_MAPS ];
    S32                 numMaps;
    S32                 currentMap;
} mapRotation_t;

typedef struct mapRotations_s
{
    mapRotation_t   rotations[ MAX_MAP_ROTATIONS ];
    S32             numRotations;
} mapRotations_t;

extern  level_locals_t  level;
extern  gentity_t       g_entities[ MAX_GENTITIES ];

#define FOFS(x) ((U64)&(((gentity_t *)0)->x))

extern vmCvar_t g_allowShare;
extern vmCvar_t g_dedicated;
extern vmCvar_t g_cheats;
extern vmCvar_t g_maxclients;     // allow this many total, including spectators
extern vmCvar_t g_maxGameClients;   // allow this many active
extern vmCvar_t g_restarted;
extern vmCvar_t g_minCommandPeriod;
extern vmCvar_t g_minNameChangePeriod;
extern vmCvar_t g_maxNameChanges;
extern vmCvar_t g_timelimit;
extern vmCvar_t g_suddenDeathTime;
extern vmCvar_t g_suddenDeath;
extern vmCvar_t g_friendlyFire;
extern vmCvar_t g_friendlyFireHumans;
extern vmCvar_t g_friendlyFireAliens;
extern vmCvar_t g_friendlyBuildableFire;
extern vmCvar_t g_password;
extern vmCvar_t g_needpass;
extern vmCvar_t g_gravity;
extern vmCvar_t g_speed;
extern vmCvar_t g_knockback;
extern vmCvar_t g_quadfactor;
extern vmCvar_t g_inactivity;
extern vmCvar_t g_debugMove;
extern vmCvar_t g_debugAlloc;
extern vmCvar_t g_debugDamage;
extern vmCvar_t g_weaponRespawn;
extern vmCvar_t g_weaponTeamRespawn;
extern vmCvar_t g_synchronousClients;
extern vmCvar_t g_motd;
extern vmCvar_t g_warmup;
extern vmCvar_t g_doWarmup;
extern vmCvar_t g_blood;
extern vmCvar_t g_allowVote;
extern vmCvar_t g_voteLimit;
extern vmCvar_t g_suddenDeathVotePercent;
extern vmCvar_t g_suddenDeathVoteDelay;
extern vmCvar_t g_teamAutoJoin;
extern vmCvar_t g_teamForceBalance;
extern vmCvar_t g_banIPs;
extern vmCvar_t g_filterBan;
extern vmCvar_t g_smoothClients;
extern vmCvar_t pmove_fixed;
extern vmCvar_t pmove_msec;
extern vmCvar_t g_rankings;
extern vmCvar_t g_enableDust;
extern vmCvar_t g_enableBreath;
extern vmCvar_t g_singlePlayer;
extern vmCvar_t g_humanBuildPoints;
extern vmCvar_t g_alienBuildPoints;
extern vmCvar_t g_humanBuildQueueTime;
extern vmCvar_t g_alienBuildQueueTime;
extern vmCvar_t g_humanStage;
extern vmCvar_t g_humanCredits;
extern vmCvar_t g_humanMaxStage;
extern vmCvar_t g_humanMaxReachedStage;
extern vmCvar_t g_humanStageThreshold;
extern vmCvar_t g_alienStage;
extern vmCvar_t g_alienCredits;
extern vmCvar_t g_alienMaxStage;
extern vmCvar_t g_alienMaxReachedStage;
extern vmCvar_t g_alienStageThreshold;
extern vmCvar_t g_unlagged;
extern vmCvar_t g_disabledEquipment;
extern vmCvar_t g_disabledClasses;
extern vmCvar_t g_disabledBuildables;
extern vmCvar_t g_markDeconstruct;
extern vmCvar_t g_debugMapRotation;
extern vmCvar_t g_currentMapRotation;
extern vmCvar_t g_currentMap;
extern vmCvar_t g_initialMapRotation;
extern vmCvar_t g_chatTeamPrefix;
extern vmCvar_t g_sayAreaRange;
extern vmCvar_t g_debugVoices;
extern vmCvar_t g_voiceChats;
extern vmCvar_t g_floodMaxDemerits;
extern vmCvar_t g_floodMinTime;
extern vmCvar_t g_shove;
extern vmCvar_t g_mapConfigs;
extern vmCvar_t g_layouts;
extern vmCvar_t g_layoutAuto;
extern vmCvar_t g_emoticonsAllowedInNames;
extern vmCvar_t g_admin;
extern vmCvar_t g_adminLog;
extern vmCvar_t g_adminParseSay;
extern vmCvar_t g_adminNameProtect;
extern vmCvar_t g_adminTempBan;
extern vmCvar_t g_adminPubkeyID;
extern vmCvar_t g_dretchPunt;
extern vmCvar_t g_privateMessages;
extern vmCvar_t g_publicAdminMessages;
extern vmCvar_t bot_developer;
extern vmCvar_t bot_challenge;
extern vmCvar_t bot_thinktime;
extern vmCvar_t bot_minaliens;
extern vmCvar_t bot_minhumans;
extern vmCvar_t bot_nochat;
extern vmCvar_t bot_fastchat;

//
// idGame
//
class idGameLocal : public idGame
{
public:
    idGameLocal();
    ~idGameLocal();
    
    virtual void Init( S32 levelTime, S32 randomSeed, S32 restart );
    virtual void Shutdown( S32 restart );
    virtual void ClientBegin( S32 clientNum );
    virtual UTF8* ClientConnect( S32 clientNum, bool firstTime, bool isBot );
    virtual void ClientThink( S32 clientNum );
    virtual void ClientUserinfoChanged( S32 clientNum );
    virtual void ClientDisconnect( S32 clientNum );
    virtual void ClientCommand( S32 clientNum );
    virtual void RunFrame( S32 levelTime );
    virtual bool ConsoleCommand( void );
    virtual bool AI_CastVisibleFromPos( vec3_t srcpos, S32 srcnum, vec3_t destpos, S32 destnum, bool updateVisPos );
    virtual bool AI_CastCheckAttackAtPos( S32 entnum, S32 enemy, vec3_t pos, bool ducking, bool allowHitWorld ) ;
    //virtual void RetrieveMoveSpeedsFromClient( S32 entnum, UTF8* text );
    //virtual bool GetModelInfo( S32 clientNum, UTF8* modelName, animModelInfo_t** modelInfo );
    virtual S32	BotAIStartFrame( S32 time );
    virtual bool SnapshotCallback( S32 entityNum, S32 clientNum );
    virtual bool BotVisibleFromPos( vec3_t srcorigin, S32 srcnum, vec3_t destorigin, S32 destent, bool dummy );
    virtual bool BotCheckAttackAtPos( S32 entnum, S32 enemy, vec3_t pos, bool ducking, bool allowHitWorld );
public:
    void InitPhysicsForEntity( gentity_t* entity, idVec3 mins, idVec3 maxs );
    void InitPhysicsForEntity( gentity_t* entity, StringEntry qpath );
    S32 DebugLine( vec3_t start, vec3_t end, S32 color );
    void DebugLineDouble( vec3_t start, vec3_t end, S32 color );
    void DeleteDebugLines( void );
    void InitBots( bool restart );
    static void Share_f( gentity_t* ent );
    void PrintSpawnQueue( spawnQueue_t* sq );
    static void UpdatePTRConnection( gclient_t* client );
    static connectionRecord_t* GenerateNewConnection( gclient_t* client );
    void ResetPTRConnections( void );
    static connectionRecord_t* FindConnectionForCode( S32 code );
    static gentity_t* BotAdd( gentity_t* ent, UTF8* name, S32 team, F32* skill, UTF8* botinfo );
    bool BotConnect( S32 clientNum, bool restart );
    void RemoveQueuedBotBegin( S32 clientNum );
    void CheckBotSpawn( void );
    void PrintRotations( void );
    static void AdvanceMapRotation( void );
    static bool StartMapRotation( UTF8* name, bool changeMap );
    static void StopMapRotation( void );
    static bool MapRotationActive( void );
    void InitMapRotations( void );
    static void EntityList_f( void );
    void ReadSessionData( gclient_t* client );
    void InitSessionData( gclient_t* client, UTF8* userinfo );
    static void WriteSessionData( void );
    static team_t TeamFromString( UTF8* str );
    static bool OnSameTeam( gentity_t* ent1, gentity_t* ent2 );
    static void LeaveTeam( gentity_t* self );
    static void ChangeTeam( gentity_t* ent, team_t newTeam );
    static gentity_t* Team_GetLocation( gentity_t* ent );
    static bool Team_GetLocationMsg( gentity_t* ent, UTF8* loc, S32 loclen );
    void TeamplayInfoMessage( gentity_t* ent );
    void CheckTeamStatus( void );
    static void BotBegin( S32 clientNum );
    static void UnlaggedStore( void );
    static void UnlaggedClear( gentity_t* ent );
    static void UnlaggedCalc( S32 time, gentity_t* skipEnt );
    static void UnlaggedOn( gentity_t* attacker, vec3_t muzzle, F32 range );
    static void UnlaggedOff( void );
    static void ClientEndFrame( gentity_t* ent );
    void RunClient( gentity_t* ent );
    static void ScoreboardMessage( gentity_t* client );
    static void MoveClientToIntermission( gentity_t* client );
    static void MapConfigs( StringEntry mapname );
    static void CalculateRanks( void );
    static void FindIntermissionPoint( void );
    void RunThink( gentity_t* ent );
    static void AdminMessage( StringEntry prefix, StringEntry fmt, ... );
    static void LogPrintf( StringEntry fmt, ... );
    static void SendScoreboardMessageToAllClients( void );
    static void Printf( StringEntry fmt, ... );
    static void Error( StringEntry fmt, ... );
    static void Vote( gentity_t* ent, bool voting );
    static void TeamVote( gentity_t* ent, bool voting );
    static void CheckVote( void );
    static void CheckTeamVote( team_t teamnum );
    static void LogExit( StringEntry string );
    static void DemoCommand( demoCommand_t cmd, StringEntry string );
    static S32 TimeTilSuddenDeath( void );
    static void FireWeapon( gentity_t* ent );
    static void FireWeapon2( gentity_t* ent );
    static void FireWeapon3( gentity_t* ent );
    static void AddCreditToClient( gclient_t* client, S16 credit, bool cap );
    static void SetClientViewAngle( gentity_t* ent, vec3_t angle );
    static gentity_t* SelectTremulousSpawnPoint( team_t  team, vec3_t preference, vec3_t origin, vec3_t angles );
    static gentity_t* SelectSpawnPoint( vec3_t avoidPoint, vec3_t origin, vec3_t angles );
    static gentity_t* SelectAlienLockSpawnPoint( vec3_t origin, vec3_t angles );
    static gentity_t* SelectHumanLockSpawnPoint( vec3_t origin, vec3_t angles );
    static void SpawnCorpse( gentity_t* ent );
    static void respawn( gentity_t* ent );
    static void BeginIntermission( void );
    static void ClientSpawn( gentity_t* ent, gentity_t* spawn, vec3_t origin, vec3_t angles );
    static void player_die( gentity_t* self, gentity_t* inflictor, gentity_t* attacker, S32 damage, S32 mod );
    static bool SpotWouldTelefrag( gentity_t* spot );
    static void ForceWeaponChange( gentity_t* ent, weapon_t weapon );
    static void GiveClientMaxAmmo( gentity_t* ent, bool buyingEnergyAmmo );
    static void CalcMuzzlePoint( gentity_t* ent, vec3_t forward, vec3_t right, vec3_t up, vec3_t muzzlePoint );
    static void SnapVectorTowards( vec3_t v, vec3_t to );
    bool CheckVenomAttack( gentity_t* ent );
    void CheckGrabAttack( gentity_t* ent );
    bool CheckPounceAttack( gentity_t* ent );
    void CheckCkitRepair( gentity_t* ent );
    void ChargeAttack( gentity_t* ent, gentity_t* victim );
    void CrushAttack( gentity_t* ent, gentity_t* victim );
    static void UpdateZaps( gentity_t* ent );
    static void TeleportPlayer( gentity_t* player, vec3_t origin, vec3_t angles );
    void RunMover( gentity_t* ent );
    static void Touch_DoorTrigger( gentity_t* ent, gentity_t* other, trace_t* trace );
    void manualTriggerSpectator( gentity_t* trigger, gentity_t* player );
    static void trigger_teleporter_touch( gentity_t* self, gentity_t* other, trace_t* trace );
    void Checktrigger_stages( team_t team, stage_t stage );
    void RunMissile( gentity_t* ent );
    static gentity_t* fire_flamer( gentity_t* self, vec3_t start, vec3_t aimdir );
    static gentity_t* fire_blaster( gentity_t* self, vec3_t start, vec3_t dir );
    static gentity_t* fire_pulseRifle( gentity_t* self, vec3_t start, vec3_t dir );
    static gentity_t* fire_luciferCannon( gentity_t* self, vec3_t start, vec3_t dir, S32 damage, S32 radius, S32 speed );
    static gentity_t* fire_lockblob( gentity_t* self, vec3_t start, vec3_t dir );
    static gentity_t* fire_slowBlob( gentity_t* self, vec3_t start, vec3_t dir );
    static gentity_t* fire_bounceBall( gentity_t* self, vec3_t start, vec3_t dir );
    static gentity_t* fire_hive( gentity_t* self, vec3_t start, vec3_t dir );
    static gentity_t* launch_grenade( gentity_t* self, vec3_t start, vec3_t dir );
    static bool CanDamage( gentity_t* targ, vec3_t origin );
    static void Damage( gentity_t* targ, gentity_t* inflictor, gentity_t* attacker, vec3_t dir, vec3_t point, S32 damage, S32 dflags, S32 mod );
    static bool RadiusDamage( vec3_t origin, gentity_t* attacker, F32 damage, F32 radius, gentity_t* ignore, S32 mod );
    static bool SelectiveRadiusDamage( vec3_t origin, gentity_t* attacker, F32 damage, F32 radius, gentity_t* ignore, S32 mod, S32 team );
    static F32 RewardAttackers( gentity_t* self );
    static void AddScore( gentity_t* ent, S32 score );
    static void LogDestruction( gentity_t* self, gentity_t* actor, S32 mod );
    static void InitDamageLocations( void );
    static bool AHovel_Blocked( gentity_t* hovel, gentity_t* player, bool provideExit );
    static gentity_t* CheckSpawnPoint( S32 spawnNum, vec3_t origin, vec3_t normal, buildable_t spawn, vec3_t spawnOrigin );
    static buildable_t IsPowered( vec3_t origin );
    static bool IsDCCBuilt( void );
    static S32 FindDCC( gentity_t* self );
    static bool FindOvermind( gentity_t* self );
    static bool FindCreep( gentity_t* self );
    static void BuildableThink( gentity_t* ent, S32 msec );
    static bool BuildableRange( vec3_t origin, F32 r, buildable_t buildable );
    static itemBuildError_t CanBuild( gentity_t* ent, buildable_t buildable, S32 distance, vec3_t origin );
    static bool BuildIfValid( gentity_t* ent, buildable_t buildable );
    static void SetBuildableAnim( gentity_t* ent, buildableAnimNumber_t anim, bool force );
    static void SetIdleBuildableAnim( gentity_t* ent, buildableAnimNumber_t anim );
    static void SpawnBuildable( gentity_t* ent, buildable_t buildable );
    void LayoutSave( UTF8* name );
    S32 LayoutList( StringEntry map, UTF8* list, S32 len );
    void LayoutSelect( void );
    void LayoutLoad( void );
    void BaseSelfDestruct( team_t team );
    static void QueueBuildPoints( gentity_t* self );
    static gentity_t* FindBuildable( buildable_t buildable );
    static S32 ParticleSystemIndex( UTF8* name );
    static S32 ShaderIndex( UTF8* name );
    static S32 ModelIndex( UTF8* name );
    static S32 SoundIndex( UTF8* name );
    static void TeamCommand( team_t team, UTF8* cmd );
    static void KillBox( gentity_t* ent );
    static gentity_t* Find( gentity_t* from, S32 fieldofs, StringEntry match );
    static gentity_t* PickTarget( UTF8* targetname );
    static void UseTargets( gentity_t* ent, gentity_t* activator );
    static void SetMovedir( vec3_t angles, vec3_t movedir );
    static void InitGentity( gentity_t* e );
    static gentity_t* Spawn( void );
    static gentity_t* TempEntity( vec3_t origin, S32 event );
    static void Sound( gentity_t* ent, S32 channel, S32 soundIndex );
    static void FreeEntity( gentity_t* e );
    void TouchTriggers( gentity_t* ent );
    static UTF8* vtos( const vec3_t v );
    static void AddPredictableEvent( gentity_t* ent, S32 event, S32 eventParm );
    static void AddEvent( gentity_t* ent, S32 event, S32 eventParm );
    static void BroadcastEvent( S32 event, S32 eventParm );
    static void SetOrigin( gentity_t* ent, vec3_t origin );
    static void AddRemap( StringEntry oldShader, StringEntry newShader, F32 timeOffset );
    static StringEntry BuildShaderStateConfig( void );
    static void TriggerMenu( S32 clientNum, dynMenu_t menu );
    static void TriggerMenu2( S32 clientNum, dynMenu_t menu, S32 arg );
    static void CloseMenus( S32 clientNum );
    static bool Visible( gentity_t* ent1, gentity_t* ent2, S32 contents );
    static gentity_t* ClosestEnt( vec3_t origin, gentity_t** entities, S32 numEntities );
    static bool SpawnString( StringEntry key, StringEntry defaultString, UTF8** out );
    static bool SpawnFloat( StringEntry key, StringEntry defaultString, F32* out );
    static bool SpawnInt( StringEntry key, StringEntry defaultString, S32* out );
    static bool SpawnVector( StringEntry key, StringEntry defaultString, F32* out );
    void SpawnEntitiesFromString( void );
    UTF8* NewString( StringEntry string );
    static void StopFromFollowing( gentity_t* ent );
    static void StopFollowing( gentity_t* ent );
    static void FollowLockView( gentity_t* ent );
    static bool FollowNewClient( gentity_t* ent, S32 dir );
    static void ToggleFollow( gentity_t* ent );
    static void MatchOnePlayer( S32* plist, S32 num, UTF8* err, S32 len );
    static S32 ClientNumberFromString( UTF8* s );
    static S32 ClientNumbersFromString( UTF8* s, S32* plist, S32 max );
    static S32 SayArgc( void );
    static bool SayArgv( S32 n, UTF8* buffer, S32 bufferLength );
    static UTF8* SayConcatArgs( S32 start );
    static void DecolorString( UTF8* in, UTF8* out, S32 len );
    static void SanitiseString( UTF8* in, UTF8* out, S32 len );
    static void PrivateMessage_f( gentity_t* ent );
    static void AdminMessage_f( gentity_t* ent );
    S32 FloodLimited( gentity_t* ent );
    void Physics( gentity_t* ent, S32 msec );
    void InitSpawnQueue( spawnQueue_t* sq );
    static S32 GetSpawnQueueLength( spawnQueue_t* sq );
    static S32 PopSpawnQueue( spawnQueue_t* sq );
    static S32 PeekSpawnQueue( spawnQueue_t* sq );
    static bool SearchSpawnQueue( spawnQueue_t* sq, S32 clientNum );
    static bool PushSpawnQueue( spawnQueue_t* sq, S32 clientNum );
    static bool RemoveFromSpawnQueue( spawnQueue_t* sq, S32 clientNum );
    static S32 GetPosInSpawnQueue( spawnQueue_t* sq, S32 clientNum );
    static void poisonCloud( gentity_t* ent );
    static void WideTrace( trace_t* tr, gentity_t* ent, F32 range, F32 width, F32 height, gentity_t** target );
    static void BounceProjectile( vec3_t start, vec3_t impact, vec3_t dir, vec3_t endout );
    static void SnapVectorNormal( vec3_t v, vec3_t normal );
    static void BloodSpurt( gentity_t* attacker, gentity_t* victim, trace_t* tr );
    static void WideBloodSpurt( gentity_t* attacker, gentity_t* victim, trace_t* tr );
    static void meleeAttack( gentity_t* ent, F32 range, F32 width, F32 height, S32 damage, meansOfDeath_t mod );
    static void bulletFire( gentity_t* ent, F32 spread, S32 damage, S32 mod );
    static void ShotgunPattern( vec3_t origin, vec3_t origin2, S32 seed, gentity_t* ent );
    static void shotgunFire( gentity_t* ent );
    static void massDriverFire( gentity_t* ent );
    static void lockBlobLauncherFire( gentity_t* ent );
    static void hiveFire( gentity_t* ent );
    static void blasterFire( gentity_t* ent );
    static void pulseRifleFire( gentity_t* ent );
    static void flamerFire( gentity_t* ent );
    static void throwGrenade( gentity_t* ent );
    static void lasGunFire( gentity_t* ent );
    static void painSawFire( gentity_t* ent );
    static void LCChargeFire( gentity_t* ent, bool secondary );
    static void teslaFire( gentity_t* self );
    static void cancelBuildFire( gentity_t* ent );
    static void buildFire( gentity_t* ent, dynMenu_t menu );
    static void slowBlobFire( gentity_t* ent );
    static void bounceBallFire( gentity_t* ent );
    static S32 FindConfigstringIndex( UTF8* name, S32 start, S32 max, bool create );
    static UTF8* GetBotInfoByName( StringEntry name );
    static bool ListChars( gentity_t* ent, S32 skiparg );
    static gentity_t* AddRandomBot( gentity_t* ent, S32 team, UTF8* name, F32* skill );
    static void BotDel( gentity_t* ent, S32 clientNum );
    static void BotRemoveAll( gentity_t* ent );
    static void DamageFeedback( gentity_t* player );
    static void WorldEffects( gentity_t* ent );
    static void SetClientSound( gentity_t* ent );
    S32 GetClientMass( gentity_t* ent );
    void ClientShove( gentity_t* ent, gentity_t* victim );
    void ClientImpacts( gentity_t* ent, pmove_t* pm );
    void SpectatorThink( gentity_t* ent, usercmd_t* ucmd );
    bool ClientInactivityTimer( gclient_t* client );
    void ClientTimerActions( gentity_t* ent, S32 msec );
    void ClientIntermissionThink( gclient_t* client );
    void ClientEvents( gentity_t* ent, S32 oldEventSequence );
    static void SendPendingPredictableEvents( playerState_t* ps );
    void UnlaggedDetectCollisions( gentity_t* ent );
    void CheckZap( gentity_t* ent );
    void ClientThink_real( gentity_t* ent );
    static void SpectatorClientEndFrame( gentity_t* ent );
    S32 RemoveRandomBot( S32 team );
    S32 CountHumanPlayers( S32 team );
    S32 CountBotPlayers( S32 team );
    void CheckMinimumPlayers( void );
    S32 ParseInfos( UTF8* buf, S32 max, UTF8* infos[] );
    void LoadBotsFromFile( UTF8* filename );
    void LoadBots( void );
    static void InitTrigger( gentity_t* self );
    static void multi_wait( gentity_t* ent );
    static void multi_trigger( gentity_t* ent, gentity_t* activator );
    static void Use_Multi( gentity_t* ent, gentity_t* other, gentity_t* activator );
    static void Touch_Multi( gentity_t* self, gentity_t* other, trace_t* trace );
    static bool SpawnVector4( StringEntry key, StringEntry defaultString, F32* out );
    static void SP_trigger_multiple( gentity_t* ent );
    static void trigger_always_think( gentity_t* ent );
    static void SP_trigger_always( gentity_t* ent );
    static void trigger_push_touch( gentity_t* self, gentity_t* other, trace_t* trace );
    static void AimAtTarget( gentity_t* self );
    static void SP_trigger_push( gentity_t* self );
    static void Use_target_push( gentity_t* self, gentity_t* other, gentity_t* activator );
    static void SP_target_push( gentity_t* self );
    static void trigger_teleporter_use( gentity_t* ent, gentity_t* other, gentity_t* activator );
    static void SP_trigger_teleport( gentity_t* self );
    static void hurt_use( gentity_t* self, gentity_t* other, gentity_t* activator );
    static void hurt_touch( gentity_t* self, gentity_t* other, trace_t* trace );
    static void SP_trigger_hurt( gentity_t* self );
    static void func_timer_think( gentity_t* self );
    static void func_timer_use( gentity_t* self, gentity_t* other, gentity_t* activator );
    static void SP_func_timer( gentity_t* self );
    static void trigger_stage_use( gentity_t* self, gentity_t* other, gentity_t* activator );
    static void SP_trigger_stage( gentity_t* self );
    static void trigger_win( gentity_t* self, gentity_t* other, gentity_t* activator );
    static void SP_trigger_win( gentity_t* self );
    static bool trigger_buildable_match( gentity_t* self, gentity_t* activator );
    static void trigger_buildable_trigger( gentity_t* self, gentity_t* activator );
    static void trigger_buildable_touch( gentity_t* ent, gentity_t* other, trace_t* trace );
    static void trigger_buildable_use( gentity_t* ent, gentity_t* other, gentity_t* activator );
    static void SP_trigger_buildable( gentity_t* self );
    static bool trigger_class_match( gentity_t* self, gentity_t* activator );
    static void trigger_class_trigger( gentity_t* self, gentity_t* activator );
    static void trigger_class_touch( gentity_t* ent, gentity_t* other, trace_t* trace );
    static void trigger_class_use( gentity_t* ent, gentity_t* other, gentity_t* activator );
    static void SP_trigger_class( gentity_t* self );
    static bool trigger_equipment_match( gentity_t* self, gentity_t* activator );
    static void trigger_equipment_trigger( gentity_t* self, gentity_t* activator );
    static void trigger_equipment_touch( gentity_t* ent, gentity_t* other, trace_t* trace );
    static void trigger_equipment_use( gentity_t* ent, gentity_t* other, gentity_t* activator );
    static void SP_trigger_equipment( gentity_t* self );
    static void trigger_gravity_touch( gentity_t* ent, gentity_t* other, trace_t* trace );
    static void trigger_gravity_use( gentity_t* ent, gentity_t* other, gentity_t* activator );
    static void SP_trigger_gravity( gentity_t* self );
    static void trigger_heal_use( gentity_t* self, gentity_t* other, gentity_t* activator );
    static void trigger_heal_touch( gentity_t* self, gentity_t* other, trace_t* trace );
    static void SP_trigger_heal( gentity_t* self );
    static void trigger_ammo_touch( gentity_t* self, gentity_t* other, trace_t* trace );
    static void SP_trigger_ammo( gentity_t* self );
    static void PrintMsg( gentity_t* ent, StringEntry fmt, ... );
    static S32 SortClients( const void* a, const void* b );
    static void Think_Target_Delay( gentity_t* ent );
    static void Use_Target_Delay( gentity_t* ent, gentity_t* other, gentity_t* activator );
    static void SP_target_delay( gentity_t* ent );
    static void Use_Target_Score( gentity_t* ent, gentity_t* other, gentity_t* activator );
    static void SP_target_score( gentity_t* ent );
    static void Use_Target_Print( gentity_t* ent, gentity_t* other, gentity_t* activator );
    static void SP_target_print( gentity_t* ent );
    static void Use_Target_Speaker( gentity_t* ent, gentity_t* other, gentity_t* activator );
    static void SP_target_speaker( gentity_t* ent );
    static void target_teleporter_use( gentity_t* self, gentity_t* other, gentity_t* activator );
    static void SP_target_teleporter( gentity_t* self );
    static void target_relay_use( gentity_t* self, gentity_t* other, gentity_t* activator );
    static void SP_target_relay( gentity_t* self );
    static void target_kill_use( gentity_t* self, gentity_t* other, gentity_t* activator );
    static void SP_target_kill( gentity_t* self );
    static void SP_target_position( gentity_t* self );
    static void target_location_linkup( gentity_t* ent );
    static void SP_target_location( gentity_t* self );
    static void target_rumble_think( gentity_t* self );
    static void target_rumble_use( gentity_t* self, gentity_t* other, gentity_t* activator );
    static void SP_target_rumble( gentity_t* self );
    static void target_alien_win_use( gentity_t* self, gentity_t* other, gentity_t* activator );
    static void SP_target_alien_win( gentity_t* self );
    static void target_human_win_use( gentity_t* self, gentity_t* other, gentity_t* activator );
    static void SP_target_human_win( gentity_t* self );
    static void target_hurt_use( gentity_t* self, gentity_t* other, gentity_t* activator );
    static void SP_target_hurt( gentity_t* self );
    gclient_t* ClientForString( UTF8* s );
    static UTF8* ConcatArgs( S32 start );
    static void Svcmd_Status_f( void );
    static void Svcmd_ForceTeam_f( void );
    static void Svcmd_LayoutSave_f( void );
    static void Svcmd_LayoutLoad_f( void );
    static void Svcmd_AdmitDefeat_f( void );
    static void Svcmd_TeamWin_f( void );
    static void Svcmd_Evacuation_f( void );
    static void Svcmd_MapRotation_f( void );
    static void Svcmd_TeamMessage_f( void );
    static void Svcmd_SendMessage( void );
    static void Svcmd_CenterPrint_f( void );
    static void Svcmd_EjectClient_f( void );
    static void Svcmd_DumpUser_f( void );
    static void Svcmd_PrintQueue_f( void );
    static void Svcmd_BotlibSetVariable_f( void );
    static void Svcmd_MessageWrapper( void );
    bool CallSpawn( gentity_t* ent );
    void ParseField( StringEntry key, StringEntry value, gentity_t* ent );
    void SpawnGEntityFromSpawnVars( void );
    UTF8* AddSpawnVarToken( StringEntry string );
    bool ParseSpawnVars( void );
    static void SP_worldspawn( void );
    static void WriteClientSessionData( gclient_t* client );
    static bool CheckForUniquePTRC( S32 code );
    void Bounce( gentity_t* ent, trace_t* trace );
    static gentity_t* TestEntityPosition( gentity_t* ent );
    void CreateRotationMatrix( vec3_t angles, vec3_t matrix[3] );
    void TransposeMatrix( vec3_t matrix[3], vec3_t transpose[3] );
    void RotatePoint( vec3_t point, vec3_t matrix[3] );
    bool TryPushingEntity( gentity_t* check, gentity_t* pusher, vec3_t move, vec3_t amove );
    bool MoverPush( gentity_t* pusher, vec3_t move, vec3_t amove, gentity_t** obstacle );
    void MoverTeam( gentity_t* ent );
    static void SetMoverState( gentity_t* ent, moverState_t moverState, S32 time );
    static void MatchTeam( gentity_t* teamLeader, S32 moverState, S32 time );
    static void ReturnToPos1( gentity_t* ent );
    static void ReturnToApos1( gentity_t* ent );
    static void Think_ClosedModelDoor( gentity_t* ent );
    static void Think_CloseModelDoor( gentity_t* ent );
    static void Think_OpenModelDoor( gentity_t* ent );
    static void Reached_BinaryMover( gentity_t* ent );
    static void Use_BinaryMover( gentity_t* ent, gentity_t* other, gentity_t* activator );
    static void InitMover( gentity_t* ent );
    static void InitRotator( gentity_t* ent );
    static void Blocked_Door( gentity_t* ent, gentity_t* other );
    static void Touch_DoorTriggerSpectator( gentity_t* ent, gentity_t* other, trace_t* trace );
    static void manualDoorTriggerSpectator( gentity_t* door, gentity_t* player );
    static void Think_SpawnNewDoorTrigger( gentity_t* ent );
    static void Think_MatchTeam( gentity_t* ent );
    static void SP_func_door( gentity_t* ent );
    static void SP_func_door_rotating( gentity_t* ent );
    static void SP_func_door_model( gentity_t* ent );
    static void Touch_Plat( gentity_t* ent, gentity_t* other, trace_t* trace );
    static void Touch_PlatCenterTrigger( gentity_t* ent, gentity_t* other, trace_t* trace );
    static void SpawnPlatTrigger( gentity_t* ent );
    static void SP_func_plat( gentity_t* ent );
    static void Touch_Button( gentity_t* ent, gentity_t* other, trace_t* trace );
    static void SP_func_button( gentity_t* ent );
    static void Think_BeginMoving( gentity_t* ent );
    static void Reached_Train( gentity_t* ent );
    static void Start_Train( gentity_t* ent, gentity_t* other, gentity_t* activator );
    static void Stop_Train( gentity_t* ent, gentity_t* other, gentity_t* activator );
    static void Use_Train( gentity_t* ent, gentity_t* other, gentity_t* activator );
    static void Think_SetupTrainTargets( gentity_t* ent );
    static void SP_path_corner( gentity_t* self );
    static void Blocked_Train( gentity_t* self, gentity_t* other );
    static void SP_func_train( gentity_t* self );
    static void SP_func_static( gentity_t* ent );
    static void SP_func_rotating( gentity_t* ent );
    static void SP_func_bobbing( gentity_t* ent );
    static void SP_func_pendulum( gentity_t* ent );
    static void BounceMissile( gentity_t* ent, trace_t* trace );
    static void ExplodeMissile( gentity_t* ent );
    static void MissileImpact( gentity_t* ent, trace_t* trace );
    static void AHive_SearchAndDestroy( gentity_t* self );
    static void SP_info_null( gentity_t* self );
    static void SP_info_notnull( gentity_t* self );
    static void SP_light( gentity_t* self );
    static void SP_misc_teleporter_dest( gentity_t* ent );
    static void SP_misc_model( gentity_t* ent );
    static void locateCamera( gentity_t* ent );
    static void SP_misc_portal_surface( gentity_t* ent );
    static void SP_misc_portal_camera( gentity_t* ent );
    static void SP_toggle_particle_system( gentity_t* self );
    static void SP_use_particle_system( gentity_t* self, gentity_t* other, gentity_t* activator );
    static void SP_misc_particle_system( gentity_t* self );
    static void SP_use_anim_model( gentity_t* self, gentity_t* other, gentity_t* activator );
    static void SP_misc_anim_model( gentity_t* self );
    static void SP_use_light_flare( gentity_t* self, gentity_t* other, gentity_t* activator );
    static void findEmptySpot( vec3_t origin, F32 radius, vec3_t spot );
    static void SP_misc_light_flare( gentity_t* self );
    void* Alloc( S32 size );
    void Free( void* ptr );
    void InitMemory( void );
    void DefragmentMemory( void );
    bool MapExists( UTF8* name );
    bool RotationExists( UTF8* name );
    bool ParseMapCommandSection( mapRotationEntry_t* mre, UTF8** text_p );
    bool ParseMapRotation( mapRotation_t* mr, UTF8** text_p );
    bool ParseMapRotationFile( StringEntry fileName );
    static S32* GetCurrentMapArray( void );
    static void SetCurrentMap( S32 currentMap, S32 rotation );
    static S32 GetCurrentMap( S32 rotation );
    static void IssueMapChange( S32 rotation );
    static mapConditionType_t ResolveConditionDestination( S32* n, UTF8* name );
    static bool EvaluateMapCondition( mapRotationCondition_t* mrc );
    void FindTeams( void );
    void RegisterCvars( void );
    void UpdateCvars( void );
    static void ClearVotes( void );
    static S32 SortRanks( const void* a, const void* b );
    void SpawnClients( team_t team );
    void CountSpawns( void );
    void DemoSetClient( void );
    void DemoRemoveClient( void );
    void DemoSetStage( void );
    void CalculateAvgPlayers( void );
    void CalculateStages( void );
    void CalculateBuildPoints( void );
    static void ExitLevel( void );
    static void SendGameStat( team_t team );
    static void CheckIntermissionExit( void );
    bool ScoreIsTied( void );
    static void CheckExitRules( void );
    void CheckCvars( void );
    void CheckDemo( void );
    void EvaluateAcceleration( gentity_t* ent, S32 msec );
    static void LookAtKiller( gentity_t* self, gentity_t* inflictor, gentity_t* attacker );
    static S32 ParseDmgScript( damageRegion_t* regions, UTF8* buf );
    static F32 GetRegionDamageModifier( gentity_t* targ, S32 _class, S32 piece );
    static F32 GetNonLocDamageModifier( gentity_t* targ, S32 _class );
    static F32 GetPointDamageModifier( gentity_t* targ, damageRegion_t* regions, S32 len, F32 angle, F32 height );
    static F32 CalcDamageModifier( vec3_t point, gentity_t* targ, gentity_t* attacker, S32 _class, S32 dflags );
    static void Cmd_Give_f( gentity_t* ent );
    static void Cmd_God_f( gentity_t* ent );
    static void Cmd_Notarget_f( gentity_t* ent );
    static void Cmd_Noclip_f( gentity_t* ent );
    static void Cmd_LevelShot_f( gentity_t* ent );
    static void Cmd_Kill_f( gentity_t* ent );
    static void Cmd_Team_f( gentity_t* ent );
    static void SayTo( gentity_t* ent, gentity_t* other, S32 mode, S32 color, StringEntry name, StringEntry message );
    static void Say( gentity_t* ent, gentity_t* target, S32 mode, StringEntry chatText );
    static void Cmd_SayArea_f( gentity_t* ent );
    static void Cmd_Say_f( gentity_t* ent );
    static void Cmd_Tell_f( gentity_t* ent );
    static void Cmd_VSay_f( gentity_t* ent );
    static void Cmd_Where_f( gentity_t* ent );
    static void Cmd_CallVote_f( gentity_t* ent );
    static void Cmd_Vote_f( gentity_t* ent );
    static void Cmd_CallTeamVote_f( gentity_t* ent );
    static void Cmd_TeamVote_f( gentity_t* ent );
    static void Cmd_SetViewpos_f( gentity_t* ent );
    static bool RoomForClassChange( gentity_t* ent, class_t _class, vec3_t newOrigin );
    static void Cmd_Class_f( gentity_t* ent );
    static void Cmd_Destroy_f( gentity_t* ent );
    static void Cmd_ActivateItem_f( gentity_t* ent );
    static void Cmd_DeActivateItem_f( gentity_t* ent );
    static void Cmd_ToggleItem_f( gentity_t* ent );
    static void Cmd_Buy_f( gentity_t* ent );
    static void Cmd_Sell_f( gentity_t* ent );
    static void Cmd_Build_f( gentity_t* ent );
    static void Cmd_Reload_f( gentity_t* ent );
    static void Cmd_Follow_f( gentity_t* ent );
    static void Cmd_FollowCycle_f( gentity_t* ent );
    static void Cmd_PTRCVerify_f( gentity_t* ent );
    static void Cmd_PTRCRestore_f( gentity_t* ent );
    static void Cmd_Ignore_f( gentity_t* ent );
    static void Cmd_Test_f( gentity_t* ent );
    static void Cmd_Damage_f( gentity_t* ent );
    static void EditPlayerInventory( gentity_t* ent );
    static void Cmd_Pubkey_f( gentity_t* ent );
    static void Cmd_Pubkey_Identify_f( gentity_t* ent );
    static void Cmd_EditBotInv_f( gentity_t* ent );
    static bool NonSegModel( StringEntry filename );
    static void ClientCleanName( StringEntry in, UTF8* out, S32 outSize );
    static bool IsEmoticon( StringEntry s, bool* escaped );
    static void BodySink( gentity_t* ent );
    static void BodyFree( gentity_t* ent );
    static gentity_t* SelectSpectatorSpawnPoint( vec3_t origin, vec3_t angles );
    static gentity_t* SelectInitialSpawnPoint( vec3_t origin, vec3_t angles );
    static gentity_t* SelectHumanSpawnPoint( vec3_t preference );
    static gentity_t* SelectAlienSpawnPoint( vec3_t preference );
    static gentity_t* SelectRandomFurthestSpawnPoint( vec3_t avoidPoint, vec3_t origin, vec3_t angles );
    static gentity_t* SelectRandomDeathmatchSpawnPoint( void );
    static gentity_t* SelectNearestDeathmatchSpawnPoint( vec3_t from );
    static void SP_info_human_intermission( gentity_t* ent );
    static void SP_info_alien_intermission( gentity_t* ent );
    static void SP_info_player_intermission( gentity_t* ent );
    static void SP_info_player_start( gentity_t* ent );
    static void SP_info_player_deathmatch( gentity_t* ent );
    static void Svcmd_GameMem_f( void );
    static S32 NumberOfDependants( gentity_t* self );
    static bool FindPower( gentity_t* self );
    static gentity_t* PowerEntityForPoint( vec3_t origin );
    static bool FindRepeater( gentity_t* self );
    static gentity_t* RepeaterEntityForPoint( vec3_t origin );
    static bool IsCreepHere( vec3_t origin );
    static void CreepSlow( gentity_t* self );
    static void nullDieFunction( gentity_t* self, gentity_t* inflictor, gentity_t* attacker, S32 damage, S32 mod );
    static void AGeneric_CreepRecede( gentity_t* self );
    static void AGeneric_Blast( gentity_t* self );
    static void AGeneric_Die( gentity_t* self, gentity_t* inflictor, gentity_t* attacker, S32 damage, S32 mod );
    static void AGeneric_CreepCheck( gentity_t* self );
    static void AGeneric_Think( gentity_t* self );
    static void AGeneric_Pain( gentity_t* self, gentity_t* attacker, S32 damage );
    static void ASpawn_Die( gentity_t* self, gentity_t* inflictor, gentity_t* attacker, S32 damage, S32 mod );
    static void ASpawn_Think( gentity_t* self );
    static void AOvermind_Think( gentity_t* self );
    static void ABarricade_Pain( gentity_t* self, gentity_t* attacker, S32 damage );
    static void ABarricade_Shrink( gentity_t* self, bool shrink );
    static void ABarricade_Die( gentity_t* self, gentity_t* inflictor, gentity_t* attacker, S32 damage, S32 mod );
    static void ABarricade_Think( gentity_t* self );
    static void ABarricade_Touch( gentity_t* self, gentity_t* other, trace_t* trace );
    static void AAcidTube_Think( gentity_t* self );
    static bool AHive_CheckTarget( gentity_t* self, gentity_t* enemy );
    static void AHive_Think( gentity_t* self );
    static void AHive_Pain( gentity_t* self, gentity_t* attacker, S32 damage );
    static void AHive_Die( gentity_t* self, gentity_t* inflictor, gentity_t* attacker, S32 damage, S32 mod );
    static bool APropHovel_Blocked( vec3_t origin, vec3_t angles, vec3_t normal, gentity_t* player );
    static void AHovel_Use( gentity_t* self, gentity_t* other, gentity_t* activator );
    static void AHovel_Think( gentity_t* self );
    static void AHovel_Die( gentity_t* self, gentity_t* inflictor, gentity_t* attacker, S32 damage, S32 mod );
    static void ABooster_Touch( gentity_t* self, gentity_t* other, trace_t* trace );
    static void ATrapper_FireOnEnemy( gentity_t* self, S32 firespeed, F32 range );
    static bool ATrapper_CheckTarget( gentity_t* self, gentity_t* target, S32 range );
    static void ATrapper_FindEnemy( gentity_t* ent, S32 range );
    static void ATrapper_Think( gentity_t* self );
    static void HRepeater_Think( gentity_t* self );
    static void HRepeater_Use( gentity_t* self, gentity_t* other, gentity_t* activator );
    static void HReactor_Think( gentity_t* self );
    static void HArmoury_Activate( gentity_t* self, gentity_t* other, gentity_t* activator );
    static void HArmoury_Think( gentity_t* self );
    static void HDCC_Think( gentity_t* self );
    static void HMedistat_Die( gentity_t* self, gentity_t* inflictor, gentity_t* attacker, S32 damage, S32 mod );
    static void HMedistat_Think( gentity_t* self );
    static bool HMGTurret_CheckTarget( gentity_t* self, gentity_t* target, bool los_check );
    static bool HMGTurret_TrackEnemy( gentity_t* self );
    static void HMGTurret_FindEnemy( gentity_t* self );
    static void HMGTurret_Think( gentity_t* self );
    static void HTeslaGen_Think( gentity_t* self );
    static void HSpawn_Disappear( gentity_t* self );
    static void HSpawn_Blast( gentity_t* self );
    static void HSpawn_Die( gentity_t* self, gentity_t* inflictor, gentity_t* attacker, S32 damage, S32 mod );
    static void HSpawn_Think( gentity_t* self );
    void BuildableTouchTriggers( gentity_t* ent );
    static void FreeMarkedBuildables( gentity_t* deconner );
    static itemBuildError_t SufficientBPAvailable( buildable_t buildable, vec3_t origin );
    static bool BuildablesIntersect( buildable_t a, vec3_t originA, buildable_t b, vec3_t originB );
    static void SetBuildableLinkState( bool link );
    static void SetBuildableMarkedLinkState( bool link );
    static gentity_t* Build( gentity_t* builder, buildable_t buildable, vec3_t origin, vec3_t angles );
    static void FinishSpawningBuildable( gentity_t* ent );
    static void LayoutBuildItem( buildable_t buildable, vec3_t origin, vec3_t angles, vec3_t origin2, vec3_t angles2 );
};

extern idGameLocal gameLocal;

#endif //!__G_LOCAL_H__
