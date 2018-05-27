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
// File name:   be_ai_goal.h
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#ifndef __BE_AI_GOAL_H__
#define __BE_AI_GOAL_H__

#define MAX_AVOIDGOALS			256
#define MAX_GOALSTACK			8

#define GFL_NONE				0
#define GFL_ITEM				1
#define GFL_ROAM				2
#define GFL_DROPPED				4

//a bot goal
typedef struct bot_goal_s
{
    vec3_t origin;				//origin of the goal
    S32 areanum;				//area number of the goal
    vec3_t mins, maxs;			//mins and maxs of the goal
    S32 entitynum;				//number of the goal entity
    S32 number;					//goal number
    S32 flags;					//goal flags
    S32 iteminfo;				//item information
} bot_goal_t;

//reset the whole goal state, but keep the item weights
void BotResetGoalState( S32 goalstate );
//reset avoid goals
void BotResetAvoidGoals( S32 goalstate );
//remove the goal with the given number from the avoid goals
void BotRemoveFromAvoidGoals( S32 goalstate, S32 number );
//push a goal onto the goal stack
void BotPushGoal( S32 goalstate, bot_goal_t* goal );
//pop a goal from the goal stack
void BotPopGoal( S32 goalstate );
//empty the bot's goal stack
void BotEmptyGoalStack( S32 goalstate );
//dump the avoid goals
void BotDumpAvoidGoals( S32 goalstate );
//dump the goal stack
void BotDumpGoalStack( S32 goalstate );
//get the name name of the goal with the given number
void BotGoalName( S32 number, UTF8* name, S32 size );
//get the top goal from the stack
S32 BotGetTopGoal( S32 goalstate, bot_goal_t* goal );
//get the second goal on the stack
S32 BotGetSecondGoal( S32 goalstate, bot_goal_t* goal );
//choose the best long term goal item for the bot
S32 BotChooseLTGItem( S32 goalstate, vec3_t origin, S32* inventory, S32 travelflags );
//choose the best nearby goal item for the bot
//the item may not be further away from the current bot position than maxtime
//also the travel time from the nearby goal towards the long term goal may not
//be larger than the travel time towards the long term goal from the current bot position
S32 BotChooseNBGItem( S32 goalstate, vec3_t origin, S32* inventory, S32 travelflags, bot_goal_t* ltg, F32 maxtime );
//returns true if the bot touches the goal
S32 BotTouchingGoal( vec3_t origin, bot_goal_t* goal );
//returns true if the goal should be visible but isn't
S32 BotItemGoalInVisButNotVisible( S32 viewer, vec3_t eye, vec3_t viewangles, bot_goal_t* goal );
//search for a goal for the given classname, the index can be used
//as a start point for the search when multiple goals are available with that same classname
S32 BotGetLevelItemGoal( S32 index, UTF8* classname, bot_goal_t* goal );
//get the next camp spot in the map
S32 BotGetNextCampSpotGoal( S32 num, bot_goal_t* goal );
//get the map location with the given name
S32 BotGetMapLocationGoal( UTF8* name, bot_goal_t* goal );
//returns the avoid goal time
F32 BotAvoidGoalTime( S32 goalstate, S32 number );
//set the avoid goal time
void BotSetAvoidGoalTime( S32 goalstate, S32 number, F32 avoidtime );
//initializes the items in the level
void BotInitLevelItems( void );
//regularly update dynamic entity items (dropped weapons, flags etc.)
void BotUpdateEntityItems( void );
//interbreed the goal fuzzy logic
void BotInterbreedGoalFuzzyLogic( S32 parent1, S32 parent2, S32 child );
//save the goal fuzzy logic to disk
void BotSaveGoalFuzzyLogic( S32 goalstate, UTF8* filename );
//mutate the goal fuzzy logic
void BotMutateGoalFuzzyLogic( S32 goalstate, F32 range );
//loads item weights for the bot
S32 BotLoadItemWeights( S32 goalstate, UTF8* filename );
//frees the item weights of the bot
void BotFreeItemWeights( S32 goalstate );
//returns the handle of a newly allocated goal state
S32 BotAllocGoalState( S32 client );
//free the given goal state
void BotFreeGoalState( S32 handle );
//setup the goal AI
S32 BotSetupGoalAI( void );
//shut down the goal AI
void BotShutdownGoalAI( void );

#endif //!__BE_AI_GOAL_H__