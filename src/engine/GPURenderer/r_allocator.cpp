////////////////////////////////////////////////////////////////////////////////////////
// Copyright(C) 2013 - 2016, OpenJK contributors
// Copyright(C) 2011 - 2018 Dusan Jocic <dusanjocic@msn.com>
//
// This file is part of OpenWolf.
//
// OpenWolf is free software; you can redistribute it
// and / or modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of the License,
// or (at your option) any later version.
//
// OpenWolf is distributed in the hope that it will be
// useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with OpenWolf; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110 - 1301  USA
//
// -------------------------------------------------------------------------------------
// File name:   r_allocator.cpp
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#include <OWLib/precompiled.h>

Allocator::Allocator( U64 memorySize )
#if defined(GLSL_BUILDTOOL)
    : memoryBase( malloc( memorySize ) )
#else
    : memoryBase( CL_RefMalloc( memorySize ) )
#endif
    , mark( memoryBase )
    , end( ( UTF8* )memoryBase + memorySize )
{
}

Allocator::~Allocator()
{
#if defined(GLSL_BUILDTOOL)
    free( memoryBase );
#else
    Z_Free( memoryBase );
#endif
}

void* Allocator::Alloc( U64 allocSize )
{
    if( ( U64 )( ( UTF8* )end - ( UTF8* )mark ) < allocSize )
    {
        return nullptr;
    }
    
    void* result = mark;
    U64 alignedSize = ( allocSize + 15 ) & ~15;
    
    mark = ( UTF8* )mark + alignedSize;
    
    return result;
}

void* Allocator::Mark() const
{
    return mark;
}

void Allocator::Reset()
{
    mark = memoryBase;
}

void Allocator::ResetTo( void* m )
{
    mark = m;
}
