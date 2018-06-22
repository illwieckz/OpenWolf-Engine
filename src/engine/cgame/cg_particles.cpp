////////////////////////////////////////////////////////////////////////////////////////
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
// File name:   cg_particles.cpp
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description: the particle system
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#include <cgame/cg_precompiled.h>

static baseParticleSystem_t   baseParticleSystems[ MAX_BASEPARTICLE_SYSTEMS ];
static baseParticleEjector_t  baseParticleEjectors[ MAX_BASEPARTICLE_EJECTORS ];
static baseParticle_t         baseParticles[ MAX_BASEPARTICLES ];
static S32                    numBaseParticleSystems = 0;
static S32                    numBaseParticleEjectors = 0;
static S32                    numBaseParticles = 0;

static particleSystem_t     particleSystems[ MAX_PARTICLE_SYSTEMS ];
static particleEjector_t    particleEjectors[ MAX_PARTICLE_EJECTORS ];
static particle_t           particles[ MAX_PARTICLES ];
static particle_t*           sortedParticles[ MAX_PARTICLES ];
static particle_t*           radixBuffer[ MAX_PARTICLES ];

/*
===============
CG_LerpValues

Lerp between two values
===============
*/
F32 idCGameLocal::LerpValues( F32 a, F32 b, F32 f )
{
    if( b == PARTICLES_SAME_AS_INITIAL )
        return a;
    else
        return ( ( a ) + ( f ) * ( ( b ) - ( a ) ) );
}

/*
===============
CG_RandomiseValue

Randomise some value by some variance
===============
*/
F32 idCGameLocal::RandomiseValue( F32 value, F32 variance )
{
    if( value != 0.0f )
        return value * ( 1.0f + ( random( ) * variance ) );
    else
        return random( ) * variance;
}

/*
===============
CG_SpreadVector

Randomly spread a vector by some amount
===============
*/
void idCGameLocal::SpreadVector( vec3_t v, F32 spread )
{
    vec3_t  p, r1, r2;
    F32 randomSpread = crandom( ) * spread;
    F32 randomRotation = random( ) * 360.0f;
    
    PerpendicularVector( p, v );
    
    RotatePointAroundVector( r1, p, v, randomSpread );
    RotatePointAroundVector( r2, v, r1, randomRotation );
    
    VectorCopy( r2, v );
}

/*
===============
CG_DestroyParticle

Destroy an individual particle
===============
*/
void idCGameLocal::DestroyParticle( particle_t* p, vec3_t impactNormal )
{
    //this particle has an onDeath particle system attached
    if( p->_class->onDeathSystemName[ 0 ] != '\0' )
    {
        particleSystem_t*  ps;
        
        ps = SpawnNewParticleSystem( p->_class->onDeathSystemHandle );
        
        if( IsParticleSystemValid( &ps ) )
        {
            if( impactNormal )
                SetParticleSystemNormal( ps, impactNormal );
                
            SetAttachmentPoint( &ps->attachment, p->origin );
            AttachToPoint( &ps->attachment );
        }
    }
    
    p->valid = false;
    
    //this gives other systems a couple of
    //frames to realise the particle is gone
    p->frameWhenInvalidated = cg.clientFrame;
}

/*
===============
CG_SpawnNewParticle

Introduce a new particle into the world
===============
*/
particle_t* idCGameLocal::SpawnNewParticle( baseParticle_t* bp, particleEjector_t* parent )
{
    S32                     i, j;
    particle_t*              p = NULL;
    particleEjector_t*       pe = parent;
    particleSystem_t*        ps = parent->parent;
    vec3_t                  attachmentPoint, attachmentVelocity;
    vec3_t                  transform[ 3 ];
    
    for( i = 0; i < MAX_PARTICLES; i++ )
    {
        p = &particles[ i ];
        
        //FIXME: the + 1 may be unnecessary
        if( !p->valid && cg.clientFrame > p->frameWhenInvalidated + 1 )
        {
            ::memset( p, 0, sizeof( particle_t ) );
            
            //found a free slot
            p->_class = bp;
            p->parent = pe;
            
            p->birthTime = cg.time;
            p->lifeTime = ( S32 )RandomiseValue( ( F32 )bp->lifeTime, bp->lifeTimeRandFrac );
            
            p->radius.delay = ( S32 )RandomiseValue( ( F32 )bp->radius.delay, bp->radius.delayRandFrac );
            p->radius.initial = RandomiseValue( bp->radius.initial, bp->radius.initialRandFrac );
            p->radius.final = RandomiseValue( bp->radius.final, bp->radius.finalRandFrac );
            
            p->radius.initial += bp->scaleWithCharge * pe->parent->charge;
            
            p->alpha.delay = ( S32 )RandomiseValue( ( F32 )bp->alpha.delay, bp->alpha.delayRandFrac );
            p->alpha.initial = RandomiseValue( bp->alpha.initial, bp->alpha.initialRandFrac );
            p->alpha.final = RandomiseValue( bp->alpha.final, bp->alpha.finalRandFrac );
            
            p->rotation.delay = ( S32 )RandomiseValue( ( F32 )bp->rotation.delay, bp->rotation.delayRandFrac );
            p->rotation.initial = RandomiseValue( bp->rotation.initial, bp->rotation.initialRandFrac );
            p->rotation.final = RandomiseValue( bp->rotation.final, bp->rotation.finalRandFrac );
            
            p->dLightRadius.delay = ( S32 )RandomiseValue( ( F32 )bp->dLightRadius.delay, bp->dLightRadius.delayRandFrac );
            p->dLightRadius.initial = RandomiseValue( bp->dLightRadius.initial, bp->dLightRadius.initialRandFrac );
            p->dLightRadius.final = RandomiseValue( bp->dLightRadius.final, bp->dLightRadius.finalRandFrac );
            
            p->colorDelay = RandomiseValue( bp->colorDelay, bp->colorDelayRandFrac );
            
            p->bounceMarkRadius = RandomiseValue( bp->bounceMarkRadius, bp->bounceMarkRadiusRandFrac );
            p->bounceMarkCount = rint( RandomiseValue( ( F32 )bp->bounceMarkCount, bp->bounceMarkCountRandFrac ) );
            p->bounceSoundCount = rint( RandomiseValue( ( F32 )bp->bounceSoundCount, bp->bounceSoundCountRandFrac ) );
            
            if( bp->numModels )
            {
                p->model = bp->models[ rand( ) % bp->numModels ];
                
                if( bp->modelAnimation.frameLerp < 0 )
                {
                    bp->modelAnimation.frameLerp = p->lifeTime / bp->modelAnimation.numFrames;
                    bp->modelAnimation.initialLerp = p->lifeTime / bp->modelAnimation.numFrames;
                }
            }
            
            if( !AttachmentPoint( &ps->attachment, attachmentPoint ) )
                return NULL;
                
            VectorCopy( attachmentPoint, p->origin );
            
            if( AttachmentAxis( &ps->attachment, transform ) )
            {
                vec3_t  transDisplacement;
                
                VectorMatrixMultiply( bp->displacement, transform, transDisplacement );
                VectorAdd( p->origin, transDisplacement, p->origin );
            }
            else
                VectorAdd( p->origin, bp->displacement, p->origin );
                
            for( j = 0; j <= 2; j++ )
                p->origin[ j ] += ( crandom( ) * bp->randDisplacement );
                
            switch( bp->velMoveType )
            {
                case PMT_STATIC:
                    if( bp->velMoveValues.dirType == PMD_POINT )
                        VectorSubtract( bp->velMoveValues.point, p->origin, p->velocity );
                    else if( bp->velMoveValues.dirType == PMD_LINEAR )
                        VectorCopy( bp->velMoveValues.dir, p->velocity );
                    break;
                    
                case PMT_STATIC_TRANSFORM:
                    if( !AttachmentAxis( &ps->attachment, transform ) )
                        return NULL;
                        
                    if( bp->velMoveValues.dirType == PMD_POINT )
                    {
                        vec3_t transPoint;
                        
                        VectorMatrixMultiply( bp->velMoveValues.point, transform, transPoint );
                        VectorSubtract( transPoint, p->origin, p->velocity );
                    }
                    else if( bp->velMoveValues.dirType == PMD_LINEAR )
                        VectorMatrixMultiply( bp->velMoveValues.dir, transform, p->velocity );
                    break;
                    
                case PMT_TAG:
                case PMT_CENT_ANGLES:
                    if( bp->velMoveValues.dirType == PMD_POINT )
                        VectorSubtract( attachmentPoint, p->origin, p->velocity );
                    else if( bp->velMoveValues.dirType == PMD_LINEAR )
                    {
                        if( !AttachmentDir( &ps->attachment, p->velocity ) )
                            return NULL;
                    }
                    break;
                    
                case PMT_NORMAL:
                    if( !ps->normalValid )
                    {
                        Printf( S_COLOR_RED "ERROR: a particle with velocityType normal has no normal\n" );
                        return NULL;
                    }
                    
                    VectorCopy( ps->normal, p->velocity );
                    
                    //normal displacement
                    VectorNormalize( p->velocity );
                    VectorMA( p->origin, bp->normalDisplacement, p->velocity, p->origin );
                    break;
            }
            
            VectorNormalize( p->velocity );
            SpreadVector( p->velocity, bp->velMoveValues.dirRandAngle );
            VectorScale( p->velocity, RandomiseValue( bp->velMoveValues.mag, bp->velMoveValues.magRandFrac ), p->velocity );
            
            if( AttachmentVelocity( &ps->attachment, attachmentVelocity ) )
            {
                VectorMA( p->velocity, RandomiseValue( bp->velMoveValues.parentVelFrac, bp->velMoveValues.parentVelFracRandFrac ), attachmentVelocity, p->velocity );
            }
            
            p->lastEvalTime = cg.time;
            
            p->valid = true;
            
            //this particle has a child particle system attached
            if( bp->childSystemName[ 0 ] != '\0' )
            {
                particleSystem_t*  ps = SpawnNewParticleSystem( bp->childSystemHandle );
                
                if( IsParticleSystemValid( &ps ) )
                {
                    SetAttachmentParticle( &ps->attachment, p );
                    AttachToParticle( &ps->attachment );
                }
            }
            
            //this particle has a child trail system attached
            if( bp->childTrailSystemName[ 0 ] != '\0' )
            {
                trailSystem_t* ts = SpawnNewTrailSystem( bp->childTrailSystemHandle );
                
                if( IsTrailSystemValid( &ts ) )
                {
                    SetAttachmentParticle( &ts->frontAttachment, p );
                    AttachToParticle( &ts->frontAttachment );
                }
            }
            
            break;
        }
    }
    
    return p;
}


/*
===============
CG_SpawnNewParticles

Check if there are any ejectors that should be
introducing new particles
===============
*/
void idCGameLocal::SpawnNewParticles( void )
{
    S32                   i, j;
    particle_t*            p;
    particleSystem_t*      ps;
    particleEjector_t*     pe;
    baseParticleEjector_t* bpe;
    F32                 lerpFrac;
    S32                   count;
    
    for( i = 0; i < MAX_PARTICLE_EJECTORS; i++ )
    {
        pe = &particleEjectors[ i ];
        ps = pe->parent;
        
        if( pe->valid )
        {
            //a non attached particle system can't make particles
            if( !Attached( &ps->attachment ) )
                continue;
                
            bpe = particleEjectors[ i ]._class;
            
            //if this system is scheduled for removal don't make any new particles
            if( !ps->lazyRemove )
            {
                while( pe->nextEjectionTime <= cg.time && ( pe->count > 0 || pe->totalParticles == PARTICLES_INFINITE ) )
                {
                    for( j = 0; j < bpe->numParticles; j++ )
                        SpawnNewParticle( bpe->particles[ j ], pe );
                        
                    if( pe->count > 0 )
                        pe->count--;
                        
                    //calculate next ejection time
                    lerpFrac = 1.0 - ( ( F32 )pe->count / ( F32 )pe->totalParticles );
                    pe->nextEjectionTime = cg.time + ( S32 )RandomiseValue( LerpValues( pe->ejectPeriod.initial, pe->ejectPeriod.final, lerpFrac ), pe->ejectPeriod.randFrac );
                }
            }
            
            if( pe->count == 0 || ps->lazyRemove )
            {
                count = 0;
                
                //wait for child particles to die before declaring this pe invalid
                for( j = 0; j < MAX_PARTICLES; j++ )
                {
                    p = &particles[ j ];
                    
                    if( p->valid && p->parent == pe )
                        count++;
                }
                
                if( !count )
                    pe->valid = false;
            }
        }
    }
}


/*
===============
CG_SpawnNewParticleEjector

Allocate a new particle ejector
===============
*/
particleEjector_t* idCGameLocal::SpawnNewParticleEjector( baseParticleEjector_t* bpe, particleSystem_t* parent )
{
    S32 i;
    particleEjector_t* pe = NULL;
    particleSystem_t* ps = parent;
    
    for( i = 0; i < MAX_PARTICLE_EJECTORS; i++ )
    {
        pe = &particleEjectors[ i ];
        
        if( !pe->valid )
        {
            ::memset( pe, 0, sizeof( particleEjector_t ) );
            
            //found a free slot
            pe->_class = bpe;
            pe->parent = ps;
            
            pe->ejectPeriod.initial = bpe->eject.initial;
            pe->ejectPeriod.final = bpe->eject.final;
            pe->ejectPeriod.randFrac = bpe->eject.randFrac;
            
            pe->nextEjectionTime = cg.time + ( S32 )RandomiseValue( ( F32 )bpe->eject.delay, bpe->eject.delayRandFrac );
            pe->count = pe->totalParticles = ( S32 )rint( RandomiseValue( ( F32 )bpe->totalParticles, bpe->totalParticlesRandFrac ) );
            
            pe->valid = true;
            
            if( cg_debugParticles.integer >= 1 )
                Printf( "PE %s created\n", ps->_class->name );
                
            break;
        }
    }
    
    return pe;
}


/*
===============
CG_SpawnNewParticleSystem

Allocate a new particle system
===============
*/
particleSystem_t* idCGameLocal::SpawnNewParticleSystem( qhandle_t psHandle )
{
    S32                   i, j;
    particleSystem_t*      ps = NULL;
    baseParticleSystem_t*  bps = &baseParticleSystems[ psHandle - 1 ];
    
    if( !bps->registered )
    {
        Printf( S_COLOR_RED "ERROR: a particle system has not been registered yet\n" );
        return NULL;
    }
    
    for( i = 0; i < MAX_PARTICLE_SYSTEMS; i++ )
    {
        ps = &particleSystems[ i ];
        
        if( !ps->valid )
        {
            ::memset( ps, 0, sizeof( particleSystem_t ) );
            
            //found a free slot
            ps->_class = bps;
            
            ps->valid = true;
            ps->lazyRemove = false;
            
            for( j = 0; j < bps->numEjectors; j++ )
                SpawnNewParticleEjector( bps->ejectors[ j ], ps );
                
            if( cg_debugParticles.integer >= 1 )
                Printf( "PS %s created\n", bps->name );
                
            break;
        }
    }
    
    return ps;
}

/*
===============
CG_RegisterParticleSystem

Load the shaders required for a particle system
===============
*/
qhandle_t idCGameLocal::RegisterParticleSystem( UTF8* name )
{
    S32                   i, j, k, l;
    baseParticleSystem_t*  bps;
    baseParticleEjector_t* bpe;
    baseParticle_t*        bp;
    
    for( i = 0; i < MAX_BASEPARTICLE_SYSTEMS; i++ )
    {
        bps = &baseParticleSystems[ i ];
        
        if( !Q_stricmpn( bps->name, name, MAX_QPATH ) )
        {
            //already registered
            if( bps->registered )
                return i + 1;
                
            for( j = 0; j < bps->numEjectors; j++ )
            {
                bpe = bps->ejectors[ j ];
                
                for( l = 0; l < bpe->numParticles; l++ )
                {
                    bp = bpe->particles[ l ];
                    
                    for( k = 0; k < bp->numFrames; k++ )
                        bp->shaders[ k ] = trap_R_RegisterShader( bp->shaderNames[ k ] );
                        
                    for( k = 0; k < bp->numModels; k++ )
                        bp->models[ k ] = trap_R_RegisterModel( bp->modelNames[ k ] );
                        
                    if( bp->bounceMarkName[ 0 ] != '\0' )
                        bp->bounceMark = trap_R_RegisterShader( bp->bounceMarkName );
                        
                    if( bp->bounceSoundName[ 0 ] != '\0' )
                        bp->bounceSound = trap_S_RegisterSound( bp->bounceSoundName, false );
                        
                    //recursively register any children
                    if( bp->childSystemName[ 0 ] != '\0' )
                    {
                        //don't care about a handle for children since
                        //the system deals with it
                        RegisterParticleSystem( bp->childSystemName );
                    }
                    
                    if( bp->onDeathSystemName[ 0 ] != '\0' )
                    {
                        //don't care about a handle for children since
                        //the system deals with it
                        RegisterParticleSystem( bp->onDeathSystemName );
                    }
                    
                    if( bp->childTrailSystemName[ 0 ] != '\0' )
                        bp->childTrailSystemHandle = RegisterTrailSystem( bp->childTrailSystemName );
                }
            }
            
            if( cg_debugParticles.integer >= 1 )
                Printf( "Registered particle system %s\n", name );
                
            bps->registered = true;
            
            //avoid returning 0
            return i + 1;
        }
    }
    
    Printf( S_COLOR_RED "ERROR: failed to register particle system %s\n", name );
    return 0;
}


/*
===============
CG_ParseValueAndVariance

Parse a value and its random variance
===============
*/
void idCGameLocal::ParseValueAndVariance( UTF8* token, F32* value, F32* variance, bool allowNegative )
{
    UTF8  valueBuffer[ 16 ];
    UTF8  varianceBuffer[ 16 ];
    UTF8*  variancePtr = NULL, *varEndPointer = NULL;
    F32 localValue = 0.0f;
    F32 localVariance = 0.0f;
    
    Q_strncpyz( valueBuffer, token, sizeof( valueBuffer ) );
    Q_strncpyz( varianceBuffer, token, sizeof( varianceBuffer ) );
    
    variancePtr = strchr( valueBuffer, '~' );
    
    //variance included
    if( variancePtr )
    {
        variancePtr[ 0 ] = '\0';
        variancePtr++;
        
        localValue = bggame->atof_neg( valueBuffer, allowNegative );
        
        varEndPointer = strchr( variancePtr, '%' );
        
        if( varEndPointer )
        {
            varEndPointer[ 0 ] = '\0';
            localVariance = bggame->atof_neg( variancePtr, false ) / 100.0f;
        }
        else
        {
            if( localValue != 0.0f )
                localVariance = bggame->atof_neg( variancePtr, false ) / localValue;
            else
                localVariance = bggame->atof_neg( variancePtr, false );
        }
    }
    else
        localValue = bggame->atof_neg( valueBuffer, allowNegative );
        
    if( value != NULL )
        *value = localValue;
        
    if( variance != NULL )
        *variance = localVariance;
}

/*
===============
CG_ParseColor
===============
*/
bool idCGameLocal::ParseColor( U8* c, UTF8** text_p )
{
    UTF8*  token;
    S32   i;
    
    for( i = 0; i <= 2; i++ )
    {
        token = COM_Parse( text_p );
        
        if( !Q_stricmp( token, "" ) )
            return false;
            
        c[ i ] = ( S32 )( ( F32 )0xFF * bggame->atof_neg( token, false ) );
    }
    
    return true;
}

/*
===============
CG_ParseParticle

Parse a particle section
===============
*/
bool idCGameLocal::ParseParticle( baseParticle_t* bp, UTF8** text_p )
{
    UTF8* token;
    F32 number, randFrac;
    S32 i;
    
    // read optional parameters
    while( 1 )
    {
        token = COM_Parse( text_p );
        
        if( !token )
            break;
            
        if( !Q_stricmp( token, "" ) )
            return false;
            
        if( !Q_stricmp( token, "bounce" ) )
        {
            token = COM_Parse( text_p );
            if( !token )
                break;
                
            if( !Q_stricmp( token, "cull" ) )
            {
                bp->bounceCull = true;
                
                bp->bounceFrac = -1.0f;
                bp->bounceFracRandFrac = 0.0f;
            }
            else
            {
                ParseValueAndVariance( token, &number, &randFrac, false );
                
                bp->bounceFrac = number;
                bp->bounceFracRandFrac = randFrac;
            }
            
            continue;
        }
        else if( !Q_stricmp( token, "bounceMark" ) )
        {
            token = COM_Parse( text_p );
            if( !*token )
                break;
                
            ParseValueAndVariance( token, &number, &randFrac, false );
            
            bp->bounceMarkCount = number;
            bp->bounceMarkCountRandFrac = randFrac;
            
            token = COM_Parse( text_p );
            if( !*token )
                break;
                
            ParseValueAndVariance( token, &number, &randFrac, false );
            
            bp->bounceMarkRadius = number;
            bp->bounceMarkRadiusRandFrac = randFrac;
            
            token = COM_ParseExt( text_p, false );
            if( !*token )
                break;
                
            Q_strncpyz( bp->bounceMarkName, token, MAX_QPATH );
            
            continue;
        }
        else if( !Q_stricmp( token, "bounceSound" ) )
        {
            token = COM_Parse( text_p );
            if( !*token )
                break;
                
            ParseValueAndVariance( token, &number, &randFrac, false );
            
            bp->bounceSoundCount = number;
            bp->bounceSoundCountRandFrac = randFrac;
            
            token = COM_Parse( text_p );
            if( !*token )
                break;
                
            Q_strncpyz( bp->bounceSoundName, token, MAX_QPATH );
            
            continue;
        }
        else if( !Q_stricmp( token, "shader" ) )
        {
            if( bp->numModels > 0 )
            {
                Printf( S_COLOR_RED "ERROR: 'shader' not allowed in conjunction with 'model'\n", token );
                break;
            }
            
            token = COM_Parse( text_p );
            if( !token )
                break;
                
            if( !Q_stricmp( token, "sync" ) )
                bp->framerate = 0.0f;
            else
                bp->framerate = bggame->atof_neg( token, false );
                
            token = COM_ParseExt( text_p, false );
            if( !*token )
                break;
                
            while( *token && bp->numFrames < MAX_PS_SHADER_FRAMES )
            {
                Q_strncpyz( bp->shaderNames[ bp->numFrames++ ], token, MAX_QPATH );
                token = COM_ParseExt( text_p, false );
            }
            
            continue;
        }
        else if( !Q_stricmp( token, "model" ) )
        {
            if( bp->numFrames > 0 )
            {
                Printf( S_COLOR_RED "ERROR: 'model' not allowed in conjunction with 'shader'\n", token );
                break;
            }
            
            token = COM_ParseExt( text_p, false );
            if( !*token )
                break;
                
            while( *token && bp->numModels < MAX_PS_MODELS )
            {
                Q_strncpyz( bp->modelNames[ bp->numModels++ ], token, MAX_QPATH );
                token = COM_ParseExt( text_p, false );
            }
            
            continue;
        }
        else if( !Q_stricmp( token, "modelAnimation" ) )
        {
            token = COM_Parse( text_p );
            if( !*token )
                break;
                
            bp->modelAnimation.firstFrame = bggame->atoi_neg( token, false );
            
            token = COM_Parse( text_p );
            if( !*token )
                break;
                
            bp->modelAnimation.numFrames = atoi( token );
            bp->modelAnimation.reversed = false;
            bp->modelAnimation.flipflop = false;
            
            // if numFrames is negative the animation is reversed
            if( bp->modelAnimation.numFrames < 0 )
            {
                bp->modelAnimation.numFrames = -bp->modelAnimation.numFrames;
                bp->modelAnimation.reversed = true;
            }
            
            token = COM_Parse( text_p );
            if( !*token )
                break;
                
            bp->modelAnimation.loopFrames = atoi( token );
            
            token = COM_Parse( text_p );
            if( !*token )
                break;
                
            if( !Q_stricmp( token, "sync" ) )
            {
                bp->modelAnimation.frameLerp = -1;
                bp->modelAnimation.initialLerp = -1;
            }
            else
            {
                F32 fps = bggame->atof_neg( token, false );
                
                if( fps == 0.0f )
                    fps = 1.0f;
                    
                bp->modelAnimation.frameLerp = 1000 / fps;
                bp->modelAnimation.initialLerp = 1000 / fps;
            }
            
            continue;
        }
        else if( !Q_stricmp( token, "velocityType" ) )
        {
            token = COM_Parse( text_p );
            if( !token )
                break;
                
            if( !Q_stricmp( token, "static" ) )
                bp->velMoveType = PMT_STATIC;
            else if( !Q_stricmp( token, "static_transform" ) )
                bp->velMoveType = PMT_STATIC_TRANSFORM;
            else if( !Q_stricmp( token, "tag" ) )
                bp->velMoveType = PMT_TAG;
            else if( !Q_stricmp( token, "cent" ) )
                bp->velMoveType = PMT_CENT_ANGLES;
            else if( !Q_stricmp( token, "normal" ) )
                bp->velMoveType = PMT_NORMAL;
                
            continue;
        }
        else if( !Q_stricmp( token, "velocityDir" ) )
        {
            token = COM_Parse( text_p );
            if( !token )
                break;
                
            if( !Q_stricmp( token, "linear" ) )
                bp->velMoveValues.dirType = PMD_LINEAR;
            else if( !Q_stricmp( token, "point" ) )
                bp->velMoveValues.dirType = PMD_POINT;
                
            continue;
        }
        else if( !Q_stricmp( token, "velocityMagnitude" ) )
        {
            token = COM_Parse( text_p );
            if( !token )
                break;
                
            ParseValueAndVariance( token, &number, &randFrac, false );
            
            bp->velMoveValues.mag = number;
            bp->velMoveValues.magRandFrac = randFrac;
            
            continue;
        }
        else if( !Q_stricmp( token, "parentVelocityFraction" ) )
        {
            token = COM_Parse( text_p );
            if( !token )
                break;
                
            ParseValueAndVariance( token, &number, &randFrac, false );
            
            bp->velMoveValues.parentVelFrac = number;
            bp->velMoveValues.parentVelFracRandFrac = randFrac;
            
            continue;
        }
        else if( !Q_stricmp( token, "velocity" ) )
        {
            for( i = 0; i <= 2; i++ )
            {
                token = COM_Parse( text_p );
                if( !token )
                    break;
                    
                bp->velMoveValues.dir[ i ] = bggame->atof_neg( token, true );
            }
            
            token = COM_Parse( text_p );
            if( !token )
                break;
                
            ParseValueAndVariance( token, NULL, &randFrac, false );
            
            bp->velMoveValues.dirRandAngle = randFrac;
            
            continue;
        }
        else if( !Q_stricmp( token, "velocityPoint" ) )
        {
            for( i = 0; i <= 2; i++ )
            {
                token = COM_Parse( text_p );
                if( !token )
                    break;
                    
                bp->velMoveValues.point[ i ] = bggame->atof_neg( token, true );
            }
            
            token = COM_Parse( text_p );
            if( !token )
                break;
                
            ParseValueAndVariance( token, NULL, &randFrac, false );
            
            bp->velMoveValues.pointRandAngle = randFrac;
            
            continue;
        }
        ///
        else if( !Q_stricmp( token, "accelerationType" ) )
        {
            token = COM_Parse( text_p );
            if( !token )
                break;
                
            if( !Q_stricmp( token, "static" ) )
                bp->accMoveType = PMT_STATIC;
            else if( !Q_stricmp( token, "static_transform" ) )
                bp->accMoveType = PMT_STATIC_TRANSFORM;
            else if( !Q_stricmp( token, "tag" ) )
                bp->accMoveType = PMT_TAG;
            else if( !Q_stricmp( token, "cent" ) )
                bp->accMoveType = PMT_CENT_ANGLES;
            else if( !Q_stricmp( token, "normal" ) )
                bp->accMoveType = PMT_NORMAL;
                
            continue;
        }
        else if( !Q_stricmp( token, "accelerationDir" ) )
        {
            token = COM_Parse( text_p );
            if( !token )
                break;
                
            if( !Q_stricmp( token, "linear" ) )
                bp->accMoveValues.dirType = PMD_LINEAR;
            else if( !Q_stricmp( token, "point" ) )
                bp->accMoveValues.dirType = PMD_POINT;
                
            continue;
        }
        else if( !Q_stricmp( token, "accelerationMagnitude" ) )
        {
            token = COM_Parse( text_p );
            if( !token )
                break;
                
            ParseValueAndVariance( token, &number, &randFrac, false );
            
            bp->accMoveValues.mag = number;
            bp->accMoveValues.magRandFrac = randFrac;
            
            continue;
        }
        else if( !Q_stricmp( token, "acceleration" ) )
        {
            for( i = 0; i <= 2; i++ )
            {
                token = COM_Parse( text_p );
                if( !token )
                    break;
                    
                bp->accMoveValues.dir[ i ] = bggame->atof_neg( token, true );
            }
            
            token = COM_Parse( text_p );
            if( !token )
                break;
                
            ParseValueAndVariance( token, NULL, &randFrac, false );
            
            bp->accMoveValues.dirRandAngle = randFrac;
            
            continue;
        }
        else if( !Q_stricmp( token, "accelerationPoint" ) )
        {
            for( i = 0; i <= 2; i++ )
            {
                token = COM_Parse( text_p );
                if( !token )
                    break;
                    
                bp->accMoveValues.point[ i ] = bggame->atof_neg( token, true );
            }
            
            token = COM_Parse( text_p );
            if( !token )
                break;
                
            ParseValueAndVariance( token, NULL, &randFrac, false );
            
            bp->accMoveValues.pointRandAngle = randFrac;
            
            continue;
        }
        ///
        else if( !Q_stricmp( token, "displacement" ) )
        {
            for( i = 0; i <= 2; i++ )
            {
                token = COM_Parse( text_p );
                if( !token )
                    break;
                    
                bp->displacement[ i ] = bggame->atof_neg( token, true );
            }
            
            token = COM_Parse( text_p );
            if( !token )
                break;
                
            ParseValueAndVariance( token, NULL, &randFrac, false );
            
            bp->randDisplacement = randFrac;
            
            continue;
        }
        else if( !Q_stricmp( token, "normalDisplacement" ) )
        {
            token = COM_Parse( text_p );
            if( !token )
                break;
                
            bp->normalDisplacement = bggame->atof_neg( token, true );
            
            continue;
        }
        else if( !Q_stricmp( token, "overdrawProtection" ) )
        {
            bp->overdrawProtection = true;
            
            continue;
        }
        else if( !Q_stricmp( token, "realLight" ) )
        {
            bp->realLight = true;
            
            continue;
        }
        else if( !Q_stricmp( token, "dynamicLight" ) )
        {
            bp->dynamicLight = true;
            
            token = COM_Parse( text_p );
            if( !*token )
                break;
                
            ParseValueAndVariance( token, &number, &randFrac, false );
            
            bp->dLightRadius.delay = ( S32 )number;
            bp->dLightRadius.delayRandFrac = randFrac;
            
            token = COM_Parse( text_p );
            if( !*token )
                break;
                
            ParseValueAndVariance( token, &number, &randFrac, false );
            
            bp->dLightRadius.initial = number;
            bp->dLightRadius.initialRandFrac = randFrac;
            
            token = COM_Parse( text_p );
            if( !*token )
                break;
                
            if( !Q_stricmp( token, "-" ) )
            {
                bp->dLightRadius.final = PARTICLES_SAME_AS_INITIAL;
                bp->dLightRadius.finalRandFrac = 0.0f;
            }
            else
            {
                ParseValueAndVariance( token, &number, &randFrac, false );
                
                bp->dLightRadius.final = number;
                bp->dLightRadius.finalRandFrac = randFrac;
            }
            
            token = COM_Parse( text_p );
            if( !*token )
                break;
                
            if( !Q_stricmp( token, "{" ) )
            {
                if( !ParseColor( bp->dLightColor, text_p ) )
                    break;
                    
                token = COM_Parse( text_p );
                if( Q_stricmp( token, "}" ) )
                {
                    Printf( S_COLOR_RED "ERROR: missing '}'\n" );
                    break;
                }
            }
            
            continue;
        }
        else if( !Q_stricmp( token, "cullOnStartSolid" ) )
        {
            bp->cullOnStartSolid = true;
            
            continue;
        }
        else if( !Q_stricmp( token, "radius" ) )
        {
            token = COM_Parse( text_p );
            if( !token )
                break;
                
            ParseValueAndVariance( token, &number, &randFrac, false );
            
            bp->radius.delay = ( S32 )number;
            bp->radius.delayRandFrac = randFrac;
            
            token = COM_Parse( text_p );
            if( !token )
                break;
                
            ParseValueAndVariance( token, &number, &randFrac, false );
            
            bp->radius.initial = number;
            bp->radius.initialRandFrac = randFrac;
            
            token = COM_Parse( text_p );
            if( !token )
                break;
                
            if( !Q_stricmp( token, "-" ) )
            {
                bp->radius.final = PARTICLES_SAME_AS_INITIAL;
                bp->radius.finalRandFrac = 0.0f;
            }
            else
            {
                ParseValueAndVariance( token, &number, &randFrac, false );
                
                bp->radius.final = number;
                bp->radius.finalRandFrac = randFrac;
            }
            
            continue;
        }
        else if( !Q_stricmp( token, "physicsRadius" ) )
        {
            token = COM_Parse( text_p );
            if( !token )
                break;
                
            bp->physicsRadius = atoi( token );
        }
        else if( !Q_stricmp( token, "alpha" ) )
        {
            token = COM_Parse( text_p );
            if( !token )
                break;
                
            ParseValueAndVariance( token, &number, &randFrac, false );
            
            bp->alpha.delay = ( S32 )number;
            bp->alpha.delayRandFrac = randFrac;
            
            token = COM_Parse( text_p );
            if( !token )
                break;
                
            ParseValueAndVariance( token, &number, &randFrac, false );
            
            bp->alpha.initial = number;
            bp->alpha.initialRandFrac = randFrac;
            
            token = COM_Parse( text_p );
            if( !token )
                break;
                
            if( !Q_stricmp( token, "-" ) )
            {
                bp->alpha.final = PARTICLES_SAME_AS_INITIAL;
                bp->alpha.finalRandFrac = 0.0f;
            }
            else
            {
                ParseValueAndVariance( token, &number, &randFrac, false );
                
                bp->alpha.final = number;
                bp->alpha.finalRandFrac = randFrac;
            }
            
            continue;
        }
        else if( !Q_stricmp( token, "color" ) )
        {
            token = COM_Parse( text_p );
            if( !*token )
                break;
                
            ParseValueAndVariance( token, &number, &randFrac, false );
            
            bp->colorDelay = ( S32 )number;
            bp->colorDelayRandFrac = randFrac;
            
            token = COM_Parse( text_p );
            if( !*token )
                break;
                
            if( !Q_stricmp( token, "{" ) )
            {
                if( !ParseColor( bp->initialColor, text_p ) )
                    break;
                    
                token = COM_Parse( text_p );
                if( Q_stricmp( token, "}" ) )
                {
                    Printf( S_COLOR_RED "ERROR: missing '}'\n" );
                    break;
                }
                
                token = COM_Parse( text_p );
                if( !*token )
                    break;
                    
                if( !Q_stricmp( token, "-" ) )
                {
                    bp->finalColor[ 0 ] = bp->initialColor[ 0 ];
                    bp->finalColor[ 1 ] = bp->initialColor[ 1 ];
                    bp->finalColor[ 2 ] = bp->initialColor[ 2 ];
                }
                else if( !Q_stricmp( token, "{" ) )
                {
                    if( !ParseColor( bp->finalColor, text_p ) )
                        break;
                        
                    token = COM_Parse( text_p );
                    if( Q_stricmp( token, "}" ) )
                    {
                        Printf( S_COLOR_RED "ERROR: missing '}'\n" );
                        break;
                    }
                }
                else
                {
                    Printf( S_COLOR_RED "ERROR: missing '{'\n" );
                    break;
                }
            }
            else
            {
                Printf( S_COLOR_RED "ERROR: missing '{'\n" );
                break;
            }
            
            continue;
        }
        else if( !Q_stricmp( token, "rotation" ) )
        {
            token = COM_Parse( text_p );
            if( !token )
                break;
                
            ParseValueAndVariance( token, &number, &randFrac, false );
            
            bp->rotation.delay = ( S32 )number;
            bp->rotation.delayRandFrac = randFrac;
            
            token = COM_Parse( text_p );
            if( !token )
                break;
                
            ParseValueAndVariance( token, &number, &randFrac, true );
            
            bp->rotation.initial = number;
            bp->rotation.initialRandFrac = randFrac;
            
            token = COM_Parse( text_p );
            if( !token )
                break;
                
            if( !Q_stricmp( token, "-" ) )
            {
                bp->rotation.final = PARTICLES_SAME_AS_INITIAL;
                bp->rotation.finalRandFrac = 0.0f;
            }
            else
            {
                ParseValueAndVariance( token, &number, &randFrac, true );
                
                bp->rotation.final = number;
                bp->rotation.finalRandFrac = randFrac;
            }
            
            continue;
        }
        else if( !Q_stricmp( token, "lifeTime" ) )
        {
            token = COM_Parse( text_p );
            if( !token )
                break;
                
            ParseValueAndVariance( token, &number, &randFrac, false );
            
            bp->lifeTime = ( S32 )number;
            bp->lifeTimeRandFrac = randFrac;
            
            continue;
        }
        else if( !Q_stricmp( token, "childSystem" ) )
        {
            token = COM_Parse( text_p );
            if( !token )
                break;
                
            Q_strncpyz( bp->childSystemName, token, MAX_QPATH );
            
            continue;
        }
        else if( !Q_stricmp( token, "onDeathSystem" ) )
        {
            token = COM_Parse( text_p );
            if( !token )
                break;
                
            Q_strncpyz( bp->onDeathSystemName, token, MAX_QPATH );
            
            continue;
        }
        else if( !Q_stricmp( token, "childTrailSystem" ) )
        {
            token = COM_Parse( text_p );
            if( !token )
                break;
                
            Q_strncpyz( bp->childTrailSystemName, token, MAX_QPATH );
            
            continue;
        }
        else if( !Q_stricmp( token, "scaleWithCharge" ) )
        {
            token = COM_Parse( text_p );
            if( !token )
                break;
                
            bp->scaleWithCharge = atof( token );
            
            continue;
        }
        else if( !Q_stricmp( token, "}" ) )
            return true; //reached the end of this particle
        else
        {
            Printf( S_COLOR_RED "ERROR: unknown token '%s' in particle\n", token );
            return false;
        }
    }
    
    return false;
}

/*
===============
CG_InitialiseBaseParticle
===============
*/
void idCGameLocal::InitialiseBaseParticle( baseParticle_t* bp )
{
    ::memset( bp, 0, sizeof( baseParticle_t ) );
    
    ::memset( bp->initialColor, 0xFF, sizeof( bp->initialColor ) );
    ::memset( bp->finalColor, 0xFF, sizeof( bp->finalColor ) );
}

/*
===============
CG_ParseParticleEjector

Parse a particle ejector section
===============
*/
bool idCGameLocal::ParseParticleEjector( baseParticleEjector_t* bpe, UTF8** text_p )
{
    UTF8*  token;
    F32 number, randFrac;
    
    // read optional parameters
    while( 1 )
    {
        token = COM_Parse( text_p );
        
        if( !token )
            break;
            
        if( !Q_stricmp( token, "" ) )
            return false;
            
        if( !Q_stricmp( token, "{" ) )
        {
            InitialiseBaseParticle( &baseParticles[ numBaseParticles ] );
            
            if( !ParseParticle( &baseParticles[ numBaseParticles ], text_p ) )
            {
                Printf( S_COLOR_RED "ERROR: failed to parse particle\n" );
                return false;
            }
            
            if( bpe->numParticles == MAX_PARTICLES_PER_EJECTOR )
            {
                Printf( S_COLOR_RED "ERROR: ejector has > %d particles\n", MAX_PARTICLES_PER_EJECTOR );
                return false;
            }
            else if( numBaseParticles == MAX_BASEPARTICLES )
            {
                Printf( S_COLOR_RED "ERROR: maximum number of particles (%d) reached\n", MAX_BASEPARTICLES );
                return false;
            }
            else
            {
                //start parsing particles again
                bpe->particles[ bpe->numParticles ] = &baseParticles[ numBaseParticles ];
                bpe->numParticles++;
                numBaseParticles++;
            }
            continue;
        }
        else if( !Q_stricmp( token, "delay" ) )
        {
            token = COM_Parse( text_p );
            if( !token )
                break;
                
            ParseValueAndVariance( token, &number, &randFrac, false );
            
            bpe->eject.delay = ( S32 )number;
            bpe->eject.delayRandFrac = randFrac;
            
            continue;
        }
        else if( !Q_stricmp( token, "period" ) )
        {
            token = COM_Parse( text_p );
            if( !token )
                break;
                
            bpe->eject.initial = bggame->atoi_neg( token, false );
            
            token = COM_Parse( text_p );
            if( !token )
                break;
                
            if( !Q_stricmp( token, "-" ) )
                bpe->eject.final = PARTICLES_SAME_AS_INITIAL;
            else
                bpe->eject.final = bggame->atoi_neg( token, false );
                
            token = COM_Parse( text_p );
            if( !token )
                break;
                
            ParseValueAndVariance( token, NULL, &bpe->eject.randFrac, false );
            
            continue;
        }
        else if( !Q_stricmp( token, "count" ) )
        {
            token = COM_Parse( text_p );
            if( !token )
                break;
                
            if( !Q_stricmp( token, "infinite" ) )
            {
                bpe->totalParticles = PARTICLES_INFINITE;
                bpe->totalParticlesRandFrac = 0.0f;
            }
            else
            {
                ParseValueAndVariance( token, &number, &randFrac, false );
                
                bpe->totalParticles = ( S32 )number;
                bpe->totalParticlesRandFrac = randFrac;
            }
            
            continue;
        }
        else if( !Q_stricmp( token, "particle" ) ) //acceptable text
            continue;
        else if( !Q_stricmp( token, "}" ) )
            return true; //reached the end of this particle ejector
        else
        {
            Printf( S_COLOR_RED "ERROR: unknown token '%s' in particle ejector\n", token );
            return false;
        }
    }
    
    return false;
}

/*
===============
CG_ParseParticleSystem

Parse a particle system section
===============
*/
bool idCGameLocal::ParseParticleSystem( baseParticleSystem_t* bps, UTF8** text_p, StringEntry name )
{
    UTF8*                  token;
    baseParticleEjector_t* bpe;
    
    // read optional parameters
    while( 1 )
    {
        token = COM_Parse( text_p );
        
        if( !token )
            break;
            
        if( !Q_stricmp( token, "" ) )
            return false;
            
        if( !Q_stricmp( token, "{" ) )
        {
            if( !ParseParticleEjector( &baseParticleEjectors[ numBaseParticleEjectors ], text_p ) )
            {
                Printf( S_COLOR_RED "ERROR: failed to parse particle ejector\n" );
                return false;
            }
            
            bpe = &baseParticleEjectors[ numBaseParticleEjectors ];
            
            //check for infinite count + zero period
            if( bpe->totalParticles == PARTICLES_INFINITE && ( bpe->eject.initial == 0.0f || bpe->eject.final == 0.0f ) )
            {
                Printf( S_COLOR_RED "ERROR: ejector with 'count infinite' potentially has zero period\n" );
                return false;
            }
            
            if( bps->numEjectors == MAX_EJECTORS_PER_SYSTEM )
            {
                Printf( S_COLOR_RED "ERROR: particle system has > %d ejectors\n", MAX_EJECTORS_PER_SYSTEM );
                return false;
            }
            else if( numBaseParticleEjectors == MAX_BASEPARTICLE_EJECTORS )
            {
                Printf( S_COLOR_RED "ERROR: maximum number of particle ejectors (%d) reached\n", MAX_BASEPARTICLE_EJECTORS );
                return false;
            }
            else
            {
                //start parsing ejectors again
                bps->ejectors[ bps->numEjectors ] = &baseParticleEjectors[ numBaseParticleEjectors ];
                bps->numEjectors++;
                numBaseParticleEjectors++;
            }
            continue;
        }
        else if( !Q_stricmp( token, "thirdPersonOnly" ) )
            bps->thirdPersonOnly = true;
        else if( !Q_stricmp( token, "ejector" ) ) //acceptable text
            continue;
        else if( !Q_stricmp( token, "}" ) )
        {
            if( cg_debugParticles.integer >= 1 )
                Printf( "Parsed particle system %s\n", name );
                
            return true; //reached the end of this particle system
        }
        else
        {
            Printf( S_COLOR_RED "ERROR: unknown token '%s' in particle system %s\n", token, bps->name );
            return false;
        }
    }
    
    return false;
}

/*
===============
CG_ParseParticleFile

Load the particle systems from a particle file
===============
*/
bool idCGameLocal::ParseParticleFile( StringEntry fileName )
{
    UTF8*          text_p;
    S32           i;
    S32           len;
    UTF8*          token;
    UTF8          text[ 32000 ];
    UTF8          psName[ MAX_QPATH ];
    bool      psNameSet = false;
    fileHandle_t  f;
    
    // load the file
    len = trap_FS_FOpenFile( fileName, &f, FS_READ );
    if( len < 0 )
        return false;
        
    if( len == 0 || len >= sizeof( text ) - 1 )
    {
        trap_FS_FCloseFile( f );
        Printf( S_COLOR_RED "ERROR: particle file %s is %s\n", fileName, len == 0 ? "empty" : "too long" );
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
        
        if( !Q_stricmp( token, "" ) )
            break;
            
        if( !Q_stricmp( token, "{" ) )
        {
            if( psNameSet )
            {
                //check for name space clashes
                for( i = 0; i < numBaseParticleSystems; i++ )
                {
                    if( !Q_stricmp( baseParticleSystems[ i ].name, psName ) )
                    {
                        Printf( S_COLOR_RED "ERROR: a particle system is already named %s\n", psName );
                        return false;
                    }
                }
                
                Q_strncpyz( baseParticleSystems[ numBaseParticleSystems ].name, psName, MAX_QPATH );
                
                if( !ParseParticleSystem( &baseParticleSystems[ numBaseParticleSystems ], &text_p, psName ) )
                {
                    Printf( S_COLOR_RED "ERROR: %s: failed to parse particle system %s\n", fileName, psName );
                    return false;
                }
                
                //start parsing particle systems again
                psNameSet = false;
                
                if( numBaseParticleSystems == MAX_BASEPARTICLE_SYSTEMS )
                {
                    Printf( S_COLOR_RED "ERROR: maximum number of particle systems (%d) reached\n", MAX_BASEPARTICLE_SYSTEMS );
                    return false;
                }
                else
                    numBaseParticleSystems++;
                    
                continue;
            }
            else
            {
                Printf( S_COLOR_RED "ERROR: unamed particle system\n" );
                return false;
            }
        }
        
        if( !psNameSet )
        {
            Q_strncpyz( psName, token, sizeof( psName ) );
            psNameSet = true;
        }
        else
        {
            Printf( S_COLOR_RED "ERROR: particle system already named\n" );
            return false;
        }
    }
    
    return true;
}


/*
===============
CG_LoadParticleSystems

Load particle systems from .particle files
===============
*/
void idCGameLocal::LoadParticleSystems( void )
{
    S32   i, j, numFiles, fileLen;
    UTF8  fileList[ MAX_PARTICLE_FILES * MAX_QPATH ];
    UTF8  fileName[ MAX_QPATH ];
    UTF8*  filePtr;
    
    //clear out the old
    numBaseParticleSystems = 0;
    numBaseParticleEjectors = 0;
    numBaseParticles = 0;
    
    for( i = 0; i < MAX_BASEPARTICLE_SYSTEMS; i++ )
    {
        baseParticleSystem_t*  bps = &baseParticleSystems[ i ];
        ::memset( bps, 0, sizeof( baseParticleSystem_t ) );
    }
    
    for( i = 0; i < MAX_BASEPARTICLE_EJECTORS; i++ )
    {
        baseParticleEjector_t*  bpe = &baseParticleEjectors[ i ];
        ::memset( bpe, 0, sizeof( baseParticleEjector_t ) );
    }
    
    for( i = 0; i < MAX_BASEPARTICLES; i++ )
    {
        baseParticle_t*  bp = &baseParticles[ i ];
        ::memset( bp, 0, sizeof( baseParticle_t ) );
    }
    
    
    //and bring in the new
    numFiles = trap_FS_GetFileList( "scripts", ".particle", fileList, MAX_PARTICLE_FILES * MAX_QPATH );
    filePtr = fileList;
    
    for( i = 0; i < numFiles; i++, filePtr += fileLen + 1 )
    {
        fileLen = strlen( filePtr );
        strcpy( fileName, "scripts/" );
        strcat( fileName, filePtr );
        //Printf( "...loading '%s'\n", fileName );
        ParseParticleFile( fileName );
    }
    
    //connect any child systems to their psHandle
    for( i = 0; i < numBaseParticles; i++ )
    {
        baseParticle_t*  bp = &baseParticles[ i ];
        
        if( bp->childSystemName[ 0 ] )
        {
            //particle _class has a child, resolve the name
            for( j = 0; j < numBaseParticleSystems; j++ )
            {
                baseParticleSystem_t*  bps = &baseParticleSystems[ j ];
                
                if( !Q_stricmp( bps->name, bp->childSystemName ) )
                {
                    //FIXME: add checks for cycles and infinite children
                    
                    bp->childSystemHandle = j + 1;
                    
                    break;
                }
            }
            
            if( j == numBaseParticleSystems )
            {
                //couldn't find named particle system
                Printf( S_COLOR_YELLOW "WARNING: failed to find child %s\n", bp->childSystemName );
                bp->childSystemName[ 0 ] = '\0';
            }
        }
        
        if( bp->onDeathSystemName[ 0 ] )
        {
            //particle _class has a child, resolve the name
            for( j = 0; j < numBaseParticleSystems; j++ )
            {
                baseParticleSystem_t*  bps = &baseParticleSystems[ j ];
                
                if( !Q_stricmp( bps->name, bp->onDeathSystemName ) )
                {
                    //FIXME: add checks for cycles and infinite children
                    
                    bp->onDeathSystemHandle = j + 1;
                    
                    break;
                }
            }
            
            if( j == numBaseParticleSystems )
            {
                //couldn't find named particle system
                Printf( S_COLOR_YELLOW "WARNING: failed to find onDeath system %s\n", bp->onDeathSystemName );
                bp->onDeathSystemName[ 0 ] = '\0';
            }
        }
    }
}

/*
===============
CG_SetParticleSystemNormal
===============
*/
void idCGameLocal::SetParticleSystemNormal( particleSystem_t* ps, vec3_t normal )
{
    if( ps == NULL || !ps->valid )
    {
        Printf( S_COLOR_YELLOW "WARNING: tried to modify a NULL particle system\n" );
        return;
    }
    
    ps->normalValid = true;
    VectorCopy( normal, ps->normal );
    VectorNormalize( ps->normal );
}


/*
===============
CG_DestroyParticleSystem

Destroy a particle system

This doesn't actually invalidate anything, it just stops
particle ejectors from producing new particles so the
garbage collector will eventually remove this system.
However is does set the pointer to NULL so the user is
unable to manipulate this particle system any longer.
===============
*/
void idCGameLocal::DestroyParticleSystem( particleSystem_t** ps )
{
    S32               i;
    particleEjector_t* pe;
    
    if( *ps == NULL || !( *ps )->valid )
    {
        Printf( S_COLOR_YELLOW "WARNING: tried to destroy a NULL particle system\n" );
        return;
    }
    
    if( cg_debugParticles.integer >= 1 )
        Printf( "PS destroyed\n" );
        
    for( i = 0; i < MAX_PARTICLE_EJECTORS; i++ )
    {
        pe = &particleEjectors[ i ];
        
        if( pe->valid && pe->parent == *ps )
            pe->totalParticles = pe->count = 0;
    }
    
    *ps = NULL;
}

/*
===============
CG_IsParticleSystemInfinite

Test a particle system for 'count infinite' ejectors
===============
*/
bool idCGameLocal::IsParticleSystemInfinite( particleSystem_t* ps )
{
    S32               i;
    particleEjector_t* pe;
    
    if( ps == NULL )
    {
        Printf( S_COLOR_YELLOW "WARNING: tried to test a NULL particle system\n" );
        return false;
    }
    
    if( !ps->valid )
    {
        Printf( S_COLOR_YELLOW "WARNING: tried to test an invalid particle system\n" );
        return false;
    }
    
    //don't bother checking already invalid systems
    if( !ps->valid )
        return false;
        
    for( i = 0; i < MAX_PARTICLE_EJECTORS; i++ )
    {
        pe = &particleEjectors[ i ];
        
        if( pe->valid && pe->parent == ps )
        {
            if( pe->totalParticles == PARTICLES_INFINITE )
                return true;
        }
    }
    
    return false;
}

/*
===============
CG_IsParticleSystemValid

Test a particle system for validity
===============
*/
bool idCGameLocal::IsParticleSystemValid( particleSystem_t** ps )
{
    if( *ps == NULL || ( *ps && !( *ps )->valid ) )
    {
        if( *ps && !( *ps )->valid )
            *ps = NULL;
            
        return false;
    }
    
    return true;
}

/*
===============
CG_GarbageCollectParticleSystems

Destroy inactive particle systems
===============
*/
void idCGameLocal::GarbageCollectParticleSystems( void )
{
    S32               i, j, count;
    particleSystem_t*  ps;
    particleEjector_t* pe;
    S32               centNum;
    
    for( i = 0; i < MAX_PARTICLE_SYSTEMS; i++ )
    {
        ps = &particleSystems[ i ];
        count = 0;
        
        //don't bother checking already invalid systems
        if( !ps->valid )
            continue;
            
        for( j = 0; j < MAX_PARTICLE_EJECTORS; j++ )
        {
            pe = &particleEjectors[ j ];
            
            if( pe->valid && pe->parent == ps )
                count++;
        }
        
        if( !count )
            ps->valid = false;
            
        //check systems where the parent cent has left the PVS
        //( local player entity is always valid )
        if( ( centNum = AttachmentCentNum( &ps->attachment ) ) >= 0 && centNum != cg.snap->ps.clientNum )
        {
            if( !cg_entities[ centNum ].valid )
                ps->lazyRemove = true;
        }
        
        if( cg_debugParticles.integer >= 1 && !ps->valid )
            Printf( "PS %s garbage collected\n", ps->_class->name );
    }
}


/*
===============
CG_CalculateTimeFrac

Calculate the fraction of time passed
===============
*/
F32 idCGameLocal::CalculateTimeFrac( S32 birth, S32 life, S32 delay )
{
    F32 frac;
    
    frac = ( ( F32 )cg.time - ( F32 )( birth + delay ) ) / ( F32 )( life - delay );
    
    if( frac < 0.0f )
        frac = 0.0f;
    else if( frac > 1.0f )
        frac = 1.0f;
        
    return frac;
}

/*
===============
CG_EvaluateParticlePhysics

Compute the physics on a specific particle
===============
*/
void idCGameLocal::EvaluateParticlePhysics( particle_t* p )
{
    particleSystem_t*  ps = p->parent->parent;
    baseParticle_t*    bp = p->_class;
    vec3_t            acceleration, newOrigin;
    vec3_t            mins, maxs;
    F32             deltaTime, bounce, radius, dot;
    trace_t           trace;
    vec3_t            transform[ 3 ];
    
    if( p->atRest )
    {
        VectorClear( p->velocity );
        return;
    }
    
    switch( bp->accMoveType )
    {
        case PMT_STATIC:
            if( bp->accMoveValues.dirType == PMD_POINT )
                VectorSubtract( bp->accMoveValues.point, p->origin, acceleration );
            else if( bp->accMoveValues.dirType == PMD_LINEAR )
                VectorCopy( bp->accMoveValues.dir, acceleration );
                
            break;
            
        case PMT_STATIC_TRANSFORM:
            if( !AttachmentAxis( &ps->attachment, transform ) )
                return;
                
            if( bp->accMoveValues.dirType == PMD_POINT )
            {
                vec3_t transPoint;
                
                VectorMatrixMultiply( bp->accMoveValues.point, transform, transPoint );
                VectorSubtract( transPoint, p->origin, acceleration );
            }
            else if( bp->accMoveValues.dirType == PMD_LINEAR )
                VectorMatrixMultiply( bp->accMoveValues.dir, transform, acceleration );
            break;
            
        case PMT_TAG:
        case PMT_CENT_ANGLES:
            if( bp->accMoveValues.dirType == PMD_POINT )
            {
                vec3_t point;
                
                if( !AttachmentPoint( &ps->attachment, point ) )
                    return;
                    
                VectorSubtract( point, p->origin, acceleration );
            }
            else if( bp->accMoveValues.dirType == PMD_LINEAR )
            {
                if( !AttachmentDir( &ps->attachment, acceleration ) )
                    return;
            }
            break;
            
        case PMT_NORMAL:
            if( !ps->normalValid )
                return;
                
            VectorCopy( ps->normal, acceleration );
            
            break;
    }
    
#define MAX_ACC_RADIUS 1000.0f
    
    if( bp->accMoveValues.dirType == PMD_POINT )
    {
        //FIXME: so this fall off is a bit... odd -- it works..
        F32 r2 = DotProduct( acceleration, acceleration ); // = radius^2
        F32 scale = ( MAX_ACC_RADIUS - r2 ) / MAX_ACC_RADIUS;
        
        if( scale > 1.0f )
            scale = 1.0f;
        else if( scale < 0.1f )
            scale = 0.1f;
            
        scale *= RandomiseValue( bp->accMoveValues.mag, bp->accMoveValues.magRandFrac );
        
        VectorNormalize( acceleration );
        SpreadVector( acceleration, bp->accMoveValues.dirRandAngle );
        VectorScale( acceleration, scale, acceleration );
    }
    else if( bp->accMoveValues.dirType == PMD_LINEAR )
    {
        VectorNormalize( acceleration );
        SpreadVector( acceleration, bp->accMoveValues.dirRandAngle );
        VectorScale( acceleration, RandomiseValue( bp->accMoveValues.mag, bp->accMoveValues.magRandFrac ), acceleration );
    }
    
    // Some particles have a visual radius that differs from their collision radius
    if( bp->physicsRadius )
        radius = bp->physicsRadius;
    else
        radius = LerpValues( p->radius.initial, p->radius.final, CalculateTimeFrac( p->birthTime, p->lifeTime, p->radius.delay ) );
        
    VectorSet( mins, -radius, -radius, -radius );
    VectorSet( maxs, radius, radius, radius );
    
    bounce = RandomiseValue( bp->bounceFrac, bp->bounceFracRandFrac );
    
    deltaTime = ( F32 )( cg.time - p->lastEvalTime ) * 0.001;
    VectorMA( p->velocity, deltaTime, acceleration, p->velocity );
    VectorMA( p->origin, deltaTime, p->velocity, newOrigin );
    p->lastEvalTime = cg.time;
    
    // we're not doing particle physics, but at least cull them in solids
    if( !cg_bounceParticles.integer )
    {
        S32 contents = trap_CM_PointContents( newOrigin, 0 );
        
        if( ( contents & CONTENTS_SOLID ) || ( contents & CONTENTS_NODROP ) )
            DestroyParticle( p, NULL );
        else
            VectorCopy( newOrigin, p->origin );
        return;
    }
    
    Trace( &trace, p->origin, mins, maxs, newOrigin, AttachmentCentNum( &ps->attachment ), CONTENTS_SOLID );
    
    //not hit anything or not a collider
    if( trace.fraction == 1.0f || bounce == 0.0f )
    {
        VectorCopy( newOrigin, p->origin );
        return;
    }
    
    //remove particles that get into a CONTENTS_NODROP brush
    if( ( trap_CM_PointContents( trace.endpos, 0 ) & CONTENTS_NODROP ) || ( bp->cullOnStartSolid && trace.startsolid ) )
    {
        DestroyParticle( p, NULL );
        return;
    }
    else if( bp->bounceCull )
    {
        DestroyParticle( p, trace.plane.normal );
        return;
    }
    
    //reflect the velocity on the trace plane
    dot = DotProduct( p->velocity, trace.plane.normal );
    VectorMA( p->velocity, -2.0f * dot, trace.plane.normal, p->velocity );
    
    VectorScale( p->velocity, bounce, p->velocity );
    
    if( trace.plane.normal[ 2 ] > 0.5f &&
            ( p->velocity[ 2 ] < 40.0f ||
              p->velocity[ 2 ] < -cg.frametime * p->velocity[ 2 ] ) )
        p->atRest = true;
        
    if( bp->bounceMarkName[ 0 ] && p->bounceMarkCount > 0 )
    {
        ImpactMark( bp->bounceMark, trace.endpos, trace.plane.normal, random( ) * 360, 1, 1, 1, 1, true, bp->bounceMarkRadius, false );
        p->bounceMarkCount--;
    }
    
    if( bp->bounceSoundName[ 0 ] && p->bounceSoundCount > 0 )
    {
        trap_S_StartSound( trace.endpos, ENTITYNUM_WORLD, CHAN_AUTO, bp->bounceSound );
        p->bounceSoundCount--;
    }
    
    VectorCopy( trace.endpos, p->origin );
}


#define GETKEY(x,y) (((x)>>y)&0xFF)

/*
===============
CG_Radix
===============
*/
void idCGameLocal::Radix( S32 bits, S32 size, particle_t** source, particle_t** dest )
{
    S32 count[ 256 ];
    S32 index[ 256 ];
    S32 i;
    
    ::memset( count, 0, sizeof( count ) );
    
    for( i = 0; i < size; i++ )
        count[ GETKEY( source[ i ]->sortKey, bits ) ]++;
        
    index[ 0 ] = 0;
    
    for( i = 1; i < 256; i++ )
        index[ i ] = index[ i - 1 ] + count[ i - 1 ];
        
    for( i = 0; i < size; i++ )
        dest[ index[ GETKEY( source[ i ]->sortKey, bits ) ]++ ] = source[ i ];
}

/*
===============
CG_RadixSort

Radix sort with 4 byte size buckets
===============
*/
void idCGameLocal::RadixSort( particle_t** source, particle_t** temp, S32 size )
{
    Radix( 0,   size, source, temp );
    Radix( 8,   size, temp, source );
    Radix( 16,  size, source, temp );
    Radix( 24,  size, temp, source );
}

/*
===============
CG_CompactAndSortParticles

Depth sort the particles
===============
*/
void idCGameLocal::CompactAndSortParticles( void )
{
    S32     i, j = 0;
    S32     numParticles;
    vec3_t  delta;
    
    for( i = 0; i < MAX_PARTICLES; i++ )
        sortedParticles[ i ] = &particles[ i ];
        
    if( !cg_depthSortParticles.integer )
        return;
        
    for( i = MAX_PARTICLES - 1; i >= 0; i-- )
    {
        if( sortedParticles[ i ]->valid )
        {
            //find the first hole
            while( j < MAX_PARTICLES && sortedParticles[ j ]->valid )
                j++;
                
            //no more holes
            if( j >= i )
                break;
                
            sortedParticles[ j ] = sortedParticles[ i ];
        }
    }
    
    numParticles = i;
    
    //set sort keys
    for( i = 0; i < numParticles; i++ )
    {
        VectorSubtract( sortedParticles[ i ]->origin, cg.refdef.vieworg, delta );
        sortedParticles[ i ]->sortKey = ( S32 )DotProduct( delta, delta );
    }
    
    RadixSort( sortedParticles, radixBuffer, numParticles );
    
    //FIXME: wtf?
    //reverse order of particles array
    for( i = 0; i < numParticles; i++ )
        radixBuffer[ i ] = sortedParticles[ numParticles - i - 1 ];
        
    for( i = 0; i < numParticles; i++ )
        sortedParticles[ i ] = radixBuffer[ i ];
}

/*
===============
CG_RenderParticle

Actually render a particle
===============
*/
void idCGameLocal::RenderParticle( particle_t* p )
{
    refEntity_t           re;
    F32                 timeFrac, scale;
    S32                   index;
    baseParticle_t*        bp = p->_class;
    particleSystem_t*      ps = p->parent->parent;
    baseParticleSystem_t*  bps = ps->_class;
    vec3_t                alight, dlight, lightdir;
    S32                   i;
    vec3_t                up = { 0.0f, 0.0f, 1.0f };
    
    ::memset( &re, 0, sizeof( refEntity_t ) );
    
    timeFrac = CalculateTimeFrac( p->birthTime, p->lifeTime, 0 );
    
    scale = LerpValues( p->radius.initial, p->radius.final, CalculateTimeFrac( p->birthTime, p->lifeTime, p->radius.delay ) );
    
    re.shaderTime = p->birthTime / 1000.0f;
    
    if( bp->numFrames )       //shader based
    {
        re.reType = RT_SPRITE;
        
        //apply environmental lighting to the particle
        if( bp->realLight )
        {
            trap_R_LightForPoint( p->origin, alight, dlight, lightdir );
            for( i = 0; i <= 2; i++ )
                re.shaderRGBA[ i ] = ( U8 )alight[ i ];
        }
        else
        {
            vec3_t  colorRange;
            
            VectorSubtract( bp->finalColor, bp->initialColor, colorRange );
            
            VectorMA( bp->initialColor, CalculateTimeFrac( p->birthTime, p->lifeTime, p->colorDelay ), colorRange, re.shaderRGBA );
        }
        
        re.shaderRGBA[ 3 ] = ( U8 )( ( F32 )0xFF * LerpValues( p->alpha.initial, p->alpha.final, CalculateTimeFrac( p->birthTime, p->lifeTime, p->alpha.delay ) ) );
        
        re.radius = scale;
        
        re.rotation = LerpValues( p->rotation.initial, p->rotation.final, CalculateTimeFrac( p->birthTime, p->lifeTime, p->rotation.delay ) );
        
        // if the view would be "inside" the sprite, kill the sprite
        // so it doesn't add too much overdraw
        if( Distance( p->origin, cg.refdef.vieworg ) < re.radius && bp->overdrawProtection )
            return;
            
        if( bp->framerate == 0.0f )
        {
            //sync animation time to lifeTime of particle
            index = ( S32 )( timeFrac * ( bp->numFrames + 1 ) );
            
            if( index >= bp->numFrames )
                index = bp->numFrames - 1;
                
            re.customShader = bp->shaders[ index ];
        }
        else
        {
            //looping animation
            index = ( S32 )( bp->framerate * timeFrac * p->lifeTime * 0.001 ) % bp->numFrames;
            re.customShader = bp->shaders[ index ];
        }
        
    }
    else if( bp->numModels )  //model based
    {
        re.reType = RT_MODEL;
        
        re.hModel = p->model;
        
        if( p->atRest )
            AxisCopy( p->lastAxis, re.axis );
        else
        {
            // convert direction of travel into axis
            VectorNormalize2( p->velocity, re.axis[ 0 ] );
            
            if( re.axis[ 0 ][ 0 ] == 0.0f && re.axis[ 0 ][ 1 ] == 0.0f )
                AxisCopy( axisDefault, re.axis );
            else
            {
                ProjectPointOnPlane( re.axis[ 2 ], up, re.axis[ 0 ] );
                VectorNormalize( re.axis[ 2 ] );
                CrossProduct( re.axis[ 2 ], re.axis[ 0 ], re.axis[ 1 ] );
            }
            
            AxisCopy( re.axis, p->lastAxis );
        }
        
        if( scale != 1.0f )
        {
            VectorScale( re.axis[ 0 ], scale, re.axis[ 0 ] );
            VectorScale( re.axis[ 1 ], scale, re.axis[ 1 ] );
            VectorScale( re.axis[ 2 ], scale, re.axis[ 2 ] );
            re.nonNormalizedAxes = true;
        }
        else
            re.nonNormalizedAxes = false;
            
        p->lf.animation = &bp->modelAnimation;
        
        //run animation
        RunLerpFrame( &p->lf, 1.0f );
        
        re.oldframe = p->lf.oldFrame;
        re.frame    = p->lf.frame;
        re.backlerp = p->lf.backlerp;
    }
    
    if( bps->thirdPersonOnly && AttachmentCentNum( &ps->attachment ) == cg.snap->ps.clientNum && !cg.renderingThirdPerson )
        re.renderfx |= RF_THIRD_PERSON;
        
    if( bp->dynamicLight && !( re.renderfx & RF_THIRD_PERSON ) )
    {
        trap_R_AddLightToScene( p->origin, LerpValues( p->dLightRadius.initial, p->dLightRadius.final, CalculateTimeFrac( p->birthTime, p->lifeTime, p->dLightRadius.delay ) ),
                                ( F32 )bp->dLightColor[ 0 ] / ( F32 )0xFF,
                                ( F32 )bp->dLightColor[ 1 ] / ( F32 )0xFF,
                                ( F32 )bp->dLightColor[ 2 ] / ( F32 )0xFF );
    }
    
    VectorCopy( p->origin, re.origin );
    
    trap_R_AddRefEntityToScene( &re );
}

/*
===============
CG_AddParticles

Add particles to the scene
===============
*/
void idCGameLocal::AddParticles( void )
{
    S32           i;
    particle_t*    p;
    S32           numPS = 0, numPE = 0, numP = 0;
    
    //remove expired particle systems
    GarbageCollectParticleSystems( );
    
    //check each ejector and introduce any new particles
    SpawnNewParticles( );
    
    //sorting
    CompactAndSortParticles( );
    
    for( i = 0; i < MAX_PARTICLES; i++ )
    {
        p = sortedParticles[ i ];
        
        if( p->valid )
        {
            if( p->birthTime + p->lifeTime > cg.time )
            {
                //particle is active
                EvaluateParticlePhysics( p );
                RenderParticle( p );
            }
            else
                DestroyParticle( p, NULL );
        }
    }
    
    if( cg_debugParticles.integer >= 2 )
    {
        for( i = 0; i < MAX_PARTICLE_SYSTEMS; i++ )
            if( particleSystems[ i ].valid )
                numPS++;
                
        for( i = 0; i < MAX_PARTICLE_EJECTORS; i++ )
            if( particleEjectors[ i ].valid )
                numPE++;
                
        for( i = 0; i < MAX_PARTICLES; i++ )
            if( particles[ i ].valid )
                numP++;
                
        Printf( "PS: %d  PE: %d  P: %d\n", numPS, numPE, numP );
    }
}

/*
===============
CG_ParticleSystemEntity

Particle system entity client code
===============
*/
void idCGameLocal::ParticleSystemEntity( centity_t* cent )
{
    entityState_t* es;
    
    es = &cent->currentState;
    
    if( es->eFlags & EF_NODRAW )
    {
        if( IsParticleSystemValid( &cent->entityPS ) && IsParticleSystemInfinite( cent->entityPS ) )
            DestroyParticleSystem( &cent->entityPS );
            
        return;
    }
    
    if( !IsParticleSystemValid( &cent->entityPS ) && !cent->entityPSMissing )
    {
        cent->entityPS = SpawnNewParticleSystem( cgs.gameParticleSystems[ es->modelindex ] );
        
        if( IsParticleSystemValid( &cent->entityPS ) )
        {
            SetAttachmentPoint( &cent->entityPS->attachment, cent->lerpOrigin );
            SetAttachmentCent( &cent->entityPS->attachment, cent );
            AttachToPoint( &cent->entityPS->attachment );
        }
        else
            cent->entityPSMissing = true;
    }
}

static particleSystem_t* testPS;
static qhandle_t        testPSHandle;

/*
===============
CG_DestroyTestPS_f

Destroy the test a particle system
===============
*/
void idCGameLocal::DestroyTestPS_f( void )
{
    if( IsParticleSystemValid( &testPS ) )
        DestroyParticleSystem( &testPS );
}

/*
===============
CG_TestPS_f

Test a particle system
===============
*/
void idCGameLocal::TestPS_f( void )
{
    vec3_t  origin;
    vec3_t  up = { 0.0f, 0.0f, 1.0f };
    UTF8    psName[ MAX_QPATH ];
    
    if( trap_Argc( ) < 2 )
        return;
        
    Q_strncpyz( psName, Argv( 1 ), MAX_QPATH );
    testPSHandle = RegisterParticleSystem( psName );
    
    if( testPSHandle )
    {
        DestroyTestPS_f( );
        
        testPS = SpawnNewParticleSystem( testPSHandle );
        
        VectorMA( cg.refdef.vieworg, 100, cg.refdef.viewaxis[ 0 ], origin );
        
        if( IsParticleSystemValid( &testPS ) )
        {
            SetAttachmentPoint( &testPS->attachment, origin );
            SetParticleSystemNormal( testPS, up );
            AttachToPoint( &testPS->attachment );
        }
    }
}
