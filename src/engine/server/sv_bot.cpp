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
// File name:   sv_bot.cpp
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#ifdef DEDICATED
#include <null/null_precompiled.h>
#else
#include <OWLib/precompiled.h>
#endif

typedef struct bot_debugpoly_s
{
    S32 inuse;
    S32 color;
    S32 numPoints;
    vec3_t points[128];
} bot_debugpoly_t;

static bot_debugpoly_t* debugpolygons;
S32 bot_maxdebugpolys;

extern botlib_export_t*	botlib_export;
S32	bot_enable;


/*
==================
SV_BotAllocateClient
==================
*/
S32 SV_BotAllocateClient( void )
{
    S32			i;
    client_t*	cl;
    
    // find a client slot
    for( i = 0, cl = svs.clients; i < sv_maxclients->integer; i++, cl++ )
    {
        if( cl->state == CS_FREE )
        {
            break;
        }
    }
    
    if( i == sv_maxclients->integer )
    {
        return -1;
    }
    
    cl->gentity = SV_GentityNum( i );
    cl->gentity->s.number = i;
    cl->state = CS_ACTIVE;
    cl->lastPacketTime = svs.time;
    cl->netchan.remoteAddress.type = NA_BOT;
    cl->rate = 16384;
    
    return i;
}

/*
==================
SV_BotAllocateClient
==================
*/
S32 SV_BotAllocateClient( S32 clientNum )
{
    S32             i;
    client_t*       cl;
    
    // Arnout: added possibility to request a clientnum
    if( clientNum > 0 )
    {
        if( clientNum >= sv_maxclients->integer )
        {
            return -1;
        }
        
        cl = &svs.clients[clientNum];
        if( cl->state != CS_FREE )
        {
            return -1;
        }
        else
        {
            i = clientNum;
        }
    }
    else
    {
        // find a client slot
        for( i = 0, cl = svs.clients; i < sv_maxclients->integer; i++, cl++ )
        {
            // Wolfenstein, never use the first slot, otherwise if a bot connects before the first client on a listen server, game won't start
            if( i < 1 )
            {
                continue;
            }
            // done.
            if( cl->state == CS_FREE )
            {
                break;
            }
        }
    }
    
    if( i == sv_maxclients->integer )
    {
        return -1;
    }
    
    cl->gentity = SV_GentityNum( i );
    cl->gentity->s.number = i;
    cl->state = CS_ACTIVE;
    cl->lastPacketTime = svs.time;
    cl->netchan.remoteAddress.type = NA_BOT;
    cl->rate = 16384;
    
    return i;
}

/*
==================
SV_BotFreeClient
==================
*/
void SV_BotFreeClient( S32 clientNum )
{
    client_t* cl;
    
    if( clientNum < 0 || clientNum >= sv_maxclients->integer )
    {
        Com_Error( ERR_DROP, "SV_BotFreeClient: bad clientNum: %i", clientNum );
    }
    cl = &svs.clients[clientNum];
    cl->state = CS_FREE;
    cl->name[0] = 0;
    if( cl->gentity )
    {
        cl->gentity->r.svFlags &= ~SVF_BOT;
    }
}

/*
==================
BotDrawDebugPolygons
==================
*/
void BotDrawDebugPolygons( void( *drawPoly )( S32 color, S32 numPoints, F32* points ), S32 value )
{
    static cvar_t* bot_debug, *bot_groundonly, *bot_reachability, *bot_highlightarea;
    bot_debugpoly_t* poly;
    S32 i, parm0;
    
    if( !debugpolygons )
        return;
    //bot debugging
    if( !bot_debug ) bot_debug = Cvar_Get( "bot_debug", "0", 0 );
    //
    if( bot_debug->integer )
    {
        //show reachabilities
        if( !bot_reachability ) bot_reachability = Cvar_Get( "bot_reachability", "0", 0 );
        //show ground faces only
        if( !bot_groundonly ) bot_groundonly = Cvar_Get( "bot_groundonly", "1", 0 );
        //get the hightlight area
        if( !bot_highlightarea ) bot_highlightarea = Cvar_Get( "bot_highlightarea", "0", 0 );
        //
        parm0 = 0;
        if( svs.clients[0].lastUsercmd.buttons & BUTTON_ATTACK ) parm0 |= 1;
        if( bot_reachability->integer ) parm0 |= 2;
        if( bot_groundonly->integer ) parm0 |= 4;
        botlib_export->BotLibVarSet( "bot_highlightarea", bot_highlightarea->string );
        botlib_export->Test( parm0, NULL, svs.clients[0].gentity->r.currentOrigin,
                             svs.clients[0].gentity->r.currentAngles );
    } //end if
    //draw all debug polys
    for( i = 0; i < bot_maxdebugpolys; i++ )
    {
        poly = &debugpolygons[i];
        if( !poly->inuse ) continue;
        drawPoly( poly->color, poly->numPoints, ( F32* )poly->points );
        //Com_Printf("poly %i, numpoints = %d\n", i, poly->numPoints);
    }
}

/*
==================
BotImport_Print
==================
*/
void BotImport_Print( S32 type, UTF8* fmt, ... )
{
    UTF8 str[2048];
    va_list ap;
    
    va_start( ap, fmt );
    vsnprintf( str, sizeof( str ), fmt, ap );
    va_end( ap );
    
    switch( type )
    {
        case PRT_MESSAGE:
        {
            Com_Printf( "%s", str );
            break;
        }
        case PRT_WARNING:
        {
            Com_Printf( S_COLOR_YELLOW "Warning: %s", str );
            break;
        }
        case PRT_ERROR:
        {
            Com_Printf( S_COLOR_RED "Error: %s", str );
            break;
        }
        case PRT_FATAL:
        {
            Com_Printf( S_COLOR_RED "Fatal: %s", str );
            break;
        }
        case PRT_EXIT:
        {
            Com_Error( ERR_DROP, S_COLOR_RED "Exit: %s", str );
            break;
        }
        default:
        {
            Com_Printf( "unknown print type\n" );
            break;
        }
    }
}

/*
==================
BotImport_Trace
==================
*/
void BotImport_Trace( bsp_trace_t* bsptrace, vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, S32 passent, S32 contentmask )
{
    trace_t trace;
    
    // always use bounding box for bot stuff ?
    SV_Trace( &trace, start, mins, maxs, end, passent, contentmask, TT_AABB );
    //copy the trace information
    bsptrace->allsolid = trace.allsolid;
    bsptrace->startsolid = trace.startsolid;
    bsptrace->fraction = trace.fraction;
    VectorCopy( trace.endpos, bsptrace->endpos );
    bsptrace->plane.dist = trace.plane.dist;
    VectorCopy( trace.plane.normal, bsptrace->plane.normal );
    bsptrace->plane.signbits = trace.plane.signbits;
    bsptrace->plane.type = trace.plane.type;
    bsptrace->surface.value = trace.surfaceFlags;
    bsptrace->ent = trace.entityNum;
    bsptrace->exp_dist = 0;
    bsptrace->sidenum = 0;
    bsptrace->contents = 0;
}

/*
==================
BotImport_EntityTrace
==================
*/
void BotImport_EntityTrace( bsp_trace_t* bsptrace, vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, S32 entnum, S32 contentmask )
{
    trace_t trace;
    
    // always use bounding box for bot stuff ?
    SV_ClipToEntity( &trace, start, mins, maxs, end, entnum, contentmask, TT_AABB );
    //copy the trace information
    bsptrace->allsolid = trace.allsolid;
    bsptrace->startsolid = trace.startsolid;
    bsptrace->fraction = trace.fraction;
    VectorCopy( trace.endpos, bsptrace->endpos );
    bsptrace->plane.dist = trace.plane.dist;
    VectorCopy( trace.plane.normal, bsptrace->plane.normal );
    bsptrace->plane.signbits = trace.plane.signbits;
    bsptrace->plane.type = trace.plane.type;
    bsptrace->surface.value = trace.surfaceFlags;
    bsptrace->ent = trace.entityNum;
    bsptrace->exp_dist = 0;
    bsptrace->sidenum = 0;
    bsptrace->contents = 0;
}


/*
==================
BotImport_PointContents
==================
*/
S32 BotImport_PointContents( vec3_t point )
{
    return SV_PointContents( point, -1 );
}

/*
==================
BotImport_inPVS
==================
*/
S32 BotImport_inPVS( vec3_t p1, vec3_t p2 )
{
    return SV_inPVS( p1, p2 );
}

/*
==================
BotImport_BSPEntityData
==================
*/
UTF8* BotImport_BSPEntityData( void )
{
    return collisionModelManager->EntityString();
}

/*
==================
BotImport_BSPModelMinsMaxsOrigin
==================
*/
void BotImport_BSPModelMinsMaxsOrigin( S32 modelnum, vec3_t angles, vec3_t outmins, vec3_t outmaxs, vec3_t origin )
{
    clipHandle_t h;
    vec3_t mins, maxs;
    F32 max;
    S32	i;
    
    h = collisionModelManager->InlineModel( modelnum );
    collisionModelManager->ModelBounds( h, mins, maxs );
    //if the model is rotated
    if( ( angles[0] || angles[1] || angles[2] ) )
    {
        // expand for rotation
        
        max = RadiusFromBounds( mins, maxs );
        for( i = 0; i < 3; i++ )
        {
            mins[i] = -max;
            maxs[i] = max;
        }
    }
    if( outmins )
    {
        VectorCopy( mins, outmins );
    }
    if( outmaxs )
    {
        VectorCopy( maxs, outmaxs );
    }
    if( origin )
    {
        VectorClear( origin );
    }
}

/*
==================
BotImport_GetMemory
==================
*/
void* BotImport_GetMemory( S32 size )
{
    void* ptr;
    
    ptr = Z_TagMalloc( size, TAG_BOTLIB );
    return ptr;
}

/*
==================
BotImport_FreeMemory
==================
*/
void BotImport_FreeMemory( void* ptr )
{
    Z_Free( ptr );
}

/*
==================
BotImport_FreeZoneMemory
==================
*/
void BotImport_FreeZoneMemory( void )
{
    Z_FreeTags( TAG_BOTLIB );
}

/*
=================
BotImport_HunkAlloc
=================
*/
void* BotImport_HunkAlloc( S32 size )
{
    if( Hunk_CheckMark() )
    {
        Com_Error( ERR_DROP, "SV_Bot_HunkAlloc: Alloc with marks already set\n" );
    }
    return Hunk_Alloc( size, h_high );
}

/*
==================
BotImport_DebugPolygonCreate
==================
*/
S32 BotImport_DebugPolygonCreate( S32 color, S32 numPoints, vec3_t* points )
{
    bot_debugpoly_t* poly;
    S32 i;
    
    if( !debugpolygons )
    {
        return 0;
    }
    
    for( i = 1; i < bot_maxdebugpolys; i++ )
    {
        if( !debugpolygons[i].inuse )
            break;
    }
    if( i >= bot_maxdebugpolys )
        return 0;
    poly = &debugpolygons[i];
    poly->inuse = true;
    poly->color = color;
    poly->numPoints = numPoints;
    ::memcpy( poly->points, points, numPoints * sizeof( vec3_t ) );
    
    return i;
}

/*
==================
BotImport_DebugPolygonCreate
==================
*/
bot_debugpoly_t* BotImport_GetFreeDebugPolygon( void )
{
    S32 i;
    
    for( i = 1; i < bot_maxdebugpolys; i++ )
    {
        if( !debugpolygons[i].inuse )
        {
            return &debugpolygons[i];
        }
    }
    return NULL;
}

/*
==================
BotImport_DebugPolygonShow
==================
*/
void BotImport_DebugPolygonShow( S32 id, S32 color, S32 numPoints, vec3_t* points )
{
    bot_debugpoly_t* poly;
    
    if( !debugpolygons ) return;
    poly = &debugpolygons[id];
    poly->inuse = true;
    poly->color = color;
    poly->numPoints = numPoints;
    memcpy( poly->points, points, numPoints * sizeof( vec3_t ) );
}

/*
==================
BotImport_DebugPolygonDelete
==================
*/
void BotImport_DebugPolygonDelete( S32 id )
{
    if( !debugpolygons ) return;
    debugpolygons[id].inuse = false;
}

/*
==================
BotImport_DebugPolygonDeletePointer
==================
*/
void BotImport_DebugPolygonDeletePointer( bot_debugpoly_t* pPoly )
{
    pPoly->inuse = false;
}

/*
==================
BotImport_DebugLineCreate
==================
*/
S32 BotImport_DebugLineCreate( void )
{
    vec3_t points[1];
    return BotImport_DebugPolygonCreate( 0, 0, points );
}

/*
==================
BotImport_DebugLineDelete
==================
*/
void BotImport_DebugLineDelete( S32 line )
{
    BotImport_DebugPolygonDelete( line );
}

/*
==================
BotImport_DebugLineShow
==================
*/
void BotImport_DebugLineShow( S32 line, vec3_t start, vec3_t end, S32 color )
{
    vec3_t points[4], dir, cross, up = { 0, 0, 1 };
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
    
    VectorMA( points[0], 2, cross, points[0] );
    VectorMA( points[1], -2, cross, points[1] );
    VectorMA( points[2], -2, cross, points[2] );
    VectorMA( points[3], 2, cross, points[3] );
    
    BotImport_DebugPolygonShow( line, color, 4, points );
    //Com_Printf( S_COLOR_RED "drawing line\n" );
}

/*
==================
BotImport_BotVisibleFromPos
==================
*/
bool BotImport_BotVisibleFromPos( vec3_t srcorigin, S32 srcnum, vec3_t destorigin, S32 destent, bool dummy )
{
    return game->BotVisibleFromPos( srcorigin, srcnum, destorigin, destent, dummy );
}

/*
==================
BotImport_BotCheckAttackAtPos
==================
*/
bool BotImport_BotCheckAttackAtPos( S32 entnum, S32 enemy, vec3_t pos, bool ducking, bool allowHitWorld )
{
    return game->BotCheckAttackAtPos( entnum, enemy, pos, ducking, allowHitWorld );
}

/*
==================
SV_BotClientCommand
==================
*/
void BotClientCommand( S32 client, UTF8* command )
{
    SV_ExecuteClientCommand( &svs.clients[client], command, true, false );
}

/*
==================
SV_BotFrame
==================
*/
void SV_BotFrame( S32 time )
{
    if( !bot_enable )
        return;
        
    //NOTE: maybe the game is already shutdown
    if( !svs.gameStarted )
        return;
        
    if( !gvm )
        return;
        
    game->BotAIStartFrame( time );
}

/*
===============
SV_BotLibSetup
===============
*/
S32 SV_BotLibSetup( void )
{
    static cvar_t*  bot_norcd, *bot_frameroutingupdates;
    
    if( !botlib_export )
    {
        Com_Printf( S_COLOR_RED "Error: SV_BotLibSetup without SV_BotInitBotLib\n" );
        return -1;
    }
    
    // RF, set RCD calculation status
    bot_norcd = Cvar_Get( "bot_norcd", "0", 0 );
    botlib_export->BotLibVarSet( "bot_norcd", bot_norcd->string );
    
    // RF, set AAS routing max per frame
    if( SV_GameIsSinglePlayer() )
    {
        bot_frameroutingupdates = Cvar_Get( "bot_frameroutingupdates", "9999999", 0 );
    }
    else     // more restrictive in multiplayer
    {
        bot_frameroutingupdates = Cvar_Get( "bot_frameroutingupdates", "1000", 0 );
    }
    botlib_export->BotLibVarSet( "bot_frameroutingupdates", bot_frameroutingupdates->string );
    
    
    return botlib_export->BotLibSetup();
}

/*
===============
SV_ShutdownBotLib

Called when either the entire server is being killed, or
it is changing to a different game directory.
===============
*/
S32 SV_BotLibShutdown( void )
{

    if( !botlib_export )
    {
        return -1;
    }
    
    return botlib_export->BotLibShutdown();
}

/*
==================
SV_BotInitCvars
==================
*/
void SV_BotInitCvars( void )
{
    Cvar_Get( "bot_enable", "1", 0 );						//enable the bot
    Cvar_Get( "bot_developer", "0", CVAR_CHEAT );			//bot developer mode
    Cvar_Get( "bot_debug", "0", CVAR_CHEAT );				//enable bot debugging
    Cvar_Get( "bot_maxdebugpolys", "100", 0 );				//maximum number of debug polys
    Cvar_Get( "bot_groundonly", "1", 0 );					//only show ground faces of areas
    Cvar_Get( "bot_reachability", "0", 0 );				//show all reachabilities to other areas
    Cvar_Get( "bot_visualizejumppads", "0", CVAR_CHEAT );	//show jumppads
    Cvar_Get( "bot_forceclustering", "0", 0 );			//force cluster calculations
    Cvar_Get( "bot_forcereachability", "0", 0 );			//force reachability calculations
    Cvar_Get( "bot_forcewrite", "0", 0 );					//force writing aas file
    Cvar_Get( "bot_aasoptimize", "0", 0 );				//no aas file optimisation
    Cvar_Get( "bot_saveroutingcache", "0", 0 );			//save routing cache
    Cvar_Get( "bot_thinktime", "100", CVAR_CHEAT );		//msec the bots thinks
    Cvar_Get( "bot_reloadcharacters", "0", 0 );			//reload the bot characters each time
    Cvar_Get( "bot_testichat", "0", 0 );					//test ichats
    Cvar_Get( "bot_testrchat", "0", 0 );					//test rchats
    Cvar_Get( "bot_testsolid", "0", CVAR_CHEAT );			//test for solid areas
    Cvar_Get( "bot_testclusters", "0", CVAR_CHEAT );		//test the AAS clusters
    Cvar_Get( "bot_fastchat", "0", 0 );					//fast chatting bots
    Cvar_Get( "bot_nochat", "0", 0 );						//disable chats
    Cvar_Get( "bot_pause", "0", CVAR_CHEAT );				//pause the bots thinking
    Cvar_Get( "bot_report", "0", CVAR_CHEAT );			//get a full report in ctf
    Cvar_Get( "bot_grapple", "0", 0 );					//enable grapple
    Cvar_Get( "bot_rocketjump", "1", 0 );					//enable rocket jumping
    Cvar_Get( "bot_challenge", "0", 0 );					//challenging bot
    Cvar_Get( "bot_minplayers", "0", 0 );					//minimum players in a team or the game
}

#ifndef DEDICATED
void            BotImport_DrawPolygon( S32 color, S32 numpoints, F32* points );
#else
/*
==================
BotImport_DrawPolygon
==================
*/
void BotImport_DrawPolygon( S32 color, S32 numpoints, S32* points )
{
    Com_DPrintf( "BotImport_DrawPolygon stub\n" );
}
#endif

// Ridah, Cast AI
/*
===============
BotImport_AICast_VisibleFromPos
===============
*/
bool BotImport_AICast_VisibleFromPos( vec3_t srcpos, S32 srcnum, vec3_t destpos, S32 destnum, bool updateVisPos )
{
    return game->BotVisibleFromPos( srcpos, srcnum, destpos, destnum, updateVisPos );
}

/*
===============
BotImport_AICast_CheckAttackAtPos
===============
*/
bool BotImport_AICast_CheckAttackAtPos( S32 entnum, S32 enemy, vec3_t pos, bool ducking, bool allowHitWorld )
{
    return game->BotCheckAttackAtPos( entnum, enemy, pos, ducking, allowHitWorld );
}
// done.

/*
==================
SV_BotInitBotLib
==================
*/
void SV_BotInitBotLib( void )
{
    botlib_import_t	botlib_import;
    
    if( debugpolygons ) Z_Free( debugpolygons );
    bot_maxdebugpolys = Cvar_VariableIntegerValue( "bot_maxdebugpolys" );
    debugpolygons = ( bot_debugpoly_t* )Z_Malloc( sizeof( bot_debugpoly_t ) * bot_maxdebugpolys );
    
    botlib_import.Print = BotImport_Print;
    botlib_import.Trace = BotImport_Trace;
    botlib_import.EntityTrace = BotImport_EntityTrace;
    botlib_import.PointContents = BotImport_PointContents;
    botlib_import.inPVS = BotImport_inPVS;
    botlib_import.BSPEntityData = BotImport_BSPEntityData;
    botlib_import.BSPModelMinsMaxsOrigin = BotImport_BSPModelMinsMaxsOrigin;
    botlib_import.BotClientCommand = BotClientCommand;
    
    //memory management
    botlib_import.GetMemory = BotImport_GetMemory;
    botlib_import.FreeMemory = BotImport_FreeMemory;
    botlib_import.AvailableMemory = Z_AvailableMemory;
    botlib_import.HunkAlloc = BotImport_HunkAlloc;
    
    // file system access
    botlib_import.FS_FOpenFile = FS_FOpenFileByMode;
    botlib_import.FS_Read = FS_Read;
    botlib_import.FS_Write = FS_Write;
    botlib_import.FS_FCloseFile = FS_FCloseFile;
    botlib_import.FS_Seek = FS_Seek;
    
    //debug lines
    botlib_import.DebugLineCreate = BotImport_DebugLineCreate;
    botlib_import.DebugLineDelete = BotImport_DebugLineDelete;
    botlib_import.DebugLineShow = BotImport_DebugLineShow;
    
    //debug polygons
    botlib_import.DebugPolygonCreate = BotImport_DebugPolygonCreate;
    botlib_import.DebugPolygonDelete = BotImport_DebugPolygonDelete;
    
    //bot routines
    botlib_import.BotVisibleFromPos = BotImport_BotVisibleFromPos;
    botlib_import.BotCheckAttackAtPos = BotImport_BotCheckAttackAtPos;
    
    botlib_export = ( botlib_export_t* )GetBotLibAPI( BOTLIB_API_VERSION, &botlib_import );
    assert( botlib_export );
}


//
//  * * * BOT AI CODE IS BELOW THIS POINT * * *
//

/*
==================
SV_BotGetConsoleMessage
==================
*/
S32 SV_BotGetConsoleMessage( S32 client, UTF8* buf, S32 size )
{
    client_t*	cl;
    S32			index;
    
    cl = &svs.clients[client];
    cl->lastPacketTime = svs.time;
    
    if( cl->reliableAcknowledge == cl->reliableSequence )
    {
        return false;
    }
    
    cl->reliableAcknowledge++;
    index = cl->reliableAcknowledge & ( MAX_RELIABLE_COMMANDS - 1 );
    
    if( !cl->reliableCommands[index][0] )
    {
        return false;
    }
    
    Q_strncpyz( buf, cl->reliableCommands[index], size );
    return true;
}

/*
==================
EntityInPVS
==================
*/
S32 EntityInPVS( S32 client, S32 entityNum )
{
    client_t*			cl;
    clientSnapshot_t*	frame;
    S32					i;
    
    cl = &svs.clients[client];
    frame = &cl->frames[cl->netchan.outgoingSequence & PACKET_MASK];
    for( i = 0; i < frame->num_entities; i++ )
    {
        if( svs.snapshotEntities[( frame->first_entity + i ) % svs.numSnapshotEntities].number == entityNum )
        {
            return true;
        }
    }
    return false;
}

/*
==================
SV_BotGetSnapshotEntity
==================
*/
S32 SV_BotGetSnapshotEntity( S32 client, S32 sequence )
{
    client_t*			cl;
    clientSnapshot_t*	frame;
    
    cl = &svs.clients[client];
    frame = &cl->frames[cl->netchan.outgoingSequence & PACKET_MASK];
    if( sequence < 0 || sequence >= frame->num_entities )
    {
        return -1;
    }
    return svs.snapshotEntities[( frame->first_entity + sequence ) % svs.numSnapshotEntities].number;
}

