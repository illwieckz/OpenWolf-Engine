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
// File name:   cg_attachment.cpp
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description: an abstract attachment system
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#include <cgame/cg_local.h>

/*
===============
AttachmentPoint

Return the attachment point
===============
*/
bool idCGameLocal::AttachmentPoint( attachment_t* a, vec3_t v )
{
    centity_t*   cent;
    
    if( !a )
        return false;
        
    // if it all breaks, then use the last point we know was correct
    VectorCopy( a->lastValidAttachmentPoint, v );
    
    switch( a->type )
    {
        case AT_STATIC:
            if( !a->staticValid )
                return false;
                
            VectorCopy( a->origin, v );
            break;
            
        case AT_TAG:
            if( !a->tagValid )
                return false;
                
            AxisCopy( axisDefault, a->re.axis );
            PositionRotatedEntityOnTag( &a->re, &a->parent,
                                        a->model, a->tagName );
            VectorCopy( a->re.origin, v );
            break;
            
        case AT_CENT:
            if( !a->centValid )
                return false;
                
            if( a->centNum == cg.predictedPlayerState.clientNum )
            {
                // this is smoother if it's the local client
                VectorCopy( cg.predictedPlayerState.origin, v );
            }
            else
            {
                cent = &cg_entities[ a->centNum ];
                VectorCopy( cent->lerpOrigin, v );
            }
            break;
            
        case AT_PARTICLE:
            if( !a->particleValid )
                return false;
                
            if( !a->particle->valid )
            {
                a->particleValid = false;
                return false;
            }
            else
                VectorCopy( a->particle->origin, v );
            break;
            
        default:
            Printf( S_COLOR_RED "ERROR: Invalid attachmentType_t in attachment\n" );
            break;
    }
    
    if( a->hasOffset )
        VectorAdd( v, a->offset, v );
        
    VectorCopy( v, a->lastValidAttachmentPoint );
    
    return true;
}

/*
===============
AttachmentDir

Return the attachment direction
===============
*/
bool idCGameLocal::AttachmentDir( attachment_t* a, vec3_t v )
{
    vec3_t      forward;
    centity_t*   cent;
    
    if( !a )
        return false;
        
    switch( a->type )
    {
        case AT_STATIC:
            return false;
            break;
            
        case AT_TAG:
            if( !a->tagValid )
                return false;
                
            VectorCopy( a->re.axis[ 0 ], v );
            break;
            
        case AT_CENT:
            if( !a->centValid )
                return false;
                
            cent = &cg_entities[ a->centNum ];
            AngleVectors( cent->lerpAngles, forward, NULL, NULL );
            VectorCopy( forward, v );
            break;
            
        case AT_PARTICLE:
            if( !a->particleValid )
                return false;
                
            if( !a->particle->valid )
            {
                a->particleValid = false;
                return false;
            }
            else
                VectorCopy( a->particle->velocity, v );
            break;
            
        default:
            Printf( S_COLOR_RED "ERROR: Invalid attachmentType_t in attachment\n" );
            break;
    }
    
    VectorNormalize( v );
    return true;
}

/*
===============
AttachmentAxis

Return the attachment axis
===============
*/
bool idCGameLocal::AttachmentAxis( attachment_t* a, vec3_t axis[ 3 ] )
{
    centity_t*   cent;
    
    if( !a )
        return false;
        
    switch( a->type )
    {
        case AT_STATIC:
            return false;
            break;
            
        case AT_TAG:
            if( !a->tagValid )
                return false;
                
            AxisCopy( a->re.axis, axis );
            break;
            
        case AT_CENT:
            if( !a->centValid )
                return false;
                
            cent = &cg_entities[ a->centNum ];
            AnglesToAxis( cent->lerpAngles, axis );
            break;
            
        case AT_PARTICLE:
            return false;
            break;
            
        default:
            Printf( S_COLOR_RED "ERROR: Invalid attachmentType_t in attachment\n" );
            break;
    }
    
    return true;
}

/*
===============
AttachmentVelocity

If the attachment can have velocity, return it
===============
*/
bool idCGameLocal::AttachmentVelocity( attachment_t* a, vec3_t v )
{
    if( !a )
        return false;
        
    if( a->particleValid && a->particle->valid )
    {
        VectorCopy( a->particle->velocity, v );
        return true;
    }
    else if( a->centValid )
    {
        centity_t* cent = &cg_entities[ a->centNum ];
        
        VectorCopy( cent->currentState.pos.trDelta, v );
        return true;
    }
    
    return false;
}

/*
===============
AttachmentCentNum

If the attachment has a centNum, return it
===============
*/
S32 idCGameLocal::AttachmentCentNum( attachment_t* a )
{
    if( !a || !a->centValid )
        return -1;
        
    return a->centNum;
}

/*
===============
Attached

If the attachment is valid, return true
===============
*/
bool idCGameLocal::Attached( attachment_t* a )
{
    if( !a )
        return false;
        
    return a->attached;
}

/*
===============
AttachToPoint

Attach to a point in space
===============
*/
void idCGameLocal::AttachToPoint( attachment_t* a )
{
    if( !a || !a->staticValid )
        return;
        
    a->type = AT_STATIC;
    a->attached = true;
}

/*
===============
AttachToCent

Attach to a centity_t
===============
*/
void idCGameLocal::AttachToCent( attachment_t* a )
{
    if( !a || !a->centValid )
        return;
        
    a->type = AT_CENT;
    a->attached = true;
}

/*
===============
AttachToTag

Attach to a model tag
===============
*/
void idCGameLocal::AttachToTag( attachment_t* a )
{
    if( !a || !a->tagValid )
        return;
        
    a->type = AT_TAG;
    a->attached = true;
}

/*
===============
AttachToParticle

Attach to a particle
===============
*/
void idCGameLocal::AttachToParticle( attachment_t* a )
{
    if( !a || !a->particleValid )
        return;
        
    a->type = AT_PARTICLE;
    a->attached = true;
}

/*
===============
SetAttachmentPoint
===============
*/
void idCGameLocal::SetAttachmentPoint( attachment_t* a, vec3_t v )
{
    if( !a )
        return;
        
    VectorCopy( v, a->origin );
    a->staticValid = true;
}

/*
===============
SetAttachmentCent
===============
*/
void idCGameLocal::SetAttachmentCent( attachment_t* a, centity_t* cent )
{
    if( !a || !cent )
        return;
        
    a->centNum = cent->currentState.number;
    a->centValid = true;
}

/*
===============
SetAttachmentTag
===============
*/
void idCGameLocal::SetAttachmentTag( attachment_t* a, refEntity_t parent, qhandle_t model, UTF8* tagName )
{
    if( !a )
        return;
        
    a->parent = parent;
    a->model = model;
    strncpy( a->tagName, tagName, MAX_STRING_CHARS );
    a->tagValid = true;
}

/*
===============
SetAttachmentParticle
===============
*/
void idCGameLocal::SetAttachmentParticle( attachment_t* a, particle_t* p )
{
    if( !a )
        return;
        
    a->particle = p;
    a->particleValid = true;
}

/*
===============
SetAttachmentOffset
===============
*/
void idCGameLocal::SetAttachmentOffset( attachment_t* a, vec3_t v )
{
    if( !a )
        return;
        
    VectorCopy( v, a->offset );
    a->hasOffset = true;
}
