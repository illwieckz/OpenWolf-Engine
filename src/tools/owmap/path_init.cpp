/* -------------------------------------------------------------------------------

   Copyright (C) 1999-2007 id Software, Inc. and contributors.
   For a list of contributors, see the accompanying CONTRIBUTORS file.

   This file is part of GtkRadiant.

   GtkRadiant is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   GtkRadiant is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GtkRadiant; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

   ----------------------------------------------------------------------------------

   This code has been altered significantly from its original form, to support
   several games based on the Quake III Arena engine, in the form of "Q3Map2."

   ------------------------------------------------------------------------------- */

// TODO total refactor

/* marker */
#define PATH_INIT_C

/* dependencies */
#include "q3map2.h"
#include <string>
#include "filematch.h"
#include <cstring>
#include "tinyformat.h"

#if GDEF_OS_WINDOWS
#include <msdirent.h>
#else
#include <dirent.h>
#endif

#define TRUE 1
#define FALSE 0

/* path support */
#define MAX_BASE_PATHS  10
#define MAX_GAME_PATHS  10
#define MAX_PAK_PATHS  200
#define NAME_MAX 255

char*                    homePath;
char installPath[ MAX_OS_PATH ];

int numBasePaths;
char*                    basePaths[ MAX_BASE_PATHS ];
int numGamePaths;
char*                    gamePaths[ MAX_GAME_PATHS ];
int numPakPaths;
char*                    pakPaths[ MAX_PAK_PATHS ];
char*                    homeBasePath = NULL;

#define PATH_MAX 260
#define VFS_MAXDIRS 64
char g_strForbiddenDirs[VFS_MAXDIRS][PATH_MAX + 1];
int g_numForbiddenDirs;

/*
   some of this code is based off the original q3map port from loki
   and finds various paths. moved here from bsp.c for clarity.
 */

/*
   PathLokiGetHomeDir()
   gets the user's home dir (for ~/.q3a)
 */

char* LokiGetHomeDir( void )
{
#ifndef Q_UNIX
    return NULL;
#else
    static char	buf[ 4096 ];
    struct passwd   pw, *pwp;
    char*            home;
    static char homeBuf[MAX_OS_PATH];
    
    
    /* get the home environment variable */
    home = getenv( "HOME" );
    
    /* look up home dir in password database */
    if( !home )
    {
        if( getpwuid_r( getuid(), &pw, buf, sizeof( buf ), &pwp ) == 0 )
        {
            return pw.pw_dir;
        }
    }
    
    snprintf( homeBuf, sizeof( homeBuf ), "%s/.", home );
    
    /* return it */
    return homeBuf;
#endif
}


/*
   PathLokiInitPaths()
   initializes some paths on linux/os x
 */

void LokiInitPaths( char* argv0 )
{
    char*        home;
    
    if( !homePath )
    {
        /* get home dir */
        home = LokiGetHomeDir();
        if( home == NULL )
        {
            home = ".";
        }
        
        /* set home path */
        homePath = home;
    }
    else
    {
        home = homePath;
    }
    
    /* this is kinda crap, but hey */
    strcpy( installPath, "../" );
}


/*
   CleanPath() - ydnar
   cleans a dos path \ -> /
 */

void CleanPath( char* path )
{
    while( *path )
    {
        if( *path == '\\' )
        {
            *path = '/';
        }
        path++;
    }
}


/*
   GetGame() - ydnar
   gets the game_t based on a -game argument
   returns NULL if no match found
 */

game_t* GetGame( char* arg )
{
    int i;
    
    
    /* dummy check */
    if( arg == NULL || arg[ 0 ] == '\0' )
    {
        return NULL;
    }
    
    /* joke */
    if( !Q_stricmp( arg, "quake1" ) ||
            !Q_stricmp( arg, "quake2" ) ||
            !Q_stricmp( arg, "unreal" ) ||
            !Q_stricmp( arg, "ut2k3" ) ||
            !Q_stricmp( arg, "dn3d" ) ||
            !Q_stricmp( arg, "dnf" ) ||
            !Q_stricmp( arg, "hl" ) )
    {
        Sys_Printf( "April fools, silly rabbit!\n" );
        exit( 0 );
    }
    
    /* test it */
    i = 0;
    while( games[ i ].arg != NULL )
    {
        if( Q_stricmp( arg, games[ i ].arg ) == 0 )
        {
            return &games[ i ];
        }
        i++;
    }
    
    /* no matching game */
    return NULL;
}


/*
   AddBasePath() - ydnar
   adds a base path to the list
 */

void AddBasePath( char* path )
{
    /* dummy check */
    if( path == NULL || path[ 0 ] == '\0' || numBasePaths >= MAX_BASE_PATHS )
    {
        return;
    }
    
    /* add it to the list */
    basePaths[ numBasePaths ] = static_cast<char*>( safe_malloc( strlen( path ) + 1 ) );
    strcpy( basePaths[ numBasePaths ], path );
    CleanPath( basePaths[ numBasePaths ] );
    numBasePaths++;
}


/*
   AddHomeBasePath() - ydnar
   adds a base path to the beginning of the list, prefixed by ~/
 */

void AddHomeBasePath( char* path )
{
    char temp[ MAX_OS_PATH ];
    int homePathLen;
    
    if( !homePath )
    {
        return;
    }
    
    /* dummy check */
    if( path == NULL || path[ 0 ] == '\0' )
    {
        return;
    }
    
    /* strip leading dot, if homePath does not end in /. */
    homePathLen = strlen( homePath );
    if( !strcmp( path, "." ) )
    {
        /* -fs_homebase . means that -fs_home is to be used as is */
        strcpy( temp, homePath );
    }
    else if( homePathLen >= 2 && !strcmp( homePath + homePathLen - 2, "/." ) )
    {
        /* remove trailing /. of homePath */
        homePathLen -= 2;
        
        /* concatenate home dir and path */
        sprintf( temp, "%.*s/%s", homePathLen, homePath, path );
    }
    else
    {
        /* remove leading . of path */
        if( path[0] == '.' )
        {
            ++path;
        }
        
        /* concatenate home dir and path */
        sprintf( temp, "%s/%s", homePath, path );
    }
    
    /* add it to the list */
    AddBasePath( temp );
}


/*
   AddGamePath() - ydnar
   adds a game path to the list
 */

void AddGamePath( char* path )
{
    int i;
    
    /* dummy check */
    if( path == NULL || path[ 0 ] == '\0' || numGamePaths >= MAX_GAME_PATHS )
    {
        return;
    }
    
    /* add it to the list */
    gamePaths[ numGamePaths ] = static_cast<char*>( safe_malloc( strlen( path ) + 1 ) );
    strcpy( gamePaths[ numGamePaths ], path );
    CleanPath( gamePaths[ numGamePaths ] );
    numGamePaths++;
    
    /* don't add it if it's already there */
    for( i = 0; i < numGamePaths - 1; i++ )
    {
        if( strcmp( gamePaths[i], gamePaths[numGamePaths - 1] ) == 0 )
        {
            free( gamePaths[numGamePaths - 1] );
            gamePaths[numGamePaths - 1] = NULL;
            numGamePaths--;
            break;
        }
    }
    
}


/*
   AddPakPath()
   adds a pak path to the list
 */

void AddPakPath( char* path )
{
    /* dummy check */
    if( path == NULL || path[ 0 ] == '\0' || numPakPaths >= MAX_PAK_PATHS )
    {
        return;
    }
    
    /* add it to the list */
    pakPaths[ numPakPaths ] = static_cast<char*>( safe_malloc( strlen( path ) + 1 ) );
    strcpy( pakPaths[ numPakPaths ], path );
    CleanPath( pakPaths[ numPakPaths ] );
    numPakPaths++;
}

static void mountDirectory( const char* path )
{
    int j;
    for( j = 0; j < g_numForbiddenDirs; ++j )
    {
        char* dbuf = strdup( path );
        if( *dbuf && dbuf[strlen( dbuf ) - 1] == '/' )
        {
            dbuf[strlen( dbuf ) - 1] = 0;
        }
        const char* p = strrchr( dbuf, '/' );
        p = ( p ? ( p + 1 ) : dbuf );
        if( matchpattern( p, g_strForbiddenDirs[j], TRUE ) )
        {
            free( dbuf );
            break;
        }
        free( dbuf );
    }
    if( j < g_numForbiddenDirs )
    {
        return;
    }
    
    g_vfs.mount( path );
    
    DIR* dir = opendir( path );
    if( dir != NULL )
    {
        while( true )
        {
            dirent* dp = readdir( dir );
            if( dp == NULL )
            {
                break;
            }
            const char* name = dp->d_name;
            if( name == NULL )
            {
                break;
            }
            
            for( j = 0; j < g_numForbiddenDirs; ++j )
            {
                const char* p = strrchr( name, '/' );
                p = ( p ? ( p + 1 ) : name );
                if( matchpattern( p, g_strForbiddenDirs[j], TRUE ) )
                {
                    break;
                }
            }
            if( j < g_numForbiddenDirs )
            {
                continue;
            }
            
            char* dirlist = strdup( name );
            char* ext = strrchr( dirlist, '.' );
            
            if( ext && ( !Q_stricmp( ext, ".pk3dir" ) || !Q_stricmp( ext, ".dpkdir" ) ) )
            {
                auto pakDirPath = tfm::format( "%s/%s", path, name );
                Sys_Printf( "VFS Init: %s\n", pakDirPath.c_str() );
                g_vfs.mount( pakDirPath );
            }
            else if( ext && ( !Q_stricmp( ext, ".pk3" ) || !Q_stricmp( ext, ".dpk" ) ) )
            {
                auto pakPath = tfm::format( "%s/%s", path, dirlist );
                g_vfs.mount( pakPath );
            }
            
            free( dirlist );
        }
        closedir( dir );
    }
}


/*
   InitPaths() - ydnar
   cleaned up some of the path initialization code from bsp.c
   will remove any arguments it uses
 */

void InitPaths( int argc, char** argv )
{
    int i, j, len, len2;
    char temp[ MAX_OS_PATH ];
    
    int noBasePath = 0;
    int noHomePath = 0;
    
    /* note it */
    Sys_FPrintf( SYS_VRB, "--- InitPaths ---\n" );
    
    /* get the install path for backup */
    LokiInitPaths( argv[ 0 ] );
    
    numBasePaths = 0;
    numGamePaths = 0;
    
    /* add standard game path */
    AddGamePath( game->gamePath );
    
    /* parse through the arguments and extract those relevant to paths */
    for( i = 0; i < argc; i++ )
    {
        /* -fs_forbiddenpath */
        if( Q_stricmp( argv[i], "-fs_forbiddenpath" ) == 0 )
        {
            if( ++i >= argc )
            {
                Error( "Out of arguments: No path specified after %s.", argv[i - 1] );
            }
            if( g_numForbiddenDirs < VFS_MAXDIRS )
            {
                strncpy( g_strForbiddenDirs[g_numForbiddenDirs], argv[i], PATH_MAX );
                g_strForbiddenDirs[g_numForbiddenDirs][PATH_MAX] = 0;
                ++g_numForbiddenDirs;
            }
        }
        
        /* -fs_nobasepath */
        else if( Q_stricmp( argv[i], "-fs_nobasepath" ) == 0 )
        {
            noBasePath = 1;
        }
        
        /* -fs_basepath */
        else if( Q_stricmp( argv[i], "-fs_basepath" ) == 0 )
        {
            if( ++i >= argc )
            {
                Error( "Out of arguments: No path specified after %s.", argv[i - 1] );
            }
            AddBasePath( argv[i] );
        }
        
        /* -fs_game */
        else if( Q_stricmp( argv[i], "-fs_game" ) == 0 )
        {
            if( ++i >= argc )
            {
                Error( "Out of arguments: No path specified after %s.", argv[i - 1] );
            }
            AddGamePath( argv[i] );
        }
        
        /* -fs_home */
        else if( Q_stricmp( argv[i], "-fs_home" ) == 0 )
        {
            if( ++i >= argc )
            {
                Error( "Out of arguments: No path specified after %s.", argv[i - 1] );
            }
            homePath = argv[i];
        }
        
        /* -fs_nohomepath */
        else if( Q_stricmp( argv[i], "-fs_nohomepath" ) == 0 )
        {
            noHomePath = 1;
        }
        
        /* -fs_homebase */
        else if( Q_stricmp( argv[i], "-fs_homebase" ) == 0 )
        {
            if( ++i >= argc )
            {
                Error( "Out of arguments: No path specified after %s.", argv[i - 1] );
            }
            homeBasePath = argv[i];
        }
        
        /* -fs_homepath - sets both of them */
        else if( Q_stricmp( argv[i], "-fs_homepath" ) == 0 )
        {
            if( ++i >= argc )
            {
                Error( "Out of arguments: No path specified after %s.", argv[i - 1] );
            }
            homePath = argv[i];
            homeBasePath = ".";
        }
        
        /* -fs_pakpath */
        else if( Q_stricmp( argv[i], "-fs_pakpath" ) == 0 )
        {
            if( ++i >= argc )
            {
                Error( "Out of arguments: No path specified after %s.", argv[i - 1] );
            }
            AddPakPath( argv[i] );
        }
        
    }
    
    /* if there is no base path set, figure it out */
    if( numBasePaths == 0 && noBasePath == 0 )
    {
        /* this is another crappy replacement for SetQdirFromPath() */
        len2 = strlen( game->magic );
        for( i = 0; i < argc && numBasePaths == 0; i++ )
        {
            /* extract the arg */
            strcpy( temp, argv[ i ] );
            CleanPath( temp );
            len = strlen( temp );
            Sys_FPrintf( SYS_VRB, "Searching for \"%s\" in \"%s\" (%d)...\n", game->magic, temp, i );
            
            /* this is slow, but only done once */
            for( j = 0; j < ( len - len2 ); j++ )
            {
                /* check for the game's magic word */
                if( Q_strncasecmp( &temp[ j ], game->magic, len2 ) == 0 )
                {
                    /* now find the next slash and nuke everything after it */
                    while( temp[ ++j ] != '/' && temp[ j ] != '\0' ) ;
                    temp[ j ] = '\0';
                    
                    /* add this as a base path */
                    AddBasePath( temp );
                    break;
                }
            }
        }
        
        /* add install path */
        if( numBasePaths == 0 )
        {
            AddBasePath( installPath );
        }
        
        /* check again */
        if( numBasePaths == 0 )
        {
            Error( "Failed to find a valid base path." );
        }
    }
    
    if( noBasePath == 1 )
    {
        numBasePaths = 0;
    }
    
    if( noHomePath == 0 )
    {
        /* this only affects unix */
        if( homeBasePath )
        {
            AddHomeBasePath( homeBasePath );
        }
        else
        {
            AddHomeBasePath( game->homeBasePath );
        }
    }
    
    /* initialize vfs paths */
    if( numPakPaths > MAX_PAK_PATHS )
    {
        numPakPaths = MAX_PAK_PATHS;
    }
    
    /* walk the list of pak paths */
    for( i = 0; i < numPakPaths; i++ )
    {
        Sys_Printf( "VFS Init: %s\n", pakPaths[i] );
        /* initialize this pak path */
        mountDirectory( pakPaths[i] );
    }
    
    /* initialize vfs paths */
    if( numBasePaths > MAX_BASE_PATHS )
    {
        numBasePaths = MAX_BASE_PATHS;
    }
    if( numGamePaths > MAX_GAME_PATHS )
    {
        numGamePaths = MAX_GAME_PATHS;
    }
    
    /* walk the list of game paths */
    for( j = 0; j < numGamePaths; j++ )
    {
        /* walk the list of base paths */
        for( i = 0; i < numBasePaths; i++ )
        {
            /* create a full path and initialize it */
            sprintf( temp, "%s/%s", basePaths[ i ], gamePaths[ j ] );
            Sys_Printf( "VFS Init: %s\n", temp );
            mountDirectory( temp );
        }
    }
    
    /* done */
    Sys_Printf( "\n" );
}
