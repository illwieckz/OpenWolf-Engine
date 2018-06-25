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
// File name:   sg_client.cpp
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#include <sgame/sg_precompiled.h>

// g_client.c -- client functions that don't happen every frame

static vec3_t playerMins = { -15, -15, -24};
static vec3_t playerMaxs = {15, 15, 32};

/*QUAKED info_player_deathmatch (1 0 1) (-16 -16 -24) (16 16 32) initial
potential spawning position for deathmatch games.
The first time a player enters the game, they will be at an 'initial' spot.
Targets will be fired when someone spawns in on them.
"nobots" will prevent bots from using this spot.
"nohumans" will prevent non-bots from using this spot.
*/
void idGameLocal::SP_info_player_deathmatch( gentity_t* ent )
{
    S32   i;
    
    SpawnInt( "nobots", "0", &i );
    
    if( i )
        ent->flags |= FL_NO_BOTS;
        
    SpawnInt( "nohumans", "0", &i );
    if( i )
        ent->flags |= FL_NO_HUMANS;
}

/*QUAKED info_player_start (1 0 0) (-16 -16 -24) (16 16 32)
equivelant to info_player_deathmatch
*/
void idGameLocal::SP_info_player_start( gentity_t* ent )
{
    ent->classname = "info_player_deathmatch";
    SP_info_player_deathmatch( ent );
}

/*QUAKED info_player_intermission (1 0 1) (-16 -16 -24) (16 16 32)
The intermission will be viewed from this point.  Target an info_notnull for the view direction.
*/
void idGameLocal::SP_info_player_intermission( gentity_t* ent )
{
}

/*QUAKED info_alien_intermission (1 0 1) (-16 -16 -24) (16 16 32)
The intermission will be viewed from this point.  Target an info_notnull for the view direction.
*/
void idGameLocal::SP_info_alien_intermission( gentity_t* ent )
{
}

/*QUAKED info_human_intermission (1 0 1) (-16 -16 -24) (16 16 32)
The intermission will be viewed from this point.  Target an info_notnull for the view direction.
*/
void idGameLocal::SP_info_human_intermission( gentity_t* ent )
{
}

/*
===============
idGameLocal::AddCreditToClient
===============
*/
void idGameLocal::AddCreditToClient( gclient_t* client, S16 credit, bool cap )
{
    if( !client )
        return;
        
    //if we're already at the max and trying to add credit then stop
    if( cap )
    {
        if( client->ps.stats[ STAT_TEAM ] == TEAM_ALIENS )
        {
            if( client->ps.persistant[ PERS_CREDIT ] >= ALIEN_MAX_CREDITS &&
                    credit > 0 )
                return;
        }
        else if( client->ps.stats[ STAT_TEAM ] == TEAM_HUMANS )
        {
            if( client->ps.persistant[ PERS_CREDIT ] >= HUMAN_MAX_CREDITS &&
                    credit > 0 )
                return;
        }
    }
    
    client->ps.persistant[ PERS_CREDIT ] += credit;
    
    if( cap )
    {
        if( client->ps.stats[ STAT_TEAM ] == TEAM_ALIENS )
        {
            if( client->ps.persistant[ PERS_CREDIT ] > ALIEN_MAX_CREDITS )
                client->ps.persistant[ PERS_CREDIT ] = ALIEN_MAX_CREDITS;
        }
        else if( client->ps.stats[ STAT_TEAM ] == TEAM_HUMANS )
        {
            if( client->ps.persistant[ PERS_CREDIT ] > HUMAN_MAX_CREDITS )
                client->ps.persistant[ PERS_CREDIT ] = HUMAN_MAX_CREDITS;
        }
    }
    
    if( client->ps.persistant[ PERS_CREDIT ] < 0 )
        client->ps.persistant[ PERS_CREDIT ] = 0;
}

/*
================
idGameLocal::SpotWouldTelefrag
================
*/
bool idGameLocal::SpotWouldTelefrag( gentity_t* spot )
{
    S32       i, num;
    S32       touch[ MAX_GENTITIES ];
    gentity_t* hit;
    vec3_t    mins, maxs;
    
    VectorAdd( spot->s.origin, playerMins, mins );
    VectorAdd( spot->s.origin, playerMaxs, maxs );
    num = trap_EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );
    
    for( i = 0; i < num; i++ )
    {
        hit = &g_entities[ touch[ i ] ];
        //if ( hit->client && hit->client->ps.stats[STAT_HEALTH] > 0 ) {
        if( hit->client )
            return true;
    }
    
    return false;
}

/*
================
G_SelectNearestDeathmatchSpawnPoint

Find the spot that we DON'T want to use
================
*/
#define MAX_SPAWN_POINTS  128
gentity_t* idGameLocal::SelectNearestDeathmatchSpawnPoint( vec3_t from )
{
    gentity_t* spot;
    vec3_t    delta;
    F32     dist, nearestDist;
    gentity_t* nearestSpot;
    
    nearestDist = 999999;
    nearestSpot = NULL;
    spot = NULL;
    
    while( ( spot = Find( spot, FOFS( classname ), "info_player_deathmatch" ) ) != NULL )
    {
        VectorSubtract( spot->s.origin, from, delta );
        dist = VectorLength( delta );
        
        if( dist < nearestDist )
        {
            nearestDist = dist;
            nearestSpot = spot;
        }
    }
    
    return nearestSpot;
}


/*
================
G_SelectRandomDeathmatchSpawnPoint

go to a random point that doesn't telefrag
================
*/
#define MAX_SPAWN_POINTS  128
gentity_t* idGameLocal::SelectRandomDeathmatchSpawnPoint( void )
{
    gentity_t* spot;
    S32       count;
    S32       selection;
    gentity_t* spots[ MAX_SPAWN_POINTS ];
    
    count = 0;
    spot = NULL;
    
    while( ( spot = Find( spot, FOFS( classname ), "info_player_deathmatch" ) ) != NULL )
    {
        if( SpotWouldTelefrag( spot ) )
            continue;
            
        spots[ count ] = spot;
        count++;
    }
    
    if( !count ) // no spots that won't telefrag
        return Find( NULL, FOFS( classname ), "info_player_deathmatch" );
        
    selection = rand( ) % count;
    return spots[ selection ];
}


/*
===========
G_SelectRandomFurthestSpawnPoint

Chooses a player start, deathmatch start, etc
============
*/
gentity_t* idGameLocal::SelectRandomFurthestSpawnPoint( vec3_t avoidPoint, vec3_t origin, vec3_t angles )
{
    gentity_t* spot;
    vec3_t    delta;
    F32     dist;
    F32     list_dist[ 64 ];
    gentity_t* list_spot[ 64 ];
    S32       numSpots, rnd, i, j;
    
    numSpots = 0;
    spot = NULL;
    
    while( ( spot = Find( spot, FOFS( classname ), "info_player_deathmatch" ) ) != NULL )
    {
        if( SpotWouldTelefrag( spot ) )
            continue;
            
        VectorSubtract( spot->s.origin, avoidPoint, delta );
        dist = VectorLength( delta );
        
        for( i = 0; i < numSpots; i++ )
        {
            if( dist > list_dist[ i ] )
            {
                if( numSpots >= 64 )
                    numSpots = 64 - 1;
                    
                for( j = numSpots; j > i; j-- )
                {
                    list_dist[ j ] = list_dist[ j - 1 ];
                    list_spot[ j ] = list_spot[ j - 1 ];
                }
                
                list_dist[ i ] = dist;
                list_spot[ i ] = spot;
                numSpots++;
                
                if( numSpots > 64 )
                    numSpots = 64;
                    
                break;
            }
        }
        
        if( i >= numSpots && numSpots < 64 )
        {
            list_dist[ numSpots ] = dist;
            list_spot[ numSpots ] = spot;
            numSpots++;
        }
    }
    
    if( !numSpots )
    {
        spot = Find( NULL, FOFS( classname ), "info_player_deathmatch" );
        
        if( !spot )
            Error( "Couldn't find a spawn point" );
            
        VectorCopy( spot->s.origin, origin );
        origin[ 2 ] += 9;
        VectorCopy( spot->s.angles, angles );
        return spot;
    }
    
    // select a random spot from the spawn points furthest away
    rnd = random( ) * ( numSpots / 2 );
    
    VectorCopy( list_spot[ rnd ]->s.origin, origin );
    origin[ 2 ] += 9;
    VectorCopy( list_spot[ rnd ]->s.angles, angles );
    
    return list_spot[ rnd ];
}


/*
================
G_SelectAlienSpawnPoint

go to a random point that doesn't telefrag
================
*/
gentity_t* idGameLocal::SelectAlienSpawnPoint( vec3_t preference )
{
    gentity_t* spot;
    S32       count;
    gentity_t* spots[ MAX_SPAWN_POINTS ];
    
    if( level.numAlienSpawns <= 0 )
        return NULL;
        
    count = 0;
    spot = NULL;
    
    while( ( spot = Find( spot, FOFS( classname ), bggame->Buildable( BA_A_SPAWN )->entityName ) ) != NULL )
    {
        if( !spot->spawned )
            continue;
            
        if( spot->health <= 0 )
            continue;
            
        if( !spot->s.groundEntityNum )
            continue;
            
        if( spot->clientSpawnTime > 0 )
            continue;
            
        if( CheckSpawnPoint( spot->s.number, spot->s.origin, spot->s.origin2, BA_A_SPAWN, NULL ) != NULL )
            continue;
            
        spots[ count ] = spot;
        count++;
    }
    
    if( !count )
        return NULL;
        
    return ClosestEnt( preference, spots, count );
}


/*
================
G_SelectHumanSpawnPoint

go to a random point that doesn't telefrag
================
*/
gentity_t* idGameLocal::SelectHumanSpawnPoint( vec3_t preference )
{
    gentity_t* spot;
    S32       count;
    gentity_t* spots[ MAX_SPAWN_POINTS ];
    
    if( level.numHumanSpawns <= 0 )
        return NULL;
        
    count = 0;
    spot = NULL;
    
    while( ( spot = Find( spot, FOFS( classname ), bggame->Buildable( BA_H_SPAWN )->entityName ) ) != NULL )
    {
        if( !spot->spawned )
            continue;
            
        if( spot->health <= 0 )
            continue;
            
        if( !spot->s.groundEntityNum )
            continue;
            
        if( spot->clientSpawnTime > 0 )
            continue;
            
        if( CheckSpawnPoint( spot->s.number, spot->s.origin, spot->s.origin2, BA_H_SPAWN, NULL ) != NULL )
            continue;
            
        spots[ count ] = spot;
        count++;
    }
    
    if( !count )
        return NULL;
        
    return ClosestEnt( preference, spots, count );
}


/*
===========
idGameLocal::SelectSpawnPoint

Chooses a player start, deathmatch start, etc
============
*/
gentity_t* idGameLocal::SelectSpawnPoint( vec3_t avoidPoint, vec3_t origin, vec3_t angles )
{
    return SelectRandomFurthestSpawnPoint( avoidPoint, origin, angles );
}


/*
===========
idGameLocal::SelectTremulousSpawnPoint

Chooses a player start, deathmatch start, etc
============
*/
gentity_t* idGameLocal::SelectTremulousSpawnPoint( team_t team, vec3_t preference, vec3_t origin, vec3_t angles )
{
    gentity_t* spot = NULL;
    
    if( team == TEAM_ALIENS )
        spot = SelectAlienSpawnPoint( preference );
    else if( team == TEAM_HUMANS )
        spot = SelectHumanSpawnPoint( preference );
        
    //no available spots
    if( !spot )
        return NULL;
        
    if( team == TEAM_ALIENS )
        CheckSpawnPoint( spot->s.number, spot->s.origin, spot->s.origin2, BA_A_SPAWN, origin );
    else if( team == TEAM_HUMANS )
        CheckSpawnPoint( spot->s.number, spot->s.origin, spot->s.origin2, BA_H_SPAWN, origin );
        
    VectorCopy( spot->s.angles, angles );
    angles[ ROLL ] = 0;
    
    return spot;
    
}

/*
===========
G_SelectInitialSpawnPoint

Try to find a spawn point marked 'initial', otherwise
use normal spawn selection.
============
*/
gentity_t* idGameLocal::SelectInitialSpawnPoint( vec3_t origin, vec3_t angles )
{
    gentity_t* spot;
    
    spot = NULL;
    while( ( spot = Find( spot, FOFS( classname ), "info_player_deathmatch" ) ) != NULL )
    {
        if( spot->spawnflags & 1 )
            break;
    }
    
    if( !spot || SpotWouldTelefrag( spot ) )
    {
        return SelectSpawnPoint( vec3_origin, origin, angles );
    }
    
    VectorCopy( spot->s.origin, origin );
    origin[ 2 ] += 9;
    VectorCopy( spot->s.angles, angles );
    
    return spot;
}

/*
===========
G_SelectSpectatorSpawnPoint

============
*/
gentity_t* idGameLocal::SelectSpectatorSpawnPoint( vec3_t origin, vec3_t angles )
{
    FindIntermissionPoint( );
    
    VectorCopy( level.intermission_origin, origin );
    VectorCopy( level.intermission_angle, angles );
    
    return NULL;
}


/*
===========
idGameLocal::SelectAlienLockSpawnPoint

Try to find a spawn point for alien intermission otherwise
use normal intermission spawn.
============
*/
gentity_t* idGameLocal::SelectAlienLockSpawnPoint( vec3_t origin, vec3_t angles )
{
    gentity_t* spot;
    
    spot = NULL;
    spot = Find( spot, FOFS( classname ), "info_alien_intermission" );
    
    if( !spot )
        return SelectSpectatorSpawnPoint( origin, angles );
        
    VectorCopy( spot->s.origin, origin );
    VectorCopy( spot->s.angles, angles );
    
    return spot;
}


/*
===========
idGameLocal::SelectHumanLockSpawnPoint

Try to find a spawn point for human intermission otherwise
use normal intermission spawn.
============
*/
gentity_t* idGameLocal::SelectHumanLockSpawnPoint( vec3_t origin, vec3_t angles )
{
    gentity_t* spot;
    
    spot = NULL;
    spot = Find( spot, FOFS( classname ), "info_human_intermission" );
    
    if( !spot )
        return SelectSpectatorSpawnPoint( origin, angles );
        
    VectorCopy( spot->s.origin, origin );
    VectorCopy( spot->s.angles, angles );
    
    return spot;
}


/*
=======================================================================

BODYQUE

=======================================================================
*/

/*
=============
BodySink

After sitting around for five seconds, fall into the ground and dissapear
=============
*/
void idGameLocal::BodySink( gentity_t* ent )
{
    //run on first BodySink call
    if( !ent->active )
    {
        ent->active = true;
        
        //sinking bodies can't be infested
        ent->killedBy = ent->s.misc = MAX_CLIENTS;
        ent->timestamp = level.time;
    }
    
    if( level.time - ent->timestamp > 6500 )
    {
        FreeEntity( ent );
        return;
    }
    
    ent->nextthink = level.time + 100;
    ent->s.pos.trBase[ 2 ] -= 1;
}


/*
=============
BodyFree

After sitting around for a while the body becomes a freebie
=============
*/
void idGameLocal::BodyFree( gentity_t* ent )
{
    ent->killedBy = -1;
    
    //if not claimed in the next minute destroy
    ent->think = BodySink;
    ent->nextthink = level.time + 60000;
}


/*
=============
idGameLocal::SpawnCorpse

A player is respawning, so make an entity that looks
just like the existing corpse to leave behind.
=============
*/
void idGameLocal::SpawnCorpse( gentity_t* ent )
{
    gentity_t*   body;
    S32         contents;
    vec3_t      origin, dest;
    trace_t     tr;
    F32       vDiff;
    
    VectorCopy( ent->r.currentOrigin, origin );
    
    trap_UnlinkEntity( ent );
    
    // if client is in a nodrop area, don't leave the body
    contents = trap_PointContents( origin, -1 );
    if( contents & CONTENTS_NODROP )
        return;
        
    body = Spawn( );
    
    VectorCopy( ent->s.apos.trBase, body->s.angles );
    body->s.eFlags = EF_DEAD;
    body->s.eType = ET_CORPSE;
    body->s.number = body - g_entities;
    body->timestamp = level.time;
    body->s.event = 0;
    body->r.contents = CONTENTS_CORPSE;
    body->s.clientNum = ent->client->ps.stats[ STAT_CLASS ];
    body->nonSegModel = ent->client->ps.persistant[ PERS_STATE ] & PS_NONSEGMODEL;
    
    if( ent->client->ps.stats[ STAT_TEAM ] == TEAM_HUMANS )
        body->classname = "humanCorpse";
    else
        body->classname = "alienCorpse";
        
    body->s.misc = MAX_CLIENTS;
    
    body->think = &idGameLocal::BodySink;
    body->nextthink = level.time + 20000;
    
    body->s.legsAnim = ent->s.legsAnim;
    
    if( !body->nonSegModel )
    {
        switch( body->s.legsAnim & ~ANIM_TOGGLEBIT )
        {
            case BOTH_DEATH1:
            case BOTH_DEAD1:
                body->s.torsoAnim = body->s.legsAnim = BOTH_DEAD1;
                break;
            case BOTH_DEATH2:
            case BOTH_DEAD2:
                body->s.torsoAnim = body->s.legsAnim = BOTH_DEAD2;
                break;
            case BOTH_DEATH3:
            case BOTH_DEAD3:
            default:
                body->s.torsoAnim = body->s.legsAnim = BOTH_DEAD3;
                break;
        }
    }
    else
    {
        switch( body->s.legsAnim & ~ANIM_TOGGLEBIT )
        {
            case NSPA_DEATH1:
            case NSPA_DEAD1:
                body->s.legsAnim = NSPA_DEAD1;
                break;
            case NSPA_DEATH2:
            case NSPA_DEAD2:
                body->s.legsAnim = NSPA_DEAD2;
                break;
            case NSPA_DEATH3:
            case NSPA_DEAD3:
            default:
                body->s.legsAnim = NSPA_DEAD3;
                break;
        }
    }
    
    body->takedamage = false;
    
    body->health = ent->health = ent->client->ps.stats[ STAT_HEALTH ];
    ent->health = 0;
    
    //change body dimensions
    bggame->ClassBoundingBox( ( class_t )ent->client->ps.stats[ STAT_CLASS ], NULL, NULL, NULL, body->r.mins, body->r.maxs );
    vDiff = body->r.mins[ 2 ] - ent->r.mins[ 2 ];
    
    //drop down to match the *model* origins of ent and body
    VectorSet( dest, origin[ 0 ], origin[ 1 ], origin[ 2 ] - vDiff );
    trap_Trace( &tr, origin, body->r.mins, body->r.maxs, dest, body->s.number, body->clipmask );
    VectorCopy( tr.endpos, origin );
    
    SetOrigin( body, origin );
    VectorCopy( origin, body->s.origin );
    body->s.pos.trType = TR_GRAVITY;
    body->s.pos.trTime = level.time;
    VectorCopy( ent->client->ps.velocity, body->s.pos.trDelta );
    
    VectorCopy( body->s.pos.trBase, body->r.currentOrigin );
    trap_LinkEntity( body );
}

//======================================================================


/*
==================
idGameLocal::SetClientViewAngle
==================
*/
void idGameLocal::SetClientViewAngle( gentity_t* ent, vec3_t angle )
{
    S32     i;
    
    // set the delta angle
    for( i = 0; i < 3; i++ )
    {
        S32   cmdAngle;
        
        cmdAngle = ANGLE2SHORT( angle[ i ] );
        ent->client->ps.delta_angles[ i ] = cmdAngle - ent->client->pers.cmd.angles[ i ];
    }
    
    VectorCopy( angle, ent->s.angles );
    VectorCopy( ent->s.angles, ent->client->ps.viewangles );
}

/*
================
idGameLocal::respawn
================
*/
void idGameLocal::respawn( gentity_t* ent )
{
    S32 i;
    
    SpawnCorpse( ent );
    
    // Clients can't respawn - they must go through the class cmd
    ent->client->pers.classSelection = PCL_NONE;
    gameLocal.ClientSpawn( ent, NULL, NULL, NULL );
    
    // stop any following clients that don't have sticky spec on
    for( i = 0; i < level.maxclients; i++ )
    {
        if( level.clients[ i ].sess.spectatorState == SPECTATOR_FOLLOW && level.clients[ i ].sess.spectatorClient == ent - g_entities )
        {
            if( !( level.clients[ i ].pers.stickySpec ) )
            {
                if( !FollowNewClient( &g_entities[ i ], 1 ) )
                    StopFollowing( &g_entities[ i ] );
            }
            else
                FollowLockView( &g_entities[ i ] );
        }
    }
}

bool idGameLocal::IsEmoticon( StringEntry s, bool* escaped )
{
    S32 i, j;
    StringEntry p = s;
    UTF8 emoticon[ MAX_EMOTICON_NAME_LEN ] = {""};
    bool escape = false;
    
    if( *p != '[' )
        return false;
    p++;
    if( *p == '[' )
    {
        escape = true;
        p++;
    }
    i = 0;
    while( *p && i < ( MAX_EMOTICON_NAME_LEN - 1 ) )
    {
        if( *p == ']' )
        {
            for( j = 0; j < level.emoticonCount; j++ )
            {
                if( !Q_stricmp( emoticon, level.emoticons[ j ] ) )
                {
                    *escaped = escape;
                    return true;
                }
            }
            return false;
        }
        emoticon[ i++ ] = *p;
        emoticon[ i ] = '\0';
        p++;
    }
    return false;
}

/*
===========
G_ClientCleanName
============
*/
void idGameLocal::ClientCleanName( StringEntry in, UTF8* out, S32 outSize )
{
    S32   len, colorlessLen;
    UTF8*  p;
    S32   spaces;
    bool escaped;
    bool invalid = false;
    
    //save room for trailing null byte
    outSize--;
    
    len = 0;
    colorlessLen = 0;
    p = out;
    *p = 0;
    spaces = 0;
    
    for( ; *in; in++ )
    {
        // don't allow leading spaces
        if( colorlessLen == 0 && *in == ' ' )
            continue;
            
        // don't allow nonprinting characters or (dead) console keys
        if( *in < ' ' || *in > '}' || *in == '`' )
            continue;
            
        // check colors
        if( Q_IsColorString( in ) )
        {
            in++;
            
            // make sure room in dest for both chars
            if( len > outSize - 2 )
                break;
                
            *out++ = Q_COLOR_ESCAPE;
            
            // don't allow black in a name, period
            if( ColorIndex( *in ) == 0 )
                *out++ = COLOR_WHITE;
            else
                *out++ = *in;
                
            len += 2;
            continue;
        }
        else if( !g_emoticonsAllowedInNames.integer && IsEmoticon( in, &escaped ) )
        {
            // make sure room in dest for both chars
            if( len > outSize - 2 )
                break;
                
            *out++ = '[';
            *out++ = '[';
            len += 2;
            if( escaped )
                in++;
            continue;
        }
        
        // don't allow too many consecutive spaces
        if( *in == ' ' )
        {
            spaces++;
            if( spaces > 3 )
                continue;
        }
        else
            spaces = 0;
            
        if( len > outSize - 1 )
            break;
            
        *out++ = *in;
        colorlessLen++;
        len++;
    }
    
    *out = 0;
    
    // don't allow names beginning with "[skipnotify]" because it messes up /ignore-related code
    if( !Q_stricmpn( p, "[skipnotify]", 12 ) )
        invalid = true;
        
    // don't allow comment-beginning strings because it messes up various parsers
    if( strstr( p, "//" ) || strstr( p, "/*" ) )
        invalid = true;
        
    // don't allow empty names
    if( *p == 0 || colorlessLen == 0 )
        invalid = true;
        
    // if something made the name bad, put them back to UnnamedPlayer
    if( invalid )
        Q_strncpyz( p, "UnnamedPlayer", outSize );
}


/*
======================
G_NonSegModel

Reads an animation.cfg to check for nonsegmentation
======================
*/
bool idGameLocal::NonSegModel( StringEntry filename )
{
    UTF8*          text_p;
    S32           len;
    UTF8*          token;
    UTF8          text[ 20000 ];
    fileHandle_t  f;
    
    // load the file
    len = trap_FS_FOpenFile( filename, &f, FS_READ );
    if( !f )
    {
        Printf( "File not found: %s\n", filename );
        return false;
    }
    
    if( len < 0 )
        return false;
        
    if( len == 0 || len >= sizeof( text ) - 1 )
    {
        trap_FS_FCloseFile( f );
        Printf( "File %s is %s\n", filename, len == 0 ? "empty" : "too long" );
        return false;
    }
    
    trap_FS_Read( text, len, f );
    text[ len ] = 0;
    trap_FS_FCloseFile( f );
    
    // parse the text
    text_p = text;
    
    // read optional parameters
    while( 1 )
    {
        token = COM_Parse( &text_p );
        
        //EOF
        if( !token[ 0 ] )
            break;
            
        if( !Q_stricmp( token, "nonsegmented" ) )
            return true;
    }
    
    return false;
}

/*
===========
idGameLocal::ClientUserinfoChanged

Called from ClientConnect when the player first connects and
directly by the server system when the player updates a userinfo variable.

The game can override any of the settings and call trap_SetUserinfo
if desired.
============
*/
void idGameLocal::ClientUserinfoChanged( S32 clientNum )
{
    gentity_t* ent;
    S32       health;
    S32       botskill;
    UTF8*      s;
    UTF8*      s2;
    UTF8      model[ MAX_QPATH ];
    UTF8      buffer[ MAX_QPATH ];
    UTF8      filename[ MAX_QPATH ];
    UTF8      oldname[ MAX_NAME_LENGTH ];
    UTF8      newname[ MAX_NAME_LENGTH ];
    UTF8      err[ MAX_STRING_CHARS ];
    bool  revertName = false;
    gclient_t* client;
    UTF8      c1[ MAX_INFO_STRING ];
    UTF8      c2[ MAX_INFO_STRING ];
    UTF8      userinfo[ MAX_INFO_STRING ];
    
    ent = g_entities + clientNum;
    client = ent->client;
    
    trap_GetUserinfo( clientNum, userinfo, sizeof( userinfo ) );
    
    // check for malformed or illegal info strings
    if( !Info_Validate( userinfo ) )
        strcpy( userinfo, "\\name\\badinfo" );
        
    // stickyspec toggle
    s = Info_ValueForKey( userinfo, "cg_stickySpec" );
    client->pers.stickySpec = atoi( s ) != 0;
    
    // set name
    Q_strncpyz( oldname, client->pers.netname, sizeof( oldname ) );
    s = Info_ValueForKey( userinfo, "name" );
    ClientCleanName( s, newname, sizeof( newname ) );
    
    if( strcmp( oldname, newname ) )
    {
        if( client->pers.nameChangeTime &&
                ( level.time - client->pers.nameChangeTime )
                <= ( g_minNameChangePeriod.value * 1000 ) )
        {
            trap_SendServerCommand( ent - g_entities, va(
                                        "print \"Name change spam protection (g_minNameChangePeriod = %d)\n\"",
                                        g_minNameChangePeriod.integer ) );
            revertName = true;
        }
        else if( g_maxNameChanges.integer > 0
                 && client->pers.nameChanges >= g_maxNameChanges.integer )
        {
            trap_SendServerCommand( ent - g_entities, va(
                                        "print \"Maximum name changes reached (g_maxNameChanges = %d)\n\"",
                                        g_maxNameChanges.integer ) );
            revertName = true;
        }
        else if( client->pers.muted )
        {
            trap_SendServerCommand( ent - g_entities,
                                    "print \"You cannot change your name while you are muted\n\"" );
            revertName = true;
        }
        else if( !adminLocal.AdminNameCheck( ent, newname, err, sizeof( err ) ) )
        {
            trap_SendServerCommand( ent - g_entities, va( "print \"%s\n\"", err ) );
            revertName = true;
        }
        
        if( revertName )
        {
            Q_strncpyz( client->pers.netname, *oldname ? oldname : "UnnamedPlayer",
                        sizeof( client->pers.netname ) );
            Info_SetValueForKey( userinfo, "name", oldname );
            trap_SetUserinfo( clientNum, userinfo );
        }
        else
        {
            Q_strncpyz( client->pers.netname, newname,
                        sizeof( client->pers.netname ) );
            if( client->pers.connected == CON_CONNECTED )
            {
                client->pers.nameChangeTime = level.time;
                client->pers.nameChanges++;
            }
        }
    }
    
    if( client->sess.spectatorState == SPECTATOR_SCOREBOARD )
        Q_strncpyz( client->pers.netname, "scoreboard", sizeof( client->pers.netname ) );
        
    if( *oldname )
    {
        if( strcmp( oldname, client->pers.netname ) )
        {
            trap_SendServerCommand( -1, va( "print \"%s" S_COLOR_WHITE
                                            " renamed to %s\n\"", oldname, client->pers.netname ) );
            LogPrintf( "ClientRename: %i [%s] (%s) \"%s^7\" -> \"%s^7\"\n", clientNum,
                       client->pers.ip, client->pers.guid, oldname, client->pers.netname );
            adminLocal.AdminNamelogUpdate( client, false );
            
            if( trap_Database_RunQuery( va( "INSERT INTO clientrename (playerid,oldname,newname) VALUES (%d,\"%s\",\"%s\")", ent->client->pers.mysqlid, oldname, ent->client->pers.netname ) ) == true )
            {
                UTF8 data[255];
                trap_Database_FinishQuery();
                trap_Database_RunQuery( "SELECT id FROM clientrename ORDER BY id DESC LIMIT 1" );
                trap_Database_FetchRow();
                trap_Database_FetchFieldByName( "id", data, sizeof( data ) );
                trap_Database_RunQuery( va( "INSERT into mainlog (idaction,typeaction,date) VALUES (%d,\"%d\",NOW())", atoi( data ), 3 ) );
                trap_Database_FinishQuery();
            }
            
        }
    }
    
    // set max health
    health = atoi( Info_ValueForKey( userinfo, "handicap" ) );
    client->pers.maxHealth = health;
    
    if( client->pers.classSelection == PCL_NONE )
    {
        //This looks hacky and frankly it is. The clientInfo string needs to hold different
        //model details to that of the spawning class or the info change will not be
        //registered and an axis appears instead of the player model. There is zero chance
        //the player can spawn with the battlesuit, hence this choice.
        Com_sprintf( buffer, MAX_QPATH, "%s/%s", bggame->ClassConfig( PCL_HUMAN_BSUIT )->modelName,
                     bggame->ClassConfig( PCL_HUMAN_BSUIT )->skinName );
    }
    else
    {
        Com_sprintf( buffer, MAX_QPATH, "%s/%s", bggame->ClassConfig( client->pers.classSelection )->modelName,
                     bggame->ClassConfig( client->pers.classSelection )->skinName );
                     
        //model segmentation
        Com_sprintf( filename, sizeof( filename ), "models/players/%s/animation.cfg",
                     bggame->ClassConfig( client->pers.classSelection )->modelName );
                     
        if( NonSegModel( filename ) )
            client->ps.persistant[ PERS_STATE ] |= PS_NONSEGMODEL;
        else
            client->ps.persistant[ PERS_STATE ] &= ~PS_NONSEGMODEL;
    }
    Q_strncpyz( model, buffer, sizeof( model ) );
    
    // wallwalk follow
    s = Info_ValueForKey( userinfo, "cg_wwFollow" );
    
    if( atoi( s ) )
        client->ps.persistant[ PERS_STATE ] |= PS_WALLCLIMBINGFOLLOW;
    else
        client->ps.persistant[ PERS_STATE ] &= ~PS_WALLCLIMBINGFOLLOW;
        
    // wallwalk toggle
    s = Info_ValueForKey( userinfo, "cg_wwToggle" );
    
    if( atoi( s ) )
        client->ps.persistant[ PERS_STATE ] |= PS_WALLCLIMBINGTOGGLE;
    else
        client->ps.persistant[ PERS_STATE ] &= ~PS_WALLCLIMBINGTOGGLE;
        
    // always sprint
    s = Info_ValueForKey( userinfo, "cg_alwaysSprint" );
    
    if( atoi( s ) )
        client->ps.persistant[ PERS_STATE ] |= PS_ALWAYSSPRINT;
    else
        client->ps.persistant[ PERS_STATE ] &= ~PS_ALWAYSSPRINT;
        
    // fly speed
    s = Info_ValueForKey( userinfo, "cg_flySpeed" );
    
    if( *s )
        client->pers.flySpeed = atoi( s );
    else
        client->pers.flySpeed = bggame->Class( PCL_NONE )->speed;
        
    // teamInfo
    s = Info_ValueForKey( userinfo, "teamoverlay" );
    s2 = Info_ValueForKey( userinfo, "cg_drawTeamStatus" );
    
    if( atoi( s ) != 0 || atoi( s2 ) != 0 )
        client->pers.teamInfo = true;
    else
        client->pers.teamInfo = false;
        
    s = Info_ValueForKey( userinfo, "cg_unlagged" );
    if( !s[0] || atoi( s ) != 0 )
        client->pers.useUnlagged = true;
    else
        client->pers.useUnlagged = false;
        
    // colors
    strcpy( c1, Info_ValueForKey( userinfo, "color1" ) );
    strcpy( c2, Info_ValueForKey( userinfo, "color2" ) );
    
    // bot skill level
    s = Info_ValueForKey( userinfo, "skill" );
    if( !s[0] || atoi( s ) == 0 )
        botskill = 0;
    else
        botskill = atoi( s );
        
        
    Q_strncpyz( client->pers.voice, Info_ValueForKey( userinfo, "voice" ),
                sizeof( client->pers.voice ) );
                
    // send over a subset of the userinfo keys so other clients can
    // print scoreboards, display models, and play custom sounds
    
    Com_sprintf( userinfo, sizeof( userinfo ),
                 "n\\%s\\t\\%i\\model\\%s\\c1\\%s\\c2\\%s\\"
                 "hc\\%i\\ig\\%16s\\v\\%s\\skill\\%i",
                 client->pers.netname, client->pers.teamSelection, model, c1, c2,
                 client->pers.maxHealth, bggame->ClientListString( &client->sess.ignoreList ),
                 client->pers.voice, botskill );
                 
    trap_SetConfigstring( CS_PLAYERS + clientNum, userinfo );
    
    // log to demo
    DemoCommand( DC_CLIENT_SET, va( "%d %s", clientNum, userinfo ) );
    
//  LogPrintf( "ClientUserinfoChanged: %i %s\n", clientNum, userinfo );
}


/*
===========
idGameLocal::ClientConnect

Called when a player begins connecting to the server.
Called again for every map change or tournement restart.

The session information will be valid after exit.

Return NULL if the client should be allowed, otherwise return
a string with the reason for denial.

Otherwise, the client will be sent the current gamestate
and will eventually get to ClientBegin.

firstTime will be true the very first time a client connects
to the server machine, but false on map changes and tournement
restarts.
============
*/
UTF8* idGameLocal::ClientConnect( S32 clientNum, bool firstTime, bool isBot )
{
    UTF8*      value;
    gclient_t* client;
    UTF8      userinfo[ MAX_INFO_STRING ];
    gentity_t* ent;
    UTF8      guid[ 33 ];
    UTF8      reason[ MAX_STRING_CHARS ] = {""};
    g_admin_admin_t* admin;
    
    ent = &g_entities[ clientNum ];
    client = &level.clients[ clientNum ];
    ent->client = client;
    ::memset( client, 0, sizeof( *client ) );
    
    trap_GetUserinfo( clientNum, userinfo, sizeof( userinfo ) );
    
    value = Info_ValueForKey( userinfo, "cl_guid" );
    Q_strncpyz( guid, value, sizeof( guid ) );
    
    // check for admin ban
    if( adminLocal.AdminBanCheck( userinfo, reason, sizeof( reason ) ) )
    {
        return va( "%s", reason );
    }
    
    // check for a password
    value = Info_ValueForKey( userinfo, "password" );
    
    if( g_password.string[ 0 ] && Q_stricmp( g_password.string, "none" ) &&
            strcmp( g_password.string, value ) != 0 )
        return "Invalid password";
        
    // add guid to session so we don't have to keep parsing userinfo everywhere
    if( !guid[0] )
    {
        Q_strncpyz( client->pers.guid, "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
                    sizeof( client->pers.guid ) );
    }
    else
    {
        Q_strncpyz( client->pers.guid, guid, sizeof( client->pers.guid ) );
    }
    
    // save ip
    value = Info_ValueForKey( userinfo, "ip" );
    Q_strncpyz( client->pers.ip, value, sizeof( client->pers.ip ) );
    
    // check for local client
    if( !strcmp( client->pers.ip, "localhost" ) )
        client->pers.localClient = true;
        
    admin = adminLocal.Admin( ent );
    client->pers.admin = admin;
    
    client->pers.connected = CON_CONNECTING;
    
    // read or initialize the session data
    if( firstTime || level.newSession )
        InitSessionData( client, userinfo );
        
    ReadSessionData( client );
    
    if( isBot )
    {
        ent->r.svFlags |= SVF_BOT;
        ent->inuse = true;
        if( !BotConnect( clientNum, !firstTime ) )
        {
            return "BotConnectfailed";
        }
    }
    
    // get and distribute relevent paramters
    ClientUserinfoChanged( clientNum );
    LogPrintf( "idGameLocal::ClientConnect: %i [%s] (%s) \"%s^7\"\n", clientNum,
               client->pers.ip, client->pers.guid, client->pers.netname );
               
    if( trap_Database_RunQuery( va( "SELECT qkey FROM players WHERE qkey = \"%s\"", client->pers.guid ) ) == true )
    {
        UTF8 data[255];
        
        if( trap_Database_FetchRow() == true )
        {
            //
        }
        else
        {
            trap_Database_FinishQuery();
            if( trap_Database_RunQuery( va( "INSERT into players (qkey,name) VALUES (\"%s\",\"%s\")", client->pers.guid, client->pers.netname ) ) == true )
            {
            }
            trap_Database_FinishQuery();
        }
        trap_Database_FinishQuery();
        if( trap_Database_RunQuery( va( "SELECT id FROM players WHERE qkey = \"%s\" LIMIT 1", client->pers.guid ) ) == true )
        {
            trap_Database_FetchRow();
            trap_Database_FetchFieldByName( "id", data, sizeof( data ) );
            client->pers.mysqlid = atoi( data );
            trap_Database_FinishQuery();
            trap_Database_RunQuery( va( "INSERT into connect (playerid,ip,playername) VALUES (%d,\"%s\",\"%s\")", client->pers.mysqlid, client->pers.ip, client->pers.netname ) );
            trap_Database_FinishQuery();
            trap_Database_RunQuery( "SELECT id FROM connect ORDER BY id DESC LIMIT 1" );
            trap_Database_FetchRow();
            trap_Database_FetchFieldByName( "id", data, sizeof( data ) );
            trap_Database_FinishQuery();
            trap_Database_RunQuery( va( "INSERT into mainlog (idaction,typeaction,date) VALUES (%d,\"%d\",NOW())", atoi( data ), 4 ) );
            trap_Database_FinishQuery();
        }
    }
    else
    {
        //database error
    }
    
    // don't do the "xxx connected" messages if they were caried over from previous level
    if( firstTime )
        trap_SendServerCommand( -1, va( "print \"%s" S_COLOR_WHITE " connected\n\"", client->pers.netname ) );
        
    // count current clients and rank for scoreboard
    CalculateRanks( );
    
    // if this is after !restart keepteams or !restart switchteams, apply said selection
    if( client->sess.restartTeam != TEAM_NONE )
    {
        ChangeTeam( ent, client->sess.restartTeam );
        client->sess.restartTeam = TEAM_NONE;
    }
    
    adminLocal.AdminNamelogUpdate( client, false );
    return NULL;
}

/*
===========
idGameLocal::ClientBegin

called when a client has finished connecting, and is ready
to be placed into the level.  This will happen every level load,
and on transition between teams, but doesn't happen on respawns
============
*/
void idGameLocal::ClientBegin( S32 clientNum )
{
    gentity_t* ent;
    gclient_t* client;
    UTF8      userinfo[ MAX_INFO_STRING ];
    S32       flags;
    g_admin_admin_t* admin;
    
    trap_GetUserinfo( clientNum, userinfo, sizeof( userinfo ) );
    
    if( Q_stricmp( Info_ValueForKey( userinfo, "cg_version" ), PRODUCT_NAME ) )
    {
        trap_SendServerCommand( clientNum, "disconnect \"Your client is missing files.\n\n"
                                "To enjoy our games in full colour and detail you need to enable autodownload (cl_allowDownload 1).\n"
                                "For a client with fast http-download visit localhost\n\n" //Dushan - change localhost
                                "Open your console and enter: /cl_allowDownload 1\n\"" );
        return;
    }
    
    ent = g_entities + clientNum;
    
    client = level.clients + clientNum;
    
    if( ent->r.linked )
        trap_UnlinkEntity( ent );
        
    InitGentity( ent );
    ent->touch = 0;
    ent->pain = 0;
    ent->client = client;
    
    client->pers.connected = CON_CONNECTED;
    client->pers.enterTime = level.time;
    client->pers.classSelection = PCL_NONE;
    
    // save eflags around this, because changing teams will
    // cause this to happen with a valid entity, and we
    // want to make sure the teleport bit is set right
    // so the viewpoint doesn't interpolate through the
    // world to the new position
    flags = client->ps.eFlags;
    ::memset( &client->ps, 0, sizeof( client->ps ) );
    ::memset( &client->pmext, 0, sizeof( client->pmext ) );
    client->ps.eFlags = flags;
    
    // locate ent at a spawn point
    
    ClientSpawn( ent, NULL, NULL, NULL );
    
    trap_SendServerCommand( -1, va( "print \"%s" S_COLOR_WHITE " entered the game\n\"", client->pers.netname ) );
    
    // name can change between ClientConnect() and ClientBegin()
    adminLocal.AdminNamelogUpdate( client, false );
    
    // request the clients PTR code
    trap_SendServerCommand( ent - g_entities, "ptrcrequest" );
    
    // ask for identification
    admin = client->pers.admin;
    
    LogPrintf( "idGameLocal::ClientBegin: %i\n", clientNum );
    
    if( trap_Database_RunQuery( va( "INSERT INTO clientbegin (playerid) VALUES (%d)", client->pers.mysqlid ) ) == true )
    {
        UTF8 data[255];
        
        trap_Database_FinishQuery();
        trap_Database_RunQuery( "SELECT id FROM clientbegin ORDER BY id DESC LIMIT 1" );
        trap_Database_FetchRow();
        trap_Database_FetchFieldByName( "id", data, sizeof( data ) );
        trap_Database_RunQuery( va( "INSERT into mainlog (idaction,typeaction,date) VALUES (%d,\"%d\",NOW())", atoi( data ), 2 ) );
        trap_Database_FinishQuery();
    }
    
    // log to demo
    DemoCommand( DC_CLIENT_SET, va( "%d \\name\\%s\\team\\%d", clientNum,
                                    client->pers.netname, client->pers.teamSelection ) );
                                    
    // count current clients and rank for scoreboard
    CalculateRanks( );
}

/*
===========
idGameLocal::BotBegin

called when a client has finished connecting, and is ready
to be placed into the level.  This will happen every level load,
and on transition between teams, but doesn't happen on respawns
============
*/
void idGameLocal::BotBegin( S32 clientNum )
{
    gentity_t* ent;
    gclient_t* client;
    S32       flags;
    UTF8      userinfo[ MAX_INFO_STRING ];
    
    trap_GetUserinfo( clientNum, userinfo, sizeof( userinfo ) );
    
    ent = g_entities + clientNum;
    
    client = level.clients + clientNum;
    
    if( ent->r.linked )
        trap_UnlinkEntity( ent );
        
    InitGentity( ent );
    ent->touch = 0;
    ent->pain = 0;
    ent->client = client;
    
    client->pers.connected = CON_CONNECTED;
    client->pers.enterTime = level.time;
    client->pers.classSelection = PCL_NONE;
    
    // save eflags around this, because changing teams will
    // cause this to happen with a valid entity, and we
    // want to make sure the teleport bit is set right
    // so the viewpoint doesn't interpolate through the
    // world to the new position
    flags = client->ps.eFlags;
    ::memset( &client->ps, 0, sizeof( client->ps ) );
    ::memset( &client->pmext, 0, sizeof( client->pmext ) );
    client->ps.eFlags = flags;
    
    // locate ent at a spawn point
    
    ClientSpawn( ent, NULL, NULL, NULL );
    
    // name can change between ClientConnect() and ClientBegin()
    adminLocal.AdminNamelogUpdate( client, false );
    
    // request the clients PTR code
    trap_SendServerCommand( ent - g_entities, "ptrcrequest" );
    
    LogPrintf( "idGameLocal::ClientBegin: %i\n", clientNum );
    
    // log to demo
    trap_GetConfigstring( CS_PLAYERS + clientNum, userinfo, sizeof( userinfo ) );
    DemoCommand( DC_CLIENT_SET, va( "%d %s", clientNum, userinfo ) );
    
    // count current clients and rank for scoreboard
    CalculateRanks( );
}

/*
===========
idGameLocal::ClientSpawn

Called every time a client is placed fresh in the world:
after the first ClientBegin, and after each respawn
Initializes all non-persistant parts of playerState
============
*/
void idGameLocal::ClientSpawn( gentity_t* ent, gentity_t* spawn, vec3_t origin, vec3_t angles )
{
    S32                 index;
    vec3_t              spawn_origin, spawn_angles;
    gclient_t*           client;
    S32                 i;
    clientPersistant_t  saved;
    clientSession_t     savedSess;
    S32                 persistant[ MAX_PERSISTANT ];
    gentity_t*           spawnPoint = NULL;
    S32                 flags;
    S32                 savedPing;
    S32                 teamLocal;
    S32                 eventSequence;
    UTF8                userinfo[ MAX_INFO_STRING ];
    vec3_t              up = { 0.0f, 0.0f, 1.0f };
    S32                 maxAmmo, maxClips;
    weapon_t            weapon;
    
    
    index = ent - g_entities;
    client = ent->client;
    
    teamLocal = client->pers.teamSelection;
    
    //if client is dead and following teammate, stop following before spawning
    if( client->sess.spectatorClient != -1 )
    {
        client->sess.spectatorClient = -1;
        client->sess.spectatorState = SPECTATOR_FREE;
    }
    
    // only start client if chosen a class and joined a team
    if( client->pers.classSelection == PCL_NONE && teamLocal == TEAM_NONE )
        client->sess.spectatorState = SPECTATOR_FREE;
    else if( client->pers.classSelection == PCL_NONE )
        client->sess.spectatorState = SPECTATOR_LOCKED;
        
    if( origin != NULL )
        VectorCopy( origin, spawn_origin );
        
    if( angles != NULL )
        VectorCopy( angles, spawn_angles );
        
    // find a spawn point
    // do it before setting health back up, so farthest
    // ranging doesn't count this client
    if( client->sess.spectatorState != SPECTATOR_NOT )
    {
        if( teamLocal == TEAM_NONE )
            spawnPoint = SelectSpectatorSpawnPoint( spawn_origin, spawn_angles );
        else if( teamLocal == TEAM_ALIENS )
            spawnPoint = SelectAlienLockSpawnPoint( spawn_origin, spawn_angles );
        else if( teamLocal == TEAM_HUMANS )
            spawnPoint = SelectHumanLockSpawnPoint( spawn_origin, spawn_angles );
    }
    else
    {
        if( spawn == NULL )
        {
            Error( "idGameLocal::ClientSpawn: spawn is NULL\n" );
            return;
        }
        
        spawnPoint = spawn;
        
        if( ent != spawn )
        {
            //start spawn animation on spawnPoint
            SetBuildableAnim( spawnPoint, BANIM_SPAWN1, true );
            
            if( spawnPoint->buildableTeam == TEAM_ALIENS )
                spawnPoint->clientSpawnTime = ALIEN_SPAWN_REPEAT_TIME;
            else if( spawnPoint->buildableTeam == TEAM_HUMANS )
                spawnPoint->clientSpawnTime = HUMAN_SPAWN_REPEAT_TIME;
        }
    }
    
    // toggle the teleport bit so the client knows to not lerp
    flags = ent->client->ps.eFlags & ( EF_TELEPORT_BIT | EF_VOTED | EF_TEAMVOTED );
    flags ^= EF_TELEPORT_BIT;
    UnlaggedClear( ent );
    
    // clear everything but the persistant data
    
    saved = client->pers;
    savedSess = client->sess;
    savedPing = client->ps.ping;
    
    for( i = 0; i < MAX_PERSISTANT; i++ )
        persistant[ i ] = client->ps.persistant[ i ];
        
    eventSequence = client->ps.eventSequence;
    ::memset( client, 0, sizeof( *client ) );
    
    client->pers = saved;
    client->sess = savedSess;
    client->ps.ping = savedPing;
    client->lastkilled_client = -1;
    
    for( i = 0; i < MAX_PERSISTANT; i++ )
        client->ps.persistant[ i ] = persistant[ i ];
        
    client->ps.eventSequence = eventSequence;
    
    // increment the spawncount so the client will detect the respawn
    client->ps.persistant[ PERS_SPAWN_COUNT ]++;
    client->ps.persistant[ PERS_SPECSTATE ] = client->sess.spectatorState;
    
    client->airOutTime = level.time + 12000;
    
    trap_GetUserinfo( index, userinfo, sizeof( userinfo ) );
    client->ps.eFlags = flags;
    
    //Com_Printf( "ent->client->pers->pclass = %i\n", ent->client->pers.classSelection );
    
    ent->s.groundEntityNum = ENTITYNUM_NONE;
    ent->client = &level.clients[ index ];
    ent->takedamage = true;
    ent->inuse = true;
    ent->classname = "player";
    ent->r.contents = CONTENTS_BODY;
    ent->clipmask = MASK_PLAYERSOLID;
    ent->die = player_die;
    ent->waterlevel = 0;
    ent->watertype = 0;
    ent->flags = 0;
    
    // calculate each client's acceleration
    ent->evaluateAcceleration = true;
    
    client->ps.stats[ STAT_MISC ] = 0;
    
    client->ps.eFlags = flags;
    client->ps.clientNum = index;
    
    bggame->ClassBoundingBox( ent->client->pers.classSelection, ent->r.mins, ent->r.maxs, NULL, NULL, NULL );
    
    if( client->sess.spectatorState == SPECTATOR_NOT )
        client->pers.maxHealth = client->ps.stats[ STAT_MAX_HEALTH ] =
                                     bggame->Class( ent->client->pers.classSelection )->health;
    else
        client->pers.maxHealth = client->ps.stats[ STAT_MAX_HEALTH ] = 100;
        
    // clear entity values
    if( ent->client->pers.classSelection == PCL_HUMAN )
    {
        bggame->AddUpgradeToInventory( UP_MEDKIT, client->ps.stats );
        weapon = client->pers.humanItemSelection;
    }
    else if( client->sess.spectatorState == SPECTATOR_NOT )
        weapon = bggame->Class( ent->client->pers.classSelection )->startWeapon;
    else
        weapon = WP_NONE;
        
    maxAmmo = bggame->Weapon( weapon )->maxAmmo;
    maxClips = bggame->Weapon( weapon )->maxClips;
    client->ps.stats[ STAT_WEAPON ] = weapon;
    client->ps.ammo = maxAmmo;
    client->ps.clips = maxClips;
    
    client->ps.persistant[ PERS_NEWWEAPON ] = 0;
    
    ent->client->ps.stats[ STAT_CLASS ] = ent->client->pers.classSelection;
    ent->client->ps.stats[ STAT_TEAM ] = ent->client->pers.teamSelection;
    
    ent->client->ps.stats[ STAT_BUILDABLE ] = BA_NONE;
    ent->client->ps.stats[ STAT_STATE ] = 0;
    VectorSet( ent->client->ps.grapplePoint, 0.0f, 0.0f, 1.0f );
    
    // health will count down towards max_health
    ent->health = client->ps.stats[ STAT_HEALTH ] = client->ps.stats[ STAT_MAX_HEALTH ]; //* 1.25;
    
    //if evolving scale health
    if( ent == spawn )
    {
        ent->health *= ent->client->pers.evolveHealthFraction;
        client->ps.stats[ STAT_HEALTH ] *= ent->client->pers.evolveHealthFraction;
    }
    
    //clear the credits array
    for( i = 0; i < MAX_CLIENTS; i++ )
        ent->credits[ i ] = 0;
        
    client->ps.stats[ STAT_STAMINA ] = MAX_STAMINA;
    
    SetOrigin( ent, spawn_origin );
    VectorCopy( spawn_origin, client->ps.origin );
    
#define UP_VEL  150.0f
#define F_VEL   50.0f
    
    //give aliens some spawn velocity
    if( client->sess.spectatorState == SPECTATOR_NOT &&
            client->ps.stats[ STAT_TEAM ] == TEAM_ALIENS )
    {
        if( ent == spawn )
        {
            //evolution particle system
            AddPredictableEvent( ent, EV_ALIEN_EVOLVE, DirToByte( up ) );
        }
        else
        {
            spawn_angles[ YAW ] += 180.0f;
            AngleNormalize360( spawn_angles[ YAW ] );
            
            if( spawnPoint->s.origin2[ 2 ] > 0.0f )
            {
                vec3_t  forward, dir;
                
                AngleVectors( spawn_angles, forward, NULL, NULL );
                VectorScale( forward, F_VEL, forward );
                VectorAdd( spawnPoint->s.origin2, forward, dir );
                VectorNormalize( dir );
                
                VectorScale( dir, UP_VEL, client->ps.velocity );
            }
            
            AddPredictableEvent( ent, EV_PLAYER_RESPAWN, 0 );
        }
    }
    else if( client->sess.spectatorState == SPECTATOR_NOT &&
             client->ps.stats[ STAT_TEAM ] == TEAM_HUMANS )
    {
        spawn_angles[ YAW ] += 180.0f;
        AngleNormalize360( spawn_angles[ YAW ] );
    }
    
    // the respawned flag will be cleared after the attack and jump keys come up
    client->ps.pm_flags |= PMF_RESPAWNED;
    
    trap_GetUsercmd( client - level.clients, &ent->client->pers.cmd );
    SetClientViewAngle( ent, spawn_angles );
    
    if( client->sess.spectatorState == SPECTATOR_NOT )
    {
        trap_LinkEntity( ent );
        
        // force the base weapon up
        client->ps.weapon = WP_NONE;
        client->ps.weaponstate = WEAPON_READY;
    }
    
    // don't allow full run speed for a bit
    client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
    client->ps.pm_time = 100;
    
    client->respawnTime = level.time;
    client->lastKillTime = level.time;
    ent->nextRegenTime = level.time;
    
    client->inactivityTime = level.time + g_inactivity.integer * 1000;
    client->latched_buttons = 0;
    
    // set default animations
    client->ps.torsoAnim = TORSO_STAND;
    client->ps.legsAnim = LEGS_IDLE;
    
    if( level.intermissiontime )
        MoveClientToIntermission( ent );
    else
    {
        // fire the targets of the spawn point
        if( !spawn )
            UseTargets( spawnPoint, ent );
            
        // select the highest weapon number available, after any
        // spawn given items have fired
        client->ps.weapon = 1;
        
        for( i = WP_NUM_WEAPONS - 1; i > 0 ; i-- )
        {
            if( bggame->InventoryContainsWeapon( i, client->ps.stats ) )
            {
                client->ps.weapon = i;
                break;
            }
        }
    }
    
    // run a client frame to drop exactly to the floor,
    // initialize animations and other things
    client->ps.commandTime = level.time - 100;
    ent->client->pers.cmd.serverTime = level.time;
    gameLocal.ClientThink( ent - g_entities );
    
    // positively link the client, even if the command times are weird
    if( client->sess.spectatorState == SPECTATOR_NOT )
    {
        bggame->PlayerStateToEntityState( &client->ps, &ent->s, true );
        VectorCopy( ent->client->ps.origin, ent->r.currentOrigin );
        trap_LinkEntity( ent );
    }
    
    // must do this here so the number of active clients is calculated
    CalculateRanks( );
    
    // run the presend to set anything else
    ClientEndFrame( ent );
    
    // clear entity state values
    bggame->PlayerStateToEntityState( &client->ps, &ent->s, true );
}


/*
===========
idGameLocal::ClientDisconnect

Called when a player drops from the server.
Will not be called between levels.

This should NOT be called directly by any game logic,
call trap_DropClient(), which will call this and do
server system housekeeping.
============
*/
void idGameLocal::ClientDisconnect( S32 clientNum )
{
    gentity_t* ent;
    gentity_t* tent;
    S32       i;
    
    // cleanup if we are kicking a bot that hasn't spawned yet
    RemoveQueuedBotBegin( clientNum );
    
    ent = g_entities + clientNum;
    
    if( !ent->client )
        return;
        
    adminLocal.AdminNamelogUpdate( ent->client, true );
    LeaveTeam( ent );
    Vote( ent, false );
    
    // stop any following clients
    for( i = 0; i < level.maxclients; i++ )
    {
        // remove any /ignore settings for this clientNum
        bggame->ClientListRemove( &level.clients[ i ].sess.ignoreList, clientNum );
    }
    
    // send effect if they were completely connected
    if( ent->client->pers.connected == CON_CONNECTED &&
            ent->client->sess.spectatorState == SPECTATOR_NOT )
    {
        tent = TempEntity( ent->client->ps.origin, EV_PLAYER_TELEPORT_OUT );
        tent->s.clientNum = ent->s.clientNum;
    }
    
    if( ent->client->pers.connection )
        ent->client->pers.connection->clientNum = -1;
        
    LogPrintf( "idGameLocal::ClientDisconnect: %i [%s] (%s) \"%s^7\"\n", clientNum,
               ent->client->pers.ip, ent->client->pers.guid, ent->client->pers.netname );
               
    if( trap_Database_RunQuery( va( "INSERT INTO disconnect (playerid,ip,name) VALUES (%d,\"%s\",\"%s\")",
                                    ent->client->pers.mysqlid, ent->client->pers.ip, ent->client->pers.netname ) ) == true )
    {
        UTF8 data[255];
        trap_Database_FinishQuery();
        trap_Database_RunQuery( "SELECT id FROM disconnect ORDER BY id DESC LIMIT 1" );
        trap_Database_FetchRow();
        trap_Database_FetchFieldByName( "id", data, sizeof( data ) );
        trap_Database_RunQuery( va( "INSERT into mainlog (idaction,typeaction,date) VALUES (%d,\"%d\",NOW())", atoi( data ), 6 ) );
        trap_Database_FinishQuery();
    }
    
    trap_UnlinkEntity( ent );
    ent->s.modelindex = 0;
    ent->inuse = false;
    ent->classname = "disconnected";
    ent->client->pers.connected = CON_DISCONNECTED;
    ent->client->sess.spectatorState = ( spectatorState_t )( ent->client->ps.persistant[ PERS_SPECSTATE ] = SPECTATOR_NOT );
    
    trap_SetConfigstring( CS_PLAYERS + clientNum, "" );
    
    DemoCommand( DC_CLIENT_REMOVE, va( "%d", clientNum ) );
    
    CalculateRanks( );
    
    if( ent->r.svFlags & SVF_BOT )
    {
        botLocal.BotAIShutdownClient( clientNum, false );
    }
}
