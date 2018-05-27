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
// File name:   be_ai_chat.h
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#ifndef __BE_AI_CHAT_H__
#define __BE_AI_CHAT_H__

#define MAX_MESSAGE_SIZE		256
#define MAX_CHATTYPE_NAME		32
#define MAX_MATCHVARIABLES		8

#define CHAT_GENDERLESS			0
#define CHAT_GENDERFEMALE		1
#define CHAT_GENDERMALE			2

#define CHAT_ALL				0
#define CHAT_TEAM				1
#define CHAT_TELL				2

//a console message
typedef struct bot_consolemessage_s
{
    S32 handle;
    F32 time;									//message time
    S32 type;									//message type
    UTF8 message[MAX_MESSAGE_SIZE];				//message
    struct bot_consolemessage_s* prev, *next;	//prev and next in list
} bot_consolemessage_t;

//match variable
typedef struct bot_matchvariable_s
{
    UTF8 offset;
    S32 length;
} bot_matchvariable_t;
//returned to AI when a match is found
typedef struct bot_match_s
{
    UTF8 string[MAX_MESSAGE_SIZE];
    S32 type;
    S32 subtype;
    bot_matchvariable_t variables[MAX_MATCHVARIABLES];
} bot_match_t;

//setup the chat AI
S32 BotSetupChatAI( void );
//shutdown the chat AI
void BotShutdownChatAI( void );
//returns the handle to a newly allocated chat state
S32 BotAllocChatState( void );
//frees the chatstate
void BotFreeChatState( S32 handle );
//adds a console message to the chat state
void BotQueueConsoleMessage( S32 chatstate, S32 type, UTF8* message );
//removes the console message from the chat state
void BotRemoveConsoleMessage( S32 chatstate, S32 handle );
//returns the next console message from the state
S32 BotNextConsoleMessage( S32 chatstate, bot_consolemessage_t* cm );
//returns the number of console messages currently stored in the state
S32 BotNumConsoleMessages( S32 chatstate );
//selects a chat message of the given type
void BotInitialChat( S32 chatstate, UTF8* type, S32 mcontext, UTF8* var0, UTF8* var1, UTF8* var2, UTF8* var3, UTF8* var4, UTF8* var5, UTF8* var6, UTF8* var7 );
//returns the number of initial chat messages of the given type
S32 BotNumInitialChats( S32 chatstate, UTF8* type );
//find and select a reply for the given message
S32 BotReplyChat( S32 chatstate, UTF8* message, S32 mcontext, S32 vcontext, UTF8* var0, UTF8* var1, UTF8* var2, UTF8* var3, UTF8* var4, UTF8* var5, UTF8* var6, UTF8* var7 );
//returns the length of the currently selected chat message
S32 BotChatLength( S32 chatstate );
//enters the selected chat message
void BotEnterChat( S32 chatstate, S32 clientto, S32 sendto );
//get the chat message ready to be output
void BotGetChatMessage( S32 chatstate, UTF8* buf, S32 size );
//checks if the first string contains the second one, returns index into first string or -1 if not found
S32 StringContains( UTF8* str1, UTF8* str2, S32 casesensitive );
//finds a match for the given string using the match templates
S32 BotFindMatch( UTF8* str, bot_match_t* match, U64 context );
//returns a variable from a match
void BotMatchVariable( bot_match_t* match, S32 variable, UTF8* buf, S32 size );
//unify all the white spaces in the string
void UnifyWhiteSpaces( UTF8* string );
//replace all the context related synonyms in the string
void BotReplaceSynonyms( UTF8* string, U64 context );
//loads a chat file for the chat state
S32 BotLoadChatFile( S32 chatstate, UTF8* chatfile, UTF8* chatname );
//store the gender of the bot in the chat state
void BotSetChatGender( S32 chatstate, S32 gender );
//store the bot name in the chat state
void BotSetChatName( S32 chatstate, UTF8* name, S32 client );

#endif //!__BE_AI_CHAT_H__