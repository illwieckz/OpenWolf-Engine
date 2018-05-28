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
// File name:   r_allocator.h
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#pragma once

class Allocator
{
public:
    Allocator( U64 memorySize );
    ~Allocator();
    
    Allocator( const Allocator& ) = delete;
    Allocator& operator=( const Allocator& ) = delete;
    
    void* Alloc( U64 allocSize );
    void* Mark() const;
    void Reset();
    void ResetTo( void* mark );
    
private:
    void* memoryBase;
    void* mark;
    void* end;
};

template<typename T>
T* ojkAllocArray( Allocator& allocator, U64 count )
{
    return static_cast<T*>( allocator.Alloc( sizeof( T ) * count ) );
}

inline char* ojkAllocString( Allocator& allocator, U64 stringLength )
{
    return ojkAllocArray<UTF8>( allocator, stringLength + 1 );
}

template<typename T>
T* ojkAlloc( Allocator& allocator )
{
    return ojkAllocArray<T>( allocator, 1 );
}
