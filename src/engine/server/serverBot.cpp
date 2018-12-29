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
// Version:     v1.01
// Created:
// Compilers:   Visual Studio 2017, gcc 7.3.0
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#ifdef DEDICATED
#include <null/null_precompiled.h>
#else
#include <OWLib/precompiled.h>
#endif

idServerBotSystemLocal serverBotSystemLocal;
idServerBotSystem* serverBotSystem = &serverBotSystemLocal;

/*
===============
idServerBotSystemLocal::idServerBotSystemLocal
===============
*/
idServerBotSystemLocal::idServerBotSystemLocal( void )
{
}

/*
===============
idServerBotSystemLocal::~idServerBotSystemLocal
===============
*/
idServerBotSystemLocal::~idServerBotSystemLocal( void )
{
}

/*
==================
idServerBotSystemLocal::BotAllocateClient
==================
*/
S32 idServerBotSystemLocal::BotAllocateClient( S32 clientNum )
{
    S32 i;
    client_t* cl;
    
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
    
    cl->gentity = serverGameSystem->GentityNum( i );
    cl->gentity->s.number = i;
    cl->state = CS_ACTIVE;
    cl->lastPacketTime = svs.time;
    cl->netchan.remoteAddress.type = NA_BOT;
    cl->rate = 16384;
    
    return i;
}

/*
==================
idServerBotSystemLocal::BotFreeClient
==================
*/
void idServerBotSystemLocal::BotFreeClient( S32 clientNum )
{
    client_t* cl;
    
    if( clientNum < 0 || clientNum >= sv_maxclients->integer )
    {
        Com_Error( ERR_DROP, "idServerBotSystemLocal::BotFreeClient: bad clientNum: %i", clientNum );
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
idServerBotSystemLocal::BotDrawDebugPolygons
==================
*/
void idServerBotSystemLocal::BotDrawDebugPolygons( void( *drawPoly )( S32 color, S32 numPoints, F32* points ), S32 value )
{
    static cvar_t* bot_debug, *bot_groundonly, *bot_reachability, *bot_highlightarea;
    bot_debugpoly_t* poly;
    S32 i, parm0;
    
    if( !debugpolygons )
    {
        return;
    }
    
    //bot debugging
    if( !bot_debug )
    {
        bot_debug = cvarSystem->Get( "bot_debug", "0", 0 );
    }
    
    if( bot_debug->integer )
    {
        //show reachabilities
        if( !bot_reachability )
        {
            bot_reachability = cvarSystem->Get( "bot_reachability", "0", 0 );
        }
        
        //show ground faces only
        if( !bot_groundonly )
        {
            bot_groundonly = cvarSystem->Get( "bot_groundonly", "1", 0 );
        }
        
        //get the hightlight area
        if( !bot_highlightarea )
        {
            bot_highlightarea = cvarSystem->Get( "bot_highlightarea", "0", 0 );
        }
        
        parm0 = 0;
        
        if( svs.clients[0].lastUsercmd.buttons & BUTTON_ATTACK )
        {
            parm0 |= 1;
        }
        if( bot_reachability->integer )
        {
            parm0 |= 2;
        }
        if( bot_groundonly->integer )
        {
            parm0 |= 4;
        }
        botlib_export->BotLibVarSet( "bot_highlightarea", bot_highlightarea->string );
        botlib_export->Test( parm0, NULL, svs.clients[0].gentity->r.currentOrigin, svs.clients[0].gentity->r.currentAngles );
        
    }
    
    //draw all debug polys
    for( i = 0; i < bot_maxdebugpolys; i++ )
    {
        poly = &debugpolygons[i];
        
        if( !poly->inuse )
        {
            continue;
        }
        
        drawPoly( poly->color, poly->numPoints, ( F32* )poly->points );
        //Com_Printf("poly %i, numpoints = %d\n", i, poly->numPoints);
    }
}

/*
==================
idServerBotSystemLocal::BotImport_Print
==================
*/
void idServerBotSystemLocal::BotImport_Print( S32 type, UTF8* fmt, ... )
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
idServerBotSystemLocal::BotImport_Trace
==================
*/
void idServerBotSystemLocal::BotImport_Trace( bsp_trace_t* bsptrace, vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, S32 passent, S32 contentmask )
{
    trace_t trace;
    
    // always use bounding box for bot stuff ?
    serverWorldSystemLocal.Trace( &trace, start, mins, maxs, end, passent, contentmask, TT_AABB );
    
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
idServerBotSystemLocal::BotImport_EntityTrace
==================
*/
void idServerBotSystemLocal::BotImport_EntityTrace( bsp_trace_t* bsptrace, vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, S32 entnum, S32 contentmask )
{
    trace_t trace;
    
    // always use bounding box for bot stuff ?
    serverWorldSystemLocal.ClipToEntity( &trace, start, mins, maxs, end, entnum, contentmask, TT_AABB );
    
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
idServerBotSystemLocal::BotImport_PointContents
==================
*/
S32 idServerBotSystemLocal::BotImport_PointContents( vec3_t point )
{
    return serverWorldSystemLocal.PointContents( point, -1 );
}

/*
==================
idServerBotSystemLocal::BotImport_inPVS
==================
*/
S32 idServerBotSystemLocal::BotImport_inPVS( vec3_t p1, vec3_t p2 )
{
    return serverGameSystem->inPVS( p1, p2 );
}

/*
==================
idServerBotSystemLocal::BotImport_BSPEntityData
==================
*/
UTF8* idServerBotSystemLocal::BotImport_BSPEntityData( void )
{
    return collisionModelManager->EntityString();
}

/*
==================
idServerBotSystemLocal::BotImport_BSPModelMinsMaxsOrigin
==================
*/
void idServerBotSystemLocal::BotImport_BSPModelMinsMaxsOrigin( S32 modelnum, vec3_t angles, vec3_t outmins, vec3_t outmaxs, vec3_t origin )
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
idServerBotSystemLocal::BotImport_GetMemory
==================
*/
void* idServerBotSystemLocal::BotImport_GetMemory( S32 size )
{
    void* ptr;
    
    ptr = Z_TagMalloc( size, TAG_BOTLIB );
    return ptr;
}

/*
==================
idServerBotSystemLocal::BotImport_FreeMemory
==================
*/
void idServerBotSystemLocal::BotImport_FreeMemory( void* ptr )
{
    Z_Free( ptr );
}

/*
==================
idServerBotSystemLocal::BotImport_FreeZoneMemory
==================
*/
void idServerBotSystemLocal::BotImport_FreeZoneMemory( void )
{
    Z_FreeTags( TAG_BOTLIB );
}

/*
=================
idServerBotSystemLocal::BotImport_HunkAlloc
=================
*/
void* BotImport_HunkAlloc( S32 size )
{
    if( Hunk_CheckMark() )
    {
        Com_Error( ERR_DROP, "idServerBotSystemLocal::Bot_HunkAlloc: Alloc with marks already set\n" );
    }
    return Hunk_Alloc( size, h_high );
}

/*
==================
idServerBotSystemLocal::BotImport_DebugPolygonCreate
==================
*/
bot_debugpoly_t* idServerBotSystemLocal::BotImport_GetFreeDebugPolygon( void )
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
idServerBotSystemLocal::BotImport_BotVisibleFromPos
==================
*/
bool idServerBotSystemLocal::BotImport_BotVisibleFromPos( vec3_t srcorigin, S32 srcnum, vec3_t destorigin, S32 destent, bool dummy )
{
    return game->BotVisibleFromPos( srcorigin, srcnum, destorigin, destent, dummy );
}

/*
==================
idServerBotSystemLocal::BotImport_BotCheckAttackAtPos
==================
*/
bool idServerBotSystemLocal::BotImport_BotCheckAttackAtPos( S32 entnum, S32 enemy, vec3_t pos, bool ducking, bool allowHitWorld )
{
    return game->BotCheckAttackAtPos( entnum, enemy, pos, ducking, allowHitWorld );
}

/*
==================
idServerBotSystemLocal::BotClientCommand
==================
*/
void idServerBotSystemLocal::BotClientCommand( S32 client, UTF8* command )
{
    serverClientSystem->ExecuteClientCommand( &svs.clients[client], command, true, false );
}

/*
==================
idServerBotSystemLocal::BotFrame
==================
*/
void idServerBotSystemLocal::BotFrame( S32 time )
{
    if( !bot_enable )
    {
        return;
    }
    
    //NOTE: maybe the game is already shutdown
    if( !svs.gameStarted )
    {
        return;
    }
    
    if( !gvm )
    {
        return;
    }
    
    game->BotAIStartFrame( time );
}

/*
===============
idServerBotSystemLocal::BotLibSetup
===============
*/
S32 idServerBotSystemLocal::BotLibSetup( void )
{
    static cvar_t*  bot_norcd, *bot_frameroutingupdates;
    
    if( !botlib_export )
    {
        Com_Printf( S_COLOR_RED "Error: idServerBotSystemLocal::BotLibSetup without SV_BotInitBotLib\n" );
        return -1;
    }
    
    // RF, set RCD calculation status
    bot_norcd = cvarSystem->Get( "bot_norcd", "0", 0 );
    botlib_export->BotLibVarSet( "bot_norcd", bot_norcd->string );
    
    // RF, set AAS routing max per frame
    if( serverGameSystem->GameIsSinglePlayer() )
    {
        bot_frameroutingupdates = cvarSystem->Get( "bot_frameroutingupdates", "9999999", 0 );
    }
    else
    {
        // more restrictive in multiplayer
        bot_frameroutingupdates = cvarSystem->Get( "bot_frameroutingupdates", "1000", 0 );
    }
    
    botlib_export->BotLibVarSet( "bot_frameroutingupdates", bot_frameroutingupdates->string );
    
    return botlib_export->BotLibSetup();
}

/*
===============
idServerBotSystemLocal::ShutdownBotLib

Called when either the entire server is being killed, or
it is changing to a different game directory.
===============
*/
S32 idServerBotSystemLocal::BotLibShutdown( void )
{

    if( !botlib_export )
    {
        return -1;
    }
    
    return botlib_export->BotLibShutdown();
}

/*
==================
idServerBotSystemLocal::BotInitCvars
==================
*/
void idServerBotSystemLocal::BotInitCvars( void )
{
    cvarSystem->Get( "bot_enable", "1", 0 );						//enable the bot
    cvarSystem->Get( "bot_developer", "0", CVAR_CHEAT );			//bot developer mode
    cvarSystem->Get( "bot_debug", "0", CVAR_CHEAT );				//enable bot debugging
    cvarSystem->Get( "bot_maxdebugpolys", "100", 0 );				//maximum number of debug polys
    cvarSystem->Get( "bot_groundonly", "1", 0 );					//only show ground faces of areas
    cvarSystem->Get( "bot_reachability", "0", 0 );				//show all reachabilities to other areas
    cvarSystem->Get( "bot_visualizejumppads", "0", CVAR_CHEAT );	//show jumppads
    cvarSystem->Get( "bot_forceclustering", "0", 0 );			//force cluster calculations
    cvarSystem->Get( "bot_forcereachability", "0", 0 );			//force reachability calculations
    cvarSystem->Get( "bot_forcewrite", "0", 0 );					//force writing aas file
    cvarSystem->Get( "bot_aasoptimize", "0", 0 );				//no aas file optimisation
    cvarSystem->Get( "bot_saveroutingcache", "0", 0 );			//save routing cache
    cvarSystem->Get( "bot_thinktime", "100", CVAR_CHEAT );		//msec the bots thinks
    cvarSystem->Get( "bot_reloadcharacters", "0", 0 );			//reload the bot characters each time
    cvarSystem->Get( "bot_testichat", "0", 0 );					//test ichats
    cvarSystem->Get( "bot_testrchat", "0", 0 );					//test rchats
    cvarSystem->Get( "bot_testsolid", "0", CVAR_CHEAT );			//test for solid areas
    cvarSystem->Get( "bot_testclusters", "0", CVAR_CHEAT );		//test the AAS clusters
    cvarSystem->Get( "bot_fastchat", "0", 0 );					//fast chatting bots
    cvarSystem->Get( "bot_nochat", "0", 0 );						//disable chats
    cvarSystem->Get( "bot_pause", "0", CVAR_CHEAT );				//pause the bots thinking
    cvarSystem->Get( "bot_report", "0", CVAR_CHEAT );			//get a full report in ctf
    cvarSystem->Get( "bot_grapple", "0", 0 );					//enable grapple
    cvarSystem->Get( "bot_rocketjump", "1", 0 );					//enable rocket jumping
    cvarSystem->Get( "bot_challenge", "0", 0 );					//challenging bot
    cvarSystem->Get( "bot_minplayers", "0", 0 );					//minimum players in a team or the game
}

#ifndef DEDICATED
void BotImport_DrawPolygon( S32 color, S32 numpoints, F32* points );
#else
/*
==================
BotImport_DrawPolygon
==================
*/
void idServerBotSystemLocal::BotImport_DrawPolygon( S32 color, S32 numpoints, S32* points )
{
    Com_DPrintf( "BotImport_DrawPolygon stub\n" );
}
#endif

// Ridah, Cast AI
/*
===============
idServerBotSystemLocal::BotImport_AICast_VisibleFromPos
===============
*/
bool idServerBotSystemLocal::BotImport_AICast_VisibleFromPos( vec3_t srcpos, S32 srcnum, vec3_t destpos, S32 destnum, bool updateVisPos )
{
    return game->BotVisibleFromPos( srcpos, srcnum, destpos, destnum, updateVisPos );
}

/*
===============
idServerBotSystemLocal::BotImport_AICast_CheckAttackAtPos
===============
*/
bool idServerBotSystemLocal::BotImport_AICast_CheckAttackAtPos( S32 entnum, S32 enemy, vec3_t pos, bool ducking, bool allowHitWorld )
{
    return game->BotCheckAttackAtPos( entnum, enemy, pos, ducking, allowHitWorld );
}
// done.

/*
==================
idServerBotSystemLocal::BotInitBotLib
==================
*/
void idServerBotSystemLocal::BotInitBotLib( void )
{
    botlib_import_t	botlib_import;
    
    if( debugpolygons )
    {
        Z_Free( debugpolygons );
    }
    
    bot_maxdebugpolys = cvarSystem->VariableIntegerValue( "bot_maxdebugpolys" );
    debugpolygons = ( bot_debugpoly_t* )Z_Malloc( sizeof( bot_debugpoly_t ) * bot_maxdebugpolys );
    
    botlib_import.Print = &idServerBotSystemLocal::BotImport_Print;
    botlib_import.Trace = &idServerBotSystemLocal::BotImport_Trace;
    botlib_import.EntityTrace = &idServerBotSystemLocal::BotImport_EntityTrace;
    botlib_import.PointContents = &idServerBotSystemLocal::BotImport_PointContents;
    botlib_import.inPVS = &idServerBotSystemLocal::BotImport_inPVS;
    botlib_import.BSPEntityData = &idServerBotSystemLocal::BotImport_BSPEntityData;
    botlib_import.BSPModelMinsMaxsOrigin = &idServerBotSystemLocal::BotImport_BSPModelMinsMaxsOrigin;
    botlib_import.BotClientCommand = &idServerBotSystemLocal::BotClientCommand;
    
    //memory management
    botlib_import.GetMemory = &idServerBotSystemLocal::BotImport_GetMemory;
    botlib_import.FreeMemory = &idServerBotSystemLocal::BotImport_FreeMemory;
    botlib_import.AvailableMemory = Z_AvailableMemory;
    botlib_import.HunkAlloc = BotImport_HunkAlloc;
    
    //bot routines
    botlib_import.BotVisibleFromPos = &idServerBotSystemLocal::BotImport_BotVisibleFromPos;
    botlib_import.BotCheckAttackAtPos = &idServerBotSystemLocal::BotImport_BotCheckAttackAtPos;
    
    botlib_export = ( botlib_export_t* )GetBotLibAPI( BOTLIB_API_VERSION, &botlib_import );
    assert( botlib_export );
}


//
//  * * * BOT AI CODE IS BELOW THIS POINT * * *
//

/*
==================
idServerBotSystemLocal::BotGetConsoleMessage
==================
*/
S32 idServerBotSystemLocal::BotGetConsoleMessage( S32 client, UTF8* buf, S32 size )
{
    client_t* cl;
    S32 index;
    
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
idServerBotSystemLocal::EntityInPVS
==================
*/
S32 idServerBotSystemLocal::EntityInPVS( S32 client, S32 entityNum )
{
    client_t* cl;
    clientSnapshot_t* frame;
    S32 i;
    
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
idServerBotSystemLocal::BotGetSnapshotEntity
==================
*/
S32 idServerBotSystemLocal::BotGetSnapshotEntity( S32 client, S32 sequence )
{
    client_t* cl;
    clientSnapshot_t* frame;
    
    cl = &svs.clients[client];
    frame = &cl->frames[cl->netchan.outgoingSequence & PACKET_MASK];
    
    if( sequence < 0 || sequence >= frame->num_entities )
    {
        return -1;
    }
    
    return svs.snapshotEntities[( frame->first_entity + sequence ) % svs.numSnapshotEntities].number;
}
