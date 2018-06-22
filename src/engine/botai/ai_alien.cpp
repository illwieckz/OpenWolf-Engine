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
// File name:   ai_alien.cpp
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////

#include <sgame/sg_precompiled.h>

#define AS_OVER_RT3 ((ALIENSENSE_RANGE*0.5f)/M_ROOT3)

typedef enum
{
    AS_SPAWN,
    AS_EVO,
    AS_ATTACK
} astates_t;

/*
==================
idBotLocal::ABotFindWidth
==================
*/
F32 idBotLocal::ABotFindWidth( S32 weapon ) //, F32 width, F32 range){
{
    F32 width, range;
    
    switch( weapon )
    {
        case WP_ABUILD:
        case WP_ABUILD2:
            range = ABUILDER_CLAW_RANGE;
            width = ABUILDER_CLAW_WIDTH;
            break;
        case WP_ALEVEL0:
            range = LEVEL0_BITE_RANGE;
            width = LEVEL0_BITE_WIDTH;
            break;
        case WP_ALEVEL1:
        case WP_ALEVEL1_UPG:
            range = LEVEL1_CLAW_RANGE;
            width = LEVEL1_CLAW_WIDTH;
            break;
        case WP_ALEVEL2:
        case WP_ALEVEL2_UPG:
            range = LEVEL2_CLAW_RANGE;
            width = LEVEL2_CLAW_WIDTH;
            break;
        case WP_ALEVEL3:
        case WP_ALEVEL3_UPG:
            range = LEVEL3_CLAW_RANGE;
            width = LEVEL3_CLAW_WIDTH;
            break;
        case WP_ALEVEL4:
            range = LEVEL4_CLAW_RANGE;
            width = LEVEL4_CLAW_WIDTH;
            break;
        default:
            range = 0.0f;
            width = 0.0f;
            break;
    }
    
    //Bot_Print( BPERROR, "weapon #: %d width: %f range: %f \n", weapon, width, range);
    return width;
}

/*
==================
idBotLocal::ABotFindRange
==================
*/
F32 idBotLocal::ABotFindRange( S32 weapon ) //, F32 width, F32 range){
{
    F32 width, range;
    
    switch( weapon )
    {
        case WP_ABUILD:
        case WP_ABUILD2:
            range = ABUILDER_CLAW_RANGE;
            width = ABUILDER_CLAW_WIDTH;
            break;
        case WP_ALEVEL0:
            range = LEVEL0_BITE_RANGE;
            width = LEVEL0_BITE_WIDTH;
            break;
        case WP_ALEVEL1:
        case WP_ALEVEL1_UPG:
            range = LEVEL1_CLAW_RANGE;
            width = LEVEL1_CLAW_WIDTH;
            break;
        case WP_ALEVEL2:
        case WP_ALEVEL2_UPG:
            range = LEVEL2_CLAW_RANGE;
            width = LEVEL2_CLAW_WIDTH;
            break;
        case WP_ALEVEL3:
        case WP_ALEVEL3_UPG:
            range = LEVEL3_CLAW_RANGE;
            width = LEVEL3_CLAW_WIDTH;
            break;
        case WP_ALEVEL4:
            range = LEVEL4_CLAW_RANGE;
            width = LEVEL4_CLAW_WIDTH;
            break;
        default:
            range = 0.0f;
            width = 0.0f;
            break;
    }
    
    //Bot_Print( BPERROR, "weapon #: %d width: %f range: %f \n", weapon, width, range);
    return range;
}

/*
==================
idBotLocal::BotAimAtEnemy
==================
 */
void idBotLocal::ABotAimAtEnemy( bot_state_t* bs )
{
    S32 i;
    vec3_t dir, bestorigin, start;
    vec3_t mins = { -4, -4, -4}, maxs = {4, 4, 4};
    aas_entityinfo_t entinfo;
    bsp_trace_t trace;
    vec3_t target;
    
    //if the bot has no enemy
    if( bs->enemy < 0 )
    {
        return;
    }
    
    //get the enemy entity information
    BotEntityInfo( bs->enemy, &entinfo );
    
    //if this is not a player
    if( bs->enemy >= MAX_CLIENTS )
    {
        //if the buildable is visible
        VectorCopy( entinfo.origin, target );
        
        //aim at the building
        VectorSubtract( target, bs->eye, dir );
        vectoangles( dir, bs->ideal_viewangles );
        
        //set the aim target before trying to attack
        VectorCopy( target, bs->aimtarget );
        return;
    }
    
    // todo: add reaction delay (enemysight time)
    
    //get the enemy entity information
    BotEntityInfo( bs->enemy, &entinfo );
    
    // todo, predict enemy movement (velocity... lastvisorigin)
    
    VectorCopy( entinfo.origin, bestorigin );
    
    //get the start point shooting from
    //NOTE: the x and y projectile start offsets are ignored
    VectorCopy( bs->origin, start );
    start[2] += bs->cur_ps.viewheight;
    
    BotAI_Trace( &trace, start, mins, maxs, bestorigin, bs->entitynum, MASK_SHOT );
    
    //if the enemy is NOT hit
    if( trace.fraction <= 1 && trace.ent != entinfo.number )
    {
        bestorigin[2] += 16;
    }
    
    VectorCopy( bestorigin, bs->aimtarget );
    
    //get aim direction
    VectorSubtract( bestorigin, bs->eye, dir );
    
    //add some randomness
    for( i = 0; i < 3; i++ ) dir[i] += 0.06 * crandom();
    
    //set the ideal view angles
    vectoangles( dir, bs->ideal_viewangles );
    bs->ideal_viewangles[PITCH] = AngleMod( bs->ideal_viewangles[PITCH] );
    bs->ideal_viewangles[YAW] = AngleMod( bs->ideal_viewangles[YAW] );
    
    /*
    //set the view angles directly
    if (bs->ideal_viewangles[PITCH] > 180) bs->ideal_viewangles[PITCH] -= 360;
    VectorCopy(bs->ideal_viewangles, bs->viewangles);
    trap_EA_View(bs->client, bs->viewanglescd );
    */
}

/*
==================
idBotLocal::BotCheckAttack
==================
 */
void idBotLocal::ABotCheckAttack( bot_state_t* bs )
{
    F32 fov, points;
    S32 attackentity;
    bsp_trace_t bsptrace;
    vec3_t forward, start, end, dir, angles;
    bsp_trace_t trace;
    aas_entityinfo_t entinfo;
    vec3_t mins = { -8, -8, -8 }, maxs = { 8, 8, 8 };
    weaponinfo_t wi;
    
    attackentity = bs->enemy;
    BotEntityInfo( attackentity, &entinfo );
    
    // if not attacking a player
    if( attackentity >= MAX_CLIENTS )
    {
        if( g_entities[entinfo.number].activator && g_entities[entinfo.number].activator->s.frame == 2 )
        {
            return;
        }
    }
    
    VectorSubtract( bs->aimtarget, bs->eye, dir );
    
    if( VectorLengthSquared( dir ) < Square( 100 ) )
    {
        fov = 120;
    }
    else
    {
        fov = 50;
    }
    
    vectoangles( dir, angles );
    
    if( !BotInFieldOfVision( bs->viewangles, fov, angles ) )
    {
        return;
    }
    
    BotAI_Trace( &bsptrace, bs->eye, NULL, NULL, bs->aimtarget, bs->client, CONTENTS_SOLID | CONTENTS_PLAYERCLIP );
    
    if( bsptrace.fraction < 1 && bsptrace.ent != attackentity )
    {
        return;
    }
    
    //get the start point shooting from
    VectorCopy( bs->origin, start );
    start[2] += bs->cur_ps.viewheight;
    
    AngleVectors( bs->viewangles, forward, NULL, NULL );
    
    //end point aiming at
    VectorMA( start, 1000, forward, end );
    
    //a little back to make sure not inside a very close enemy
    VectorMA( start, -12, forward, start );
    BotAI_Trace( &trace, start, mins, maxs, end, bs->entitynum, MASK_SHOT );
    
    //if the entity is a client
    if( trace.ent > 0 && trace.ent <= MAX_CLIENTS )
    {
        if( trace.ent != attackentity )
        {
            //if a teammate is hit
            if( BotSameTeam( bs, trace.ent ) )
            {
                return;
            }
        }
    }
    
    // TODO avoid radial damage
    if( wi.proj.damagetype & DAMAGETYPE_RADIAL )
    {
        if( trace.fraction * 1000 < wi.proj.radius )
        {
            points = ( wi.proj.damage - 0.5 * trace.fraction * 1000 ) * 0.5;
            if( points > 0 )
            {
                return;
            }
        }
    }
    
    //if fire has to be release to activate weapon
    if( wi.flags & WFL_FIRERELEASED )
    {
        if( bs->flags & BFL_ATTACKED )
        {
            trap_EA_Attack( bs->client );
        }
    }
    else
    {
        trap_EA_Attack( bs->client );
    }
    
    bs->flags ^= BFL_ATTACKED;
}

/*
==================
idBotLocal::HumansNearby
==================
*/
bool idBotLocal::HumansNearby( bot_state_t* bs )
{
    S32 i, num;
    gentity_t* other;
    S32 entityList[ MAX_GENTITIES ];
    vec3_t range = { AS_OVER_RT3, AS_OVER_RT3, AS_OVER_RT3 };
    vec3_t mins, maxs;
    
    //check there are no humans nearby
    VectorAdd( bs->origin , range, maxs );
    VectorSubtract( bs->origin, range, mins );
    
    num = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );
    for( i = 0; i < num; i++ )
    {
        other = &g_entities[ entityList[ i ] ];
        
        if( ( other->client && other->client->ps.stats[ STAT_TEAM ] == TEAM_HUMANS ) ||
                ( other->s.eType == ET_BUILDABLE && bggame->Buildable( ( buildable_t )other->s.modelindex )->team ==  TEAM_HUMANS ) )
        {
            return true;
        }
    }
    return false;
}

// armoury AI: buy stuff depending on credits (stage, situation)
#define CanEvoTo(x) -1 != bggame->ClassCanEvolveFromTo( (class_t)currentClass, x, \
  bs->cur_ps.persistant[ PERS_CREDIT ], \
  g_alienStage.integer, 0 )

/*
==================
idBotLocal::ABotClassOK
==================
*/
bool idBotLocal::ABotClassOK( bot_state_t* bs )
{
    S32 numEvos;
    S32 currentClass = bs->ent->client->ps.stats[ STAT_CLASS ];
    
    if( !HumansNearby( bs ) )
    {
        numEvos = bs->inventory[BI_CREDITS] + bggame->Class( ( class_t )currentClass )->value;
        switch( g_alienStage.integer )
        {
            case 0:
                if( CanEvoTo( PCL_ALIEN_LEVEL1 ) && currentClass != PCL_ALIEN_LEVEL1 )
                {
                    return false;
                }
                break;
            case 1:
                if( CanEvoTo( PCL_ALIEN_LEVEL2 ) && currentClass != PCL_ALIEN_LEVEL2 )
                {
                    return false;
                }
                break;
            case 2:
                if( CanEvoTo( PCL_ALIEN_LEVEL3 ) && currentClass != PCL_ALIEN_LEVEL3 )
                {
                    return false;
                }
                break;
            case 3:
                if( CanEvoTo( PCL_ALIEN_LEVEL4 ) && currentClass != PCL_ALIEN_LEVEL4 )
                {
                    return false;
                }
                break;
        }
    }
    else
    {
        return true;
    }
    return true;
}

/*
==================
idBotLocal::ABotEvolve
==================
*/
void idBotLocal::ABotEvolve( bot_state_t* bs )
{
    S32 numEvos;
    S32 currentClass = bs->ent->client->ps.stats[ STAT_CLASS ];
    
    numEvos = bs->inventory[BI_CREDITS] + bggame->Class( ( class_t )currentClass )->cost;
    switch( g_alienStage.integer )
    {
        case 0:
        case 1:
            if( CanEvoTo( PCL_ALIEN_LEVEL2 ) )
            {
                trap_EA_Command( bs->client, "class level1" );
            }
            break;
        case 2:
            if( CanEvoTo( PCL_ALIEN_LEVEL2 ) )
            {
                trap_EA_Command( bs->client, "class level2" );
            }
            break;
        case 3:
            if( CanEvoTo( PCL_ALIEN_LEVEL3 ) )
            {
                trap_EA_Command( bs->client, "class level3" );
            }
            break;
        case 4:
            if( CanEvoTo( PCL_ALIEN_LEVEL4 ) )
            {
                trap_EA_Command( bs->client, "class level4" );
            }
            break;
    }
    //now update your inventory
    ABotUpdateInventory( bs );
}

/*
==================
idBotLocal::ABotCheckRespawn
==================
*/
void idBotLocal::ABotCheckRespawn( bot_state_t* bs )
{
    UTF8 buf[144];
    
    if( bs->cur_ps.pm_type == PM_DEAD || bs->cur_ps.pm_type == PM_SPECTATOR || bs->cur_ps.pm_type == PM_FREEZE )
    {
        //on the spawn queue?
        if( bs->cur_ps.pm_flags & PMF_QUEUED )
        {
            return;
        }
        
        Com_sprintf( buf, sizeof( buf ), "class %s", "level0" );
        trap_EA_Command( bs->client, buf );
    }
}

/*
==================
idBotLocal::ABotUpdateInventory

inventory becomes quite useless, thanks to the bggame->Inventory functions..
==================
*/
void idBotLocal::ABotUpdateInventory( bot_state_t* bs )
{
    bs->inventory[BI_HEALTH] = bs->cur_ps.stats[ STAT_HEALTH ];
    bs->inventory[BI_CREDITS] = bs->cur_ps.persistant[ PERS_CREDIT ];
    bs->inventory[BI_CLASS] = bs->ent->client->pers.classSelection;
    bs->inventory[BI_WEAPON] = bs->ent->client->ps.weapon;
    
    //Dushan we don't want spam
    //Bot_Print( BPMSG, "\nI have %d health, %d credits and %d stamina", bs->cur_ps.stats[STAT_HEALTH], bs->cur_ps.persistant[ PERS_CREDIT ], bs->cur_ps.stats[ STAT_STAMINA ] );
}

/*
==================
idBotLocal::ABotFindEnemy

if enemy is found: set bs->goal and bs->enemy and return true
==================
*/
bool idBotLocal::ABotFindEnemy( bot_state_t* bs )
{
    // return 0 if no human building can be found
    
    //see if there's an enemy in range, and go for it
    if( BotGoalForEnemy( bs, &bs->goal ) )
    {
        goto gotenemy;
    }
    
    // go for turrets
    if( BotGoalForClosestBuildable( bs, &bs->goal, BA_H_MGTURRET, &idBotLocal::Nullcheckfuct ) )
    {
        goto gotenemy;
    }
    
    // go for teslagens
    if( BotGoalForClosestBuildable( bs, &bs->goal, BA_H_TESLAGEN, &idBotLocal::Nullcheckfuct ) )
    {
        goto gotenemy;
    }
    
    // go for armoury
    if( BotGoalForClosestBuildable( bs, &bs->goal, BA_H_ARMOURY, &idBotLocal::Nullcheckfuct ) )
    {
        goto gotenemy;
    }
    
    // go for defence computer
    if( BotGoalForClosestBuildable( bs, &bs->goal, BA_H_DCC, &idBotLocal::Nullcheckfuct ) )
    {
        goto gotenemy;
    }
    
    // go for medistation
    if( BotGoalForClosestBuildable( bs, &bs->goal, BA_H_MEDISTAT, &idBotLocal::Nullcheckfuct ) )
    {
        goto gotenemy;
    }
    
    // go for telenodes
    if( BotGoalForClosestBuildable( bs, &bs->goal, BA_H_SPAWN, &idBotLocal::Nullcheckfuct ) )
    {
        goto gotenemy;
    }
    
    // go for reactor
    if( BotGoalForClosestBuildable( bs, &bs->goal, BA_H_REACTOR, &idBotLocal::Nullcheckfuct ) )
    {
        goto gotenemy;
    }
    
    // go for repeater
    if( BotGoalForClosestBuildable( bs, &bs->goal, BA_H_REPEATER, &idBotLocal::Nullcheckfuct ) )
    {
        goto gotenemy;
    }
    
    // haven't returned yet so no enemy found > dont do anything
    BotAddInfo( bs, "enemy", "none" );
    return false;
    
gotenemy:
    bs->enemy = bs->goal.entitynum;
    BotAddInfo( bs, "enemy", va( "# %d", bs->enemy ) );
    
    return true;
}

/*
==================
idBotLocal::ABotAttack
==================
*/
bool idBotLocal::ABotAttack( bot_state_t* bs )
{
    bot_moveresult_t moveresult;
    vec3_t target, dir;
    
    //state transitions
    // dead -> spawn
    if( !BotIsAlive( bs ) )
    {
        bs->state = AS_SPAWN;
        return false;
    }
    
    // not satisfied with current class -> EVO
    if( !ABotClassOK( bs ) )
    {
        bs->state = AS_EVO;
        return false;
    }
    
    // report
    BotAddInfo( bs, "task", "attack" );
    
    // find target
    if( !ABotFindEnemy( bs ) )
    {
        return true;
    }
    
    // shoot if target in sight
    if( BotEntityVisible( bs->entitynum, bs->eye, bs->viewangles, 360, bs->enemy ) )
    {
        // aim and check attack
        BotAddInfo( bs, "action", "shoot" );
        ABotAimAtEnemy( bs );
        ABotCheckAttack( bs );
    }
    
    //if we are in range of the target, but can't see it, move around a bit
    // move to target
    bs->tfl = TFL_DEFAULT;
    BotAddInfo( bs, "action", va( "movetogoal %d", trap_AAS_AreaTravelTimeToGoalArea( bs->areanum, bs->cur_ps.origin, bs->goal.areanum, bs->tfl ) ) );
    BotSetupForMovement( bs );
    //move towards the goal
    trap_BotMoveToGoal( &moveresult, bs->ms, &bs->goal, bs->tfl );
    BotAIBlocked( bs, &moveresult, true );
    
    //BotMovementViewTarget(S32 movestate, bot_goal_t *goal, S32 travelflags, F32 lookahead, vec3_t target)
    if( trap_BotMovementViewTarget( bs->ms, &bs->goal, bs->tfl, 300, target ) )
    {
        //Bot_Print(BPMSG, "I'm uh...fixing my view angle?");
        VectorSubtract( target, bs->origin, dir );
        vectoangles( dir, bs->ideal_viewangles );
    }
    
    return true;
}

/*
==================
idBotLocal::ABotSpawn

go for arm and gear up
==================
*/
bool idBotLocal::ABotEvo( bot_state_t* bs )
{
    //bot_goal_t goal;
    
    //state transitions
    
    // dead -> spawn
    if( !BotIsAlive( bs ) )
    {
        bs->state = AS_SPAWN;
        return false;
    }
    
    // satisfied with current equip? -> attack!
    if( ABotClassOK( bs ) )
    {
        bs->state = AS_ATTACK;
        return false;
    }
    
    ABotEvolve( bs );
    BotAddInfo( bs, "task", "evolving!" );
    //now that we've bought ammo, move to the next state
    bs->state = AS_ATTACK;
    
    return false;
}

/*
==================
idBotLocal::ABotSpawn
==================
*/
bool idBotLocal::ABotSpawn( bot_state_t* bs )
{
    //state transitions
    if( BotIsAlive( bs ) )
    {
        bs->state = AS_EVO;
        return false;
    }
    
    ABotCheckRespawn( bs );
    return true;
}

/*
==================
idBotLocal::ABotrunstate

ABotrunstate: return false if state is changed,
so new state can be executed within the same frame
==================
*/
bool idBotLocal::ABotRunState( bot_state_t* bs )
{
    switch( bs->state )
    {
        case AS_SPAWN:
            return ABotSpawn( bs );
        case AS_EVO:
            return ABotEvo( bs );
        case AS_ATTACK:
            return ABotAttack( bs );
        default:
            Bot_Print( BPERROR, "bs->state irregular value %d \n", bs->state );
            bs->state = AS_SPAWN;
            return true;
    }
}

/*
==================
idBotLocal::BotAlienAI
==================
*/
void idBotLocal::BotAlienAI( bot_state_t* bs, F32 thinktime )
{
    //UTF8 buf[144];
    //bot_goal_t goal;
    //bot_moveresult_t moveresult;
    //S32 tt;
    //vec3_t target, dir;
    //UTF8 userinfo[MAX_INFO_STRING];
    
    //if the bot has just been setup
    if( bs->setupcount > 0 )
    {
        bs->setupcount--;
        if( bs->setupcount > 0 ) return;
        
        trap_EA_Command( bs->client, "team aliens" );
        //
        //bs->lastframe_health = bs->inventory[INVENTORY_HEALTH];
        bs->lasthitcount = bs->cur_ps.persistant[PERS_HITS];
        bs->setupcount = 0;
    }
    
    // update knowledge base and inventory
    ABotUpdateInventory( bs );
    
    // run the FSM
    bs->statecycles = 0;
    while( !ABotRunState( bs ) )
    {
        if( ++( bs->statecycles ) > 5 )
        {
            BotAddInfo( bs, "botstates", "loop" );
            break;
        }
    }
    
    // update the hud
    if( bot_developer.integer )
    {
        // fsm state
        BotAddInfo( bs, "state", va( "%d", bs->state ) );
        //Dushan - aliens don't have weapons and ammos for the weapons
        
        //target
        BotAddInfo( bs, "goal", va( "%d", bs->goal.entitynum ) );
        
        //Enemy Info
        if( bs->enemyent && bs->enemyent->client )
        {
            BotAddInfo( bs, "enemyname", va( "%s", bs->enemyent->client->pers.netname ) );
        }
        // copy config string
        trap_SetConfigstring( CS_BOTINFOS + bs->client, bs->hudinfo );
    }
}
