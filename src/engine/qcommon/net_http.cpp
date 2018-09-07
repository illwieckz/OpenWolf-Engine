////////////////////////////////////////////////////////////////////////////////////////
// Copyright(C) 2007 HermitWorks Entertainment Corporation
// Copyright(C) 2011 - 2018 Dusan Jocic <dusanjocic@msn.com>
//
// OpenWolf is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// OpenWolf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// -------------------------------------------------------------------------------------
// File name:   net_http.cpp
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

#if defined(USE_HTTP)

#define MAX_COOKIE_SIZE			256

typedef struct
{
    CURL* 			handle;
    void* 			notifyData;
    HTTP_response	response;
    UTF8			postfields[ 4096 ];
    S32				code;
    
    
    struct curl_httppost* 	firstitem;
    struct curl_httppost* 	lastitem;
    struct curl_slist* 		headerlist;
    
} httpTaskInfo_t;

typedef struct
{

    CURLM* 					multi_handle;
    S32						still_running;
    struct	curl_slist* 	headers;
    
} httpInfo_t;


static httpInfo_t		http;
static httpTaskInfo_t	tasks[ 8 ];

cvar_t*	http_bugauth;
cvar_t* 	http_usernameandpassword;

/*
==================
Net_HTTP_write_callback

this is called when there is data to be receieved from the websever
==================
*/
U64 Net_HTTP_write_callback( StringEntry buffer, U64 size, U64 nitems, void* outstream )
{
    httpTaskInfo_t* task = ( httpTaskInfo_t* )outstream;
    
    if( task->response && task->code < 300 )
    {
        return task->response( HTTP_WRITE, buffer, size * nitems, task->notifyData );
    }
    
    return size * nitems;
}

/*
==================
Net_HTTP_read_callback

this is called when there is data to be receieved from the websever
==================
*/
U64 Net_HTTP_read_callback( StringEntry buffer, U64 size, U64 nitems, void* outstream )
{

    httpTaskInfo_t* task = ( httpTaskInfo_t* )outstream;
    
    if( task->response )
    {
        return task->response( HTTP_READ, buffer, size * nitems, task->notifyData );
    }
    
    return size * nitems;
}


/*
==================
Net_HTTP_Init
==================
*/
S32 Net_HTTP_Init()
{
    http.multi_handle = curl_multi_init();
    
    if( http.multi_handle == NULL )
    {
        return 0;
    }
    
    http_usernameandpassword = Cvar_Get( "usernameandpassword", "username:password", CVAR_ARCHIVE );
    http_bugauth = Cvar_Get( "http_bugauth", "username:password", CVAR_ARCHIVE );
    
    http.headers = curl_slist_append( http.headers, "Accept: application/openwolf" );
    return -1;
}

/*
==================
Net_HTTP_Cleanup
==================
*/
void Net_HTTP_Cleanup( CURL* handle )
{
    curl_multi_remove_handle( http.multi_handle, handle );
    curl_easy_cleanup( handle );
}

/*
==================
Net_HTTP_Kill
==================
*/
void Net_HTTP_Kill()
{
    S32 i;
    
    curl_slist_free_all( http.headers );
    http.headers = NULL;
    
    for( i = 0; i < lengthof( tasks ); i++ )
    {
        if( tasks[i].handle )
        {
            Net_HTTP_Cleanup( tasks[i].handle );
            tasks[i].handle = NULL;
        }
    }
    curl_multi_cleanup( http.multi_handle );
}

/*
==================
Net_HTTP_EscapeField
==================
*/
void Net_HTTP_EscapeField( UTF8* out_url, StringEntry in_url, S32 len )
{
    UTF8* url;
    url = curl_escape( in_url, 0 );
    Q_strncpyz( out_url, url, len );
    curl_free( url );
}

/*
==================
Net_HTTP_ParseHeader
==================
*/
U64 Net_HTTP_ParseHeader( StringEntry ptr, U64 size, U64 nmemb, void* stream )
{

    httpTaskInfo_t* task = ( httpTaskInfo_t* )stream;
    
    if( task->response )
    {
    
        S32 httpversion_major, httpversion, httpcode;
        
        if( sscanf( ptr, " HTTP/%d.%d %3d", &httpversion_major, &httpversion, &httpcode ) == 3 )
        {
        
            task->code = httpcode;
            
            if( httpcode >= 400 )
            {
            
                Com_Error( ERR_DROP, "HTTP Error: (%d)\n", httpcode );
                
                task->response( HTTP_FAILED, ptr, httpcode, task->notifyData );
                task->response = 0;	// don't send anymore messages
            }
            
        }
        else
        {
        
            if( task->code < 300 )
            {
                UTF8* title = COM_Parse( &ptr );
                if( !Q_stricmp( title, "Content-Length:" ) )
                {
                    UTF8* value = COM_Parse( &ptr );
                    task->response( HTTP_LENGTH, value, strlen( value ), task->notifyData );
                }
                else if( !Q_stricmp( title, "Set-Cookie:" ) )
                {
                    UTF8 tmp[ MAX_INFO_STRING ];
                    
                    for( tmp[ 0 ] = '\0'; ; )
                    {
                    
                        UTF8* value = COM_ParseExt( &ptr, false );
                        
                        if( value && value[0] )
                        {
                            Q_strcat( tmp, sizeof( tmp ), value );
                            Q_strcat( tmp, sizeof( tmp ), " " );
                        }
                        else
                            break;
                    }
                    
                    Cvar_Set( "com_sessionid", tmp );
                }
            }
        }
    }
    
    
    return size * nmemb;
}


/*
==================
Net_HTTP_Pump
==================
*/
S32 Net_HTTP_Pump()
{
    struct timeval timeout;
    S32 rc; /* select() return code */
    fd_set fdread;
    fd_set fdwrite;
    fd_set fdexcep;
    S32 maxfd;
    
    if( !http.still_running )
        return 0;
        
    FD_ZERO( &fdread );
    FD_ZERO( &fdwrite );
    FD_ZERO( &fdexcep );
    
    /* we are polling, so no timeout */
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    
    /* get file descriptors from the transfers */
    curl_multi_fdset( http.multi_handle, &fdread, &fdwrite, &fdexcep, &maxfd );
    
    rc = select( maxfd + 1, &fdread, &fdwrite, &fdexcep, &timeout );
    switch( rc )
    {
        case -1:
            /* select error */
            break;
        case 0:
            /* timeout, do something else */
            break;
        default:
        {
            /* one or more of curl's file descriptors say there's data to read or write */
            while( CURLM_CALL_MULTI_PERFORM == curl_multi_perform( http.multi_handle, &http.still_running ) );
            
        }
        break;
    }
    
    if( !http.still_running )
    {
        S32 i;
        for( i = 0; i < lengthof( tasks ); i++ )
        {
        
            if( tasks[ i ].handle )
            {
                if( tasks[ i ].response && tasks[ i ].code < 300 )
                {
                    tasks[ i ].response( HTTP_DONE, 0, 0, tasks[i].notifyData );
                }
                if( tasks[i].firstitem )
                    curl_formfree( tasks[i].firstitem );
                    
                if( tasks[i].headerlist )
                    curl_slist_free_all( tasks[i].headerlist );
                    
                curl_multi_remove_handle( http.multi_handle, tasks[i].handle );
                curl_easy_cleanup( tasks[i].handle );
                
                ::memset( tasks + i, 0, sizeof( httpTaskInfo_t ) );
            }
        }
    }
    
    return http.still_running;
}


static httpTaskInfo_t* new_task()
{
    S32 i;
    for( i = 0; i < lengthof( tasks ); i++ )
    {
        if( tasks[i].handle == NULL )
        {
            ::memset( tasks + i, 0, sizeof( httpTaskInfo_t ) );
            return tasks + i;
        }
    }
    return 0;
}


/*
==================
HTTP_GetUrl
==================
*/
void HTTP_GetUrl( StringEntry url, HTTP_response response, void* notifyData, S32 resume_from )
{

    CURL* handle	= curl_easy_init();
    httpTaskInfo_t* task = new_task();
    if( !task )
        return;
        
    task->handle		= handle;
    task->notifyData	= notifyData;
    task->response		= response;
    
    curl_easy_setopt( handle, CURLOPT_URL,				url );
    curl_easy_setopt( handle, CURLOPT_HEADERFUNCTION,	Net_HTTP_ParseHeader );
    curl_easy_setopt( handle, CURLOPT_HEADERDATA,		task );
    curl_easy_setopt( handle, CURLOPT_CONNECTTIMEOUT,	5 );
    curl_easy_setopt( handle, CURLOPT_NOSIGNAL,			1 );
    curl_easy_setopt( handle, CURLOPT_HTTPHEADER,		http.headers );
    curl_easy_setopt( handle, CURLOPT_RESUME_FROM,		resume_from );
    curl_easy_setopt( handle, CURLOPT_FOLLOWLOCATION,	1 );
    
    //turn on http auth since we have the website password protected
    curl_easy_setopt( handle, CURLOPT_HTTPAUTH,			CURLAUTH_BASIC );
    curl_easy_setopt( handle, CURLOPT_USERPWD,			http_usernameandpassword->string );
    
    curl_easy_setopt( handle, CURLOPT_WRITEFUNCTION,	Net_HTTP_write_callback );
    curl_easy_setopt( handle, CURLOPT_WRITEDATA,		task );
    
    if( com_sessionid->string[0] )
    {
        curl_easy_setopt( handle, CURLOPT_COOKIE, com_sessionid->string );
    }
    
    curl_multi_add_handle( http.multi_handle, handle );
    
    //make sure CURL is ready for this handle
    //spin here so when we pump later on it's ready
    while( CURLM_CALL_MULTI_PERFORM == curl_multi_perform( http.multi_handle, &http.still_running ) );
}

/*
==================
HTTP_PostUrl
==================
*/
void HTTP_PostUrl( StringEntry url, HTTP_response response, void* notifyData, StringEntry fmt, ... )
{

    CURL* handle	= curl_easy_init();
    httpTaskInfo_t* task = new_task();
    if( !task )
        return;
        
    if( fmt )
    {
        va_list arglist;
        va_start( arglist, fmt );
        Q_vsnprintf( task->postfields, sizeof( task->postfields ), fmt, arglist );
        va_end( arglist );
    }
    else
        task->postfields[ 0 ] = '\0';
        
    task->handle		= handle;
    task->notifyData	= notifyData;
    task->response		= response;
    
    curl_easy_setopt( handle, CURLOPT_URL,				url );
    curl_easy_setopt( handle, CURLOPT_HEADERFUNCTION,	Net_HTTP_ParseHeader );
    curl_easy_setopt( handle, CURLOPT_HEADERDATA,		task );
    curl_easy_setopt( handle, CURLOPT_CONNECTTIMEOUT,	5 );
    curl_easy_setopt( handle, CURLOPT_NOSIGNAL,			1 );
    curl_easy_setopt( handle, CURLOPT_HTTPHEADER,		http.headers );
    curl_easy_setopt( handle, CURLOPT_FOLLOWLOCATION,	1 );
    
    if( fmt )
    {
        curl_easy_setopt( handle, CURLOPT_POSTFIELDS,	task->postfields );
    }
    
#if 1
    //turn on http auth since we have the website password protected
    curl_easy_setopt( handle, CURLOPT_HTTPAUTH,			CURLAUTH_BASIC );
    curl_easy_setopt( handle, CURLOPT_USERPWD,			http_usernameandpassword->string );
#endif
    
    curl_easy_setopt( handle, CURLOPT_WRITEFUNCTION,	Net_HTTP_write_callback );
    curl_easy_setopt( handle, CURLOPT_WRITEDATA,		task );
    
    if( com_sessionid && com_sessionid->string[0] )
        curl_easy_setopt( handle, CURLOPT_COOKIE, com_sessionid->string );
        
    curl_multi_add_handle( http.multi_handle, handle );
    
    //make sure CURL is ready for this handle
    //spin here so when we pump later on it's ready
    while( CURLM_CALL_MULTI_PERFORM == curl_multi_perform( http.multi_handle, &http.still_running ) );
}

#ifndef DEDICATED
extern StringEntry Con_GetText( S32 console );

void HTTP_PostBug( StringEntry fileName )
{

    cvar_t* map		= Cvar_Get( "mapname", "", 0 );
    cvar_t* bugreport	= Cvar_Get( "r_bugreport", "", 0 );
    cvar_t* challenge	= Cvar_Get( "gamename", "", 0 );
    cvar_t* username	= Cvar_Get( "gui_username", "", 0 );
    
    UTF8 comment[ 512 ];
    StringEntry condump;
    
    CURL* handle	= curl_easy_init();
    httpTaskInfo_t* task = new_task();
    if( !task )
        return;
        
    Q_strncpyz( comment, "[ ", sizeof( comment ) );
    if( username->string[0] )
        Q_strcat( comment, sizeof( comment ), va( "{%s} ", username->string ) );
        
    if( challenge->string[ 0 ] )
        Q_strcat( comment, sizeof( comment ), va( "< %s > : ", challenge->string ) );
        
    Q_strcat( comment, sizeof( comment ), ( map->string[0] ) ? map->string : "frontend" );
    Q_strcat( comment, sizeof( comment ), va( " ] %s", bugreport->string ) );
    
    condump = Con_GetText( 0 );
    
    task->handle		= handle;
    task->notifyData	= NULL;
    task->response		= NULL;
    task->firstitem		= NULL;
    task->lastitem		= NULL;
    task->headerlist	= NULL;
    
    curl_formadd( &task->firstitem, &task->lastitem,
                  CURLFORM_COPYNAME,		"project_id",
                  CURLFORM_COPYCONTENTS,	"7",
                  CURLFORM_END );
                  
    curl_formadd( &task->firstitem, &task->lastitem,
                  CURLFORM_COPYNAME,		"summary",
                  CURLFORM_COPYCONTENTS,	comment,
                  CURLFORM_END );
                  
    curl_formadd( &task->firstitem, &task->lastitem,
                  CURLFORM_COPYNAME,		"description",
                  CURLFORM_COPYCONTENTS,	bugreport->string,
                  CURLFORM_END );
                  
    curl_formadd( &task->firstitem, &task->lastitem,
                  CURLFORM_COPYNAME,		"steps_to_reproduce",
                  CURLFORM_COPYCONTENTS,	condump,
                  CURLFORM_END );
                  
    if( fileName )
    {
    
        UTF8 ospath[MAX_OSPATH];
        FS_BuildOSHomePath( ospath, sizeof( ospath ), fileName );
        curl_formadd( &task->firstitem, &task->lastitem,
                      CURLFORM_COPYNAME,		"file",
                      CURLFORM_FILE,	ospath,
                      CURLFORM_CONTENTTYPE, "image/png",
                      CURLFORM_END );
    }
    
    curl_easy_setopt( handle, CURLOPT_URL,				"http://example.com/mantis/bug_report.php" );
    curl_easy_setopt( handle, CURLOPT_HTTPHEADER,		task->headerlist );
    curl_easy_setopt( handle, CURLOPT_HTTPPOST,			task->firstitem );
    
    //turn on http auth since we have the website password protected
    curl_easy_setopt( handle, CURLOPT_HTTPAUTH,			CURLAUTH_BASIC );
    curl_easy_setopt( handle, CURLOPT_USERPWD,			"username:password" );
    curl_easy_setopt( handle, CURLOPT_FOLLOWLOCATION,	1 );
    
    
    curl_multi_add_handle( http.multi_handle, handle );
    
    //make sure CURL is ready for this handle
    //spin here so when we pump later on it's ready
    while( CURLM_CALL_MULTI_PERFORM == curl_multi_perform( http.multi_handle, &http.still_running ) );
}

void HTTP_PostErrorNotice( StringEntry type, StringEntry msg )
{
    cvar_t* map		= Cvar_Get( "mapname", "", 0 );
    cvar_t* challenge	= Cvar_Get( "gamename", "", 0 );
    cvar_t* username	= Cvar_Get( "gui_username", "", 0 );
    
    UTF8 comment[ 512 ];
    StringEntry condump;
    
    CURL* handle;
    struct curl_httppost* first, *last;
    
    if( curl_global_init( CURL_GLOBAL_ALL ) != CURLE_OK )
        return;
        
    handle = curl_easy_init();
    if( !handle )
        return;
        
    Q_strncpyz( comment, type, sizeof( comment ) );
    Q_strncpyz( comment, " [ ", sizeof( comment ) );
    if( username->string[0] )
        Q_strcat( comment, sizeof( comment ), va( "{%s} ", username->string ) );
        
    if( challenge->string[ 0 ] )
        Q_strcat( comment, sizeof( comment ), va( "< %s > : ", challenge->string ) );
        
    Q_strcat( comment, sizeof( comment ), ( map->string[0] ) ? map->string : "frontend" );
    Q_strcat( comment, sizeof( comment ), va( " ] %s", msg ) );
    
    condump = Con_GetText( 0 );
    
    first = NULL;
    last = NULL;
    
    curl_formadd( &first, &last,
                  CURLFORM_COPYNAME,		"project_id",
                  CURLFORM_COPYCONTENTS,	"7",
                  CURLFORM_END );
                  
    curl_formadd( &first, &last,
                  CURLFORM_COPYNAME,		"summary",
                  CURLFORM_COPYCONTENTS,	comment,
                  CURLFORM_END );
                  
    curl_formadd( &first, &last,
                  CURLFORM_COPYNAME,		"description",
                  CURLFORM_COPYCONTENTS,	msg,
                  CURLFORM_END );
                  
    curl_formadd( &first, &last,
                  CURLFORM_COPYNAME,		"steps_to_reproduce",
                  CURLFORM_COPYCONTENTS,	condump,
                  CURLFORM_END );
                  
    curl_easy_setopt( handle, CURLOPT_URL,				"http://example.com/mantis/bug_report.php" );
    //curl_easy_setopt( handle, CURLOPT_HTTPHEADER,		NULL );
    curl_easy_setopt( handle, CURLOPT_HTTPPOST,			first );
    
    //turn on http auth since we have the website password protected
    curl_easy_setopt( handle, CURLOPT_HTTPAUTH,			CURLAUTH_BASIC );
    curl_easy_setopt( handle, CURLOPT_USERPWD,			http_bugauth->string );
    curl_easy_setopt( handle, CURLOPT_FOLLOWLOCATION,	1 );
    
    
    curl_easy_perform( handle );
    curl_easy_cleanup( handle );
    curl_formfree( first );
}

#endif //DEDICATED

#endif // USE_HTTP
