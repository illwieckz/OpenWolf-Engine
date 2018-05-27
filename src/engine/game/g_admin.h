////////////////////////////////////////////////////////////////////////////////////////
// Copyright(C) 2004 - 2006 Tony J.White
// Copyright(C) 2011 - 2018 Dusan Jocic <dusanjocic@msn.com>
//
// This file is part of OpenWolf.
//
// This shrubbot implementation is the original work of Tony J.White.
//
// Contains contributions from Wesley van Beelen, Chris Bajumpaa, Josh Menke,
// and Travis Maurer.
//
// The functionality of this code mimics the behaviour of the currently
// inactive project shrubet(http://www.etstats.com/shrubet/index.php?ver=2)
// by Ryan Mannion.However, shrubet was a closed - source project and
// none of it's code has been copied, only it's functionality.
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
// -------------------------------------------------------------------------------------
// File name:   g_admin.h
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#ifndef __G_ADMIN_H__
#define __G_ADMIN_H__

#define MAX_ADMIN_LEVELS 32
#define MAX_ADMIN_ADMINS 1024
#define MAX_ADMIN_BANS 1024
#define MAX_ADMIN_NAMELOGS 128
#define MAX_ADMIN_NAMELOG_NAMES 5
#define MAX_ADMIN_FLAGS 64
#define MAX_ADMIN_COMMANDS 64
#define MAX_ADMIN_CMD_LEN 20
#define MAX_ADMIN_BAN_REASON 50
#define MAX_ADMIN_ADMINLOGS 128
#define MAX_ADMIN_ADMINLOG_ARGS 50

/*
 * 1 - cannot be vote kicked, vote muted
 * 2 - cannot be censored or flood protected TODO
 * 3 - never loses credits for changing teams
 * 4 - can see team chat as a spectator
 * 5 - can switch teams any time, regardless of balance
 * 6 - does not need to specify a reason for a kick/ban
 * 7 - can call a vote at any time (regardless of a vote being disabled or
 * voting limitations)
 * 8 - does not need to specify a duration for a ban
 * 9 - can run commands from team chat
 * 0 - inactivity rules do not apply to them
 * ! - admin commands cannot be used on them
 * @ - does not show up as an admin in !listplayers
 * $ - sees all information in !listplayers
 * ? - receieves and can send /a admin messages
 */
#define ADMF_IMMUNITY '1'
#define ADMF_NOCENSORFLOOD '2' /* TODO */
#define ADMF_TEAMCHANGEFREE '3'
#define ADMF_SPEC_ALLCHAT '4'
#define ADMF_FORCETEAMCHANGE '5'
#define ADMF_UNACCOUNTABLE '6'
#define ADMF_NO_VOTE_LIMIT '7'
#define ADMF_CAN_PERM_BAN '8'
#define ADMF_TEAMCHAT_CMD '9'
#define ADMF_ACTIVITY '0'
#define ADMF_IMMUTABLE '!'
#define ADMF_INCOGNITO '@'
#define ADMF_SEESFULLLISTPLAYERS '$'
#define ADMF_ADMINCHAT '?'

#define MAX_ADMIN_LISTITEMS 20
#define MAX_ADMIN_SHOWBANS 10

// important note: QVM does not seem to allow a single UTF8 to be a
// member of a struct at init time.  flag has been converted to UTF8*
typedef struct g_admin_cmd
{
    UTF8* keyword;
    bool( * handler )( gentity_t* ent, S32 skiparg );
    UTF8* flag;
    UTF8* function;  // used for !help
    UTF8* syntax;  // used for !help
} g_admin_cmd_t;

typedef struct g_admin_level
{
    S32 level;
    UTF8 name[ MAX_NAME_LENGTH ];
    UTF8 flags[ MAX_ADMIN_FLAGS ];
} g_admin_level_t;

typedef struct g_admin_admin
{
    UTF8 guid[ 33 ];
    UTF8 name[ MAX_NAME_LENGTH ];
    S32 level;
    UTF8 flags[ MAX_ADMIN_FLAGS ];
    UTF8 pubkey[RSA_STRING_LENGTH];
    UTF8 msg[RSA_STRING_LENGTH];
    UTF8 msg2[RSA_STRING_LENGTH];
    S32 counter;
} g_admin_admin_t;

typedef struct g_admin_ban
{
    UTF8 name[ MAX_NAME_LENGTH ];
    UTF8 guid[ 33 ];
    UTF8 ip[ 40 ];
    UTF8 reason[ MAX_ADMIN_BAN_REASON ];
    UTF8 made[ 18 ]; // big enough for strftime() %c
    S32 expires;
    UTF8 banner[ MAX_NAME_LENGTH ];
} g_admin_ban_t;

typedef struct g_admin_command
{
    UTF8 command[ MAX_ADMIN_CMD_LEN ];
    UTF8 exec[ MAX_QPATH ];
    UTF8 desc[ 50 ];
    S32 levels[ MAX_ADMIN_LEVELS + 1 ];
} g_admin_command_t;

typedef struct g_admin_namelog
{
    UTF8 name[ MAX_ADMIN_NAMELOG_NAMES ][MAX_NAME_LENGTH ];
    UTF8 ip[ 40 ];
    UTF8 guid[ 33 ];
    S32 slot;
    bool banned;
} g_admin_namelog_t;

typedef struct g_admin_adminlog
{
    UTF8 name[MAX_NAME_LENGTH];
    UTF8 command[MAX_ADMIN_CMD_LEN];
    UTF8 args[MAX_ADMIN_ADMINLOG_ARGS];
    S32 id;
    S32 time;
    S32 level;
    bool success;
} g_admin_adminlog_t;

//
// idAdminLocal
//
class idAdminLocal
{
public:
    bool AdminBanCheck( UTF8* userinfo, UTF8* reason, S32 rlen );
    bool AdminCmdCheck( gentity_t* ent, bool say );
    static bool AdminReadconfig( gentity_t* ent, S32 skiparg );
    void AdminWriteConfig( void );
    bool AdminPermission( gentity_t* ent, UTF8 flag );
    bool AdminNameCheck( gentity_t* ent, UTF8* name, UTF8* err, S32 len );
    void AdminNamelogUpdate( gclient_t* ent, bool disconnect );
    g_admin_admin_t* Admin( gentity_t* ent );
    void AdminPubkey( void );
    S32 AdminParseTime( StringEntry time );
    
    static bool AdminTime( gentity_t* ent, S32 skiparg );
    static bool AdminSetlevel( gentity_t* ent, S32 skiparg );
    static bool AdminKick( gentity_t* ent, S32 skiparg );
    static bool AdminAdjustBan( gentity_t* ent, S32 skiparg );
    static bool AdminBan( gentity_t* ent, S32 skiparg );
    static bool AdminUnBan( gentity_t* ent, S32 skiparg );
    static bool AdminPutTeam( gentity_t* ent, S32 skiparg );
    static bool AdminListAdmins( gentity_t* ent, S32 skiparg );
    static bool AdminListLayouts( gentity_t* ent, S32 skiparg );
    static bool AdminListPlayers( gentity_t* ent, S32 skiparg );
    static bool AdminMap( gentity_t* ent, S32 skiparg );
    static bool AdminMute( gentity_t* ent, S32 skiparg );
    static bool AdminDenyBuild( gentity_t* ent, S32 skiparg );
    static bool AdminShowBans( gentity_t* ent, S32 skiparg );
    static bool AdminHelp( gentity_t* ent, S32 skiparg );
    static bool AdminAdminTest( gentity_t* ent, S32 skiparg );
    static bool AdminAllReady( gentity_t* ent, S32 skiparg );
    static bool AdminCancelVote( gentity_t* ent, S32 skiparg );
    static bool AdminPassVote( gentity_t* ent, S32 skiparg );
    static bool AdminSpec999( gentity_t* ent, S32 skiparg );
    static bool AdminRename( gentity_t* ent, S32 skiparg );
    static bool AdminRestart( gentity_t* ent, S32 skiparg );
    static bool AdminNextmap( gentity_t* ent, S32 skiparg );
    static bool AdminNameLog( gentity_t* ent, S32 skiparg );
    static bool AdminLock( gentity_t* ent, S32 skiparg );
    static bool AdminUnlock( gentity_t* ent, S32 skiparg );
    static bool AdminBot( gentity_t* ent, S32 skiparg );
    void AdminPrint( gentity_t* ent, UTF8* m );
    void AdminBufferPrint( gentity_t* ent, UTF8* m );
    void AdminBufferBegin( void );
    void AdminBufferEnd( gentity_t* ent );
    void AdminDuration( S32 secs, UTF8* duration, S32 dursize );
    void AdminCleanup( void );
    void AdminNamelogCleanup( void );
    bool AdminCreateBan( gentity_t* ent, UTF8* netname, UTF8* guid, UTF8* ip, S32 seconds, UTF8* reason );
    bool AdminHigherGuid( UTF8* admin_guid, UTF8* victim_guid );
    bool AdminHigher( gentity_t* admin, gentity_t* victim );
    void AdminWriteConfigString( UTF8* s, fileHandle_t f );
    void AdminWriteConfigInt( S32 v, fileHandle_t f );
    void AdminReadConfigString( UTF8** cnf, UTF8* s, S32 size );
    void AdminReadConfigInt( UTF8** cnf, S32* v );
    void AdminDefaultLevels( void );
    bool AdminCommandPermission( gentity_t* ent, UTF8* command );
    void AdminLog( gentity_t* admin, UTF8* cmd, S32 skiparg );
    S32 AdminListAdmins( gentity_t* ent, S32 start, UTF8* search );
    
public:
#define AP(x) trap_SendServerCommand(-1, x)
#define CP(x) trap_SendServerCommand(ent-g_entities, x)
#define CPx(x, y) trap_SendServerCommand(x, y)
#define ADMP(x) AdminPrint(ent, x)
#define ADMBP(x) AdminBufferPrint(ent, x)
#define ADMBP_begin() AdminBufferBegin()
#define ADMBP_end() AdminBufferEnd(ent)
};

extern idAdminLocal adminLocal;

#endif //!_G_ADMIN_H__ 
