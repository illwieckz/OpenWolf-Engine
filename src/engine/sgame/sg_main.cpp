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
// File name:   sg_main.cpp
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#include <sgame/sg_precompiled.h>

level_locals_t  level;

typedef struct
{
    vmCvar_t* vmCvar;
    UTF8* cvarName;
    UTF8* defaultString;
    S32 cvarFlags;
    S32 modificationCount;  // for tracking changes
    bool trackChange;       // track this variable, and announce if changed
} cvarTable_t;

gentity_t g_entities[ MAX_GENTITIES ];
gclient_t g_clients[ MAX_CLIENTS ];

vmCvar_t  g_allowShare;
vmCvar_t  g_fraglimit;
vmCvar_t  g_timelimit;
vmCvar_t  g_suddenDeathTime;
vmCvar_t  g_suddenDeath;
vmCvar_t  g_friendlyFire;
vmCvar_t  g_friendlyFireAliens;
vmCvar_t  g_friendlyFireHumans;
vmCvar_t  g_friendlyBuildableFire;
vmCvar_t  g_password;
vmCvar_t  g_needpass;
vmCvar_t  g_maxclients;
vmCvar_t  g_maxGameClients;
vmCvar_t  g_dedicated;
vmCvar_t  g_speed;
vmCvar_t  g_gravity;
vmCvar_t  g_cheats;
vmCvar_t  g_demoState;
vmCvar_t  g_knockback;
vmCvar_t  g_quadfactor;
vmCvar_t  g_inactivity;
vmCvar_t  g_debugMove;
vmCvar_t  g_debugAlloc;
vmCvar_t  g_debugDamage;
vmCvar_t  g_weaponRespawn;
vmCvar_t  g_weaponTeamRespawn;
vmCvar_t  g_motd;
vmCvar_t  g_synchronousClients;
vmCvar_t  g_warmup;
vmCvar_t  g_doWarmup;
vmCvar_t  g_restarted;
vmCvar_t  g_logFile;
vmCvar_t  g_logFileSync;
vmCvar_t  g_blood;
vmCvar_t  g_podiumDist;
vmCvar_t  g_podiumDrop;
vmCvar_t  g_allowVote;
vmCvar_t  g_voteLimit;
vmCvar_t  g_suddenDeathVotePercent;
vmCvar_t  g_suddenDeathVoteDelay;
vmCvar_t  g_teamAutoJoin;
vmCvar_t  g_teamForceBalance;
vmCvar_t  g_banIPs;
vmCvar_t  g_filterBan;
vmCvar_t  g_smoothClients;
vmCvar_t  pmove_fixed;
vmCvar_t  pmove_msec;
vmCvar_t  g_rankings;
vmCvar_t  g_listEntity;
vmCvar_t  g_minCommandPeriod;
vmCvar_t  g_minNameChangePeriod;
vmCvar_t  g_maxNameChanges;
vmCvar_t  g_humanBuildPoints;
vmCvar_t  g_alienBuildPoints;
vmCvar_t  g_humanBuildQueueTime;
vmCvar_t  g_alienBuildQueueTime;
vmCvar_t  g_humanStage;
vmCvar_t  g_humanCredits;
vmCvar_t  g_humanMaxStage;
vmCvar_t  g_humanMaxReachedStage;
vmCvar_t  g_humanStageThreshold;
vmCvar_t  g_alienStage;
vmCvar_t  g_alienCredits;
vmCvar_t  g_alienMaxStage;
vmCvar_t  g_alienMaxReachedStage;
vmCvar_t  g_alienStageThreshold;
vmCvar_t  g_allowStageDowns;
vmCvar_t  g_unlagged;
vmCvar_t  g_disabledEquipment;
vmCvar_t  g_disabledClasses;
vmCvar_t  g_disabledBuildables;
vmCvar_t  g_markDeconstruct;
vmCvar_t  g_debugMapRotation;
vmCvar_t  g_currentMapRotation;
vmCvar_t  g_currentMap;
vmCvar_t  g_initialMapRotation;
vmCvar_t  g_debugVoices;
vmCvar_t  g_voiceChats;
vmCvar_t  g_shove;
vmCvar_t  g_mapConfigs;
vmCvar_t  g_chatTeamPrefix;
vmCvar_t  g_sayAreaRange;
vmCvar_t  g_floodMaxDemerits;
vmCvar_t  g_floodMinTime;
vmCvar_t  g_layouts;
vmCvar_t  g_layoutAuto;
vmCvar_t  g_emoticonsAllowedInNames;
vmCvar_t  g_admin;
vmCvar_t  g_adminLog;
vmCvar_t  g_adminParseSay;
vmCvar_t  g_adminNameProtect;
vmCvar_t  g_adminTempBan;
vmCvar_t  g_dretchPunt;
vmCvar_t  g_privateMessages;
vmCvar_t  g_publicAdminMessages;
vmCvar_t  g_tag;
vmCvar_t  bot_developer;
vmCvar_t  bot_challenge;
vmCvar_t  bot_thinktime;
vmCvar_t  bot_minaliens;
vmCvar_t  bot_minhumans;
vmCvar_t  bot_nochat;
vmCvar_t  bot_fastchat;

idGameLocal gameLocal;
idGame* game = &gameLocal;

static cvarTable_t   gameCvarTable[ ] =
{
    // don't override the cheat state set by the system
    { &g_cheats, "sv_cheats", "", 0, 0, false },
    
    // demo state
    { &g_demoState, "sv_demoState", "", 0, 0, false },
    
    // noset vars
    { NULL, "gamename", GAME_VERSION , CVAR_SERVERINFO | CVAR_ROM, 0, false  },
    { NULL, "gamedate", __DATE__ , CVAR_ROM, 0, false  },
    { &g_restarted, "g_restarted", "0", CVAR_ROM, 0, false  },
    { NULL, "sv_mapname", "", CVAR_SERVERINFO | CVAR_ROM, 0, false  },
    { NULL, "P", "", CVAR_SERVERINFO | CVAR_ROM, 0, false  },
    { NULL, "ff", "0", CVAR_SERVERINFO | CVAR_ROM, 0, false  },
    
    // latched vars
    
    { &g_maxclients, "sv_maxclients", "8", CVAR_SERVERINFO | CVAR_LATCH | CVAR_ARCHIVE, 0, false  },
    
    // change anytime vars
    { &g_maxGameClients, "g_maxGameClients", "0", CVAR_SERVERINFO | CVAR_ARCHIVE, 0, false  },
    
    { &g_timelimit, "timelimit", "0", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_NORESTART, 0, true },
    { &g_suddenDeathTime, "g_suddenDeathTime", "0", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_NORESTART, 0, true },
    { &g_suddenDeath, "g_suddenDeath", "0", CVAR_SERVERINFO | CVAR_NORESTART, 0, true },
    
    { &g_synchronousClients, "g_synchronousClients", "0", CVAR_SYSTEMINFO, 0, false  },
    
    { &g_friendlyFire, "g_friendlyFire", "0", CVAR_ARCHIVE, 0, true  },
    { &g_friendlyFireAliens, "g_friendlyFireAliens", "0", CVAR_ARCHIVE, 0, true  },
    { &g_friendlyFireHumans, "g_friendlyFireHumans", "0", CVAR_ARCHIVE, 0, true  },
    { &g_friendlyBuildableFire, "g_friendlyBuildableFire", "0", CVAR_ARCHIVE, 0, true  },
    
    { &g_teamAutoJoin, "g_teamAutoJoin", "0", CVAR_ARCHIVE  },
    { &g_teamForceBalance, "g_teamForceBalance", "0", CVAR_ARCHIVE  },
    
    { &g_warmup, "g_warmup", "20", CVAR_ARCHIVE, 0, true  },
    { &g_doWarmup, "g_doWarmup", "0", 0, 0, true  },
    { &g_logFile, "g_logFile", "games.log", CVAR_ARCHIVE, 0, false  },
    { &g_logFileSync, "g_logFileSync", "0", CVAR_ARCHIVE, 0, false  },
    
    { &g_password, "g_password", "", CVAR_USERINFO, 0, false  },
    
    { &g_banIPs, "g_banIPs", "", CVAR_ARCHIVE, 0, false  },
    { &g_filterBan, "g_filterBan", "1", CVAR_ARCHIVE, 0, false  },
    
    { &g_needpass, "g_needpass", "0", CVAR_SERVERINFO | CVAR_ROM, 0, false },
    
    { &g_dedicated, "dedicated", "0", 0, 0, false  },
    { &g_allowShare, "g_allowShare", "1", CVAR_ARCHIVE | CVAR_SERVERINFO, 0, false },
    { &g_speed, "g_speed", "320", 0, 0, true  },
    { &g_gravity, "g_gravity", "800", 0, 0, true  },
    { &g_knockback, "g_knockback", "1000", 0, 0, true  },
    { &g_quadfactor, "g_quadfactor", "3", 0, 0, true  },
    { &g_weaponRespawn, "g_weaponrespawn", "5", 0, 0, true  },
    { &g_weaponTeamRespawn, "g_weaponTeamRespawn", "30", 0, 0, true },
    { &g_inactivity, "g_inactivity", "0", 0, 0, true },
    { &g_debugMove, "g_debugMove", "0", 0, 0, false },
    { &g_debugAlloc, "g_debugAlloc", "0", 0, 0, false },
    { &g_debugDamage, "g_debugDamage", "0", 0, 0, false },
    { &g_motd, "g_motd", "", 0, 0, false },
    { &g_blood, "com_blood", "1", 0, 0, false },
    
    { &g_podiumDist, "g_podiumDist", "80", 0, 0, false },
    { &g_podiumDrop, "g_podiumDrop", "70", 0, 0, false },
    
    { &g_allowVote, "g_allowVote", "1", CVAR_ARCHIVE, 0, false },
    { &g_voteLimit, "g_voteLimit", "5", CVAR_ARCHIVE, 0, false },
    { &g_suddenDeathVotePercent, "g_suddenDeathVotePercent", "74", CVAR_ARCHIVE, 0, false },
    { &g_suddenDeathVoteDelay, "g_suddenDeathVoteDelay", "180", CVAR_ARCHIVE, 0, false },
    { &g_listEntity, "g_listEntity", "0", 0, 0, false },
    { &g_minCommandPeriod, "g_minCommandPeriod", "500", 0, 0, false},
    { &g_minNameChangePeriod, "g_minNameChangePeriod", "5", 0, 0, false},
    { &g_maxNameChanges, "g_maxNameChanges", "5", 0, 0, false},
    
    { &g_smoothClients, "g_smoothClients", "1", 0, 0, false},
    { &pmove_fixed, "pmove_fixed", "0", CVAR_SYSTEMINFO, 0, false},
    { &pmove_msec, "pmove_msec", "8", CVAR_SYSTEMINFO, 0, false},
    
    { &g_humanBuildPoints, "g_humanBuildPoints", DEFAULT_HUMAN_BUILDPOINTS, 0, 0, false  },
    { &g_alienBuildPoints, "g_alienBuildPoints", DEFAULT_ALIEN_BUILDPOINTS, 0, 0, false  },
    { &g_humanBuildQueueTime, "g_humanBuildQueueTime", DEFAULT_HUMAN_QUEUE_TIME, 0, 0, false  },
    { &g_alienBuildQueueTime, "g_alienBuildQueueTime", DEFAULT_ALIEN_QUEUE_TIME, 0, 0, false  },
    { &g_humanStage, "g_humanStage", "0", 0, 0, false  },
    { &g_humanCredits, "g_humanCredits", "0", 0, 0, false  },
    { &g_humanMaxStage, "g_humanMaxStage", DEFAULT_HUMAN_MAX_STAGE, 0, 0, false  },
    { &g_humanMaxReachedStage, "g_humanMaxReachedStage", "0", 0, 0, false },
    { &g_humanStageThreshold, "g_humanStageThreshold", DEFAULT_HUMAN_STAGE_THRESH, 0, 0, false  },
    { &g_alienStage, "g_alienStage", "0", 0, 0, false  },
    { &g_alienCredits, "g_alienCredits", "0", 0, 0, false  },
    { &g_alienMaxStage, "g_alienMaxStage", DEFAULT_ALIEN_MAX_STAGE, 0, 0, false  },
    { &g_alienMaxReachedStage, "g_alienMaxReachedStage", "0", 0, 0, false },
    { &g_alienStageThreshold, "g_alienStageThreshold", DEFAULT_ALIEN_STAGE_THRESH, 0, 0, false  },
    
    { &g_unlagged, "g_unlagged", "1", CVAR_SERVERINFO | CVAR_ARCHIVE, 0, false  },
    
    { &g_disabledEquipment, "g_disabledEquipment", "", CVAR_ROM, 0, false  },
    { &g_disabledClasses, "g_disabledClasses", "", CVAR_ROM, 0, false  },
    { &g_disabledBuildables, "g_disabledBuildables", "", CVAR_ROM, 0, false  },
    
    { &g_chatTeamPrefix, "g_chatTeamPrefix", "0", CVAR_ARCHIVE, 0, false  },
    { &g_sayAreaRange, "g_sayAreaRange", "1000.0", CVAR_ARCHIVE, 0, true  },
    
    { &g_floodMaxDemerits, "g_floodMaxDemerits", "5000", CVAR_ARCHIVE, 0, false  },
    { &g_floodMinTime, "g_floodMinTime", "2000", CVAR_ARCHIVE, 0, false  },
    
    { &g_markDeconstruct, "g_markDeconstruct", "1", CVAR_SERVERINFO | CVAR_ARCHIVE, 0, false  },
    
    { &g_debugMapRotation, "g_debugMapRotation", "0", 0, 0, false  },
    { &g_currentMapRotation, "g_currentMapRotation", "-1", 0, 0, false  }, // -1 = NOT_ROTATING
    { &g_currentMap, "g_currentMap", "0", 0, 0, false  },
    { &g_initialMapRotation, "g_initialMapRotation", "", CVAR_ARCHIVE, 0, false  },
    { &g_debugVoices, "g_debugVoices", "0", 0, 0, false  },
    { &g_voiceChats, "g_voiceChats", "1", CVAR_ARCHIVE, 0, false },
    { &g_shove, "g_shove", "0.0", CVAR_ARCHIVE, 0, false  },
    { &g_mapConfigs, "g_mapConfigs", "", CVAR_ARCHIVE, 0, false  },
    { NULL, "g_mapConfigsLoaded", "0", CVAR_ROM, 0, false  },
    
    { &g_layouts, "g_layouts", "", CVAR_LATCH, 0, false  },
    { &g_layoutAuto, "g_layoutAuto", "1", CVAR_ARCHIVE, 0, false  },
    
    { &g_emoticonsAllowedInNames, "g_emoticonsAllowedInNames", "1", CVAR_LATCH | CVAR_ARCHIVE, 0, false  },
    
    { &g_admin, "g_admin", "admin.dat", CVAR_ARCHIVE, 0, false  },
    { &g_adminLog, "g_adminLog", "admin.log", CVAR_ARCHIVE, 0, false  },
    { &g_adminParseSay, "g_adminParseSay", "1", CVAR_ARCHIVE, 0, false  },
    { &g_adminNameProtect, "g_adminNameProtect", "1", CVAR_ARCHIVE, 0, false  },
    { &g_adminTempBan, "g_adminTempBan", "2m", CVAR_ARCHIVE, 0, false  },
    
    { &g_dretchPunt, "g_dretchPunt", "0", CVAR_ARCHIVE, 0, false  },
    
    { &g_privateMessages, "g_privateMessages", "1", CVAR_ARCHIVE, 0, false  },
    { &g_publicAdminMessages, "g_publicAdminMessages", "1", CVAR_ARCHIVE, 0, false  },
    
    { &g_tag, "g_tag", "main", CVAR_INIT, 0, false },
    
    //Start Champ bot cvars
    { &bot_thinktime, "bot_thinktime", "100", CVAR_CHEAT, 0, false  },
    { &bot_minaliens, "bot_minaliens", "0", CVAR_SERVERINFO, 0, false  },
    { &bot_minhumans, "bot_minhumans", "0", CVAR_SERVERINFO, 0, false  },
    { &bot_developer, "bot_developer", "0", CVAR_SERVERINFO, 0, false  },
    { &bot_challenge, "bot_challenge", "0", CVAR_SERVERINFO, 0, false  },
    { &bot_nochat,    "bot_nochat",    "0", CVAR_TEMP, 0, false  },
    { &bot_fastchat,  "bot_fastchat",  "0", CVAR_TEMP, 0, false  },
    //End Champ bot cvars
    
    { &g_rankings, "g_rankings", "0", 0, 0, false}
};

static S32 gameCvarTableSize = sizeof( gameCvarTable ) / sizeof( gameCvarTable[ 0 ] );

bool idGameLocal::SnapshotCallback( S32 entityNum, S32 clientNum )
{
#if 0
    gentity_t* ent = &g_entities[ entityNum ];
    
    if( ent->s.eType == ET_MISSILE )
    {
        if( ent->s.weapon == WP_LANDMINE )
        {
            return G_LandmineSnapshotCallback( entityNum, clientNum );
        }
    }
#endif
    
    return true;
}

bool idGameLocal::AI_CastVisibleFromPos( vec3_t srcpos, S32 srcnum, vec3_t destpos, S32 destnum, bool updateVisPos )
{
    return 0; //AICastVisibleFromPos( srcpos, srcnum, destpos, destnum, updateVisPos );
}

bool idGameLocal::AI_CastCheckAttackAtPos( S32 entnum, S32 enemy, vec3_t pos, bool ducking, bool allowHitWorld )
{
    return 0; //AICastCheckAttackAtPos( entnum, enemy, pos, ducking, allowHitWorld );
}

void idGameLocal::Printf( StringEntry fmt, ... )
{
    va_list argptr;
    UTF8    text[ 1024 ];
    
    va_start( argptr, fmt );
    Q_vsnprintf( text, sizeof( text ), fmt, argptr );
    va_end( argptr );
    
    trap_Print( text );
}

void idGameLocal::Error( StringEntry fmt, ... )
{
    va_list argptr;
    UTF8    text[ 1024 ];
    
    va_start( argptr, fmt );
    Q_vsnprintf( text, sizeof( text ), fmt, argptr );
    va_end( argptr );
    
    trap_Error( text );
}

/*
================
G_FindTeams

Chain together all entities with a matching team field.
Entity teams are used for item groups and multi-entity mover groups.

All but the first will have the FL_TEAMSLAVE flag set and teammaster field set
All but the last will have the teamchain field set to the next one
================
*/
void idGameLocal::FindTeams( void )
{
    gentity_t* e, *e2;
    S32       i, j;
    S32       c, c2;
    
    c = 0;
    c2 = 0;
    
    for( i = 1, e = g_entities + i; i < level.num_entities; i++, e++ )
    {
        if( !e->inuse )
            continue;
            
        if( !e->team )
            continue;
            
        if( e->flags & FL_TEAMSLAVE )
            continue;
            
        e->teammaster = e;
        c++;
        c2++;
        
        for( j = i + 1, e2 = e + 1; j < level.num_entities; j++, e2++ )
        {
            if( !e2->inuse )
                continue;
                
            if( !e2->team )
                continue;
                
            if( e2->flags & FL_TEAMSLAVE )
                continue;
                
            if( !strcmp( e->team, e2->team ) )
            {
                c2++;
                e2->teamchain = e->teamchain;
                e->teamchain = e2;
                e2->teammaster = e;
                e2->flags |= FL_TEAMSLAVE;
                
                // make sure that targets only point at the master
                if( e2->targetname )
                {
                    e->targetname = e2->targetname;
                    e2->targetname = NULL;
                }
            }
        }
    }
    
    Printf( "%i teams with %i entities\n", c, c2 );
}


/*
=================
idGameLocal::RegisterCvars
=================
*/
void idGameLocal::RegisterCvars( void )
{
    S32         i;
    cvarTable_t* cv;
    
    for( i = 0, cv = gameCvarTable; i < gameCvarTableSize; i++, cv++ )
    {
        trap_Cvar_Register( cv->vmCvar, cv->cvarName,
                            cv->defaultString, cv->cvarFlags );
                            
        if( cv->vmCvar )
            cv->modificationCount = cv->vmCvar->modificationCount;
    }
    
    // check some things
    level.warmupModificationCount = g_warmup.modificationCount;
}

/*
=================
idGameLocal::UpdateCvars
=================
*/
void idGameLocal::UpdateCvars( void )
{
    S32         i;
    cvarTable_t* cv;
    
    for( i = 0, cv = gameCvarTable; i < gameCvarTableSize; i++, cv++ )
    {
        if( cv->vmCvar )
        {
            trap_Cvar_Update( cv->vmCvar );
            
            if( cv->modificationCount != cv->vmCvar->modificationCount )
            {
                cv->modificationCount = cv->vmCvar->modificationCount;
                
                if( cv->trackChange )
                {
                    trap_SendServerCommand( -1, va( "print \"Server: %s changed to %s\n\"",
                                                    cv->cvarName, cv->vmCvar->string ) );
                    // update serverinfo in case this cvar is passed to clients indirectly
                    CalculateRanks( );
                }
            }
        }
    }
}

/*
============
idGameLocal::InitPhysicsForEntity
============
*/
void idGameLocal::InitPhysicsForEntity( gentity_t* entity, StringEntry qpath )
{
    // Don't create a duplicate trace model.
    if( entity->r.traceModel != NULL )
    {
        return;
    }
    
    // Allocate the trace model.
    entity->r.traceModel = trap_AllocTraceModel();
    
    // Init the trace model.
    if( entity->mass == 0 )
    {
        entity->mass = 1;
    }
    
    entity->r.traceModel->InitFromModel( qpath, entity->s.origin, entity->s.angles, entity->s.number, entity->mass );
}

/*
============
idGameLocal::InitPhysicsForEntity
============
*/
void idGameLocal::InitPhysicsForEntity( gentity_t* entity, idVec3 mins, idVec3 maxs )
{
    // Don't create a duplicate trace model.
    if( entity->r.traceModel != NULL )
    {
        return;
    }
    
    // Allocate the trace model.
    entity->r.traceModel = trap_AllocTraceModel();
    
    // Init the trace model.
    if( entity->mass == 0 )
    {
        entity->mass = 1;
    }
    
    entity->r.traceModel->Init( entity->s.number, entity->mass, entity->s.origin, entity->s.angles, mins, maxs );
}

/*
=================
idGameLocal::MapConfigs
=================
*/
void idGameLocal::MapConfigs( StringEntry mapname )
{
    if( !g_mapConfigs.string[0] )
        return;
        
    if( trap_Cvar_VariableIntegerValue( "g_mapConfigsLoaded" ) )
        return;
        
    trap_SendConsoleCommand( EXEC_APPEND, va( "exec \"%s/default.cfg\"\n", g_mapConfigs.string ) );
    
    trap_SendConsoleCommand( EXEC_APPEND, va( "exec \"%s/%s.cfg\"\n", g_mapConfigs.string, mapname ) );
    
    trap_Cvar_Set( "g_mapConfigsLoaded", "1" );
}

idGameLocal::idGameLocal( void )
{

}

idGameLocal::~idGameLocal( void )
{

}

/*
============
idGameLocal::Init
============
*/
void idGameLocal::Init( S32 levelTime, S32 randomSeed, S32 restart )
{
    S32 i;
    UTF8 buffer[MAX_CVAR_VALUE_STRING];
    S32 a, b;
    
    srand( randomSeed );
    
    RegisterCvars();
    
    Printf( "------- idGameLocal::Init : Game Initialization -------\n" );
    Printf( "idGameLocal::Init:Gamename: %s\n", GAME_VERSION );
    Printf( "idGameLocal::Init:Gamedate: %s\n", __DATE__ );
    
    bggame->InitMemory();
    
    // set some level globals
    ::memset( &level, 0, sizeof( level ) );
    level.time = levelTime;
    level.startTime = levelTime;
    level.alienStage2Time = level.alienStage3Time =
                                level.humanStage2Time = level.humanStage3Time = level.startTime;
    level.alienStagedownCredits = level.humanStagedownCredits = -1;
    trap_Cvar_VariableStringBuffer( "session", buffer, sizeof( buffer ) );
    sscanf( buffer, "%i %i", &a, &b );
    if( a != trap_Cvar_VariableIntegerValue( "sv_maxclients" ) ||
            b != trap_Cvar_VariableIntegerValue( "sv_democlients" ) )
        level.newSession = true;
        
    level.snd_fry = SoundIndex( "sound/misc/fry.wav" ); // FIXME standing in lava / slime
    
    if( g_logFile.string[0] )
    {
        if( g_logFileSync.integer )
            trap_FS_FOpenFile( g_logFile.string, &level.logFile, FS_APPEND_SYNC );
        else
            trap_FS_FOpenFile( g_logFile.string, &level.logFile, FS_APPEND );
            
        if( !level.logFile )
            Printf( "WARNING: Couldn't open logfile: %s\n", g_logFile.string );
        else
        {
            UTF8 serverinfo[MAX_INFO_STRING];
            qtime_t qt;
            S32 t;
            
            trap_GetServerinfo( serverinfo, sizeof( serverinfo ) );
            
            LogPrintf( "------------------------------------------------------------\n" );
            LogPrintf( "InitGame: %s\n", serverinfo );
            
            t = trap_RealTime( &qt );
            LogPrintf( "RealTime: %04i/%02i/%02i %02i:%02i:%02i\n",
                       qt.tm_year + 1900, qt.tm_mon + 1, qt.tm_mday,
                       qt.tm_hour, qt.tm_min, qt.tm_sec );
                       
        }
    }
    else
    {
        Printf( "Not logging to disk\n" );
    }
    
    UTF8 map[MAX_CVAR_VALUE_STRING] = { "" };
    UTF8 data[255];
    
    trap_Cvar_VariableStringBuffer( "mapname", map, sizeof( map ) );
    MapConfigs( map );
    
    if( trap_Database_RunQuery( va( "INSERT INTO games (startdate,map) VALUES (NOW(),\"%s\")", map ) ) == true )
    {
        trap_Database_FinishQuery();
        trap_Database_RunQuery( "SELECT id FROM games ORDER BY id DESC LIMIT 1" );
        trap_Database_FetchRow();
        trap_Database_FetchFieldByName( "id", data, sizeof( data ) );
        trap_Database_RunQuery( va( "INSERT into mainlog (idaction,typeaction,date) VALUES (%d,\"%d\",NOW())", atoi( data ), 7 ) );
        trap_Database_FinishQuery();
    }
    
    // we're done with g_mapConfigs, so reset this for the next map
    trap_Cvar_Set( "g_mapConfigsLoaded", "0" );
    
    adminLocal.AdminReadconfig( NULL, 0 );
    
    // initialize all entities for this game
    ::memset( g_entities, 0, MAX_GENTITIES * sizeof( g_entities[0] ) );
    level.gentities = g_entities;
    
    // initialize all clients for this game
    level.maxclients = g_maxclients.integer;
    ::memset( g_clients, 0, MAX_CLIENTS * sizeof( g_clients[0] ) );
    level.clients = g_clients;
    
    // set client fields on player ents
    for( i = 0; i < level.maxclients; i++ )
        g_entities[i].client = level.clients + i;
        
    // always leave room for the max number of clients,
    // even if they aren't all used, so numbers inside that
    // range are NEVER anything but clients
    level.num_entities = MAX_CLIENTS;
    
    // let the server system know where the entites are
    trap_LocateGameData( level.gentities, level.num_entities, sizeof( gentity_t ),
                         &level.clients[0].ps, sizeof( level.clients[0] ) );
                         
    level.emoticonCount = bggame->LoadEmoticons( level.emoticons, NULL );
    
    trap_SetConfigstring( CS_INTERMISSION, "0" );
    
    // test to see if a custom buildable layout will be loaded
    LayoutSelect();
    
    // parse the key/value pairs and spawn gentities
    SpawnEntitiesFromString();
    
    // load up a custom building layout if there is one
    LayoutLoad();
    
    // the map might disable some things
    bggame->InitAllowedGameElements();
    
    // general initialization
    FindTeams();
    
    bggame->InitClassConfigs();
    bggame->InitBuildableConfigs();
    InitDamageLocations();
    InitMapRotations();
    InitSpawnQueue( &level.alienSpawnQueue );
    InitSpawnQueue( &level.humanSpawnQueue );
    
    if( g_debugMapRotation.integer )
    {
        PrintRotations();
    }
    
    level.voices = bggame->VoiceInit( );
    bggame->PrintVoices( level.voices, g_debugVoices.integer );
    
    //reset stages
    trap_Cvar_Set( "g_alienStage", va( "%d", S1 ) );
    trap_Cvar_Set( "g_humanStage", va( "%d", S1 ) );
    trap_Cvar_Set( "g_humanMaxReachedStage", va( "%d", S1 ) );
    trap_Cvar_Set( "g_alienMaxReachedStage", va( "%d", S1 ) );
    trap_Cvar_Set( "g_alienCredits", 0 );
    trap_Cvar_Set( "g_humanCredits", 0 );
    trap_Cvar_Set( "g_suddenDeath", 0 );
    level.suddenDeathBeginTime = g_suddenDeathTime.integer * 60000;
    
    Printf( "-----------------------------------\n" );
    
    if( trap_Cvar_VariableIntegerValue( "bot_enable" ) )
    {
        botLocal.BotAISetup( restart );
        botLocal.BotAILoadMap( restart );
        InitBots( restart );
    }
    
    //G_RemapTeamShaders( );
    
    // so the server counts the spawns without a client attached
    CountSpawns( );
    
    ResetPTRConnections( );
}

/*
==================
G_ClearVotes

remove all currently active votes
==================
*/
void idGameLocal::ClearVotes( void )
{
    level.voteTime = 0;
    trap_SetConfigstring( CS_VOTE_TIME, "" );
    level.teamVoteTime[ 0 ] = 0;
    trap_SetConfigstring( CS_TEAMVOTE_TIME, "" );
    level.teamVoteTime[ 1 ] = 0;
    trap_SetConfigstring( CS_TEAMVOTE_TIME + 1, "" );
}

/*
=================
idGameLocal::Shutdown
=================
*/
void idGameLocal::Shutdown( S32 restart )
{
    S32 i, clients;
    UTF8 data[255];
    
    // in case of a map_restart
    ClearVotes( );
    
    Printf( "==== idGameLocal::Shutdown ====\n" );
    
    if( trap_Database_RunQuery( va( "UPDATE games SET enddate=NOW(),reason=\"%d\" ORDER BY id DESC LIMIT 1", level.lastWin ) ) == true )
    {
        trap_Database_FinishQuery();
        trap_Database_RunQuery( "SELECT id FROM games ORDER BY id DESC LIMIT 1" );
        trap_Database_FetchRow();
        trap_Database_FetchFieldByName( "id", data, sizeof( data ) );
        trap_Database_RunQuery( va( "INSERT into mainlog (idaction,typeaction,date) VALUES (%d,\"%d\",NOW())", atoi( data ), 7 ) );
        trap_Database_FinishQuery();
    }
    
    if( level.logFile )
    {
        LogPrintf( "idGameLocal::Shutdown:\n" );
        LogPrintf( "------------------------------------------------------------\n" );
        trap_FS_FCloseFile( level.logFile );
    }
    
    // write all the client session data so we can get it back
    WriteSessionData( );
    
    adminLocal.AdminCleanup( );
    adminLocal.AdminNamelogCleanup( );
    
    level.restarted = false;
    level.surrenderTeam = TEAM_NONE;
    trap_SetConfigstring( CS_WINNER, "" );
    
    // clear all demo clients
    clients = trap_Cvar_VariableIntegerValue( "sv_democlients" );
    for( i = 0; i < clients; i++ )
    {
        trap_SetConfigstring( CS_PLAYERS + i, NULL );
    }
    
    if( trap_Cvar_VariableIntegerValue( "bot_enable" ) )
    {
        botLocal.BotAIShutdown( restart );
    }
}

void Com_Error( S32 level, StringEntry error, ... )
{
    va_list argptr;
    UTF8    text[ 1024 ];
    
    va_start( argptr, error );
    vsprintf( text, error, argptr );
    va_end( argptr );
    
    gameLocal.Error( "%s", text );
}

void Com_Printf( StringEntry msg, ... )
{
    va_list argptr;
    UTF8    text[ 1024 ];
    
    va_start( argptr, msg );
    vsprintf( text, msg, argptr );
    va_end( argptr );
    
    gameLocal.Printf( "%s", text );
}

/*
========================================================================

PLAYER COUNTING / SCORE SORTING

========================================================================
*/


/*
=============
SortRanks
=============
*/
S32 idGameLocal::SortRanks( const void* a, const void* b )
{
    gclient_t* ca, *cb;
    
    ca = &level.clients[ *( S32* )a ];
    cb = &level.clients[ *( S32* )b ];
    
    // then sort by score
    if( ca->ps.persistant[ PERS_SCORE ] > cb->ps.persistant[ PERS_SCORE ] )
        return -1;
    if( ca->ps.persistant[ PERS_SCORE ] < cb->ps.persistant[ PERS_SCORE ] )
        return 1;
    return 0;
}

/*
============
idGameLocal::InitSpawnQueue

Initialise a spawn queue
============
*/
void idGameLocal::InitSpawnQueue( spawnQueue_t* sq )
{
    S32 i;
    
    sq->back = sq->front = 0;
    sq->back = QUEUE_MINUS1( sq->back );
    
    //0 is a valid clientNum, so use something else
    for( i = 0; i < MAX_CLIENTS; i++ )
        sq->clients[ i ] = -1;
}

/*
============
idGameLocal::GetSpawnQueueLength

Return the length of a spawn queue
============
*/
S32 idGameLocal::GetSpawnQueueLength( spawnQueue_t* sq )
{
    S32 length = sq->back - sq->front + 1;
    
    while( length < 0 )
        length += MAX_CLIENTS;
        
    while( length >= MAX_CLIENTS )
        length -= MAX_CLIENTS;
        
    return length;
}

/*
============
idGameLocal::PopSpawnQueue

Remove from front element from a spawn queue
============
*/
S32 idGameLocal::PopSpawnQueue( spawnQueue_t* sq )
{
    S32 clientNum = sq->clients[ sq->front ];
    
    if( GetSpawnQueueLength( sq ) > 0 )
    {
        sq->clients[ sq->front ] = -1;
        sq->front = QUEUE_PLUS1( sq->front );
        StopFollowing( g_entities + clientNum );
        g_entities[ clientNum ].client->ps.pm_flags &= ~PMF_QUEUED;
        
        return clientNum;
    }
    return -1;
}

/*
============
idGameLocal::PeekSpawnQueue

Look at front element from a spawn queue
============
*/
S32 idGameLocal::PeekSpawnQueue( spawnQueue_t* sq )
{
    return sq->clients[ sq->front ];
}

/*
============
idGameLocal::SearchSpawnQueue

Look to see if clientNum is already in the spawnQueue
============
*/
bool idGameLocal::SearchSpawnQueue( spawnQueue_t* sq, S32 clientNum )
{
    S32 i;
    
    for( i = 0; i < MAX_CLIENTS; i++ )
        if( sq->clients[ i ] == clientNum )
            return true;
    return false;
}

/*
============
idGameLocal::PushSpawnQueue

Add an element to the back of the spawn queue
============
*/
bool idGameLocal::PushSpawnQueue( spawnQueue_t* sq, S32 clientNum )
{
    // don't add the same client more than once
    if( SearchSpawnQueue( sq, clientNum ) )
        return false;
        
    sq->back = QUEUE_PLUS1( sq->back );
    sq->clients[ sq->back ] = clientNum;
    
    g_entities[ clientNum ].client->ps.pm_flags |= PMF_QUEUED;
    return true;
}

/*
============
idGameLocal::RemoveFromSpawnQueue

remove a specific client from a spawn queue
============
*/
bool idGameLocal::RemoveFromSpawnQueue( spawnQueue_t* sq, S32 clientNum )
{
    S32 i = sq->front;
    
    if( GetSpawnQueueLength( sq ) )
    {
        do
        {
            if( sq->clients[ i ] == clientNum )
            {
                //and this kids is why it would have
                //been better to use an LL for internal
                //representation
                do
                {
                    sq->clients[ i ] = sq->clients[ QUEUE_PLUS1( i ) ];
                    
                    i = QUEUE_PLUS1( i );
                }
                while( i != QUEUE_PLUS1( sq->back ) );
                
                sq->back = QUEUE_MINUS1( sq->back );
                g_entities[ clientNum ].client->ps.pm_flags &= ~PMF_QUEUED;
                
                return true;
            }
            
            i = QUEUE_PLUS1( i );
        }
        while( i != QUEUE_PLUS1( sq->back ) );
    }
    
    return false;
}

/*
============
idGameLocal::GetPosInSpawnQueue

Get the position of a client in a spawn queue
============
*/
S32 idGameLocal::GetPosInSpawnQueue( spawnQueue_t* sq, S32 clientNum )
{
    S32 i = sq->front;
    
    if( GetSpawnQueueLength( sq ) )
    {
        do
        {
            if( sq->clients[ i ] == clientNum )
            {
                if( i < sq->front )
                    return i + MAX_CLIENTS - sq->front;
                else
                    return i - sq->front;
            }
            
            i = QUEUE_PLUS1( i );
        }
        while( i != QUEUE_PLUS1( sq->back ) );
    }
    
    return -1;
}

/*
============
idGameLocal::PrintSpawnQueue

Print the contents of a spawn queue
============
*/
void idGameLocal::PrintSpawnQueue( spawnQueue_t* sq )
{
    S32 i = sq->front;
    S32 length = GetSpawnQueueLength( sq );
    
    Printf( "l:%d f:%d b:%d    :", length, sq->front, sq->back );
    
    if( length > 0 )
    {
        do
        {
            if( sq->clients[ i ] == -1 )
                Printf( "*:" );
            else
                Printf( "%d:", sq->clients[ i ] );
                
            i = QUEUE_PLUS1( i );
        }
        while( i != QUEUE_PLUS1( sq->back ) );
    }
    
    Printf( "\n" );
}

/*
============
idGameLocal::SpawnClients

Spawn queued clients
============
*/
void idGameLocal::SpawnClients( team_t team )
{
    S32           clientNum;
    gentity_t*     ent, *spawn;
    vec3_t        spawn_origin, spawn_angles;
    spawnQueue_t*  sq = NULL;
    S32           numSpawns = 0;
    
    if( team == TEAM_ALIENS )
    {
        sq = &level.alienSpawnQueue;
        numSpawns = level.numAlienSpawns;
    }
    else if( team == TEAM_HUMANS )
    {
        sq = &level.humanSpawnQueue;
        numSpawns = level.numHumanSpawns;
    }
    
    if( GetSpawnQueueLength( sq ) > 0 && numSpawns > 0 )
    {
        clientNum = PeekSpawnQueue( sq );
        ent = &g_entities[ clientNum ];
        
        if( ( spawn = SelectTremulousSpawnPoint( team, ent->client->pers.lastDeathLocation, spawn_origin, spawn_angles ) ) )
        {
            clientNum = PopSpawnQueue( sq );
            
            if( clientNum < 0 )
                return;
                
            ent = &g_entities[ clientNum ];
            
            ent->client->sess.spectatorState = SPECTATOR_NOT;
            ClientUserinfoChanged( clientNum );
            ClientSpawn( ent, spawn, spawn_origin, spawn_angles );
        }
    }
}

/*
============
G_CountSpawns

Counts the number of spawns for each team
============
*/
void idGameLocal::CountSpawns( void )
{
    S32 i;
    gentity_t* ent;
    
    level.numAlienSpawns = 0;
    level.numHumanSpawns = 0;
    
    for( i = 1, ent = g_entities + i ; i < level.num_entities ; i++, ent++ )
    {
        if( !ent->inuse )
            continue;
            
        if( ent->s.modelindex == BA_A_SPAWN && ent->health > 0 )
            level.numAlienSpawns++;
            
        if( ent->s.modelindex == BA_H_SPAWN && ent->health > 0 )
            level.numHumanSpawns++;
    }
}

/*
============
idGameLocal::TimeTilSuddenDeath
============
*/
S32 idGameLocal::TimeTilSuddenDeath( void )
{
    if( ( !g_suddenDeathTime.integer && level.suddenDeathBeginTime == 0 ) ||
            ( level.suddenDeathBeginTime < 0 ) )
        return 1; // Always some time away
        
    return ( ( level.suddenDeathBeginTime ) - ( level.time - level.startTime ) );
}


#define PLAYER_COUNT_MOD 5.0f

/*
============
G_CalculateBuildPoints

Recalculate the quantity of building points available to the teams
============
*/
void idGameLocal::CalculateBuildPoints( void )
{
    S32         i;
    buildable_t buildable;
    gentity_t*   ent;
    S32         localHTP = level.humanBuildPoints = g_humanBuildPoints.integer,
                localATP = level.alienBuildPoints = g_alienBuildPoints.integer;
                
    // BP queue updates
    while( level.alienBuildPointQueue > 0 &&
            level.alienNextQueueTime < level.time )
    {
        level.alienBuildPointQueue--;
        level.alienNextQueueTime += g_alienBuildQueueTime.integer;
    }
    
    while( level.humanBuildPointQueue > 0 &&
            level.humanNextQueueTime < level.time )
    {
        level.humanBuildPointQueue--;
        level.humanNextQueueTime += g_alienBuildQueueTime.integer;
    }
    
    // Sudden Death checks
    
    // Note: g_suddenDeath sets what is going on and level.suddenDeath
    // trails it to run stuff here. They're only inconsistent for as long
    // as it takes this function to run once and update
    
    // reset if SD was on, but now it's off
    if( !g_suddenDeath.integer && level.suddenDeath )
    {
        level.suddenDeath = false;
        level.suddenDeathWarning = ( timeWarning_t )0;
        level.suddenDeathBeginTime = -1;
        if( ( level.time - level.startTime ) < ( g_suddenDeathTime.integer * 60000 ) )
            level.suddenDeathBeginTime = g_suddenDeathTime.integer * 60000;
        else
            level.suddenDeathBeginTime = -1;
    }
    
    // SD checks and warnings
    if( !level.suddenDeath )
    {
        // check conditions to enter sudden death
        if( !level.warmupTime && ( g_suddenDeath.integer || TimeTilSuddenDeath( ) <= 0 ) )
        {
            // begin sudden death
            localHTP = 0;
            localATP = 0;
            
            if( level.suddenDeathWarning < TW_PASSED )
            {
                LogPrintf( "Beginning Sudden Death\n" );
                trap_SendServerCommand( -1, "cp \"Sudden Death!\"" );
                
                level.suddenDeathBeginTime = level.time;
                level.suddenDeath = true;
                trap_Cvar_Set( "g_suddenDeath", "1" );
                
                level.suddenDeathWarning = TW_PASSED;
            }
        }
        else
        {
            // warn about sudden death
            if( TimeTilSuddenDeath( ) <= 60000 && level.suddenDeathWarning < TW_IMMINENT )
            {
                trap_SendServerCommand( -1, va( "cp \"Sudden Death in %d seconds!\"",
                                                ( S32 )( TimeTilSuddenDeath() / 1000 ) ) );
                level.suddenDeathWarning = TW_IMMINENT;
            }
        }
    }
    else // it is SD already
    {
        localHTP = 0;
        localATP = 0;
    }
    
    level.humanBuildPoints = localHTP - level.humanBuildPointQueue;
    level.alienBuildPoints = localATP - level.alienBuildPointQueue;
    
    level.reactorPresent = false;
    level.overmindPresent = false;
    
    for( i = 1, ent = g_entities + i ; i < level.num_entities ; i++, ent++ )
    {
        if( !ent->inuse )
            continue;
            
        if( ent->s.eType != ET_BUILDABLE )
            continue;
            
        if( ent->s.eFlags & EF_DEAD )
            continue;
            
        buildable = ( buildable_t )ent->s.modelindex;
        
        if( buildable != BA_NONE )
        {
            if( buildable == BA_H_REACTOR && ent->spawned && ent->health > 0 )
                level.reactorPresent = true;
                
            if( buildable == BA_A_OVERMIND && ent->spawned && ent->health > 0 )
                level.overmindPresent = true;
                
            if( bggame->Buildable( buildable )->team == TEAM_HUMANS )
            {
                level.humanBuildPoints -= bggame->Buildable( buildable )->buildPoints;
            }
            else if( bggame->Buildable( buildable )->team == TEAM_ALIENS )
            {
                level.alienBuildPoints -= bggame->Buildable( buildable )->buildPoints;
            }
        }
    }
    
    if( level.humanBuildPoints < 0 )
    {
        localHTP -= level.humanBuildPoints;
        level.humanBuildPoints = 0;
    }
    
    if( level.alienBuildPoints < 0 )
    {
        localATP -= level.alienBuildPoints;
        level.alienBuildPoints = 0;
    }
    
    trap_SetConfigstring( CS_BUILDPOINTS, va( "%d %d %d %d",
                          level.alienBuildPoints, localATP,
                          level.humanBuildPoints, localHTP ) );
                          
    //may as well pump the stages here too
    {
        F32 alienPlayerCountMod;
        F32 humanPlayerCountMod;
        
        alienPlayerCountMod = level.averageNumAlienClients / PLAYER_COUNT_MOD;
        humanPlayerCountMod = level.averageNumHumanClients / PLAYER_COUNT_MOD;
        
        if( alienPlayerCountMod < 0.1f )
            alienPlayerCountMod = 0.1f;
            
        if( humanPlayerCountMod < 0.1f )
            humanPlayerCountMod = 0.1f;
            
        S32 alienNextStageThreshold, humanNextStageThreshold;
        
        if( g_alienStage.integer < g_alienMaxStage.integer )
            alienNextStageThreshold = ( S32 )( ceil( ( F32 )g_alienStageThreshold.integer * ( g_alienStage.integer + 1 ) * alienPlayerCountMod ) );
        else if( g_humanStage.integer > S1 )
            alienNextStageThreshold = ( S32 )( ceil( ( F32 )level.alienStagedownCredits + g_alienStageThreshold.integer * alienPlayerCountMod ) );
        else
            alienNextStageThreshold = -1;
            
        if( g_humanStage.integer < g_humanMaxStage.integer )
            humanNextStageThreshold = ( S32 )( ceil( ( F32 )g_humanStageThreshold.integer * ( g_humanStage.integer + 1 ) * humanPlayerCountMod ) );
        else if( g_alienStage.integer > S1 )
            humanNextStageThreshold = ( S32 )( ceil( ( F32 )level.humanStagedownCredits + g_humanStageThreshold.integer * humanPlayerCountMod ) );
        else
            humanNextStageThreshold = -1;
            
        // save a lot of bandwidth by rounding thresholds up to the nearest 100
        if( alienNextStageThreshold > 0 )
            alienNextStageThreshold = ceil( ( F32 )alienNextStageThreshold / 100 ) * 100;
        if( humanNextStageThreshold > 0 )
            humanNextStageThreshold = ceil( ( F32 )humanNextStageThreshold / 100 ) * 100;
            
        trap_SetConfigstring( CS_STAGES, va( "%d %d %d %d %d %d",
                                             g_alienStage.integer, g_humanStage.integer,
                                             g_alienCredits.integer, g_humanCredits.integer,
                                             alienNextStageThreshold, humanNextStageThreshold ) );
    }
}

/*
============
G_CalculateStages
============
*/
void idGameLocal::CalculateStages( void )
{
    F32         alienPlayerCountMod     = level.averageNumAlienClients / PLAYER_COUNT_MOD;
    F32         humanPlayerCountMod     = level.averageNumHumanClients / PLAYER_COUNT_MOD;
    static S32    lastAlienStageModCount  = 1;
    static S32    lastHumanStageModCount  = 1;
    static S32    lastAlienStage  = -1;
    static S32    lastHumanStage  = -1;
    
    
    if( alienPlayerCountMod < 0.1f )
        alienPlayerCountMod = 0.1f;
        
    if( humanPlayerCountMod < 0.1f )
        humanPlayerCountMod = 0.1f;
        
    if( g_alienCredits.integer >=
            ( S32 )( ceil( ( F32 )g_alienStageThreshold.integer * S2 * alienPlayerCountMod ) ) &&
            g_alienStage.integer == S1 && g_alienMaxStage.integer > S1 )
    {
        trap_Cvar_Set( "g_alienStage", va( "%d", S2 ) );
        level.alienStage2Time = level.time;
        lastAlienStageModCount = g_alienMaxReachedStage.modificationCount;
        if( g_humanStage.integer == S3 )
            level.humanStagedownCredits = g_humanCredits.integer;
        LogPrintf( "stagedownlog: aliens s2, humans s%d; %d %d %d %d\n",
                   g_humanStage.integer + 1,
                   g_humanCredits.integer,
                   level.humanStagedownCredits,
                   g_alienCredits.integer,
                   level.alienStagedownCredits );
    }
    
    if( g_alienCredits.integer >=
            ( S32 )( ceil( ( F32 )g_alienStageThreshold.integer * S3 * alienPlayerCountMod ) ) &&
            g_alienStage.integer == S2 && g_alienMaxStage.integer > S2 )
    {
        trap_Cvar_Set( "g_alienStage", va( "%d", S3 ) );
        level.alienStage3Time = level.time;
        lastAlienStageModCount = g_alienMaxReachedStage.modificationCount;
        if( g_humanStage.integer > S1 )
            level.alienStagedownCredits = g_alienCredits.integer;
        LogPrintf( "stagedownlog: aliens s3, humans s%d; %d %d %d %d\n",
                   g_humanStage.integer + 1,
                   g_humanCredits.integer,
                   level.humanStagedownCredits,
                   g_alienCredits.integer,
                   level.alienStagedownCredits );
    }
    
    if( g_humanCredits.integer >=
            ( S32 )( ceil( ( F32 )g_humanStageThreshold.integer * S2 * humanPlayerCountMod ) ) &&
            g_humanStage.integer == S1 && g_humanMaxStage.integer > S1 )
    {
        trap_Cvar_Set( "g_humanStage", va( "%d", S2 ) );
        
        level.humanStage2Time = level.time;
        lastHumanStageModCount = g_humanMaxReachedStage.modificationCount;
        if( g_alienStage.integer == S3 )
            level.alienStagedownCredits = g_alienCredits.integer;
        LogPrintf( "stagedownlog: humans s2, aliens s%d; %d %d %d %d\n",
                   g_alienStage.integer + 1,
                   g_humanCredits.integer,
                   level.humanStagedownCredits,
                   g_alienCredits.integer,
                   level.alienStagedownCredits );
    }
    
    if( g_humanCredits.integer >=
            ( S32 )( ceil( ( F32 )g_humanStageThreshold.integer * S3 * humanPlayerCountMod ) ) &&
            g_humanStage.integer == S2 && g_humanMaxStage.integer > S2 )
    {
        trap_Cvar_Set( "g_humanStage", va( "%d", S3 ) );
        level.humanStage3Time = level.time;
        lastHumanStageModCount = g_humanMaxReachedStage.modificationCount;
        if( g_alienStage.integer > S1 )
            level.humanStagedownCredits = g_humanCredits.integer;
        LogPrintf( "stagedownlog: humans s3, aliens s%d; %d %d %d %d\n",
                   g_alienStage.integer + 1,
                   g_humanCredits.integer,
                   level.humanStagedownCredits,
                   g_alienCredits.integer,
                   level.alienStagedownCredits );
    }
    if( g_alienStage.integer > S1 &&
            g_humanStage.integer == S3 &&
            g_humanCredits.integer - level.humanStagedownCredits >=
            ( S32 ) ceil( g_humanStageThreshold.integer * humanPlayerCountMod ) )
    {
        LogPrintf( "stagedownlog: aliens staging down\n"
                   "stagedownlog: before: %d %d %d %d %d %d\n",
                   g_humanStage.integer,
                   g_humanCredits.integer,
                   level.humanStagedownCredits,
                   g_alienStage.integer,
                   g_alienCredits.integer,
                   level.alienStagedownCredits );
                   
        if( g_alienStage.integer == S3 )
            level.alienStage3Time = level.startTime;
        if( g_alienStage.integer == S2 )
            level.alienStage2Time = level.startTime;
        lastAlienStageModCount = g_alienStage.modificationCount;
        trap_Cvar_Set( "g_alienStage", va( "%d", g_alienStage.integer - 1 ) );
        trap_Cvar_Set( "g_alienCredits", va( "%d", ( S32 )ceil( g_alienCredits.integer - g_alienStageThreshold.integer * alienPlayerCountMod ) ) );
        trap_Cvar_Set( "g_humanCredits", va( "%d", ( S32 )ceil( g_humanCredits.integer - g_humanStageThreshold.integer * humanPlayerCountMod ) ) );
        LogPrintf( "stagedownlog: after: %d %d %d %d %d %d\n",
                   trap_Cvar_VariableIntegerValue( "g_humanStage" ),
                   trap_Cvar_VariableIntegerValue( "g_humanCredits" ),
                   level.humanStagedownCredits,
                   trap_Cvar_VariableIntegerValue( "g_alienStage" ),
                   trap_Cvar_VariableIntegerValue( "g_alienCredits" ),
                   level.alienStagedownCredits );
        trap_Cvar_Update( &g_humanCredits );
        trap_Cvar_Update( &g_alienCredits );
        trap_Cvar_Update( &g_humanStage );
        
    }
    if( g_humanStage.integer > S1 &&
            g_alienStage.integer == S3 &&
            g_alienCredits.integer - level.alienStagedownCredits >=
            ( S32 ) ceil( g_alienStageThreshold.integer * alienPlayerCountMod ) )
    {
        LogPrintf( "stagedownlog: humans staging down\n"
                   "stagedownlog: before: %d %d %d %d %d %d\n",
                   g_humanStage.integer,
                   g_humanCredits.integer,
                   level.humanStagedownCredits,
                   g_alienStage.integer,
                   g_alienCredits.integer,
                   level.alienStagedownCredits );
                   
        if( g_humanStage.integer == S3 )
            level.humanStage3Time = level.startTime;
        if( g_humanStage.integer == S2 )
            level.humanStage2Time = level.startTime;
        lastHumanStageModCount = g_humanStage.modificationCount;
        trap_Cvar_Set( "g_humanStage", va( "%d", g_humanStage.integer - 1 ) );
        trap_Cvar_Set( "g_humanCredits", va( "%d", ( S32 )ceil( g_humanCredits.integer - g_humanStageThreshold.integer * humanPlayerCountMod ) ) );
        trap_Cvar_Set( "g_alienCredits", va( "%d", ( S32 )ceil( g_alienCredits.integer - g_alienStageThreshold.integer * alienPlayerCountMod ) ) );
        LogPrintf( "stagedownlog: after: %d %d %d %d %d %d\n",
                   trap_Cvar_VariableIntegerValue( "g_humanStage" ),
                   trap_Cvar_VariableIntegerValue( "g_humanCredits" ),
                   level.humanStagedownCredits,
                   trap_Cvar_VariableIntegerValue( "g_alienStage" ),
                   trap_Cvar_VariableIntegerValue( "g_alienCredits" ),
                   level.alienStagedownCredits );
        trap_Cvar_Update( &g_humanCredits );
        trap_Cvar_Update( &g_alienCredits );
        trap_Cvar_Update( &g_humanStage );
    }
    
    if( g_alienMaxReachedStage.modificationCount > lastAlienStageModCount )
    {
        Checktrigger_stages( TEAM_ALIENS, ( stage_t )g_alienMaxReachedStage.integer );
        
        if( g_alienStage.integer == S2 )
            level.alienStage2Time = level.time;
        else if( g_alienStage.integer == S3 )
            level.alienStage3Time = level.time;
            
        lastAlienStageModCount = g_alienMaxReachedStage.modificationCount;
    }
    
    if( g_humanMaxReachedStage.modificationCount > lastHumanStageModCount )
    {
        Checktrigger_stages( TEAM_HUMANS, ( stage_t )g_humanMaxReachedStage.integer );
        
        if( g_humanStage.integer == S2 )
            level.humanStage2Time = level.time;
        else if( g_humanStage.integer == S3 )
            level.humanStage3Time = level.time;
            
        lastHumanStageModCount = g_humanMaxReachedStage.modificationCount;
    }
    
    if( level.demoState == DS_RECORDING &&
            ( trap_Cvar_VariableIntegerValue( "g_alienStage" ) != lastAlienStage ||
              trap_Cvar_VariableIntegerValue( "g_humanStage" ) != lastHumanStage ) )
    {
        lastAlienStage = trap_Cvar_VariableIntegerValue( "g_alienStage" );
        lastHumanStage = trap_Cvar_VariableIntegerValue( "g_humanStage" );
        DemoCommand( DC_SET_STAGE, va( "%d %d", lastHumanStage, lastAlienStage ) );
    }
}

/*
============
CalculateAvgPlayers

Calculates the average number of players playing this game
============
*/
void idGameLocal::CalculateAvgPlayers( void )
{
    //there are no clients or only spectators connected, so
    //reset the number of samples in order to avoid the situation
    //where the average tends to 0
    if( !level.numAlienClients )
    {
        level.numAlienSamples = 0;
        trap_Cvar_Set( "g_alienCredits", "0" );
    }
    
    if( !level.numHumanClients )
    {
        level.numHumanSamples = 0;
        trap_Cvar_Set( "g_humanCredits", "0" );
    }
    
    //calculate average number of clients for stats
    level.averageNumAlienClients =
        ( ( level.averageNumAlienClients * level.numAlienSamples )
          + level.numAlienClients ) /
        ( F32 )( level.numAlienSamples + 1 );
    level.numAlienSamples++;
    
    level.averageNumHumanClients =
        ( ( level.averageNumHumanClients * level.numHumanSamples )
          + level.numHumanClients ) /
        ( F32 )( level.numHumanSamples + 1 );
    level.numHumanSamples++;
}

/*
============
idGameLocal::CalculateRanks

Recalculates the score ranks of all players
This will be called on every client connect, begin, disconnect, death,
and team change.
============
*/
void idGameLocal::CalculateRanks( void )
{
    S32       i;
    UTF8      P[ MAX_CLIENTS + 1 ] = {""};
    S32       ff = 0;
    S32		numBots = 0;
    
    level.numConnectedClients = 0;
    level.numPlayingClients = 0;
    level.numVotingClients = 0;   // don't count bots
    level.numAlienClients = 0;
    level.numHumanClients = 0;
    level.numLiveAlienClients = 0;
    level.numLiveHumanClients = 0;
    
    for( i = 0; i < level.maxclients; i++ )
    {
        P[ i ] = '-';
        if( level.clients[ i ].pers.connected != CON_DISCONNECTED ||
                level.clients[ i ].pers.demoClient )
        {
            level.sortedClients[ level.numConnectedClients ] = i;
            level.numConnectedClients++;
            P[ i ] = ( UTF8 )'0' + level.clients[ i ].pers.teamSelection;
            
            if( level.clients[ i ].pers.connected != CON_CONNECTED &&
                    !level.clients[ i ].pers.demoClient )
                continue;
                
            level.numVotingClients++;
            if( level.clients[ i ].pers.teamSelection != TEAM_NONE )
            {
                level.numPlayingClients++;
                if( g_entities[ i ].r.svFlags & SVF_BOT )
                    numBots++;
                if( level.clients[ i ].pers.teamSelection == TEAM_ALIENS )
                {
                    level.numAlienClients++;
                    if( level.clients[ i ].sess.spectatorState == SPECTATOR_NOT )
                        level.numLiveAlienClients++;
                }
                else if( level.clients[ i ].pers.teamSelection == TEAM_HUMANS )
                {
                    level.numHumanClients++;
                    if( level.clients[ i ].sess.spectatorState == SPECTATOR_NOT )
                        level.numLiveHumanClients++;
                }
            }
        }
    }
    level.numNonSpectatorClients = level.numLiveAlienClients +
                                   level.numLiveHumanClients;
    level.numVotingClients = level.numConnectedClients - numBots;
    level.numteamVotingClients[ 0 ] = level.numHumanClients;
    level.numteamVotingClients[ 1 ] = level.numAlienClients;
    P[ i ] = '\0';
    trap_Cvar_Set( "P", P );
    
    if( g_friendlyFire.integer )
        ff |= ( FFF_HUMANS | FFF_ALIENS );
    if( g_friendlyFireHumans.integer )
        ff |=  FFF_HUMANS;
    if( g_friendlyFireAliens.integer )
        ff |=  FFF_ALIENS;
    if( g_friendlyBuildableFire.integer )
        ff |=  FFF_BUILDABLES;
    trap_Cvar_Set( "ff", va( "%i", ff ) );
    
    qsort( level.sortedClients, level.numConnectedClients, sizeof( level.sortedClients[ 0 ] ), &idGameLocal::SortRanks );
    
    // see if it is time to end the level
    CheckExitRules( );
    
    // if we are at the intermission, send the new info to everyone
    if( level.intermissiontime )
        SendScoreboardMessageToAllClients( );
}

/*
============
idGameLocal::DemoCommand

Store a demo command to a demo if we are recording
============
*/
void idGameLocal::DemoCommand( demoCommand_t cmd, StringEntry string )
{
    //if( level.demoState == DS_RECORDING )
    //    trap_DemoCommand( cmd, string );
}

/*
============
G_DemoSetClient

Mark a client as a demo client and load info into it
============
*/
void idGameLocal::DemoSetClient( void )
{
    UTF8 buffer[ MAX_INFO_STRING ];
    S32 clientNum;
    gclient_t* client;
    UTF8* s;
    
    trap_Argv( 0, buffer, sizeof( buffer ) );
    clientNum = atoi( buffer );
    client = level.clients + clientNum;
    client->pers.demoClient = true;
    
    trap_Argv( 1, buffer, sizeof( buffer ) );
    s = Info_ValueForKey( buffer, "n" );
    if( *s )
        Q_strncpyz( client->pers.netname, s, sizeof( client->pers.netname ) );
    s = Info_ValueForKey( buffer, "t" );
    if( *s )
        client->pers.teamSelection = ( team_t )atoi( s );
    client->sess.spectatorState = SPECTATOR_NOT;
    trap_SetConfigstring( CS_PLAYERS + clientNum, buffer );
}

/*
============
G_DemoRemoveClient

Unmark a client as a demo client
============
*/
void idGameLocal::DemoRemoveClient( void )
{
    UTF8 buffer[ 3 ];
    S32 clientNum;
    
    trap_Argv( 0, buffer, sizeof( buffer ) );
    clientNum = atoi( buffer );
    level.clients[clientNum].pers.demoClient = false;
    trap_SetConfigstring( CS_PLAYERS + clientNum, NULL );
}

/*
============
G_DemoSetStage

Set the stages in a demo
============
*/
void idGameLocal::DemoSetStage( void )
{
    UTF8 buffer[ 2 ];
    
    trap_Argv( 0, buffer, sizeof( buffer ) );
    trap_Cvar_Set( "g_humanStage", buffer );
    trap_Argv( 1, buffer, sizeof( buffer ) );
    trap_Cvar_Set( "g_alienStage", buffer );
}


/*
========================================================================

MAP CHANGING

========================================================================
*/

/*
========================
idGameLocal::SendScoreboardMessageToAllClients

Do this at BeginIntermission time and whenever ranks are recalculated
due to enters/exits/forced team changes
========================
*/
void idGameLocal::SendScoreboardMessageToAllClients( void )
{
    S32   i;
    
    for( i = 0; i < level.maxclients; i++ )
    {
        if( level.clients[ i ].pers.connected == CON_CONNECTED )
            ScoreboardMessage( g_entities + i );
    }
}

/*
========================
idGameLocal::MoveClientToIntermission

When the intermission starts, this will be called for all players.
If a new client connects, this will be called after the spawn function.
========================
*/
void idGameLocal::MoveClientToIntermission( gentity_t* ent )
{
    // take out of follow mode if needed
    if( ent->client->sess.spectatorState == SPECTATOR_FOLLOW )
        StopFollowing( ent );
        
    // move to the spot
    VectorCopy( level.intermission_origin, ent->s.origin );
    VectorCopy( level.intermission_origin, ent->client->ps.origin );
    VectorCopy( level.intermission_angle, ent->client->ps.viewangles );
    ent->client->ps.pm_type = PM_INTERMISSION;
    
    // clean up powerup info
    ::memset( ent->client->ps.misc, 0, sizeof( ent->client->ps.misc ) );
    
    ent->client->ps.eFlags = 0;
    ent->s.eFlags = 0;
    ent->s.eType = ET_GENERAL;
    ent->s.modelindex = 0;
    ent->s.loopSound = 0;
    ent->s.event = 0;
    ent->r.contents = 0;
}

/*
==================
idGameLocal::FindIntermissionPoint

This is also used for spectator spawns
==================
*/
void idGameLocal::FindIntermissionPoint( void )
{
    gentity_t* ent, *target;
    vec3_t    dir;
    
    // find the intermission spot
    ent = Find( NULL, FOFS( classname ), "info_player_intermission" );
    
    if( !ent )
    {
        // the map creator forgot to put in an intermission point...
        SelectSpawnPoint( vec3_origin, level.intermission_origin, level.intermission_angle );
    }
    else
    {
        VectorCopy( ent->s.origin, level.intermission_origin );
        VectorCopy( ent->s.angles, level.intermission_angle );
        // if it has a target, look towards it
        if( ent->target )
        {
            target = PickTarget( ent->target );
            
            if( target )
            {
                VectorSubtract( target->s.origin, level.intermission_origin, dir );
                vectoangles( dir, level.intermission_angle );
            }
        }
    }
    
}

/*
==================
idGameLocal::BeginIntermission
==================
*/
void idGameLocal::BeginIntermission( void )
{
    S32     i;
    gentity_t* client;
    
    if( level.intermissiontime )
        return;   // already active
        
    level.intermissiontime = level.time;
    
    ClearVotes( );
    
    FindIntermissionPoint( );
    
    // move all clients to the intermission point
    for( i = 0; i < level.maxclients; i++ )
    {
        client = g_entities + i;
        
        if( !client->inuse )
            continue;
            
        // respawn if dead
        if( client->health <= 0 )
            respawn( client );
            
        MoveClientToIntermission( client );
    }
    botLocal.BotBeginIntermission();
    // send the current scoring to all clients
    SendScoreboardMessageToAllClients( );
}


/*
=============
ExitLevel

When the intermission has been exited, the server is either moved
to a new map based on the map rotation or the current map restarted
=============
*/
void idGameLocal::ExitLevel( void )
{
    S32       i;
    gclient_t* cl;
    
    if( MapRotationActive() )
    {
        AdvanceMapRotation();
    }
    else
    {
        trap_SendConsoleCommand( EXEC_APPEND, "map_restart\n" );
    }
    
    level.restarted = true;
    level.changemap = NULL;
    level.intermissiontime = 0;
    
    // reset all the scores so we don't enter the intermission again
    for( i = 0; i < g_maxclients.integer; i++ )
    {
        cl = level.clients + i;
        if( cl->pers.connected != CON_CONNECTED )
            continue;
            
        cl->ps.persistant[ PERS_SCORE ] = 0;
    }
    
    // we need to do this here before chaning to CON_CONNECTING
    WriteSessionData( );
    
    // change all client states to connecting, so the early players into the
    // next level will know the others aren't done reconnecting
    for( i = 0; i < g_maxclients.integer; i++ )
    {
        if( level.clients[ i ].pers.connected == CON_CONNECTED )
            level.clients[ i ].pers.connected = CON_CONNECTING;
    }
}

/*
=================
idGameLocal::AdminMessage

Print to all active server admins, and to the logfile, and to the server console
Prepend *prefix, or '[SERVER]' if no *prefix is given
=================
*/
void idGameLocal::AdminMessage( StringEntry prefix, StringEntry fmt, ... )
{
    va_list argptr;
    UTF8    string[ 1024 ];
    UTF8    outstring[ 1024 ];
    S32     i;
    
    // Format the text
    va_start( argptr, fmt );
    Q_vsnprintf( string, sizeof( string ), fmt, argptr );
    va_end( argptr );
    
    // If there is no prefix, assume that this function was called directly
    // and we should add one
    if( !prefix || !prefix[ 0 ] )
    {
        prefix = "[SERVER]:";
    }
    
    // Create the final string
    Com_sprintf( outstring, sizeof( outstring ), "%s " S_COLOR_MAGENTA "%s",
                 prefix, string );
                 
    // Send to all appropriate clients
    for( i = 0; i < level.maxclients; i++ )
        if( adminLocal.AdminPermission( &g_entities[ i ], ADMF_ADMINCHAT ) )
            trap_SendServerCommand( i, va( "chat \"%s\"", outstring ) );
            
    // Send to the logfile and server console
    LogPrintf( "adminmsg: %s\n", outstring );
}


/*
=================
idGameLocal::LogPrintf

Print to the logfile with a time stamp if it is open, and to the server console
=================
*/
void idGameLocal::LogPrintf( StringEntry fmt, ... )
{
    va_list argptr;
    UTF8 string[BIG_INFO_STRING], decolored[BIG_INFO_STRING];
    S32 min, tens, sec;
    U64 tslen;
    
    sec = level.time / 1000;
    
    min = sec / 60;
    sec -= min * 60;
    tens = sec / 10;
    sec -= tens * 10;
    
    Com_sprintf( string, sizeof( string ), "%3i:%i%i ", min, tens, sec );
    
    tslen = strlen( string );
    
    va_start( argptr, fmt );
    Q_vsnprintf( string + tslen, sizeof( string ) - tslen, fmt, argptr );
    va_end( argptr );
    
    if( !level.logFile )
    {
        return;
    }
    
    DecolorString( string, decolored, sizeof( decolored ) );
    trap_FS_Write( decolored, strlen( decolored ), level.logFile );
}

/*
=================
idGameLocal::SendGameStat
=================
*/
void idGameLocal::SendGameStat( team_t team )
{
    UTF8      map[ MAX_STRING_CHARS ];
    UTF8      teamChar;
    UTF8      data[ BIG_INFO_STRING ];
    UTF8      entry[ MAX_STRING_CHARS ];
    S32       i, dataLength, entryLength;
    gclient_t* cl;
    
    trap_Cvar_VariableStringBuffer( "mapname", map, sizeof( map ) );
    
    switch( team )
    {
        case TEAM_ALIENS:
            teamChar = 'A';
            break;
        case TEAM_HUMANS:
            teamChar = 'H';
            break;
        case TEAM_NONE:
            teamChar = 'L';
            break;
        default:
            return;
    }
    
    Com_sprintf( data, BIG_INFO_STRING,
                 "%s %s T:%c A:%f H:%f M:%s D:%d SD:%d AS:%d AS2T:%d AS3T:%d HS:%d HS2T:%d HS3T:%d CL:%d",
                 Q3_VERSION,
                 g_tag.string,
                 teamChar,
                 level.averageNumAlienClients,
                 level.averageNumHumanClients,
                 map,
                 level.time - level.startTime,
                 TimeTilSuddenDeath( ),
                 g_alienStage.integer,
                 level.alienStage2Time - level.startTime,
                 level.alienStage3Time - level.startTime,
                 g_humanStage.integer,
                 level.humanStage2Time - level.startTime,
                 level.humanStage3Time - level.startTime,
                 level.numConnectedClients );
                 
    dataLength = strlen( data );
    
    for( i = 0; i < level.numConnectedClients; i++ )
    {
        S32 ping;
        
        cl = &level.clients[ level.sortedClients[ i ] ];
        
        if( cl->pers.connected == CON_CONNECTING )
            ping = -1;
        else
            ping = cl->ps.ping < 999 ? cl->ps.ping : 999;
            
        switch( cl->ps.stats[ STAT_TEAM ] )
        {
            case TEAM_ALIENS:
                teamChar = 'A';
                break;
            case TEAM_HUMANS:
                teamChar = 'H';
                break;
            case TEAM_NONE:
                teamChar = 'S';
                break;
            default:
                return;
        }
        
        Com_sprintf( entry, MAX_STRING_CHARS,
                     " \"%s\" %c %d %d %d",
                     cl->pers.netname,
                     teamChar,
                     cl->ps.persistant[ PERS_SCORE ],
                     ping,
                     ( level.time - cl->pers.enterTime ) / 60000 );
                     
        entryLength = strlen( entry );
        
        if( dataLength + entryLength >= BIG_INFO_STRING )
            break;
            
        strcpy( data + dataLength, entry );
        dataLength += entryLength;
    }
    
    trap_SendGameStat( data );
}

/*
================
idGameLocal::LogExit

Append information about this game to the log file
================
*/
void idGameLocal::LogExit( StringEntry string )
{
    S32         i, numSorted;
    gclient_t*   cl;
    gentity_t*   ent;
    
    LogPrintf( "Exit: %s\n", string );
    
    level.intermissionQueued = level.time;
    
    // this will keep the clients from playing any voice sounds
    // that will get cut off when the queued intermission starts
    trap_SetConfigstring( CS_INTERMISSION, "1" );
    
    // don't send more than 32 scores (FIXME?)
    numSorted = level.numConnectedClients;
    if( numSorted > 32 )
        numSorted = 32;
        
    for( i = 0; i < numSorted; i++ )
    {
        S32   ping;
        
        cl = &level.clients[ level.sortedClients[ i ] ];
        
        if( cl->ps.stats[ STAT_TEAM ] == TEAM_NONE )
            continue;
            
        if( cl->pers.connected == CON_CONNECTING )
            continue;
            
        ping = cl->ps.ping < 999 ? cl->ps.ping : 999;
        
        LogPrintf( "score: %i  ping: %i  client: %i %s\n",
                   cl->ps.persistant[ PERS_SCORE ], ping, level.sortedClients[ i ],
                   cl->pers.netname );
    }
    
    for( i = 1, ent = g_entities + i ; i < level.num_entities ; i++, ent++ )
    {
        if( !ent->inuse )
            continue;
            
        if( !Q_stricmp( ent->classname, "trigger_win" ) )
        {
            if( level.lastWin == ent->stageTeam )
                ent->use( ent, ent, ent );
        }
    }
    
    SendGameStat( level.lastWin );
}


/*
=================
CheckIntermissionExit

The level will stay at the intermission for a minimum of 5 seconds
If all players wish to continue, the level will then exit.
If one or more players have not acknowledged the continue, the game will
wait 10 seconds before going on.
=================
*/
void idGameLocal::CheckIntermissionExit( void )
{
    S32       ready, notReady;
    S32       i;
    gclient_t* cl;
    U8      readyMasks[( MAX_CLIENTS + 7 ) / 8 ];
    
    // each byte in readyMasks will become two characters 00 - ff in the string
    UTF8      readyString[ 2 * sizeof( readyMasks ) + 1 ];
    
    //if no clients are connected, just exit
    if( level.numConnectedClients == 0 )
    {
        ExitLevel( );
        return;
    }
    
    // see which players are ready
    ready = 0;
    notReady = 0;
    ::memset( readyMasks, 0, sizeof( readyMasks ) );
    for( i = 0; i < g_maxclients.integer; i++ )
    {
        cl = level.clients + i;
        if( cl->pers.connected != CON_CONNECTED )
            continue;
            
        if( cl->ps.stats[ STAT_TEAM ] == TEAM_NONE )
            continue;
            
        if( g_entities[ cl->ps.clientNum ].r.svFlags & SVF_BOT )
            continue;
            
        if( cl->readyToExit )
        {
            ready++;
            // the nth bit of readyMasks is for client (n - 1)
            readyMasks[ i / 8 ] |= 1 << ( 7 - ( i % 8 ) );
        }
        else
            notReady++;
    }
    
    // this is hex because we can convert bits to a hex string in pieces,
    // whereas a decimal string would have to all be written at once
    // (and we can't fit a number that large in an S32)
    for( i = 0; i < ( g_maxclients.integer + 7 ) / 8; i++ )
        Com_sprintf( &readyString[ i * 2 ], sizeof( readyString ) - i * 2,
                     "%2.2x", readyMasks[ i ] );
                     
    trap_SetConfigstring( CS_CLIENTS_READY, readyString );
    
    // never exit in less than five seconds
    if( level.time < level.intermissiontime + 5000 )
        return;
        
    // never let intermission go on for over 1 minute
    if( level.time > level.intermissiontime + 60000 )
    {
        ExitLevel( );
        return;
    }
    
    // if nobody wants to go, clear timer
    if( ready == 0 && notReady > 0 )
    {
        level.readyToExit = false;
        return;
    }
    
    // if everyone wants to go, go now
    if( notReady == 0 )
    {
        ExitLevel( );
        return;
    }
    
    // the first person to ready starts the thirty second timeout
    if( !level.readyToExit )
    {
        level.readyToExit = true;
        level.exitTime = level.time;
    }
    
    // if we have waited thirty seconds since at least one player
    // wanted to exit, go ahead
    if( level.time < level.exitTime + 30000 )
        return;
        
    ExitLevel( );
}

/*
=============
ScoreIsTied
=============
*/
bool idGameLocal::ScoreIsTied( void )
{
    S32   a, b;
    
    if( level.numPlayingClients < 2 )
        return false;
        
    a = level.clients[ level.sortedClients[ 0 ] ].ps.persistant[ PERS_SCORE ];
    b = level.clients[ level.sortedClients[ 1 ] ].ps.persistant[ PERS_SCORE ];
    
    return a == b;
}

/*
=================
CheckExitRules

There will be a delay between the time the exit is qualified for
and the time everyone is moved to the intermission spot, so you
can see the last frag.
=================
*/
void idGameLocal::CheckExitRules( void )
{
    // don't exit in demos
    if( level.demoState == DS_PLAYBACK )
        return;
        
    // if at the intermission, wait for all non-bots to
    // signal ready, then go to next level
    if( level.intermissiontime )
    {
        CheckIntermissionExit( );
        return;
    }
    
    if( level.intermissionQueued )
    {
        if( level.time - level.intermissionQueued >= INTERMISSION_DELAY_TIME )
        {
            level.intermissionQueued = 0;
            BeginIntermission( );
        }
        
        return;
    }
    
    if( g_timelimit.integer && !level.warmupTime )
    {
        if( level.time - level.startTime >= g_timelimit.integer * 60000 )
        {
            level.lastWin = TEAM_NONE;
            trap_SendServerCommand( -1, "print \"Timelimit hit\n\"" );
            trap_SetConfigstring( CS_WINNER, "Stalemate" );
            LogExit( "Timelimit hit." );
            return;
        }
        else if( level.time - level.startTime >= ( g_timelimit.integer - 5 ) * 60000 &&
                 level.timelimitWarning < TW_IMMINENT )
        {
            trap_SendServerCommand( -1, "cp \"5 minutes remaining!\"" );
            level.timelimitWarning = TW_IMMINENT;
        }
        else if( level.time - level.startTime >= ( g_timelimit.integer - 1 ) * 60000 &&
                 level.timelimitWarning < TW_PASSED )
        {
            trap_SendServerCommand( -1, "cp \"1 minute remaining!\"" );
            level.timelimitWarning = TW_PASSED;
        }
    }
    
    if( level.uncondHumanWin ||
            ( ( level.time > level.startTime + 1000 ) &&
              ( level.numAlienSpawns == 0 ) &&
              ( level.numLiveAlienClients == 0 ) ) )
    {
        //humans win
        level.lastWin = TEAM_HUMANS;
        trap_SendServerCommand( -1, "print \"Humans win\n\"" );
        trap_SetConfigstring( CS_WINNER, "Humans Win" );
        LogExit( "Humans win." );
    }
    else if( level.uncondAlienWin ||
             ( ( level.time > level.startTime + 1000 ) &&
               ( level.numHumanSpawns == 0 ) &&
               ( level.numLiveHumanClients == 0 ) ) )
    {
        //aliens win
        level.lastWin = TEAM_ALIENS;
        trap_SendServerCommand( -1, "print \"Aliens win\n\"" );
        trap_SetConfigstring( CS_WINNER, "Aliens Win" );
        LogExit( "Aliens win." );
    }
}

/*
==================
idGameLocal::Vote
==================
*/
void idGameLocal::Vote( gentity_t* ent, bool voting )
{
    if( !level.voteTime )
        return;
        
    if( voting )
    {
        if( ent->client->ps.eFlags & EF_VOTED )
            return;
        ent->client->ps.eFlags |= EF_VOTED;
    }
    else
    {
        if( !( ent->client->ps.eFlags & EF_VOTED ) )
            return;
        ent->client->ps.eFlags &= ~EF_VOTED;
    }
    
    if( ent->client->pers.vote )
    {
        if( voting )
            level.voteYes++;
        else
            level.voteYes--;
        trap_SetConfigstring( CS_VOTE_YES, va( "%d", level.voteYes ) );
    }
    else
    {
        if( voting )
            level.voteNo++;
        else
            level.voteNo--;
        trap_SetConfigstring( CS_VOTE_NO, va( "%d", level.voteNo ) );
    }
}

/*
==================
idGameLocal::TeamVote
==================
*/
void idGameLocal::TeamVote( gentity_t* ent, bool voting )
{
    S32 cs_offset;
    
    if( ent->client->pers.teamSelection == TEAM_HUMANS )
        cs_offset = 0;
    else if( ent->client->pers.teamSelection == TEAM_ALIENS )
        cs_offset = 1;
    else
        return;
        
    if( !level.teamVoteTime[ cs_offset ] )
        return;
        
    if( voting )
    {
        if( ent->client->ps.eFlags & EF_TEAMVOTED )
            return;
        ent->client->ps.eFlags |= EF_TEAMVOTED;
    }
    else
    {
        if( !( ent->client->ps.eFlags & EF_TEAMVOTED ) )
            return;
        ent->client->ps.eFlags &= ~EF_TEAMVOTED;
    }
    
    if( ent->client->pers.teamVote )
    {
        if( voting )
            level.teamVoteYes[ cs_offset ]++;
        else
            level.teamVoteYes[ cs_offset ]--;
        trap_SetConfigstring( CS_TEAMVOTE_YES + cs_offset, va( "%d", level.teamVoteYes[ cs_offset ] ) );
    }
    else
    {
        if( voting )
            level.teamVoteNo[ cs_offset ]++;
        else
            level.teamVoteNo[ cs_offset ]--;
        trap_SetConfigstring( CS_TEAMVOTE_NO + cs_offset, va( "%d", level.teamVoteNo[ cs_offset ] ) );
    }
}


/*
========================================================================

FUNCTIONS CALLED EVERY FRAME

========================================================================
*/


/*
==================
idGameLocal::CheckVote
==================
*/
void idGameLocal::CheckVote( void )
{
    S32 votePassThreshold = level.votePassThreshold;
    S32 voteYesPercent;
    
    if( level.voteExecuteTime && level.voteExecuteTime < level.time )
    {
        level.voteExecuteTime = 0;
        
        trap_SendConsoleCommand( EXEC_APPEND, va( "%s\n", level.voteString ) );
        if( !Q_stricmp( level.voteString, "map_restart" ) ||
                !Q_stricmpn( level.voteString, "map", 3 ) )
        {
            level.restarted = true;
        }
        
        if( !Q_stricmp( level.voteString, "suddendeath" ) )
        {
            level.suddenDeathBeginTime = level.time +
                                         ( 1000 * g_suddenDeathVoteDelay.integer ) - level.startTime;
            level.voteString[0] = '\0';
            
            if( g_suddenDeathVoteDelay.integer )
                trap_SendServerCommand( -1, va( "cp \"Sudden Death will begin in %d seconds\n\"", g_suddenDeathVoteDelay.integer ) );
        }
    }
    
    if( !level.voteTime )
        return;
        
    if( level.voteYes + level.voteNo > 0 )
        voteYesPercent = ( S32 )( 100 * ( level.voteYes ) / ( level.voteYes + level.voteNo ) );
    else
        voteYesPercent = 0;
        
    if( ( level.time - level.voteTime >= VOTE_TIME ) ||
            ( level.voteYes + level.voteNo == level.numConnectedClients ) )
    {
        if( voteYesPercent > votePassThreshold || level.voteNo == 0 )
        {
            // execute the command, then remove the vote
            trap_SendServerCommand( -1, va( "print \"Vote passed (%d - %d)\n\"",
                                            level.voteYes, level.voteNo ) );
            LogPrintf( "Vote: Vote passed (%d-%d)\n", level.voteYes, level.voteNo );
            level.voteExecuteTime = level.time + 3000;
        }
        else
        {
            // same behavior as a timeout
            trap_SendServerCommand( -1, va( "print \"Vote failed (%d - %d)\n\"",
                                            level.voteYes, level.voteNo ) );
            LogPrintf( "Vote: Vote failed (%d - %d)\n", level.voteYes, level.voteNo );
        }
    }
    else
    {
        if( level.voteYes > ( S32 )( ( F64 ) level.numConnectedClients *
                                     ( ( F64 ) votePassThreshold / 100.0 ) ) )
        {
            // execute the command, then remove the vote
            trap_SendServerCommand( -1, va( "print \"Vote passed (%d - %d)\n\"",
                                            level.voteYes, level.voteNo ) );
            LogPrintf( "Vote: Vote passed (%d - %d)\n", level.voteYes, level.voteNo );
            level.voteExecuteTime = level.time + 3000;
        }
        else if( level.voteNo > ( S32 )( ( F64 ) level.numConnectedClients *
                                         ( ( F64 )( 100.0 - votePassThreshold ) / 100.0 ) ) )
        {
            // same behavior as a timeout
            trap_SendServerCommand( -1, va( "print \"Vote failed (%d - %d)\n\"",
                                            level.voteYes, level.voteNo ) );
            LogPrintf( "Vote failed\n" );
        }
        else
        {
            // still waiting for a majority
            return;
        }
    }
    
    level.voteTime = 0;
    trap_SetConfigstring( CS_VOTE_TIME, "" );
}


/*
==================
idGameLocal::CheckTeamVote
==================
*/
void idGameLocal::CheckTeamVote( team_t team )
{
    S32 cs_offset;
    
    if( team == TEAM_HUMANS )
        cs_offset = 0;
    else if( team == TEAM_ALIENS )
        cs_offset = 1;
    else
        return;
        
    if( !level.teamVoteTime[ cs_offset ] )
        return;
        
    if( level.time - level.teamVoteTime[ cs_offset ] >= VOTE_TIME )
    {
        TeamCommand( team, "print \"Team vote failed\n\"" );
    }
    else
    {
        if( level.teamVoteYes[ cs_offset ] > level.numteamVotingClients[ cs_offset ] / 2 )
        {
            // execute the command, then remove the vote
            TeamCommand( team, "print \"Team vote passed\n\"" );
            trap_SendConsoleCommand( EXEC_APPEND, va( "%s\n", level.teamVoteString[ cs_offset ] ) );
        }
        else if( level.teamVoteNo[ cs_offset ] >= level.numteamVotingClients[ cs_offset ] / 2 )
        {
            // same behavior as a timeout
            TeamCommand( team, "print \"Team vote failed\n\"" );
        }
        else
        {
            // still waiting for a majority
            return;
        }
    }
    
    level.teamVoteTime[ cs_offset ] = 0;
    trap_SetConfigstring( CS_TEAMVOTE_TIME + cs_offset, "" );
}

/*
==================
CheckCvars
==================
*/
void idGameLocal::CheckCvars( void )
{
    static S32 lastPasswordModCount   = -1;
    static S32 lastMarkDeconModCount  = -1;
    static S32 lastSDTimeModCount = -1;
    
    if( g_password.modificationCount != lastPasswordModCount )
    {
        lastPasswordModCount = g_password.modificationCount;
        
        if( *g_password.string && Q_stricmp( g_password.string, "none" ) )
            trap_Cvar_Set( "g_needpass", "1" );
        else
            trap_Cvar_Set( "g_needpass", "0" );
    }
    
    // Unmark any structures for deconstruction when
    // the server setting is changed
    if( g_markDeconstruct.modificationCount != lastMarkDeconModCount )
    {
        S32       i;
        gentity_t* ent;
        
        lastMarkDeconModCount = g_markDeconstruct.modificationCount;
        
        for( i = 1, ent = g_entities + i ; i < level.num_entities ; i++, ent++ )
        {
            if( !ent->inuse )
                continue;
                
            if( ent->s.eType != ET_BUILDABLE )
                continue;
                
            ent->deconstruct = false;
        }
    }
    
    // If we change g_suddenDeathTime during a map, we need to update
    // when sd will begin
    if( g_suddenDeathTime.modificationCount != lastSDTimeModCount )
    {
        lastSDTimeModCount = g_suddenDeathTime.modificationCount;
        level.suddenDeathBeginTime = g_suddenDeathTime.integer * 60000;
    }
    
    level.frameMsec = trap_Milliseconds( );
}

/*
==================
CheckDemo
==================
*/
void idGameLocal::CheckDemo( void )
{
    S32 i;
    
    // Don't do anything if no change
    if( g_demoState.integer == level.demoState )
        return;
    level.demoState = ( demoState_t )g_demoState.integer;
    
    // log all connected clients
    if( g_demoState.integer == DS_RECORDING )
    {
        for( i = 0; i < level.maxclients; i++ )
        {
            if( level.clients[ i ].pers.connected != CON_DISCONNECTED )
            {
                UTF8 userinfo[ MAX_INFO_STRING ];
                trap_GetConfigstring( CS_PLAYERS + i, userinfo, sizeof( userinfo ) );
                DemoCommand( DC_CLIENT_SET, va( "%d %s", i, userinfo ) );
            }
        }
    }
    
    // empty teams and display a message
    else if( g_demoState.integer == DS_PLAYBACK )
    {
        trap_SendServerCommand( -1, "print \"A demo has been started on the server.\n\"" );
        for( i = 0; i < level.maxclients; i++ )
        {
            if( level.clients[ i ].pers.teamSelection != TEAM_NONE )
                ChangeTeam( g_entities + i, TEAM_NONE );
        }
    }
    
    // clear all demo clients
    if( g_demoState.integer == DS_NONE || g_demoState.integer == DS_PLAYBACK )
    {
        S32 clients = trap_Cvar_VariableIntegerValue( "sv_democlients" );
        for( i = 0; i < clients; i++ )
            trap_SetConfigstring( CS_PLAYERS + i, NULL );
    }
}

/*
=============
idGameLocal::RunThink

Runs thinking code for this frame if necessary
=============
*/
void idGameLocal::RunThink( gentity_t* ent )
{
    F32 thinktime;
    
    thinktime = ent->nextthink;
    if( thinktime <= 0 )
        return;
        
    if( thinktime > level.time )
        return;
        
    ent->nextthink = 0;
    if( !ent->think )
        Error( "NULL ent->think" );
        
    ent->think( ent );
}

/*
=============
G_EvaluateAcceleration

Calculates the acceleration for an entity
=============
*/
void idGameLocal::EvaluateAcceleration( gentity_t* ent, S32 msec )
{
    vec3_t  deltaVelocity;
    vec3_t  deltaAccel;
    
    VectorSubtract( ent->s.pos.trDelta, ent->oldVelocity, deltaVelocity );
    VectorScale( deltaVelocity, 1.0f / ( F32 )msec, ent->acceleration );
    
    VectorSubtract( ent->acceleration, ent->oldAccel, deltaAccel );
    VectorScale( deltaAccel, 1.0f / ( F32 )msec, ent->jerk );
    
    VectorCopy( ent->s.pos.trDelta, ent->oldVelocity );
    VectorCopy( ent->acceleration, ent->oldAccel );
}

/*
================
idGameLocal::RunFrame

Advances the non-player objects in the world
================
*/
void idGameLocal::RunFrame( S32 levelTime )
{
    S32       i;
    gentity_t* ent;
    S32       msec;
    S32       start, end;
    
    // if we are waiting for the level to restart, do nothing
    if( level.restarted )
        return;
        
    level.framenum++;
    level.previousTime = level.time;
    level.time = levelTime;
    msec = level.time - level.previousTime;
    
    // seed the rng
    srand( level.framenum );
    
    // get any cvar changes
    UpdateCvars( );
    
    // check demo state
    CheckDemo( );
    
    //
    // go through all allocated objects
    //
    start = trap_Milliseconds( );
    ent = &g_entities[ 0 ];
    
    for( i = 0; i < ( level.demoState == DS_PLAYBACK ? g_maxclients.integer : level.num_entities ); i++, ent++ )
    {
        if( !ent->inuse )
            continue;
            
        // clear events that are too old
        if( level.time - ent->eventTime > EVENT_VALID_MSEC )
        {
            if( ent->s.event )
            {
                ent->s.event = 0; // &= EV_EVENT_BITS;
                if( ent->client )
                {
                    ent->client->ps.externalEvent = 0;
                    //ent->client->ps.events[0] = 0;
                    //ent->client->ps.events[1] = 0;
                }
            }
            
            if( ent->freeAfterEvent )
            {
                // tempEntities or dropped items completely go away after their event
                FreeEntity( ent );
                continue;
            }
            else if( ent->unlinkAfterEvent )
            {
                // items that will respawn will hide themselves after their pickup event
                ent->unlinkAfterEvent = false;
                trap_UnlinkEntity( ent );
            }
        }
        
        // temporary entities don't think
        if( ent->freeAfterEvent )
            continue;
            
        // calculate the acceleration of this entity
        if( ent->evaluateAcceleration )
            EvaluateAcceleration( ent, msec );
            
        if( !ent->r.linked && ent->neverFree )
            continue;
            
        if( ent->s.eType == ET_MISSILE )
        {
            RunMissile( ent );
            continue;
        }
        
        if( ent->s.eType == ET_BUILDABLE )
        {
            BuildableThink( ent, msec );
            continue;
        }
        
        if( ent->s.eType == ET_CORPSE || ent->physicsObject )
        {
            Physics( ent, msec );
            continue;
        }
        
        if( ent->s.eType == ET_MOVER )
        {
            RunMover( ent );
            continue;
        }
        
        if( i < MAX_CLIENTS )
        {
            RunClient( ent );
            continue;
        }
        
        RunThink( ent );
    }
    end = trap_Milliseconds();
    
    start = trap_Milliseconds();
    
    // perform final fixups on the players
    ent = &g_entities[ 0 ];
    
    for( i = 0; i < level.maxclients; i++, ent++ )
    {
        if( ent->inuse )
            ClientEndFrame( ent );
    }
    
    // save position information for all active clients
    UnlaggedStore( );
    
    end = trap_Milliseconds();
    
    CountSpawns( );
    CalculateBuildPoints( );
    CalculateStages( );
    SpawnClients( TEAM_ALIENS );
    SpawnClients( TEAM_HUMANS );
    CalculateAvgPlayers( );
    //UpdateZaps( msec );
    
    // see if it is time to end the level
    CheckExitRules( );
    
    // update to team status?
    CheckTeamStatus( );
    
    // cancel vote if timed out
    CheckVote( );
    
    // check team votes
    CheckTeamVote( TEAM_HUMANS );
    CheckTeamVote( TEAM_ALIENS );
    
    // for tracking changes
    CheckCvars( );
    
    if( g_listEntity.integer )
    {
        for( i = 0; i < MAX_GENTITIES; i++ )
            Printf( "%4i: %s\n", i, g_entities[ i ].classname );
            
        trap_Cvar_Set( "g_listEntity", "0" );
    }
    
    level.frameMsec = trap_Milliseconds();
}

/*
================
idGameLocal::BotVisibleFromPos
================
*/
bool idGameLocal::BotVisibleFromPos( vec3_t srcorigin, S32 srcnum, vec3_t destorigin, S32 destent, bool dummy )
{
    vec3_t eye;
    //
    VectorCopy( srcorigin, eye );
    eye[2] += level.clients[srcnum].ps.viewheight;
    //
    //if( BotEntityVisible( srcnum, eye, vec3_origin, 360, destent, destorigin ) )
    //{
    //    return true;
    //}
    //
    return false;
}

/*
================
idGameLocal::BotCheckAttackAtPos

  FIXME: do better testing here
================
*/
bool idGameLocal::BotCheckAttackAtPos( S32 entnum, S32 enemy, vec3_t pos, bool ducking, bool allowHitWorld )
{
    vec3_t eye;
    //
    VectorCopy( pos, eye );
    eye[2] += level.clients[entnum].ps.viewheight;
    
    //if( BotEntityVisible( entnum, eye, vec3_origin, 360, enemy, NULL ) )
    //{
    //    return true;
    //}
    //
    return false;
}
