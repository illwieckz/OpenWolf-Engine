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
// File name:   cm_traceModel.h
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#ifndef __CM_TRACEMODEL_H__
#define __CM_TRACEMODEL_H__

//
// idTraceModel
//
class idTraceModel
{
public:
    virtual void			Init( S32 entityNum, F32 mass, idVec3 origin, idVec3 angle, idVec3& mins, idVec3& maxs ) = 0;
    virtual void			InitFromModel( const UTF8* qpath, idVec3 origin, idVec3 angle, S32 entityNum, F32 mass ) = 0;
    virtual S32				GetEntityNum() = 0;
    virtual void			SetTransform( idVec3& origin, idVec3& angle ) = 0;
    virtual void			GetTransform( idVec3& xyz, idVec3& angle ) = 0;
    virtual void			SetVelocity( idVec3& vel ) = 0;
    
    virtual F32				GetInverseMass( void ) = 0;
    virtual void			PhysicsApplyCentralImpulse( idVec3& direction ) = 0;
    virtual void			GetUpVectorBasis( idVec3& upVector ) = 0;
    virtual void			SetPhysicsActive( void ) = 0;
};

#endif // !__CM_TRACEMODEL_H__
