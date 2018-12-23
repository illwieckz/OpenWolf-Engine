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
// File name:   serverBot_api.h
// Version:     v1.01
// Created:     11/24/2018
// Compilers:   Visual Studio 2017, gcc 7.3.0
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#ifndef __SERVERBOT_API_H__
#define __SERVERBOT_API_H__

//
// idServerBotSystem
//
class idServerBotSystem
{
public:
    virtual S32 BotAllocateClient( S32 clientNum ) = 0;
    virtual void BotFreeClient( S32 clientNum ) = 0;
    virtual S32 BotGetSnapshotEntity( S32 client, S32 sequence ) = 0;
    virtual S32 BotGetConsoleMessage( S32 client, UTF8* buf, S32 size ) = 0;
    virtual S32 BotLibSetup( void ) = 0;
    virtual S32 BotLibShutdown( void ) = 0;
    virtual void BotFrame( S32 time ) = 0;
    virtual void BotInitCvars( void ) = 0;
    virtual void BotInitBotLib( void ) = 0;
};

extern idServerBotSystem* serverBotSystem;

#endif
