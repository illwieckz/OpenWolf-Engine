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
// File name:   be_ai_weight.h
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#ifndef __BE_AI_WEIGHT_H__
#define __BE_AI_WEIGHT_H__

#define WT_BALANCE			1
#define MAX_WEIGHTS			128

//fuzzy seperator
typedef struct fuzzyseperator_s
{
    S32 index;
    S32 value;
    S32 type;
    F32 weight;
    F32 minweight;
    F32 maxweight;
    struct fuzzyseperator_s* child;
    struct fuzzyseperator_s* next;
} fuzzyseperator_t;

//fuzzy weight
typedef struct weight_s
{
    UTF8* name;
    struct fuzzyseperator_s* firstseperator;
} weight_t;

//weight configuration
typedef struct weightconfig_s
{
    S32 numweights;
    weight_t weights[MAX_WEIGHTS];
    UTF8		filename[MAX_QPATH];
} weightconfig_t;

//reads a weight configuration
weightconfig_t* ReadWeightConfig( UTF8* filename );
//free a weight configuration
void FreeWeightConfig( weightconfig_t* config );
//writes a weight configuration, returns true if successfull
bool WriteWeightConfig( UTF8* filename, weightconfig_t* config );
//find the fuzzy weight with the given name
S32 FindFuzzyWeight( weightconfig_t* wc, UTF8* name );
//returns the fuzzy weight for the given inventory and weight
F32 FuzzyWeight( S32* inventory, weightconfig_t* wc, S32 weightnum );
F32 FuzzyWeightUndecided( S32* inventory, weightconfig_t* wc, S32 weightnum );
//scales the weight with the given name
void ScaleWeight( weightconfig_t* config, UTF8* name, F32 scale );
//scale the balance range
void ScaleBalanceRange( weightconfig_t* config, F32 scale );
//evolves the weight configuration
void EvolveWeightConfig( weightconfig_t* config );
//interbreed the weight configurations and stores the interbreeded one in configout
void InterbreedWeightConfigs( weightconfig_t* config1, weightconfig_t* config2, weightconfig_t* configout );
//frees cached weight configurations
void BotShutdownWeights( void );

#endif //!__BE_AI_WEIGHT_H__