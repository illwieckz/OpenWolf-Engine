////////////////////////////////////////////////////////////////////////////////////////
// Copyright(C) 2011 JV Software
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
// File name:   physics_debug.cpp
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#include <OWLIb/precompiled.h>

#if !defined ( Q3MAP2 ) && !defined ( DEDICATED )

GLDebugDrawer::GLDebugDrawer()
    : m_debugMode( 0 )
{

}

void GLDebugDrawer::drawLine( const btVector3& from, const btVector3& to, const btVector3& fromColor, const btVector3& toColor )
{
    glBegin( GL_LINES );
    glColor3f( fromColor.getX(), fromColor.getY(), fromColor.getZ() );
    glVertex3d( from.getX(), from.getY(), from.getZ() );
    glColor3f( toColor.getX(), toColor.getY(), toColor.getZ() );
    glVertex3d( to.getX(), to.getY(), to.getZ() );
    glEnd();
}

void GLDebugDrawer::drawLine( const btVector3& from, const btVector3& to, const btVector3& color )
{
    drawLine( from, to, color, color );
}

void GLDebugDrawer::drawSphere( const btVector3& p, btScalar radius, const btVector3& color )
{
    glColor4f( color.getX(), color.getY(), color.getZ(), btScalar( 1.0f ) );
    glPushMatrix();
    glTranslatef( p.getX(), p.getY(), p.getZ() );
    
    S32 lats = 5;
    S32 longs = 5;
    
    S32 i, j;
    for( i = 0; i <= lats; i++ )
    {
        btScalar lat0 = SIMD_PI * ( -btScalar( 0.5 ) + ( btScalar )( i - 1 ) / lats );
        btScalar z0  = radius * sin( lat0 );
        btScalar zr0 =  radius * cos( lat0 );
        
        btScalar lat1 = SIMD_PI * ( -btScalar( 0.5 ) + ( btScalar ) i / lats );
        btScalar z1 = radius * sin( lat1 );
        btScalar zr1 = radius * cos( lat1 );
        
        glBegin( GL_QUAD_STRIP );
        for( j = 0; j <= longs; j++ )
        {
            btScalar lng = 2 * SIMD_PI * ( btScalar )( j - 1 ) / longs;
            btScalar x = cos( lng );
            btScalar y = sin( lng );
            
            glNormal3f( x * zr0, y * zr0, z0 );
            glVertex3f( x * zr0, y * zr0, z0 );
            glNormal3f( x * zr1, y * zr1, z1 );
            glVertex3f( x * zr1, y * zr1, z1 );
        }
        glEnd();
    }
    
    glPopMatrix();
}

void GLDebugDrawer::drawBox( const btVector3& boxMin, const btVector3& boxMax, const btVector3& color, btScalar alpha )
{
    btVector3 halfExtent = ( boxMax - boxMin ) * btScalar( 0.5f );
    btVector3 center = ( boxMax + boxMin ) * btScalar( 0.5f );
    //glEnable(GL_BLEND);     // Turn blending On
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glColor4f( color.getX(), color.getY(), color.getZ(), alpha );
    glPushMatrix();
    glTranslatef( center.getX(), center.getY(), center.getZ() );
    glScaled( 2 * halfExtent[0], 2 * halfExtent[1], 2 * halfExtent[2] );
//	glutSolidCube(1.0);
    glPopMatrix();
    //glDisable(GL_BLEND);
}

void GLDebugDrawer::drawTriangle( const btVector3& a, const btVector3& b, const btVector3& c, const btVector3& color, btScalar alpha )
{
//	if (m_debugMode > 0)
    {
        const btVector3	n = btCross( b - a, c - a ).normalized();
        glBegin( GL_TRIANGLES );
        glColor4f( color.getX(), color.getY(), color.getZ(), alpha );
        glNormal3d( n.getX(), n.getY(), n.getZ() );
        glVertex3d( a.getX(), a.getY(), a.getZ() );
        glVertex3d( b.getX(), b.getY(), b.getZ() );
        glVertex3d( c.getX(), c.getY(), c.getZ() );
        glEnd();
    }
}

void GLDebugDrawer::setDebugMode( S32 debugMode )
{
    m_debugMode = debugMode;
    
}

void GLDebugDrawer::draw3dText( const btVector3& location, StringEntry textString )
{
    glRasterPos3f( location.x(),  location.y(),  location.z() );
}

void GLDebugDrawer::reportErrorWarning( StringEntry warningString )
{
    printf( "%s\n", warningString );
}

void GLDebugDrawer::drawContactPoint( const btVector3& pointOnB, const btVector3& normalOnB, btScalar distance, S32 lifeTime, const btVector3& color )
{
    {
        btVector3 to = pointOnB + normalOnB * distance;
        const btVector3& from = pointOnB;
        glColor4f( color.getX(), color.getY(), color.getZ(), 1.f );
        //glColor4f(0,0,0,1.f);
        
        glBegin( GL_LINES );
        glVertex3d( from.getX(), from.getY(), from.getZ() );
        glVertex3d( to.getX(), to.getY(), to.getZ() );
        glEnd();
        
        
        glRasterPos3f( from.x(),  from.y(),  from.z() );
        UTF8 buf[12];
        sprintf( buf, " %d", lifeTime );
    }
}

#endif
