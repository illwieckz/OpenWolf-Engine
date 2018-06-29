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
// File name:   precompiled.h
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#ifndef __PRECOMPILED_H__
#define __PRECOMPILED_H__

#include <chrono>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <setjmp.h>

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
#include <float.h>
#pragma fenv_access (on)
#else
#include <fenv.h>
#endif

#ifdef _WIN32
#include <GL/glew.h>
#include <GL/wglew.h>
#elif __LINUX__
#include <GL/glew.h>
#include <GL/glxew.h>
#elif __ANDROID__
#include <GL/glew.h>
#include <GL/eglew.h>
#else
// Mac - I Have no idea
#endif

#include <database/db_local.h>

#ifdef _WIN32
#include <SDL.h>
#else
#include <SDL/SDL.h>
#endif
#include <curl/curl.h>
#include <CL/cl.h>
#include <sys/resource.h>

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
#include <audio/snd_local.h>
#include <bgame/bg_public.h>
#include <bgame/bg_local.h>

#include <qcommon/q_shared.h>
#include <qcommon/qcommon.h>
#include <qcommon/dl_public.h>
#include <qcommon/md4.h>
#include <server/server.h>

#include <GPURenderer/r_local.h>
#include <GPURenderer/r_fbo.h>
#include <GPURenderer/r_dsa.h>
#include <GPURenderer/r_common.h>
#include <GPURenderer/r_allocator.h>

#include <client/client.h>
#include <client/keys.h>

#include <qcommon/unzip.h>
#include <qcommon/htable.h>
#include <qcommon/json.h>
#include <qcommon/md4.h>
#include <qcommon/puff.h>
#include <qcommon/surfaceflags.h>

#include <sys/sdl_icon.h>
#include <sys/sys_loadlib.h>
#include <sys/sys_local.h>

#include <physicslib/physics_local.h>
#include <physicslib/physics_public.h>

#include <GUI/gui_local.h>

#include <GPUWorker/GPUWorker.h>
#include <GPUWorker/GPUWorker_Local.h>
#include <GPUWorker/GPUWorker_CLCache.h>
#include <GPUWorker/GPUWorker_Backend.h>
#include <GPUWorker/GPUWorker_OpenCL.h>

#include <cm/cm_local.h>
#include <cm/cm_patch.h>

#include <cgame/cg_api.h>

#include <client/cl_HydraManager.h>
#ifdef _WIN32
#include <OVR.h>
#endif

#ifdef _WIN32
#include <freetype/ft2build.h>
#else
#include <freetype2/ft2build.h>
#endif
#undef getch

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

#endif //!__PRECOMPILED_H__
