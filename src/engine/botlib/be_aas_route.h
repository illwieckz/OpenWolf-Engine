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
// File name:   be_aas_route.h
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#ifndef __BE_AAS_ROUTE_H__
#define __BE_AAS_ROUTE_H__

#ifdef AASINTERN
//initialize the AAS routing
void AAS_InitRouting( void );
//free the AAS routing caches
void AAS_FreeRoutingCaches( void );
//returns the travel time from start to end in the given area
U16 AAS_AreaTravelTime( S32 areanum, vec3_t start, vec3_t end );
//
void AAS_CreateAllRoutingCache( void );
void AAS_WriteRouteCache( void );
//
void AAS_RoutingInfo( void );
#endif //AASINTERN

//returns the travel flag for the given travel type
S32 AAS_TravelFlagForType( S32 traveltype );
//return the travel flag(s) for traveling through this area
S32 AAS_AreaContentsTravelFlags( S32 areanum );
//returns the index of the next reachability for the given area
S32 AAS_NextAreaReachability( S32 areanum, S32 reachnum );
//returns the reachability with the given index
void AAS_ReachabilityFromNum( S32 num, struct aas_reachability_s* reach );
//returns a random goal area and goal origin
S32 AAS_RandomGoalArea( S32 areanum, S32 travelflags, S32* goalareanum, vec3_t goalorigin );
//enable or disable an area for routing
S32 AAS_EnableRoutingArea( S32 areanum, S32 enable );
//returns the travel time within the given area from start to end
U16 AAS_AreaTravelTime( S32 areanum, vec3_t start, vec3_t end );
//returns the travel time from the area to the goal area using the given travel flags
S32 AAS_AreaTravelTimeToGoalArea( S32 areanum, vec3_t origin, S32 goalareanum, S32 travelflags );
//predict a route up to a stop event
S32 AAS_PredictRoute( struct aas_predictroute_s* route, S32 areanum, vec3_t origin, S32 goalareanum, S32 travelflags, S32 maxareas, S32 maxtime, S32 stopevent, S32 stopcontents, S32 stoptfl, S32 stopareanum );

#endif //!__BE_AAS_ROUTE_H__