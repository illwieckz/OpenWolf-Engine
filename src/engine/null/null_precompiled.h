////////////////////////////////////////////////////////////////////////////////////////
// Copyright(C) 2018 Dusan Jocic <dusanjocic@msn.com>
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
// File name:   null_precompiled.h
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#ifndef __NULLPRECOMPILED_H__
#define __NULLPRECOMPILED_H__

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <setjmp.h>
#include <iostream>

#ifndef _WIN32
#include <sys/ioctl.h>
#endif

#include <fcntl.h>
#include <algorithm>
#ifdef _WIN32
#include <SDL_syswm.h>
#else
#include <SDL/SDL_syswm.h>
#endif

#include <signal.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>

#ifdef _WIN32
#include <io.h>
#include <shellapi.h>
#include <timeapi.h>
#include <windows.h>
#include <direct.h>
#include <lmerr.h>
#include <lmcons.h>
#include <lmwksta.h>
#include <conio.h>
#include <wincrypt.h>
#include <shlobj.h>
#include <psapi.h>
#include <float.h>
#pragma fenv_access (on)
#else
#include <fenv.h>
#endif

#include <database/db_local.h>

#ifdef _WIN32
#include <SDL.h>
#else
#include <SDL/SDL.h>
#endif
#include <curl/curl.h>
#include <platform/windows/resource.h>

#include <botlib/botlib.h>
#include <botlib/l_script.h>
#include <botlib/l_precomp.h>
#include <botlib/l_struct.h>
#include <botlib/l_utils.h>
#include <botlib/be_interface.h>
#include <botlib/l_memory.h>
#include <botlib/l_log.h>
#include <botlib/l_libvar.h>
#include <botlib/aasfile.h>
#include <botlib/botlib.h>
#include <botlib/be_aas.h>
#include <botlib/be_aas_funcs.h>
#include <botlib/be_aas_def.h>
#include <botlib/be_interface.h>
#include <botlib/be_ea.h>
#include <botlib/be_ai_weight.h>
#include <botlib/be_ai_goal.h>
#include <botlib/be_ai_move.h>
#include <botlib/be_ai_weap.h>
#include <botlib/be_ai_chat.h>
#include <botlib/be_ai_char.h>
#include <botlib/be_ai_gen.h>
#include <botlib/l_crc.h>
#include <botlib/be_aas_route.h>
#include <botlib/be_aas_routealt.h>

#include <sgame/sg_api.h>
#include <audio/s_local.h>
#include <bgame/bg_public.h>
#include <bgame/bg_local.h>

#include <qcommon/q_shared.h>
#include <qcommon/qcommon.h>
#include <qcommon/dl_public.h>
#include <qcommon/md4.h>
#include <server/server.h>
#include <client/client.h>
#include <client/keys.h>

#include <qcommon/unzip.h>
#include <qcommon/htable.h>
#include <qcommon/json.h>
#include <qcommon/md4.h>
#include <qcommon/puff.h>
#include <qcommon/surfaceflags.h>

#include <platform/sys_icon.h>
#include <platform/sys_loadlib.h>
#include <platform/sys_local.h>

#include <physicslib/physics_local.h>
#include <physicslib/physics_public.h>

#include <GUI/gui_local.h>
#include <cm/cm_local.h>
#include <cm/cm_patch.h>

#include <cgame/cg_api.h>

// Dushan
#if defined(_WIN32) || defined(_WIN64)
#include <winsock.h>
#include <mysql.h>
#else
#include <mysql/mysql.h>
#endif

#include <OWLib/types.h>
#include <OWLib/util_list.h>
#include <OWLib/util_str.h>

#ifdef __LINUX__
#include <CL/cl_gl.h>
#endif

#include <omp.h>

// curses.h defines COLOR_*, which are already defined in q_shared.h
#undef COLOR_BLACK
#undef COLOR_RED
#undef COLOR_GREEN
#undef COLOR_YELLOW
#undef COLOR_BLUE
#undef COLOR_MAGENTA
#undef COLOR_CYAN
#undef COLOR_WHITE

#include <curses.h>

#endif //!__NULLPRECOMPILED_H__
