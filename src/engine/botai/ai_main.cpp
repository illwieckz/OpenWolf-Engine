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
// File name:   ai_main.cpp
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////

#include <sgame/sg_precompiled.h>

#ifndef MAX_PATH
#define MAX_PATH 144
#endif

//bot states
bot_state_t*	botstates[MAX_CLIENTS];

//number of bots
S32 numbots;

//floating point time
F32 floattime;

vmCvar_t bot_memorydump;
vmCvar_t bot_saveroutingcache;
vmCvar_t bot_pause;
vmCvar_t bot_report;
vmCvar_t bot_testsolid;
vmCvar_t bot_testclusters;
vmCvar_t bot_interbreedchar;
vmCvar_t bot_interbreedbots;
vmCvar_t bot_interbreedcycle;
vmCvar_t bot_interbreedwrite;

idBotLocal botLocal;

/*
==================
idBotLocal::BotAI_Print
==================
*/
void idBotLocal::BotAI_Print( S32 type, UTF8* fmt, ... )
{
    UTF8 str[2048];
    va_list ap;
    
    va_start( ap, fmt );
    Q_vsnprintf( str, sizeof( str ), fmt, ap );
    va_end( ap );
    
    switch( type )
    {
        case PRT_MESSAGE:
        {
            gameLocal.Printf( "%s", str );
            break;
        }
        case PRT_WARNING:
        {
            gameLocal.Printf( S_COLOR_YELLOW "Warning: %s", str );
            break;
        }
        case PRT_ERROR:
        {
            gameLocal.Printf( S_COLOR_RED "Error: %s", str );
            break;
        }
        case PRT_FATAL:
        {
            gameLocal.Printf( S_COLOR_RED "Fatal: %s", str );
            break;
        }
        case PRT_EXIT:
        {
            gameLocal.Error( S_COLOR_RED "Exit: %s", str );
            break;
        }
        default:
        {
            gameLocal.Printf( "unknown print type\n" );
            break;
        }
    }
}

/*
==============
idBotLocal::BotInputToUserCommand
==============
 */
void idBotLocal::BotInputToUserCommand( bot_input_t* bi, usercmd_t* ucmd, S32 delta_angles[3], S32 time )
{
    vec3_t angles, forward, right;
    S16 temp;
    S32 j, forwardmv, rightmv, upmv;
    
    //clear the whole structure
    ::memset( ucmd, 0, sizeof( usercmd_t ) );
    
    //Com_Printf("dir = %f %f %f speed = %f\n", bi->dir[0], bi->dir[1], bi->dir[2], bi->speed);
    //the duration for the user command in milli seconds
    ucmd->serverTime = time;
    
    //set the buttons
    if( bi->actionflags & ACTION_RESPAWN )
    {
        ucmd->buttons = BUTTON_ATTACK;
    }
    if( bi->actionflags & ACTION_ATTACK )
    {
        ucmd->buttons |= BUTTON_ATTACK;
    }
    if( bi->actionflags & ACTION_TALK )
    {
        ucmd->buttons |= BUTTON_TALK;
    }
    if( bi->actionflags & ACTION_GESTURE )
    {
        ucmd->buttons |= BUTTON_GESTURE;
    }
    if( bi->actionflags & ACTION_USE )
    {
        ucmd->buttons |= BUTTON_USE_HOLDABLE;
    }
    if( bi->actionflags & ACTION_WALK )
    {
        ucmd->buttons |= BUTTON_WALKING;
    }
    
    ucmd->weapon = bi->weapon;
    
    //set the view angles
    //NOTE: the ucmd->angles are the angles WITHOUT the delta angles
    ucmd->angles[PITCH] = ANGLE2SHORT( bi->viewangles[PITCH] );
    ucmd->angles[YAW] = ANGLE2SHORT( bi->viewangles[YAW] );
    ucmd->angles[ROLL] = ANGLE2SHORT( bi->viewangles[ROLL] );
    
    //subtract the delta angles
    for( j = 0; j < 3; j++ )
    {
        temp = ucmd->angles[j] - delta_angles[j];
        /*NOTE: disabled because temp should be mod first
        	if ( j == PITCH ) {
        		// don't let the player look up or down more than 90 degrees
        		if ( temp > 16000 ) temp = 16000;
        		else if ( temp < -16000 ) temp = -16000;
        	}
         */
        ucmd->angles[j] = temp;
    }
    
    //NOTE: movement is relative to the REAL view angles
    //get the horizontal forward and right vector
    //get the pitch in the range [-180, 180]
    if( bi->dir[2] )
    {
        angles[PITCH] = bi->viewangles[PITCH];
    }
    else
    {
        angles[PITCH] = 0;
    }
    
    angles[YAW] = bi->viewangles[YAW];
    angles[ROLL] = 0;
    AngleVectors( angles, forward, right, NULL );
    
    //bot input speed is in the range [0, 400]
    bi->speed = bi->speed * 127 / 400;
    
    //set the view independent movement
    forwardmv = DotProduct( forward, bi->dir ) * bi->speed;
    rightmv = DotProduct( right, bi->dir ) * bi->speed;
    upmv = fabs( forward[2] ) * bi->dir[2] * bi->speed;
    
    //normal keyboard movement
    if( bi->actionflags & ACTION_MOVEFORWARD )
    {
        forwardmv += 127;
    }
    
    if( bi->actionflags & ACTION_MOVEBACK )
    {
        forwardmv -= 127;
    }
    
    if( bi->actionflags & ACTION_MOVELEFT )
    {
        rightmv -= 127;
    }
    
    if( bi->actionflags & ACTION_MOVERIGHT )
    {
        rightmv += 127;
    }
    
    //jump/moveup
    if( bi->actionflags & ACTION_JUMP )
    {
        upmv += 127;
    }
    
    //crouch/movedown
    if( bi->actionflags & ACTION_CROUCH )
    {
        upmv -= 127;
    }
    
    ucmd->forwardmove = ( forwardmv > 127 ) ? 127 : forwardmv;
    ucmd->rightmove = ( rightmv > 127 ) ? 127 : rightmv;
    ucmd->upmove = ( upmv > 127 ) ? 127 : upmv;
    
    //Com_Printf("forward = %d right = %d up = %d\n", ucmd->forwardmove, ucmd->rightmove, ucmd->upmove);
    //Com_Printf("ucmd->serverTime = %d\n", ucmd->serverTime);
}

/*
==============
idBotLocal::BotUpdateInput
==============
*/
void idBotLocal::BotUpdateInput( bot_state_t* bs, S32 time, S32 elapsed_time )
{
    bot_input_t bi;
    S32 j;
    
    //add the delta angles to the bot's current view angles
    for( j = 0; j < 3; j++ )
    {
        bs->viewangles[j] = AngleMod( bs->viewangles[j] + SHORT2ANGLE( bs->cur_ps.delta_angles[j] ) );
    }
    
    //change the bot view angles
    BotChangeViewAngles( bs, ( F32 ) elapsed_time / 1000 );
    
    //retrieve the bot input
    trap_EA_GetInput( bs->client, ( F32 ) time / 1000, &bi );
    
    //convert the bot input to a usercmd
    BotInputToUserCommand( &bi, &bs->lastucmd, bs->cur_ps.delta_angles, time );
    
    //subtract the delta angles
    for( j = 0; j < 3; j++ )
    {
        bs->viewangles[j] = AngleMod( bs->viewangles[j] - SHORT2ANGLE( bs->cur_ps.delta_angles[j] ) );
    }
}

/*
==================
BotAI_GetEntityState
==================
 */
S32 idBotLocal::BotAI_GetEntityState( S32 entityNum, entityState_t* state )
{
    gentity_t*	ent;
    
    ent = &g_entities[entityNum];
    ::memset( state, 0, sizeof( entityState_t ) );
    
    if( !ent->inuse )
    {
        return false;
    }
    
    if( !ent->r.linked )
    {
        return false;
    }
    
    if( ent->r.svFlags & SVF_NOCLIENT )
    {
        return false;
    }
    
    ::memcpy( state, &ent->s, sizeof( entityState_t ) );
    
    return true;
}

/*
==================
BotAI_GetSnapshotEntity
==================
 */
S32 idBotLocal::BotAI_GetSnapshotEntity( S32 clientNum, S32 sequence, entityState_t* state )
{
    S32		entNum;
    
    entNum = trap_BotGetSnapshotEntity( clientNum, sequence );
    if( entNum == -1 )
    {
        ::memset( state, 0, sizeof( entityState_t ) );
        return -1;
    }
    
    BotAI_GetEntityState( entNum, state );
    
    return sequence + 1;
}

/*
==================
BotCheckEvents
==================
 */
void idBotLocal::BotCheckEvents( bot_state_t* bs, entityState_t* state )
{
    S32 event;
    //UTF8 buf[128];
    vec3_t lastteleport_origin;		//last teleport event origin
    F32 lastteleport_time;		//last teleport event time
    F32 dist;
    vec3_t dir;
    
    //NOTE: this sucks, we're accessing the gentity_t directly
    //but there's no other fast way to do it right now
    if( bs->entityeventTime[state->number] == g_entities[state->number].eventTime )
    {
        return;
    }
    bs->entityeventTime[state->number] = g_entities[state->number].eventTime;
    
    //if it's an event only entity
    if( state->eType > ET_EVENTS )
    {
        event = ( state->eType - ET_EVENTS ) & ~EV_EVENT_BITS;
    }
    else
    {
        event = state->event & ~EV_EVENT_BITS;
    }
    
    switch( event )
    {
        //client obituary event
        case EV_OBITUARY:
        {
            S32 target, attacker, mod;
            
            target = state->otherEntityNum;
            attacker = state->otherEntityNum2;
            mod = state->eventParm;
            
            if( target == bs->client )
            {
                bs->botdeathtype = mod;
                bs->lastkilledby = attacker;
                BotChat_Death( bs ); // talk
                HBotEnterChat( bs );
                //trap_BotEnterChat(bs->cs, 0, bs->chatto);
                
                if( target == attacker || target == ENTITYNUM_NONE || target == ENTITYNUM_WORLD )
                {
                    bs->botsuicide = true;
                }
                else
                {
                    bs->botsuicide = false;
                }
                
                bs->num_deaths++;
            }
            //else if this client was killed by the bot
            else if( attacker == bs->client )
            {
                bs->enemydeathtype = mod;
                bs->lastkilledplayer = target;
                bs->killedenemy_time = FloatTime();
                
                BotChat_Kill( bs );
                HBotEnterChat( bs );
                //trap_BotEnterChat(bs->cs, 0, bs->chatto);
                
                bs->num_kills++;
                
            }
            else if( attacker == bs->enemy && target == attacker )
            {
                bs->enemysuicide = true;
            }
            break;
        }
        case EV_GLOBAL_SOUND:
        {
            break;
        }
        case EV_PLAYER_TELEPORT_IN:
        {
            VectorCopy( state->origin, lastteleport_origin );
            lastteleport_time = FloatTime();
            break;
        }
        case EV_GENERAL_SOUND:
        {
            break;
        }
        //case EV_FOOTSTEP:
        //case EV_FOOTSTEP_METAL:
        //case EV_FOOTSPLASH:
        //case EV_FOOTWADE:
        case EV_SWIM:
        //case EV_FALL_SHORT:
        //case EV_FALL_MEDIUM:
        //case EV_FALL_FAR:
        //case EV_STEP_4:
        //case EV_STEP_8:
        //case EV_STEP_12:
        //case EV_STEP_16:
        //case EV_JUMP_PAD:
        case EV_JUMP:
        case EV_TAUNT:
        //case EV_WATER_TOUCH:
        //case EV_WATER_LEAVE:
        //case EV_WATER_UNDER:
        //case EV_WATER_CLEAR:
        //case EV_ITEM_PICKUP:
        //case EV_GLOBAL_ITEM_PICKUP:
        case EV_NOAMMO:
        case EV_CHANGE_WEAPON:
        case EV_FIRE_WEAPON:
        case EV_FIRE_WEAPON2:
        case EV_FIRE_WEAPON3:
            VectorSubtract( state->origin, bs->origin, dir );
            dist = VectorLength( dir );
            if( dist < 300.0 )
            {
                VectorCopy( state->origin, bs->lastheardorigin );
                bs->lastheardtime = FloatTime();
                //Bot_Print(BPMSG, "Event happend %0.0f units away at time \n", dist, FloatTime());
                //vectoangles(dir, bs->ideal_viewangles);
            }
            break;
    }
}

/*
==============
idBotLocal::BotCheckSnapshot
==============
*/
void idBotLocal::BotCheckSnapshot( bot_state_t* bs )
{
    S32 ent;
    entityState_t state;
    
    //remove all avoid spots
    //trap_BotAddAvoidSpot(bs->ms, vec3_origin, 0, AVOID_CLEAR);
    //reset kamikaze body
    //bs->kamikazebody = 0;
    //reset number of proxmines
    //bs->numproxmines = 0;
    //
    ent = 0;
    while( ( ent = BotAI_GetSnapshotEntity( bs->client, ent, &state ) ) != -1 )
    {
        //check the entity state for events
        BotCheckEvents( bs, &state );
        //check for grenades the bot should avoid
        //BotCheckForGrenades(bs, &state);
    }
    
    //check the player state for events
    BotAI_GetEntityState( bs->client, &state );
    
    //copy the player state events to the entity state
    state.event = bs->cur_ps.externalEvent;
    state.eventParm = bs->cur_ps.externalEventParm;
    //
    BotCheckEvents( bs, &state );
}

/*
==============
idBotLocal::BotGetMenuText
==============
*/
UTF8* idBotLocal::BotGetMenuText( S32 menu, S32 arg )
{
    switch( menu )
    {
        case MN_TEAM:
            return"menu tremulous_teamselect";
        case MN_A_CLASS:
            return "menu tremulous_alienclass";
        case MN_H_SPAWN:
            return "menu tremulous_humanitem";
        case MN_A_BUILD:
            return "menu tremulous_alienbuild";
        case MN_H_BUILD:
            return "menu tremulous_humanbuild";
        case MN_H_ARMOURY:
            return "menu tremulous_humanarmoury";
        case MN_H_UNKNOWNITEM:
            return "Unknown item";
        case MN_A_TEAMFULL:
            return "The alien team has too many players";
        case MN_H_TEAMFULL:
            return "The human team has too many players";
        case MN_A_TEAMCHANGEBUILDTIMER:
            return "You cannot change teams until your build timer expires.";
        case MN_H_TEAMCHANGEBUILDTIMER:
            return "You cannot change teams until your build timer expires.";
        case MN_CMD_CHEAT:
            return "Cheats are not enabled on this server";
        case MN_CMD_TEAM:
            return "Join a team first";
        case MN_CMD_SPEC:
            return "You can only use this command when spectating";
        case MN_CMD_ALIEN:
            return "Must be alien to use this command";
        case MN_CMD_HUMAN:
            return "Must be human to use this command";
        case MN_CMD_LIVING:
            return "Must be living to use this command";
        case MN_B_NOROOM:
            return "There is no room to build here";
        case MN_B_NORMAL:
            return "Cannot build on this surface";
        case MN_B_CANNOT:
            return "You cannot build that structure";
        case MN_B_LASTSPAWN:
            return "You may not deconstruct the last spawn";
        case MN_B_SUDDENDEATH:
            return "Cannot build during Sudden Death";
        case MN_B_REVOKED:
            return "Your building rights have been revoked";
        case MN_B_SURRENDER:
            return "Building is denied to traitorous cowards";
        //      longMsg   = "Your team has decided to admit defeat and concede the game:"
        //                  "traitors and cowards are not allowed to build.";
        //                  // too harsh? <- LOL
        case MN_H_NOBP:
            return "There is no power remaining";
        case MN_H_NOTPOWERED:
            return "This buildable is not powered";
        case MN_H_ONEREACTOR:
            return "There can only be one Reactor";
        case MN_H_NOPOWERHERE:
            return "There is no power here";
        case MN_H_NODCC:
            return "There is no Defense Computer";
        case MN_H_RPTPOWERHERE:
            return "This area already has power";
        case MN_H_NOSLOTS:
            return "You have no room to carry this";
        case MN_H_NOFUNDS:
            return "Insufficient funds";
        case MN_H_ITEMHELD:
            return "You already hold this item";
        case MN_H_NOARMOURYHERE:
            return "You must be near a powered Armoury";
        case MN_H_NOENERGYAMMOHERE:
            return "You must be near a Reactor or a powered Armoury or Repeater";
        case MN_H_NOROOMBSUITON:
            return "Not enough room here to put on a Battle Suit";
        case MN_H_NOROOMBSUITOFF:
            return "Not enough room here to take off your Battle Suit";
        case MN_H_ARMOURYBUILDTIMER:
            return "You can not buy or sell weapons until your build timer expires";
        case MN_H_DEADTOCLASS:
            return "You must be dead to use the class command";
        case MN_H_UNKNOWNSPAWNITEM:
            return "Unknown starting item";
        case MN_A_NOCREEP:
            return "There is no creep here";
        case MN_A_NOOVMND:
            return "There is no Overmind";
        case MN_A_ONEOVERMIND:
            return "There can only be one Overmind";
        case MN_A_ONEHOVEL:
            return "There can only be one Hovel";
        case MN_A_NOBP:
            return "The Overmind cannot control any more structures";
        case MN_A_NOEROOM:
            return "There is no room to evolve here";
        case MN_A_TOOCLOSE:
            return "This location is too close to the enemy to evolve";
        case MN_A_NOOVMND_EVOLVE:
            return "There is no Overmind";
        case MN_A_EVOLVEBUILDTIMER:
            return "You cannot Evolve until your build timer expires";
        case MN_A_HOVEL_OCCUPIED:
            return "This Hovel is already occupied by another builder";
        case MN_A_HOVEL_BLOCKED:
            return "The exit to this Hovel is currently blocked";
        case MN_A_HOVEL_EXIT:
            return "The exit to this Hovel would always be blocked";
        case MN_A_INFEST:
            return "menu tremulous_alienupgrade\n";
        case MN_A_CANTEVOLVE:
            return va( "You cannot evolve into a %s", bggame->ClassConfig( ( class_t )arg )->humanName );
        case MN_A_EVOLVEWALLWALK:
            return "You cannot evolve while wallwalking";
        case MN_A_UNKNOWNCLASS:
            return "Unknown class";
        case MN_A_CLASSNOTSPAWN:
            return va( "You cannot spawn as a %s", bggame->ClassConfig( ( class_t )arg )->humanName );
        case MN_A_CLASSNOTALLOWED:
            return va( "The %s is not allowed", bggame->ClassConfig( ( class_t )arg )->humanName );
        case MN_A_CLASSNOTATSTAGE:
            return va( "The %s is not allowed at Stage %d", bggame->ClassConfig( ( class_t )arg )->humanName, g_alienStage.integer + 1 );
        default:
            return va( "BotGetMenuText: debug: no such menu %d\n", menu );
    }
}

/*
==============
idBotLocal::BotAI
==============
*/
S32 idBotLocal::BotAI( S32 client, F32 thinktime )
{
    bot_state_t* bs;
    UTF8 buf[1024], *args;
    S32 j;
    
    trap_EA_ResetInput( client );
    //
    bs = botstates[client];
    
    if( !bs || !bs->inuse )
    {
        Bot_Print( BPERROR, "BotAI: client %d is not setup\n", client );
        return false;
    }
    
    BotCheckSnapshot( bs );
    //retrieve the current client state
    
    BotAI_GetClientState( client, &bs->cur_ps );
    //retrieve any waiting server commands
    
    while( trap_BotGetServerCommand( client, buf, sizeof( buf ) ) )
    {
        //have buf point to the command and args to the command arguments
        args = strchr( buf, ' ' );
        if( !args )
        {
            continue;
        }
        *args++ = '\0';
        
        //remove color espace sequences from the arguments
        //RemoveColorEscapeSequences( args );
        
        if( !Q_stricmp( buf, "print" ) )
        {
            //remove first and last quote from the chat message
            if( bot_developer.integer )
            {
                ::memmove( args, args + 1, strlen( args ) );
                args[strlen( args ) - 1] = '\0';
                Bot_Print( BPDEBUG, "[BOT_MSG] %s\n", args );
            }
            trap_BotQueueConsoleMessage( bs->cs, CMS_NORMAL, args );
        }
        else if( !Q_stricmp( buf, "chat" ) )
        {
            //remove first and last quote from the chat message
            ::memmove( args, args + 1, strlen( args ) );
            args[strlen( args ) - 1] = '\0';
            trap_BotQueueConsoleMessage( bs->cs, CMS_CHAT, args );
        }
        else if( !Q_stricmp( buf, "tchat" ) )
        {
            //remove first and last quote from the chat message
            ::memmove( args, args + 1, strlen( args ) );
            args[strlen( args ) - 1] = '\0';
            trap_BotQueueConsoleMessage( bs->cs, CMS_CHAT, args );
        }
        else if( !Q_stricmp( buf, "servermenu" ) )
        {
            UTF8 arg1[ 4 ], arg2[ 4 ];
            UTF8* menu_text;
            if( trap_Argc( ) == 2 )
            {
                trap_Argv( 1, arg1, sizeof( arg1 ) );
                menu_text = BotGetMenuText( atoi( arg1 ), 0 );
            }
            if( trap_Argc( ) == 3 )
            {
                trap_Argv( 1, arg1, sizeof( arg1 ) );
                trap_Argv( 2, arg2, sizeof( arg2 ) );
                menu_text = BotGetMenuText( atoi( arg1 ), atoi( arg2 ) );
            }
            //Bot_Print( BPDEBUG, "[BOT_MENU] %s\n", menu_text );
        }
        else if( !Q_stricmp( buf, "scores" ) )	// parse scores?
        {
        
        }
        else if( !Q_stricmp( buf, "clientLevelShot" ) )
        {
        
        }
        else if( !Q_stricmp( buf, "cp " ) )	// CenterPrint
        {
        
        }
        else if( !Q_stricmp( buf, "cs" ) )	// confing string modified
        {
        
        }
        
    }
    
    //add the delta angles to the bot's current view angles
    for( j = 0; j < 3; j++ )
    {
        bs->viewangles[j] = AngleMod( bs->viewangles[j] + SHORT2ANGLE( bs->cur_ps.delta_angles[j] ) );
    }
    
    //increase the local time of the bot
    bs->ltime += thinktime;
    bs->thinktime = thinktime;
    
    //origin of the bot
    VectorCopy( bs->cur_ps.origin, bs->origin );
    
    //eye coordinates of the bot
    VectorCopy( bs->cur_ps.origin, bs->eye );
    bs->eye[2] += bs->cur_ps.viewheight;
    
    //get the area the bot is in
    bs->areanum = BotPointAreaNum( bs->origin );
    
    // invalidate the hud config string
    bs->hudinfo[0] = '\0';
    
    // update the team info
    if( bs->team == 0 )
    {
        bs->team = g_entities[bs->entitynum].client->pers.teamSelection;
    }
    
    //the real AI
    if( bs->team == TEAM_ALIENS )
    {
        BotAlienAI( bs, thinktime );
    }
    else if( bs->team == TEAM_HUMANS )
    {
        BotHumanAI( bs, thinktime );
    }
    else
    {
        Bot_Print( BPERROR, "BotAI: odd bs->team value %d\n", bs->team );
    }
    
    //set the weapon selection every AI frame
    //trap_EA_SelectWeapon(bs->client, bs->weaponnum);
    
    //subtract the delta angles
    for( j = 0; j < 3; j++ )
    {
        bs->viewangles[j] = AngleMod( bs->viewangles[j] - SHORT2ANGLE( bs->cur_ps.delta_angles[j] ) );
    }
    
    bs->lastframe_health = bs->inventory[BI_HEALTH];
    
    //everything was ok
    return true;
}

/*
==============
idBotLocal::BotCheckDeath
==============
*/
void idBotLocal::BotCheckDeath( S32 target, S32 attacker, S32 mod )
{
    bot_state_t* tbs, *abs;
    
    tbs = botstates[target];
    abs = botstates[attacker];
    if( ( !tbs || !tbs->inuse ) && ( !abs || !abs->inuse ) )
    {
        // Both aren't bots so return
        return;
    }
    
    if( tbs && tbs->client && target == tbs->client )
    {
        tbs->botdeathtype = mod;
        tbs->lastkilledby = attacker;
        if( tbs->team == TEAM_HUMANS )
        {
            if( BotChat_Death( tbs ) )
            {
                // talk
                HBotEnterChat( tbs );
            }
            //trap_BotEnterChat(tbs->cs, 0, tbs->chatto);
        }
        
        if( target == attacker || target == ENTITYNUM_NONE || target == ENTITYNUM_WORLD )
        {
            tbs->botsuicide = true;
        }
        else
        {
            tbs->botsuicide = false;
        }
        tbs->num_deaths++;
    }
    
    //else if this client was killed by the bot
    if( abs && abs->client && attacker == abs->client )
    {
        abs->enemydeathtype = mod;
        abs->lastkilledplayer = target;
        abs->killedenemy_time = FloatTime();
        if( abs->team == TEAM_HUMANS )
        {
            if( BotChat_Kill( abs ) )
            {
                HBotEnterChat( abs );
            }
            //trap_BotEnterChat(abs->cs, 0, abs->chatto);
        }
        
        abs->num_kills++;
    }
}

/*
==============
idBotLocal::BotScheduleBotThink
==============
*/
void idBotLocal::BotScheduleBotThink( void )
{
    S32 i, botnum;
    
    botnum = 0;
    
    for( i = 0; i < MAX_CLIENTS; i++ )
    {
        if( !botstates[i] || !botstates[i]->inuse )
        {
            continue;
        }
        
        //initialize the bot think residual time
        botstates[i]->botthink_residual = bot_thinktime.integer * botnum / numbots;
        botnum++;
    }
}

/*
==============
idGameLocal::BotAIStartFrame
==============
*/
S32 idGameLocal::BotAIStartFrame( S32 time )
{
    S32 elapsed_time;
    static S32 local_time = 0;
    static S32 lastbotthink_time;
    S32 thinktime;
    static S32 botlib_residual;
    S32 i;
    bot_entitystate_t state;
    gentity_t*	ent;
    
    gameLocal.CheckBotSpawn();
    
    trap_Cvar_Update( &bot_fastchat );
    trap_Cvar_Update( &bot_nochat );
    trap_Cvar_Update( &bot_thinktime );
    trap_Cvar_Update( &bot_memorydump );
    trap_Cvar_Update( &bot_saveroutingcache );
    trap_Cvar_Update( &bot_pause );
    trap_Cvar_Update( &bot_report );
    trap_Cvar_Update( &bot_developer );
    
    if( bot_memorydump.integer )
    {
        trap_BotLibVarSet( "memorydump", "1" );
        trap_Cvar_Set( "bot_memorydump", "0" );
    }
    if( bot_saveroutingcache.integer )
    {
        trap_BotLibVarSet( "saveroutingcache", "1" );
        trap_Cvar_Set( "bot_saveroutingcache", "0" );
    }
    
    //cap the bot think time
    if( bot_thinktime.integer > 200 )
    {
        trap_Cvar_Set( "bot_thinktime", "200" );
    }
    
    // reschedule if think time was changed
    trap_Cvar_Update( &bot_thinktime );
    if( bot_thinktime.integer > 200 )
    {
        trap_Cvar_Set( "bot_thinktime", "200" );
    }
    
    if( bot_thinktime.integer != lastbotthink_time )
    {
        lastbotthink_time = bot_thinktime.integer;
        botLocal.BotScheduleBotThink();
    }
    
    elapsed_time = time - local_time;
    local_time = time;
    
    botlib_residual += elapsed_time;
    
    if( elapsed_time > bot_thinktime.integer )
    {
        thinktime = elapsed_time;
    }
    else
    {
        thinktime = bot_thinktime.integer;
    }
    
    // update the bot library
    if( botlib_residual >= thinktime )
    {
        botlib_residual -= thinktime;
        
        trap_BotLibStartFrame( ( F32 ) time / 1000 );
        
        if( !trap_AAS_Initialized() )
        {
            return false;
        }
        
        //update entities in the botlib
        for( i = 0; i < MAX_GENTITIES; i++ )
        {
            ent = &g_entities[i];
            
            if( !ent->inuse )
            {
                trap_BotLibUpdateEntity( i, NULL );
                continue;
            }
            
            if( !ent->r.linked )
            {
                trap_BotLibUpdateEntity( i, NULL );
                continue;
            }
            
            if( ent->r.svFlags & SVF_NOCLIENT )
            {
                trap_BotLibUpdateEntity( i, NULL );
                continue;
            }
            
            // do not update missiles
            if( ent->s.eType == ET_MISSILE )
            {
                trap_BotLibUpdateEntity( i, NULL );
                continue;
            }
            
            // do not update event only entities
            if( ent->s.eType > ET_EVENTS )
            {
                trap_BotLibUpdateEntity( i, NULL );
                continue;
            }
            
            ::memset( &state, 0, sizeof( bot_entitystate_t ) );
            
            VectorCopy( ent->r.currentOrigin, state.origin );
            if( i < MAX_CLIENTS )
            {
                VectorCopy( ent->s.apos.trBase, state.angles );
            }
            else
            {
                VectorCopy( ent->r.currentAngles, state.angles );
            }
            
            VectorCopy( ent->s.origin2, state.old_origin );
            VectorCopy( ent->r.mins, state.mins );
            VectorCopy( ent->r.maxs, state.maxs );
            
            state.type = ent->s.eType;
            state.flags = ent->s.eFlags;
            
            if( ent->r.bmodel )
            {
                state.solid = SOLID_BSP;
            }
            else
            {
                state.solid = SOLID_BBOX;
            }
            
            state.groundent = ent->s.groundEntityNum;
            state.modelindex = ent->s.modelindex;
            state.modelindex2 = ent->s.modelindex2;
            state.frame = ent->s.frame;
            state.event = ent->s.event;
            state.eventParm = ent->s.eventParm;
            state.legsAnim = ent->s.legsAnim;
            state.torsoAnim = ent->s.torsoAnim;
            state.weapon = ent->s.weapon;
            
            trap_BotLibUpdateEntity( i, &state );
        }
        
        //BotAIRegularUpdate();		// update the ai itemlist, see BotUpdateEntityItems
    }
    
    floattime = trap_AAS_Time();
    
    // execute scheduled bot AI
    for( i = 0; i < MAX_CLIENTS; i++ )
    {
        if( !botstates[i] || !botstates[i]->inuse )
        {
            continue;
        }
        
        botstates[i]->botthink_residual += elapsed_time;
        
        if( botstates[i]->botthink_residual >= thinktime )
        {
            botstates[i]->botthink_residual -= thinktime;
            
            if( !trap_AAS_Initialized() )
            {
                return false;
            }
            
            if( g_entities[i].client->pers.connected == CON_CONNECTED )
            {
                //botstates[i]->frametime = time;		// cyr, clock frame duration
                botLocal.BotAI( i, ( F32 ) thinktime / 1000 );
            }
        }
    }
    
    //gameLocal.DeleteDebugLines();
    
    // execute bot user commands every frame
    for( i = 0; i < MAX_CLIENTS; i++ )
    {
        if( !botstates[i] || !botstates[i]->inuse )
        {
            continue;
        }
        if( g_entities[i].client->pers.connected != CON_CONNECTED )
        {
            continue;
        }
        
        botLocal.BotShowViewAngles( botstates[i] );
        botLocal.BotUpdateInput( botstates[i], time, elapsed_time );
        trap_BotUserCommand( botstates[i]->client, &botstates[i]->lastucmd );
    }
    
    return true;
}

/*
==============
idBotLocal::BotAISetupClient
==============
*/
S32 idBotLocal::BotAISetupClient( S32 client, struct bot_settings_s* settings, bool restart )
{
    UTF8 filename[MAX_PATH], name[MAX_PATH],  csname[MAX_PATH];
    bot_state_t* bs;
    S32 team, errnum;
    
    if( !Q_stricmp( settings->team, "aliens" ) )
    {
        team = TEAM_ALIENS;
    }
    else if( !Q_stricmp( settings->team, "humans" ) )
    {
        team = TEAM_HUMANS;
    }
    else
    {
        team = TEAM_NONE;
    }
    
    if( !botstates[client] )
    {
        botstates[client] = ( bot_state_t* )bggame->Alloc( sizeof( bot_state_t ) );
    }
    bs = botstates[client];
    
    if( bs && bs->inuse )
    {
        Bot_Print( BPERROR, "idBotLocal::BotAISetupClient: client %d already setup\n", client );
        return false;
    }
    
    if( !trap_AAS_Initialized() )
    {
        Bot_Print( BPERROR, "AAS not initialized\n" );
        return false;
    }
    
    //allocate a weapon state
    bs->ws = trap_BotAllocWeaponState();
    //Bot_Print(BPMSG, "loading skill %f from %s\n", settings->skill, settings->characterfile);
    
    //load the bot character
    //if (team == TEAM_HUMANS){
    bs->character = trap_BotLoadCharacter( settings->characterfile, settings->skill );
    //Bot_Print(BPMSG, "loaded skill %f from %s\n", settings->skill, settings->characterfile);
    
    if( !bs->character && team == TEAM_HUMANS )
    {
        Bot_Print( BPERROR, "couldn't load skill %f from %s\n", settings->skill, settings->characterfile );
        return false;
    }
    
    //allocate a chat state
    bs->cs = trap_BotAllocChatState();
    
    //load the chat file
    trap_Characteristic_String( bs->character, CHARACTERISTIC_CHAT_FILE, filename, MAX_PATH );
    trap_Characteristic_String( bs->character, CHARACTERISTIC_CHAT_NAME, name, MAX_PATH );
    errnum = trap_BotLoadChatFile( bs->cs, filename, name );
    
    if( errnum != BLERR_NOERROR && team == TEAM_HUMANS )
    {
        trap_BotFreeChatState( bs->cs );
        //trap_BotFreeGoalState(bs->gs);
        trap_BotFreeWeaponState( bs->ws );
        return false;
    }
    
    //copy the settings
    ::memcpy( &bs->settings, settings, sizeof( bot_settings_t ) );
    
    //set the chat name
    ClientName( client, csname, sizeof( csname ) );
    trap_BotSetChatName( bs->cs, csname, client );
    
    bs->inuse = true;
    bs->client = client;
    bs->entitynum = client;
    bs->setupcount = 4;
    bs->entergame_time = FloatTime();
    bs->ms = trap_BotAllocMoveState();
    bs->team = team;
    bs->ent = &g_entities[ bs->client ];
    //bs->ent->bs = bs;
    bs->tfl = TFL_DEFAULT;
    numbots++;
    
    //NOTE: reschedule the bot thinking
    BotScheduleBotThink();
    if( BotChat_EnterGame( bs ) )
    {
        // talk
        HBotEnterChat( bs );
    }
    
    //bot has been setup succesfully
    return true;
}

/*
==============
idBotLocal::BotAIShutdownClient
==============
*/
S32 idBotLocal::BotAIShutdownClient( S32 client, bool restart )
{
    bot_state_t* bs;
    
    bs = botstates[client];
    if( !bs || !bs->inuse )
    {
        Bot_Print( BPERROR, "idBotLocal::BotAIShutdownClient: client %d already shutdown\n", client );
        return false;
    }
    
    trap_BotFreeMoveState( bs->ms );
    
    //free the goal state`
    //trap_BotFreeGoalState(bs->gs);
    
    //free the chat file
    trap_BotFreeChatState( bs->cs );
    
    //free the weapon weights
    trap_BotFreeWeaponState( bs->ws );
    
    //free the bot character
    trap_BotFreeCharacter( bs->character );
    
    BotFreeWaypoints( bs->checkpoints );
    BotFreeWaypoints( bs->patrolpoints );
    
    //clear activate goal stack
    //BotClearActivateGoalStack(bs);
    
    //clear the bot state
    ::memset( bs, 0, sizeof( bot_state_t ) );
    
    //set the inuse flag to false
    bs->inuse = false;
    
    //there's one bot less
    numbots--;
    
    //everything went ok
    return true;
}

/*
==============
idBotLocal::BotResetState

called when a bot enters the intermission or observer mode and
when the level is changed
==============
*/
void idBotLocal::BotResetState( bot_state_t* bs )
{
    S32 client, entitynum, inuse;
    S32 movestate, goalstate, chatstate, weaponstate;
    bot_settings_t settings;
    S32 character;
    playerState_t ps;							//current player state
    F32 entergame_time;
    
    //save some things that should not be reset here
    memcpy( &settings, &bs->settings, sizeof( bot_settings_t ) );
    memcpy( &ps, &bs->cur_ps, sizeof( playerState_t ) );
    inuse = bs->inuse;
    client = bs->client;
    entitynum = bs->entitynum;
    character = bs->character;
    movestate = bs->ms;
    goalstate = bs->gs;
    chatstate = bs->cs;
    weaponstate = bs->ws;
    entergame_time = bs->entergame_time;
    
    //free checkpoints and patrol points
    BotFreeWaypoints( bs->checkpoints );
    BotFreeWaypoints( bs->patrolpoints );
    
    //reset the whole state
    ::memset( bs, 0, sizeof( bot_state_t ) );
    
    //copy back some state stuff that should not be reset
    bs->ms = movestate;
    bs->gs = goalstate;
    bs->cs = chatstate;
    bs->ws = weaponstate;
    ::memcpy( &bs->cur_ps, &ps, sizeof( playerState_t ) );
    ::memcpy( &bs->settings, &settings, sizeof( bot_settings_t ) );
    bs->inuse = inuse;
    bs->client = client;
    bs->entitynum = entitynum;
    bs->character = character;
    bs->entergame_time = entergame_time;
    
    //reset several states
    if( bs->ms )
    {
        trap_BotResetMoveState( bs->ms );
    }
    if( bs->gs )
    {
        trap_BotResetGoalState( bs->gs );
    }
    if( bs->ws )
    {
        trap_BotResetWeaponState( bs->ws );
    }
    if( bs->gs )
    {
        trap_BotResetAvoidGoals( bs->gs );
    }
    if( bs->ms )
    {
        trap_BotResetAvoidReach( bs->ms );
    }
}

/*
==============
idBotLocal::BotAILoadMap
==============
*/
S32 idBotLocal::BotAILoadMap( S32 restart )
{
    S32			i;
    vmCvar_t	mapname;
    
    if( !restart )
    {
        trap_Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );
        trap_BotLibLoadMap( mapname.string );
    }
    
    for( i = 0; i < MAX_CLIENTS; i++ )
    {
        if( botstates[i] && botstates[i]->inuse )
        {
            BotResetState( botstates[i] );
            botstates[i]->setupcount = 4;
        }
    }
    
    return true;
}

/*
==============
idBotLocal::BotInitLibrary
==============
*/
S32 idBotLocal::BotInitLibrary( void )
{
    UTF8 buf[144];
    
    //set the maxclients and maxentities library variables before calling BotSetupLibrary
    trap_Cvar_VariableStringBuffer( "sv_maxclients", buf, sizeof( buf ) );
    
    if( !strlen( buf ) )
    {
        strcpy( buf, "8" );
    }
    
    trap_BotLibVarSet( "maxclients", buf );
    Com_sprintf( buf, sizeof( buf ), "%d", MAX_GENTITIES );
    trap_BotLibVarSet( "maxentities", buf );
    
    //bsp checksum
    trap_Cvar_VariableStringBuffer( "sv_mapChecksum", buf, sizeof( buf ) );
    if( strlen( buf ) )
    {
        trap_BotLibVarSet( "sv_mapChecksum", buf );
    }
    
    //maximum number of aas links
    trap_Cvar_VariableStringBuffer( "max_aaslinks", buf, sizeof( buf ) );
    if( strlen( buf ) )
    {
        trap_BotLibVarSet( "max_aaslinks", buf );
    }
    
    //maximum number of items in a level
    trap_Cvar_VariableStringBuffer( "max_levelitems", buf, sizeof( buf ) );
    if( strlen( buf ) )
    {
        trap_BotLibVarSet( "max_levelitems", buf );
    }
    
    //game type
    trap_Cvar_VariableStringBuffer( "g_gametype", buf, sizeof( buf ) );
    if( !strlen( buf ) )
    {
        strcpy( buf, "0" );
    }
    trap_BotLibVarSet( "g_gametype", buf );
    
    //bot developer mode and log file
    trap_BotLibVarSet( "bot_developer", bot_developer.string );
    trap_Cvar_VariableStringBuffer( "logfile", buf, sizeof( buf ) );
    trap_BotLibVarSet( "log", buf );
    
    //no chatting
    trap_Cvar_VariableStringBuffer( "bot_nochat", buf, sizeof( buf ) );
    if( strlen( buf ) )
    {
        trap_BotLibVarSet( "nochat", buf );
    }
    
    //visualize jump pads
    trap_Cvar_VariableStringBuffer( "bot_visualizejumppads", buf, sizeof( buf ) );
    if( strlen( buf ) )
    {
        trap_BotLibVarSet( "bot_visualizejumppads", buf );
    }
    
    //forced clustering calculations
    trap_Cvar_VariableStringBuffer( "bot_forceclustering", buf, sizeof( buf ) );
    if( strlen( buf ) )
    {
        trap_BotLibVarSet( "forceclustering", buf );
    }
    
    //forced reachability calculations
    trap_Cvar_VariableStringBuffer( "bot_forcereachability", buf, sizeof( buf ) );
    if( strlen( buf ) )
    {
        trap_BotLibVarSet( "forcereachability", buf );
    }
    
    //force writing of AAS to file
    trap_Cvar_VariableStringBuffer( "bot_forcewrite", buf, sizeof( buf ) );
    if( strlen( buf ) )
    {
        trap_BotLibVarSet( "forcewrite", buf );
    }
    
    //no AAS optimization
    trap_Cvar_VariableStringBuffer( "bot_aasoptimize", buf, sizeof( buf ) );
    if( strlen( buf ) )
    {
        trap_BotLibVarSet( "aasoptimize", buf );
    }
    
    trap_Cvar_VariableStringBuffer( "bot_saveroutingcache", buf, sizeof( buf ) );
    if( strlen( buf ) )
    {
        trap_BotLibVarSet( "saveroutingcache", buf );
    }
    
    //reload instead of cache bot character files
    trap_Cvar_VariableStringBuffer( "bot_reloadcharacters", buf, sizeof( buf ) );
    
    if( !strlen( buf ) )
    {
        strcpy( buf, "0" );
    }
    
    trap_BotLibVarSet( "bot_reloadcharacters", buf );
    
    //base directory
    trap_Cvar_VariableStringBuffer( "fs_basepath", buf, sizeof( buf ) );
    if( strlen( buf ) )
    {
        trap_BotLibVarSet( "basedir", buf );
    }
    
    //game directory
    trap_Cvar_VariableStringBuffer( "fs_game", buf, sizeof( buf ) );
    if( strlen( buf ) )
    {
        trap_BotLibVarSet( "gamedir", buf );
    }
    
    //cd directory
    trap_Cvar_VariableStringBuffer( "fs_cdpath", buf, sizeof( buf ) );
    if( strlen( buf ) )
    {
        trap_BotLibVarSet( "cddir", buf );
    }
    
    //home directory
    trap_Cvar_VariableStringBuffer( "fs_homepath", buf, sizeof( buf ) );
    if( strlen( buf ) )
    {
        trap_BotLibVarSet( "homedir", buf );
    }
    
    //setup the bot library
    return trap_BotLibSetup();
}

/*
==============
idBotLocal::BotAISetup
==============
*/
S32 idBotLocal::BotAISetup( S32 restart )
{
    S32 errnum;
    
    trap_Cvar_Register( &bot_thinktime, "bot_thinktime", "100", CVAR_CHEAT );
    trap_Cvar_Register( &bot_memorydump, "bot_memorydump", "0", CVAR_CHEAT );
    trap_Cvar_Register( &bot_saveroutingcache, "bot_saveroutingcache", "0", CVAR_CHEAT );
    trap_Cvar_Register( &bot_pause, "bot_pause", "0", CVAR_CHEAT );
    trap_Cvar_Register( &bot_report, "bot_report", "0", CVAR_CHEAT );
    trap_Cvar_Register( &bot_testsolid, "bot_testsolid", "0", CVAR_CHEAT );
    trap_Cvar_Register( &bot_testclusters, "bot_testclusters", "0", CVAR_CHEAT );
    trap_Cvar_Register( &bot_developer, "bot_developer", "0", CVAR_CHEAT );
    trap_Cvar_Register( &bot_interbreedchar, "bot_interbreedchar", "", 0 );
    trap_Cvar_Register( &bot_interbreedbots, "bot_interbreedbots", "10", 0 );
    trap_Cvar_Register( &bot_interbreedcycle, "bot_interbreedcycle", "20", 0 );
    trap_Cvar_Register( &bot_interbreedwrite, "bot_interbreedwrite", "", 0 );
    
    //if the game is restarted for a tournament
    if( restart )
    {
        return true;
    }
    
    //reset the bot states
    ::memset( botstates, 0, sizeof( botstates ) );
    
    errnum = BotInitLibrary();
    if( errnum != BLERR_NOERROR )
    {
        return false;
    }
    return true;
}

/*
==============
idBotLocal::BotAIShutdown
==============
*/
S32 idBotLocal::BotAIShutdown( S32 restart )
{
    S32 i;
    
    //if the game is restarted for a tournament
    if( restart )
    {
        //shutdown all the bots in the botlib
        for( i = 0; i < MAX_CLIENTS; i++ )
        {
            if( botstates[i] && botstates[i]->inuse )
            {
                BotAIShutdownClient( botstates[i]->client, restart );
            }
        }
        //don't shutdown the bot library
    }
    else
    {
        trap_BotLibShutdown();
    }
    return true;
}

/*
==============
idBotLocal::BotEntityInfo
==============
*/
void idBotLocal::BotEntityInfo( S32 entnum, aas_entityinfo_t* info )
{
    trap_AAS_EntityInfo( entnum, info );
}

/*
==================
idBotLocal::BotRandomMove
==================
 */
void idBotLocal::BotRandomMove( bot_state_t* bs, bot_moveresult_t* moveresult )
{
    vec3_t dir, angles;
    
    angles[0] = 0;
    angles[1] = random() * 360;
    angles[2] = 0;
    AngleVectors( angles, dir, NULL, NULL );
    
    trap_BotMoveInDirection( bs->ms, dir, 400, MOVE_WALK );
    
    moveresult->failure = false;
    VectorCopy( dir, moveresult->movedir );
}

/*
==================
idBotLocal::BotAIBlocked

Very basic handling of bots being blocked by other entities.
Check what kind of entity is blocking the bot and try to activate
it. If that's not an option then try to walk around or over the entity.
Before the bot ends in this part of the AI it should predict which doors to
open, which buttons to activate etc.
==================
*/
void idBotLocal::BotAIBlocked( bot_state_t* bs, bot_moveresult_t* moveresult, S32 activate )
{
    S32 movetype;//, bspent;
    vec3_t hordir, start, end, mins, maxs, sideward, angles, up = {0, 0, 1};
    aas_entityinfo_t entinfo;
    gentity_t* blockent = &g_entities[ moveresult->blockentity ];
    
    // if the bot is not blocked by anything
    if( !moveresult->blocked )
    {
        bs->notblocked_time = FloatTime();
        return;
    }
    
    if( blockent->s.eType == ET_BUILDABLE )
    {
        trap_EA_Jump( bs->client );
    }
    
    // if stuck in a solid area
    if( moveresult->type == RESULTTYPE_INSOLIDAREA )
    {
        // move in a random direction in the hope to get out
        BotRandomMove( bs, moveresult );
        return;
    }
    
    // get info for the entity that is blocking the bot
    BotEntityInfo( moveresult->blockentity, &entinfo );
    
#ifdef OBSTACLEDEBUG
    ClientName( bs->client, netname, sizeof( netname ) );
    BotAI_Print( PRT_MESSAGE, "%s: I'm blocked by model %d\n", netname, entinfo.modelindex );
#endif // OBSTACLEDEBUG
    
    // just some basic dynamic obstacle avoidance code
    hordir[0] = moveresult->movedir[0];
    hordir[1] = moveresult->movedir[1];
    hordir[2] = 0;
    
    // if no direction just take a random direction
    if( VectorNormalize( hordir ) < 0.1 )
    {
        VectorSet( angles, 0, 360 * random(), 0 );
        AngleVectors( angles, hordir, NULL, NULL );
    }
    
    //if (moveresult->flags & MOVERESULT_ONTOPOFOBSTACLE) movetype = MOVE_JUMP;
    //else
    movetype = MOVE_WALK;
    // if there's an obstacle at the bot's feet and head then
    // the bot might be able to crouch through
    VectorCopy( bs->origin, start );
    start[2] += 18;
    VectorMA( start, 5, hordir, end );
    VectorSet( mins, -16, -16, -24 );
    VectorSet( maxs, 16, 16, 4 );
    
    //bsptrace = AAS_Trace(start, mins, maxs, end, bs->entitynum, MASK_PLAYERSOLID);
    //if (bsptrace.fraction >= 1) movetype = MOVE_CROUCH;
    // get the sideward vector
    CrossProduct( hordir, up, sideward );
    
    if( bs->flags & BFL_AVOIDRIGHT )
    {
        VectorNegate( sideward, sideward );
    }
    
    // try to crouch straight forward?
    if( movetype != MOVE_CROUCH || !trap_BotMoveInDirection( bs->ms, hordir, 400, movetype ) )
    {
        // perform the movement
        if( !trap_BotMoveInDirection( bs->ms, sideward, 400, movetype ) )
        {
            // flip the avoid direction flag
            bs->flags ^= BFL_AVOIDRIGHT;
            // flip the direction
            // VectorNegate(sideward, sideward);
            VectorMA( sideward, -1, hordir, sideward );
            // move in the other direction
            trap_BotMoveInDirection( bs->ms, sideward, 400, movetype );
        }
    }
}

/*
==============
idBotLocal::NumBots
==============
*/
S32 idBotLocal::NumBots( void )
{
    return numbots;
}

/*
==============
idBotLocal::BotBeginIntermission
==============
*/
void idBotLocal::BotBeginIntermission( void )
{
    S32 i;
    
    for( i = 0; i < MAX_CLIENTS; i++ )
    {
        if( botstates[i] && botstates[i]->inuse )
        {
            if( botstates[i]->team == TEAM_HUMANS || botstates[i]->team == TEAM_ALIENS )
            {
                BotChat_EndLevel( botstates[i] );
                trap_BotEnterChat( botstates[i]->cs, 0, botstates[i]->chatto );
                gameLocal.ClientDisconnect( i );
                //this needs to be done to free up the client slot - I think - Ender Feb 18 2008
                trap_DropClient( i, "disconnected", 0 );
            }
        }
    }
}

/*
==================
idBotLocal::Bot_TargetValue

Bot_TargetValue
Calculate this entities target value
==================
*/
S32 idBotLocal::Bot_TargetValue( bot_state_t* bs, S32 i )
{
    S32 target_value = 10;
    aas_entityinfo_t entinfo;
    gentity_t* ent;
    S32 x;
    vec3_t origin;
    F32 f;
    
    ent = &g_entities[ i ];
    BotEntityInfo( i, &entinfo );
    VectorCopy( ent->s.origin, origin );
    x = GetWalkingDist( bs, origin );
    
    if( x == -1 )
    {
        return -1;
    }
    
    if( BotSameTeam( bs, i ) )
    {
        return -1;
    }
    
    if( ent->s.eType == ET_BUILDABLE )
    {
        switch( ent->s.modelindex )
        {
            case BA_H_SPAWN:
                origin[2] += 5.0;
            case BA_A_SPAWN:
                target_value += 1;
                break;
            case BA_A_TRAPPER:
                target_value += 2;
                break;
            case BA_H_MEDISTAT:
                origin[2] += 5.0;
                break;
            default:
                break;
        }
        f = ( F32 )ent->health / ( F32 )bggame->Buildable( ( buildable_t )ent->s.modelindex )->health;
    }
    else if( ent->s.eType == ET_PLAYER )
    {
        switch( ent->client->ps.stats[ STAT_CLASS ] )
        {
            case WP_ALEVEL0:
            case WP_ALEVEL1:
            case WP_ALEVEL1_UPG:
                origin[2] += 5.0;
                break;
            default:
                break;
        }
        f = ( F32 )ent->health / ( F32 )bggame->Class( ( class_t )ent->client->ps.stats[ STAT_CLASS ] )->health;
    }
    else
    {
        return -1;
    }
    
    /* Check Health */
    if( f >= 1.0 )
    {
        target_value -= 2;
    }
    else if( f >= 0.5 )
    {
        target_value -= 1;
    }
    else if( f >= 0.2 )
    {
        target_value -= 0;
    }
    else if( f > 0.0 )
    {
        target_value += 2;
    }
    else
    {
        return -1;
    }
    /* Check visibility */
    if( BotEntityVisible( bs->entitynum, bs->eye, bs->viewangles, 120.0, i ) )
    {
        vec3_t dir, entangles, middle;
        target_value += 1;
        
        VectorAdd( entinfo.mins, entinfo.maxs, middle );
        VectorScale( middle, 0.5, middle );
        VectorAdd( entinfo.origin, middle, middle );
        
        //check if entity is within inner field of vision
        VectorSubtract( middle, bs->eye, dir );
        vectoangles( dir, entangles );
        if( BotInFieldOfVision( bs->viewangles, 10.0, entangles ) )
        {
            target_value += 2;
        }
    }
    
    /* Check walking distance */
    if( x < 40 )
    {
        target_value += 4;
    }
    else if( x < 80 )
    {
        target_value += 2;
    }
    else if( x < 160 )
    {
        target_value += 1;
    }
    else if( x < 640 )
    {
        target_value -= 1;
    }
    else
    {
        target_value -= 2;
    }
    
    return target_value;
}

/*
==================
idBotLocal::Bot_FindTarget
==================
*/
S32 idBotLocal::Bot_FindTarget( bot_state_t* bs )
{
    S32 i, value, best_value, best_target;
    
    best_target = -1;
    best_value = 0;
    
    for( i = 0; i < level.num_entities; i++ )
    {
        value = Bot_TargetValue( bs, i );
        if( value > best_value )
        {
            best_value = value;
            best_target = i;
        }
    }
    
    return best_target;
}
