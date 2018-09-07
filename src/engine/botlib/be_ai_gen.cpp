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
// File name:   be_ai_gen.cpp
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description: genetic selection
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#ifdef DEDICATED
#include <null/null_precompiled.h>
#else
#include <OWLib/precompiled.h>
#endif

//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
S32 GeneticSelection( S32 numranks, F32* rankings )
{
    F32 sum, select;
    S32 i, index;
    
    sum = 0;
    for( i = 0; i < numranks; i++ )
    {
        if( rankings[i] < 0 ) continue;
        sum += rankings[i];
    } //end for
    if( sum > 0 )
    {
        //select a bot where the ones with the higest rankings have
        //the highest chance of being selected
        select = random() * sum;
        for( i = 0; i < numranks; i++ )
        {
            if( rankings[i] < 0 ) continue;
            sum -= rankings[i];
            if( sum <= 0 ) return i;
        } //end for
    } //end if
    //select a bot randomly
    index = random() * numranks;
    for( i = 0; i < numranks; i++ )
    {
        if( rankings[index] >= 0 ) return index;
        index = ( index + 1 ) % numranks;
    } //end for
    return 0;
} //end of the function GeneticSelection
//===========================================================================
//
// Parameter:			-
// Returns:				-
// Changes Globals:		-
//===========================================================================
S32 GeneticParentsAndChildSelection( S32 numranks, F32* ranks, S32* parent1, S32* parent2, S32* child )
{
    F32 rankings[256], max;
    S32 i;
    
    if( numranks > 256 )
    {
        botimport.Print( PRT_WARNING, "GeneticParentsAndChildSelection: too many bots\n" );
        *parent1 = *parent2 = *child = 0;
        return false;
    } //end if
    for( max = 0, i = 0; i < numranks; i++ )
    {
        if( ranks[i] < 0 ) continue;
        max++;
    } //end for
    if( max < 3 )
    {
        botimport.Print( PRT_WARNING, "GeneticParentsAndChildSelection: too few valid bots\n" );
        *parent1 = *parent2 = *child = 0;
        return false;
    } //end if
    ::memcpy( rankings, ranks, sizeof( F32 ) * numranks );
    //select first parent
    *parent1 = GeneticSelection( numranks, rankings );
    rankings[*parent1] = -1;
    //select second parent
    *parent2 = GeneticSelection( numranks, rankings );
    rankings[*parent2] = -1;
    //reverse the rankings
    max = 0;
    for( i = 0; i < numranks; i++ )
    {
        if( rankings[i] < 0 ) continue;
        if( rankings[i] > max ) max = rankings[i];
    } //end for
    for( i = 0; i < numranks; i++ )
    {
        if( rankings[i] < 0 ) continue;
        rankings[i] = max - rankings[i];
    } //end for
    //select child
    *child = GeneticSelection( numranks, rankings );
    return true;
} //end of the function GeneticParentsAndChildSelection
