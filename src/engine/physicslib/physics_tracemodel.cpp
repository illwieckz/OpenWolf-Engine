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
// File name:   physics_tracemodel.cpp
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#include <OWLIb/precompiled.h>

/*
==============
idTraceModelLocal::Init
==============
*/
void idTraceModelLocal::Init( S32 entityNum, F32 mass, idVec3 origin, idVec3 angle, idVec3& mins, idVec3& maxs )
{
    btVector3 localInertia( 0, 0, 0 );
    
    // Assign the current num for the trace model.
    this->entityNum = entityNum;
    
    if( collisionShape == NULL )
    {
        // All traces will use a capsule trace, will this be a problem?
        CreateCapsuleShape( mins, maxs );
    }
    
    // Setup the local inertia.
    CreateLocalInertia( mass, localInertia );
    
    // Create the start transform with the rigidbody physics.
    CreateInitialTransform( origin, angle, mass, localInertia );
    
    // Create the ghost object for fast iteration over what we hit.
    //CreateGhostObject();
    
    // Only do CCD if  motion in one timestep (1.f/60.f) exceeds CUBE_HALF_EXTENTS
    body->setCcdMotionThreshold( 30 );
    
    //Experimental: better estimation of CCD Time of Impact:
    body->setCcdSweptSphereRadius( 0.2 * 0.5f );
}

/*
==============
InitCMFromMDMayaModel
==============
*/
#if 0 //!defined ( Q3MAP2 ) && !defined ( DEDICATED ) && !defined ( UPDATE_SERVER )
void idTraceModelLocal::InitCMFromMDMayaModel( model_t* model )
{
    btCompoundShape* compoundShape = new btCompoundShape();
    S32 numSurfaces = R_GetModelSurfaceCount( model );
    btAlignedObjectArray<btVector3> vertexpool;
    
    for( S32 i = 0; i < numSurfaces; i++ )
    {
        mdMayaVertex_t* vertex;
        mdMayaSurface_t* surface;
        
        surface = ( mdMayaSurface_t* )R_GetSurfaceForModel( model, i );
        vertex = ( mdMayaVertex_t* )R_GetVertexesForModel( model, surface->startVertex );
        
        for( S32 c = 0; c < surface->numVertexes; c++, vertex++ )
        {
            vertexpool.push_back( btVector3( vertex->xyz[0], vertex->xyz[1], vertex->xyz[2] ) );
        }
        
        physicsManagerLocal.GetCollisionManager()->addConvexShapeToCompound( compoundShape, vertexpool );
        
        vertexpool.clear();
    }
    
    collisionShape = compoundShape;
}
#endif

/*
==============
idTraceModelLocal::InitCMFromBrushModel
==============
*/
#if !defined ( Q3MAP2 ) && !defined ( DEDICATED ) && !defined ( UPDATE_SERVER )
void idTraceModelLocal::InitCMFromBrushModel( idCollisionShape_t* shape )
{
    btAlignedObjectArray<btVector3> vertexpool;
    btCompoundShape* compoundShape = new btCompoundShape();
    
    for( S32 i = 0; i < shape->numVertexes; i++ )
    {
        idVec3 xyz = shape->xyz[i];
        vertexpool.push_back( btVector3( xyz.x, xyz.y, xyz.z ) );
    }
    
    physicsManagerLocal.GetCollisionManager()->addConvexShapeToCompound( compoundShape, vertexpool );
    collisionShape = compoundShape;
}
#endif


/*
==============
InitFromModel::InitFromModel
==============
*/
void idTraceModelLocal::InitFromModel( StringEntry qpath, idVec3 origin, idVec3 angle, S32 entityNum, F32 mass )
{
#if 0 //!defined ( Q3MAP2 ) && !defined ( DEDICATED )
    model_t model;
    void* buffer;
    S32 bufferlen, ident;
    
    // Check to see if this is a brush model.
    if( qpath[0] == '*' )
    {
        qpath++;
        S32 brushNum = atoi( qpath );
        idCollisionShape_t* cmshape = ( idCollisionShape_t* )collisionModelManager->GetBrushModelVertexes( brushNum );
        
        InitCMFromBrushModel( cmshape );
    }
    else if( strstr( qpath, ".modelcm" ) )
    {
        idCollisionModel* cm = new idCollisionModel();
        
        // Open the model for reading.
        bufferlen = FS_ReadFile( qpath, &buffer );
        if( bufferlen <= 0 || buffer == NULL )
        {
            Com_Error( ERR_FATAL, "idTraceModelLocal::InitFromModel: Failed to open %s\n", qpath );
            return;
        }
        
        cm->Init( ( cmHeader_t* )buffer );
        idCollisionShape_t* cmshape = ( idCollisionShape_t* )cm->GetBrushShape( 1 );
        
        InitCMFromBrushModel( cmshape );
        
        // Free the buffer.
        FS_FreeFile( buffer );
        
        delete cm;
    }
    else
    {
        // Open the model for reading.
        bufferlen = FS_ReadFile( qpath, &buffer );
        if( bufferlen <= 0 || buffer == NULL )
        {
            Com_Error( ERR_FATAL, "idTraceModelLocal::InitFromModel: Failed to open %s\n", qpath );
            return;
        }
        
        // Check and see what kind of model this is.
        ident = ( S32 )LittleLong( buffer );
        if( ident == MDMAYA_IDENT )
        {
            // Load in the MDV model.
            if( !R_LoadMDMaya( &model, ( U8* )buffer, bufferlen, qpath ) )
            {
                Com_Error( ERR_FATAL, "idTraceModelLocal::InitFromModel: %s failed to load. \n", qpath );
            }
            
            // Create the collision model.
            InitCMFromMDMayaModel( &model );
            
            // Free the memory used by the model since we don't need it anymore.
            Hunk_FreeTempMemory( model.mdMaya );
        }
        else
        {
            Com_Error( ERR_FATAL, "idTraceModelLocal::InitFromModel: Invalid iden - %s is an unsupported model type.\n", qpath );
            return;
        }
        
        // Free the buffer.
        FS_FreeFile( buffer );
    }
    
    // Finish creating the collison model.
    idVec3 cm_zero;
    cm_zero.Zero();
    Init( entityNum, mass, origin, angle, cm_zero, cm_zero );
#endif
}


/*
==============
idTraceModelLocal::SetPhysicsActive
==============
*/
void idTraceModelLocal::SetPhysicsActive( void )
{
    body->forceActivationState( ACTIVE_TAG );
    body->activate();
    body->setDeactivationTime( 0 );
    
    physicsManagerLocal.GetWorldDynamics()->getBroadphase()->getOverlappingPairCache()->cleanProxyFromPairs( body->getBroadphaseHandle(), physicsManagerLocal.GetWorldDynamics()->getDispatcher() );
    
    if( body && !body->isStaticObject() )
    {
        body->setLinearVelocity( btVector3( 0, 0, 0 ) );
        body->setAngularVelocity( btVector3( 0, 0, 0 ) );
    }
}

/*
==============
idTraceModelLocal::GetInverseMass
==============
*/
F32 idTraceModelLocal::GetInverseMass( void )
{
    return body->getInvMass();
}

/*
==============
idTraceModelLocal::PhysicsApplyCentralImpulse
==============
*/
void idTraceModelLocal::PhysicsApplyCentralImpulse( idVec3& direction )
{
    body->applyCentralImpulse( btVector3( direction.x, direction.y, direction.z ) );
}

/*
==============
idTraceModelLocal::GetUpVectorBasis
==============
*/
void idTraceModelLocal::GetUpVectorBasis( idVec3& upVector )
{
    btTransform xform;
    body->getMotionState()->getWorldTransform( xform );
    btVector3 up = xform.getBasis()[2];
    
    upVector.x = up[0];
    upVector.y = up[1];
    upVector.z = up[2];
}

/*
==============
idTraceModelLocal::SetTransform
==============
*/
void idTraceModelLocal::SetTransform( idVec3& origin, idVec3& angle )
{
#ifndef Q3MAP2
    btTransform transform;
    idVec4 matrix[4];
    
    TransformToMatrix( origin, angle, matrix );
    transform.setFromOpenGLMatrix( &matrix[0][0] );
    
    body->getMotionState()->setWorldTransform( transform );
    body->setCenterOfMassTransform( transform );
    //ghostObject->setWorldTransform( transform );
#endif
}

/*
==============
idTraceModelLocal::CreateInitialTransform
==============
*/
void idTraceModelLocal::CreateInitialTransform( idVec3& origin, idVec3& angle, F32 mass, btVector3& localInertia )
{
#ifndef Q3MAP2
    btTransform startTransform;
    startTransform.setIdentity();
    idVec4 matrix[4];
    
    TransformToMatrix( origin, angle, matrix );
    
    startTransform.setFromOpenGLMatrix( &matrix[0][0] );
    
    btDefaultMotionState* myMotionState = new btDefaultMotionState( startTransform );
    btRigidBody::btRigidBodyConstructionInfo rbInfo( mass, myMotionState, collisionShape, localInertia );
    body = new btRigidBody( rbInfo );
    body->setActivationState( ACTIVE_TAG );
    physicsManagerLocal.GetWorldDynamics()->addRigidBody( body );
    body->setActivationState( ACTIVE_TAG );
    
    body->setSleepingThresholds( 0.0, 0.0 );
    body->setAngularFactor( 0.0 );
    
    // Only do CCD if  motion in one timestep (1.f/60.f) exceeds CUBE_HALF_EXTENTS
    body->setCcdMotionThreshold( 30 );
    
    //Experimental: better estimation of CCD Time of Impact:
    body->setCcdSweptSphereRadius( 0.2 * 0.5f );
#endif
}


/*
==============
idTraceModelLocal::CreateInitialTransform
==============
*/
void idTraceModelLocal::SetVelocity( idVec3& vel )
{
    body->setLinearVelocity( btVector3( vel.x, vel.y, vel.z ) );
}

/*
==============
idTraceModelLocal::CreateGhostObject
==============
*/
void idTraceModelLocal::CreateGhostObject( void )
{
    ghostObject = new btGhostObject();
    ghostObject->setCollisionShape( collisionShape );
    ghostObject->setCollisionFlags( btCollisionObject::CF_NO_CONTACT_RESPONSE );
    
    physicsManagerLocal.GetWorldDynamics()->addCollisionObject( ghostObject, btBroadphaseProxy::SensorTrigger, btBroadphaseProxy::AllFilter & ~btBroadphaseProxy::SensorTrigger );
}

void toAngles( btMatrix3x3& src, idVec3& dst )
{
    F64 theta;
    F64 cp;
    F64 sp;
    
    
    sp = src[0][2];
    
    // cap off our sin value so that we don't get any NANs
    if( sp > 1.0 )
    {
        sp = 1.0;
    }
    else if( sp < -1.0 )
    {
        sp = -1.0;
    }
    
    theta = -asin( sp );
    cp = cos( theta );
    
    if( cp > 8192 * FLT_EPSILON )
    {
        dst[PITCH] = theta * 180 / M_PI;
        dst[YAW] = atan2( src[0][1], src[0][0] ) * 180 / M_PI;
        dst[ROLL] = atan2( src[1][2], src[2][2] ) * 180 / M_PI;
    }
    else
    {
        dst[PITCH] = theta * 180 / M_PI;
        dst[YAW] = -atan2( src[1][0], src[1][1] ) * 180 / M_PI;
        dst[ROLL] = 0;
    }
}

/*
==============
idTraceModelLocal::GetTransform
==============
*/
void idTraceModelLocal::GetTransform( idVec3& origin, idVec3& angle )
{
    btTransform transform;
    
    body->getMotionState()->getWorldTransform( transform );
    btVector3 xyz = transform.getOrigin();
    btMatrix3x3 matrix = transform.getBasis();
    
    toAngles( matrix, angle );
    
    origin.set( xyz[0], xyz[1], xyz[2] );
}

/*
==============
idTraceModelLocal::CreateLocalInertia
==============
*/
void idTraceModelLocal::CreateLocalInertia( F32 mass, btVector3& localInertia )
{
    if( mass == 0.0f )
    {
        return;
    }
    collisionShape->calculateLocalInertia( mass, localInertia );
}

/*
==============
CreateCapsuleShape
==============
*/
//#define USE_CAPSULE_TRACE 1
void idTraceModelLocal::CreateCapsuleShape( idVec3& mins, idVec3& maxs )
{
    btBoxShape* aabb[2];
    
    aabb[0] = new btBoxShape( btVector3( mins.x, mins.y, mins.z ) );
    aabb[1] = new btBoxShape( btVector3( maxs.x, maxs.y, maxs.z ) );
    
    //btCompoundShape::addChildShape
    btTransform transform;
    transform.setIdentity();
    
    if( mins.z != 0 )
    {
        transform.setOrigin( btVector3( 0, 0, maxs.z + mins.z ) );
    }
    else
    {
        transform.setOrigin( btVector3( 0, 0, 0 ) );
    }
    
    btCompoundShape* compound = new btCompoundShape();
    compound->addChildShape( transform, aabb[0] );
    compound->addChildShape( transform, aabb[1] );
#if USE_CAPSULE_TRACE
    btVector3 sphereCenter;
    btScalar sphereRadius;
    compound->getBoundingSphere( sphereCenter, sphereRadius );
    
    collisionShape = new btMultiSphereShape( &sphereCenter, &sphereRadius, 1 );
    collisionShape->setMargin( 0.05f );
#else
    collisionShape = compound;
#endif
    physicsManagerLocal.GetCollisionManager()->RegisterCollisionShape( collisionShape );
    
#if USE_CAPSULE_TRACE
    // Wow this isn't ugly at all :/.
    delete compound;
    delete aabb[0];
    delete aabb[1];
    compound = NULL;
#endif
}

/*
==============
idTraceModelLocal::Free
==============
*/
void idTraceModelLocal::Free( void )
{
    delete collisionShape;
}