////////////////////////////////////////////////////////////////////////////////////////
// Copyright(C) 2011 JV Software
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
// File name:   physics_main.cpp
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#include <OWLib/precompiled.h>

owPhysicsManagerLocal	physicsManagerLocal;
owPhysicsManager*		physicsManager = &physicsManagerLocal;

#if !defined ( Q3MAP2 ) && !defined ( DEDICATED )
GLDebugDrawer debugDrawer;
#endif

/*
==============
owPhysicsManager::Init
==============
*/
void owPhysicsManagerLocal::Init( void )
{
#ifndef Q3MAP2
    Com_Printf( "----------- Bullet Physics Init ----------\n" );
    
    Com_Printf( "Init Physics Similuation Environment...\n" );
#endif
    collision.Init();
    
    broadphase = new btDbvtBroadphase();
    solver = new btSequentialImpulseConstraintSolver();
    dynamicsWorld = new btDiscreteDynamicsWorld( collision.GetCollisionDispatcher(), broadphase, solver, collision.GetCollisionConfiguration() );
    
//	debugDrawer.setDebugMode(btIDebugDraw::DBG_DrawWireframe);
//	dynamicsWorld->setDebugDrawer(&debugDrawer);

    dynamicsWorld->getSolverInfo().m_solverMode |= SOLVER_RANDMIZE_ORDER;
    broadphase->getOverlappingPairCache()->setInternalGhostPairCallback( new btGhostPairCallback() );
}

/*
==============
owPhysicsManagerLocal::Shutdown
==============
*/
void owPhysicsManagerLocal::Shutdown( void )
{
    if( dynamicsWorld == NULL )
    {
        return;
    }
#ifndef Q3MAP2
    Com_Printf( "----------- Bullet Physics Shutdown ----------\n" );
#endif
    
    collision.Shutdown();
    
    //delete dynamics world
    delete dynamicsWorld;
    dynamicsWorld = NULL;
    
    //delete solver
    delete solver;
    
    //delete broadphase
    delete broadphase;
}

/*
==============
owPhysicsManagerLocal::Reset
==============
*/
void owPhysicsManagerLocal::Reset( void )
{
    //reset some internal cached data in the broadphase
    dynamicsWorld->getBroadphase()->resetPool( dynamicsWorld->getDispatcher() );
    dynamicsWorld->getConstraintSolver()->reset();
}

/*
==============
owPhysicsManagerLocal::TraceModel
==============
*/
void owPhysicsManagerLocal::TraceModel( trace_t* results, idTraceModel* model, const idVec3& start, const idVec3& end, S32 mask )
{
    collision.TraceModel( results, model, start, end, mask );
}

/*
==============
owPhysicsManagerLocal::AllocTraceModel
==============
*/
idTraceModel* owPhysicsManagerLocal::AllocTraceModel()
{
    return collision.AllocTraceModel();
}

/*
==============
owPhysicsManager::SetGravity
==============
*/
void owPhysicsManagerLocal::SetGravity( const idVec3& gravity )
{
    dynamicsWorld->setGravity( ( const btVector3& )gravity );
}

/*
==============
owPhysicsManagerLocal::getDeltaTimeMicroseconds
==============
*/
btScalar owPhysicsManagerLocal::getDeltaTimeMicroseconds()
{
    btScalar dt = ( btScalar )m_clock.getTimeMicroseconds();
    m_clock.reset();
    return dt;
}

/*
==============
owPhysicsManagerLocal::DrawDebug
==============
*/
void owPhysicsManagerLocal::DrawDebug()
{
    dynamicsWorld->debugDrawWorld();
}

/*
==============
owPhysicsManagerLocal::Frame
==============
*/
void owPhysicsManagerLocal::Frame( void )
{
    //Dushan
    if( dynamicsWorld == 0 )
    {
        return;
    }
    
    F32 dt = getDeltaTimeMicroseconds() * 0.000001f;
    
    //during idle mode, just run 1 simulation step maximum
    S32 numSimSteps = 0;
    numSimSteps = dynamicsWorld->stepSimulation( dt, 100, 1.0f / 60.0f );
}

/*
==============
GetTraceModelForEntity
==============
*/
idTraceModel* owPhysicsManagerLocal::GetTraceModelForEntity( S32 entityNum )
{
    return collision.GetTraceModelForEntity( entityNum );
}

/*
==============
owPhysicsManagerLocal::CreateCollisionModelFromBSP
==============
*/
void owPhysicsManagerLocal::CreateCollisionModelFromBSP( void* collisionModel )
{
    collision.CreateCollisionModelFromBSP( ( clipMap_t* )collisionModel );
}

/*
==============
owPhysicsManagerLocal::CreateCollisionModelFromAbstractBSP
==============
*/
void owPhysicsManagerLocal::CreateCollisionModelFromAbstractBSP( idPhysicsAbstractBSP* collisionModel )
{
    collision.CreateCollisionModelFromAbstractBSP( collisionModel );
}

/*
==============
owPhysicsManagerLocal::WriteBulletPhysicsFile
==============
*/
void owPhysicsManagerLocal::WriteBulletPhysicsFile( StringEntry fullpath )
{
    S32 maxSerializeBufferSize = 1024 * 1024 * 10;
    
    //Serialize the entire world.
    btDefaultSerializer* serializer = new btDefaultSerializer( maxSerializeBufferSize );
    dynamicsWorld->serialize( serializer );
    
    FILE* f2 = fopen( fullpath, "wb" );
    fwrite( serializer->getBufferPointer(), serializer->getCurrentBufferSize(), 1, f2 );
    fclose( f2 );
}

/*
==============
owPhysicsManagerLocal::LoadBulletPhysicsFile
==============
*/
void owPhysicsManagerLocal::LoadBulletPhysicsFile( StringEntry fullpath )
{
#ifndef Q3MAP2
    UTF8 cmpath[1024];
    void* buffer;
    
    btBulletWorldImporter* fileLoader = new btBulletWorldImporter( dynamicsWorld );
    
    COM_StripExtension( fullpath, cmpath );
    Com_sprintf( cmpath, sizeof( cmpath ), "%s.bullet", cmpath );
    
    S32 cmlen = FS_ReadFile( cmpath, &buffer );
    if( cmlen <= 0 || buffer == NULL )
    {
        Com_Printf( "owPhysicsManagerLocal::LoadBulletPhysicsFile: Failed to load bullet - %s\n", cmpath );
    }
    else
    {
        Com_Printf( "owPhysicsManagerLocal::LoadBulletPhysicsFile: Loaded bullet file - %s\n", cmpath );
        //Load .bullet file
        fileLoader->loadFile( cmpath );
        FS_FreeFile( buffer );
    }
#endif
}
