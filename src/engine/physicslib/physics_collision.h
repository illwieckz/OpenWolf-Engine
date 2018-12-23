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
// File name:   physics_collision.h
// Version:     v1.01
// Created:
// Compilers:   Visual Studio 2017, gcc 7.3.0
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#ifndef __PHYSICS_COLLISION_H__
#define __PHYSICS_COLLISION_H__

#ifndef __R_LOCAL_H__
#ifndef Q3MAP2
#include <GPURenderer/r_local.h>
#endif
#endif

//
// idTraceModel
//
class idTraceModelLocal : public idTraceModel
{
public:
    idTraceModelLocal()
    {
        collisionShape = NULL;
    };
    virtual void			Init( S32 entityNum, F32 mass, idVec3 origin, idVec3 angle, idVec3& mins, idVec3& maxs );
    virtual void			InitFromModel( StringEntry qpath, idVec3 origin, idVec3 angle, S32 entityNum, F32 mass );
    virtual void			CreateCapsuleShape( idVec3& mins, idVec3& maxs );
    virtual void			SetTransform( idVec3& origin, idVec3& angle );
    virtual S32				GetEntityNum()
    {
        return entityNum;
    };
    virtual void			GetTransform( idVec3& xyz, idVec3& angle );
    virtual void			SetVelocity( idVec3& vel );
    virtual F32			GetInverseMass( void );
    virtual void			PhysicsApplyCentralImpulse( idVec3& direction );
    virtual void			GetUpVectorBasis( idVec3& upVector );
    virtual void			SetPhysicsActive( void );
public:
    void Free( void );
    btRigidBody* GetBody()
    {
        return body;
    };
private:
    void CreateLocalInertia( F32 mass, btVector3& localInertia );
    void CreateInitialTransform( idVec3& origin, idVec3& angle, F32 mass, btVector3& localInertia );
    void CreateGhostObject( void );
#if !defined ( DEDICATED ) && !defined  ( Q3MAP2 )
    void InitCMFromMDMayaModel( model_t* model );
    void InitCMFromBrushModel( idCollisionShape_t* shape );
#endif
    
    btCollisionShape*	collisionShape;
    btGhostObject*		ghostObject;
    btRigidBody*			body;
    S32	entityNum;
};

//
// owPhysicsCollisionDetection
//
class owPhysicsCollisionDetection
{
public:
    void		Init( void );
    void		Shutdown( void );
    
    void		CreateCollisionModelFromBSP( clipMap_t* collisionModel );
    void		CreateCollisionModelFromAbstractBSP( idPhysicsAbstractBSP* collisionModel );
    
    idTraceModel* AllocTraceModel();
    idTraceModel* GetTraceModelForEntity( S32 entityNum );
    
    void	addConvexVerticesCollider( btAlignedObjectArray<btVector3>& vertices, bool isEntity, const btVector3& entityTargetLocation );
    void	addConvexShapeToCompound( btCompoundShape* compoundShape, btAlignedObjectArray<btVector3>& vertices );
    void	TraceModel( trace_t* results, idTraceModel* model, const idVec3& start, const idVec3& end, S32 mask );
    btRigidBody*	CreateRigidBody( F32 mass, const btTransform& startTransform, btCollisionShape* shape );
    
    //
    // RegisterCollisionShape
    //
    void RegisterCollisionShape( btCollisionShape* shape )
    {
        collisionShapes.push_back( shape );
    }
    
    //
    // GetCollisionDispatcher
    //
    btCollisionDispatcher*	GetCollisionDispatcher( void )
    {
        return dispatcher;
    }
    
    //
    // GetCollisionConfiguration
    //
    btDefaultCollisionConfiguration* GetCollisionConfiguration( void )
    {
        return collisionConfiguration;
    }
protected:
    btCollisionDispatcher*	dispatcher;
    btDefaultCollisionConfiguration* collisionConfiguration;
    btAlignedObjectArray<btCollisionShape*>	collisionShapes;
    btAlignedObjectArray<idTraceModelLocal*> traceModels;
};

#endif //!__PHYSICS_COLLISION_H__
