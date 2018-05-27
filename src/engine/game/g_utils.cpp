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
// File name:   g_utils.cpp
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description: misc utility functions for game module
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#include <game/g_local.h>

typedef struct
{
    UTF8 oldShader[ MAX_QPATH ];
    UTF8 newShader[ MAX_QPATH ];
    F32 timeOffset;
} shaderRemap_t;

#define MAX_SHADER_REMAPS 128

S32 remapCount = 0;
shaderRemap_t remappedShaders[ MAX_SHADER_REMAPS ];

/*
================
idGameLocal::AddRemap
================
*/
void idGameLocal::AddRemap( StringEntry oldShader, StringEntry newShader, F32 timeOffset )
{
    S32 i;
    
    for( i = 0; i < remapCount; i++ )
    {
        if( Q_stricmp( oldShader, remappedShaders[ i ].oldShader ) == 0 )
        {
            // found it, just update this one
            strcpy( remappedShaders[ i ].newShader, newShader );
            remappedShaders[ i ].timeOffset = timeOffset;
            return;
        }
    }
    
    if( remapCount < MAX_SHADER_REMAPS )
    {
        strcpy( remappedShaders[ remapCount ].newShader, newShader );
        strcpy( remappedShaders[ remapCount ].oldShader, oldShader );
        remappedShaders[ remapCount ].timeOffset = timeOffset;
        remapCount++;
    }
}

/*
================
idGameLocal::BuildShaderStateConfig
================
*/
StringEntry idGameLocal::BuildShaderStateConfig( void )
{
    static UTF8 buff[ MAX_STRING_CHARS * 4 ];
    UTF8 out[( MAX_QPATH * 2 ) + 5 ];
    S32 i;
    
    ::memset( buff, 0, MAX_STRING_CHARS );
    
    for( i = 0; i < remapCount; i++ )
    {
        Com_sprintf( out, ( MAX_QPATH * 2 ) + 5, "%s=%s:%5.2f@", remappedShaders[ i ].oldShader, remappedShaders[ i ].newShader, remappedShaders[ i ].timeOffset );
        Q_strcat( buff, sizeof( buff ), out );
    }
    return buff;
}

/*
=========================================================================
Model / Sound configstring indexes
=========================================================================
*/

/*
================
idGameLocal::FindConfigstringIndex
================
*/
S32 idGameLocal::FindConfigstringIndex( UTF8* name, S32 start, S32 max, bool create )
{
    S32 i;
    UTF8 s[ MAX_STRING_CHARS ];
    
    if( !name || !name[0] )
    {
        return 0;
    }
    
    for( i = 1; i < max; i++ )
    {
        trap_GetConfigstring( start + i, s, sizeof( s ) );
        if( !s[0] )
        {
            break;
        }
        
        if( !strcmp( s, name ) )
        {
            return i;
        }
    }
    
    if( !create )
    {
        return 0;
    }
    
    if( i == max )
    {
        Error( "idGameLocal::FindConfigstringIndex: overflow" );
    }
    
    trap_SetConfigstring( start + i, name );
    
    return i;
}

/*
===============
idGameLocal::ParticleSystemIndex
===============
*/
S32 idGameLocal::ParticleSystemIndex( UTF8* name )
{
    return FindConfigstringIndex( name, CS_PARTICLE_SYSTEMS, MAX_GAME_PARTICLE_SYSTEMS, true );
}

/*
===============
idGameLocal::ShaderIndex
===============
*/
S32 idGameLocal::ShaderIndex( UTF8* name )
{
    return FindConfigstringIndex( name, CS_SHADERS, MAX_GAME_SHADERS, true );
}

/*
===============
idGameLocal::ModelIndex
===============
*/
S32 idGameLocal::ModelIndex( UTF8* name )
{
    return FindConfigstringIndex( name, CS_MODELS, MAX_MODELS, true );
}

/*
===============
idGameLocal::SoundIndex
===============
*/
S32 idGameLocal::SoundIndex( UTF8* name )
{
    return FindConfigstringIndex( name, CS_SOUNDS, MAX_SOUNDS, true );
}

/*
================
idGameLocal::TeamCommand

Broadcasts a command to only a specific team
================
*/
void idGameLocal::TeamCommand( team_t team, UTF8* cmd )
{
    S32   i;
    
    for( i = 0 ; i < level.maxclients ; i++ )
    {
        if( level.clients[ i ].pers.connected == CON_CONNECTED )
        {
            if( level.clients[ i ].pers.teamSelection == team ||
                    ( level.clients[ i ].pers.teamSelection == TEAM_NONE && adminLocal.AdminPermission( &g_entities[ i ], ADMF_SPEC_ALLCHAT ) ) ||
                    ( level.clients[ i ].pers.teamSelection == TEAM_NONE && level.clients[ i ].sess.spectatorState == SPECTATOR_FOLLOW &&
                      level.clients[ i ].sess.spectatorClient >= 0 && level.clients[ level.clients[ i ].sess.spectatorClient ].pers.teamSelection == team ) )
                trap_SendServerCommand( i, cmd );
        }
    }
    
    DemoCommand( DC_SERVER_COMMAND, cmd );
}


/*
=============
idGameLocal::Find

Searches all active entities for the next one that holds
the matching string at fieldofs (use the FOFS() macro) in the structure.

Searches beginning at the entity after from, or the beginning if NULL
NULL will be returned if the end of the list is reached.

=============
*/
gentity_t* idGameLocal::Find( gentity_t* from, S32 fieldofs, StringEntry match )
{
    UTF8*  s;
    
    if( !from )
    {
        from = g_entities;
    }
    else
    {
        from++;
    }
    
    for( ; from < &g_entities[ level.num_entities ]; from++ )
    {
        if( !from->inuse )
        {
            continue;
        }
        s = *( UTF8** )( ( U8* )from + fieldofs );
        
        if( !s )
        {
            continue;
        }
        
        if( !Q_stricmp( s, match ) )
        {
            return from;
        }
    }
    
    return NULL;
}


/*
=============
idGameLocal::PickTarget

Selects a random entity from among the targets
=============
*/
#define MAXCHOICES  32

gentity_t* idGameLocal::PickTarget( UTF8* targetname )
{
    gentity_t* ent = NULL;
    S32 num_choices = 0;
    gentity_t* choice[ MAXCHOICES ];
    
    if( !targetname )
    {
        Printf( "idGameLocal::PickTarget: Called with NULL targetname\n" );
        return NULL;
    }
    
    while( 1 )
    {
        ent = Find( ent, FOFS( targetname ), targetname );
        
        if( !ent )
        {
            break;
        }
        
        choice[ num_choices++ ] = ent;
        
        if( num_choices == MAXCHOICES )
        {
            break;
        }
    }
    
    if( !num_choices )
    {
        Printf( "idGameLocal::PickTarget: Target %s not found\n", targetname );
        return NULL;
    }
    
    return choice[ rand( ) % num_choices ];
}


/*
==============================
idGameLocal::UseTargets

"activator" should be set to the entity that initiated the firing.

Search for (string)targetname in all entities that
match (string)self.target and call their .use function
==============================
*/
void idGameLocal::UseTargets( gentity_t* ent, gentity_t* activator )
{
    gentity_t* t;
    
    if( !ent )
    {
        return;
    }
    
    if( ent->targetShaderName && ent->targetShaderNewName )
    {
        F32 f = level.time * 0.001;
        AddRemap( ent->targetShaderName, ent->targetShaderNewName, f );
        trap_SetConfigstring( CS_SHADERSTATE, BuildShaderStateConfig( ) );
    }
    
    if( !ent->target )
    {
        return;
    }
    
    t = NULL;
    while( ( t = Find( t, FOFS( targetname ), ent->target ) ) != NULL )
    {
        if( t == ent )
        {
            Printf( "idGameLocal::UseTargets: Entity used itself.\n" );
        }
        else
        {
            if( t->use )
            {
                t->use( t, ent, activator );
            }
        }
        
        if( !ent->inuse )
        {
            Printf( "idGameLocal::UseTargets: Entity was removed while using targets\n" );
            return;
        }
    }
}

/*
=============
VectorToString

This is just a convenience function
for printing vectors
=============
*/
UTF8* idGameLocal::vtos( const vec3_t v )
{
    static S32 index;
    static UTF8 str[ 8 ][ 32 ];
    UTF8* s;
    
    // use an array so that multiple vtos won't collide
    s = str[ index ];
    index = ( index + 1 ) & 7;
    
    Com_sprintf( s, 32, "(%i %i %i)", ( S32 )v[ 0 ], ( S32 )v[ 1 ], ( S32 )v[ 2 ] );
    
    return s;
}


/*
===============
idGameLocal::SetMovedir

The editor only specifies a single value for angles (yaw),
but we have special constants to generate an up or down direction.
Angles will be cleared, because it is being used to represent a direction
instead of an orientation.
===============
*/
void idGameLocal::SetMovedir( vec3_t angles, vec3_t movedir )
{
    static vec3_t VEC_UP = { 0, -1, 0 };
    static vec3_t MOVEDIR_UP = { 0, 0, 1 };
    static vec3_t VEC_DOWN = { 0, -2, 0 };
    static vec3_t MOVEDIR_DOWN = { 0, 0, -1 };
    
    if( VectorCompare( angles, VEC_UP ) )
    {
        VectorCopy( MOVEDIR_UP, movedir );
    }
    else if( VectorCompare( angles, VEC_DOWN ) )
    {
        VectorCopy( MOVEDIR_DOWN, movedir );
    }
    else
    {
        AngleVectors( angles, movedir, NULL, NULL );
    }
    
    VectorClear( angles );
}

/*
===============
idGameLocal::InitGentity
===============
*/
void idGameLocal::InitGentity( gentity_t* e )
{
    e->inuse = true;
    e->classname = "noclass";
    e->s.number = e - g_entities;
    e->r.ownerNum = ENTITYNUM_NONE;
}

/*
=================
idGameLocal::Spawn

Either finds a free entity, or allocates a new one.

The slots from 0 to MAX_CLIENTS-1 are always reserved for clients, and will
never be used by anything else.

Try to avoid reusing an entity that was recently freed, because it
can cause the client to think the entity morphed into something else
instead of being removed and recreated, which can cause interpolated
angles and bad trails.
=================
*/
gentity_t* idGameLocal::Spawn( void )
{
    S32 i, force;
    gentity_t* e;
    
    e = NULL; // shut up warning
    i = 0;    // shut up warning
    
    for( force = 0; force < 2; force++ )
    {
        // if we go through all entities and can't find one to free,
        // override the normal minimum times before use
        e = &g_entities[ MAX_CLIENTS ];
        
        for( i = MAX_CLIENTS; i < level.num_entities; i++, e++ )
        {
            if( e->inuse )
            {
                continue;
            }
            
            // the first couple seconds of server time can involve a lot of
            // freeing and allocating, so relax the replacement policy
            if( !force && e->freetime > level.startTime + 2000 && level.time - e->freetime < 1000 )
            {
                continue;
            }
            
            // reuse this slot
            InitGentity( e );
            return e;
        }
        
        if( i != MAX_GENTITIES )
        {
            break;
        }
    }
    
    if( i == ENTITYNUM_MAX_NORMAL )
    {
        for( i = 0; i < MAX_GENTITIES; i++ )
        {
            Printf( "%4i: %s\n", i, g_entities[i].classname );
        }
        
        Error( "idGameLocal::Spawn: no free entities" );
    }
    
    // open up a new slot
    level.num_entities++;
    
    // let the server system know that there are more entities
    trap_LocateGameData( level.gentities, level.num_entities, sizeof( gentity_t ),
                         &level.clients[ 0 ].ps, sizeof( level.clients[ 0 ] ) );
                         
    InitGentity( e );
    return e;
}

/*
=================
idGameLocal::FreeEntity

Marks the entity as free
=================
*/
void idGameLocal::FreeEntity( gentity_t* ent )
{
    trap_UnlinkEntity( ent );   // unlink from world
    
    if( ent->neverFree )
    {
        return;
    }
    
    ::memset( ent, 0, sizeof( *ent ) );
    ent->classname = "freent";
    ent->freetime = level.time;
    ent->inuse = false;
}

/*
=================
idGameLocal::TempEntity

Spawns an event entity that will be auto-removed
The origin will be snapped to save net bandwidth, so care
must be taken if the origin is right on a surface (snap towards start vector first)
=================
*/
gentity_t* idGameLocal::TempEntity( vec3_t origin, S32 event )
{
    gentity_t* e;
    vec3_t    snapped;
    
    e = Spawn( );
    e->s.eType = ( entityType_t )( ET_EVENTS + event );
    
    e->classname = "tempEntity";
    e->eventTime = level.time;
    e->freeAfterEvent = true;
    
    VectorCopy( origin, snapped );
    SnapVector( snapped );    // save network bandwidth
    SetOrigin( e, snapped );
    
    // find cluster for PVS
    trap_LinkEntity( e );
    
    return e;
}

/*
==============================================================================
Kill box
==============================================================================
*/

/*
=================
G_KillBox

Kills all entities that would touch the proposed new positioning
of ent.  Ent should be unlinked before calling this!
=================
*/
void idGameLocal::KillBox( gentity_t* ent )
{
    S32       i, num;
    S32       touch[ MAX_GENTITIES ];
    gentity_t* hit;
    vec3_t    mins, maxs;
    
    VectorAdd( ent->client->ps.origin, ent->r.mins, mins );
    VectorAdd( ent->client->ps.origin, ent->r.maxs, maxs );
    num = trap_EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );
    
    for( i = 0; i < num; i++ )
    {
        hit = &g_entities[ touch[ i ] ];
        
        if( !hit->client )
        {
            continue;
        }
        
        // impossible to telefrag self
        if( ent == hit )
        {
            continue;
        }
        
        // nail it
        Damage( hit, ent, ent, NULL, NULL, 100000, DAMAGE_NO_PROTECTION, MOD_TELEFRAG );
    }
    
}

/*
===============
idGameLocal::AddPredictableEvent

Use for non-pmove events that would also be predicted on the
client side: jumppads and item pickups
Adds an event+parm and twiddles the event counter
===============
*/
void idGameLocal::AddPredictableEvent( gentity_t* ent, S32 event, S32 eventParm )
{
    if( !ent->client )
    {
        return;
    }
    
    bggame->AddPredictableEventToPlayerstate( event, eventParm, &ent->client->ps );
}


/*
===============
idGameLocal::AddEvent

Adds an event+parm and twiddles the event counter
===============
*/
void idGameLocal::AddEvent( gentity_t* ent, S32 event, S32 eventParm )
{
    S32 bits;
    
    if( !event )
    {
        Printf( "idGameLocal::AddEvent: zero event added for entity %i\n", ent->s.number );
        return;
    }
    
    // eventParm is converted to uint8_t (0 - 255) in msg.c
    if( eventParm & ~0xFF )
    {
        Printf( S_COLOR_YELLOW "WARNING: G_AddEvent( %s ) has eventParm %d, "
                "which will overflow\n", bggame->EventName( event ), eventParm );
    }
    
    // clients need to add the event in playerState_t instead of entityState_t
    if( ent->client )
    {
        bits = ent->client->ps.externalEvent & EV_EVENT_BITS;
        bits = ( bits + EV_EVENT_BIT1 ) & EV_EVENT_BITS;
        ent->client->ps.externalEvent = event | bits;
        ent->client->ps.externalEventParm = eventParm;
        ent->client->ps.externalEventTime = level.time;
    }
    else
    {
        bits = ent->s.event & EV_EVENT_BITS;
        bits = ( bits + EV_EVENT_BIT1 ) & EV_EVENT_BITS;
        ent->s.event = event | bits;
        ent->s.eventParm = eventParm;
    }
    
    ent->eventTime = level.time;
}

/*
===============
idGameLocal::BroadcastEvent

Sends an event to every client
===============
*/
void idGameLocal::BroadcastEvent( S32 event, S32 eventParm )
{
    gentity_t* ent;
    
    ent = TempEntity( vec3_origin, event );
    ent->s.eventParm = eventParm;
    ent->r.svFlags = SVF_BROADCAST; // send to everyone
}

/*
=============
idGameLocal::Sound
=============
*/
void idGameLocal::Sound( gentity_t* ent, S32 channel, S32 soundIndex )
{
    gentity_t* te;
    
    te = TempEntity( ent->r.currentOrigin, EV_GENERAL_SOUND );
    te->s.eventParm = soundIndex;
}

/*
================
idGameLocal::SetOrigin

Sets the pos trajectory for a fixed position
================
*/
void idGameLocal::SetOrigin( gentity_t* ent, vec3_t origin )
{
    VectorCopy( origin, ent->s.pos.trBase );
    ent->s.pos.trType = TR_STATIONARY;
    ent->s.pos.trTime = 0;
    ent->s.pos.trDuration = 0;
    VectorClear( ent->s.pos.trDelta );
    
    VectorCopy( origin, ent->r.currentOrigin );
    VectorCopy( origin, ent->s.origin );
}

/*
===============
idGameLocal::Visible

Test for a LOS between two entities
===============
*/
bool idGameLocal::Visible( gentity_t* ent1, gentity_t* ent2, S32 contents )
{
    trace_t trace;
    
    trap_Trace( &trace, ent1->s.pos.trBase, NULL, NULL, ent2->s.pos.trBase, ent1->s.number, contents );
    return trace.fraction >= 1.0f || trace.entityNum == ent2 - g_entities;
}

/*
===============
idGameLocal::ClosestEnt

Test a list of entities for the closest to a particular point
===============
*/
gentity_t* idGameLocal::ClosestEnt( vec3_t origin, gentity_t** entities, S32 numEntities )
{
    S32 i;
    F32 nd, d = 1000000.0f;
    gentity_t* closestEnt = NULL;
    
    for( i = 0; i < numEntities; i++ )
    {
        gentity_t* ent = entities[ i ];
        
        nd = DistanceSquared( origin, ent->s.origin );
        
        if( i == 0 || nd < d )
        {
            d = nd;
            closestEnt = ent;
        }
    }
    
    return closestEnt;
}

/*
===============
idGameLocal::TriggerMenu

Trigger a menu on some client
===============
*/
void idGameLocal::TriggerMenu( S32 clientNum, dynMenu_t menu )
{
    UTF8 buffer[ 32 ];
    
    Com_sprintf( buffer, sizeof( buffer ), "servermenu %d", menu );
    trap_SendServerCommand( clientNum, buffer );
}

/*
===============
idGameLocal::TriggerMenu2

Trigger a menu on some client and passes an argument
===============
*/
void idGameLocal::TriggerMenu2( S32 clientNum, dynMenu_t menu, S32 arg )
{
    UTF8 buffer[ 64 ];
    
    Com_sprintf( buffer, sizeof( buffer ), "servermenu %d %d", menu, arg );
    trap_SendServerCommand( clientNum, buffer );
}

/*
===============
idGameLocal::CloseMenus

Close all open menus on some client
===============
*/
void idGameLocal::CloseMenus( S32 clientNum )
{
    UTF8 buffer[ 32 ];
    
    Com_sprintf( buffer, 32, "serverclosemenus" );
    trap_SendServerCommand( clientNum, buffer );
}

/*
================
idGameLocal::DebugLine

debug polygons only work when running a local game
with r_debugSurface set to 2
================
*/
S32 idGameLocal::DebugLine( vec3_t start, vec3_t end, S32 color )
{
    vec3_t points[ 4 ], dir, cross, up = { 0, 0, 1 };
    F32 dot;
    
    VectorCopy( start, points[ 0 ] );
    VectorCopy( start, points[ 1 ] );
    //points[1][2] -= 2;
    VectorCopy( end, points[ 2 ] );
    //points[2][2] -= 2;
    VectorCopy( end, points[ 3 ] );
    
    VectorSubtract( end, start, dir );
    VectorNormalize( dir );
    dot = DotProduct( dir, up );
    
    if( dot > 0.99 || dot < -0.99 )
    {
        VectorSet( cross, 1, 0, 0 );
    }
    else
    {
        CrossProduct( dir, up, cross );
    }
    
    VectorNormalize( cross );
    
    VectorMA( points[ 0 ], 2, cross, points[ 0 ] );
    VectorMA( points[ 1 ], -2, cross, points[ 1 ] );
    VectorMA( points[ 2 ], -2, cross, points[ 2 ] );
    VectorMA( points[ 3 ], 2, cross, points[ 3 ] );
    
    return trap_DebugPolygonCreate( color, 4, points );
}

/*
===============
idGameLocal::DeleteDebugLines
===============
*/
void idGameLocal::DebugLineDouble( vec3_t start, vec3_t end, S32 color )
{
    vec3_t points[4], morepoints[4], dir, cross, up = {0, 0, 1};
    F32 dot;
    
    VectorCopy( start, points[0] );
    VectorCopy( start, points[1] );
    //points[1][2] -= 2;
    VectorCopy( end, points[2] );
    //points[2][2] -= 2;
    VectorCopy( end, points[3] );
    
    VectorSubtract( end, start, dir );
    VectorNormalize( dir );
    dot = DotProduct( dir, up );
    
    if( dot > 0.99 || dot < -0.99 )
    {
        VectorSet( cross, 1, 0, 0 );
    }
    else
    {
        CrossProduct( dir, up, cross );
    }
    
    VectorNormalize( cross );
    
    VectorMA( points[0], 2, up, morepoints[0] );
    VectorMA( points[1], -2, up, morepoints[1] );
    VectorMA( points[2], -2, up, morepoints[2] );
    VectorMA( points[3], 2, up, morepoints[3] );
    
    VectorMA( points[0], 2, cross, points[0] );
    VectorMA( points[1], -2, cross, points[1] );
    VectorMA( points[2], -2, cross, points[2] );
    VectorMA( points[3], 2, cross, points[3] );
    
    trap_DebugPolygonCreate( color, 4, points );
    trap_DebugPolygonCreate( color, 4, morepoints );
}

/*
===============
idGameLocal::DeleteDebugLines
===============
*/
void idGameLocal::DeleteDebugLines()
{
    S32 i;
    UTF8 buf[100];
    
    trap_Cvar_VariableStringBuffer( "bot_maxdebugpolys", buf, sizeof( buf ) );
    
    for( i = 0; i < atoi( buf ); i++ )
    {
        trap_DebugPolygonDelete( i );
    }
}
