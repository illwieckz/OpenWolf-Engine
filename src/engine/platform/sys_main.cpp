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
// File name:   sys_main.cpp
// Version:     v1.01
// Created:
// Compilers:   Visual Studio 2017, gcc 7.3.0
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#ifdef DEDICATED
#include <null/null_precompiled.h>
#else
#include <OWLib/precompiled.h>
#endif

static UTF8 binaryPath[ MAX_OSPATH ] = { 0 };
static UTF8 installPath[ MAX_OSPATH ] = { 0 };
static UTF8 libPath[ MAX_OSPATH ] = { 0 };

#ifdef USE_CURSES
static bool nocurses = false;
void CON_Init_tty( void );
#endif

/*
=================
Sys_SetBinaryPath
=================
*/
void Sys_SetBinaryPath( StringEntry path )
{
    Q_strncpyz( binaryPath, path, sizeof( binaryPath ) );
}

/*
=================
Sys_BinaryPath
=================
*/
UTF8* Sys_BinaryPath( void )
{
    return binaryPath;
}

/*
=================
Sys_SetDefaultInstallPath
=================
*/
void Sys_SetDefaultInstallPath( StringEntry path )
{
    Q_strncpyz( installPath, path, sizeof( installPath ) );
}

/*
=================
Sys_DefaultInstallPath
=================
*/
UTF8* Sys_DefaultInstallPath( void )
{
    static UTF8 installdir[MAX_OSPATH];
    
    Com_sprintf( installdir, sizeof( installdir ), "%s", Sys_Cwd() );
    
    // Windows
    Q_strreplace( installdir, sizeof( installdir ), "bin/windows", "" );
    Q_strreplace( installdir, sizeof( installdir ), "bin\\windows", "" );
    
    // Linux
    Q_strreplace( installdir, sizeof( installdir ), "bin/unix", "" );
    
    // BSD
    Q_strreplace( installdir, sizeof( installdir ), "bin/bsd", "" );
    // MacOS X
    Q_strreplace( installdir, sizeof( installdir ), "bin/macosx", "" );
    
    return installdir;
}

/*
=================
Sys_SetDefaultLibPath
=================
*/
void Sys_SetDefaultLibPath( StringEntry path )
{
    Q_strncpyz( libPath, path, sizeof( libPath ) );
}

/*
=================
Sys_DefaultLibPath
=================
*/
UTF8* Sys_DefaultLibPath( void )
{
    if( *libPath )
    {
        return libPath;
    }
    else
    {
        return Sys_Cwd();
    }
}

/*
=================
Sys_DefaultAppPath
=================
*/
UTF8* Sys_DefaultAppPath( void )
{
    return Sys_BinaryPath();
}

/*
=================
Sys_In_Restart_f

Restart the input subsystem
=================
*/
void Sys_In_Restart_f( void )
{
    IN_Restart( );
}

/*
=================
Sys_ConsoleInput

Handle new console input
=================
*/
UTF8* Sys_ConsoleInput( void )
{
    return CON_Input( );
}

#ifdef DEDICATED
#	define PID_FILENAME PRODUCT_NAME_UPPPER "_server.pid"
#else
#	define PID_FILENAME PRODUCT_NAME_UPPPER ".pid"
#endif

/*
=================
Sys_PIDFileName
=================
*/
static UTF8* Sys_PIDFileName( void )
{
    return va( "%s/%s", Sys_TempPath( ), PID_FILENAME );
}

/*
=================
Sys_WritePIDFile

Return true if there is an existing stale PID file
=================
*/
bool Sys_WritePIDFile( void )
{
    UTF8*      pidFile = Sys_PIDFileName( );
    FILE*      f;
    bool  stale = false;
    
    // First, check if the pid file is already there
    if( ( f = fopen( pidFile, "r" ) ) != NULL )
    {
        UTF8  pidBuffer[ 64 ] = { 0 };
        S32   pid;
        
        fread( pidBuffer, sizeof( UTF8 ), sizeof( pidBuffer ) - 1, f );
        fclose( f );
        
        pid = atoi( pidBuffer );
        if( !Sys_PIDIsRunning( pid ) )
        {
            stale = true;
        }
    }
    
    if( ( f = fopen( pidFile, "w" ) ) != NULL )
    {
        fprintf( f, "%d", Sys_PID( ) );
        fclose( f );
    }
    else
    {
        Com_Printf( S_COLOR_YELLOW "Couldn't write %s.\n", pidFile );
    }
    
    return stale;
}

/*
=================
Sys_Exit

Single exit point (regular exit or in case of error)
=================
*/
static void Sys_Exit( S32 exitCode )
{
    CON_Shutdown( );
    
#ifndef DEDICATED
    SDL_Quit( );
#endif
    
    if( exitCode < 2 )
    {
        // Normal exit
        remove( Sys_PIDFileName( ) );
    }
    
    exit( exitCode );
}

/*
=================
Sys_Quit
=================
*/
void Sys_Quit( void )
{
    Sys_Exit( 0 );
}

/*
=================
Sys_Init
=================
*/
void Sys_Init( void )
{
    Cmd_AddCommand( "in_restart", Sys_In_Restart_f );
    cvarSystem->Set( "arch", OS_STRING " " ARCH_STRING );
    cvarSystem->Set( "username", Sys_GetCurrentUser( ) );
}

/*
=================
Sys_AnsiColorPrint

Transform Q3 colour codes to ANSI escape sequences
=================
*/
void Sys_AnsiColorPrint( StringEntry msg )
{
    static UTF8 buffer[ MAXPRINTMSG ];
    S32         length = 0;
    static S32  q3ToAnsi[ 8 ] =
    {
        30, // COLOR_BLACK
        31, // COLOR_RED
        32, // COLOR_GREEN
        33, // COLOR_YELLOW
        34, // COLOR_BLUE
        36, // COLOR_CYAN
        35, // COLOR_MAGENTA
        0   // COLOR_WHITE
    };
    
    while( *msg )
    {
        if( Q_IsColorString( msg ) || *msg == '\n' )
        {
            // First empty the buffer
            if( length > 0 )
            {
                buffer[ length ] = '\0';
                fputs( buffer, stderr );
                length = 0;
            }
            
            if( *msg == '\n' )
            {
                // Issue a reset and then the newline
                fputs( "\033[0m\n", stderr );
                msg++;
            }
            else
            {
                // Print the color code
                Com_sprintf( buffer, sizeof( buffer ), "\033[%dm",
                             q3ToAnsi[ ColorIndex( *( msg + 1 ) ) ] );
                fputs( buffer, stderr );
                msg += 2;
            }
        }
        else
        {
            if( length >= MAXPRINTMSG - 1 )
                break;
                
            buffer[ length ] = *msg;
            length++;
            msg++;
        }
    }
    
    // Empty anything still left in the buffer
    if( length > 0 )
    {
        buffer[ length ] = '\0';
        fputs( buffer, stderr );
    }
}

/*
=================
Host_RecordError
=================
*/
void Host_RecordError( StringEntry msg )
{
    // TODO
}

/*
=================
Sys_WriteDump
=================
*/
void Sys_WriteDump( StringEntry fmt, ... )
{
#if defined( _WIN32 )

#ifndef DEVELOPER
    if( com_developer && com_developer->integer )
#endif
    {
        //this memory should live as long as the SEH is doing its thing...I hope
        static UTF8 msg[2048];
        va_list vargs;
        
        /*
        	Do our own vsnprintf as using va's will change global state
        	that might be pertinent to the dump.
        */
        
        va_start( vargs, fmt );
        vsnprintf( msg, sizeof( msg ) - 1, fmt, vargs );
        va_end( vargs );
        
        msg[sizeof( msg ) - 1] = 0; //ensure null termination
        
        Host_RecordError( msg );
    }
    
#endif
}

/*
=================
Sys_Print
=================
*/
void Sys_Print( StringEntry msg )
{
    CON_LogWrite( msg );
    CON_Print( msg );
}

/*
=================
Sys_Error
=================
*/
void Sys_Error( StringEntry error, ... )
{
    va_list argptr;
    UTF8    string[4096];
    
    va_start( argptr, error );
    vsnprintf( string, sizeof( string ), error, argptr );
    va_end( argptr );
    
    // Print text in the console window/box
    Sys_Print( string );
    Sys_Print( "\n" );
    
    CL_Shutdown( );
    Sys_ErrorDialog( string );
    
    Sys_Exit( 3 );
}

/*
=================
Sys_Warn
=================
*/
void __attribute__( ( format( printf, 1, 2 ) ) ) Sys_Warn( UTF8* warning, ... )
{
    va_list argptr;
    UTF8    string[1024];
    
    va_start( argptr, warning );
    vsnprintf( string, sizeof( string ), warning, argptr );
    va_end( argptr );
    
    CON_Print( va( "Warning: %s", string ) );
}

/*
============
Sys_FileTime

returns -1 if not present
============
*/
S32 Sys_FileTime( UTF8* path )
{
    struct stat buf;
    
    if( stat( path, &buf ) == -1 )
    {
        return -1;
    }
    
    return buf.st_mtime;
}

/*
=================
Sys_UnloadDll
=================
*/
void Sys_UnloadDll( void* dllHandle )
{
    if( !dllHandle )
    {
        Com_Printf( "Sys_UnloadDll(NULL)\n" );
        return;
    }
    
    Sys_UnloadLibrary( dllHandle );
}
/*
=================
Sys_GetDLLName

Used to load a development dll instead of a virtual machine
=================
*/
UTF8* Sys_GetDLLName( StringEntry name )
{
    //Dushan - I have no idea what this mess is and what I made it before
    return va( "%s" ARCH_STRING DLL_EXT, name );
}

/*
=================
Sys_LoadDll

Used to load a development dll instead of a virtual machine
#1 look in fs_homepath
#2 look in fs_basepath
#4 look in fs_libpath under FreeBSD
=================
*/
void* Sys_LoadDll( StringEntry name )
{
    static S32 lastWarning = 0;
    void*    libHandle = NULL;
    UTF8*    basepath;
    UTF8*    homepath;
    UTF8*    gamedir;
    UTF8*    fn;
    UTF8 filename[MAX_QPATH];
    
    Q_strncpyz( filename, Sys_GetDLLName( name ), sizeof( filename ) );
    
    // check current folder only if we are a developer
    if( 1 )
    {
        libHandle = Sys_LoadLibrary( filename );
        if( libHandle )
        {
            return libHandle;
        }
    }
    
    basepath = cvarSystem->VariableString( "fs_basepath" );
    homepath = cvarSystem->VariableString( "fs_homepath" );
    gamedir = cvarSystem->VariableString( "fs_game" );
    
    fn = fileSystem->BuildOSPath( basepath, gamedir, filename );
    
#if !defined( DEDICATED )
    // if the server is pure, extract the dlls from the mp_bin.pk3 so
    // that they can be referenced
    if( cvarSystem->VariableValue( "sv_pure" ) && Q_stricmp( name, "sgame" ) )
    {
        fileSystem->CL_ExtractFromPakFile( homepath, gamedir, filename );
    }
#endif
    
    libHandle = Sys_LoadLibrary( fn );
    
    if( !libHandle )
    {
        if( homepath[0] )
        {
            fn = fileSystem->BuildOSPath( basepath, gamedir, filename );
            libHandle = Sys_LoadLibrary( fn );
        }
        
        if( !libHandle )
        {
            return NULL;
        }
    }
    
    return libHandle;
}

void* Sys_GetProcAddress( void* dllhandle, StringEntry name )
{
    return Sys_LoadFunction( dllhandle, name );
}

/*
=================
Sys_ParseArgs
=================
*/
void Sys_ParseArgs( S32 argc, UTF8** argv )
{
#if defined(USE_CURSES) || !defined(_WIN32)
    S32 i;
#endif
    
    if( argc == 2 )
    {
        if( !strcmp( argv[1], "--version" ) || !strcmp( argv[1], "-v" ) )
        {
            StringEntry date = __DATE__;
#ifdef DEDICATED
            fprintf( stdout, Q3_VERSION " dedicated server (%s)\n", date );
#else
            fprintf( stdout, Q3_VERSION " client (%s)\n", date );
#endif
            Sys_Exit( 0 );
        }
    }
#ifdef USE_CURSES
    for( i = 1; i < argc; i++ )
    {
        if( !strcmp( argv[i], "+nocurses" ) )
        {
            nocurses = true;
            break;
        }
    }
#endif
}

#ifndef DEFAULT_BASEDIR
#	ifdef MACOS_X
#		define DEFAULT_BASEDIR Sys_StripAppBundle(Sys_BinaryPath())
#	else
#		define DEFAULT_BASEDIR Sys_BinaryPath()
#	endif
#endif

/*
================
SignalToString
================
*/
static StringEntry SignalToString( S32 sig )
{
    switch( sig )
    {
        case SIGINT:
            return "Terminal interrupt signal";
        case SIGILL:
            return "Illegal instruction";
        case SIGFPE:
            return "Erroneous arithmetic operation";
        case SIGSEGV:
            return "Invalid memory reference";
        case SIGTERM:
            return "Termination signal";
#if defined (_WIN32)
        case SIGBREAK:
            return "Control-break";
#endif
        case SIGABRT:
            return "Process abort signal";
        default:
            return "unknown";
    }
}

/*
=================
Sys_SigHandler
=================
*/
void Sys_SigHandler( S32 signal )
{
    static bool signalcaught = false;
    
    if( signalcaught )
    {
        Com_Printf( "DOUBLE SIGNAL FAULT: Received signal %d: \"%s\", exiting...\n", signal, SignalToString( signal ) );
        exit( 1 );
    }
    else
    {
        signalcaught = true;
#ifndef DEDICATED
        CL_Shutdown( );
#endif
        serverInitSystem->Shutdown( va( "Received signal %d", signal ) );
    }
    
    if( signal == SIGTERM || signal == SIGINT )
    {
        Sys_Exit( 1 );
    }
    else
    {
        Sys_Exit( 2 );
    }
}

/*
================
Sys_SnapVector
================
*/
#if _WIN32
static ID_INLINE F32 roundfloat( F32 n )
{
    return ( n < 0.0f ) ? ceilf( n - 0.5f ) : floorf( n + 0.5f );
}
#endif

void Sys_SnapVector( F32* v )
{
#if _WIN32
    U32 oldcontrol;
    U32 newcontrol;
    
    _controlfp_s( &oldcontrol, 0, 0 );
    _controlfp_s( &newcontrol, _RC_NEAR, _MCW_RC );
    
    v[0] = roundfloat( v[0] );
    v[1] = roundfloat( v[1] );
    v[2] = roundfloat( v[2] );
    
    _controlfp_s( &newcontrol, oldcontrol, _MCW_RC );
#else
    // pure C99
    S32 oldround = fegetround();
    fesetround( FE_TONEAREST );
    
    v[0] = nearbyintf( v[0] );
    v[1] = nearbyintf( v[1] );
    v[2] = nearbyintf( v[2] );
    
    fesetround( oldround );
#endif
}

/*
=================
main
=================
*/
S32 main( S32 argc, UTF8** argv )
{
    S32   i;
    UTF8  commandLine[ MAX_STRING_CHARS ] = { 0 };
    
#ifndef DEDICATED
    // SDL version check
    
    // Compile time
#	if !SDL_VERSION_ATLEAST(MINSDL_MAJOR,MINSDL_MINOR,MINSDL_PATCH)
#		error A more recent version of SDL is required
#	endif
    
    // Run time
    SDL_version ver;
    SDL_GetVersion( &ver );
    
#define MINSDL_VERSION \
	XSTRING(MINSDL_MAJOR) "." \
	XSTRING(MINSDL_MINOR) "." \
	XSTRING(MINSDL_PATCH)
    
    if( SDL_VERSIONNUM( ver.major, ver.minor, ver.patch ) <
            SDL_VERSIONNUM( MINSDL_MAJOR, MINSDL_MINOR, MINSDL_PATCH ) )
    {
        Sys_Dialog( DT_ERROR, va( "SDL version " MINSDL_VERSION " or greater is required, "
                                  "but only version %d.%d.%d was found. You may be able to obtain a more recent copy "
                                  "from http://www.libsdl.org/.", ver.major, ver.minor, ver.patch ), "SDL Library Too Old" );
                                  
        Sys_Exit( 1 );
    }
#endif
    
    Sys_ParseArgs( argc, argv );
    
    Sys_PlatformInit( );
    
    // Set the initial time base
    Sys_Milliseconds( );
    
    Sys_SetBinaryPath( Sys_Dirname( argv[ 0 ] ) );
    Sys_SetDefaultInstallPath( DEFAULT_BASEDIR );
    Sys_SetDefaultLibPath( DEFAULT_BASEDIR );
    
    // Concatenate the command line for passing to Com_Init
    for( i = 1; i < argc; i++ )
    {
        const bool containsSpaces = ( bool )( strchr( argv[i], ' ' ) != NULL );
        
        if( !strcmp( argv[ i ], "+nocurses" ) )
        {
            continue;
        }
        
        if( !strcmp( argv[ i ], "+showconsole" ) )
        {
            continue;
        }
        
        if( containsSpaces )
        {
            Q_strcat( commandLine, sizeof( commandLine ), "\"" );
        }
        
        Q_strcat( commandLine, sizeof( commandLine ), argv[ i ] );
        
        if( containsSpaces )
        {
            Q_strcat( commandLine, sizeof( commandLine ), "\"" );
        }
        
        Q_strcat( commandLine, sizeof( commandLine ), " " );
    }
    
#ifdef USE_CURSES
    if( nocurses )
    {
        CON_Init_tty();
    }
    else
    {
        CON_Init();
    }
#else
    CON_Init();
#endif
    
    Com_Init( commandLine );
    NET_Init( );
    
    signal( SIGILL, Sys_SigHandler );
    signal( SIGFPE, Sys_SigHandler );
    signal( SIGSEGV, Sys_SigHandler );
    signal( SIGTERM, Sys_SigHandler );
    signal( SIGINT, Sys_SigHandler );
    
    while( 1 )
    {
        IN_Frame( );
        Com_Frame( );
    }
    
    return 0;
}

