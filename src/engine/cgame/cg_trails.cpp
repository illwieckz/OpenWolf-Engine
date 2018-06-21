////////////////////////////////////////////////////////////////////////////////////////
// Copyright(C) 1999 - 2005 Id Software, Inc.
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
// File name:   cg_trails.cpp
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description: the trail system
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#include <OWLIb/precompiled.h>

static baseTrailSystem_t  baseTrailSystems[ MAX_BASETRAIL_SYSTEMS ];
static baseTrailBeam_t    baseTrailBeams[ MAX_BASETRAIL_BEAMS ];
static S32                numBaseTrailSystems = 0;
static S32                numBaseTrailBeams = 0;

static trailSystem_t      trailSystems[ MAX_TRAIL_SYSTEMS ];
static trailBeam_t        trailBeams[ MAX_TRAIL_BEAMS ];

/*
===============
CG_CalculateBeamNodeProperties

Fills in trailBeamNode_t.textureCoord
===============
*/
void idCGameLocal::CalculateBeamNodeProperties( trailBeam_t* tb )
{
    trailBeamNode_t* i = NULL;
    trailSystem_t*   ts;
    baseTrailBeam_t* btb;
    F32           nodeDistances[ MAX_TRAIL_BEAM_NODES ];
    F32           totalDistance = 0.0f, position = 0.0f;
    S32             j, numNodes = 0;
    F32           TCRange, widthRange, alphaRange;
    vec3_t          colorRange;
    F32           fadeAlpha = 1.0f;
    
    if( !tb || !tb->nodes )
        return;
        
    ts = tb->parent;
    btb = tb->_class;
    
    if( ts->destroyTime > 0 && btb->fadeOutTime )
    {
        fadeAlpha -= ( cg.time - ts->destroyTime ) / btb->fadeOutTime;
        
        if( fadeAlpha < 0.0f )
            fadeAlpha = 0.0f;
    }
    
    TCRange = tb->_class->backTextureCoord -
              tb->_class->frontTextureCoord;
    widthRange = tb->_class->backWidth -
                 tb->_class->frontWidth;
    alphaRange = tb->_class->backAlpha -
                 tb->_class->frontAlpha;
    VectorSubtract( tb->_class->backColor,
                    tb->_class->frontColor, colorRange );
                    
    for( i = tb->nodes; i && i->next; i = i->next )
    {
        nodeDistances[ numNodes++ ] =
            Distance( i->position, i->next->position );
    }
    
    for( j = 0; j < numNodes; j++ )
        totalDistance += nodeDistances[ j ];
        
    for( j = 0, i = tb->nodes; i; i = i->next, j++ )
    {
        if( tb->_class->textureType == TBTT_STRETCH )
        {
            i->textureCoord = tb->_class->frontTextureCoord +
                              ( ( position / totalDistance ) * TCRange );
        }
        else if( tb->_class->textureType == TBTT_REPEAT )
        {
            if( tb->_class->clampToBack )
                i->textureCoord = ( totalDistance - position ) /
                                  tb->_class->repeatLength;
            else
                i->textureCoord = position / tb->_class->repeatLength;
        }
        
        i->halfWidth = ( tb->_class->frontWidth +
                         ( ( position / totalDistance ) * widthRange ) ) / 2.0f;
        i->alpha = ( U8 )( ( F32 )0xFF * ( tb->_class->frontAlpha +
                                           ( ( position / totalDistance ) * alphaRange ) ) * fadeAlpha );
        VectorMA( tb->_class->frontColor, ( position / totalDistance ),
                  colorRange, i->color );
                  
        position += nodeDistances[ j ];
    }
}

/*
===============
CG_LightVertex

Lights a particular vertex
===============
*/
void idCGameLocal::LightVertex( vec3_t point, U8 alpha, U8* rgba )
{
    S32     i;
    vec3_t  alight, dlight, lightdir;
    
    trap_R_LightForPoint( point, alight, dlight, lightdir );
    for( i = 0; i <= 2; i++ )
        rgba[ i ] = ( S32 )alight[ i ];
        
    rgba[ 3 ] = alpha;
}

/*
===============
CG_RenderBeam

Renders a beam
===============
*/
void idCGameLocal::RenderBeam( trailBeam_t* tb )
{
    trailBeamNode_t*   i = NULL;
    trailBeamNode_t*   prev = NULL;
    trailBeamNode_t*   next = NULL;
    vec3_t            up;
    polyVert_t        verts[( MAX_TRAIL_BEAM_NODES - 1 ) * 4 ];
    S32               numVerts = 0;
    baseTrailBeam_t*   btb;
    trailSystem_t*     ts;
    baseTrailSystem_t* bts;
    
    if( !tb || !tb->nodes )
        return;
        
    btb = tb->_class;
    ts = tb->parent;
    bts = ts->_class;
    
    if( bts->thirdPersonOnly && ( AttachmentCentNum( &ts->frontAttachment ) == cg.snap->ps.clientNum || AttachmentCentNum( &ts->backAttachment ) == cg.snap->ps.clientNum ) && !cg.renderingThirdPerson )
        return;
        
    CalculateBeamNodeProperties( tb );
    
    i = tb->nodes;
    
    do
    {
        prev = i->prev;
        next = i->next;
        
        if( prev && next )
        {
            //this node has two neighbours
            GetPerpendicularViewVector( cg.refdef.vieworg, next->position, prev->position, up );
        }
        else if( !prev && next )
        {
            //this is the front
            GetPerpendicularViewVector( cg.refdef.vieworg, next->position, i->position, up );
        }
        else if( prev && !next )
        {
            //this is the back
            GetPerpendicularViewVector( cg.refdef.vieworg, i->position, prev->position, up );
        }
        else
            break;
            
        if( prev )
        {
            VectorMA( i->position, i->halfWidth, up, verts[ numVerts ].xyz );
            verts[ numVerts ].st[ 0 ] = i->textureCoord;
            verts[ numVerts ].st[ 1 ] = 1.0f;
            
            if( btb->realLight )
                LightVertex( verts[ numVerts ].xyz, i->alpha, verts[ numVerts ].modulate );
            else
            {
                VectorCopy( i->color, verts[ numVerts ].modulate );
                verts[ numVerts ].modulate[ 3 ] = i->alpha;
            }
            
            numVerts++;
            
            VectorMA( i->position, -i->halfWidth, up, verts[ numVerts ].xyz );
            verts[ numVerts ].st[ 0 ] = i->textureCoord;
            verts[ numVerts ].st[ 1 ] = 0.0f;
            
            if( btb->realLight )
                LightVertex( verts[ numVerts ].xyz, i->alpha, verts[ numVerts ].modulate );
            else
            {
                VectorCopy( i->color, verts[ numVerts ].modulate );
                verts[ numVerts ].modulate[ 3 ] = i->alpha;
            }
            
            numVerts++;
        }
        
        if( next )
        {
            VectorMA( i->position, -i->halfWidth, up, verts[ numVerts ].xyz );
            verts[ numVerts ].st[ 0 ] = i->textureCoord;
            verts[ numVerts ].st[ 1 ] = 0.0f;
            
            if( btb->realLight )
                LightVertex( verts[ numVerts ].xyz, i->alpha, verts[ numVerts ].modulate );
            else
            {
                VectorCopy( i->color, verts[ numVerts ].modulate );
                verts[ numVerts ].modulate[ 3 ] = i->alpha;
            }
            
            numVerts++;
            
            VectorMA( i->position, i->halfWidth, up, verts[ numVerts ].xyz );
            verts[ numVerts ].st[ 0 ] = i->textureCoord;
            verts[ numVerts ].st[ 1 ] = 1.0f;
            
            if( btb->realLight )
                LightVertex( verts[ numVerts ].xyz, i->alpha, verts[ numVerts ].modulate );
            else
            {
                VectorCopy( i->color, verts[ numVerts ].modulate );
                verts[ numVerts ].modulate[ 3 ] = i->alpha;
            }
            
            numVerts++;
        }
        
        i = i->next;
    }
    while( i );
    
    trap_R_AddPolysToScene( tb->_class->shader, 4, &verts[ 0 ], numVerts / 4 );
}

/*
===============
CG_AllocateBeamNode

Allocates a trailBeamNode_t from a trailBeam_t's nodePool
===============
*/
trailBeamNode_t* idCGameLocal::AllocateBeamNode( trailBeam_t* tb )
{
    baseTrailBeam_t* btb = tb->_class;
    S32             i;
    trailBeamNode_t* tbn;
    
    for( i = 0; i < MAX_TRAIL_BEAM_NODES; i++ )
    {
        tbn = &tb->nodePool[ i ];
        if( !tbn->used )
        {
            tbn->timeLeft = btb->segmentTime;
            tbn->prev = NULL;
            tbn->next = NULL;
            tbn->used = true;
            return tbn;
        }
    }
    
    // no space left
    return NULL;
}

/*
===============
CG_DestroyBeamNode

Removes a node from a beam
Returns the new head
===============
*/
trailBeamNode_t* idCGameLocal::DestroyBeamNode( trailBeamNode_t* tbn )
{
    trailBeamNode_t* newHead = NULL;
    
    if( tbn->prev )
    {
        if( tbn->next )
        {
            // node is in the middle
            tbn->prev->next = tbn->next;
            tbn->next->prev = tbn->prev;
        }
        else // node is at the back
            tbn->prev->next = NULL;
            
        // find the new head (shouldn't have changed)
        newHead = tbn->prev;
        
        while( newHead->prev )
            newHead = newHead->prev;
    }
    else if( tbn->next )
    {
        //node is at the front
        tbn->next->prev = NULL;
        newHead = tbn->next;
    }
    
    tbn->prev = NULL;
    tbn->next = NULL;
    tbn->used = false;
    
    return newHead;
}

/*
===============
CG_FindLastBeamNode

Returns the last beam node in a beam
===============
*/
trailBeamNode_t* idCGameLocal::FindLastBeamNode( trailBeam_t* tb )
{
    trailBeamNode_t* i = tb->nodes;
    
    while( i && i->next )
        i = i->next;
        
    return i;
}

/*
===============
CG_CountBeamNodes

Returns the number of nodes in a beam
===============
*/
S32 idCGameLocal::CountBeamNodes( trailBeam_t* tb )
{
    trailBeamNode_t* i = tb->nodes;
    S32             numNodes = 0;
    
    while( i )
    {
        numNodes++;
        i = i->next;
    }
    
    return numNodes;
}

/*
===============
CG_PrependBeamNode

Prepend a new beam node to the front of a beam
Returns the new node
===============
*/
trailBeamNode_t* idCGameLocal::PrependBeamNode( trailBeam_t* tb )
{
    trailBeamNode_t* i;
    
    if( tb->nodes )
    {
        // prepend another node
        i = AllocateBeamNode( tb );
        
        if( i )
        {
            i->next = tb->nodes;
            tb->nodes->prev = i;
            tb->nodes = i;
        }
    }
    else //add first node
    {
        i = AllocateBeamNode( tb );
        
        if( i )
            tb->nodes = i;
    }
    
    return i;
}

/*
===============
CG_AppendBeamNode

Append a new beam node to the back of a beam
Returns the new node
===============
*/
trailBeamNode_t* idCGameLocal::AppendBeamNode( trailBeam_t* tb )
{
    trailBeamNode_t* last, *i;
    
    if( tb->nodes )
    {
        // append another node
        last = FindLastBeamNode( tb );
        i = AllocateBeamNode( tb );
        
        if( i )
        {
            last->next = i;
            i->prev = last;
            i->next = NULL;
        }
    }
    else //add first node
    {
        i = AllocateBeamNode( tb );
        
        if( i )
            tb->nodes = i;
    }
    
    return i;
}

/*
===============
CG_ApplyJitters
===============
*/
void idCGameLocal::ApplyJitters( trailBeam_t* tb )
{
    trailBeamNode_t* i = NULL;
    S32             j;
    baseTrailBeam_t* btb;
    trailSystem_t*   ts;
    trailBeamNode_t* start;
    trailBeamNode_t* end;
    
    if( !tb || !tb->nodes )
        return;
        
    btb = tb->_class;
    ts = tb->parent;
    
    for( j = 0; j < btb->numJitters; j++ )
    {
        if( tb->nextJitterTimes[ j ] <= cg.time )
        {
            for( i = tb->nodes; i; i = i->next )
            {
                i->jitters[ j ][ 0 ] = ( crandom( ) * btb->jitters[ j ].magnitude );
                i->jitters[ j ][ 1 ] = ( crandom( ) * btb->jitters[ j ].magnitude );
            }
            
            tb->nextJitterTimes[ j ] = cg.time + btb->jitters[ j ].period;
        }
    }
    
    start = tb->nodes;
    end = FindLastBeamNode( tb );
    
    if( !btb->jitterAttachments )
    {
        if( Attached( &ts->frontAttachment ) && start->next )
            start = start->next;
            
        if( Attached( &ts->backAttachment ) && end->prev )
            end = end->prev;
    }
    
    for( i = start; i; i = i->next )
    {
        vec3_t          forward, right, up;
        trailBeamNode_t* prev;
        trailBeamNode_t* next;
        F32           upJitter = 0.0f, rightJitter = 0.0f;
        
        prev = i->prev;
        next = i->next;
        
        if( prev && next )
        {
            //this node has two neighbours
            GetPerpendicularViewVector( cg.refdef.vieworg, next->position, prev->position, up );
            VectorSubtract( next->position, prev->position, forward );
        }
        else if( !prev && next )
        {
            //this is the front
            GetPerpendicularViewVector( cg.refdef.vieworg, next->position, i->position, up );
            VectorSubtract( next->position, i->position, forward );
        }
        else if( prev && !next )
        {
            //this is the back
            GetPerpendicularViewVector( cg.refdef.vieworg, i->position, prev->position, up );
            VectorSubtract( i->position, prev->position, forward );
        }
        
        VectorNormalize( forward );
        CrossProduct( forward, up, right );
        VectorNormalize( right );
        
        for( j = 0; j < btb->numJitters; j++ )
        {
            upJitter += i->jitters[ j ][ 0 ];
            rightJitter += i->jitters[ j ][ 1 ];
        }
        
        VectorMA( i->position, upJitter, up, i->position );
        VectorMA( i->position, rightJitter, right, i->position );
        
        if( i == end )
            break;
    }
}

/*
===============
CG_UpdateBeam

Updates a beam
===============
*/
void idCGameLocal::UpdateBeam( trailBeam_t* tb )
{
    baseTrailBeam_t* btb;
    trailSystem_t*   ts;
    trailBeamNode_t* i;
    S32             deltaTime;
    S32             nodesToAdd;
    S32             j;
    S32             numNodes;
    
    if( !tb )
        return;
        
    btb = tb->_class;
    ts = tb->parent;
    
    deltaTime = cg.time - tb->lastEvalTime;
    tb->lastEvalTime = cg.time;
    
    // first make sure this beam has enough nodes
    if( ts->destroyTime <= 0 )
    {
        nodesToAdd = btb->numSegments - CountBeamNodes( tb ) + 1;
        
        while( nodesToAdd-- )
        {
            i = AppendBeamNode( tb );
            
            if( !tb->nodes->next && Attached( &ts->frontAttachment ) )
            {
                // this is the first node to be added
                if( !AttachmentPoint( &ts->frontAttachment, i->refPosition ) )
                    DestroyTrailSystem( &ts );
            }
            else if( i->prev )
                VectorCopy( i->prev->refPosition, i->refPosition );
        }
    }
    
    numNodes = CountBeamNodes( tb );
    
    for( i = tb->nodes; i; i = i->next )
        VectorCopy( i->refPosition, i->position );
        
    if( Attached( &ts->frontAttachment ) && Attached( &ts->backAttachment ) )
    {
        // beam between two attachments
        vec3_t dir, front, back;
        
        if( ts->destroyTime > 0 && ( cg.time - ts->destroyTime ) >= btb->fadeOutTime )
        {
            tb->valid = false;
            return;
        }
        
        if( !AttachmentPoint( &ts->frontAttachment, front ) )
            DestroyTrailSystem( &ts );
            
        if( !AttachmentPoint( &ts->backAttachment, back ) )
            DestroyTrailSystem( &ts );
            
        VectorSubtract( back, front, dir );
        
        for( j = 0, i = tb->nodes; i; i = i->next, j++ )
        {
            F32 scale = ( F32 )j / ( F32 )( numNodes - 1 );
            
            VectorMA( front, scale, dir, i->position );
        }
    }
    else if( Attached( &ts->frontAttachment ) )
    {
        // beam from one attachment
        
        // cull the trail tail
        i = FindLastBeamNode( tb );
        
        if( i && i->timeLeft >= 0 )
        {
            i->timeLeft -= deltaTime;
            
            if( i->timeLeft < 0 )
            {
                tb->nodes = DestroyBeamNode( i );
                
                if( !tb->nodes )
                {
                    tb->valid = false;
                    return;
                }
                
                // if the ts has been destroyed, stop creating new nodes
                if( ts->destroyTime <= 0 )
                    PrependBeamNode( tb );
            }
            else if( i->timeLeft >= 0 && i->prev )
            {
                vec3_t  dir;
                F32   length;
                
                VectorSubtract( i->refPosition, i->prev->refPosition, dir );
                length = VectorNormalize( dir ) *
                         ( ( F32 )i->timeLeft / ( F32 )tb->_class->segmentTime );
                         
                VectorMA( i->prev->refPosition, length, dir, i->position );
            }
        }
        
        if( tb->nodes )
        {
            if( !AttachmentPoint( &ts->frontAttachment, tb->nodes->refPosition ) )
                DestroyTrailSystem( &ts );
                
            VectorCopy( tb->nodes->refPosition, tb->nodes->position );
        }
    }
    
    ApplyJitters( tb );
}

/*
===============
CG_ParseTrailBeamColor
===============
*/
bool idCGameLocal::ParseTrailBeamColor( U8* c, UTF8** text_p )
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
CG_ParseTrailBeam

Parse a trail beam
===============
*/
bool idCGameLocal::ParseTrailBeam( baseTrailBeam_t* btb, UTF8** text_p )
{
    UTF8*  token;
    
    // read optional parameters
    while( 1 )
    {
        token = COM_Parse( text_p );
        
        if( !Q_stricmp( token, "" ) )
            return false;
            
        if( !Q_stricmp( token, "segments" ) )
        {
            token = COM_Parse( text_p );
            if( !Q_stricmp( token, "" ) )
                break;
                
            btb->numSegments = bggame->atoi_neg( token, false );
            
            if( btb->numSegments >= MAX_TRAIL_BEAM_NODES )
            {
                btb->numSegments = MAX_TRAIL_BEAM_NODES - 1;
                Printf( S_COLOR_YELLOW "WARNING: too many segments in trail beam\n" );
            }
            continue;
        }
        else if( !Q_stricmp( token, "width" ) )
        {
            token = COM_Parse( text_p );
            if( !Q_stricmp( token, "" ) )
                break;
                
            btb->frontWidth = bggame->atof_neg( token, false );
            
            token = COM_Parse( text_p );
            if( !Q_stricmp( token, "" ) )
                break;
                
            if( !Q_stricmp( token, "-" ) )
                btb->backWidth = btb->frontWidth;
            else
                btb->backWidth = bggame->atof_neg( token, false );
            continue;
        }
        else if( !Q_stricmp( token, "alpha" ) )
        {
            token = COM_Parse( text_p );
            if( !Q_stricmp( token, "" ) )
                break;
                
            btb->frontAlpha = bggame->atof_neg( token, false );
            
            token = COM_Parse( text_p );
            if( !Q_stricmp( token, "" ) )
                break;
                
            if( !Q_stricmp( token, "-" ) )
                btb->backAlpha = btb->frontAlpha;
            else
                btb->backAlpha = bggame->atof_neg( token, false );
            continue;
        }
        else if( !Q_stricmp( token, "color" ) )
        {
            token = COM_Parse( text_p );
            if( !Q_stricmp( token, "" ) )
                break;
                
            if( !Q_stricmp( token, "{" ) )
            {
                if( !ParseTrailBeamColor( btb->frontColor, text_p ) )
                    break;
                    
                token = COM_Parse( text_p );
                if( Q_stricmp( token, "}" ) )
                {
                    Printf( S_COLOR_RED "ERROR: missing '}'\n" );
                    break;
                }
                
                token = COM_Parse( text_p );
                if( !Q_stricmp( token, "" ) )
                    break;
                    
                if( !Q_stricmp( token, "-" ) )
                {
                    btb->backColor[ 0 ] = btb->frontColor[ 0 ];
                    btb->backColor[ 1 ] = btb->frontColor[ 1 ];
                    btb->backColor[ 2 ] = btb->frontColor[ 2 ];
                }
                else if( !Q_stricmp( token, "{" ) )
                {
                    if( !ParseTrailBeamColor( btb->backColor, text_p ) )
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
        else if( !Q_stricmp( token, "segmentTime" ) )
        {
            token = COM_Parse( text_p );
            if( !Q_stricmp( token, "" ) )
                break;
                
            btb->segmentTime = bggame->atoi_neg( token, false );
            continue;
        }
        else if( !Q_stricmp( token, "fadeOutTime" ) )
        {
            token = COM_Parse( text_p );
            if( !Q_stricmp( token, "" ) )
                break;
                
            btb->fadeOutTime = bggame->atoi_neg( token, false );
            continue;
        }
        else if( !Q_stricmp( token, "shader" ) )
        {
            token = COM_Parse( text_p );
            if( !Q_stricmp( token, "" ) )
                break;
                
            Q_strncpyz( btb->shaderName, token, MAX_QPATH );
            
            continue;
        }
        else if( !Q_stricmp( token, "textureType" ) )
        {
            token = COM_Parse( text_p );
            if( !Q_stricmp( token, "" ) )
                break;
                
            if( !Q_stricmp( token, "stretch" ) )
            {
                btb->textureType = TBTT_STRETCH;
                
                token = COM_Parse( text_p );
                if( !Q_stricmp( token, "" ) )
                    break;
                    
                btb->frontTextureCoord = bggame->atof_neg( token, false );
                
                token = COM_Parse( text_p );
                if( !Q_stricmp( token, "" ) )
                    break;
                    
                btb->backTextureCoord = bggame->atof_neg( token, false );
            }
            else if( !Q_stricmp( token, "repeat" ) )
            {
                btb->textureType = TBTT_REPEAT;
                
                token = COM_Parse( text_p );
                if( !Q_stricmp( token, "" ) )
                    break;
                    
                if( !Q_stricmp( token, "front" ) )
                    btb->clampToBack = false;
                else if( !Q_stricmp( token, "back" ) )
                    btb->clampToBack = true;
                else
                {
                    Printf( S_COLOR_RED "ERROR: unknown textureType clamp \"%s\"\n", token );
                    break;
                }
                
                token = COM_Parse( text_p );
                if( !Q_stricmp( token, "" ) )
                    break;
                    
                btb->repeatLength = bggame->atof_neg( token, false );
            }
            else
            {
                Printf( S_COLOR_RED "ERROR: unknown textureType \"%s\"\n", token );
                break;
            }
            
            continue;
        }
        else if( !Q_stricmp( token, "realLight" ) )
        {
            btb->realLight = true;
            
            continue;
        }
        else if( !Q_stricmp( token, "jitter" ) )
        {
            if( btb->numJitters == MAX_TRAIL_BEAM_JITTERS )
            {
                Printf( S_COLOR_RED "ERROR: too many jitters\n", token );
                break;
            }
            
            token = COM_Parse( text_p );
            if( !Q_stricmp( token, "" ) )
                break;
                
            btb->jitters[ btb->numJitters ].magnitude = bggame->atof_neg( token, false );
            
            token = COM_Parse( text_p );
            if( !Q_stricmp( token, "" ) )
                break;
                
            btb->jitters[ btb->numJitters ].period = bggame->atoi_neg( token, false );
            
            btb->numJitters++;
            
            continue;
        }
        else if( !Q_stricmp( token, "jitterAttachments" ) )
        {
            btb->jitterAttachments = true;
            
            continue;
        }
        else if( !Q_stricmp( token, "}" ) )
            return true; //reached the end of this trail beam
        else
        {
            Printf( S_COLOR_RED "ERROR: unknown token '%s' in trail beam\n", token );
            return false;
        }
    }
    
    return false;
}

/*
===============
CG_InitialiseBaseTrailBeam
===============
*/
void idCGameLocal::InitialiseBaseTrailBeam( baseTrailBeam_t* btb )
{
    ::memset( btb, 0, sizeof( baseTrailBeam_t ) );
    
    btb->numSegments = 1;
    btb->frontWidth = btb->backWidth = 1.0f;
    btb->frontAlpha = btb->backAlpha = 1.0f;
    ::memset( btb->frontColor, 0xFF, sizeof( btb->frontColor ) );
    ::memset( btb->backColor, 0xFF, sizeof( btb->backColor ) );
    
    btb->segmentTime = 100;
    
    btb->textureType = TBTT_STRETCH;
    btb->frontTextureCoord = 0.0f;
    btb->backTextureCoord = 1.0f;
}

/*
===============
CG_ParseTrailSystem

Parse a trail system section
===============
*/
bool idCGameLocal::ParseTrailSystem( baseTrailSystem_t* bts, UTF8** text_p, StringEntry name )
{
    UTF8* token;
    
    // read optional parameters
    while( 1 )
    {
        token = COM_Parse( text_p );
        
        if( !Q_stricmp( token, "" ) )
            return false;
            
        if( !Q_stricmp( token, "{" ) )
        {
            InitialiseBaseTrailBeam( &baseTrailBeams[ numBaseTrailBeams ] );
            
            if( !ParseTrailBeam( &baseTrailBeams[ numBaseTrailBeams ], text_p ) )
            {
                Printf( S_COLOR_RED "ERROR: failed to parse trail beam\n" );
                return false;
            }
            
            if( bts->numBeams == MAX_BEAMS_PER_SYSTEM )
            {
                Printf( S_COLOR_RED "ERROR: trail system has > %d beams\n", MAX_BEAMS_PER_SYSTEM );
                return false;
            }
            else if( numBaseTrailBeams == MAX_BASETRAIL_BEAMS )
            {
                Printf( S_COLOR_RED "ERROR: maximum number of trail beams (%d) reached\n", MAX_BASETRAIL_BEAMS );
                return false;
            }
            else
            {
                //start parsing beams again
                bts->beams[ bts->numBeams ] = &baseTrailBeams[ numBaseTrailBeams ];
                bts->numBeams++;
                numBaseTrailBeams++;
            }
            continue;
        }
        else if( !Q_stricmp( token, "thirdPersonOnly" ) )
            bts->thirdPersonOnly = true;
        else if( !Q_stricmp( token, "lifeTime" ) )
        {
            token = COM_Parse( text_p );
            if( !Q_stricmp( token, "" ) )
                break;
                
            bts->lifeTime = bggame->atoi_neg( token, false );
            continue;
        }
        else if( !Q_stricmp( token, "beam" ) ) //acceptable text
            continue;
        else if( !Q_stricmp( token, "}" ) )
        {
            if( cg_debugTrails.integer >= 1 )
                Printf( "Parsed trail system %s\n", name );
                
            return true; //reached the end of this trail system
        }
        else
        {
            Printf( S_COLOR_RED "ERROR: unknown token '%s' in trail system %s\n", token, bts->name );
            return false;
        }
    }
    
    return false;
}

/*
===============
CG_ParseTrailFile

Load the trail systems from a trail file
===============
*/
bool idCGameLocal::ParseTrailFile( StringEntry fileName )
{
    UTF8*          text_p;
    S32           i;
    S32           len;
    UTF8*          token;
    UTF8          text[ 32000 ];
    UTF8          tsName[ MAX_QPATH ];
    bool      tsNameSet = false;
    fileHandle_t  f;
    
    // load the file
    len = trap_FS_FOpenFile( fileName, &f, FS_READ );
    if( len <= 0 )
        return false;
        
    if( len == 0 || len >= sizeof( text ) - 1 )
    {
        trap_FS_FCloseFile( f );
        Printf( S_COLOR_RED "ERROR: trail file %s is %s\n", fileName, len == 0 ? "empty" : "too long" );
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
            if( tsNameSet )
            {
                //check for name space clashes
                for( i = 0; i < numBaseTrailSystems; i++ )
                {
                    if( !Q_stricmp( baseTrailSystems[ i ].name, tsName ) )
                    {
                        Printf( S_COLOR_RED "ERROR: a trail system is already named %s\n", tsName );
                        return false;
                    }
                }
                
                Q_strncpyz( baseTrailSystems[ numBaseTrailSystems ].name, tsName, MAX_QPATH );
                
                if( !ParseTrailSystem( &baseTrailSystems[ numBaseTrailSystems ], &text_p, tsName ) )
                {
                    Printf( S_COLOR_RED "ERROR: %s: failed to parse trail system %s\n", fileName, tsName );
                    return false;
                }
                
                //start parsing trail systems again
                tsNameSet = false;
                
                if( numBaseTrailSystems == MAX_BASETRAIL_SYSTEMS )
                {
                    Printf( S_COLOR_RED "ERROR: maximum number of trail systems (%d) reached\n", MAX_BASETRAIL_SYSTEMS );
                    return false;
                }
                else
                    numBaseTrailSystems++;
                    
                continue;
            }
            else
            {
                Printf( S_COLOR_RED "ERROR: unamed trail system\n" );
                return false;
            }
        }
        
        if( !tsNameSet )
        {
            Q_strncpyz( tsName, token, sizeof( tsName ) );
            tsNameSet = true;
        }
        else
        {
            Printf( S_COLOR_RED "ERROR: trail system already named\n" );
            return false;
        }
    }
    
    return true;
}

/*
===============
CG_LoadTrailSystems

Load trail system templates
===============
*/
void idCGameLocal::LoadTrailSystems( void )
{
    S32   i, numFiles, fileLen;
    UTF8  fileList[ MAX_TRAIL_FILES * MAX_QPATH ];
    UTF8  fileName[ MAX_QPATH ];
    UTF8*  filePtr;
    
    //clear out the old
    numBaseTrailSystems = 0;
    numBaseTrailBeams = 0;
    
    for( i = 0; i < MAX_BASETRAIL_SYSTEMS; i++ )
    {
        baseTrailSystem_t*  bts = &baseTrailSystems[ i ];
        ::memset( bts, 0, sizeof( baseTrailSystem_t ) );
    }
    
    for( i = 0; i < MAX_BASETRAIL_BEAMS; i++ )
    {
        baseTrailBeam_t*  btb = &baseTrailBeams[ i ];
        ::memset( btb, 0, sizeof( baseTrailBeam_t ) );
    }
    
    //and bring in the new
    numFiles = trap_FS_GetFileList( "scripts", ".trail", fileList, MAX_TRAIL_FILES * MAX_QPATH );
    filePtr = fileList;
    
    for( i = 0; i < numFiles; i++, filePtr += fileLen + 1 )
    {
        fileLen = strlen( filePtr );
        strcpy( fileName, "scripts/" );
        strcat( fileName, filePtr );
        //Printf( "...loading '%s'\n", fileName );
        ParseTrailFile( fileName );
    }
}

/*
===============
CG_RegisterTrailSystem

Load the media that a trail system needs
===============
*/
qhandle_t idCGameLocal::RegisterTrailSystem( UTF8* name )
{
    S32               i, j;
    baseTrailSystem_t* bts;
    baseTrailBeam_t*   btb;
    
    for( i = 0; i < MAX_BASETRAIL_SYSTEMS; i++ )
    {
        bts = &baseTrailSystems[ i ];
        
        if( !Q_stricmp( bts->name, name ) )
        {
            //already registered
            if( bts->registered )
                return i + 1;
                
            for( j = 0; j < bts->numBeams; j++ )
            {
                btb = bts->beams[ j ];
                
                btb->shader = trap_R_RegisterShader( btb->shaderName );
            }
            
            if( cg_debugTrails.integer >= 1 )
                Printf( "Registered trail system %s\n", name );
                
            bts->registered = true;
            
            //avoid returning 0
            return i + 1;
        }
    }
    
    Printf( S_COLOR_RED "ERROR: failed to register trail system %s\n", name );
    return 0;
}


/*
===============
CG_SpawnNewTrailBeam

Allocate a new trail beam
===============
*/
trailBeam_t* idCGameLocal::SpawnNewTrailBeam( baseTrailBeam_t* btb, trailSystem_t* parent )
{
    S32           i;
    trailBeam_t*   tb = NULL;
    trailSystem_t* ts = parent;
    
    for( i = 0; i < MAX_TRAIL_BEAMS; i++ )
    {
        tb = &trailBeams[ i ];
        
        if( !tb->valid )
        {
            ::memset( tb, 0, sizeof( trailBeam_t ) );
            
            //found a free slot
            tb->_class = btb;
            tb->parent = ts;
            
            tb->valid = true;
            
            if( cg_debugTrails.integer >= 1 )
                Printf( "TB %s created\n", ts->_class->name );
                
            break;
        }
    }
    
    return tb;
}


/*
===============
CG_SpawnNewTrailSystem

Spawns a new trail system
===============
*/
trailSystem_t* idCGameLocal::SpawnNewTrailSystem( qhandle_t psHandle )
{
    S32               i, j;
    trailSystem_t*     ts = NULL;
    baseTrailSystem_t* bts = &baseTrailSystems[ psHandle - 1 ];
    
    if( !bts->registered )
    {
        Printf( S_COLOR_RED "ERROR: a trail system has not been registered yet\n" );
        return NULL;
    }
    
    for( i = 0; i < MAX_TRAIL_SYSTEMS; i++ )
    {
        ts = &trailSystems[ i ];
        
        if( !ts->valid )
        {
            ::memset( ts, 0, sizeof( trailSystem_t ) );
            
            //found a free slot
            ts->_class = bts;
            
            ts->valid = true;
            ts->destroyTime = -1;
            ts->birthTime = cg.time;
            
            for( j = 0; j < bts->numBeams; j++ )
                SpawnNewTrailBeam( bts->beams[ j ], ts );
                
            if( cg_debugTrails.integer >= 1 )
                Printf( "TS %s created\n", bts->name );
                
            break;
        }
    }
    
    return ts;
}

/*
===============
CG_DestroyTrailSystem

Destroy a trail system
===============
*/
void idCGameLocal::DestroyTrailSystem( trailSystem_t** ts )
{
    ( *ts )->destroyTime = cg.time;
    
    if( Attached( &( *ts )->frontAttachment ) && !Attached( &( *ts )->backAttachment ) )
    {
        vec3_t v;
        
        // attach the trail head to a static point
        AttachmentPoint( &( *ts )->frontAttachment, v );
        SetAttachmentPoint( &( *ts )->frontAttachment, v );
        AttachToPoint( &( *ts )->frontAttachment );
        
        ( *ts )->frontAttachment.centValid = false; // a bit naughty
    }
    
    ts = NULL;
}

/*
===============
CG_IsTrailSystemValid

Test a trail system for validity
===============
*/
bool idCGameLocal::IsTrailSystemValid( trailSystem_t** ts )
{
    if( *ts == NULL || ( *ts && !( *ts )->valid ) )
    {
        if( *ts && !( *ts )->valid )
            *ts = NULL;
            
        return false;
    }
    
    return true;
}

/*
===============
CG_GarbageCollectTrailSystems

Destroy inactive trail systems
===============
*/
void idCGameLocal::GarbageCollectTrailSystems( void )
{
    S32           i, j, count;
    trailSystem_t* ts;
    trailBeam_t*   tb;
    S32           centNum;
    
    for( i = 0; i < MAX_TRAIL_SYSTEMS; i++ )
    {
        ts = &trailSystems[ i ];
        count = 0;
        
        //don't bother checking already invalid systems
        if( !ts->valid )
            continue;
            
        for( j = 0; j < MAX_TRAIL_BEAMS; j++ )
        {
            tb = &trailBeams[ j ];
            
            if( tb->valid && tb->parent == ts )
                count++;
        }
        
        if( !count )
            ts->valid = false;
            
        //check systems where the parent cent has left the PVS
        //( local player entity is always valid )
        if( ( centNum = AttachmentCentNum( &ts->frontAttachment ) ) >= 0 &&
                centNum != cg.snap->ps.clientNum )
        {
            trailSystem_t* tempTS = ts;
            
            if( !cg_entities[ centNum ].valid )
                DestroyTrailSystem( &tempTS );
        }
        
        if( ( centNum = AttachmentCentNum( &ts->backAttachment ) ) >= 0 &&
                centNum != cg.snap->ps.clientNum )
        {
            trailSystem_t* tempTS = ts;
            
            if( !cg_entities[ centNum ].valid )
                DestroyTrailSystem( &tempTS );
        }
        
        // lifetime expired
        if( ts->destroyTime <= 0 && ts->_class->lifeTime &&
                ts->birthTime + ts->_class->lifeTime < cg.time )
        {
            trailSystem_t* tempTS = ts;
            
            DestroyTrailSystem( &tempTS );
            if( cg_debugTrails.integer >= 1 )
                Printf( "TS %s expired (born %d, lives %d, now %d)\n",
                        ts->_class->name, ts->birthTime, ts->_class->lifeTime,
                        cg.time );
        }
        
        if( cg_debugTrails.integer >= 1 && !ts->valid )
            Printf( "TS %s garbage collected\n", ts->_class->name );
    }
}

/*
===============
CG_AddTrails

Add trails to the scene
===============
*/
void idCGameLocal::AddTrails( void )
{
    S32           i;
    trailBeam_t*   tb;
    S32           numTS = 0, numTB = 0;
    
    //remove expired trail systems
    GarbageCollectTrailSystems( );
    
    for( i = 0; i < MAX_TRAIL_BEAMS; i++ )
    {
        tb = &trailBeams[ i ];
        
        if( tb->valid )
        {
            UpdateBeam( tb );
            RenderBeam( tb );
        }
    }
    
    if( cg_debugTrails.integer >= 2 )
    {
        for( i = 0; i < MAX_TRAIL_SYSTEMS; i++ )
            if( trailSystems[ i ].valid )
                numTS++;
                
        for( i = 0; i < MAX_TRAIL_BEAMS; i++ )
            if( trailBeams[ i ].valid )
                numTB++;
                
        Printf( "TS: %d  TB: %d\n", numTS, numTB );
    }
}

static trailSystem_t*  testTS;
static qhandle_t      testTSHandle;

/*
===============
CG_DestroyTestTS_f

Destroy the test a trail system
===============
*/
void idCGameLocal::DestroyTestTS_f( void )
{
    if( IsTrailSystemValid( &testTS ) )
        DestroyTrailSystem( &testTS );
}

/*
===============
CG_TestTS_f

Test a trail system
===============
*/
void idCGameLocal::TestTS_f( void )
{
    UTF8 tsName[ MAX_QPATH ];
    
    if( trap_Argc( ) < 2 )
        return;
        
    Q_strncpyz( tsName, Argv( 1 ), MAX_QPATH );
    testTSHandle = RegisterTrailSystem( tsName );
    
    if( testTSHandle )
    {
        DestroyTestTS_f( );
        
        testTS = SpawnNewTrailSystem( testTSHandle );
        
        if( IsTrailSystemValid( &testTS ) )
        {
            SetAttachmentCent( &testTS->frontAttachment, &cg_entities[ 0 ] );
            AttachToCent( &testTS->frontAttachment );
        }
    }
}
