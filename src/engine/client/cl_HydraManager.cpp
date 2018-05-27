////////////////////////////////////////////////////////////////////////////////////////
// Copyright(C) 1999 - 2010 id Software LLC, a ZeniMax Media company.
// Copyright(C) 2013 Frederic Lauzon
// Copyright(C) 2016 - 2018 Dusan Jocic <dusanjocic@msn.com>
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
// File name:   cl_HydraManager.cpp
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#include <client/client.h>
#include <client/cl_HydraManager.h>
#include <OVR.h>

using namespace OVR;

// flags that the controller manager system can set to tell the graphics system to draw the instructions
// for the player
static bool controller_manager_screen_visible = true;
std::string controller_manager_text_string;

static void controller_manager_setup_callback( sixenseUtils::ControllerManager::setup_step step )
{
    if( sixenseUtils::getTheControllerManager()->isMenuVisible() )
    {
        controller_manager_screen_visible = true;
        controller_manager_text_string = sixenseUtils::getTheControllerManager()->getStepString();
        
        // We could also load the supplied controllermanager textures using the filename: sixenseUtils::getTheControllerManager()->getTextureFileName();
    }
    else
    {
        // We're done with the setup, so hide the instruction screen.
        controller_manager_screen_visible = false;
    }
}

/*
=====================
idHydraManagerLocal::Initialize
=====================
*/
void idHydraManagerLocal::Initialize()
{
    sixenseInit();
    
    sixenseUtils::ControllerManager* controllerManager = sixenseUtils::getTheControllerManager();
    
    sixenseUtils::getTheControllerManager()->setGameType( sixenseUtils::ControllerManager::ONE_PLAYER_TWO_CONTROLLER );
    sixenseUtils::getTheControllerManager()->registerSetupCallback( controller_manager_setup_callback );
}

/*
=====================
idHydraManagerLocal::Exit
=====================
*/
void idHydraManagerLocal::Exit( void )
{
    sixenseExit();
}

/*
=====================
idHydraManagerLocal::Peak
=====================
*/
S32 idHydraManagerLocal::Peak( S32 hand, F32* joyx, F32* joyy, F32* pos, F32* trigger, F32* yaw, F32* pitch, F32* roll, U32* buttons )
{
    const  sixenseUtils::IControllerManager::controller_desc HandIndex[] =
    {
        sixenseUtils::ControllerManager::P1L, sixenseUtils::ControllerManager::P1R
    };
    static sixenseUtils::ButtonStates btnState;
    
    sixenseSetActiveBase( 0 );
    sixenseGetAllNewestData( &mAcd );
    sixenseUtils::getTheControllerManager()->update( &mAcd );
    
    S32 idx = sixenseUtils::getTheControllerManager()->getIndex( HandIndex[hand] );
    
    sixenseControllerData& ref = mAcd.controllers[idx];
    btnState.update( &ref );
    
    *buttons = ref.buttons;
    
    if( !ref.enabled )
    {
        pos[0] = 0.0f;
        pos[1] = 0.0f;
        pos[2] = 0.0f;
        *joyx  = 0.0f;
        *joyy  = 0.0f;
        *trigger = 0.0f;
        *yaw     = 0.0f;
        *pitch   = 0.0f;
        *roll    = 0.0f;
        return 0;
    }
    
    pos[0] = ref.pos[0];
    pos[1] = ref.pos[1];
    pos[2] = ref.pos[2];
    
    *joyx  = ref.joystick_x;
    *joyy  = ref.joystick_y;
    
    *trigger = ref.trigger;
    
    Quatf orient( ref.rot_quat[0], ref.rot_quat[1], ref.rot_quat[2], ref.rot_quat[3] );
    orient.GetEulerAngles<Axis_Y, Axis_X, Axis_Z>( yaw, pitch, roll );
    
    return 1;
}


/*
=====================
idHydraManagerLocal::isDocked
=====================
*/
bool idHydraManagerLocal::isDocked( S32 controllerIndex ) const
{
    if( sixenseIsControllerEnabled( controllerIndex ) && mAcd.controllers[controllerIndex].is_docked != 1 )
    {
        return false;
    }
    
    return true;
}