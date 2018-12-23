////////////////////////////////////////////////////////////////////////////////////////
// Copyright(C) 2018 Dusan Jocic <dusanjocic@msn.com>
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
// File name:   serverBot.h
// Version:     v1.01
// Created:
// Compilers:   Visual Studio 2017, gcc 7.3.0
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#ifndef __SERVERBOT_H__
#define __SERVERBOT_H__

typedef struct bot_debugpoly_s
{
    S32 inuse;
    S32 color;
    S32 numPoints;
    vec3_t points[128];
} bot_debugpoly_t;

static bot_debugpoly_t* debugpolygons;
static S32 bot_maxdebugpolys;

extern botlib_export_t*	botlib_export;
static S32 bot_enable;

//
// idServerBotSystemLocal
//
class idServerBotSystemLocal : public idServerBotSystem
{
public:
    virtual S32 BotAllocateClient( S32 clientNum );
    virtual void BotFreeClient( S32 clientNum );
    virtual S32 BotLibSetup( void );
    virtual S32 BotLibShutdown( void );
    virtual S32 BotGetConsoleMessage( S32 client, UTF8* buf, S32 size );
    virtual S32 BotGetSnapshotEntity( S32 client, S32 sequence );
    virtual void BotFrame( S32 time );
    virtual void BotInitCvars( void );
    virtual void BotInitBotLib( void );
public:
    idServerBotSystemLocal();
    ~idServerBotSystemLocal();
    
    static void BotDrawDebugPolygons( void( *drawPoly )( S32 color, S32 numPoints, F32* points ), S32 value );
    static void BotImport_Print( S32 type, UTF8* fmt, ... );
    static void BotImport_Trace( bsp_trace_t* bsptrace, vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, S32 passent, S32 contentmask );
    static void BotImport_EntityTrace( bsp_trace_t* bsptrace, vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, S32 entnum, S32 contentmask );
    static S32 BotImport_PointContents( vec3_t point );
    static S32 BotImport_inPVS( vec3_t p1, vec3_t p2 );
    static UTF8* BotImport_BSPEntityData( void );
    static void BotImport_BSPModelMinsMaxsOrigin( S32 modelnum, vec3_t angles, vec3_t outmins, vec3_t outmaxs, vec3_t origin );
    static void* BotImport_GetMemory( S32 size );
    static void BotImport_FreeMemory( void* ptr );
    static void BotImport_FreeZoneMemory( void );
    static bot_debugpoly_t* BotImport_GetFreeDebugPolygon( void );
    static bool BotImport_BotVisibleFromPos( vec3_t srcorigin, S32 srcnum, vec3_t destorigin, S32 destent, bool dummy );
    static bool BotImport_BotCheckAttackAtPos( S32 entnum, S32 enemy, vec3_t pos, bool ducking, bool allowHitWorld );
    static void BotClientCommand( S32 client, UTF8* command );
    static void BotImport_DrawPolygon( S32 color, S32 numpoints, S32* points );
    static bool BotImport_AICast_VisibleFromPos( vec3_t srcpos, S32 srcnum, vec3_t destpos, S32 destnum, bool updateVisPos );
    static bool BotImport_AICast_CheckAttackAtPos( S32 entnum, S32 enemy, vec3_t pos, bool ducking, bool allowHitWorld );
    static S32 EntityInPVS( S32 client, S32 entityNum );
};

extern idServerBotSystemLocal serverBotLocal;

#endif //!__SERVERBOT_H__
