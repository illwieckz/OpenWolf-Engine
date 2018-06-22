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
// File name:   g_bot.cpp
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#include <sgame/sg_precompiled.h>

static S32 g_numBots;
static UTF8* g_botInfos[MAX_BOTS];

vmCvar_t bot_minplayers;

#define MAX_NUMBER_BOTS 20
#define BOT_SPAWN_QUEUE_DEPTH 16

typedef struct
{
    S32 clientNum;
    S32 spawnTime;
} botSpawnQueue_t;

static botSpawnQueue_t  botSpawnQueue[BOT_SPAWN_QUEUE_DEPTH];

/*
===============
idGameLocal::BotDel
===============
*/
void idGameLocal::BotDel( gentity_t* ent, S32 clientNum )
{
    gentity_t* bot;
    StringEntry  teamstring;
    
    bot = &g_entities[clientNum];
    
    if( !( bot->r.svFlags & SVF_BOT ) )
    {
        adminLocal.ADMP( va( "'^7%s^7' is not a bot\n", bot->client->pers.netname ) );
        return;
    }
    
    switch( bot->client->pers.teamSelection )
    {
        case TEAM_ALIENS:
            teamstring = "^1Alien team^7";
            break;
        case TEAM_HUMANS:
            teamstring = "^4Human team^7";
            break;
        case TEAM_NONE:
            teamstring = "^2Spectator team^7";
            break;
        default:
            teamstring = "Team error - you shouldn't see this";
            break;
    }
    
    level.numBots--;
    gameLocal.ClientDisconnect( clientNum );
    
    //this needs to be done to free up the client slot - I think - Ender Feb 18 2008
    trap_DropClient( clientNum, va( "was deleted by ^7%s^7 from the %s^7\n\"", ( ent ) ? ent->client->pers.netname : "The Console" , teamstring ), 0 );
}

/*
===============
idGameLocal::BotRemoveAll
===============
*/
void idGameLocal::BotRemoveAll( gentity_t* ent )
{
    S32 i;
    
    for( i = 0; i < level.num_entities; i++ )
    {
        if( g_entities[ i ].r.svFlags & SVF_BOT )
        {
            BotDel( ent, i );
        }
    }
    
    AP( va( "print \"^3!bot removeall: ^7%s^7 deleted all bots\n\"", ( ent ) ? ent->client->pers.netname : "The Console" ) );
}

/*
===============
idGameLocal::RemoveQueuedBotBegin

Called on client disconnect to make sure the delayed spawn
doesn't happen on a freed index
===============
*/
void idGameLocal::RemoveQueuedBotBegin( S32 clientNum )
{
    S32   n;
    
    for( n = 0; n < BOT_SPAWN_QUEUE_DEPTH; n++ )
    {
        if( botSpawnQueue[n].clientNum == clientNum )
        {
            botSpawnQueue[n].spawnTime = 0;
            return;
        }
    }
}

/*
===============
idGameLocal::AddRandomBot
===============
*/
gentity_t* idGameLocal::AddRandomBot( gentity_t* ent, S32 team , UTF8* name, F32* skill )
{
    S32   i, n, num;
    //F32 skill;
    UTF8*  value, netname[36], *teamstr;
    UTF8*      botinfo;
    gclient_t* cl;
    
    num = 0;
    for( n = 0; n < g_numBots ; n++ )
    {
        value = Info_ValueForKey( g_botInfos[n], "name" );
        
        for( i = 0 ; i < g_maxclients.integer ; i++ )
        {
            cl = level.clients + i;
            
            if( cl->pers.connected != CON_CONNECTED )
            {
                continue;
            }
            
            if( !( g_entities[cl->ps.clientNum].r.svFlags & SVF_BOT ) )
            {
                continue;
            }
            
            if( team >= 0 && cl->pers.teamSelection != team )
            {
                continue;
            }
            
            if( !Q_stricmp( value, cl->pers.netname ) )
            {
                break;
            }
        }
        
        if( i >= g_maxclients.integer )
        {
            num++;
        }
    }
    
    num = random() * num;
    
    for( n = 0; n < g_numBots ; n++ )
    {
        value = Info_ValueForKey( g_botInfos[n], "name" );
        
        for( i = 0 ; i < g_maxclients.integer ; i++ )
        {
            cl = level.clients + i;
            
            if( cl->pers.connected != CON_CONNECTED )
            {
                continue;
            }
            
            if( !( g_entities[cl->ps.clientNum].r.svFlags & SVF_BOT ) )
            {
                continue;
            }
            
            if( team >= 0 && cl->pers.teamSelection != team )
            {
                continue;
            }
            if( !Q_stricmp( value, cl->pers.netname ) )
            {
                break;
            }
        }
        if( i >= g_maxclients.integer )
        {
            num--;
            if( num <= 0 )
            {
                if( *skill == -1.0 )
                {
                    *skill = 2.0;
                }
                //trap_Cvar_VariableValue( "g_spSkill" );
                if( team == TEAM_ALIENS )
                {
                    teamstr = "aliens";
                }
                else if( team == TEAM_HUMANS )
                {
                    teamstr = "humans";
                }
                else
                {
                    teamstr = "";
                }
                
                strncpy( netname, value, sizeof( netname ) - 1 );
                netname[sizeof( netname ) - 1] = '\0';
                
                Q_CleanStr( netname );
                
                botinfo = GetBotInfoByName( netname );
                
                //trap_SendConsoleCommand( EXEC_INSERT, va("addbot %s %f %s %i\n", netname, skill, teamstr, 0) );
                
                return BotAdd( ent, ( name ) ? name : netname, team, skill, botinfo );
            }
        }
    }
    return NULL;
}

/*
===============
idGameLocal::RemoveRandomBot
===============
*/
S32 idGameLocal::RemoveRandomBot( S32 team )
{
    gclient_t* cl;
    S32 i;
    
    for( i = 0 ; i < g_maxclients.integer ; i++ )
    {
        cl = level.clients + i;
        if( cl->pers.connected != CON_CONNECTED )
        {
            continue;
        }
        
        if( !( g_entities[cl->ps.clientNum].r.svFlags & SVF_BOT ) )
        {
            continue;
        }
        
        if( team >= 0 && cl->ps.stats[STAT_TEAM] != team )
        {
            continue;
        }
        
        //strcpy(netname, cl->pers.netname);
        //Q_CleanStr(netname);
        //trap_SendConsoleCommand( EXEC_INSERT, va("kick %s\n", netname) );
        trap_SendConsoleCommand( EXEC_INSERT, va( "clientkick %d\n", cl->ps.clientNum ) );
        return true;
    }
    return false;
}

/*
===============
idGameLocal::CountHumanPlayers
===============
*/
S32 idGameLocal::CountHumanPlayers( S32 team )
{
    S32 i, num;
    gclient_t* cl;
    
    num = 0;
    
    for( i = 0 ; i < g_maxclients.integer ; i++ )
    {
        cl = level.clients + i;
        
        if( cl->pers.connected != CON_CONNECTED )
        {
            continue;
        }
        if( g_entities[cl->ps.clientNum].r.svFlags & SVF_BOT )
        {
            continue;
        }
        if( team >= 0 && cl->ps.stats[ STAT_TEAM ] != team )
        {
            continue;
        }
        num++;
    }
    
    return num;
}

/*
===============
idGameLocal::CountBotPlayers
===============
*/
S32 idGameLocal::CountBotPlayers( S32 team )
{
    S32 i, n, num;
    gclient_t* cl;
    
    num = 0;
    for( i = 0 ; i < g_maxclients.integer ; i++ )
    {
        cl = level.clients + i;
        
        if( cl->pers.connected != CON_CONNECTED )
        {
            continue;
        }
        
        if( !( g_entities[cl->ps.clientNum].r.svFlags & SVF_BOT ) )
        {
            continue;
        }
        
        if( team >= 0 && cl->ps.stats[ STAT_TEAM ] != team )
        {
            continue;
        }
        num++;
    }
    
    for( n = 0; n < BOT_SPAWN_QUEUE_DEPTH; n++ )
    {
        if( !botSpawnQueue[n].spawnTime )
        {
            continue;
        }
        if( botSpawnQueue[n].spawnTime > level.time )
        {
            continue;
        }
        num++;
    }
    
    return num;
}

/*
===============
idGameLocal::CheckMinimumPlayers
===============
*/
void idGameLocal::CheckMinimumPlayers( void )
{
    S32 minaliens, minhumans;
    S32 humanplayers, botplayers;
    static S32 checkminimumplayers_time;
    F32 skill_float;
    
    if( level.intermissiontime ) return;
    
    //only check once each 5 seconds
    if( checkminimumplayers_time > level.time - 1000 )
    {
        return;
    }
    checkminimumplayers_time = level.time;
    
    // check alien team
    trap_Cvar_Update( &bot_minaliens );
    minaliens = bot_minaliens.integer;
    
    if( minaliens <= 0 )
    {
        return;
    }
    
    if( minaliens >= g_maxclients.integer / 2 )
    {
        minaliens = ( g_maxclients.integer / 2 ) - 1;
    }
    
    humanplayers = CountHumanPlayers( TEAM_ALIENS );
    botplayers = CountBotPlayers( TEAM_ALIENS );
    //
    if( humanplayers + botplayers < minaliens )
    {
        skill_float = -1;
        AddRandomBot( NULL, TEAM_ALIENS, NULL, &skill_float );
    }
    else if( humanplayers + botplayers > minaliens && botplayers )
    {
        RemoveRandomBot( TEAM_ALIENS );
    }
    
    // check human team
    trap_Cvar_Update( &bot_minhumans );
    minhumans = bot_minhumans.integer;
    
    if( minhumans <= 0 )
    {
        return;
    }
    
    if( minhumans >= g_maxclients.integer / 2 )
    {
        minhumans = ( g_maxclients.integer / 2 ) - 1;
    }
    
    humanplayers = CountHumanPlayers( TEAM_HUMANS );
    botplayers = CountBotPlayers( TEAM_HUMANS );
    
    if( humanplayers + botplayers < minhumans )
    {
        skill_float = -1;
        AddRandomBot( NULL, TEAM_HUMANS , NULL, &skill_float );
    }
    else if( humanplayers + botplayers > minhumans && botplayers )
    {
        RemoveRandomBot( TEAM_HUMANS );
    }
}

/*
===============
idGameLocal::CheckBotSpawn
===============
*/
void idGameLocal::CheckBotSpawn( void )
{
    S32 n;
    
    CheckMinimumPlayers();
    
    for( n = 0; n < BOT_SPAWN_QUEUE_DEPTH; n++ )
    {
        if( !botSpawnQueue[n].spawnTime )
        {
            continue;
        }
        
        if( botSpawnQueue[n].spawnTime > level.time )
        {
            continue;
        }
        
        ClientBegin( botSpawnQueue[n].clientNum );
        botSpawnQueue[n].spawnTime = 0;
    }
}

/*
===============
idGameLocal::BotConnect
===============
*/
bool idGameLocal::BotConnect( S32 clientNum, bool restart )
{
    bot_settings_t  settings;
    UTF8      teamstr[MAX_FILEPATH];
    UTF8      userinfo[MAX_INFO_STRING];
    S32       team;
    
    trap_GetUserinfo( clientNum, userinfo, sizeof( userinfo ) );
    
    Q_strncpyz( settings.characterfile, Info_ValueForKey( userinfo, "characterfile" ), sizeof( settings.characterfile ) );
    settings.skill = atof( Info_ValueForKey( userinfo, "skill" ) );
    Q_strncpyz( settings.team, Info_ValueForKey( userinfo, "team" ), sizeof( settings.team ) );
    
    // At this point, it's quite possible that the bot doesn't have a team, as in it
    // was just created
    if( !Q_stricmp( teamstr, "aliens" ) )
    {
        team = TEAM_ALIENS;
    }
    else if( !Q_stricmp( teamstr, "humans" ) )
    {
        team = TEAM_HUMANS;
    }
    else
    {
        team = TEAM_NONE;
    }
    
    if( !botLocal.BotAISetupClient( clientNum, &settings, restart ) )
    {
        trap_DropClient( clientNum, "idGameLocal::BotConnect failed", 0 );
        return false;
    }
    
    return true;
}

/*
===============
idGameLocal::ParseInfos
===============
*/
S32 idGameLocal::ParseInfos( UTF8* buf, S32 max, UTF8* infos[] )
{
    UTF8*  token;
    S32   count;
    UTF8  key[MAX_TOKEN_CHARS];
    UTF8  info[MAX_INFO_STRING];
    
    count = 0;
    
    while( 1 )
    {
        token = COM_Parse( &buf );
        if( !token[0] )
        {
            break;
        }
        if( strcmp( token, "{" ) )
        {
            Com_Printf( "Missing { in info file\n" );
            break;
        }
        
        if( count == max )
        {
            Com_Printf( "Max infos exceeded\n" );
            break;
        }
        
        info[0] = '\0';
        while( 1 )
        {
            token = COM_ParseExt( &buf, true );
            if( !token[0] )
            {
                Com_Printf( "Unexpected end of info file\n" );
                break;
            }
            if( !strcmp( token, "}" ) )
            {
                break;
            }
            Q_strncpyz( key, token, sizeof( key ) );
            
            token = COM_ParseExt( &buf, false );
            if( !token[0] )
            {
                strcpy( token, "<NULL>" );
            }
            Info_SetValueForKey( info, key, token );
        }
        
        //NOTE: extra space for arena number
        infos[count] = ( UTF8* )bggame->Alloc( strlen( info ) + strlen( "\\num\\" ) + strlen( va( "%d", MAX_ARENAS ) ) + 1 );
        if( infos[count] )
        {
            strcpy( infos[count], info );
            count++;
        }
    }
    
    return count;
}

/*
===============
idGameLocal::LoadBotsFromFile
===============
*/
void idGameLocal::LoadBotsFromFile( UTF8* filename )
{
    S32 len;
    fileHandle_t  f;
    UTF8 buf[MAX_BOTS_TEXT];
    
    len = trap_FS_FOpenFile( filename, &f, FS_READ );
    
    if( !f )
    {
        Printf( S_COLOR_RED "file not found: %s\n", filename );
        return;
    }
    
    if( len >= MAX_BOTS_TEXT )
    {
        Printf( S_COLOR_RED "file too large: %s is %i, max allowed is %i", filename, len, MAX_BOTS_TEXT );
        trap_FS_FCloseFile( f );
        return;
    }
    
    trap_FS_Read( buf, len, f );
    buf[len] = 0;
    trap_FS_FCloseFile( f );
    
    g_numBots += ParseInfos( buf, MAX_BOTS - g_numBots, &g_botInfos[g_numBots] );
}

/*
===============
idGameLocal::LoadBots
===============
*/
void idGameLocal::LoadBots( void )
{
    vmCvar_t botsFile;
    S32 numdirs;
    UTF8 filename[128];
    UTF8 dirlist[1024];
    UTF8* dirptr;
    S32 i, dirlen;
    
    if( !trap_Cvar_VariableIntegerValue( "bot_enable" ) )
    {
        return;
    }
    
    g_numBots = 0;
    
    trap_Cvar_Register( &botsFile, "g_botsFile", "", CVAR_INIT | CVAR_ROM );
    
    if( *botsFile.string )
    {
        LoadBotsFromFile( botsFile.string );
    }
    else
    {
        LoadBotsFromFile( "scripts/bots.txt" );
    }
    
    // get all bots from .bot files
    numdirs = trap_FS_GetFileList( "scripts", ".bot", dirlist, 1024 );
    dirptr  = dirlist;
    
    for( i = 0; i < numdirs; i++, dirptr += dirlen + 1 )
    {
        dirlen = strlen( dirptr );
        strcpy( filename, "scripts/" );
        strcat( filename, dirptr );
        LoadBotsFromFile( filename );
    }
    
    Printf( "%i bots parsed\n", g_numBots );
}

/*
===============
idGameLocal::GetBotInfoByName
===============
*/
UTF8* idGameLocal::GetBotInfoByName( StringEntry name )
{
    S32   n;
    UTF8*  value;
    
    Printf( " Searching %i bot infos for '%s'...\n", g_numBots, name );
    for( n = 0; n < g_numBots ; n++ )
    {
        value = Info_ValueForKey( g_botInfos[n], "name" );
        if( !Q_stricmp( value, name ) )
        {
            return g_botInfos[n];
        }
    }
    
    return NULL;
}

/*
===============
idGameLocal::InitBots
===============
*/
void idGameLocal::InitBots( bool restart )
{
    Printf( "------------- idGameLocal::InitBots --------------\n" );
    LoadBots();
    //LoadArenas();
    Printf( "--------------------------------------------------\n" );
}

/*
===============
idGameLocal::BotAdd
===============
*/
gentity_t* idGameLocal::BotAdd( gentity_t* ent, UTF8* name, S32 team, F32* skill, UTF8* botinfo )
{
    S32 botsuffixnumber;
    S32 clientNum;
    //UTF8 *botinfo;
    UTF8 userinfo[MAX_INFO_STRING];
    UTF8 err[ MAX_STRING_CHARS ];
    UTF8 newname[ MAX_NAME_LENGTH ];
    S32 reservedSlots = 0;
    UTF8*  teamstring, *teamstr;
    gentity_t* bot;
    
    if( bot_developer.integer )
    {
        Printf( "idGameLocal::BotAdd : Trying to add bot '%s'...\n", name );
    }
    reservedSlots = trap_Cvar_VariableIntegerValue( "sv_privateclients" );
    
    /*
    // get the botinfo from bots.txt
    botinfo = GetBotInfoByName( name );
    if ( !botinfo ) {
      Printf( S_COLOR_RED "Error: Bot '%s' not defined\n", name );
      return;
    }*/
    
    // find what clientNum to use for bot
    clientNum = trap_BotAllocateClient( 0 );
    if( clientNum == -1 )
    {
        Printf( S_COLOR_RED "idGameLocal::BotAdd : Unable to add bot.  All player slots are in use.\n" );
        Printf( S_COLOR_RED "idGameLocal::BotAdd : Start server with more 'open' slots (or check setting of sv_maxclients cvar).\n" );
        return NULL;
    }
    
    newname[0] = '\0';
    
    switch( team )
    {
        case TEAM_ALIENS:
            teamstring = "^1Alien team^7";
            teamstr = "aliens";
            Q_strcat( newname, sizeof( newname ), "^1[BOT]^7" );
            Q_strcat( newname, MAX_NAME_LENGTH, name );
            break;
        case TEAM_HUMANS:
            teamstring = "^4Human team";
            teamstr = "humans";
            Q_strcat( newname, sizeof( newname ), "^4[BOT]^7" );
            Q_strcat( newname, MAX_NAME_LENGTH, name );
            break;
        default:
            return NULL;
    }
    
    //now make sure that we can add bots of the same name, but just incremented
    //numerically. We'll now use name as a temp buffer, since we have the
    //real name in newname.
    botsuffixnumber = 1;
    
    if( !adminLocal.AdminNameCheck( NULL, newname, err, sizeof( err ) ) )
    {
        while( botsuffixnumber < MAX_NUMBER_BOTS )
        {
            strcpy( name, va( "%s%d", newname, botsuffixnumber ) );
            if( adminLocal.AdminNameCheck( NULL, name, err, sizeof( err ) ) )
            {
                strcpy( newname, name );
                break;
            }
            botsuffixnumber++; // Only increments if the last requested name was used.
        }
    }
    
    bot = &g_entities[ clientNum ];
    bot->r.svFlags |= SVF_BOT;
    bot->inuse = true;
    
    // register user information
    userinfo[0] = '\0';
    Info_SetValueForKey( userinfo, "characterfile", Info_ValueForKey( botinfo, "aifile" ) );
    Info_SetValueForKey( userinfo, "name", newname );
    Info_SetValueForKey( userinfo, "rate", "25000" );
    Info_SetValueForKey( userinfo, "snaps", "20" );
    Info_SetValueForKey( userinfo, "skill", va( "%f", *skill ) );
    Info_SetValueForKey( userinfo, "teamstr", teamstr );
    
    trap_SetUserinfo( clientNum, userinfo );
    
    // have it connect to the game as a normal client
    if( gameLocal.ClientConnect( clientNum, true, true ) != NULL )
    {
        // won't let us join
        return NULL;
    }
    if( team == TEAM_HUMANS )
    {
        AP( va( "print \"^3!bot add: ^7%s^7 added bot: %s^7 to the %s^7 with character: %s and with skill level: %.0f\n\"",
                ( ent ) ? ent->client->pers.netname : "The Console", bot->client->pers.netname, teamstring, Info_ValueForKey( botinfo, "name" ), *skill ) );
    }
    else
    {
        AP( va( "print \"^3!bot add: ^7%s^7 added bot: %s^7 to the %s^7\n\"", ( ent ) ? ent->client->pers.netname : "The Console" , bot->client->pers.netname, teamstring ) );
    }
    
    BotBegin( clientNum );
    ChangeTeam( bot, ( team_t )team );
    level.numBots++;
    
    return bot;
}

/*
===============
idGameLocal::ListChars
===============
*/
bool idGameLocal::ListChars( gentity_t* ent, S32 skiparg )
{
    S32   i, handle;
    UTF8  charname[ MAX_NAME_LENGTH ];
    UTF8*  botinfo;
    UTF8  skill[4];
    F32   skill_float = 2.0;
    
    adminLocal.ADMBP_begin();
    
    if( SayArgc() < 2 + skiparg )
    {
        adminLocal.ADMBP( va( "^3!listchars: ^7 List of bot characters\n" ) );
        for( i = 0; i < g_numBots ; i++ )
        {
            adminLocal.ADMBP( va( "%s \n", Info_ValueForKey( g_botInfos[i], "name" ) ) );
            //ADMBP( va( "%s \n", g_botInfos[i] ) );
        }
        
        adminLocal.ADMBP( va( "To get information on a specific character type: \n^3!listchars [name] (skill) ^7\n" ) );
    }
    else
    {
        SayArgv( 1 + skiparg, charname, sizeof( charname ) );
        
        if( SayArgc() < 3 + skiparg )
        {
            skill_float = 2;
        }
        else
        {
            SayArgv( 2 + skiparg, skill, sizeof( skill ) );
            skill_float = atof( skill );
        }
        if( skill_float > 5.0 || skill_float < 1.0 )
        {
            adminLocal.ADMP( "^3!listchars: ^7bot skill level must be between 1 and 5\n" );
            return false;
        }
        
        botinfo = GetBotInfoByName( charname );
        
        if( !botinfo )
        {
            adminLocal.ADMP( va( S_COLOR_RED "Error: Bot character '%s' not defined\n", charname ) );
            return false;
        }
        
        adminLocal.ADMBP( va( "^3!listchars: ^7 character info for '%s' with skill level: %.1f \n", charname, skill_float ) );
        handle = trap_BotLoadCharacter( Info_ValueForKey( botinfo, "aifile" ), skill_float );
        adminLocal.ADMBP( va( "^3 Aim skill:^7 %.2f \n", trap_Characteristic_BFloat( handle, CHARACTERISTIC_AIM_SKILL, 0, 1 ) ) );
        adminLocal.ADMBP( va( "^3 Aim Accuracy:^7 %.2f \n", trap_Characteristic_BFloat( handle, CHARACTERISTIC_AIM_ACCURACY, 0, 1 ) ) );
        adminLocal.ADMBP( va( "^3 Attack skill:^7 %.2f \n", trap_Characteristic_BFloat( handle, CHARACTERISTIC_ATTACK_SKILL, 0, 1 ) ) );
        adminLocal.ADMBP( va( "^3 Reaction time:^7 %.2fs \n", trap_Characteristic_BFloat( handle, CHARACTERISTIC_REACTIONTIME, 0, 1 ) ) );
        //ADMBP( va("^3 Reaction time:^7 %.2fs \n", trap_Characteristic_BFloat(handle, CHARACTERISTIC_REACTIONTIME, 0, 1) ) );
        trap_BotFreeCharacter( handle );
    }
    
    adminLocal.ADMBP_end();
    //Info_ValueForKey( g_botInfos[n], "name" );
    return true;
}
