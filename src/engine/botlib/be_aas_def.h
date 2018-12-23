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
// File name:   be_ass_def.h
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#ifndef __BE_AAS_DEF_H__
#define __BE_AAS_DEF_H__

//debugging on
#define AAS_DEBUG

#define MAX_CLIENTS			64
#define	MAX_MODELS			256		// these are sent over the net as 8 bits
#define	MAX_SOUNDS			256		// so they cannot be blindly increased
#define	MAX_CONFIGSTRINGS	1024

#define	CS_SCORES			32
//#define	CS_MODELS			(CS_SCORES+MAX_CLIENTS)
//#define	CS_SOUNDS			(CS_MODELS+MAX_MODELS)

#define DF_AASENTNUMBER(x)		((x) - aasworld.entities)
#define DF_NUMBERAASENT(x)		(&aasworld.entities[x])
#define DF_AASENTCLIENT(x)		((x) - aasworld.entities - 1)
#define DF_CLIENTAASENT(x)		(&aasworld.entities[(x) + 1])

#ifndef MAX_PATH
#define MAX_PATH				MAX_QPATH
#endif

//string index (for model, sound and image index)
typedef struct aas_stringindex_s
{
    S32 numindexes;
    UTF8** index;
} aas_stringindex_t;

//structure to link entities to areas and areas to entities
typedef struct aas_link_s
{
    S32 entnum;
    S32 areanum;
    struct aas_link_s* next_ent, *prev_ent;
    struct aas_link_s* next_area, *prev_area;
} aas_link_t;

//structure to link entities to leaves and leaves to entities
typedef struct bsp_link_s
{
    S32 entnum;
    S32 leafnum;
    struct bsp_link_s* next_ent, *prev_ent;
    struct bsp_link_s* next_leaf, *prev_leaf;
} bsp_link_t;

typedef struct bsp_entdata_s
{
    vec3_t origin;
    vec3_t angles;
    vec3_t absmins;
    vec3_t absmaxs;
    S32 solid;
    S32 modelnum;
} bsp_entdata_t;

//entity
typedef struct aas_entity_s
{
    //entity info
    aas_entityinfo_t i;
    //links into the AAS areas
    aas_link_t* areas;
    //links into the BSP leaves
    bsp_link_t* leaves;
} aas_entity_t;

typedef struct aas_settings_s
{
    vec3_t phys_gravitydirection;
    F32 phys_friction;
    F32 phys_stopspeed;
    F32 phys_gravity;
    F32 phys_waterfriction;
    F32 phys_watergravity;
    F32 phys_maxvelocity;
    F32 phys_maxwalkvelocity;
    F32 phys_maxcrouchvelocity;
    F32 phys_maxswimvelocity;
    F32 phys_walkaccelerate;
    F32 phys_airaccelerate;
    F32 phys_swimaccelerate;
    F32 phys_maxstep;
    F32 phys_maxsteepness;
    F32 phys_maxwaterjump;
    F32 phys_maxbarrier;
    F32 phys_jumpvel;
    F32 phys_falldelta5;
    F32 phys_falldelta10;
    F32 rs_waterjump;
    F32 rs_teleport;
    F32 rs_barrierjump;
    F32 rs_startcrouch;
    F32 rs_startgrapple;
    F32 rs_startwalkoffledge;
    F32 rs_startjump;
    F32 rs_rocketjump;
    F32 rs_bfgjump;
    F32 rs_jumppad;
    F32 rs_aircontrolledjumppad;
    F32 rs_funcbob;
    F32 rs_startelevator;
    F32 rs_falldamage5;
    F32 rs_falldamage10;
    F32 rs_maxfallheight;
    F32 rs_maxjumpfallheight;
} aas_settings_t;

#define CACHETYPE_PORTAL		0
#define CACHETYPE_AREA			1

//routing cache
typedef struct aas_routingcache_s
{
    U8 type;									//portal or area cache
    F32 time;									//last time accessed or updated
    S32 size;									//size of the routing cache
    S32 cluster;								//cluster the cache is for
    S32 areanum;								//area the cache is created for
    vec3_t origin;								//origin within the area
    F32 starttraveltime;						//travel time to start with
    S32 travelflags;							//combinations of the travel flags
    struct aas_routingcache_s* prev, *next;
    struct aas_routingcache_s* time_prev, *time_next;
    U8* reachabilities;				//reachabilities used for routing
    U16 traveltimes[1];			//travel time for every area (variable sized)
} aas_routingcache_t;

//fields for the routing algorithm
typedef struct aas_routingupdate_s
{
    S32 cluster;
    S32 areanum;								//area number of the update
    vec3_t start;								//start point the area was entered
    U16 tmptraveltime;			//temporary travel time
    U16* areatraveltimes;		//travel times within the area
    bool inlist;							//true if the update is in the list
    struct aas_routingupdate_s* next;
    struct aas_routingupdate_s* prev;
} aas_routingupdate_t;

//reversed reachability link
typedef struct aas_reversedlink_s
{
    S32 linknum;								//the aas_areareachability_t
    S32 areanum;								//reachable from this area
    struct aas_reversedlink_s* next;			//next link
} aas_reversedlink_t;

//reversed area reachability
typedef struct aas_reversedreachability_s
{
    S32 numlinks;
    aas_reversedlink_t* first;
} aas_reversedreachability_t;

//areas a reachability goes through
typedef struct aas_reachabilityareas_s
{
    S32 firstarea, numareas;
} aas_reachabilityareas_t;

typedef struct aas_s
{
    S32 loaded;									//true when an AAS file is loaded
    S32 initialized;							//true when AAS has been initialized
    S32 savefile;								//set true when file should be saved
    S32 bspchecksum;
    //current time
    F32 time;
    S32 numframes;
    //name of the aas file
    UTF8 filename[MAX_PATH];
    UTF8 mapname[MAX_PATH];
    //bounding boxes
    S32 numbboxes;
    aas_bbox_t* bboxes;
    //vertexes
    S32 numvertexes;
    aas_vertex_t* vertexes;
    //planes
    S32 numplanes;
    aas_plane_t* planes;
    //edges
    S32 numedges;
    aas_edge_t* edges;
    //edge index
    S32 edgeindexsize;
    aas_edgeindex_t* edgeindex;
    //faces
    S32 numfaces;
    aas_face_t* faces;
    //face index
    S32 faceindexsize;
    aas_faceindex_t* faceindex;
    //convex areas
    S32 numareas;
    aas_area_t* areas;
    //convex area settings
    S32 numareasettings;
    aas_areasettings_t* areasettings;
    //reachablity list
    S32 reachabilitysize;
    aas_reachability_t* reachability;
    //nodes of the bsp tree
    S32 numnodes;
    aas_node_t* nodes;
    //cluster portals
    S32 numportals;
    aas_portal_t* portals;
    //cluster portal index
    S32 portalindexsize;
    aas_portalindex_t* portalindex;
    //clusters
    S32 numclusters;
    aas_cluster_t* clusters;
    //
    S32 numreachabilityareas;
    F32 reachabilitytime;
    //enities linked in the areas
    aas_link_t* linkheap;						//heap with link structures
    S32 linkheapsize;							//size of the link heap
    aas_link_t* freelinks;						//first free link
    aas_link_t** arealinkedentities;			//entities linked into areas
    //entities
    S32 maxentities;
    S32 maxclients;
    aas_entity_t* entities;
    //string indexes
    UTF8* configstrings[MAX_CONFIGSTRINGS];
    S32 indexessetup;
    //index to retrieve travel flag for a travel type
    S32 travelflagfortype[MAX_TRAVELTYPES];
    //travel flags for each area based on contents
    S32* areacontentstravelflags;
    //routing update
    aas_routingupdate_t* areaupdate;
    aas_routingupdate_t* portalupdate;
    //number of routing updates during a frame (reset every frame)
    S32 frameroutingupdates;
    //reversed reachability links
    aas_reversedreachability_t* reversedreachability;
    //travel times within the areas
    U16*** areatraveltimes;
    //array of size numclusters with cluster cache
    aas_routingcache_t*** clusterareacache;
    aas_routingcache_t** portalcache;
    //cache list sorted on time
    aas_routingcache_t* oldestcache;		// start of cache list sorted on time
    aas_routingcache_t* newestcache;		// end of cache list sorted on time
    //maximum travel time through portal areas
    S32* portalmaxtraveltimes;
    //areas the reachabilities go through
    S32* reachabilityareaindex;
    aas_reachabilityareas_t* reachabilityareas;
} aas_t;

#define AASINTERN

#ifndef BSPCINCLUDE

#include "be_aas_main.h"
#include "be_aas_entity.h"
#include "be_aas_sample.h"
#include "be_aas_cluster.h"
#include "be_aas_reach.h"
#include "be_aas_route.h"
#include "be_aas_routealt.h"
#include "be_aas_debug.h"
#include "be_aas_file.h"
#include "be_aas_optimize.h"
#include "be_aas_bsp.h"
#include "be_aas_move.h"

#endif //BSPCINCLUDE

#endif //__BE_AAS_DEF_H__
