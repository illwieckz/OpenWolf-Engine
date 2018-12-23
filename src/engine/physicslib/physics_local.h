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
// File name:   physics_local.h
// Version:     v1.01
// Created:
// Compilers:   Visual Studio 2017, gcc 7.3.0
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#ifndef __PHYSICS_LOCAL_H__
#define __PHYSICS_LOCAL_H__

#ifndef __Q_SHARED_H__
#include <qcommon/q_shared.h>
#endif
#ifndef __QCOMMON_H__
#include <qcommon/qcommon.h>
#endif
#ifndef __CM_LOCAL_H__
#include <cm/cm_local.h>
#endif
#ifndef __SURFACEFLAGS_H__
#include <qcommon/surfaceflags.h>
#endif
#ifndef __MATH_ANGLES_H__
#include <OWLib/math_angles.h>
#endif
#ifndef __MATH_MATRIX_H__
#include <OWLib/math_matrix.h>
#endif
#ifndef __MATH_ANGLES_H__
#include <OWLib/math_angles.h>
#endif
#ifndef BULLET_C_API_H
#include <Bullet-C-Api.h>
#endif
#ifndef BULLET_COLLISION_COMMON_H
#include <btBulletCollisionCommon.h>
#endif
#ifndef BULLET_DYNAMICS_COMMON_H
#include <btBulletDynamicsCommon.h>
#endif
#ifndef BT_GEOMETRY_UTIL_H
#include <LinearMath/btGeometryUtil.h>
#endif
#ifndef BT_GHOST_OBJECT_H
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#endif
#ifndef BULLET_WORLD_IMPORTER_H
#include <btBulletWorldImporter.h>
#endif
#ifndef __PHYSICS_COLLISION_H__
#include <physicslib/physics_collision.h>
#endif

//
// owPhysicsManagerLocal
//
class owPhysicsManagerLocal : public owPhysicsManager
{
public:
    virtual void			Init( void );
    virtual void			Shutdown( void );
    virtual void			SetGravity( const idVec3& gravity );
    virtual void			Frame( void );
    virtual void			CreateCollisionModelFromBSP( void* collisionModel );
    virtual void			CreateCollisionModelFromAbstractBSP( idPhysicsAbstractBSP* collisionModel );
    virtual void			WriteBulletPhysicsFile( StringEntry fullpath );
    virtual void            LoadBulletPhysicsFile( StringEntry fullpath );
    virtual idTraceModel*	AllocTraceModel();
    virtual idTraceModel*	GetTraceModelForEntity( S32 entityNum );
    virtual void			DrawDebug();
    virtual void			TraceModel( trace_t* results, idTraceModel* model, const idVec3& start, const idVec3& end, S32 mask );
    virtual void			Reset( void );
    
    //
    // GetCollisionManager
    //
    owPhysicsCollisionDetection* GetCollisionManager( void )
    {
        return &collision;
    }
    
    //
    // GetDefaultContactProcessingThreshold
    //
    F32 GetDefaultContactProcessingThreshold( void )
    {
        return BT_LARGE_FLOAT;
    }
    
    //
    // GetWorldDynamics
    //
    btDiscreteDynamicsWorld* GetWorldDynamics( void )
    {
        return dynamicsWorld;
    }
private:
    btScalar getDeltaTimeMicroseconds();
protected:
    btClock m_clock;
    
    btBroadphaseInterface*	broadphase;
    btConstraintSolver*		solver;
    btDiscreteDynamicsWorld* dynamicsWorld;
    
    owPhysicsCollisionDetection collision;
};


#if !defined ( Q3MAP2 ) && !defined ( DEDICATED )
class GLDebugDrawer : public btIDebugDraw
{
    S32 m_debugMode;
    
public:

    GLDebugDrawer();
    
    
    virtual void	drawLine( const btVector3& from, const btVector3& to, const btVector3& fromColor, const btVector3& toColor );
    
    virtual void	drawLine( const btVector3& from, const btVector3& to, const btVector3& color );
    
    virtual void	drawSphere( const btVector3& p, btScalar radius, const btVector3& color );
    virtual void	drawBox( const btVector3& boxMin, const btVector3& boxMax, const btVector3& color, btScalar alpha );
    
    virtual void	drawTriangle( const btVector3& a, const btVector3& b, const btVector3& c, const btVector3& color, btScalar alpha );
    
    virtual void	drawContactPoint( const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, S32 lifeTime, const btVector3& color );
    
    virtual void	reportErrorWarning( StringEntry warningString );
    
    virtual void	draw3dText( const btVector3& location, StringEntry textString );
    
    virtual void	setDebugMode( S32 debugMode );
    
    virtual S32		getDebugMode() const
    {
        return m_debugMode;
    }
};
#endif

extern owPhysicsManagerLocal physicsManagerLocal;

#endif //!__PHYSICS_LOCAL_H__
