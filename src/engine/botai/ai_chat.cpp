////////////////////////////////////////////////////////////////////////////
// Copyright(C) 1999 - 2005 Id Software, Inc.
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
// File name:   ai_chat.cpp
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////

#include <sgame/sg_precompiled.h>

#define TIME_BETWEENCHATTING	25

/*
==================
idBotLocal::BotAI_BotInitialChat
==================
*/
void idBotLocal::BotAI_BotInitialChat( bot_state_t* bs, UTF8* type, ... )
{
    S32 i;
    va_list	ap;
    UTF8* p;
    UTF8* vars[MAX_MATCHVARIABLES];
    
    ::memset( vars, 0, sizeof( vars ) );
    
    va_start( ap, type );
    p = va_arg( ap, UTF8* );
    for( i = 0; i < MAX_MATCHVARIABLES; i++ )
    {
        if( !p )
        {
            break;
        }
        vars[i] = p;
        p = va_arg( ap, UTF8* );
    }
    va_end( ap );
    
    if( bot_developer.integer )
    {
        Bot_Print( BPMSG, "calling trap_BotInitialChat( %d, %s, %d,  %s, %s, %s, %s, %s, %s, %s, %s\n", bs->cs, type,
                   CONTEXT_NORMAL, vars[0], vars[1], vars[2], vars[3], vars[4], vars[5], vars[6], vars[7] );
    }
    
    trap_BotInitialChat( bs->cs, type, CONTEXT_NORMAL, vars[0], vars[1], vars[2], vars[3], vars[4], vars[5], vars[6], vars[7] );
}

/*
==================
idBotLocal::EntityCarriesFlag
==================
*/
bool idBotLocal::EntityCarriesFlag( aas_entityinfo_t* entinfo )
{
    return false;
}

/*
==================
idBotLocal::BotNumActivePlayers
==================
*/
S32 idBotLocal::BotNumActivePlayers( void )
{
    S32 i, num;
    UTF8 buf[MAX_INFO_STRING];
    static S32 maxclients;
    
    if( !maxclients )
    {
        maxclients = trap_Cvar_VariableIntegerValue( "sv_maxclients" );
    }
    
    num = 0;
    for( i = 0; i < maxclients && i < MAX_CLIENTS; i++ )
    {
        trap_GetConfigstring( CS_PLAYERS + i, buf, sizeof( buf ) );
        
        // if no config string or no name
        if( !strlen( buf ) || !strlen( Info_ValueForKey( buf, "n" ) ) )
        {
            continue;
        }
        
        // skip spectators
        if( atoi( Info_ValueForKey( buf, "t" ) ) == TEAM_NONE )
        {
            continue;
        }
        
        num++;
    }
    
    return num;
}

/*
==================
idBotLocal::BotIsFirstInRankings
==================
*/
S32 idBotLocal::BotIsFirstInRankings( bot_state_t* bs )
{
    S32 i, score;
    UTF8 buf[MAX_INFO_STRING];
    static S32 maxclients;
    playerState_t ps;
    
    if( !maxclients )
    {
        maxclients = trap_Cvar_VariableIntegerValue( "sv_maxclients" );
    }
    
    score = bs->cur_ps.persistant[PERS_SCORE];
    for( i = 0; i < maxclients && i < MAX_CLIENTS; i++ )
    {
        trap_GetConfigstring( CS_PLAYERS + i, buf, sizeof( buf ) );
        
        // if no config string or no name
        if( !strlen( buf ) || !strlen( Info_ValueForKey( buf, "n" ) ) )
        {
            continue;
        }
        
        // skip spectators
        if( atoi( Info_ValueForKey( buf, "t" ) ) == TEAM_NONE )
        {
            continue;
        }
        
        BotAI_GetClientState( i, &ps );
        if( score < ps.persistant[PERS_SCORE] )
        {
            return false;
        }
    }
    
    return true;
}

/*
==================
idBotLocal::BotIsLastInRankings
==================
*/
S32 idBotLocal::BotIsLastInRankings( bot_state_t* bs )
{
    S32 i, score;
    UTF8 buf[MAX_INFO_STRING];
    static S32 maxclients;
    playerState_t ps;
    
    if( !maxclients )
    {
        maxclients = trap_Cvar_VariableIntegerValue( "sv_maxclients" );
    }
    
    score = bs->cur_ps.persistant[PERS_SCORE];
    for( i = 0; i < maxclients && i < MAX_CLIENTS; i++ )
    {
        trap_GetConfigstring( CS_PLAYERS + i, buf, sizeof( buf ) );
        
        // if no config string or no name
        if( !strlen( buf ) || !strlen( Info_ValueForKey( buf, "n" ) ) )
        {
            continue;
        }
        
        // skip spectators
        if( atoi( Info_ValueForKey( buf, "t" ) ) == TEAM_NONE )
        {
            continue;
        }
        
        BotAI_GetClientState( i, &ps );
        if( score > ps.persistant[PERS_SCORE] )
        {
            return false;
        }
    }
    
    return true;
}

/*
==================
idBotLocal::BotFirstClientInRankings
==================
*/
UTF8* idBotLocal::BotFirstClientInRankings( void )
{
    S32 i, bestscore, bestclient;
    UTF8 buf[MAX_INFO_STRING];
    static UTF8 name[32];
    static S32 maxclients;
    playerState_t ps;
    
    if( !maxclients )
    {
        maxclients = trap_Cvar_VariableIntegerValue( "sv_maxclients" );
    }
    
    bestscore = -999999;
    bestclient = 0;
    for( i = 0; i < maxclients && i < MAX_CLIENTS; i++ )
    {
        trap_GetConfigstring( CS_PLAYERS + i, buf, sizeof( buf ) );
        
        // if no config string or no name
        if( !strlen( buf ) || !strlen( Info_ValueForKey( buf, "n" ) ) )
        {
            continue;
        }
        
        // skip spectators
        if( atoi( Info_ValueForKey( buf, "t" ) ) == TEAM_NONE )
        {
            continue;
        }
        
        BotAI_GetClientState( i, &ps );
        
        if( ps.persistant[PERS_SCORE] > bestscore )
        {
            bestscore = ps.persistant[PERS_SCORE];
            bestclient = i;
        }
    }
    
    EasyClientName( bestclient, name, 32 );
    return name;
}

/*
==================
idBotLocal::BotLastClientInRankings
==================
*/
UTF8* idBotLocal::BotLastClientInRankings( void )
{
    S32 i, worstscore, bestclient;
    UTF8 buf[MAX_INFO_STRING];
    static UTF8 name[32];
    static S32 maxclients;
    playerState_t ps;
    
    if( !maxclients )
    {
        maxclients = trap_Cvar_VariableIntegerValue( "sv_maxclients" );
    }
    
    worstscore = 999999;
    bestclient = 0;
    for( i = 0; i < maxclients && i < MAX_CLIENTS; i++ )
    {
        trap_GetConfigstring( CS_PLAYERS + i, buf, sizeof( buf ) );
        
        // if no config string or no name
        if( !strlen( buf ) || !strlen( Info_ValueForKey( buf, "n" ) ) )
        {
            continue;
        }
        
        // skip spectators
        if( atoi( Info_ValueForKey( buf, "t" ) ) == TEAM_NONE )
        {
            continue;
        }
        
        BotAI_GetClientState( i, &ps );
        
        if( ps.persistant[PERS_SCORE] < worstscore )
        {
            worstscore = ps.persistant[PERS_SCORE];
            bestclient = i;
        }
    }
    
    EasyClientName( bestclient, name, 32 );
    return name;
}

/*
==================
idBotLocal::BotRandomOpponentName
==================
*/
UTF8* idBotLocal::BotRandomOpponentName( bot_state_t* bs )
{
    S32 i, count;
    UTF8 buf[MAX_INFO_STRING];
    S32 opponents[MAX_CLIENTS], numopponents;
    static S32 maxclients;
    static UTF8 name[32];
    
    if( !maxclients )
    {
        maxclients = trap_Cvar_VariableIntegerValue( "sv_maxclients" );
    }
    
    numopponents = 0;
    opponents[0] = 0;
    for( i = 0; i < maxclients && i < MAX_CLIENTS; i++ )
    {
        if( i == bs->client )
        {
            continue;
        }
        
        trap_GetConfigstring( CS_PLAYERS + i, buf, sizeof( buf ) );
        
        //if no config string or no name
        if( !strlen( buf ) || !strlen( Info_ValueForKey( buf, "n" ) ) )
        {
            continue;
            
        }
        //skip spectators
        if( atoi( Info_ValueForKey( buf, "t" ) ) == TEAM_NONE )
        {
            continue;
        }
        
        //skip team mates
        if( BotSameTeam( bs, i ) )
        {
            continue;
        }
        opponents[numopponents] = i;
        numopponents++;
    }
    
    count = random() * numopponents;
    for( i = 0; i < numopponents; i++ )
    {
        count--;
        if( count <= 0 )
        {
            EasyClientName( opponents[i], name, sizeof( name ) );
            return name;
        }
    }
    
    EasyClientName( opponents[0], name, sizeof( name ) );
    return name;
}

/*
==================
idBotLocal::BotMapTitle
==================
*/
UTF8* idBotLocal::BotMapTitle( void )
{
    UTF8 info[1024];
    static UTF8 mapname[128];
    
    trap_GetServerinfo( info, sizeof( info ) );
    
    ::strncpy( mapname, Info_ValueForKey( info, "mapname" ), sizeof( mapname ) - 1 );
    mapname[sizeof( mapname ) - 1] = '\0';
    
    return mapname;
}

/*
==================
idBotLocal::BotWeaponNameForMeansOfDeath
==================
*/
UTF8* idBotLocal::BotWeaponNameForMeansOfDeath( S32 mod )
{
    switch( mod )
    {
        case MOD_SHOTGUN:
            return "Shotgun";
        case MOD_BLASTER:
            return "Blaster";
        case MOD_PAINSAW:
            return "Painsaw";
        case MOD_MACHINEGUN:
            return "Machinegun";
        case MOD_CHAINGUN:
            return "Chaingun";
        case MOD_PRIFLE:
            return "Prifle";
        case MOD_MDRIVER:
            return "Mdriver";
        case MOD_LASGUN:
            return "Lastergun";
        case MOD_LCANNON:
            return "Lcannon";
        case MOD_FLAMER:
            return "Flamer";
        case MOD_GRENADE:
            return "Grenade";
        case MOD_LEVEL0_BITE:
            return "Bite";
        case MOD_LEVEL1_CLAW:
            return "Claw";
        case MOD_LEVEL1_PCLOUD:
            return "PCloud";
        case MOD_LEVEL3_CLAW:
            return "Claw";
        case MOD_LEVEL3_POUNCE:
            return "Pounce";
        case MOD_LEVEL3_BOUNCEBALL:
            return "Bouncball";
        case MOD_LEVEL2_CLAW:
            return "Claw";
        case MOD_LEVEL2_ZAP:
            return "Zap";
        case MOD_LEVEL4_CLAW:
            return "Claw";
        case MOD_LEVEL4_TRAMPLE:
            return "Trample";
        case MOD_LEVEL4_CRUSH:
            return "Crush";
        case MOD_SLOWBLOB:
            return "Blob";
        case MOD_POISON:
            return "Poison";
        case MOD_HSPAWN:
            return "Spawn";
        case MOD_TESLAGEN:
            return "Tesla";
        case MOD_MGTURRET:
            return "Turret";
        case MOD_REACTOR:
            return "Reactor";
        case MOD_ASPAWN:
            return "Aspawn";
        case MOD_ATUBE:
            return "Acid Tube";
        case MOD_OVERMIND:
            return "Overmind";
            //Dushan - maybe more?
            
        default:
            return "[unknown weapon]";
    }
}

/*
==================
idBotLocal::BotRandomWeaponName
==================
*/
UTF8* idBotLocal::BotRandomWeaponName( void )
{
    S32 rnd = random() * 8.9;
    
    switch( rnd )
    {
        case 0:
            return "Painsaw";
        case 1:
            return "Shotgun";
        case 2:
            return "Machinegun";
        case 3:
            return "Lasgun";
        case 4:
            return "Pulse Rifle";
        case 5:
            return "Mass driver";
        case 6:
            return "Railgun";
        case 7:
            return "Lightning Gun";
        case 8:
            return "Nailgun";
        case 9:
            return "Chaingun";
        case 10:
            return "Flammer";
        default:
            return "Luci Cannon";
    }
}

/*
==================
idBotLocal::BotVisibleEnemies
==================
*/
S32 idBotLocal::BotVisibleEnemies( bot_state_t* bs )
{
    F32 vis;
    S32 i;
    aas_entityinfo_t entinfo;
    
    for( i = 0; i < MAX_CLIENTS; i++ )
    {
    
        if( i == bs->client )
        {
            continue;
        }
        BotEntityInfo( i, &entinfo );
        
        if( !entinfo.valid )
        {
            continue;
        }
        
        //if the enemy isn't dead and the enemy isn't the bot self
        if( EntityIsDead( &entinfo ) || entinfo.number == bs->entitynum )
        {
            continue;
        }
        
        //if the enemy is invisible and not shooting
        if( EntityIsInvisible( &entinfo ) && !EntityIsShooting( &entinfo ) )
        {
            continue;
        }
        
        //if on the same team
        if( BotSameTeam( bs, i ) )
        {
            continue;
        }
        
        //check if the enemy is visible
        vis = BotEntityVisible( bs->entitynum, bs->eye, bs->viewangles, 360, i );
        if( vis > 0 )
        {
            return true;
        }
    }
    return false;
}

/*
==================
idBotLocal::BotValidChatPosition
==================
*/
S32 idBotLocal::BotValidChatPosition( bot_state_t* bs )
{
    vec3_t point, start, end, mins, maxs;
    bsp_trace_t trace;
    
    //if the bot is dead all positions are valid
    if( BotIsDead( bs ) )
    {
        return true;
    }
    
    if( bs->cur_ps.groundEntityNum != ENTITYNUM_NONE )
    {
        return false;
    }
    
    //do not chat if in lava or slime
    VectorCopy( bs->origin, point );
    point[2] -= 24;
    
    if( trap_PointContents( point, bs->entitynum ) & ( CONTENTS_LAVA | CONTENTS_SLIME ) )
    {
        return false;
    }
    
    //do not chat if under water
    VectorCopy( bs->origin, point );
    point[2] += 32;
    if( trap_PointContents( point, bs->entitynum ) & MASK_WATER )
    {
        return false;
    }
    
    //must be standing on the world entity
    VectorCopy( bs->origin, start );
    VectorCopy( bs->origin, end );
    start[2] += 1;
    end[2] -= 10;
    
    trap_AAS_PresenceTypeBoundingBox( PRESENCE_CROUCH, mins, maxs );
    BotAI_Trace( &trace, start, mins, maxs, end, bs->client, MASK_SOLID );
    
    if( trace.ent != ENTITYNUM_WORLD )
    {
        return false;
    }
    
    //the bot is in a position where it can chat
    return true;
}

/*
==================
idBotLocal::BotChat_EnterGame
==================
*/
S32 idBotLocal::BotChat_EnterGame( bot_state_t* bs )
{
    UTF8 name[32];
    F32 rnd;
    
    if( bot_nochat.integer )
    {
        return false;
    }
    if( bs->lastchat_time > FloatTime() - TIME_BETWEENCHATTING )
    {
        return false;
    }
    
    // don't chat in tournament mode
    rnd = trap_Characteristic_BFloat( bs->character, CHARACTERISTIC_CHAT_ENTEREXITGAME, 0, 1 );
    if( !bot_fastchat.integer )
    {
        if( random() > rnd )
        {
            return false;
        }
    }
    if( BotNumActivePlayers() <= 1 )
    {
        return false;
    }
    if( !BotValidChatPosition( bs ) )
    {
        return false;
    }
    
    BotAI_BotInitialChat( bs, "game_enter",
                          EasyClientName( bs->client, name, 32 ),   // 0
                          BotRandomOpponentName( bs ),              // 1
                          "[invalid var]",                          // 2
                          "[invalid var]",                          // 3
                          BotMapTitle(),                            // 4
                          NULL );
                          
    bs->lastchat_time = FloatTime();
    bs->chatto = CHAT_ALL;
    
    return true;
}

/*
==================
idBotLocal::BotChat_ExitGame
==================
*/
S32 idBotLocal::BotChat_ExitGame( bot_state_t* bs )
{
    UTF8 name[32];
    F32 rnd;
    
    if( bot_nochat.integer )
    {
        return false;
    }
    if( bs->lastchat_time > FloatTime() - TIME_BETWEENCHATTING )
    {
        return false;
    }
    
    //don't chat in teamplay
    //	if (TeamPlayIsOn()) return false;
    // don't chat in tournament mode
    //	if (gametype == GT_TOURNAMENT) return false;
    
    rnd = trap_Characteristic_BFloat( bs->character, CHARACTERISTIC_CHAT_ENTEREXITGAME, 0, 1 );
    if( !bot_fastchat.integer )
    {
        if( random() > rnd )
        {
            return false;
        }
    }
    
    if( BotNumActivePlayers() <= 1 )
    {
        return false;
    }
    
    BotAI_BotInitialChat( bs, "game_exit",
                          EasyClientName( bs->client, name, 32 ),	// 0
                          BotRandomOpponentName( bs ),				// 1
                          "[invalid var]",						    // 2
                          "[invalid var]",					    	// 3
                          BotMapTitle(),							// 4
                          NULL );
                          
    bs->lastchat_time = FloatTime();
    bs->chatto = CHAT_ALL;
    
    return true;
}

/*
==================
idBotLocal::BotChat_StartLevel
==================
*/
S32 idBotLocal::BotChat_StartLevel( bot_state_t* bs )
{
    UTF8 name[32];
    F32 rnd;
    
    if( bot_nochat.integer )
    {
        return false;
    }
    
    if( BotIsObserver( bs ) )
    {
        return false;
    }
    
    if( bs->lastchat_time > FloatTime() - TIME_BETWEENCHATTING )
    {
        return false;
    }
    
    rnd = trap_Characteristic_BFloat( bs->character, CHARACTERISTIC_CHAT_STARTENDLEVEL, 0, 1 );
    
    if( !bot_fastchat.integer )
    {
        if( random() > rnd )
        {
            return false;
        }
    }
    
    if( BotNumActivePlayers() <= 1 )
    {
        return false;
    }
    
    BotAI_BotInitialChat( bs, "level_start",  EasyClientName( bs->client, name, 32 ), NULL );
    
    bs->lastchat_time = FloatTime();
    bs->chatto = CHAT_ALL;
    return true;
}

/*
==================
idBotLocal::BotChat_EndLevel
==================
*/
S32 idBotLocal::BotChat_EndLevel( bot_state_t* bs )
{
    UTF8 name[32];
    F32 rnd;
    
    if( bot_nochat.integer )
    {
        return false;
    }
    
    if( BotIsObserver( bs ) )
    {
        return false;
    }
    
    if( bs->lastchat_time > FloatTime() - TIME_BETWEENCHATTING )
    {
        return false;
    }
    
    rnd = trap_Characteristic_BFloat( bs->character, CHARACTERISTIC_CHAT_STARTENDLEVEL, 0, 1 );
    
    if( !bot_fastchat.integer )
    {
        if( random() > rnd )
        {
            return false;
        }
    }
    
    if( BotNumActivePlayers() <= 1 )
    {
        return false;
    }
    
    if( BotIsFirstInRankings( bs ) )
    {
        BotAI_BotInitialChat( bs, "level_end_victory",
                              EasyClientName( bs->client, name, 32 ),	// 0
                              BotRandomOpponentName( bs ),				// 1
                              "[invalid var]",							// 2
                              BotLastClientInRankings(),				// 3
                              BotMapTitle(),							// 4
                              NULL );
    }
    else if( BotIsLastInRankings( bs ) )
    {
        BotAI_BotInitialChat( bs, "level_end_lose",
                              EasyClientName( bs->client, name, 32 ),	// 0
                              BotRandomOpponentName( bs ),				// 1
                              BotFirstClientInRankings(),				// 2
                              "[invalid var]",							// 3
                              BotMapTitle(),							// 4
                              NULL );
    }
    else
    {
        BotAI_BotInitialChat( bs, "level_end",
                              EasyClientName( bs->client, name, 32 ),	// 0
                              BotRandomOpponentName( bs ),				// 1
                              BotFirstClientInRankings(),				// 2
                              BotLastClientInRankings(),				// 3
                              BotMapTitle(),							// 4
                              NULL );
    }
    
    bs->lastchat_time = FloatTime();
    bs->chatto = CHAT_ALL;
    
    return true;
}

/*
==================
idBotLocal::BotChat_Death
==================
*/
S32 idBotLocal::BotChat_Death( bot_state_t* bs )
{
    UTF8 name[32];
    F32 rnd;
    
    //Bot_Print(BPMSG, "BotChat_Death called\n");
    
    if( bot_nochat.integer )
    {
        return false;
    }
    
    if( bs->lastchat_time > FloatTime() - TIME_BETWEENCHATTING )
    {
        return false;
    }
    
    rnd = trap_Characteristic_BFloat( bs->character, CHARACTERISTIC_CHAT_DEATH, 0, 1 );
    
    //if fast chatting is off
    if( !bot_fastchat.integer )
    {
        if( random() > rnd )
        {
            return false;
        }
    }
    
    if( BotNumActivePlayers() <= 1 )
    {
        return false;
    }
    
    if( bs->lastkilledby >= 0 && bs->lastkilledby < MAX_CLIENTS )
    {
        EasyClientName( bs->lastkilledby, name, 32 );
    }
    else
    {
        strcpy( name, "[world]" );
    }
    
    if( 0 )
    {
    
    }
    else
    {
        //teamplay
        if( bs->botdeathtype == MOD_WATER )
        {
            BotAI_BotInitialChat( bs, "death_drown", BotRandomOpponentName( bs ), NULL );
        }
        else if( bs->botdeathtype == MOD_SLIME )
        {
            BotAI_BotInitialChat( bs, "death_slime", BotRandomOpponentName( bs ), NULL );
        }
        else if( bs->botdeathtype == MOD_LAVA )
        {
            BotAI_BotInitialChat( bs, "death_lava", BotRandomOpponentName( bs ), NULL );
        }
        else if( bs->botdeathtype == MOD_FALLING )
        {
            BotAI_BotInitialChat( bs, "death_cratered", BotRandomOpponentName( bs ), NULL );
        }
        else if( bs->botdeathtype == MOD_TELEFRAG )
        {
            BotAI_BotInitialChat( bs, "death_telefrag", name, NULL );
        }
        else
        {
            /*if ((bs->botdeathtype == MOD_GAUNTLET ||
            		bs->botdeathtype == MOD_RAILGUN ||
            		bs->botdeathtype == MOD_BFG ||
            		bs->botdeathtype == MOD_BFG_SPLASH) && random() < 0.5) {
            
            
            		if (bs->botdeathtype == MOD_GAUNTLET)
            			BotAI_BotInitialChat(bs, "death_painsaw",
            					name,												// 0
            					BotWeaponNameForMeansOfDeath(bs->botdeathtype),		// 1
            					NULL);
            		else if (bs->botdeathtype == MOD_RAILGUN)
            			BotAI_BotInitialChat(bs, "death_rail",
            					name,												// 0
            					BotWeaponNameForMeansOfDeath(bs->botdeathtype),		// 1
            					NULL);
            		else
            			BotAI_BotInitialChat(bs, "death_bfg",
            					name,												// 0
            					BotWeaponNameForMeansOfDeath(bs->botdeathtype),		// 1
            					NULL);
            	}*/
            //choose between insult and praise
            /*else*/
            if( random() < trap_Characteristic_BFloat( bs->character, CHARACTERISTIC_CHAT_INSULT, 0, 1 ) )
            {
                BotAI_BotInitialChat( bs, "death_insult",
                                      name,													// 0
                                      BotWeaponNameForMeansOfDeath( bs->botdeathtype ),		// 1
                                      NULL );
            }
            else
            {
                BotAI_BotInitialChat( bs, "death_praise",
                                      name,													// 0
                                      BotWeaponNameForMeansOfDeath( bs->botdeathtype ),		// 1
                                      NULL );
            }
        }
        bs->chatto = CHAT_ALL;
    }
    
    bs->lastchat_time = FloatTime();
    return true;
}

/*
==================
idBotLocal::BotChat_Kill
==================
*/
S32 idBotLocal::BotChat_Kill( bot_state_t* bs )
{
    UTF8 name[32];
    F32 rnd;
    
    if( bot_nochat.integer )
    {
        return false;
    }
    if( bs->lastchat_time > FloatTime() - TIME_BETWEENCHATTING )
    {
        return false;
    }
    rnd = trap_Characteristic_BFloat( bs->character, CHARACTERISTIC_CHAT_KILL, 0, 1 );
    
    //if fast chat is off
    if( !bot_fastchat.integer )
    {
        if( random() > rnd )
        {
            return false;
        }
    }
    
    if( bs->lastkilledplayer == bs->client )
    {
        return false;
    }
    
    if( BotNumActivePlayers() <= 1 )
    {
        return false;
    }
    
    if( !BotValidChatPosition( bs ) )
    {
        return false;
    }
    
    if( BotVisibleEnemies( bs ) )
    {
        return false;
    }
    
    EasyClientName( bs->lastkilledplayer, name, 32 );
    
    bs->chatto = CHAT_ALL;
    
    if( 0 )
    {
    
    }
    else
    {
        if( bs->enemydeathtype == MOD_TELEFRAG )
        {
            BotAI_BotInitialChat( bs, "kill_telefrag", name, NULL );
        }
        //choose between insult and praise
        else if( random() < trap_Characteristic_BFloat( bs->character, CHARACTERISTIC_CHAT_INSULT, 0, 1 ) )
        {
            BotAI_BotInitialChat( bs, "kill_insult", name, NULL );
        }
        else
        {
            BotAI_BotInitialChat( bs, "kill_praise", name, NULL );
        }
    }
    
    bs->lastchat_time = FloatTime();
    return true;
}

/*
==================
idBotLocal::BotChat_EnemySuicide
==================
*/
S32 idBotLocal::BotChat_EnemySuicide( bot_state_t* bs )
{
    UTF8 name[32];
    F32 rnd;
    
    if( bot_nochat.integer )
    {
        return false;
    }
    
    if( bs->lastchat_time > FloatTime() - TIME_BETWEENCHATTING )
    {
        return false;
    }
    
    if( BotNumActivePlayers() <= 1 )
    {
        return false;
    }
    rnd = trap_Characteristic_BFloat( bs->character, CHARACTERISTIC_CHAT_KILL, 0, 1 );
    
    //if fast chat is off
    if( !bot_fastchat.integer )
    {
        if( random() > rnd )
        {
            return false;
        }
    }
    
    if( !BotValidChatPosition( bs ) )
    {
        return false;
    }
    
    if( BotVisibleEnemies( bs ) )
    {
        return false;
    }
    
    if( bs->enemy >= 0 )
    {
        EasyClientName( bs->enemy, name, 32 );
    }
    else
    {
        strcpy( name, "" );
    }
    
    BotAI_BotInitialChat( bs, "enemy_suicide", name, NULL );
    
    bs->lastchat_time = FloatTime();
    bs->chatto = CHAT_ALL;
    
    return true;
}

/*
==================
idBotLocal::BotChat_HitTalking
==================
*/
S32 idBotLocal::BotChat_HitTalking( bot_state_t* bs )
{
    UTF8 name[32], *weap;
    S32 lasthurt_client;
    F32 rnd;
    
    if( bot_nochat.integer )
    {
        return false;
    }
    
    if( bs->lastchat_time > FloatTime() - TIME_BETWEENCHATTING )
    {
        return false;
    }
    if( BotNumActivePlayers() <= 1 )
    {
        return false;
    }
    lasthurt_client = g_entities[bs->client].client->lasthurt_client;
    
    if( !lasthurt_client )
    {
        return false;
    }
    
    if( lasthurt_client == bs->client )
    {
        return false;
    }
    
    if( lasthurt_client < 0 || lasthurt_client >= MAX_CLIENTS )
    {
        return false;
    }
    
    rnd = trap_Characteristic_BFloat( bs->character, CHARACTERISTIC_CHAT_HITTALKING, 0, 1 );
    
    //if fast chat is off
    if( !bot_fastchat.integer )
    {
        if( random() > rnd * 0.5 )
        {
            return false;
        }
    }
    
    if( !BotValidChatPosition( bs ) )
    {
        return false;
    }
    
    ClientName( g_entities[bs->client].client->lasthurt_client, name, sizeof( name ) );
    weap = BotWeaponNameForMeansOfDeath( g_entities[bs->client].client->lasthurt_client );
    
    BotAI_BotInitialChat( bs, "hit_talking", name, weap, NULL );
    
    bs->lastchat_time = FloatTime();
    bs->chatto = CHAT_ALL;
    
    return true;
}

/*
==================
idBotLocal::BotChat_HitNoDeath
==================
*/
S32 idBotLocal::BotChat_HitNoDeath( bot_state_t* bs )
{
    UTF8 name[32], *weap;
    F32 rnd;
    S32 lasthurt_client;
    aas_entityinfo_t entinfo;
    
    lasthurt_client = g_entities[bs->client].client->lasthurt_client;
    
    if( !lasthurt_client )
    {
        return false;
    }
    if( lasthurt_client == bs->client )
    {
        return false;
    }
    
    if( lasthurt_client < 0 || lasthurt_client >= MAX_CLIENTS )
    {
        return false;
    }
    if( bot_nochat.integer )
    {
        return false;
    }
    if( bs->lastchat_time > FloatTime() - TIME_BETWEENCHATTING )
    {
        return false;
    }
    if( BotNumActivePlayers() <= 1 )
    {
        return false;
    }
    
    rnd = trap_Characteristic_BFloat( bs->character, CHARACTERISTIC_CHAT_HITNODEATH, 0, 1 );
    
    //if fast chat is off
    if( !bot_fastchat.integer )
    {
        if( random() > rnd * 0.5 )
        {
            return false;
        }
    }
    
    if( !BotValidChatPosition( bs ) )
    {
        return false;
    }
    
    if( BotVisibleEnemies( bs ) )
    {
        return false;
    }
    
    BotEntityInfo( bs->enemy, &entinfo );
    
    if( EntityIsShooting( &entinfo ) )
    {
        return false;
    }
    
    ClientName( lasthurt_client, name, sizeof( name ) );
    weap = BotWeaponNameForMeansOfDeath( g_entities[bs->client].client->lasthurt_mod );
    
    BotAI_BotInitialChat( bs, "hit_nodeath", name, weap, NULL );
    
    bs->lastchat_time = FloatTime();
    bs->chatto = CHAT_ALL;
    
    return true;
}

/*
==================
idBotLocal::BotChat_HitNoKill
==================
*/
S32 idBotLocal::BotChat_HitNoKill( bot_state_t* bs )
{
    UTF8 name[32], *weap;
    F32 rnd;
    aas_entityinfo_t entinfo;
    
    if( bot_nochat.integer )
    {
        return false;
    }
    if( bs->lastchat_time > FloatTime() - TIME_BETWEENCHATTING )
    {
        return false;
    }
    if( BotNumActivePlayers() <= 1 )
    {
        return false;
    }
    rnd = trap_Characteristic_BFloat( bs->character, CHARACTERISTIC_CHAT_HITNOKILL, 0, 1 );
    
    //if fast chat is off
    if( !bot_fastchat.integer )
    {
        if( random() > rnd * 0.5 )
        {
            return false;
        }
    }
    if( !BotValidChatPosition( bs ) )
    {
        return false;
    }
    
    if( BotVisibleEnemies( bs ) )
    {
        return false;
    }
    
    BotEntityInfo( bs->enemy, &entinfo );
    if( EntityIsShooting( &entinfo ) )
    {
        return false;
    }
    
    ClientName( bs->enemy, name, sizeof( name ) );
    weap = BotWeaponNameForMeansOfDeath( g_entities[bs->enemy].client->lasthurt_mod );
    
    BotAI_BotInitialChat( bs, "hit_nokill", name, weap, NULL );
    
    bs->lastchat_time = FloatTime();
    bs->chatto = CHAT_ALL;
    
    return true;
}

/*
==================
idBotLocal::BotChat_Random
==================
*/
S32 idBotLocal::BotChat_Random( bot_state_t* bs )
{
    F32 rnd;
    UTF8 name[32];
    
    if( bot_nochat.integer )
    {
        return false;
    }
    
    if( BotIsObserver( bs ) )
    {
        return false;
    }
    
    if( bs->lastchat_time > FloatTime() - TIME_BETWEENCHATTING )
    {
        return false;
    }
    
    //don't chat when doing something important :)
    rnd = trap_Characteristic_BFloat( bs->character, CHARACTERISTIC_CHAT_RANDOM, 0, 1 );
    //	if (random() > bs->thinktime * 0.1) return false;
    if( !bot_fastchat.integer )
    {
        if( random() > rnd )
        {
            return false;
        }
    }
    
    if( BotNumActivePlayers() <= 1 )
    {
        return false;
    }
    
    if( !BotValidChatPosition( bs ) )
    {
        return false;
    }
    
    if( BotVisibleEnemies( bs ) )
    {
        return false;
    }
    
    if( bs->lastkilledplayer == bs->client )
    {
        strcpy( name, BotRandomOpponentName( bs ) );
    }
    else
    {
        EasyClientName( bs->lastkilledplayer, name, sizeof( name ) );
    }
    
    if( random() < trap_Characteristic_BFloat( bs->character, CHARACTERISTIC_CHAT_MISC, 0, 1 ) )
    {
        BotAI_BotInitialChat( bs, "random_misc",
                              BotRandomOpponentName( bs ),	// 0
                              name,						    // 1
                              "[invalid var]",		    	// 2
                              "[invalid var]",				// 3
                              BotMapTitle(),				// 4
                              BotRandomWeaponName(),		// 5
                              NULL );
    }
    else
    {
        BotAI_BotInitialChat( bs, "random_insult",
                              BotRandomOpponentName( bs ),	// 0
                              name,							// 1
                              "[invalid var]",				// 2
                              "[invalid var]",				// 3
                              BotMapTitle(),				// 4
                              BotRandomWeaponName(),		// 5
                              NULL );
    }
    
    bs->lastchat_time = FloatTime();
    bs->chatto = CHAT_ALL;
    
    return true;
}

/*
==================
idBotLocal::BotChatTime
==================
*/
F32 idBotLocal::BotChatTime( bot_state_t* bs )
{
    S32 cpm;
    
    cpm = trap_Characteristic_BInteger( bs->character, CHARACTERISTIC_CHAT_CPM, 1, 4000 );
    
    return trap_BotChatLength( bs->cs ) * 30 / cpm;
}

/*
==================
idBotLocal::BotChatTest
==================
*/
void idBotLocal::BotChatTest( bot_state_t* bs )
{
    UTF8 name[32];
    UTF8* weap;
    S32 num, i;
    
    num = trap_BotNumInitialChats( bs->cs, "game_enter" );
    for( i = 0; i < num; i++ )
    {
        BotAI_BotInitialChat( bs, "game_enter",
                              EasyClientName( bs->client, name, 32 ),	// 0
                              BotRandomOpponentName( bs ),				// 1
                              "[invalid var]",							// 2
                              "[invalid var]",							// 3
                              BotMapTitle(),							// 4
                              NULL );
        trap_BotEnterChat( bs->cs, 0, CHAT_ALL );
    }
    
    num = trap_BotNumInitialChats( bs->cs, "game_exit" );
    
    for( i = 0; i < num; i++ )
    {
        BotAI_BotInitialChat( bs, "game_exit",
                              EasyClientName( bs->client, name, 32 ),	// 0
                              BotRandomOpponentName( bs ),				// 1
                              "[invalid var]",							// 2
                              "[invalid var]",							// 3
                              BotMapTitle(),							// 4
                              NULL );
        trap_BotEnterChat( bs->cs, 0, CHAT_ALL );
    }
    
    num = trap_BotNumInitialChats( bs->cs, "level_start" );
    
    for( i = 0; i < num; i++ )
    {
        BotAI_BotInitialChat( bs, "level_start",
                              EasyClientName( bs->client, name, 32 ),	// 0
                              NULL );
        trap_BotEnterChat( bs->cs, 0, CHAT_ALL );
    }
    
    num = trap_BotNumInitialChats( bs->cs, "level_end_victory" );
    
    for( i = 0; i < num; i++ )
    {
        BotAI_BotInitialChat( bs, "level_end_victory",
                              EasyClientName( bs->client, name, 32 ),	// 0
                              BotRandomOpponentName( bs ),				// 1
                              BotFirstClientInRankings(),				// 2
                              BotLastClientInRankings(),				// 3
                              BotMapTitle(),							// 4
                              NULL );
        trap_BotEnterChat( bs->cs, 0, CHAT_ALL );
    }
    
    num = trap_BotNumInitialChats( bs->cs, "level_end_lose" );
    
    for( i = 0; i < num; i++ )
    {
        BotAI_BotInitialChat( bs, "level_end_lose",
                              EasyClientName( bs->client, name, 32 ),	// 0
                              BotRandomOpponentName( bs ),				// 1
                              BotFirstClientInRankings(),				// 2
                              BotLastClientInRankings(),				// 3
                              BotMapTitle(),							// 4
                              NULL );
        trap_BotEnterChat( bs->cs, 0, CHAT_ALL );
    }
    
    num = trap_BotNumInitialChats( bs->cs, "level_end" );
    
    for( i = 0; i < num; i++ )
    {
        BotAI_BotInitialChat( bs, "level_end",
                              EasyClientName( bs->client, name, 32 ),	// 0
                              BotRandomOpponentName( bs ),				// 1
                              BotFirstClientInRankings(),				// 2
                              BotLastClientInRankings(),				// 3
                              BotMapTitle(),							// 4
                              NULL );
        trap_BotEnterChat( bs->cs, 0, CHAT_ALL );
    }
    
    EasyClientName( bs->lastkilledby, name, sizeof( name ) );
    
    num = trap_BotNumInitialChats( bs->cs, "death_drown" );
    for( i = 0; i < num; i++ )
    {
        //
        BotAI_BotInitialChat( bs, "death_drown", name, NULL );
        trap_BotEnterChat( bs->cs, 0, CHAT_ALL );
    }
    
    num = trap_BotNumInitialChats( bs->cs, "death_slime" );
    for( i = 0; i < num; i++ )
    {
        BotAI_BotInitialChat( bs, "death_slime", name, NULL );
        trap_BotEnterChat( bs->cs, 0, CHAT_ALL );
    }
    
    num = trap_BotNumInitialChats( bs->cs, "death_lava" );
    for( i = 0; i < num; i++ )
    {
        BotAI_BotInitialChat( bs, "death_lava", name, NULL );
        trap_BotEnterChat( bs->cs, 0, CHAT_ALL );
    }
    
    num = trap_BotNumInitialChats( bs->cs, "death_cratered" );
    for( i = 0; i < num; i++ )
    {
        BotAI_BotInitialChat( bs, "death_cratered", name, NULL );
        trap_BotEnterChat( bs->cs, 0, CHAT_ALL );
    }
    
    num = trap_BotNumInitialChats( bs->cs, "death_suicide" );
    for( i = 0; i < num; i++ )
    {
        BotAI_BotInitialChat( bs, "death_suicide", name, NULL );
        trap_BotEnterChat( bs->cs, 0, CHAT_ALL );
    }
    
    num = trap_BotNumInitialChats( bs->cs, "death_telefrag" );
    for( i = 0; i < num; i++ )
    {
        BotAI_BotInitialChat( bs, "death_telefrag", name, NULL );
        trap_BotEnterChat( bs->cs, 0, CHAT_ALL );
    }
    
    num = trap_BotNumInitialChats( bs->cs, "death_painsaw" );
    for( i = 0; i < num; i++ )
    {
        BotAI_BotInitialChat( bs, "death_painsaw",
                              name,												// 0
                              BotWeaponNameForMeansOfDeath( bs->botdeathtype ),	// 1
                              NULL );
        trap_BotEnterChat( bs->cs, 0, CHAT_ALL );
    }
    
    num = trap_BotNumInitialChats( bs->cs, "death_rail" );
    for( i = 0; i < num; i++ )
    {
        BotAI_BotInitialChat( bs, "death_rail",
                              name,												// 0
                              BotWeaponNameForMeansOfDeath( bs->botdeathtype ),	// 1
                              NULL );
        trap_BotEnterChat( bs->cs, 0, CHAT_ALL );
    }
    
    num = trap_BotNumInitialChats( bs->cs, "death_bfg" );
    for( i = 0; i < num; i++ )
    {
        BotAI_BotInitialChat( bs, "death_bfg",
                              name,												// 0
                              BotWeaponNameForMeansOfDeath( bs->botdeathtype ),	// 1
                              NULL );
        trap_BotEnterChat( bs->cs, 0, CHAT_ALL );
    }
    
    num = trap_BotNumInitialChats( bs->cs, "death_insult" );
    for( i = 0; i < num; i++ )
    {
        BotAI_BotInitialChat( bs, "death_insult",
                              name,												// 0
                              BotWeaponNameForMeansOfDeath( bs->botdeathtype ),	// 1
                              NULL );
        trap_BotEnterChat( bs->cs, 0, CHAT_ALL );
    }
    
    num = trap_BotNumInitialChats( bs->cs, "death_praise" );
    for( i = 0; i < num; i++ )
    {
        BotAI_BotInitialChat( bs, "death_praise",
                              name,												// 0
                              BotWeaponNameForMeansOfDeath( bs->botdeathtype ),	// 1
                              NULL );
        trap_BotEnterChat( bs->cs, 0, CHAT_ALL );
    }
    
    EasyClientName( bs->lastkilledplayer, name, 32 );
    
    num = trap_BotNumInitialChats( bs->cs, "kill_gauntlet" );
    for( i = 0; i < num; i++ )
    {
        //
        BotAI_BotInitialChat( bs, "kill_gauntlet", name, NULL );
        trap_BotEnterChat( bs->cs, 0, CHAT_ALL );
    }
    
    num = trap_BotNumInitialChats( bs->cs, "kill_rail" );
    for( i = 0; i < num; i++ )
    {
        BotAI_BotInitialChat( bs, "kill_rail", name, NULL );
        trap_BotEnterChat( bs->cs, 0, CHAT_ALL );
    }
    
    num = trap_BotNumInitialChats( bs->cs, "kill_telefrag" );
    for( i = 0; i < num; i++ )
    {
        BotAI_BotInitialChat( bs, "kill_telefrag", name, NULL );
        trap_BotEnterChat( bs->cs, 0, CHAT_ALL );
    }
    
    num = trap_BotNumInitialChats( bs->cs, "kill_insult" );
    for( i = 0; i < num; i++ )
    {
        BotAI_BotInitialChat( bs, "kill_insult", name, NULL );
        trap_BotEnterChat( bs->cs, 0, CHAT_ALL );
    }
    
    num = trap_BotNumInitialChats( bs->cs, "kill_praise" );
    for( i = 0; i < num; i++ )
    {
        BotAI_BotInitialChat( bs, "kill_praise", name, NULL );
        trap_BotEnterChat( bs->cs, 0, CHAT_ALL );
    }
    
    num = trap_BotNumInitialChats( bs->cs, "enemy_suicide" );
    for( i = 0; i < num; i++ )
    {
        BotAI_BotInitialChat( bs, "enemy_suicide", name, NULL );
        trap_BotEnterChat( bs->cs, 0, CHAT_ALL );
    }
    
    ClientName( g_entities[bs->client].client->lasthurt_client, name, sizeof( name ) );
    weap = BotWeaponNameForMeansOfDeath( g_entities[bs->client].client->lasthurt_client );
    
    num = trap_BotNumInitialChats( bs->cs, "hit_talking" );
    for( i = 0; i < num; i++ )
    {
        BotAI_BotInitialChat( bs, "hit_talking", name, weap, NULL );
        trap_BotEnterChat( bs->cs, 0, CHAT_ALL );
    }
    
    num = trap_BotNumInitialChats( bs->cs, "hit_nodeath" );
    for( i = 0; i < num; i++ )
    {
        BotAI_BotInitialChat( bs, "hit_nodeath", name, weap, NULL );
        trap_BotEnterChat( bs->cs, 0, CHAT_ALL );
    }
    
    num = trap_BotNumInitialChats( bs->cs, "hit_nokill" );
    for( i = 0; i < num; i++ )
    {
        BotAI_BotInitialChat( bs, "hit_nokill", name, weap, NULL );
        trap_BotEnterChat( bs->cs, 0, CHAT_ALL );
    }
    
    if( bs->lastkilledplayer == bs->client )
    {
        strcpy( name, BotRandomOpponentName( bs ) );
    }
    else
    {
        EasyClientName( bs->lastkilledplayer, name, sizeof( name ) );
    }
    
    num = trap_BotNumInitialChats( bs->cs, "random_misc" );
    for( i = 0; i < num; i++ )
    {
        //
        BotAI_BotInitialChat( bs, "random_misc",
                              BotRandomOpponentName( bs ),	// 0
                              name,							// 1
                              "[invalid var]",				// 2
                              "[invalid var]",				// 3
                              BotMapTitle(),				// 4
                              BotRandomWeaponName(),		// 5
                              NULL );
        trap_BotEnterChat( bs->cs, 0, CHAT_ALL );
    }
    
    num = trap_BotNumInitialChats( bs->cs, "random_insult" );
    for( i = 0; i < num; i++ )
    {
        BotAI_BotInitialChat( bs, "random_insult",
                              BotRandomOpponentName( bs ),	// 0
                              name,							// 1
                              "[invalid var]",				// 2
                              "[invalid var]",				// 3
                              BotMapTitle(),				// 4
                              BotRandomWeaponName(),		// 5
                              NULL );
        trap_BotEnterChat( bs->cs, 0, CHAT_ALL );
    }
}
