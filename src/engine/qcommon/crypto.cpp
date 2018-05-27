////////////////////////////////////////////////////////////////////////////////////////
// Copyright(C) 2007-2008 Amanieu d'Antras (amanieu@gmail.com)
// Copyright(C) 2012 - 2018 Dusan Jocic <dusanjocic@msn.com>
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
// File name:   crypto.cpp
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description: Public-key identification
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#include <qcommon/q_shared.h>
#include <qcommon/qcommon.h>
#include <qcommon/crypto.h>

void* gmp_alloc( U64 size )
{
    return Z_TagMalloc( size, TAG_CRYPTO );
}

void* gmp_realloc( void* ptr, U64 old_size, U64 new_size )
{
    void* new_ptr = Z_TagMalloc( new_size, TAG_CRYPTO );
    
    if( old_size >= new_size )
        return ptr;
    if( ptr )
    {
        ::memcpy( new_ptr, ptr, old_size );
        Z_Free( ptr );
    }
    return new_ptr;
}

void gmp_free( void* ptr, U64 size )
{
    Z_Free( ptr );
}

void* nettle_realloc( void* ctx, void* ptr, U32 size )
{
    void* new_ptr = gmp_realloc( ptr, *( S32* )ctx, size );
    // new size will always be > old size
    *( S32* )ctx = size;
    return new_ptr;
}

void qnettle_random( void* ctx, U32 length, U8* dst )
{
    Com_RandomBytes( dst, length );
}

void qnettle_buffer_init( struct nettle_buffer* buffer, S32* size )
{
    nettle_buffer_init_realloc( buffer, size, nettle_realloc );
}

bool Crypto_Init( void )
{
    mp_set_memory_functions( gmp_alloc, gmp_realloc, gmp_free );
    return true;
}

void Crypto_Shutdown( void )
{
    Z_FreeTags( TAG_CRYPTO );
}