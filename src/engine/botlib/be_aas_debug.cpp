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
// File name:   be_aas_debug.cpp
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description: AAS debug code
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#include <qcommon/q_shared.h>
#include <botlib/l_memory.h>
#include <botlib/l_script.h>
#include <botlib/l_precomp.h>
#include <botlib/l_struct.h>
#include <botlib/l_libvar.h>
#include <botlib/aasfile.h>
#include <botlib/botlib.h>
#include <botlib/be_aas.h>
#include <botlib/be_interface.h>
#include <botlib/be_aas_funcs.h>
#include <botlib/be_aas_def.h>

#define MAX_DEBUGLINES				1024
#define MAX_DEBUGPOLYGONS			8192

S32 debuglines[MAX_DEBUGLINES];
S32 debuglinevisible[MAX_DEBUGLINES];
S32 numdebuglines;

static S32 debugpolygons[MAX_DEBUGPOLYGONS];

//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_ClearShownPolygons( void )
{
    S32 i;
//*
    for( i = 0; i < MAX_DEBUGPOLYGONS; i++ )
    {
        if( debugpolygons[i] ) botimport.DebugPolygonDelete( debugpolygons[i] );
        debugpolygons[i] = 0;
    } //end for
//*/
    /*
    	for (i = 0; i < MAX_DEBUGPOLYGONS; i++)
    	{
    		botimport.DebugPolygonDelete(i);
    		debugpolygons[i] = 0;
    	} //end for
    */
} //end of the function AAS_ClearShownPolygons
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_ShowPolygon( S32 color, S32 numpoints, vec3_t* points )
{
    S32 i;
    
    for( i = 0; i < MAX_DEBUGPOLYGONS; i++ )
    {
        if( !debugpolygons[i] )
        {
            debugpolygons[i] = botimport.DebugPolygonCreate( color, numpoints, points );
            break;
        } //end if
    } //end for
} //end of the function AAS_ShowPolygon
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_ClearShownDebugLines( void )
{
    S32 i;
    
    //make all lines invisible
    for( i = 0; i < MAX_DEBUGLINES; i++ )
    {
        if( debuglines[i] )
        {
            //botimport.DebugLineShow(debuglines[i], NULL, NULL, LINECOLOR_NONE);
            botimport.DebugLineDelete( debuglines[i] );
            debuglines[i] = 0;
            debuglinevisible[i] = false;
        } //end if
    } //end for
} //end of the function AAS_ClearShownDebugLines
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_DebugLine( vec3_t start, vec3_t end, S32 color )
{
    S32 line;
    
    for( line = 0; line < MAX_DEBUGLINES; line++ )
    {
        if( !debuglines[line] )
        {
            debuglines[line] = botimport.DebugLineCreate();
            debuglinevisible[line] = false;
            numdebuglines++;
        } //end if
        if( !debuglinevisible[line] )
        {
            botimport.DebugLineShow( debuglines[line], start, end, color );
            debuglinevisible[line] = true;
            return;
        } //end else
    } //end for
} //end of the function AAS_DebugLine
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void AAS_PermanentLine( vec3_t start, vec3_t end, S32 color )
{
    S32 line;
    
    line = botimport.DebugLineCreate();
    botimport.DebugLineShow( line, start, end, color );
} //end of the function AAS_PermenentLine
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
void AAS_DrawPermanentCross( vec3_t origin, F32 size, S32 color )
{
    S32 i, debugline;
    vec3_t start, end;
    
    for( i = 0; i < 3; i++ )
    {
        VectorCopy( origin, start );
        start[i] += size;
        VectorCopy( origin, end );
        end[i] -= size;
        AAS_DebugLine( start, end, color );
        debugline = botimport.DebugLineCreate();
        botimport.DebugLineShow( debugline, start, end, color );
    } //end for
} //end of the function AAS_DrawPermanentCross
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_DrawPlaneCross( vec3_t point, vec3_t normal, F32 dist, S32 type, S32 color )
{
    S32 n0, n1, n2, j, line, lines[2];
    vec3_t start1, end1, start2, end2;
    
    //make a cross in the hit plane at the hit point
    VectorCopy( point, start1 );
    VectorCopy( point, end1 );
    VectorCopy( point, start2 );
    VectorCopy( point, end2 );
    
    n0 = type % 3;
    n1 = ( type + 1 ) % 3;
    n2 = ( type + 2 ) % 3;
    start1[n1] -= 6;
    start1[n2] -= 6;
    end1[n1] += 6;
    end1[n2] += 6;
    start2[n1] += 6;
    start2[n2] -= 6;
    end2[n1] -= 6;
    end2[n2] += 6;
    
    start1[n0] = ( dist - ( start1[n1] * normal[n1] +
                            start1[n2] * normal[n2] ) ) / normal[n0];
    end1[n0] = ( dist - ( end1[n1] * normal[n1] +
                          end1[n2] * normal[n2] ) ) / normal[n0];
    start2[n0] = ( dist - ( start2[n1] * normal[n1] +
                            start2[n2] * normal[n2] ) ) / normal[n0];
    end2[n0] = ( dist - ( end2[n1] * normal[n1] +
                          end2[n2] * normal[n2] ) ) / normal[n0];
                          
    for( j = 0, line = 0; j < 2 && line < MAX_DEBUGLINES; line++ )
    {
        if( !debuglines[line] )
        {
            debuglines[line] = botimport.DebugLineCreate();
            lines[j++] = debuglines[line];
            debuglinevisible[line] = true;
            numdebuglines++;
        } //end if
        else if( !debuglinevisible[line] )
        {
            lines[j++] = debuglines[line];
            debuglinevisible[line] = true;
        } //end else
    } //end for
    botimport.DebugLineShow( lines[0], start1, end1, color );
    botimport.DebugLineShow( lines[1], start2, end2, color );
} //end of the function AAS_DrawPlaneCross
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_ShowBoundingBox( vec3_t origin, vec3_t mins, vec3_t maxs )
{
    vec3_t bboxcorners[8];
    S32 lines[3];
    S32 i, j, line;
    
    //upper corners
    bboxcorners[0][0] = origin[0] + maxs[0];
    bboxcorners[0][1] = origin[1] + maxs[1];
    bboxcorners[0][2] = origin[2] + maxs[2];
    //
    bboxcorners[1][0] = origin[0] + mins[0];
    bboxcorners[1][1] = origin[1] + maxs[1];
    bboxcorners[1][2] = origin[2] + maxs[2];
    //
    bboxcorners[2][0] = origin[0] + mins[0];
    bboxcorners[2][1] = origin[1] + mins[1];
    bboxcorners[2][2] = origin[2] + maxs[2];
    //
    bboxcorners[3][0] = origin[0] + maxs[0];
    bboxcorners[3][1] = origin[1] + mins[1];
    bboxcorners[3][2] = origin[2] + maxs[2];
    //lower corners
    ::memcpy( bboxcorners[4], bboxcorners[0], sizeof( vec3_t ) * 4 );
    for( i = 0; i < 4; i++ ) bboxcorners[4 + i][2] = origin[2] + mins[2];
    //draw bounding box
    for( i = 0; i < 4; i++ )
    {
        for( j = 0, line = 0; j < 3 && line < MAX_DEBUGLINES; line++ )
        {
            if( !debuglines[line] )
            {
                debuglines[line] = botimport.DebugLineCreate();
                lines[j++] = debuglines[line];
                debuglinevisible[line] = true;
                numdebuglines++;
            } //end if
            else if( !debuglinevisible[line] )
            {
                lines[j++] = debuglines[line];
                debuglinevisible[line] = true;
            } //end else
        } //end for
        //top plane
        botimport.DebugLineShow( lines[0], bboxcorners[i],
                                 bboxcorners[( i + 1 ) & 3], LINECOLOR_RED );
        //bottom plane
        botimport.DebugLineShow( lines[1], bboxcorners[4 + i],
                                 bboxcorners[4 + ( ( i + 1 ) & 3 )], LINECOLOR_RED );
        //vertical lines
        botimport.DebugLineShow( lines[2], bboxcorners[i],
                                 bboxcorners[4 + i], LINECOLOR_RED );
    } //end for
} //end of the function AAS_ShowBoundingBox
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_ShowFace( S32 facenum )
{
    S32 i, color, edgenum;
    aas_edge_t* edge;
    aas_face_t* face;
    aas_plane_t* plane;
    vec3_t start, end;
    
    color = LINECOLOR_YELLOW;
    //check if face number is in range
    if( facenum >= aasworld.numfaces )
    {
        botimport.Print( PRT_ERROR, "facenum %d out of range\n", facenum );
    } //end if
    face = &aasworld.faces[facenum];
    //walk through the edges of the face
    for( i = 0; i < face->numedges; i++ )
    {
        //edge number
        edgenum = abs( aasworld.edgeindex[face->firstedge + i] );
        //check if edge number is in range
        if( edgenum >= aasworld.numedges )
        {
            botimport.Print( PRT_ERROR, "edgenum %d out of range\n", edgenum );
        } //end if
        edge = &aasworld.edges[edgenum];
        if( color == LINECOLOR_RED ) color = LINECOLOR_GREEN;
        else if( color == LINECOLOR_GREEN ) color = LINECOLOR_BLUE;
        else if( color == LINECOLOR_BLUE ) color = LINECOLOR_YELLOW;
        else color = LINECOLOR_RED;
        AAS_DebugLine( aasworld.vertexes[edge->v[0]],
                       aasworld.vertexes[edge->v[1]],
                       color );
    } //end for
    plane = &aasworld.planes[face->planenum];
    edgenum = abs( aasworld.edgeindex[face->firstedge] );
    edge = &aasworld.edges[edgenum];
    VectorCopy( aasworld.vertexes[edge->v[0]], start );
    VectorMA( start, 20, plane->normal, end );
    AAS_DebugLine( start, end, LINECOLOR_RED );
} //end of the function AAS_ShowFace
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_ShowFacePolygon( S32 facenum, S32 color, S32 flip )
{
    S32 i, edgenum, numpoints;
    vec3_t points[128];
    aas_edge_t* edge;
    aas_face_t* face;
    
    //check if face number is in range
    if( facenum >= aasworld.numfaces )
    {
        botimport.Print( PRT_ERROR, "facenum %d out of range\n", facenum );
    } //end if
    face = &aasworld.faces[facenum];
    //walk through the edges of the face
    numpoints = 0;
    if( flip )
    {
        for( i = face->numedges - 1; i >= 0; i-- )
        {
            //edge number
            edgenum = aasworld.edgeindex[face->firstedge + i];
            edge = &aasworld.edges[abs( edgenum )];
            VectorCopy( aasworld.vertexes[edge->v[edgenum < 0]], points[numpoints] );
            numpoints++;
        } //end for
    } //end if
    else
    {
        for( i = 0; i < face->numedges; i++ )
        {
            //edge number
            edgenum = aasworld.edgeindex[face->firstedge + i];
            edge = &aasworld.edges[abs( edgenum )];
            VectorCopy( aasworld.vertexes[edge->v[edgenum < 0]], points[numpoints] );
            numpoints++;
        } //end for
    } //end else
    AAS_ShowPolygon( color, numpoints, points );
} //end of the function AAS_ShowFacePolygon
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_ShowArea( S32 areanum, S32 groundfacesonly )
{
    S32 areaedges[MAX_DEBUGLINES];
    S32 numareaedges, i, j, n, color = 0, line;
    S32 facenum, edgenum;
    aas_area_t* area;
    aas_face_t* face;
    aas_edge_t* edge;
    
    //
    numareaedges = 0;
    //
    if( areanum < 0 || areanum >= aasworld.numareas )
    {
        botimport.Print( PRT_ERROR, "area %d out of range [0, %d]\n",
                         areanum, aasworld.numareas );
        return;
    } //end if
    //pointer to the convex area
    area = &aasworld.areas[areanum];
    //walk through the faces of the area
    for( i = 0; i < area->numfaces; i++ )
    {
        facenum = abs( aasworld.faceindex[area->firstface + i] );
        //check if face number is in range
        if( facenum >= aasworld.numfaces )
        {
            botimport.Print( PRT_ERROR, "facenum %d out of range\n", facenum );
        } //end if
        face = &aasworld.faces[facenum];
        //ground faces only
        if( groundfacesonly )
        {
            if( !( face->faceflags & ( FACE_GROUND | FACE_LADDER ) ) ) continue;
        } //end if
        //walk through the edges of the face
        for( j = 0; j < face->numedges; j++ )
        {
            //edge number
            edgenum = abs( aasworld.edgeindex[face->firstedge + j] );
            //check if edge number is in range
            if( edgenum >= aasworld.numedges )
            {
                botimport.Print( PRT_ERROR, "edgenum %d out of range\n", edgenum );
            } //end if
            //check if the edge is stored already
            for( n = 0; n < numareaedges; n++ )
            {
                if( areaedges[n] == edgenum ) break;
            } //end for
            if( n == numareaedges && numareaedges < MAX_DEBUGLINES )
            {
                areaedges[numareaedges++] = edgenum;
            } //end if
        } //end for
        //AAS_ShowFace(facenum);
    } //end for
    //draw all the edges
    for( n = 0; n < numareaedges; n++ )
    {
        for( line = 0; line < MAX_DEBUGLINES; line++ )
        {
            if( !debuglines[line] )
            {
                debuglines[line] = botimport.DebugLineCreate();
                debuglinevisible[line] = false;
                numdebuglines++;
            } //end if
            if( !debuglinevisible[line] )
            {
                break;
            } //end else
        } //end for
        if( line >= MAX_DEBUGLINES ) return;
        edge = &aasworld.edges[areaedges[n]];
        if( color == LINECOLOR_RED ) color = LINECOLOR_BLUE;
        else if( color == LINECOLOR_BLUE ) color = LINECOLOR_GREEN;
        else if( color == LINECOLOR_GREEN ) color = LINECOLOR_YELLOW;
        else color = LINECOLOR_RED;
        botimport.DebugLineShow( debuglines[line],
                                 aasworld.vertexes[edge->v[0]],
                                 aasworld.vertexes[edge->v[1]],
                                 color );
        debuglinevisible[line] = true;
    } //end for*/
} //end of the function AAS_ShowArea
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_ShowAreaPolygons( S32 areanum, S32 color, S32 groundfacesonly )
{
    S32 i, facenum;
    aas_area_t* area;
    aas_face_t* face;
    
    //
    if( areanum < 0 || areanum >= aasworld.numareas )
    {
        botimport.Print( PRT_ERROR, "area %d out of range [0, %d]\n",
                         areanum, aasworld.numareas );
        return;
    } //end if
    //pointer to the convex area
    area = &aasworld.areas[areanum];
    //walk through the faces of the area
    for( i = 0; i < area->numfaces; i++ )
    {
        facenum = abs( aasworld.faceindex[area->firstface + i] );
        //check if face number is in range
        if( facenum >= aasworld.numfaces )
        {
            botimport.Print( PRT_ERROR, "facenum %d out of range\n", facenum );
        } //end if
        face = &aasworld.faces[facenum];
        //ground faces only
        if( groundfacesonly )
        {
            if( !( face->faceflags & ( FACE_GROUND | FACE_LADDER ) ) ) continue;
        } //end if
        AAS_ShowFacePolygon( facenum, color, face->frontarea != areanum );
    } //end for
} //end of the function AAS_ShowAreaPolygons
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_DrawCross( vec3_t origin, F32 size, S32 color )
{
    S32 i;
    vec3_t start, end;
    
    for( i = 0; i < 3; i++ )
    {
        VectorCopy( origin, start );
        start[i] += size;
        VectorCopy( origin, end );
        end[i] -= size;
        AAS_DebugLine( start, end, color );
    } //end for
} //end of the function AAS_DrawCross
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_PrintTravelType( S32 traveltype )
{
#ifdef DEBUG
    UTF8* str;
    //
    switch( traveltype & TRAVELTYPE_MASK )
    {
        case TRAVEL_INVALID:
            str = "TRAVEL_INVALID";
            break;
        case TRAVEL_WALK:
            str = "TRAVEL_WALK";
            break;
        case TRAVEL_CROUCH:
            str = "TRAVEL_CROUCH";
            break;
        case TRAVEL_BARRIERJUMP:
            str = "TRAVEL_BARRIERJUMP";
            break;
        case TRAVEL_JUMP:
            str = "TRAVEL_JUMP";
            break;
        case TRAVEL_LADDER:
            str = "TRAVEL_LADDER";
            break;
        case TRAVEL_WALKOFFLEDGE:
            str = "TRAVEL_WALKOFFLEDGE";
            break;
        case TRAVEL_SWIM:
            str = "TRAVEL_SWIM";
            break;
        case TRAVEL_WATERJUMP:
            str = "TRAVEL_WATERJUMP";
            break;
        case TRAVEL_TELEPORT:
            str = "TRAVEL_TELEPORT";
            break;
        case TRAVEL_ELEVATOR:
            str = "TRAVEL_ELEVATOR";
            break;
        case TRAVEL_ROCKETJUMP:
            str = "TRAVEL_ROCKETJUMP";
            break;
        case TRAVEL_BFGJUMP:
            str = "TRAVEL_BFGJUMP";
            break;
        case TRAVEL_GRAPPLEHOOK:
            str = "TRAVEL_GRAPPLEHOOK";
            break;
        case TRAVEL_JUMPPAD:
            str = "TRAVEL_JUMPPAD";
            break;
        case TRAVEL_FUNCBOB:
            str = "TRAVEL_FUNCBOB";
            break;
        default:
            str = "UNKNOWN TRAVEL TYPE";
            break;
    } //end switch
    botimport.Print( PRT_MESSAGE, "%s", str );
#endif
} //end of the function AAS_PrintTravelType
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_DrawArrow( vec3_t start, vec3_t end, S32 linecolor, S32 arrowcolor )
{
    vec3_t dir, cross, p1, p2, up = {0, 0, 1};
    F32 dot;
    
    VectorSubtract( end, start, dir );
    VectorNormalize( dir );
    dot = DotProduct( dir, up );
    if( dot > 0.99 || dot < -0.99 ) VectorSet( cross, 1, 0, 0 );
    else CrossProduct( dir, up, cross );
    
    VectorMA( end, -6, dir, p1 );
    VectorCopy( p1, p2 );
    VectorMA( p1, 6, cross, p1 );
    VectorMA( p2, -6, cross, p2 );
    
    AAS_DebugLine( start, end, linecolor );
    AAS_DebugLine( p1, end, arrowcolor );
    AAS_DebugLine( p2, end, arrowcolor );
} //end of the function AAS_DrawArrow
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_ShowReachability( aas_reachability_t* reach )
{
    vec3_t dir, cmdmove, velocity;
    F32 speed, zvel;
    aas_clientmove_t move;
    
    AAS_ShowAreaPolygons( reach->areanum, 5, true );
    //AAS_ShowArea(reach->areanum, true);
    AAS_DrawArrow( reach->start, reach->end, LINECOLOR_BLUE, LINECOLOR_YELLOW );
    //
    if( ( reach->traveltype & TRAVELTYPE_MASK ) == TRAVEL_JUMP ||
            ( reach->traveltype & TRAVELTYPE_MASK ) == TRAVEL_WALKOFFLEDGE )
    {
        AAS_HorizontalVelocityForJump( aassettings.phys_jumpvel, reach->start, reach->end, &speed );
        //
        VectorSubtract( reach->end, reach->start, dir );
        dir[2] = 0;
        VectorNormalize( dir );
        //set the velocity
        VectorScale( dir, speed, velocity );
        //set the command movement
        VectorClear( cmdmove );
        cmdmove[2] = aassettings.phys_jumpvel;
        //
        AAS_PredictClientMovement( &move, -1, reach->start, PRESENCE_NORMAL, true,
                                   velocity, cmdmove, 3, 30, 0.1f,
                                   SE_HITGROUND | SE_ENTERWATER | SE_ENTERSLIME |
                                   SE_ENTERLAVA | SE_HITGROUNDDAMAGE, 0, true );
        //
        if( ( reach->traveltype & TRAVELTYPE_MASK ) == TRAVEL_JUMP )
        {
            AAS_JumpReachRunStart( reach, dir );
            AAS_DrawCross( dir, 4, LINECOLOR_BLUE );
        } //end if
    } //end if
    else if( ( reach->traveltype & TRAVELTYPE_MASK ) == TRAVEL_ROCKETJUMP )
    {
        zvel = AAS_RocketJumpZVelocity( reach->start );
        AAS_HorizontalVelocityForJump( zvel, reach->start, reach->end, &speed );
        //
        VectorSubtract( reach->end, reach->start, dir );
        dir[2] = 0;
        VectorNormalize( dir );
        //get command movement
        VectorScale( dir, speed, cmdmove );
        VectorSet( velocity, 0, 0, zvel );
        //
        AAS_PredictClientMovement( &move, -1, reach->start, PRESENCE_NORMAL, true,
                                   velocity, cmdmove, 30, 30, 0.1f,
                                   SE_ENTERWATER | SE_ENTERSLIME |
                                   SE_ENTERLAVA | SE_HITGROUNDDAMAGE |
                                   SE_TOUCHJUMPPAD | SE_HITGROUNDAREA, reach->areanum, true );
    } //end else if
    else if( ( reach->traveltype & TRAVELTYPE_MASK ) == TRAVEL_JUMPPAD )
    {
        VectorSet( cmdmove, 0, 0, 0 );
        //
        VectorSubtract( reach->end, reach->start, dir );
        dir[2] = 0;
        VectorNormalize( dir );
        //set the velocity
        //NOTE: the edgenum is the horizontal velocity
        VectorScale( dir, reach->edgenum, velocity );
        //NOTE: the facenum is the Z velocity
        velocity[2] = reach->facenum;
        //
        AAS_PredictClientMovement( &move, -1, reach->start, PRESENCE_NORMAL, true,
                                   velocity, cmdmove, 30, 30, 0.1f,
                                   SE_ENTERWATER | SE_ENTERSLIME |
                                   SE_ENTERLAVA | SE_HITGROUNDDAMAGE |
                                   SE_TOUCHJUMPPAD | SE_HITGROUNDAREA, reach->areanum, true );
    } //end else if
} //end of the function AAS_ShowReachability
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AAS_ShowReachableAreas( S32 areanum )
{
    aas_areasettings_t* settings;
    static aas_reachability_t reach;
    static S32 index, lastareanum;
    static F32 lasttime;
    
    if( areanum != lastareanum )
    {
        index = 0;
        lastareanum = areanum;
    } //end if
    settings = &aasworld.areasettings[areanum];
    //
    if( !settings->numreachableareas ) return;
    //
    if( index >= settings->numreachableareas ) index = 0;
    //
    if( AAS_Time() - lasttime > 1.5 )
    {
        ::memcpy( &reach, &aasworld.reachability[settings->firstreachablearea + index], sizeof( aas_reachability_t ) );
        index++;
        lasttime = AAS_Time();
        AAS_PrintTravelType( reach.traveltype & TRAVELTYPE_MASK );
        botimport.Print( PRT_MESSAGE, "\n" );
    } //end if
    AAS_ShowReachability( &reach );
} //end of the function ShowReachableAreas

void AAS_FloodAreas_r( S32 areanum, S32 cluster, S32* done )
{
    S32 nextareanum, i, facenum;
    aas_area_t* area;
    aas_face_t* face;
    aas_areasettings_t* settings;
    aas_reachability_t* reach;
    
    AAS_ShowAreaPolygons( areanum, 1, true );
    //pointer to the convex area
    area = &aasworld.areas[areanum];
    settings = &aasworld.areasettings[areanum];
    //walk through the faces of the area
    for( i = 0; i < area->numfaces; i++ )
    {
        facenum = abs( aasworld.faceindex[area->firstface + i] );
        face = &aasworld.faces[facenum];
        if( face->frontarea == areanum )
            nextareanum = face->backarea;
        else
            nextareanum = face->frontarea;
        if( !nextareanum )
            continue;
        if( done[nextareanum] )
            continue;
        done[nextareanum] = true;
        if( aasworld.areasettings[nextareanum].contents & AREACONTENTS_VIEWPORTAL )
            continue;
        if( AAS_AreaCluster( nextareanum ) != cluster )
            continue;
        AAS_FloodAreas_r( nextareanum, cluster, done );
    } //end for
    //
    for( i = 0; i < settings->numreachableareas; i++ )
    {
        reach = &aasworld.reachability[settings->firstreachablearea + i];
        nextareanum = reach->areanum;
        if( !nextareanum )
            continue;
        if( done[nextareanum] )
            continue;
        done[nextareanum] = true;
        if( aasworld.areasettings[nextareanum].contents & AREACONTENTS_VIEWPORTAL )
            continue;
        if( AAS_AreaCluster( nextareanum ) != cluster )
            continue;
        /*
        if ((reach->traveltype & TRAVELTYPE_MASK) == TRAVEL_WALKOFFLEDGE)
        {
        	AAS_DebugLine(reach->start, reach->end, 1);
        }
        */
        AAS_FloodAreas_r( nextareanum, cluster, done );
    }
}

void AAS_FloodAreas( vec3_t origin )
{
    S32 areanum, cluster, *done;
    
    done = ( S32* ) GetClearedMemory( aasworld.numareas * sizeof( S32 ) );
    areanum = AAS_PointAreaNum( origin );
    cluster = AAS_AreaCluster( areanum );
    AAS_FloodAreas_r( areanum, cluster, done );
}