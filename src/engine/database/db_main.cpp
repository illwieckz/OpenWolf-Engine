////////////////////////////////////////////////////////////////////////////////////////
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
// File name:   db_main.cpp
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#ifdef DEDICATED
#include <null/null_precompiled.h>
#else
#include <OWLib/precompiled.h>
#endif

idDatabaseSystemLocal databaseSystemLocal;
idDatabaseSystem* databaseSystem = &databaseSystemLocal;

cvar_t* db_mysql;

MYSQL* connection;
MYSQL_RES* results;
MYSQL_ROW row;

/*
===============
idDatabaseSystemLocal::Init
===============
*/
void idDatabaseSystemLocal::Init( void )
{
    db_mysql = Cvar_Get( "db_mysql", "0", CVAR_SERVERINFO );
    
    //Dushan - connect to the mysql
    connection = mysql_init( NULL );
    
    //Dushan - setup MySQL connection
    if( !mysql_real_connect( connection, "localhost", "root", "", "dbname", 0, NULL, 0 ) )
    {
        Com_Printf( "^3WARNING:^7 MySQL loading failed: %s\n", mysql_error( connection ) );
        Cvar_Set( "db_mysql", "0" );
    }
    
    //Dushan - write some info about the MySQL version
    Com_Printf( "MySQL loaded version: %s\n", mysql_get_client_info() );
    Cvar_Set( "db_mysql", "1" );
}

/*
===============
idDatabaseSystemLocal::Shutdown
===============
*/
void idDatabaseSystemLocal::Shutdown( void )
{
    //Dushan - close MySQL connection
    mysql_close( connection );
    Com_Printf( "MySQL Closed\n" );
    Cvar_Set( "db_mysql", "0" );
}

/*
===============
idDatabaseSystemLocal::Runquery
===============
*/
bool idDatabaseSystemLocal::Runquery( UTF8* query )
{
    //Dushan - query connection
    if( db_mysql->integer == 1 )
    {
        if( mysql_query( connection, query ) )
        {
            Com_Printf( "^3WARNING:^7 MySQL Query failed: %s\n", mysql_error( connection ) );
            return false;
        }
        
        results = mysql_store_result( connection );
        return true;
    }
    else
    {
        return false;
    }
}

/*
===============
idDatabaseSystemLocal::FinishQuery
===============
*/
void idDatabaseSystemLocal::FinishQuery( void )
{
    if( db_mysql->integer == 1 )
    {
        //Dushan - finish query
        mysql_free_result( results );
    }
}

/*
===============
idDatabaseSystemLocal::FetchRow
===============
*/
bool idDatabaseSystemLocal::FetchRow( void )
{
    if( db_mysql->integer == 1 )
    {
        //Dushan - fetch row
        row = mysql_fetch_row( results );
        if( !row )
        {
            return false;
        }
        return true;
    }
    else
    {
        return false;
    }
}

/*
===============
idDatabaseSystemLocal::FetchFieldbyID
===============
*/
void idDatabaseSystemLocal::FetchFieldbyID( S32 id, UTF8* buffer, S32 len )
{
    if( db_mysql->integer == 1 )
    {
        //Dushan - fetch field by id
        if( row[id] )
        {
            Q_strncpyz( buffer, row[id], len );
        }
        else
        {
            //do nothing
        }
    }
}

/*
===============
idDatabaseSystemLocal::FetchFieldByName
===============
*/
void idDatabaseSystemLocal::FetchFieldByName( StringEntry name, UTF8* buffer, S32 len )
{
    MYSQL_FIELD* fields;
    S32 num_fields;
    S32 i;
    
    if( db_mysql->integer == 1 )
    {
        //Dushan - fetch field by name
        num_fields = mysql_num_fields( results );
        fields = mysql_fetch_fields( results );
        
        for( i = 0; i < num_fields; i++ )
        {
            if( !strcmp( fields[i].name, name ) )
            {
                Q_strncpyz( buffer, row[i], len );
                return;
            }
        }
    }
}
