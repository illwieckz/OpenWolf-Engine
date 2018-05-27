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
// File name:   sys_win32.cpp
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#include <qcommon/q_shared.h>
#include <qcommon/qcommon.h>
#include <client/client.h>
#include <sys/sys_local.h>
#include <sys/resource.h>

#include <windows.h>
#include <lmerr.h>
#include <lmcons.h>
#include <lmwksta.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <direct.h>
#include <io.h>
#include <conio.h>
#include <wincrypt.h>
#include <shlobj.h>
#include <psapi.h>
#include <float.h>
#include <setjmp.h>


// Used to determine where to store user-specific files
static UTF8 homePath[ MAX_OSPATH ] = { 0 };
static jmp_buf sys_exitframe;
static S32 sys_retcode;
static UTF8 sys_exitstr[MAX_STRING_CHARS];

/*
==================
CON_CtrlHandler

The Windows Console doesn't use signals for terminating the application
with Ctrl-C, logging off, window closing, etc.  Instead it uses a special
handler routine.  Fortunately, the values for Ctrl signals don't seem to
overlap with true signal codes that Windows provides, so calling
Sys_SigHandler() with those numbers should be safe for generating unique
shutdown messages.
==================
*/
static BOOL WINAPI CON_CtrlHandler( DWORD sig )
{
    Sys_SigHandler( sig );
    return TRUE;
}

/*
================
Sys_SetFPUCW
Set FPU control word to default value
================
*/

#ifndef _RC_CHOP
// mingw doesn't seem to have these defined :(

#define _MCW_EM	0x0008001fU
#define _MCW_RC	0x00000300U
#define _MCW_PC	0x00030000U
#define _RC_CHOP	0x00000300U
#define _PC_53	0x00010000U

U32 _controlfp( U32 new, U32 mask );
#endif

#define FPUCWMASK1 (_MCW_RC | _MCW_EM)
#define FPUCW (_RC_CHOP | _MCW_EM | _PC_53)

#if idx64
#define FPUCWMASK	(FPUCWMASK1)
#else
#define FPUCWMASK	(FPUCWMASK1 | _MCW_PC)
#endif

/*
==============
Sys_SetFloatEnv
==============
*/
void Sys_SetFloatEnv( void )
{
    _controlfp( FPUCW, FPUCWMASK );
}


/*
================
Sys_DefaultHomePath
================
*/
// Dushan - SHFolder.dll is common in Windows nowadays, so we dont need much of this stuff anyway
UTF8* Sys_DefaultHomePath( UTF8* buffer, S32 size )
{
    if( SHGetSpecialFolderPath( NULL, buffer, CSIDL_PERSONAL, TRUE ) != NOERROR )
    {
        Q_strcat( buffer, size, "\\My Games\\OpenWolf" );
    }
    else
    {
        Com_Error( ERR_FATAL, "couldn't find home path.\n" );
        buffer[0] = 0;
    }
    
    return buffer;
}

/*
================
Sys_TempPath
================
*/
StringEntry Sys_TempPath( void )
{
    static	TCHAR path[ MAX_PATH ];
    DWORD	length;
    UTF8	tmp[ MAX_OSPATH ];
    
    length = GetTempPath( sizeof( path ), path );
    
    if( length > sizeof( path ) || length == 0 )
    {
        return Sys_DefaultHomePath( tmp, sizeof( tmp ) );
    }
    else
    {
        return path;
    }
}

/*
================
Sys_Milliseconds
================
*/
S32 sys_timeBase;
S32 Sys_Milliseconds( void )
{
    S32             sys_curtime;
    static bool initialized = false;
    
    if( !initialized )
    {
        sys_timeBase = timeGetTime();
        initialized = true;
    }
    sys_curtime = timeGetTime() - sys_timeBase;
    
    return sys_curtime;
}

/*
================
Sys_RandomBytes
================
*/
bool Sys_RandomBytes( U8* string, S32 len )
{
    HCRYPTPROV  prov;
    
    if( !CryptAcquireContext( &prov, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT ) )
    {
        return false;
    }
    
    if( !CryptGenRandom( prov, len, ( BYTE* )string ) )
    {
        CryptReleaseContext( prov, 0 );
        return false;
    }
    CryptReleaseContext( prov, 0 );
    return true;
}

/*
================
Sys_GetCurrentUser
================
*/
UTF8* Sys_GetCurrentUser( void )
{
    static UTF8 s_userName[1024];
    U64 size = sizeof( s_userName );
    
    if( !GetUserName( s_userName, ( LPDWORD )&size ) )
    {
        strcpy( s_userName, "player" );
    }
    
    if( !s_userName[0] )
    {
        strcpy( s_userName, "player" );
    }
    
    return s_userName;
}

/*
================
Sys_GetClipboardData
================
*/
UTF8* Sys_GetClipboardData( void )
{
    UTF8* data = NULL, *cliptext;
    
    if( OpenClipboard( NULL ) != 0 )
    {
        HANDLE hClipboardData;
        
        if( ( hClipboardData = GetClipboardData( CF_TEXT ) ) != 0 )
        {
            if( ( cliptext = ( UTF8* )GlobalLock( hClipboardData ) ) != 0 )
            {
                data = ( UTF8* )Z_Malloc( GlobalSize( hClipboardData ) + 1 );
                Q_strncpyz( data, cliptext, GlobalSize( hClipboardData ) );
                GlobalUnlock( hClipboardData );
                
                strtok( data, "\n\r\b" );
            }
        }
        CloseClipboard();
    }
    return data;
}

#define MEM_THRESHOLD 96*1024*1024

/*
==================
Sys_LowPhysicalMemory
==================
*/
bool Sys_LowPhysicalMemory( void )
{
#if defined (__WIN64__)
    MEMORYSTATUSEX stat;
    GlobalMemoryStatusEx( &stat );
    
    return ( stat.ullTotalPhys <= MEM_THRESHOLD ) ? true : false;
#else
    MEMORYSTATUS stat;
    GlobalMemoryStatus( &stat );
    
    return ( stat.dwTotalPhys <= MEM_THRESHOLD ) ? true : false;
#endif
}

/*
==============
Sys_Basename
==============
*/
StringEntry Sys_Basename( UTF8* path )
{
    static UTF8 base[ MAX_OSPATH ] = { 0 };
    S32			length;
    
    length = strlen( path ) - 1;
    
    // Skip trailing slashes
    while( length > 0 && path[ length ] == '\\' )
    {
        length--;
    }
    
    while( length > 0 && path[ length - 1 ] != '\\' )
    {
        length--;
    }
    
    Q_strncpyz( base, &path[ length ], sizeof( base ) );
    
    length = strlen( base ) - 1;
    
    // Strip trailing slashes
    while( length > 0 && base[ length ] == '\\' )
        base[ length-- ] = '\0';
        
    return base;
}

/*
==============
Sys_Dirname
==============
*/
StringEntry Sys_Dirname( UTF8* path )
{
    static UTF8 dir[ MAX_OSPATH ] = { 0 };
    S32			length;
    
    Q_strncpyz( dir, path, sizeof( dir ) );
    length = strlen( dir ) - 1;
    
    while( length > 0 && dir[ length ] != '\\' )
    {
        length--;
    }
    
    dir[ length ] = '\0';
    
    return dir;
}

/*
==============
Sys_Mkdir
==============
*/
bool Sys_Mkdir( StringEntry path )
{
    if( !CreateDirectory( path, NULL ) )
    {
        if( GetLastError( ) != ERROR_ALREADY_EXISTS )
        {
            return false;
        }
    }
    
    return true;
}

/*
==============
Sys_Cwd
==============
*/
UTF8* Sys_Cwd( void )
{
    static UTF8 cwd[MAX_OSPATH];
    
    _getcwd( cwd, sizeof( cwd ) - 1 );
    cwd[MAX_OSPATH - 1] = 0;
    
    return cwd;
}

/*
==============================================================
DIRECTORY SCANNING
==============================================================
*/

#define MAX_FOUND_FILES 0x1000

/*
==============
Sys_ListFilteredFiles
==============
*/
void Sys_ListFilteredFiles( StringEntry basedir, UTF8* subdirs, UTF8* filter, UTF8** list, S32* numfiles )
{
    UTF8		search[MAX_OSPATH], newsubdirs[MAX_OSPATH];
    UTF8		filename[MAX_OSPATH];
    intptr_t	findhandle;
    struct		_finddata_t findinfo;
    
    if( *numfiles >= MAX_FOUND_FILES - 1 )
    {
        return;
    }
    
    if( strlen( subdirs ) )
    {
        Com_sprintf( search, sizeof( search ), "%s\\%s\\*", basedir, subdirs );
    }
    else
    {
        Com_sprintf( search, sizeof( search ), "%s\\*", basedir );
    }
    
    findhandle = _findfirst( search, &findinfo );
    if( findhandle == -1 )
    {
        return;
    }
    
    do
    {
        if( findinfo.attrib & _A_SUBDIR )
        {
            if( Q_stricmp( findinfo.name, "." ) && Q_stricmp( findinfo.name, ".." ) )
            {
                if( strlen( subdirs ) )
                {
                    Com_sprintf( newsubdirs, sizeof( newsubdirs ), "%s\\%s", subdirs, findinfo.name );
                }
                else
                {
                    Com_sprintf( newsubdirs, sizeof( newsubdirs ), "%s", findinfo.name );
                }
                Sys_ListFilteredFiles( basedir, newsubdirs, filter, list, numfiles );
            }
        }
        if( *numfiles >= MAX_FOUND_FILES - 1 )
        {
            break;
        }
        Com_sprintf( filename, sizeof( filename ), "%s\\%s", subdirs, findinfo.name );
        if( !Com_FilterPath( filter, filename, false ) )
        {
            continue;
        }
        list[ *numfiles ] = CopyString( filename );
        ( *numfiles )++;
    }
    while( _findnext( findhandle, &findinfo ) != -1 );
    
    _findclose( findhandle );
}

/*
==============
strgtr
==============
*/
static bool strgtr( StringEntry s0, StringEntry s1 )
{
    S32 l0, l1, i;
    
    l0 = strlen( s0 );
    l1 = strlen( s1 );
    
    if( l1 < l0 )
    {
        l0 = l1;
    }
    
    for( i = 0; i < l0; i++ )
    {
        if( s1[i] > s0[i] )
        {
            return true;
        }
        if( s1[i] < s0[i] )
        {
            return false;
        }
    }
    return false;
}

/*
==============
Sys_ListFiles
==============
*/
UTF8** Sys_ListFiles( StringEntry directory, StringEntry extension, UTF8* filter, S32* numfiles, bool wantsubs )
{
    UTF8				search[MAX_OSPATH];
    S32					nfiles, flag, i;
    UTF8**				listCopy, *list[MAX_FOUND_FILES];
    struct _finddata_t	findinfo;
    intptr_t			findhandle;
    
    if( filter )
    {
        nfiles = 0;
        Sys_ListFilteredFiles( directory, "", filter, list, &nfiles );
        
        list[ nfiles ] = 0;
        *numfiles = nfiles;
        
        if( !nfiles )
        {
            return NULL;
        }
        
        listCopy = ( UTF8** )Z_Malloc( ( nfiles + 1 ) * sizeof( *listCopy ) );
        for( i = 0 ; i < nfiles ; i++ )
        {
            listCopy[i] = list[i];
        }
        listCopy[i] = NULL;
        
        return listCopy;
    }
    
    if( !extension )
    {
        extension = "";
    }
    
    // passing a slash as extension will find directories
    if( extension[0] == '/' && extension[1] == 0 )
    {
        extension = "";
        flag = 0;
    }
    else
    {
        flag = _A_SUBDIR;
    }
    
    Com_sprintf( search, sizeof( search ), "%s\\*%s", directory, extension );
    
    // search
    nfiles = 0;
    
    findhandle = _findfirst( search, &findinfo );
    if( findhandle == -1 )
    {
        *numfiles = 0;
        return NULL;
    }
    
    do
    {
        if( ( !wantsubs && flag ^ ( findinfo.attrib & _A_SUBDIR ) ) || ( wantsubs && findinfo.attrib & _A_SUBDIR ) )
        {
            if( nfiles == MAX_FOUND_FILES - 1 )
            {
                break;
            }
            list[ nfiles ] = CopyString( findinfo.name );
            nfiles++;
        }
    }
    while( _findnext( findhandle, &findinfo ) != -1 );
    
    list[ nfiles ] = 0;
    
    _findclose( findhandle );
    
    // return a copy of the list
    *numfiles = nfiles;
    
    if( !nfiles )
    {
        return NULL;
    }
    
    listCopy = ( UTF8** )Z_Malloc( ( nfiles + 1 ) * sizeof( *listCopy ) );
    for( i = 0 ; i < nfiles ; i++ )
    {
        listCopy[i] = list[i];
    }
    listCopy[i] = NULL;
    
    do
    {
        flag = 0;
        for( i = 1; i < nfiles; i++ )
        {
            if( strgtr( listCopy[i - 1], listCopy[i] ) )
            {
                UTF8* temp = listCopy[i];
                listCopy[i] = listCopy[i - 1];
                listCopy[i - 1] = temp;
                flag = 1;
            }
        }
    }
    while( flag );
    
    return listCopy;
}

/*
==============
Sys_FreeFileList
==============
*/
void Sys_FreeFileList( UTF8** list )
{
    S32 i;
    
    if( !list )
    {
        return;
    }
    
    for( i = 0 ; list[i] ; i++ )
    {
        Z_Free( list[i] );
    }
    
    Z_Free( list );
}


/*
==============
Sys_Sleep

Block execution for msec or until input is received.
==============
*/
void Sys_Sleep( S32 msec )
{
    if( msec == 0 )
        return;
        
#ifdef DEDICATED
    if( msec < 0 )
    {
        WaitForSingleObject( GetStdHandle( STD_INPUT_HANDLE ), INFINITE );
    }
    else
    {
        WaitForSingleObject( GetStdHandle( STD_INPUT_HANDLE ), msec );
    }
#else
    // Client Sys_Sleep doesn't support waiting on stdin
    if( msec < 0 )
    {
        return;
    }
    
    Sleep( msec );
#endif
}

/*
==============
Sys_OpenUrl
==============
*/
bool Sys_OpenUrl( StringEntry url )
{
    return ( ( S32 )ShellExecute( NULL, NULL, url, NULL, NULL, SW_SHOWNORMAL ) > 32 ) ? true : false;
}

/*
==============
Sys_ErrorDialog

Display an error message
==============
*/
void Sys_ErrorDialog( StringEntry error )
{
    StringEntry homepath = Cvar_VariableString( "fs_homepath" ), gamedir = Cvar_VariableString( "fs_gamedir" ), fileName = "crashlog.txt";
    UTF8 buffer[ 1024 ], *ospath = FS_BuildOSPath( homepath, gamedir, fileName );
    U32 size;
    S32 f = -1;
    
    Sys_Print( va( "%s\n", error ) );
    
#ifndef DEDICATED
    Sys_Dialog( DT_ERROR, va( "%s. See \"%s\" for details.", error, ospath ), "Error" );
#endif
    
    // Make sure the write path for the crashlog exists...
    if( FS_CreatePath( ospath ) )
    {
        Com_Printf( "ERROR: couldn't create path '%s' for crash log.\n", ospath );
        return;
    }
    
    // We might be crashing because we maxed out the Quake MAX_FILE_HANDLES,
    // which will come through here, so we don't want to recurse forever by
    // calling FS_FOpenFileWrite()...use the Unix system APIs instead.
    f = open( ospath, O_CREAT | O_TRUNC | O_WRONLY, 0640 );
    if( f == -1 )
    {
        Com_Printf( "ERROR: couldn't open %s\n", fileName );
        return;
    }
    
    // We're crashing, so we don't care much if write() or close() fails.
    while( ( size = CON_LogRead( buffer, sizeof( buffer ) ) ) > 0 )
    {
        if( write( f, buffer, size ) != size )
        {
            Com_Printf( "ERROR: couldn't fully write to %s\n", fileName );
            break;
        }
    }
    
    close( f );
}

/*
==============
Sys_Dialog

Display a win32 dialog box
==============
*/
dialogResult_t Sys_Dialog( dialogType_t type, StringEntry message, StringEntry title )
{
    UINT uType;
    
    switch( type )
    {
        default:
        case DT_INFO:
            uType = MB_ICONINFORMATION | MB_OK;
            break;
        case DT_WARNING:
            uType = MB_ICONWARNING | MB_OK;
            break;
        case DT_ERROR:
            uType = MB_ICONERROR | MB_OK;
            break;
        case DT_YES_NO:
            uType = MB_ICONQUESTION | MB_YESNO;
            break;
        case DT_OK_CANCEL:
            uType = MB_ICONWARNING | MB_OKCANCEL;
            break;
    }
    
    switch( MessageBox( NULL, message, title, uType ) )
    {
        default:
        case IDOK:
            return DR_OK;
        case IDCANCEL:
            return DR_CANCEL;
        case IDYES:
            return DR_YES;
        case IDNO:
            return DR_NO;
    }
}

#ifndef DEDICATED
static bool SDL_VIDEODRIVER_externallySet = false;
#endif

/*
==============
Sys_GLimpSafeInit

Windows specific "safe" GL implementation initialisation
==============
*/
void Sys_GLimpSafeInit( void )
{
#ifndef DEDICATED
    if( !SDL_VIDEODRIVER_externallySet )
    {
        // Here, we want to let SDL decide what do to unless
        // explicitly requested otherwise
        _putenv( "SDL_VIDEODRIVER=" );
    }
#endif
}

/*
==============
Sys_GLimpInit

Windows specific GL implementation initialisation
==============
*/
void Sys_GLimpInit( void )
{
#ifndef DEDICATED
    if( !SDL_VIDEODRIVER_externallySet )
    {
        // It's a little bit weird having in_mouse control the
        // video driver, but from ioq3's point of view they're
        // virtually the same except for the mouse input anyway
        if( Cvar_VariableIntegerValue( "in_mouse" ) == -1 )
        {
            // Use the windib SDL backend, which is closest to
            // the behaviour of idq3 with in_mouse set to -1
            _putenv( "SDL_VIDEODRIVER=windib" );
        }
        else
        {
            // Use the DirectX SDL backend
            _putenv( "SDL_VIDEODRIVER=directx" );
        }
    }
#endif
}

/*
==============
Sys_PlatformInit

Windows specific initialisation
==============
*/
static void resetTime( void )
{
    timeEndPeriod( 1 );
}

void Sys_PlatformInit( void )
{
#if 0 //ndef DEDICATED
    StringEntry SDL_VIDEODRIVER = getenv( "SDL_VIDEODRIVER" );
#endif
    
    Sys_SetFloatEnv();
    
#if 0//ndef DEDICATED
    if( SDL_VIDEODRIVER )
    {
        Com_Printf( "SDL_VIDEODRIVER is externally set to \"%s\", "
                    "in_mouse -1 will have no effect\n", SDL_VIDEODRIVER );
        SDL_VIDEODRIVER_externallySet = true;
    }
    else
    {
        SDL_VIDEODRIVER_externallySet = false;
    }
#endif
    
    // Handle Ctrl-C or other console termination
    SetConsoleCtrlHandler( CON_CtrlHandler, TRUE );
    
    // Increase sleep resolution
    timeBeginPeriod( 1 );
    atexit( resetTime );
}

/*
==============
Sys_SetEnv

set/unset environment variables (empty value removes it)
==============
*/
void Sys_SetEnv( StringEntry name, StringEntry value )
{
    _putenv( va( "%s=%s", name, value ) );
}

/*
==============
Sys_PID
==============
*/
S32 Sys_PID( void )
{
    return GetCurrentProcessId( );
}

/*
==============
Sys_PIDIsRunning
==============
*/
bool Sys_PIDIsRunning( S32 pid )
{
    DWORD	processes[ 1024 ], numBytes, numProcesses;
    S32		i;
    
    if( !EnumProcesses( processes, sizeof( processes ), &numBytes ) )
    {
        return false; // Assume it's not running
    }
    
    numProcesses = numBytes / sizeof( DWORD );
    
    // Search for the pid
    for( i = 0; i < numProcesses; i++ )
    {
        if( processes[ i ] == pid )
        {
            return true;
        }
    }
    
    return false;
}

/*
==================
Sys_StartProcess

NERVE - SMF
==================
*/
void Sys_StartProcess( UTF8* exeName, bool doexit )
{
    TCHAR				szPathOrig[_MAX_PATH];
    STARTUPINFO			si;
    PROCESS_INFORMATION pi;
    
    ZeroMemory( &si, sizeof( si ) );
    si.cb = sizeof( si );
    
    GetCurrentDirectory( _MAX_PATH, szPathOrig );
    
    // JPW NERVE swiped from Sherman's SP code
    if( !CreateProcess( NULL, va( "%s\\%s", szPathOrig, exeName ), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi ) )
    {
        // couldn't start it, popup error box
        Com_Error( ERR_DROP, "Could not start process: '%s\\%s' ", szPathOrig, exeName );
        return;
    }
    // jpw
    
    // TTimo: similar way of exiting as used in Sys_OpenURL below
    if( doexit )
    {
        Cbuf_ExecuteText( EXEC_APPEND, "quit\n" );
    }
}

/*
==================
Sys_OpenURL

NERVE - SMF
==================
*/
void Sys_OpenURL( StringEntry url, bool doexit )
{
    HWND wnd;
    
    static bool doexit_spamguard = false;
    
    if( doexit_spamguard )
    {
        Com_DPrintf( "Sys_OpenURL: already in a doexit sequence, ignoring %s\n", url );
        return;
    }
    
    Com_Printf( "Open URL: %s\n", url );
    
    if( !ShellExecute( NULL, "open", url, NULL, NULL, SW_RESTORE ) )
    {
        // couldn't start it, popup error box
        Com_Error( ERR_DROP, "Could not open url: '%s' ", url );
        return;
    }
    
    wnd = GetForegroundWindow();
    
    if( wnd )
    {
        ShowWindow( wnd, SW_MAXIMIZE );
    }
    
    if( doexit )
    {
        // show_bug.cgi?id=612
        doexit_spamguard = true;
        Cbuf_ExecuteText( EXEC_APPEND, "quit\n" );
    }
}

/*
========================================================================

EVENT LOOP

========================================================================
*/

#define MAX_QUED_EVENTS     256
#define MASK_QUED_EVENTS    ( MAX_QUED_EVENTS - 1 )

sysEvent_t eventQue[MAX_QUED_EVENTS];
S32 eventHead, eventTail;
U8 sys_packetReceived[MAX_MSGLEN];

/*
==============
Sys_IsNumLockDown
==============
*/
bool Sys_IsNumLockDown( void )
{
    SHORT state = GetKeyState( VK_NUMLOCK );
    
    if( state & 0x01 )
    {
        return true;
    }
    
    return false;
}

