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
// File name:   math_quaternation.h
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#ifndef __MATH_QUATERNION_H__
#define __MATH_QUATERNION_H__

#include <assert.h>
#include <math.h>

class idVec3;
class angles_t;
class mat3_t;

class idQuat
{
public:
    float x;
    float y;
    float z;
    float w;
    
    idQuat();
    idQuat( float x, float y, float z, float w );
    
    float*           vec4( void );
    
    float operator[]( int index ) const;
    float&           operator[]( int index );
    
    void            set( float x, float y, float z, float w );
    void Normalize();
    idQuat operator*( idQuat& a ) const;
    friend idQuat operator*( const idQuat q, idVec3& v );
    
    void RotatePoint( const idVec3 in, idVec3& out );
};

ID_INLINE idQuat::idQuat()
{
}

ID_INLINE idQuat::idQuat( float x, float y, float z, float w )
{
    this->x = x;
    this->y = y;
    this->z = z;
    this->w = w;
}

ID_INLINE float* idQuat::vec4( void )
{
    return &x;
}

ID_INLINE float idQuat::operator[]( int index ) const
{
    assert( ( index >= 0 ) && ( index < 4 ) );
    return ( &x )[ index ];
}

ID_INLINE float& idQuat::operator[]( int index )
{
    assert( ( index >= 0 ) && ( index < 4 ) );
    return ( &x )[ index ];
}

ID_INLINE void idQuat::set( float x, float y, float z, float w )
{
    this->x = x;
    this->y = y;
    this->z = z;
    this->w = w;
}

ID_INLINE void idQuat::Normalize()
{
    /* Compute magnitude of the quaternion */
    float mag = sqrt( ( x * x ) + ( y * y )
                      + ( z * z ) + ( w * w ) );
                      
    /* Check for bogus length, to protect against divide by zero */
    if( mag > 0.0f )
    {
        /* Normalize it */
        float oneOverMag = 1.0f / mag;
        
        w *= oneOverMag;
        x *= oneOverMag;
        y *= oneOverMag;
        z *= oneOverMag;
    }
}

ID_INLINE idQuat idQuat::operator*( idQuat& qb ) const
{
    idQuat out;
    out[3] = ( w * qb[3] ) - ( x * qb[0] ) - ( y * qb[1] ) - ( z * qb[2] );
    out[0] = ( x * qb[3] ) + ( w * qb[0] ) + ( y * qb[2] ) - ( z * qb[1] );
    out[1] = ( y * qb[3] ) + ( w * qb[1] ) + ( z * qb[0] ) - ( x * qb[2] );
    out[2] = ( z * qb[3] ) + ( w * qb[2] ) + ( x * qb[1] ) - ( y * qb[0] );
    
    return out;
}

ID_INLINE idQuat operator*( const idQuat q, idVec3& v )
{
    idQuat out;
    out[3] = - ( q.x * v[0] ) - ( q.y * v[1] ) - ( q.z * v[2] );
    out[0] = ( q.w * v[0] ) + ( q.y * v[2] ) - ( q.z * v[1] );
    out[1] = ( q.w * v[1] ) + ( q.z * v[0] ) - ( q.x * v[2] );
    out[2] = ( q.w * v[2] ) + ( q.x * v[1] ) - ( q.y * v[0] );
    
    return out;
}

ID_INLINE void idQuat::RotatePoint( idVec3 in, idVec3& out )
{
    idQuat inv, final, tmp;
    
    inv[0] = -x;
    inv[1] = -y;
    inv[2] = -z;
    inv[3] =  w;
    
    inv.Normalize();
    
    tmp = *this * in;
    final = tmp * inv;
    
    out[0] = final[0];
    out[1] = final[1];
    out[2] = final[2];
}


#endif /* !__MATH_QUATERNION_H__ */
