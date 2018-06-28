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
// File name:   cl_HydraManager.h
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#ifndef __CL_HYDRAMANAGER_H__
#define __CL_HYDRAMANAGER_H__

#if defined _WIN32 && !defined ( DEDICATED ) && !defined ( UPDATE_SERVER )

#ifndef SIXENSE_UTILS_DERIVATIVES_HPP
#include <sixense_utils/derivatives.hpp>
#endif
#ifndef SIXENSE_UTILS_BUTTON_STATES_HPP
#include <sixense_utils/button_states.hpp>
#endif
#ifndef EVENT_TRIGGERS_HPP
#include <sixense_utils/event_triggers.hpp>
#endif
#ifndef CONTROLLER_MANAGER_HPP
#include <sixense_utils/controller_manager/controller_manager.hpp>
#endif
#ifndef SIXENSE_MATH_HPP
#include <sixense_math.hpp>
#endif

class idHydraManagerLocal
{
public:
    void Initialize();
    void Exit( void );
    
    S32 Peak( S32 hand, F32* joyx, F32* joyy, F32* pos, F32* trigger, F32* yaw, F32* pitch, F32* roll, U32* buttons );
    bool isDocked( S32 controllerIndex ) const;
private:
    sixenseAllControllerData mAcd;
};

extern idHydraManagerLocal HydraManagerLocal;

#endif

#endif //!__CL_HYDRAMANAGER_H__
