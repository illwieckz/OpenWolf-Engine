////////////////////////////////////////////////////////////////////////////////////////
// Copyright(C) 2011-2018 Dusan Jocic <dusanjocic@msn.com>
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
// File name:   db_local.h
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#ifndef __DATABASE_LOCAL_H__
#define __DATABASE_LOCAL_H__

#ifndef __Q_SHARED_H__
#include <qcommon/q_shared.h>
#endif

//
// idDatabaseSystemLocal
//
class idDatabaseSystemLocal : public idDatabaseSystem
{
public:
    virtual void Init( void );
    virtual void Shutdown( void );
    virtual bool Runquery( UTF8* query );
    virtual void FinishQuery( void );
    virtual bool FetchRow( void );
    virtual void FetchFieldbyID( S32 id, UTF8* buffer, S32 len );
    virtual void FetchFieldByName( StringEntry name, UTF8* buffer, S32 len );
};

extern idDatabaseSystemLocal dbSystemLocal;

#endif // !__DATABASE_LOCAL_H__

