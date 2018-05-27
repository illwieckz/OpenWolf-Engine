////////////////////////////////////////////////////////////////////////////////////////
// Copyright(C) 2007 - 2008 Amanieu d'Antras (amanieu@gmail.com)
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
// File name:   cryto.h
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#ifndef __CRYPTO_H__
#define __CRYPTO_H__

#ifndef __Q_SHARED_H__
#include "q_shared.h"
#endif
#ifndef __QCOMMON_H__
#include "qcommon.h"
#endif

#include <gmp.h>
#include <nettle/bignum.h>
#include <nettle/rsa.h>
#include <nettle/buffer.h>

bool Crypto_Init( void );
void Crypto_Shutdown( void );
// The size is stored in the location pointed to by second arg, and will change as the buffer grows
void qnettle_buffer_init( struct nettle_buffer* buffer, S32* size );
// Random function used for key generation and encryption
void qnettle_random( void* ctx, U32 length, U8* dst );

#endif /* __CRYPTO_H__ */
