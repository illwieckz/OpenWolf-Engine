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
// File name:   l_precomp.h
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#ifndef __L_PRECOMP_H__
#define __L_PRECOMP_H__

#ifndef MAX_PATH
#define MAX_PATH			MAX_QPATH
#endif

#ifndef PATH_SEPERATORSTR
#if defined(WIN32)|defined(_WIN32)|defined(__NT__)|defined(__WINDOWS__)|defined(__WINDOWS_386__)
#define PATHSEPERATOR_STR		"\\"
#else
#define PATHSEPERATOR_STR		"/"
#endif
#endif
#ifndef PATH_SEPERATORCHAR
#if defined(WIN32)|defined(_WIN32)|defined(__NT__)|defined(__WINDOWS__)|defined(__WINDOWS_386__)
#define PATHSEPERATOR_CHAR		'\\'
#else
#define PATHSEPERATOR_CHAR		'/'
#endif
#endif

#define DEFINE_FIXED			0x0001

#define BUILTIN_LINE			1
#define BUILTIN_FILE			2
#define BUILTIN_DATE			3
#define BUILTIN_TIME			4
#define BUILTIN_STDC			5

#define INDENT_IF				0x0001
#define INDENT_ELSE				0x0002
#define INDENT_ELIF				0x0004
#define INDENT_IFDEF			0x0008
#define INDENT_IFNDEF			0x0010

//macro definitions
typedef struct define_s
{
    UTF8* name;							//define name
    S32 flags;							//define flags
    S32 builtin;						// > 0 if builtin define
    S32 numparms;						//number of define parameters
    token_t* parms;						//define parameters
    token_t* tokens;					//macro tokens (possibly containing parm tokens)
    struct define_s* next;				//next defined macro in a list
    struct define_s* hashnext;			//next define in the hash chain
} define_t;

//indents
//used for conditional compilation directives:
//#if, #else, #elif, #ifdef, #ifndef
typedef struct indent_s
{
    S32 type;								//indent type
    S32 skip;								//true if skipping current indent
    script_t* script;						//script the indent was in
    struct indent_s* next;					//next indent on the indent stack
} indent_t;

//source file
typedef struct source_s
{
    UTF8 filename[1024];					//file name of the script
    UTF8 includepath[1024];					//path to include files
    punctuation_t* punctuations;			//punctuations to use
    script_t* scriptstack;					//stack with scripts of the source
    token_t* tokens;						//tokens to read first
    define_t* defines;						//list with macro definitions
    define_t** definehash;					//hash chain with defines
    indent_t* indentstack;					//stack with indents
    S32 skip;								// > 0 if skipping conditional code
    token_t token;							//last read token
} source_t;


//read a token from the source
S32 PC_ReadToken( source_t* source, token_t* token );
//expect a certain token
S32 PC_ExpectTokenString( source_t* source, UTF8* string );
//expect a certain token type
S32 PC_ExpectTokenType( source_t* source, S32 type, S32 subtype, token_t* token );
//expect a token
S32 PC_ExpectAnyToken( source_t* source, token_t* token );
//returns true when the token is available
S32 PC_CheckTokenString( source_t* source, UTF8* string );
//returns true an reads the token when a token with the given type is available
S32 PC_CheckTokenType( source_t* source, S32 type, S32 subtype, token_t* token );
//skip tokens until the given token string is read
S32 PC_SkipUntilString( source_t* source, UTF8* string );
//unread the last token read from the script
void PC_UnreadLastToken( source_t* source );
//unread the given token
void PC_UnreadToken( source_t* source, token_t* token );
//read a token only if on the same line, lines are concatenated with a slash
S32 PC_ReadLine( source_t* source, token_t* token );
//returns true if there was a white space in front of the token
S32 PC_WhiteSpaceBeforeToken( token_t* token );
//add a define to the source
S32 PC_AddDefine( source_t* source, UTF8* string );
//add a globals define that will be added to all opened sources
S32 PC_AddGlobalDefine( UTF8* string );
//remove the given global define
S32 PC_RemoveGlobalDefine( UTF8* name );
//remove all globals defines
void PC_RemoveAllGlobalDefines( void );
//add builtin defines
void PC_AddBuiltinDefines( source_t* source );
//set the source include path
void PC_SetIncludePath( source_t* source, UTF8* path );
//set the punction set
void PC_SetPunctuations( source_t* source, punctuation_t* p );
//set the base folder to load files from
void PC_SetBaseFolder( UTF8* path );
//load a source file
source_t* LoadSourceFile( StringEntry filename );
//load a source from memory
source_t* LoadSourceMemory( UTF8* ptr, S32 length, UTF8* name );
//free the given source
void FreeSource( source_t* source );
//print a source error
void SourceError( source_t* source, UTF8* str, ... );
//print a source warning
void SourceWarning( source_t* source, UTF8* str, ... );

#ifdef BSPC
// some of BSPC source does include game/q_shared.h and some does not
// we define pc_token_s pc_token_t if needed (yes, it's ugly)
#ifndef __Q_SHARED_H
#define MAX_TOKENLENGTH		1024
typedef struct pc_token_s
{
    S32 type;
    S32 subtype;
    S32 intvalue;
    F32 floatvalue;
    UTF8 string[MAX_TOKENLENGTH];
} pc_token_t;
#endif //!_Q_SHARED_H
#endif //BSPC

//
S32 PC_LoadSourceHandle( StringEntry filename );
S32 PC_FreeSourceHandle( S32 handle );
S32 PC_ReadTokenHandle( S32 handle, pc_token_t* pc_token );
S32 PC_SourceFileAndLine( S32 handle, UTF8* filename, S32* line );
void PC_CheckOpenSourceHandles( void );

#endif //!__L_PRECOMP_H__