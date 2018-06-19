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
// File name:   qcommon.h
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description: definitions common between client and server, but not game or ref module
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#ifndef __QCOMMON_H__
#define __QCOMMON_H__

#ifdef _PHYSICSLIB
#ifndef __PHYSICS_PUBLIC_H__
#include <physicslib/physics_public.h>
#endif
#endif

//bani
#if defined __GNUC__ || defined __clang__
#define _attribute( x ) __attribute__( x )
#else
#define _attribute( x )
#endif

//#define PRE_RELEASE_DEMO
#ifdef PRE_RELEASE_DEMO
#define PRE_RELEASE_DEMO_NODEVMAP
#endif							// PRE_RELEASE_DEMO

//============================================================================

//
// msg.c
//
typedef struct msg_s
{
    bool        allowoverflow;	// if false, do a Com_Error
    bool        overflowed;	// set to true if the buffer size failed (with allowoverflow set)
    bool        oob;		// set to true if the buffer size failed (with allowoverflow set)
    U8*           data;
    S32             maxsize;
    S32             cursize;
    S32             uncompsize;	// NERVE - SMF - net debugging
    S32             readcount;
    S32             bit;		// for bitwise reads and writes
} msg_t;

void            MSG_Init( msg_t* buf, U8* data, S32 length );
void            MSG_InitOOB( msg_t* buf, U8* data, S32 length );
void            MSG_Clear( msg_t* buf );
void*           MSG_GetSpace( msg_t* buf, S32 length );
void            MSG_WriteData( msg_t* buf, const void* data, S32 length );
void            MSG_Bitstream( msg_t* buf );
void            MSG_Uncompressed( msg_t* buf );

// TTimo
// copy a msg_t in case we need to store it as is for a bit
// (as I needed this to keep an msg_t from a static var for later use)
// sets data buffer as MSG_Init does prior to do the copy
void            MSG_Copy( msg_t* buf, U8* data, S32 length, msg_t* src );

struct usercmd_s;
struct entityState_s;
struct playerState_s;

void            MSG_WriteBits( msg_t* msg, S32 value, S32 bits );

void            MSG_WriteChar( msg_t* sb, S32 c );
void            MSG_WriteByte( msg_t* sb, S32 c );
void            MSG_WriteShort( msg_t* sb, S32 c );
void            MSG_WriteLong( msg_t* sb, S32 c );
void            MSG_WriteFloat( msg_t* sb, F32 f );
void            MSG_WriteString( msg_t* sb, StringEntry s );
void            MSG_WriteBigString( msg_t* sb, StringEntry s );
void            MSG_WriteAngle16( msg_t* sb, F32 f );

void            MSG_BeginReading( msg_t* sb );
void            MSG_BeginReadingOOB( msg_t* sb );
void            MSG_BeginReadingUncompressed( msg_t* msg );

S32             MSG_ReadBits( msg_t* msg, S32 bits );

S32             MSG_ReadChar( msg_t* sb );
S32             MSG_ReadByte( msg_t* sb );
S32             MSG_ReadShort( msg_t* sb );
S32             MSG_ReadLong( msg_t* sb );
F32           MSG_ReadFloat( msg_t* sb );
UTF8*           MSG_ReadString( msg_t* sb );
UTF8*           MSG_ReadBigString( msg_t* sb );
UTF8*           MSG_ReadStringLine( msg_t* sb );
F32           MSG_ReadAngle16( msg_t* sb );
void            MSG_ReadData( msg_t* sb, void* buffer, S32 size );
S32				MSG_LookaheadByte( msg_t* msg );

void            MSG_WriteDeltaUsercmd( msg_t* msg, struct usercmd_s* from, struct usercmd_s* to );
void            MSG_ReadDeltaUsercmd( msg_t* msg, struct usercmd_s* from, struct usercmd_s* to );

void            MSG_WriteDeltaUsercmdKey( msg_t* msg, S32 key, usercmd_t* from, usercmd_t* to );
void            MSG_ReadDeltaUsercmdKey( msg_t* msg, S32 key, usercmd_t* from, usercmd_t* to );

void            MSG_WriteDeltaEntity( msg_t* msg, struct entityState_s* from, struct entityState_s* to, bool force );
void            MSG_ReadDeltaEntity( msg_t* msg, entityState_t* from, entityState_t* to, S32 number );

void            MSG_WriteDeltaPlayerstate( msg_t* msg, struct playerState_s* from, struct playerState_s* to );
void            MSG_ReadDeltaPlayerstate( msg_t* msg, struct playerState_s* from, struct playerState_s* to );


void            MSG_ReportChangeVectors_f( void );

//============================================================================

/*
==============================================================

NET

==============================================================
*/

#define NET_ENABLEV4            0x01
#define NET_ENABLEV6            0x02
// if this flag is set, always attempt ipv6 connections instead of ipv4 if a v6 address is found.
#define NET_PRIOV6              0x04
// disables ipv6 multicast support if set.
#define NET_DISABLEMCAST        0x08

#define PACKET_BACKUP   32  // number of old messages that must be kept on client and
// server for delta comrpession and ping estimation
#define PACKET_MASK     ( PACKET_BACKUP - 1 )

#define MAX_PACKET_USERCMDS     32      // max number of usercmd_t in a packet

#define PORT_ANY            -1

#define MAX_MASTER_SERVERS  5

// RF, increased this, seems to keep causing problems when set to 64, especially when loading
// a savegame, which is hard to fix on that side, since we can't really spread out a loadgame
// among several frames
//#define   MAX_RELIABLE_COMMANDS   64          // max string commands buffered for restransmit
//#define   MAX_RELIABLE_COMMANDS   128         // max string commands buffered for restransmit
#define MAX_RELIABLE_COMMANDS   256	// bigger!

typedef enum
{
    NA_BOT,
    NA_BAD,                 // an address lookup failed
    NA_LOOPBACK,
    NA_BROADCAST,
    NA_IP,
    NA_IP6,
    NA_MULTICAST6,
    NA_UNSPEC
} netadrtype_t;

typedef enum
{
    NS_CLIENT,
    NS_SERVER
} netsrc_t;

#define NET_ADDRSTRMAXLEN 48	// maximum length of an IPv6 address string including trailing '\0'
typedef struct
{
    netadrtype_t type;
    
    U8 ip[4];
    U8 ip6[16];
    
    U16 port;
    U64 scope_id;	// Needed for IPv6 link-local addresses
} netadr_t;

void			NET_Init( void );
void            NET_Shutdown( void );
void			NET_Restart_f( void );
void			NET_Config( bool enableNetworking );
void            NET_FlushPacketQueue( void );
void			NET_SendPacket( netsrc_t sock, S32 length, const void* data, netadr_t to );
void      NET_OutOfBandPrint( netsrc_t net_socket, netadr_t adr, StringEntry format, ... ) __attribute__( ( format( printf, 3, 4 ) ) );
void 	NET_OutOfBandData( netsrc_t sock, netadr_t adr, U8* format, S32 len );

bool		NET_CompareAdr( netadr_t a, netadr_t b );
bool		NET_CompareBaseAdr( netadr_t a, netadr_t b );
bool		NET_IsLocalAddress( netadr_t adr );
bool		NET_IsIPXAddress( StringEntry buf );
StringEntry		NET_AdrToString( netadr_t a );
StringEntry		NET_AdrToStringwPort( netadr_t a );
S32				NET_StringToAdr( StringEntry s, netadr_t* a, netadrtype_t family );
bool		NET_GetLoopPacket( netsrc_t sock, netadr_t* net_from, msg_t* net_message );
void            NET_JoinMulticast6( void );
void            NET_LeaveMulticast6( void );

void			NET_Sleep( S32 msec );

#if defined(USE_HTTP)

// Dushan
typedef enum
{
    HTTP_WRITE,
    HTTP_READ,
    HTTP_DONE,
    HTTP_LENGTH,	//	Content-Length:
    HTTP_FAILED,
} httpInfo_e;

typedef S32( * HTTP_response )( httpInfo_e code, StringEntry buffer, S32 length, void* notifyData );

//			HTTP_GetUrl should be used only for receiving some content from webpage
void		HTTP_GetUrl( StringEntry url, HTTP_response, void* notifyData, S32 resume_from );
//			HTTP_PostUrl should be used only when you want to post on webpage/database
void        HTTP_PostUrl( StringEntry url, HTTP_response, void* notifyData, StringEntry fmt, ... ) __attribute__( ( format( printf, 4, 5 ) ) );

#ifndef DEDICATED
void		HTTP_PostBug( StringEntry fileName );
void		HTTP_PostErrorNotice( StringEntry type, StringEntry msg );
#endif

S32			Net_HTTP_Init();
S32			Net_HTTP_Pump();
void		Net_HTTP_Kill();

#endif // USE_HTTP

//----(SA)  increased for larger submodel entity counts
#define MAX_MSGLEN					32768		// max length of a message, which may
//#define   MAX_MSGLEN              16384       // max length of a message, which may
// be fragmented into multiple packets
#define MAX_DOWNLOAD_WINDOW         8	// max of eight download frames
#define MAX_DOWNLOAD_BLKSIZE        2048	// 2048 U8 block chunks


/*
Netchan handles packet fragmentation and out of order / duplicate suppression
*/

typedef struct
{
    netsrc_t        sock;
    
    S32             dropped;	// between last packet and previous
    
    netadr_t        remoteAddress;
    S32             qport;		// qport value to write when transmitting
    
    // sequencing variables
    S32             incomingSequence;
    S32             outgoingSequence;
    
    // incoming fragment assembly buffer
    S32             fragmentSequence;
    S32             fragmentLength;
    U8            fragmentBuffer[MAX_MSGLEN];
    
    // outgoing fragment buffer
    // we need to space out the sending of large fragmented messages
    bool        unsentFragments;
    S32             unsentFragmentStart;
    S32             unsentLength;
    U8            unsentBuffer[MAX_MSGLEN];
    
} netchan_t;

void            Netchan_Init( S32 qport );
void            Netchan_Setup( netsrc_t sock, netchan_t* chan, netadr_t adr, S32 qport );

void            Netchan_Transmit( netchan_t* chan, S32 length, const U8* data );
void            Netchan_TransmitNextFragment( netchan_t* chan );

bool        Netchan_Process( netchan_t* chan, msg_t* msg );


/*
==============================================================

PROTOCOL

==============================================================
*/

// sent by the server, printed on connection screen, works for all clients
// (restrictions: does not handle \n, no more than 256 chars)
#define PROTOCOL_MISMATCH_ERROR "ERROR: Protocol Mismatch Between Client and Server.\
The server you are attempting to join is running an incompatible version of the game."

// long version used by the client in diagnostic window
#define PROTOCOL_MISMATCH_ERROR_LONG "ERROR: Protocol Mismatch Between Client and Server.\n\n\
The server you attempted to join is running an incompatible version of the game.\n\
You or the server may be running older versions of the game. Press the auto-update\
 button if it appears on the Main Menu screen."

#define GAMENAME_STRING "et"
#ifndef PRE_RELEASE_DEMO
// 2.56 - protocol 83
// 2.4 - protocol 80
// 1.33 - protocol 59
// 1.4 - protocol 60
#define ETPROTOCOL_VERSION    84
#else
// the demo uses a different protocol version for independant browsing
#define ETPROTOCOL_VERSION    72
#endif

// maintain a list of compatible protocols for demo playing
// NOTE: that stuff only works with two digits protocols
extern S32 demo_protocols[];

// NERVE - SMF - wolf multiplayer master servers
#ifndef MASTER_SERVER_NAME
#define MASTER_SERVER_NAME      "localhost"
#endif
#define MOTD_SERVER_NAME        "localhost"//"etmotd.idsoftware.com" // ?.?.?.?

// TTimo: override autoupdate server for testing
#ifndef AUTOUPDATE_SERVER_NAME
#define AUTOUPDATE_SERVER_NAME "127.0.0.1"
//#define AUTOUPDATE_SERVER_NAME "au2rtcw2.activision.com"
#endif

// TTimo: allow override for easy dev/testing..
// FIXME: not planning to support more than 1 auto update server
// see cons -- update_server=myhost
#define MAX_AUTOUPDATE_SERVERS  5
#if !defined( AUTOUPDATE_SERVER_NAME )
#define AUTOUPDATE_SERVER1_NAME   "au2rtcw1.activision.com"	// DHM - Nerve
#define AUTOUPDATE_SERVER2_NAME   "au2rtcw2.activision.com"	// DHM - Nerve
#define AUTOUPDATE_SERVER3_NAME   "au2rtcw3.activision.com"	// DHM - Nerve
#define AUTOUPDATE_SERVER4_NAME   "au2rtcw4.activision.com"	// DHM - Nerve
#define AUTOUPDATE_SERVER5_NAME   "au2rtcw5.activision.com"	// DHM - Nerve
#else
#define AUTOUPDATE_SERVER1_NAME   AUTOUPDATE_SERVER_NAME
#define AUTOUPDATE_SERVER2_NAME   AUTOUPDATE_SERVER_NAME
#define AUTOUPDATE_SERVER3_NAME   AUTOUPDATE_SERVER_NAME
#define AUTOUPDATE_SERVER4_NAME   AUTOUPDATE_SERVER_NAME
#define AUTOUPDATE_SERVER5_NAME   AUTOUPDATE_SERVER_NAME
#endif

#define PORT_MASTER         27950
#define PORT_MOTD           27950
#define PORT_SERVER         27960
#define NUM_SERVER_PORTS    4	// broadcast scan this many ports after
// PORT_SERVER so a single machine can
// run multiple servers


// override on command line, config files etc.
#if defined (USE_HTTP)
#ifndef AUTHORIZE_SERVER_NAME
#define AUTHORIZE_SERVER_NAME cl_authserver->string
#endif
#endif


// the svc_strings[] array in cl_parse.c should mirror this
//
// server to client
//
enum svc_ops_e
{
    svc_bad,
    svc_nop,
    svc_gamestate,
    svc_configstring,			// [short] [string] only in gamestate messages
    svc_baseline,				// only in gamestate messages
    svc_serverCommand,			// [string] to be executed by client game module
    svc_download,				// [short] size [size bytes]
    svc_snapshot,
    svc_EOF,
    
    // svc_extension follows a svc_EOF, followed by another svc_* ...
    //  this keeps legacy clients compatible.
    svc_extension
};


//
// client to server
//
enum clc_ops_e
{
    clc_bad,
    clc_nop,
    clc_move,					// [[usercmd_t]
    clc_moveNoDelta,			// [[usercmd_t]
    clc_clientCommand,			// [string] message
    clc_EOF,
};

void            Cbuf_Init( void );

// allocates an initial text buffer that will grow as needed

void            Cbuf_AddText( StringEntry text );

// Adds command text at the end of the buffer, does NOT add a final \n

void            Cbuf_ExecuteText( S32 exec_when, StringEntry text );

// this can be used in place of either Cbuf_AddText or Cbuf_InsertText

void            Cbuf_Execute( void );

// Pulls off \n terminated lines of text from the command buffer and sends
// them through Cmd_ExecuteString.  Stops when the buffer is empty.
// Normally called once per frame, but may be explicitly invoked.
// Do not call inside a command function, or current args will be destroyed.

void            Cdelay_Frame( void );

//Check if a delayed command have to be executed and decreases the remaining
//delay time for all of them

//===========================================================================

/*

Command execution takes a null terminated string, breaks it into tokens,
then searches for a command or variable that matches the first token.

*/

typedef void ( *xcommand_t )( void );

void            Cmd_Init( void );

void            Cmd_AddCommand( StringEntry cmd_name, xcommand_t function );

// called by the init functions of other parts of the program to
// register commands and functions to call for them.
// The cmd_name is referenced later, so it should not be in temp memory
// if function is NULL, the command will be forwarded to the server
// as a clc_clientCommand instead of executed locally

void            Cmd_RemoveCommand( StringEntry cmd_name );

void            Cmd_CommandCompletion( void ( *callback )( StringEntry s ) );

typedef void ( *completionFunc_t )( UTF8* args, S32 argNum );

void	Cmd_CommandCompletion( void( *callback )( StringEntry s ) );
void	Cmd_AliasCompletion( void( *callback )( StringEntry s ) );
void	Cmd_DelayCompletion( void( *callback )( StringEntry s ) );


void Cmd_SetCommandCompletionFunc( StringEntry command, completionFunc_t complete );
void Cmd_CompleteArgument( StringEntry command, UTF8* args, S32 argNum );
void Cmd_CompleteCfgName( UTF8* args, S32 argNum );

// callback with each valid string

S32             Cmd_Argc( void );
UTF8*           Cmd_Argv( S32 arg );
void            Cmd_ArgvBuffer( S32 arg, UTF8* buffer, S32 bufferLength );
UTF8*           Cmd_Args( void );
UTF8*           Cmd_ArgsFrom( S32 arg );
void            Cmd_ArgsBuffer( UTF8* buffer, S32 bufferLength );
UTF8*           Cmd_Cmd( void );
UTF8*           Cmd_Cmd_FromNth( S32 );
UTF8*           Cmd_EscapeString( StringEntry in );

// The functions that execute commands get their parameters with these
// functions. Cmd_Argv () will return an empty string, not a NULL
// if arg > argc, so string operations are allways safe.

void            Cmd_TokenizeString( StringEntry text );
void			Cmd_TokenizeStringIgnoreQuotes( StringEntry text_in );
void			Cmd_LiteralArgsBuffer( UTF8* buffer, S32 bufferLength );
void 			Cmd_SaveCmdContext( void );
void			Cmd_RestoreCmdContext( void );

// Takes a null terminated string.  Does not need to be /n terminated.
// breaks the string up into arg tokens.

void            Cmd_ExecuteString( StringEntry text );

// Parses a single line of text into arguments and tries to execute it
// as if it was typed at the console


/*
==============================================================

CVAR

==============================================================
*/

/*

cvar_t variables are used to hold scalar or string variables that can be changed
or displayed at the console or prog code as well as accessed directly
in C code.

The user can access cvars from the console in three ways:
r_draworder			prints the current value
r_draworder 0		sets the current value to 0
set r_draworder 0	as above, but creates the cvar if not present

Cvars are restricted from having the same names as commands to keep this
interface from being ambiguous.

The are also occasionally used to communicated information between different
modules of the program.

*/

cvar_t*         Cvar_Get( StringEntry var_name, StringEntry value, S32 flags );

// creates the variable if it doesn't exist, or returns the existing one
// if it exists, the value will not be changed, but flags will be ORed in
// that allows variables to be unarchived without needing bitflags
// if value is "", the value will not override a previously set value.

void            Cvar_Register( vmCvar_t* vmCvar, StringEntry varName, StringEntry defaultValue, S32 flags );

// basically a slightly modified Cvar_Get for the interpreted modules

void            Cvar_Update( vmCvar_t* vmCvar );

// updates an interpreted modules' version of a cvar

void            Cvar_Set( StringEntry var_name, StringEntry value );

// will create the variable with no flags if it doesn't exist

void            Cvar_SetLatched( StringEntry var_name, StringEntry value );

// don't set the cvar immediately

void            Cvar_SetValue( StringEntry var_name, F32 value );
void			Cvar_SetValueSafe( StringEntry var_name, F32 value );
void            Cvar_SetValueLatched( StringEntry var_name, F32 value );

// expands value to a string and calls Cvar_Set

F32           Cvar_VariableValue( StringEntry var_name );
S32             Cvar_VariableIntegerValue( StringEntry var_name );

// returns 0 if not defined or non numeric

UTF8*           Cvar_VariableString( StringEntry var_name );
void            Cvar_VariableStringBuffer( StringEntry var_name, UTF8* buffer, S32 bufsize );

// returns an empty string if not defined
void            Cvar_LatchedVariableStringBuffer( StringEntry var_name, UTF8* buffer, S32 bufsize );

// Gordon: returns the latched value if there is one, else the normal one, empty string if not defined as usual

S32	Cvar_Flags( StringEntry var_name );
void            Cvar_CommandCompletion( void ( *callback )( StringEntry s ) );

// callback with each valid string

void            Cvar_Reset( StringEntry var_name );

void            Cvar_SetCheatState( void );

// reset all testing vars to a safe value

bool        Cvar_Command( void );

// called by Cmd_ExecuteString when Cmd_Argv(0) doesn't match a known
// command.  Returns true if the command was a variable reference that
// was handled. (print or change)

void            Cvar_WriteVariables( fileHandle_t f );

// writes lines containing "set variable value" for all variables
// with the archive flag set to true.
void Cvar_CompleteCvarName( UTF8* args, S32 argNum );
void            Cvar_Init( void );

UTF8*           Cvar_InfoString( S32 bit );
UTF8*           Cvar_InfoString_Big( S32 bit );

// returns an info string containing all the cvars that have the given bit set
// in their flags ( CVAR_USERINFO, CVAR_SERVERINFO, CVAR_SYSTEMINFO, etc )
void            Cvar_InfoStringBuffer( S32 bit, UTF8* buff, S32 buffsize );
void            Cvar_CheckRange( cvar_t* cv, F32 minVal, F32 maxVal, bool shouldBeIntegral );

void            Cvar_Restart_f( void );

extern S32      cvar_modifiedFlags;

// whenever a cvar is modifed, its flags will be OR'd into this, so
// a single check can determine if any CVAR_USERINFO, CVAR_SERVERINFO,
// etc, variables have been modified since the last check.  The bit
// can then be cleared to allow another change detection.

/*
==============================================================

FILESYSTEM

No stdio calls should be used by any part of the game, because
we need to deal with all sorts of directory and seperator UTF8
issues.
==============================================================
*/

// referenced flags
// these are in loop specific order so don't change the order
#define FS_GENERAL_REF  0x01
#define FS_UI_REF       0x02
#define FS_CGAME_REF    0x04
#define FS_QAGAME_REF   0x08
// number of id paks that will never be autodownloaded from baseq3
#define NUM_ID_PAKS     9

#define MAX_FILE_HANDLES    64

#ifdef WIN32
#define Q_rmdir _rmdir
#else
#define Q_rmdir rmdir
#endif

bool        FS_Initialized();

void            FS_InitFilesystem( void );
void            FS_Shutdown( bool closemfp );

bool        FS_ConditionalRestart( S32 checksumFeed );
void            FS_Restart( S32 checksumFeed );

// shutdown and restart the filesystem so changes to fs_gamedir can take effect

UTF8**          FS_ListFiles( StringEntry directory, StringEntry extension, S32* numfiles );

// directory should not have either a leading or trailing /
// if extension is "/", only subdirectories will be returned
// the returned files will not include any directories or /

void            FS_FreeFileList( UTF8** list );

bool        FS_FileExists( StringEntry file );
bool        FS_OS_FileExists( StringEntry file );	// TTimo - test file existence given OS path

S32             FS_LoadStack();

S32             FS_GetFileList( StringEntry path, StringEntry extension, UTF8* listbuf, S32 bufsize );
S32             FS_GetModList( UTF8* listbuf, S32 bufsize );

fileHandle_t    FS_FOpenFileWrite( StringEntry qpath );

// will properly create any needed paths and deal with seperater character issues

S32             FS_filelength( fileHandle_t f );
fileHandle_t    FS_SV_FOpenFileWrite( StringEntry filename );
S32             FS_SV_FOpenFileRead( StringEntry filename, fileHandle_t* fp );
void            FS_SV_Rename( StringEntry from, StringEntry to );
S32             FS_FOpenFileRead( StringEntry qpath, fileHandle_t* file, bool uniqueFILE );

/*
if uniqueFILE is true, then a new FILE will be fopened even if the file
is found in an already open pak file.  If uniqueFILE is false, you must call
FS_FCloseFile instead of fclose, otherwise the pak FILE would be improperly closed
It is generally safe to always set uniqueFILE to true, because the majority of
file IO goes through FS_ReadFile, which Does The Right Thing already.
*/
/* TTimo
show_bug.cgi?id=506
added exclude flag to filter out regular dirs or pack files on demand
would rather have used FS_FOpenFileRead(..., S32 filter_flag = 0)
but that's a C++ construct ..
*/
#define FS_EXCLUDE_DIR 0x1
#define FS_EXCLUDE_PK3 0x2
S32             FS_FOpenFileRead_Filtered( StringEntry qpath, fileHandle_t* file, bool uniqueFILE, S32 filter_flag );

S32             FS_FileIsInPAK( StringEntry filename, S32* pChecksum );

// returns 1 if a file is in the PAK file, otherwise -1

S32             FS_Delete( UTF8* filename );	// only works inside the 'save' directory (for deleting savegames/images)
S32             FS_Write( const void* buffer, S32 len, fileHandle_t f );
S32				FS_FPrintf( fileHandle_t f, StringEntry fmt, ... );
S32             FS_Read2( void* buffer, S32 len, fileHandle_t f );
S32             FS_Read( void* buffer, S32 len, fileHandle_t f );

// properly handles partial reads and reads from other dlls

void            FS_FCloseFile( fileHandle_t f );

// note: you can't just fclose from another DLL, due to MS libc issues

S64	        FS_ReadFileDir( StringEntry qpath, void* searchPath, void** buffer );
S32             FS_ReadFile( StringEntry qpath, void** buffer );

// returns the length of the file
// a null buffer will just return the file length without loading
// as a quick check for existance. -1 length == not present
// A 0 U8 will always be appended at the end, so string ops are safe.
// the buffer should be considered read-only, because it may be cached
// for other uses.

void			FS_ForceFlush( fileHandle_t f );
// forces flush on files we're writing to.

void            FS_FreeFile( void* buffer );

// frees the memory returned by FS_ReadFile

void            FS_WriteFile( StringEntry qpath, const void* buffer, S32 size );

// writes a complete file, creating any subdirectories needed

S32             FS_filelength( fileHandle_t f );

// doesn't work for files that are opened from a pack file

S32             FS_FTell( fileHandle_t f );

// where are we?

void            FS_Flush( fileHandle_t f );

void       FS_Printf( fileHandle_t f, StringEntry fmt, ... ) __attribute__( ( format( printf, 2, 3 ) ) );

// like fprintf

S32             FS_FOpenFileByMode( StringEntry qpath, fileHandle_t* f, fsMode_t mode );

// opens a file for reading, writing, or appending depending on the value of mode

S32             FS_Seek( fileHandle_t f, S64 offset, S32 origin );

// seek on a file (doesn't work for zip files!!!!!!!!)

bool        FS_FilenameCompare( StringEntry s1, StringEntry s2 );

StringEntry     FS_GamePureChecksum( void );

// Returns the checksum of the pk3 from which the server loaded the qagame.qvm

StringEntry     FS_LoadedPakNames( void );
StringEntry     FS_LoadedPakChecksums( void );
StringEntry     FS_LoadedPakPureChecksums( void );

// Returns a space separated string containing the checksums of all loaded pk3 files.
// Servers with sv_pure set will get this string and pass it to clients.

StringEntry     FS_ReferencedPakNames( void );
StringEntry     FS_ReferencedPakChecksums( void );
StringEntry     FS_ReferencedPakPureChecksums( void );

// Returns a space separated string containing the checksums of all loaded
// AND referenced pk3 files. Servers with sv_pure set will get this string
// back from clients for pure validation

void            FS_ClearPakReferences( S32 flags );

// clears referenced booleans on loaded pk3s

void            FS_PureServerSetReferencedPaks( StringEntry pakSums, StringEntry pakNames );
void            FS_PureServerSetLoadedPaks( StringEntry pakSums, StringEntry pakNames );

// If the string is empty, all data sources will be allowed.
// If not empty, only pk3 files that match one of the space
// separated checksums will be checked for files, with the
// sole exception of .cfg files.

bool        FS_idPak( UTF8* pak, UTF8* base );
bool        FS_VerifyOfficialPaks( void );
bool        FS_ComparePaks( UTF8* neededpaks, S32 len, bool dlstring );

void            FS_Rename( StringEntry from, StringEntry to );

UTF8*           FS_BuildOSPath( StringEntry base, StringEntry game, StringEntry qpath );
void            FS_BuildOSHomePath( UTF8* ospath, S32 size, StringEntry qpath );

#if !defined( DEDICATED )
extern S32      cl_connectedToPureServer;
bool        FS_CL_ExtractFromPakFile( StringEntry base, StringEntry gamedir, StringEntry filename );
#endif

#if defined( DO_LIGHT_DEDICATED )
S32             FS_RandChecksumFeed();
#endif

UTF8*           FS_ShiftedStrStr( StringEntry string, StringEntry substring, S32 shift );
UTF8*           FS_ShiftStr( StringEntry string, S32 shift );

void            FS_CopyFile( UTF8* fromOSPath, UTF8* toOSPath );

UTF8*           FS_FindDll( StringEntry filename );

S32             FS_CreatePath( StringEntry OSPath );

bool        FS_VerifyPak( StringEntry pak );

bool        FS_IsPure( void );

U32    FS_ChecksumOSPath( UTF8* OSPath );

// XreaL BEGIN
void			FS_HomeRemove( StringEntry homePath );
// XreaL END

void            FS_FilenameCompletion( StringEntry dir, StringEntry ext, bool stripExt, void( *callback )( StringEntry s ) );

StringEntry     FS_GetCurrentGameDir( void );
bool        FS_Which( StringEntry filename, void* searchPath );

bool        FS_SV_FileExists( StringEntry file );
// return the current gamedir (eg. "main", "mymod"...)
StringEntry     FS_GetGameDir();
// remove a file from the homepath (eg. C:\users\(name)\My Documents\My Games\OpenWolf\; ~/.OpenWolf/).
bool        FS_OW_RemoveFile( StringEntry filepath );

/*
==============================================================

DOWNLOAD

==============================================================
*/

#include "dl_public.h"

/*
==============================================================

Edit fields and command line history/completion

==============================================================
*/

#define MAX_EDIT_LINE   256
typedef struct
{
    S32             cursor;
    S32             scroll;
    S32             widthInChars;
    UTF8            buffer[MAX_EDIT_LINE];
} field_t;

void Field_Clear( field_t* edit );
void Field_Set( field_t* edit, StringEntry text );
void Field_WordDelete( field_t* edit );
void Field_AutoComplete( field_t* edit, StringEntry prompt );
void Field_CompleteKeyname( void );
void Field_CompleteCgame( S32 argNum );
void Field_CompleteFilename( StringEntry dir, StringEntry ext, bool stripExt );
void Field_CompleteAlias( void );
void Field_CompleteDelay( void );
void Field_CompleteCommand( UTF8* cmd, bool doCommands, bool doCvars );

/*
==============================================================

MISC

==============================================================
*/

// centralizing the declarations for cl_cdkey
// (old code causing buffer overflows)
extern UTF8     cl_cdkey[34];
void            Com_AppendCDKey( StringEntry filename );
void            Com_ReadCDKey( StringEntry filename );

typedef struct gameInfo_s
{
    bool        spEnabled;
    S32         spGameTypes;
    S32         defaultSPGameType;
    S32         coopGameTypes;
    S32         defaultCoopGameType;
    S32         defaultGameType;
    bool        usesProfiles;
} gameInfo_t;

extern gameInfo_t com_gameInfo;

// TTimo
// centralized and cleaned, that's the max string you can send to a Com_Printf / Com_DPrintf (above gets truncated)
#define MAXPRINTMSG 4096

UTF8*           CopyString( StringEntry in );
void            Info_Print( StringEntry s );

void            Com_BeginRedirect( UTF8* buffer, S32 buffersize, void ( *flush )( UTF8* ) );
void            Com_EndRedirect( void );

void            Com_Quit_f( void );
S32             Com_EventLoop( void );
S32             Com_Milliseconds( void );	// will be journaled properly
U32        Com_BlockChecksum( const void* buffer, S32 length );
UTF8*           Com_MD5File( StringEntry fn, S32 length, StringEntry prefix, S32 prefix_len );
UTF8*           Com_MD5FileOWCompat( StringEntry filename );
S32             Com_Filter( UTF8* filter, UTF8* name, S32 casesensitive );
S32             Com_FilterPath( UTF8* filter, UTF8* name, S32 casesensitive );
S32             Com_RealTime( qtime_t* qtime );
bool            Com_SafeMode( void );

void            Com_StartupVariable( StringEntry match );
void            Com_SetRecommended();

void            Sys_WriteDump( StringEntry fmt, ... );

// checks for and removes command line "+set var arg" constructs
// if match is NULL, all set commands will be executed, otherwise
// only a set with the exact name.  Only used during startup.

//bani - profile functions
void            Com_TrackProfile( UTF8* profile_path );
bool        Com_CheckProfile( UTF8* profile_path );
bool        Com_WriteProfile( UTF8* profile_path );

extern cvar_t*  com_crashed;

extern cvar_t*  com_ignorecrash;	//bani

extern cvar_t*  com_protocol;
extern cvar_t*  com_pid;		//bani

extern cvar_t*  com_developer;
extern cvar_t*  com_dedicated;
extern cvar_t*  com_speeds;
extern cvar_t*  com_timescale;
extern cvar_t*  com_sv_running;
extern cvar_t*  com_cl_running;
extern cvar_t*  com_viewlog;	// 0 = hidden, 1 = visible, 2 = minimized
extern cvar_t*  com_version;

//extern    cvar_t  *com_blood;
extern cvar_t*  com_buildScript;	// for building release pak files
extern cvar_t*  com_journal;
extern cvar_t*  com_cameraMode;
extern cvar_t*  com_ansiColor;
extern cvar_t*  com_logosPlaying;

extern cvar_t*  com_unfocused;
extern cvar_t*  com_minimized;


// watchdog
extern cvar_t*  com_watchdog;
extern cvar_t*  com_watchdog_cmd;

#if defined (USE_HTTP)
extern cvar_t*  com_sessionid;
#endif

// both client and server must agree to pause
extern cvar_t*  cl_paused;
extern cvar_t*  sv_paused;

extern cvar_t*  cl_packetdelay;
extern cvar_t*  sv_packetdelay;

// com_speeds times
extern S32      time_game;
extern S32      time_frontend;
extern S32      time_backend;	// renderer backend time

extern S32      com_frameTime;
extern S32      com_frameMsec;
extern S32      com_expectedhunkusage;
extern S32      com_hunkusedvalue;

extern bool com_errorEntered;

extern fileHandle_t com_journalFile;
extern fileHandle_t com_journalDataFile;

typedef enum
{
    TAG_FREE,
    TAG_GENERAL,
    TAG_BOTLIB,
    TAG_RENDERER,
    TAG_SMALL,
    TAG_CRYPTO,
    TAG_STATIC
} memtag_t;

/*

--- low memory ----
server vm
server clipmap
---mark---
renderer initialization (shaders, etc)
UI vm
cgame vm
renderer map
renderer models

---free---

temp file loading
--- high memory ---

*/

#if defined( _DEBUG ) && !defined( BSPC )
#define ZONE_DEBUG
#endif

#ifdef ZONE_DEBUG
#define Z_TagMalloc( size, tag )          Z_TagMallocDebug( size, tag, # size, __FILE__, __LINE__ )
#define Z_Malloc( size )                  Z_MallocDebug( size, # size, __FILE__, __LINE__ )
#define S_Malloc( size )                  S_MallocDebug( size, # size, __FILE__, __LINE__ )
void*           Z_TagMallocDebug( S32 size, S32 tag, UTF8* label, UTF8* file, S32 line );	// NOT 0 filled memory
void*           Z_MallocDebug( S32 size, UTF8* label, UTF8* file, S32 line );	// returns 0 filled memory
void*           S_MallocDebug( S32 size, UTF8* label, UTF8* file, S32 line );	// returns 0 filled memory
#else
void*           Z_TagMalloc( S32 size, S32 tag );	// NOT 0 filled memory
void*           Z_Malloc( S32 size );	// returns 0 filled memory
void*           S_Malloc( S32 size );	// NOT 0 filled memory only for small allocations
#endif
void            Z_Free( void* ptr );
void            Z_FreeTags( S32 tag );
S32             Z_AvailableMemory( void );
void            Z_LogHeap( void );

void            Hunk_Clear( void );
void            Hunk_ClearToMark( void );
void            Hunk_SetMark( void );
bool        Hunk_CheckMark( void );

//void *Hunk_Alloc( S32 size );
// void *Hunk_Alloc( S32 size, ha_pref preference );
void            Hunk_ClearTempMemory( void );
void*           Hunk_AllocateTempMemory( S32 size );
void            Hunk_FreeTempMemory( void* buf );
S32             Hunk_MemoryRemaining( void );
void            Hunk_SmallLog( void );
void            Hunk_Log( void );

void            Com_TouchMemory( void );
void            Com_ReleaseMemory( void );

// commandLine should not include the executable name (argv[0])
void            Com_Init( UTF8* commandLine );
void            Com_Frame( void );
void            Com_Shutdown( bool badProfile );

void			CL_ShutdownCGame( void );
void			CL_ShutdownUI( void );
void			SV_ShutdownGameProgs( void );

/*
==============================================================

CLIENT / SERVER SYSTEMS

==============================================================
*/

//
// client interface
//
void            CL_InitKeyCommands( void );

// the keyboard binding interface must be setup before execing
// config files, but the rest of client startup will happen later

void            CL_Init( void );
void            CL_ClearStaticDownload( void );
void            CL_Disconnect( bool showMainMenu );
void            CL_Shutdown( void );
void            CL_Frame( S32 msec );
bool            CL_GameCommand( void );
void            CL_KeyEvent( S32 key, S32 down, U32 time );
void       CL_RefPrintf( S32 print_level, StringEntry fmt, ... );
void            CL_CharEvent( S32 key );

// UTF8 events are for field typing, not game control

void            CL_MouseEvent( S32 dx, S32 dy, S32 time );

void            CL_JoystickEvent( S32 axis, S32 value, S32 time );

void            CL_PacketEvent( netadr_t from, msg_t* msg );

void            CL_ConsolePrint( UTF8* text );

void            CL_MapLoading( void );

// do a screen update before starting to load a map
// when the server is going to load a new map, the entire hunk
// will be cleared, so the client must shutdown cgame, ui, and
// the renderer

void            CL_ForwardCommandToServer( StringEntry string );

// adds the current command line as a clc_clientCommand to the client message.
// things like godmode, noclip, etc, are commands directed to the server,
// so when they are typed in at the console, they will need to be forwarded.

void            CL_CDDialog( void );

// bring up the "need a cd to play" dialog

void            CL_ShutdownAll( void );

// shutdown all the client stuff

void            CL_FlushMemory( void );

// dump all memory on an error

void            CL_StartHunkUsers( void );

void DB_InitExportTable( void );
void DB_ShutdownGameProgs( void );
void DB_InitGameProgs( void );

// start all the client stuff using the hunk

#if !defined(UPDATE_SERVER)
void            CL_CheckAutoUpdate( void );
bool        CL_NextUpdateServer( void );
void            CL_GetAutoUpdate( void );
#endif


void			Key_KeynameCompletion( void( *callback )( StringEntry s ) );
// for keyname autocompletion

void			CL_CgameCompletion( void( *callback )( StringEntry s ), S32 argNum );
// for cgame command autocompletion

void            Key_WriteBindings( fileHandle_t f );

void            SCR_DebugGraph( F32 value, S32 color );	// FIXME: move logging to common?


// AVI files have the start of pixel lines 4 U8-aligned
#define AVI_LINE_PADDING 4


//
// server interface
//
void            SV_Init( void );
void            SV_Shutdown( UTF8* finalmsg );
void            SV_Frame( S32 msec );
void            SV_PacketEvent( netadr_t from, msg_t* msg );
bool        SV_GameCommand( void );


//
// UI interface
//
bool        UI_GameCommand( void );
bool        UI_usesUniqueCDKey();

/*
==============================================================

NON-PORTABLE SYSTEM SERVICES

==============================================================
*/

#define MAX_JOYSTICK_AXIS 16
#if !defined ( BSPC )
typedef enum
{
    // bk001129 - make sure SE_NONE is zero
    SE_NONE = 0,				// evTime is still valid
    SE_KEY,						// evValue is a key code, evValue2 is the down flag
    SE_CHAR,					// evValue is an ascii UTF8
    SE_MOUSE,					// evValue and evValue2 are reletive signed x / y moves
    SE_JOYSTICK_AXIS,			// evValue is an axis number and evValue2 is the current state (-127 to 127)
    SE_CONSOLE,					// evPtr is a UTF8*
    SE_PACKET					// evPtr is a netadr_t followed by data bytes to evPtrLength
} sysEventType_t;

typedef struct sysEvent_s
{
    S32             evTime;
    sysEventType_t  evType;
    S32             evValue, evValue2;
    S32             evPtrLength;	// bytes of data pointed to by evPtr, for journaling
    void*           evPtr;		// this must be manually freed if not NULL
} sysEvent_t;

void			Com_QueueEvent( S32 time, sysEventType_t type, S32 value, S32 value2, S32 ptrLength, void* ptr );
S32				Com_EventLoop( void );
sysEvent_t		Com_GetSystemEvent( void );
#endif

void            Sys_Init( void );
bool            Sys_IsNumLockDown( void );

UTF8*           Sys_GetDLLName( StringEntry name );

void*	        Sys_LoadDll( StringEntry name );
void*	        Sys_LoadSystemDll( StringEntry name );
void*	        Sys_GetProcAddress( void* dllhandle, StringEntry name );
void            Sys_UnloadDll( void* dllHandle );
void            Sys_UnloadGame( void );
void*           Sys_GetGameAPI( void* parms );

void            Sys_UnloadCGame( void );
void*           Sys_GetCGameAPI( void );

void            Sys_UnloadUI( void );
void*           Sys_GetUIAPI( void );

void*	   	Sys_LoadFunction( void* dllHandle, StringEntry functionName );
UTF8*	   	Sys_DLLError();

// RB: added to link OS specific pointers to the renderer.dll space
void*           Sys_GetSystemHandles( void );

UTF8*           Sys_GetCurrentUser( void );

void       Sys_Error( StringEntry error, ... ) __attribute__( ( format( printf, 1, 2 ) ) );
void            Sys_Quit( void );
UTF8*           Sys_GetClipboardData( void );	// note that this isn't journaled...

void            Sys_Print( StringEntry msg );


// Sys_Milliseconds should only be used for profiling purposes,
// any game related timing information should come from event timestamps
S32             Sys_Milliseconds( void );

void            Sys_SnapVector( F32* v );

bool		Sys_RandomBytes( U8* string, S32 len );

// the system console is shown when a dedicated server is running
void            Sys_DisplaySystemConsole( bool show );
void            Sys_ShowConsole( S32 level, bool quitOnClose );
void            Sys_SetErrorText( StringEntry text );

void            Sys_SendPacket( S32 length, const void* data, netadr_t to );
bool        Sys_GetPacket( netadr_t* net_from, msg_t* net_message );

bool		Sys_StringToAdr( StringEntry s, netadr_t* a, netadrtype_t family );

//Does NOT parse port numbers, only base addresses.

bool        Sys_IsLANAddress( netadr_t adr );
void            Sys_ShowIP( void );

bool        Sys_Mkdir( StringEntry path );
UTF8*           Sys_Cwd( void );
UTF8*           Sys_DefaultInstallPath( void );

#ifdef MACOS_X
UTF8*           Sys_DefaultAppPath( void );
#endif

void           Sys_SetDefaultLibPath( StringEntry path );
UTF8*          Sys_DefaultLibPath( void );

UTF8*           Sys_DefaultHomePath( UTF8* buffer, S32 size );
bool        Sys_Fork( StringEntry path, StringEntry cmdLine );
StringEntry     Sys_TempPath( void );
StringEntry     Sys_Dirname( UTF8* path );
StringEntry     Sys_Basename( UTF8* path );
UTF8*           Sys_ConsoleInput( void );

UTF8**          Sys_ListFiles( StringEntry directory, StringEntry extension, UTF8* filter, S32* numfiles, bool wantsubs );
void            Sys_FreeFileList( UTF8** list );

void			Sys_Sleep( S32 msec );

bool        Sys_OpenUrl( StringEntry url );

bool        Sys_LowPhysicalMemory();
S32    Sys_ProcessorCount();

void			Sys_SetEnv( StringEntry name, StringEntry value );

typedef enum
{
    DR_YES = 0,
    DR_NO = 1,
    DR_OK = 0,
    DR_CANCEL = 1
} dialogResult_t;

typedef enum
{
    DT_INFO,
    DT_WARNING,
    DT_ERROR,
    DT_YES_NO,
    DT_OK_CANCEL
} dialogType_t;

dialogResult_t	Sys_Dialog( dialogType_t type, StringEntry message, StringEntry title );

bool		Sys_WritePIDFile( void );

// NOTE TTimo - on win32 the cwd is prepended .. non portable behaviour
void            Sys_StartProcess( UTF8* exeName, bool doexit );	// NERVE - SMF
void            Sys_OpenURL( StringEntry url, bool doexit );	// NERVE - SMF

#ifdef __linux__
// TTimo only on linux .. maybe on Mac too?
// will OR with the existing mode (chmod ..+..)
void            Sys_Chmod( UTF8* file, S32 mode );
#endif

void Hist_Load( void );
void Hist_Add( StringEntry field );
StringEntry Hist_Next( void );
StringEntry Hist_Prev( void );

/* This is based on the Adaptive Huffman algorithm described in Sayood's Data
 * Compression book.  The ranks are not actually stored, but implicitly defined
 * by the location of a node within a doubly-linked list */

#define NYT HMAX				/* NYT = Not Yet Transmitted */
#define INTERNAL_NODE ( HMAX + 1 )

#ifndef BSPC
typedef struct nodetype
{
    struct nodetype* left, *right, *parent;	/* tree structure */
    struct nodetype* next, *prev;	/* doubly-linked list */
    struct nodetype** head;		/* highest ranked node in block */
    S32             weight;
    S32             symbol;
} node_t;


#define HMAX 256				/* Maximum symbol */

typedef struct
{
    S32             blocNode;
    S32             blocPtrs;
    
    node_t*         tree;
    node_t*         lhead;
    node_t*         ltail;
    node_t*         loc[HMAX + 1];
    node_t**        freelist;
    
    node_t          nodeList[768];
    node_t*         nodePtrs[768];
} huff_t;

typedef struct
{
    huff_t          compressor;
    huff_t          decompressor;
} huffman_t;

void            Huff_Compress( msg_t* buf, S32 offset );
void            Huff_Decompress( msg_t* buf, S32 offset );
void            Huff_Init( huffman_t* huff );
void            Huff_addRef( huff_t* huff, U8 ch );
S32             Huff_Receive( node_t* node, S32* ch, U8* fin );
void            Huff_transmit( huff_t* huff, S32 ch, U8* fout );
void            Huff_offsetReceive( node_t* node, S32* ch, U8* fin, S32* offset );
void            Huff_offsetTransmit( huff_t* huff, S32 ch, U8* fout, S32* offset );
void            Huff_putBit( S32 bit, U8* fout, S32* offset );
S32             Huff_getBit( U8* fout, S32* offset );

// don't use if you don't know what you're doing.
S32				Huff_getBloc( void );
void			Huff_setBloc( S32 _bloc );

extern huffman_t clientHuffTables;
#endif

#define SV_ENCODE_START     4
#define SV_DECODE_START     12
#define CL_ENCODE_START     12
#define CL_DECODE_START     4

S32				Parse_AddGlobalDefine( UTF8* string );
S32				Parse_LoadSourceHandle( StringEntry filename );
S32				Parse_FreeSourceHandle( S32 handle );
S32				Parse_ReadTokenHandle( S32 handle, pc_token_t* pc_token );
S32				Parse_SourceFileAndLine( S32 handle, UTF8* filename, S32* line );

void            Com_GetHunkInfo( S32* hunkused, S32* hunkexpected );
void            Com_RandomBytes( U8* string, S32 len );

#if !defined ( BSPC )
void            Com_QueueEvent( S32 time, sysEventType_t type, S32 value, S32 value2, S32 ptrLength, void* ptr );
#endif

#endif //!__QCOMMON_H__