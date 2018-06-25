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
// File name:   physics_collision.cpp
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#include <OWLIb/precompiled.h>

/*
==============
owPhysicsCollisionDetection::Init
==============
*/
void owPhysicsCollisionDetection::Init( void )
{
    collisionConfiguration = new btDefaultCollisionConfiguration();
    dispatcher = new btCollisionDispatcher( collisionConfiguration );
}

/*
==============
owPhysicsCollisionDetection::Shutdown
==============
*/
void owPhysicsCollisionDetection::Shutdown( void )
{
    //remove the rigidbodies from the dynamics world and delete them
    S32 i;
    for( i = physicsManagerLocal.GetWorldDynamics()->getNumCollisionObjects() - 1; i >= 0 ; i-- )
    {
        btCollisionObject* obj = physicsManagerLocal.GetWorldDynamics()->getCollisionObjectArray()[i];
        btRigidBody* body = btRigidBody::upcast( obj );
        if( body && body->getMotionState() )
        {
            delete body->getMotionState();
        }
        physicsManagerLocal.GetWorldDynamics()->removeCollisionObject( obj );
        delete obj;
    }
    
    for( S32 j = 0; j < traceModels.size(); j++ )
    {
        traceModels[j]->Free();
        delete traceModels[j];
    }
    
    traceModels.clear();
    
    delete collisionConfiguration;
    
    //delete dispatcher
    delete dispatcher;
}

/*
==============
owPhysicsCollisionDetection::AllocTraceModel
==============
*/
idTraceModel* owPhysicsCollisionDetection::AllocTraceModel()
{
    idTraceModelLocal* traceModel = new idTraceModelLocal();
    
    traceModels.push_back( traceModel );
    return traceModels[traceModels.size() - 1];
}

/*
==============
owPhysicsCollisionDetection::GetTraceModelForEntity
==============
*/
idTraceModel* owPhysicsCollisionDetection::GetTraceModelForEntity( S32 entityNum )
{
    for( S32 i = 0; i < traceModels.size(); i++ )
    {
        if( traceModels[i]->GetEntityNum() == entityNum )
        {
            return traceModels[i];
        }
    }
    return NULL;
}

class ClosestNotMe : public btCollisionWorld::ClosestRayResultCallback
{
public:
    ClosestNotMe( btRigidBody* me ) : btCollisionWorld::ClosestRayResultCallback( btVector3( 0.0, 0.0, 0.0 ), btVector3( 0.0, 0.0, 0.0 ) )
    {
        m_me = me;
    }
    
    virtual btScalar addSingleResult( btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace )
    {
        if( rayResult.m_collisionObject == m_me )
            return 1.0;
            
        return ClosestRayResultCallback::addSingleResult( rayResult, normalInWorldSpace
                                                        );
    }
protected:
    btRigidBody* m_me;
};

/*
==============
owPhysicsCollisionDetection::TraceModel
==============
*/
void owPhysicsCollisionDetection::TraceModel( trace_t* results, idTraceModel* model, const idVec3& start, const idVec3& end, S32 mask )
{
    ClosestNotMe rayCallback( ( ( idTraceModelLocal* )model )->GetBody() );
    
    S32 i = 0;
    
    rayCallback.m_closestHitFraction = 1.0;
    physicsManagerLocal.GetWorldDynamics()->rayTest( btVector3( start[0], start[1], start[2] ), btVector3( end[0], end[1], end[2] ), rayCallback );
    
    for( i = 0; i < 3; i++ )
    {
        if( rayCallback.hasHit() )
        {
            results->fractionreal[i] = rayCallback.m_closestHitFraction;
        }
        else
        {
            results->fractionreal[i] = 1.0;
        }
    }
}

/*
==============
addConvexShapeToCompound
==============
*/
void owPhysicsCollisionDetection::addConvexShapeToCompound( btCompoundShape* compoundShape, btAlignedObjectArray<btVector3>& vertices )
{
    //perhaps we can do something special with entities (isEntity)
    //like adding a collision Triggering (as example)
    
    if( vertices.size() <= 0 )
    {
#ifndef Q3MAP2
        Com_Error( ERR_FATAL, "CM_AddConvexShapeToCompound: Vertex buffer zero. \n" );
#endif
    }
    btCollisionShape* shape = new btConvexHullShape( &( vertices[0].getX() ), vertices.size() );
    
    btTransform transform;
    transform.setIdentity();
    
    compoundShape->addChildShape( transform, shape );
}


/*
==============
addConvexVerticesCollider
==============
*/
void owPhysicsCollisionDetection::addConvexVerticesCollider( btAlignedObjectArray<btVector3>& vertices, bool isEntity, const btVector3& entityTargetLocation )
{
    //perhaps we can do something special with entities (isEntity)
    //like adding a collision Triggering (as example)
    
    if( vertices.size() > 0 )
    {
        F32 mass = 0.f;
        btTransform startTransform;
        //can use a shift
        startTransform.setIdentity();
        startTransform.setOrigin( btVector3( 0, 0, 0.0f ) );
        //this create an internal copy of the vertices
        
        btCollisionShape* shape = new btConvexHullShape( &( vertices[0].getX() ), vertices.size() );
        collisionShapes.push_back( shape );
        
        //btRigidBody* body = m_demoApp->localCreateRigidBody(mass, startTransform,shape);
        CreateRigidBody( mass, startTransform, shape );
    }
}

/*
==============
CreateRigidBody
==============
*/
btRigidBody*	owPhysicsCollisionDetection::CreateRigidBody( F32 mass, const btTransform& startTransform, btCollisionShape* shape )
{
    btAssert( ( !shape || shape->getShapeType() != INVALID_SHAPE_PROXYTYPE ) );
    
    //rigidbody is dynamic if and only if mass is non zero, otherwise static
    bool isDynamic = ( mass != 0.f );
    
    btVector3 localInertia( 0, 0, 0 );
    if( isDynamic )
        shape->calculateLocalInertia( mass, localInertia );
        
    //using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
#define USE_MOTIONSTATE 1
#ifdef USE_MOTIONSTATE
    btDefaultMotionState* myMotionState = new btDefaultMotionState( startTransform );
    
    btRigidBody::btRigidBodyConstructionInfo cInfo( mass, myMotionState, shape, localInertia );
    
    btRigidBody* body = new btRigidBody( cInfo );
    body->setContactProcessingThreshold( physicsManagerLocal.GetDefaultContactProcessingThreshold() );
    
#else
    btRigidBody* body = new btRigidBody( mass, 0, shape, localInertia );
    body->setWorldTransform( startTransform );
#endif
    
    physicsManagerLocal.GetWorldDynamics()->addRigidBody( body );
    
    return body;
}

/*
==============
owPhysicsCollisionDetection::CreateCollisionModelFromAbstractBSP
==============
*/
void owPhysicsCollisionDetection::CreateCollisionModelFromAbstractBSP( idPhysicsAbstractBSP* collisionModel )
{
    F32 scaling = 1.0f;
    
    for( S32 i = 0; i < collisionModel->numLeafs; i++ )
    {
        bool isValidBrush = false;
        
        bspLeaf_t&	leaf = collisionModel->leafs[i];
        
        for( S32 b = 0; b < leaf.numBSPLeafBrushes; b++ )
        {
            btAlignedObjectArray<btVector3> planeEquations;
            
            S32 brushid = collisionModel->leafBrushes[leaf.firstBSPLeafBrush + b];
            
            bspBrush_t& brush = collisionModel->brushes[brushid];
            if( brush.shaderNum != -1 )
            {
                if( collisionModel->shaders[brush.shaderNum].contentFlags & CONTENTS_SOLID )
                {
                    brush.shaderNum = -1;
                    
                    for( S32 p = 0; p < brush.numSides; p++ )
                    {
                        bspBrushSide_t& brushside = collisionModel->brushSides[brush.firstSide + p];
                        bspPlane_t* plane = &collisionModel->planes[brushside.planeNum];
                        
                        btVector3 planeEq;
                        planeEq.setValue( plane->normal[0], plane->normal[1], plane->normal[2] );
                        planeEq[3] = scaling * -plane->dist;
                        
                        planeEquations.push_back( planeEq );
                        isValidBrush = true;
                    }
                    if( isValidBrush )
                    {
                        btAlignedObjectArray<btVector3>	vertices;
                        btGeometryUtil::getVerticesFromPlaneEquations( planeEquations, vertices );
                        
                        bool isEntity = false;
                        btVector3 entityTarget( 0.f, 0.f, 0.f );
                        addConvexVerticesCollider( vertices, isEntity, entityTarget );
                    }
                }
            }
        }
    }
}

/*
==============
owPhysicsCollisionDetection::CreateCollisionModelFromBSP
==============
*/
void owPhysicsCollisionDetection::CreateCollisionModelFromBSP( clipMap_t* collisionModel )
{
    F32 scaling = 1.0f;
    
    for( S32 i = 0; i < collisionModel->numLeafs; i++ )
    {
        bool isValidBrush = false;
        
        cLeaf_t&	leaf = collisionModel->leafs[i];
        
        for( S32 b = 0; b < leaf.numLeafBrushes; b++ )
        {
            btAlignedObjectArray<btVector3> planeEquations;
            
            S32 brushid = collisionModel->leafbrushes[leaf.firstLeafBrush + b];
            
            cbrush_t& brush = collisionModel->brushes[brushid];
            if( brush.shaderNum != -1 && brush.physicsprocessed == false )
            {
                if( collisionModel->shaders[brush.shaderNum].contentFlags & CONTENTS_SOLID )
                {
                    brush.physicsprocessed = true;
                    
                    for( S32 p = 0; p < brush.numsides; p++ )
                    {
                        cbrushside_t& brushside = brush.sides[p];
                        
                        btVector3 planeEq;
                        planeEq.setValue( brushside.plane->normal[0], brushside.plane->normal[1], brushside.plane->normal[2] );
                        planeEq[3] = scaling * -brushside.plane->dist;
                        
                        planeEquations.push_back( planeEq );
                        isValidBrush = true;
                    }
                    if( isValidBrush )
                    {
                        btAlignedObjectArray<btVector3>	vertices;
                        btGeometryUtil::getVerticesFromPlaneEquations( planeEquations, vertices );
                        
                        bool isEntity = false;
                        btVector3 entityTarget( 0.f, 0.f, 0.f );
                        addConvexVerticesCollider( vertices, isEntity, entityTarget );
                    }
                }
            }
        }
    }
}

