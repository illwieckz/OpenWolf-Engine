#
# Termulous CMakeLists.txt
#
# cmake required (http://www.cmake.org/)
#

cmake_minimum_required( VERSION 2.8 )

if( POLICY CMP0017 )
  cmake_policy( SET CMP0017 NEW )
endif()

include( CMakeDependentOption )
if( WIN32 )
include( cmake/precompiledHeaders.cmake )
endif( WIN32 )

# This line has to appear before 'PROJECT' in order to be able to disable incremental linking
set( MSVC_INCREMENTAL_DEFAULT ON )

set( CMAKE_BUILD_TYPE Release CACHE STRING "Build configuration (Debug, Release, RelWithDebInfo, MinSizeRel)" )

project( Termulous C CXX )

if( WIN32 )
  set( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /fp:precise" )
  set( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /LARGEADDRESSAWARE" )
  set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LARGEADDRESSAWARE" )
else()
  set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse2 -pipe -Wno-write-strings -Wno-narrowing -fpermissive -Wsizeof-array-argument -I/usr/include/freetype2" )
  set( CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g" )
  set( CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} -DNDEBUG" )
  set( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DNDEBUG -ffast-math -fomit-frame-pointer -fno-strict-aliasing" )
  set( CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -DNDEBUG -ffast-math -fomit-frame-pointer -fno-strict-aliasing -g" )
endif()

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_C_FLAGS}" )
set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${CMAKE_C_FLAGS_DEBUG}" )
set( CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} ${CMAKE_C_FLAGS_MINSIZEREL}" )
set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${CMAKE_C_FLAGS_RELEASE}" )
set( CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${CMAKE_C_FLAGS_RELWITHDEBINFO}" )
set( CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "" ) # Need this to get rid of -rdynamic linker flag set by default by cmake
set( CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" )

# Options
option( BUILD_CLIENT                   "Build Tremulous client"                                ON )
option( BUILD_SERVER                   "Build Tremulous dedicated server"                      ON )
option( BUILD_AUTOUPDATE_SERVER        "Build Tremulous AutoUpdate server"                     ON ) #Dushan - this will be changed soon
option( BUILD_GAME                     "Build game logic shared libraries"                     ON )
option( BUILD_MASTER_SERVER            "Build master server"                                   ON )
option( BUILD_OWMAP                    "Build Mapping tool"                                    ON )

# Package info
set( CPACK_PACKAGE_DESCRIPTION_SUMMARY "Termulous client" )
set( CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.txt" )
set( CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/GPL.txt" )
include( CPack )

set( MOUNT_DIR ${CMAKE_SOURCE_DIR}/src/engine )
set( LIB_DIR ${CMAKE_SOURCE_DIR}/src/libs )
set( TOOLS_DIR ${CMAKE_SOURCE_DIR}/src/tools )
set( TREMGAME_DIR ${CMAKE_SOURCE_DIR}/src/engine/gameLogics/nextTrem )

find_package( PkgConfig )

if( WIN32 )
  add_definitions( -D_CRT_SECURE_NO_WARNINGS )
endif()

set( CONSOLE "tty" )

# Determine platform
if( WIN32 )
  set( PLATFORM "win32" )
  set( CONSOLE "win32" )
elseif( UNIX )
  if( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
    add_definitions( -DMACOS_X -DMACOS_X_ACCELERATION_HACK -D USE_FILE32API -DMACOS_X_CURSOR_HACK )
  endif()
  set( PLATFORM "unix" )
endif()

# Determine arch
if( ${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64" )
  set( BUILD_ARCH "AMD64" )
else()
  set( BUILD_ARCH ${CMAKE_SYSTEM_PROCESSOR} )
endif()

######################
# Define Build Files #
######################

set( SHAREDLIST_HEADERS
  ${MOUNT_DIR}/qcommon/q_platform.h
  ${MOUNT_DIR}/qcommon/q_shared.h
)

set( SHAREDLIST_SOURCES
  ${MOUNT_DIR}/qcommon/q_shared.cpp
  ${MOUNT_DIR}/qcommon/q_math.cpp
)

set( GPURENDERER_HEADERS 
  ${MOUNT_DIR}/GPURenderer/r_allocator.h
  ${MOUNT_DIR}/GPURenderer/iqm.h
  ${MOUNT_DIR}/GPURenderer/qgl.h
  ${MOUNT_DIR}/GPURenderer/r_common.h
  ${MOUNT_DIR}/GPURenderer/r_dsa.h
  ${MOUNT_DIR}/GPURenderer/r_extramath.h
  ${MOUNT_DIR}/GPURenderer/r_extratypes.h
  ${MOUNT_DIR}/GPURenderer/r_fbo.h
  ${MOUNT_DIR}/GPURenderer/r_local.h
  ${MOUNT_DIR}/GPURenderer/r_postprocess.h
  ${MOUNT_DIR}/API/renderer_api.h
  ${MOUNT_DIR}/GPURenderer/r_types.h
)

set( GPURENDERER_SOURCES
  ${MOUNT_DIR}/GPURenderer/r_allocator.cpp
  ${MOUNT_DIR}/GPURenderer/r_animation.cpp
  ${MOUNT_DIR}/GPURenderer/r_backend.cpp
  ${MOUNT_DIR}/GPURenderer/r_bsp.cpp
  ${MOUNT_DIR}/GPURenderer/r_cmds.cpp
  ${MOUNT_DIR}/GPURenderer/r_curve.cpp
  ${MOUNT_DIR}/GPURenderer/r_dsa.cpp
  ${MOUNT_DIR}/GPURenderer/r_extensions.cpp
  ${MOUNT_DIR}/GPURenderer/r_extramath.cpp
  ${MOUNT_DIR}/GPURenderer/r_fbo.cpp
  ${MOUNT_DIR}/GPURenderer/r_pbo.cpp
  ${MOUNT_DIR}/GPURenderer/r_flares.cpp
  ${MOUNT_DIR}/GPURenderer/r_font.cpp
  ${MOUNT_DIR}/GPURenderer/r_glsl.cpp
  ${MOUNT_DIR}/GPURenderer/r_glsl_parse.cpp
  ${MOUNT_DIR}/GPURenderer/r_image.cpp
  ${MOUNT_DIR}/GPURenderer/r_image_dds.cpp
  ${MOUNT_DIR}/GPURenderer/r_image_jpg.cpp
  ${MOUNT_DIR}/GPURenderer/r_image_png.cpp
  ${MOUNT_DIR}/GPURenderer/r_image_tga.cpp
  ${MOUNT_DIR}/GPURenderer/r_init.cpp
  ${MOUNT_DIR}/GPURenderer/r_light.cpp
  ${MOUNT_DIR}/GPURenderer/r_main.cpp
  ${MOUNT_DIR}/GPURenderer/r_marks.cpp
  ${MOUNT_DIR}/GPURenderer/r_mesh.cpp
  ${MOUNT_DIR}/GPURenderer/r_model.cpp
  ${MOUNT_DIR}/GPURenderer/r_model_iqm.cpp
  ${MOUNT_DIR}/GPURenderer/r_noise.cpp
  ${MOUNT_DIR}/GPURenderer/r_postprocess.cpp
  ${MOUNT_DIR}/GPURenderer/r_scene.cpp
  ${MOUNT_DIR}/GPURenderer/r_shade.cpp
  ${MOUNT_DIR}/GPURenderer/r_shade_calc.cpp
  ${MOUNT_DIR}/GPURenderer/r_shader.cpp
  ${MOUNT_DIR}/GPURenderer/r_shadows.cpp
  ${MOUNT_DIR}/GPURenderer/r_sky.cpp
  ${MOUNT_DIR}/GPURenderer/r_surface.cpp
  ${MOUNT_DIR}/GPURenderer/r_vbo.cpp
  ${MOUNT_DIR}/GPURenderer/r_world.cpp
  ${MOUNT_DIR}/GPURenderer/r_glimp.cpp
)

set( GPUWORKERS_HEADERS
  ${MOUNT_DIR}/GPUWorker/GPUWorker.h
  ${MOUNT_DIR}/GPUWorker/GPUWorker_Backend.h
  ${MOUNT_DIR}/GPUWorker/GPUWorker_CLCache.h
  ${MOUNT_DIR}/GPUWorker/GPUWorker_Local.h
  ${MOUNT_DIR}/GPUWorker/GPUWorker_OpenCL.h
  ${MOUNT_DIR}/GPUWorker/GPUWorker_Program.h
)

set( GPUWORKERS_SOURCES
  ${MOUNT_DIR}/GPUWorker/GPUWorker.cpp
  ${MOUNT_DIR}/GPUWorker/GPUWorker_Backend.cpp
  ${MOUNT_DIR}/GPUWorker/GPUWorker_CLCache.cpp
  ${MOUNT_DIR}/GPUWorker/GPUWorker_CLManager.cpp
  ${MOUNT_DIR}/GPUWorker/GPUWorker_Error.cpp
  ${MOUNT_DIR}/GPUWorker/GPUWorker_Program.cpp
)

set( GUI_HEADERS 
  ${MOUNT_DIR}/API/bgame_api.h
  ${MOUNT_DIR}/GUI/gui_local.h
  ${MOUNT_DIR}/GUI/gui_shared.h
  ${MOUNT_DIR}/GUI/keycodes.h
  ${MOUNT_DIR}/GUI/menudef.h
)

set( GUI_SOURCES
  ${TREMGAME_DIR}/bgame/bgame_misc.cpp
  ${TREMGAME_DIR}/bgame/bgame_alloc.cpp
  ${TREMGAME_DIR}/bgame/bgame_pmove.cpp
  ${TREMGAME_DIR}/bgame/bgame_slidemove.cpp
  ${TREMGAME_DIR}/bgame/bgame_voice.cpp
  ${MOUNT_DIR}/GUI/gui_atoms.cpp
  ${MOUNT_DIR}/GUI/gui_gameinfo.cpp
  ${MOUNT_DIR}/GUI/gui_main.cpp
  ${MOUNT_DIR}/GUI/gui_shared.cpp
)

set( SERVERLIST_HEADERS
  ${MOUNT_DIR}/server/server.h
  ${MOUNT_DIR}/server/serverBot.h
  ${MOUNT_DIR}/server/serverCcmds.h
  ${MOUNT_DIR}/server/serverClient.h
  ${MOUNT_DIR}/server/serverGame.h
  ${MOUNT_DIR}/server/serverWorld.h
  ${MOUNT_DIR}/server/serverSnapshot.h
  ${MOUNT_DIR}/server/serverNetChan.h
  ${MOUNT_DIR}/server/serverInit.h
  ${MOUNT_DIR}/server/serverMain.h
  ${MOUNT_DIR}/API/sgame_api.h
  ${MOUNT_DIR}/API/serverBot_api.h
  ${MOUNT_DIR}/API/serverClient_api.h
  ${MOUNT_DIR}/API/serverGame_api.h
  ${MOUNT_DIR}/API/serverWorld_api.h
  ${MOUNT_DIR}/API/serverSnapshot_api.h
  ${MOUNT_DIR}/API/serverNetChan_api.h
  ${MOUNT_DIR}/API/serverInit_api.h
  ${MOUNT_DIR}/API/serverMain_api.h
)

set( SERVERLIST_SOURCES
  ${MOUNT_DIR}/server/serverBot.cpp
  ${MOUNT_DIR}/server/serverCcmds.cpp
  ${MOUNT_DIR}/server/serverClient.cpp
  ${MOUNT_DIR}/server/serverGame.cpp
  ${MOUNT_DIR}/server/serverWorld.cpp
  ${MOUNT_DIR}/server/serverSnapshot.cpp
  ${MOUNT_DIR}/server/serverNetChan.cpp
  ${MOUNT_DIR}/server/serverInit.cpp
  ${MOUNT_DIR}/server/serverMain.cpp
)

set( COLLISIONMODEL_HEADERS
  ${MOUNT_DIR}/cm/cm_header.h
  ${MOUNT_DIR}/cm/cm_local.h
  ${MOUNT_DIR}/cm/cm_model.h
  ${MOUNT_DIR}/cm/cm_patch.h
  ${MOUNT_DIR}/cm/cm_polylib.h
  ${MOUNT_DIR}/API/cm_api.h
  ${MOUNT_DIR}/cm/cm_traceModel.h
)

set( COLLISIONMODEL_SOURCES
  ${MOUNT_DIR}/cm/cm_load.cpp
  ${MOUNT_DIR}/cm/cm_model.cpp
  ${MOUNT_DIR}/cm/cm_patch.cpp
  ${MOUNT_DIR}/cm/cm_polylib.cpp
  ${MOUNT_DIR}/cm/cm_test.cpp
  ${MOUNT_DIR}/cm/cm_trace.cpp
  ${MOUNT_DIR}/cm/cm_trisoup.cpp
)

set( QCOMMONLIST_HEADERS
  ${MOUNT_DIR}/qcommon/dl_local.h
  ${MOUNT_DIR}/API/download_api.h
  ${MOUNT_DIR}/qcommon/htable.h
  ${MOUNT_DIR}/qcommon/ioapi.h
  ${MOUNT_DIR}/qcommon/json.h
  ${MOUNT_DIR}/qcommon/md4.h
  ${MOUNT_DIR}/qcommon/puff.h
  ${MOUNT_DIR}/qcommon/q_shared.h
  ${MOUNT_DIR}/qcommon/qcommon.h
  ${MOUNT_DIR}/qcommon/qfiles.h
  ${MOUNT_DIR}/qcommon/surfaceflags.h
  ${MOUNT_DIR}/qcommon/unzip.h
)

set( QCOMMONLIST_SOURCES
  ${MOUNT_DIR}/qcommon/cmd.cpp
  ${MOUNT_DIR}/qcommon/common.cpp
  ${MOUNT_DIR}/qcommon/htable.cpp
  ${MOUNT_DIR}/qcommon/huffman.cpp
  ${MOUNT_DIR}/qcommon/ioapi.cpp
  ${MOUNT_DIR}/qcommon/json.cpp
  ${MOUNT_DIR}/qcommon/md4.cpp
  ${MOUNT_DIR}/qcommon/md5.cpp
  ${MOUNT_DIR}/qcommon/msg.cpp
  ${MOUNT_DIR}/qcommon/net_chan.cpp
  ${MOUNT_DIR}/qcommon/net_http.cpp
  ${MOUNT_DIR}/qcommon/net_ip.cpp
  ${MOUNT_DIR}/qcommon/puff.cpp
  ${MOUNT_DIR}/qcommon/unzip.cpp
)

set( FRAMEWORKS_HEADERS
  ${MOUNT_DIR}/API/FileSystem_api.h
  ${MOUNT_DIR}/API/CVarSystem_api.h
  ${MOUNT_DIR}/framework/FileSystem.h
  ${MOUNT_DIR}/framework/CVarSystem.h
)

set( FRAMEWORKS_SOURCES
  ${MOUNT_DIR}/framework/FileSystem.cpp
  ${MOUNT_DIR}/framework/CVarSystem.cpp
)

if(USE_OPENSSL)
  find_package(OpenSSL REQUIRED)
  TARGET_INCLUDE_DIRECTORIES(${OPENSSL_INCLUDE_DIR})
  add_definitions( -DUSE_OPENSSL )
endif()

set( DATABASELIST_HEADERS 
  ${MOUNT_DIR}/database/db_local.h
  ${MOUNT_DIR}/API/database_api.h
)
set( DATABASELIST_SOURCES
  ${MOUNT_DIR}/database/db_main.cpp
)

set( CLIENTLIST_SOURCES
  ${MOUNT_DIR}/client/cin_ogm.cpp
  ${MOUNT_DIR}/client/cl_avi.cpp
  ${MOUNT_DIR}/client/cl_cgame.cpp
  ${MOUNT_DIR}/client/cl_cin.cpp
  ${MOUNT_DIR}/client/cl_console.cpp
  ${MOUNT_DIR}/client/cl_input.cpp
  ${MOUNT_DIR}/client/cl_HydraManager.cpp
  ${MOUNT_DIR}/client/cl_keys.cpp
  ${MOUNT_DIR}/client/cl_main.cpp
  ${MOUNT_DIR}/client/cl_net_chan.cpp
  ${MOUNT_DIR}/client/cl_parse.cpp
  ${MOUNT_DIR}/client/cl_scrn.cpp
  ${MOUNT_DIR}/client/cl_ui.cpp
)

set( AUDIO_HEADERS
  ${MOUNT_DIR}/audio/s_codec.h
  ${MOUNT_DIR}/audio/s_local.h
  ${MOUNT_DIR}/API/sound_api.h
)

set( AUDIO_SOURCES
  ${MOUNT_DIR}/audio/s_codec.cpp
  ${MOUNT_DIR}/audio/s_codec_wav.cpp
  ${MOUNT_DIR}/audio/s_codec_ogg.cpp
  ${MOUNT_DIR}/audio/s_dma.cpp
  ${MOUNT_DIR}/audio/s_main.cpp
  ${MOUNT_DIR}/audio/s_mem.cpp
  ${MOUNT_DIR}/audio/s_mix.cpp
  ${MOUNT_DIR}/audio/s_openal.cpp
)

set( PLATFORMLIST_SOURCES
  ${MOUNT_DIR}/platform/Windows/sys_win32.cpp
  ${MOUNT_DIR}/platform/Linux/sys_unix.cpp
  ${MOUNT_DIR}/platform/sys_main.cpp
  ${MOUNT_DIR}/console/con_log.cpp
  ${MOUNT_DIR}/console/con_win32.cpp
  ${MOUNT_DIR}/console/con_tty.cpp
  ${MOUNT_DIR}/console/con_curses.cpp
)

SET ( PLATFORM_DEDICATED_SOURCES
    ${MOUNT_DIR}/platform/sys_main.cpp
	${MOUNT_DIR}/platform/Windows/sys_win32.cpp
	${MOUNT_DIR}/console/con_win32.cpp
    ${MOUNT_DIR}/console/con_log.cpp
    ${MOUNT_DIR}/console/con_curses.cpp
    ${MOUNT_DIR}/platform/Linux/sys_unix.cpp
    ${MOUNT_DIR}/platform/sys_main.cpp
    ${MOUNT_DIR}/console/con_log.cpp
    ${MOUNT_DIR}/console/con_win32.cpp
    ${MOUNT_DIR}/console/con_tty.cpp
    ${MOUNT_DIR}/console/con_curses.cpp
)

SET ( PLATFORMLIST_HEADERS
    ${MOUNT_DIR}/platform/Windows/resource.h
	${MOUNT_DIR}/platform/sys_icon.h
	${MOUNT_DIR}/platform/sys_loadlib.h
	${MOUNT_DIR}/platform/sys_local.h
)

set( SDLLIST
  ${MOUNT_DIR}/platform/sys_input.cpp
  ${MOUNT_DIR}/platform/sys_snd.cpp
)

set( BOTLIBLIST_HEADERS
  ${MOUNT_DIR}/botlib/aasfile.h
  ${MOUNT_DIR}/botlib/be_aas.h
  ${MOUNT_DIR}/botlib/be_aas_bsp.h
  ${MOUNT_DIR}/botlib/be_aas_cluster.h
  ${MOUNT_DIR}/botlib/be_aas_debug.h
  ${MOUNT_DIR}/botlib/be_aas_def.h
  ${MOUNT_DIR}/botlib/be_aas_entity.h
  ${MOUNT_DIR}/botlib/be_aas_entity.h
  ${MOUNT_DIR}/botlib/be_aas_file.h
  ${MOUNT_DIR}/botlib/be_aas_funcs.h
  ${MOUNT_DIR}/botlib/be_aas_main.h
  ${MOUNT_DIR}/botlib/be_aas_move.h
  ${MOUNT_DIR}/botlib/be_aas_optimize.h
  ${MOUNT_DIR}/botlib/be_aas_reach.h
  ${MOUNT_DIR}/botlib/be_aas_route.h
  ${MOUNT_DIR}/botlib/be_aas_routealt.h
  ${MOUNT_DIR}/botlib/be_aas_sample.h
  ${MOUNT_DIR}/botlib/be_ai_char.h
  ${MOUNT_DIR}/botlib/be_ai_chat.h
  ${MOUNT_DIR}/botlib/be_ai_gen.h
  ${MOUNT_DIR}/botlib/be_ai_goal.h
  ${MOUNT_DIR}/botlib/be_ai_move.h
  ${MOUNT_DIR}/botlib/be_ai_weap.h
  ${MOUNT_DIR}/botlib/be_ai_weight.h
  ${MOUNT_DIR}/botlib/be_ea.h
  ${MOUNT_DIR}/botlib/be_interface.h
  ${MOUNT_DIR}/botlib/botlib.h
  ${MOUNT_DIR}/botlib/l_crc.h
  ${MOUNT_DIR}/botlib/l_libvar.h
  ${MOUNT_DIR}/botlib/l_log.h
  ${MOUNT_DIR}/botlib/l_memory.h
  ${MOUNT_DIR}/botlib/l_precomp.h
  ${MOUNT_DIR}/botlib/l_script.h
  ${MOUNT_DIR}/botlib/l_struct.h
  ${MOUNT_DIR}/botlib/l_utils.h
  ${TREMGAME_DIR}/sgame/chars.h
  ${TREMGAME_DIR}/sgame/inv.h
  ${TREMGAME_DIR}/sgame/match.h
  ${TREMGAME_DIR}/sgame/syn.h

)

set( BOTLIBLIST_SOURCES
  ${MOUNT_DIR}/botlib/be_aas_bspq3.cpp
  ${MOUNT_DIR}/botlib/be_aas_cluster.cpp
  ${MOUNT_DIR}/botlib/be_aas_debug.cpp
  ${MOUNT_DIR}/botlib/be_aas_entity.cpp
  ${MOUNT_DIR}/botlib/be_aas_file.cpp
  ${MOUNT_DIR}/botlib/be_aas_main.cpp
  ${MOUNT_DIR}/botlib/be_aas_move.cpp
  ${MOUNT_DIR}/botlib/be_aas_optimize.cpp
  ${MOUNT_DIR}/botlib/be_aas_reach.cpp
  ${MOUNT_DIR}/botlib/be_aas_route.cpp
  ${MOUNT_DIR}/botlib/be_aas_routealt.cpp
  ${MOUNT_DIR}/botlib/be_aas_sample.cpp
  ${MOUNT_DIR}/botlib/be_ai_char.cpp
  ${MOUNT_DIR}/botlib/be_ai_chat.cpp
  ${MOUNT_DIR}/botlib/be_ai_gen.cpp
  ${MOUNT_DIR}/botlib/be_ai_goal.cpp
  ${MOUNT_DIR}/botlib/be_ai_move.cpp
  ${MOUNT_DIR}/botlib/be_ai_weap.cpp
  ${MOUNT_DIR}/botlib/be_ai_weight.cpp
  ${MOUNT_DIR}/botlib/be_ea.cpp
  ${MOUNT_DIR}/botlib/be_interface.cpp
  ${MOUNT_DIR}/botlib/l_crc.cpp
  ${MOUNT_DIR}/botlib/l_libvar.cpp
  ${MOUNT_DIR}/botlib/l_log.cpp
  ${MOUNT_DIR}/botlib/l_memory.cpp
  ${MOUNT_DIR}/botlib/l_precomp.cpp
  ${MOUNT_DIR}/botlib/l_script.cpp
  ${MOUNT_DIR}/botlib/l_struct.cpp
)

set( PHYSICLIB_HEADERS
  ${MOUNT_DIR}/physicslib/physics_collision.h
  ${MOUNT_DIR}/physicslib/physics_local.h
  ${MOUNT_DIR}/API/physics_api.h
)

set( PHYSICLIB_SOURCES
  ${MOUNT_DIR}/physicslib/physics_main.cpp
  ${MOUNT_DIR}/physicslib/physics_collision.cpp
  ${MOUNT_DIR}/physicslib/physics_debug.cpp
  ${MOUNT_DIR}/physicslib/physics_tracemodel.cpp
)

set ( OWLIB_HEADERS
  ${MOUNT_DIR}/OWLib/math_angles.h
  ${MOUNT_DIR}/OWLib/math_matrix.h
  ${MOUNT_DIR}/OWLib/math_quaternion.h
  ${MOUNT_DIR}/OWLib/math_vector.h
  ${MOUNT_DIR}/OWLib/q_splineshared.h
  ${MOUNT_DIR}/OWLib/splines.h
  ${MOUNT_DIR}/OWLib/types.h
  ${MOUNT_DIR}/OWLib/util_list.h
  ${MOUNT_DIR}/OWLib/util_str.h
)

set ( OWLIB_SOURCES
  ${MOUNT_DIR}/OWLib/math_angles.cpp
  ${MOUNT_DIR}/OWLib/math_matrix.cpp
  ${MOUNT_DIR}/OWLib/math_vector.cpp
  ${MOUNT_DIR}/OWLib/q_parse.cpp
  ${MOUNT_DIR}/OWLib/q_shared.cpp
  ${MOUNT_DIR}/OWLib/splines.cpp
  ${MOUNT_DIR}/OWLib/util_str.cpp
)

set( QAGAMELIST_HEADERS
  ${TREMGAME_DIR}/sgame/sgame_precompiled.h
  ${TREMGAME_DIR}/botai/botai_chat.h
  ${TREMGAME_DIR}/botai/botai_local.h
  ${TREMGAME_DIR}/bgame/bgame_local.h
  ${MOUNT_DIR}/API/bgame_api.h
  ${TREMGAME_DIR}/sgame/sgame_admin.h
  ${TREMGAME_DIR}/sgame/sgame_local.h
  ${MOUNT_DIR}/qcommon/surfaceflags.h
  ${MOUNT_DIR}/API/sgame_api.h
  ${TREMGAME_DIR}/sgame/tremulous.h
  ${TREMGAME_DIR}/sgame/chars.h
  ${TREMGAME_DIR}/sgame/inv.h
  ${TREMGAME_DIR}/sgame/match.h
  ${TREMGAME_DIR}/sgame/syn.h
)

set( QAGAMELIST_SOURCES
  ${TREMGAME_DIR}/sgame/sgame_precompiled.cpp
  ${TREMGAME_DIR}/botai/botai_alien.cpp
  ${TREMGAME_DIR}/botai/botai_chat.cpp
  ${TREMGAME_DIR}/botai/botai_human.cpp
  ${TREMGAME_DIR}/botai/botai_main.cpp
  ${TREMGAME_DIR}/botai/botai_utils.cpp
  ${TREMGAME_DIR}/bgame/bgame_alloc.cpp
  ${TREMGAME_DIR}/bgame/bgame_misc.cpp
  ${TREMGAME_DIR}/bgame/bgame_pmove.cpp
  ${TREMGAME_DIR}/bgame/bgame_slidemove.cpp
  ${TREMGAME_DIR}/bgame/bgame_voice.cpp
  ${TREMGAME_DIR}/sgame/sgame_active.cpp
  ${TREMGAME_DIR}/sgame/sgame_admin.cpp
  ${TREMGAME_DIR}/sgame/sgame_api.cpp
  ${TREMGAME_DIR}/sgame/sgame_bot.cpp
  ${TREMGAME_DIR}/sgame/sgame_buildable.cpp
  ${TREMGAME_DIR}/sgame/sgame_client.cpp
  ${TREMGAME_DIR}/sgame/sgame_cmds.cpp
  ${TREMGAME_DIR}/sgame/sgame_combat.cpp
  ${TREMGAME_DIR}/sgame/sgame_main.cpp
  ${TREMGAME_DIR}/sgame/sgame_maprotation.cpp
  ${TREMGAME_DIR}/sgame/sgame_mem.cpp
  ${TREMGAME_DIR}/sgame/sgame_misc.cpp
  ${TREMGAME_DIR}/sgame/sgame_missile.cpp
  ${TREMGAME_DIR}/sgame/sgame_mover.cpp
  ${TREMGAME_DIR}/sgame/sgame_physics.cpp
  ${TREMGAME_DIR}/sgame/sgame_ptr.cpp
  ${TREMGAME_DIR}/sgame/sgame_session.cpp
  ${TREMGAME_DIR}/sgame/sgame_spawn.cpp
  ${TREMGAME_DIR}/sgame/sgame_svcmds.cpp
  ${TREMGAME_DIR}/sgame/sgame_target.cpp
  ${TREMGAME_DIR}/sgame/sgame_team.cpp
  ${TREMGAME_DIR}/sgame/sgame_trigger.cpp
  ${TREMGAME_DIR}/sgame/sgame_utils.cpp
  ${TREMGAME_DIR}/sgame/sgame_weapon.cpp
)

set( CGAMELIST_HEADERS
  ${MOUNT_DIR}/API/cgame_api.h
  ${MOUNT_DIR}/GUI/menudef.h
  ${MOUNT_DIR}/API/bgame_api.h
  ${MOUNT_DIR}/qcommon/surfaceflags.h
  ${MOUNT_DIR}/GUI/keycodes.h
  ${MOUNT_DIR}/GUI/gui_shared.h
  ${MOUNT_DIR}/GPURenderer/r_types.h
  ${TREMGAME_DIR}/bgame/bgame_local.h
  ${TREMGAME_DIR}/cgame/cgame_local.h
  ${TREMGAME_DIR}/cgame/cgame_precompiled.h
  ${TREMGAME_DIR}/cgame/cgame_animation.h
  ${TREMGAME_DIR}/cgame/cgame_animmapobj.h
  ${TREMGAME_DIR}/cgame/cgame_attachment.h
  ${TREMGAME_DIR}/cgame/cgame_buildable.h
  ${TREMGAME_DIR}/cgame/cgame_consolecmds.h
  ${TREMGAME_DIR}/cgame/cgame_draw.h
  ${TREMGAME_DIR}/cgame/cgame_drawtools.h
  ${TREMGAME_DIR}/cgame/cgame_ents.h
  ${TREMGAME_DIR}/cgame/cgame_event.h
  ${TREMGAME_DIR}/cgame/cgame_main.h
  ${TREMGAME_DIR}/cgame/cgame_marks.h
  ${TREMGAME_DIR}/cgame/cgame_mem.h
  ${TREMGAME_DIR}/cgame/cgame_particles.h
  ${TREMGAME_DIR}/cgame/cgame_players.h
  ${TREMGAME_DIR}/cgame/cgame_playerstate.h
  ${TREMGAME_DIR}/cgame/cgame_predict.h
  ${TREMGAME_DIR}/cgame/cgame_ptr.h
  ${TREMGAME_DIR}/cgame/cgame_scanner.h
  ${TREMGAME_DIR}/cgame/cgame_servercmds.h
  ${TREMGAME_DIR}/cgame/cgame_snapshot.h
  ${TREMGAME_DIR}/cgame/cgame_trails.h
  ${TREMGAME_DIR}/cgame/cgame_tutorial.h
  ${TREMGAME_DIR}/cgame/cgame_view.h
  ${TREMGAME_DIR}/cgame/cgame_weapons.h
)

set( CGAMELIST_SOURCES
  ${TREMGAME_DIR}/cgame/cgame_precompiled.cpp
  ${TREMGAME_DIR}/bgame/bgame_alloc.cpp
  ${TREMGAME_DIR}/bgame/bgame_misc.cpp
  ${TREMGAME_DIR}/bgame/bgame_pmove.cpp
  ${TREMGAME_DIR}/bgame/bgame_voice.cpp
  ${TREMGAME_DIR}/bgame/bgame_slidemove.cpp
  ${TREMGAME_DIR}/cgame/cgame_animation.cpp
  ${TREMGAME_DIR}/cgame/cgame_animmapobj.cpp
  ${TREMGAME_DIR}/cgame/cgame_api.cpp
  ${TREMGAME_DIR}/cgame/cgame_attachment.cpp
  ${TREMGAME_DIR}/cgame/cgame_buildable.cpp
  ${TREMGAME_DIR}/cgame/cgame_consolecmds.cpp
  ${TREMGAME_DIR}/cgame/cgame_draw.cpp
  ${TREMGAME_DIR}/cgame/cgame_drawtools.cpp
  ${TREMGAME_DIR}/cgame/cgame_ents.cpp
  ${TREMGAME_DIR}/cgame/cgame_event.cpp
  ${TREMGAME_DIR}/cgame/cgame_main.cpp
  ${TREMGAME_DIR}/cgame/cgame_marks.cpp
  ${TREMGAME_DIR}/cgame/cgame_mem.cpp
  ${TREMGAME_DIR}/cgame/cgame_particles.cpp
  ${TREMGAME_DIR}/cgame/cgame_players.cpp
  ${TREMGAME_DIR}/cgame/cgame_playerstate.cpp
  ${TREMGAME_DIR}/cgame/cgame_predict.cpp
  ${TREMGAME_DIR}/cgame/cgame_ptr.cpp
  ${TREMGAME_DIR}/cgame/cgame_scanner.cpp
  ${TREMGAME_DIR}/cgame/cgame_servercmds.cpp
  ${TREMGAME_DIR}/cgame/cgame_snapshot.cpp
  ${TREMGAME_DIR}/cgame/cgame_trails.cpp
  ${TREMGAME_DIR}/cgame/cgame_tutorial.cpp
  ${TREMGAME_DIR}/cgame/cgame_view.cpp
  ${TREMGAME_DIR}/cgame/cgame_weapons.cpp
  ${MOUNT_DIR}/GUI/gui_shared.cpp
)

set( NULL_SOURCES 
  ${MOUNT_DIR}/null/null_client.cpp
  ${MOUNT_DIR}/null/null_input.cpp
  ${MOUNT_DIR}/null/null_precompiled.cpp
)

set( NULL_HEADERS
  ${MOUNT_DIR}/null/null_precompiled.h
)

set( OWMAPLIST_HEADERS
  ${TOOLS_DIR}/owmap/bspfile.h
  ${TOOLS_DIR}/owmap/bytebool.h
  ${TOOLS_DIR}/owmap/cmdlib.h
  ${TOOLS_DIR}/owmap/ddslib.h
  ${TOOLS_DIR}/owmap/bspfile_abstract.h
  ${TOOLS_DIR}/owmap/etclib.h
  ${TOOLS_DIR}/owmap/filematch.h
  ${TOOLS_DIR}/owmap/game__null.h
  ${TOOLS_DIR}/owmap/game_tremulous.h
  ${TOOLS_DIR}/owmap/globaldefs.h
  ${TOOLS_DIR}/owmap/imagelib.h
  ${TOOLS_DIR}/owmap/inout.h
  ${TOOLS_DIR}/owmap/mathlib.h
  ${TOOLS_DIR}/owmap/md4.h
  ${TOOLS_DIR}/owmap/md5lib.h
  ${TOOLS_DIR}/owmap/polylib.h
  ${TOOLS_DIR}/owmap/polyset.h
  ${TOOLS_DIR}/owmap/q3map2.h
  ${TOOLS_DIR}/owmap/qfiles.h
  ${TOOLS_DIR}/owmap/qthreads.h
  ${TOOLS_DIR}/owmap/scriplib.h
  ${TOOLS_DIR}/owmap/surfaceflags.h
  ${TOOLS_DIR}/owmap/trilib.h
  ${TOOLS_DIR}/owmap/assets_loader.hpp
  ${TOOLS_DIR}/owmap/stb_image_write.h
  ${TOOLS_DIR}/owmap/libs/assetsys/assetsys.h
  ${TOOLS_DIR}/owmap/libs/assetsys/strpool.h
  ${TOOLS_DIR}/owmap/libs/catch2/catch.hpp
  ${TOOLS_DIR}/owmap/libs/filesystem/fwd.h
  ${TOOLS_DIR}/owmap/libs/filesystem/path.h
  ${TOOLS_DIR}/owmap/libs/filesystem/resolver.h
  ${TOOLS_DIR}/owmap/libs/jpeg-compressor/jpgd.h
  ${TOOLS_DIR}/owmap/libs/jpeg-compressor/jpge.h
  ${TOOLS_DIR}/owmap/libs/lodepng/lodepng.h
  ${TOOLS_DIR}/owmap/libs/lodepng/lodepng_util.h
  ${TOOLS_DIR}/owmap/libs/msdirent/msdirent.h
  ${TOOLS_DIR}/owmap/libs/picomodel/src/picointernal.h
  ${TOOLS_DIR}/owmap/libs/picomodel/src/picomodel.h
  ${TOOLS_DIR}/owmap/libs/picomodel/src/platfdefs.h
  ${TOOLS_DIR}/owmap/libs/picomodel/src/pm_fm.h
  ${TOOLS_DIR}/owmap/libs/picomodel/src/lwo/lwo2.h
  ${TOOLS_DIR}/owmap/libs/table-builder/table_builder.hpp
  ${TOOLS_DIR}/owmap/libs/tinyformat/tinyformat.h
)

set( OWMAPLIST_SOURCES
  ${TOOLS_DIR}/owmap/assets_loader.cpp
  ${TOOLS_DIR}/owmap/bbox.cpp
  ${TOOLS_DIR}/owmap/brush.cpp
  ${TOOLS_DIR}/owmap/brush_primit.cpp
  ${TOOLS_DIR}/owmap/bsp.cpp
  ${TOOLS_DIR}/owmap/bsp_analyze.cpp
  ${TOOLS_DIR}/owmap/bsp_info.cpp
  ${TOOLS_DIR}/owmap/bsp_scale.cpp
  ${TOOLS_DIR}/owmap/bspfile_abstract.cpp
  ${TOOLS_DIR}/owmap/bspfile_obsp.cpp
  ${TOOLS_DIR}/owmap/cmdlib.cpp
  ${TOOLS_DIR}/owmap/cmfile.cpp
  ${TOOLS_DIR}/owmap/convert_ase.cpp
  ${TOOLS_DIR}/owmap/convert_bsp.cpp
  ${TOOLS_DIR}/owmap/convert_map.cpp
  ${TOOLS_DIR}/owmap/convert_obj.cpp
  ${TOOLS_DIR}/owmap/ddslib.cpp
  ${TOOLS_DIR}/owmap/decals.cpp
  ${TOOLS_DIR}/owmap/etclib.cpp
  ${TOOLS_DIR}/owmap/exportents.cpp
  ${TOOLS_DIR}/owmap/facebsp.cpp
  ${TOOLS_DIR}/owmap/filematch.cpp
  ${TOOLS_DIR}/owmap/fixaas.cpp
  ${TOOLS_DIR}/owmap/fog.cpp
  ${TOOLS_DIR}/owmap/help.cpp
  ${TOOLS_DIR}/owmap/image.cpp
  ${TOOLS_DIR}/owmap/imagelib.cpp
  ${TOOLS_DIR}/owmap/inout.cpp
  ${TOOLS_DIR}/owmap/leakfile.cpp
  ${TOOLS_DIR}/owmap/light.cpp
  ${TOOLS_DIR}/owmap/light_bounce.cpp
  ${TOOLS_DIR}/owmap/light_trace.cpp
  ${TOOLS_DIR}/owmap/light_ydnar.cpp
  ${TOOLS_DIR}/owmap/lightmaps_ydnar.cpp
  ${TOOLS_DIR}/owmap/line.cpp
  ${TOOLS_DIR}/owmap/m4x4.cpp
  ${TOOLS_DIR}/owmap/main.cpp
  ${TOOLS_DIR}/owmap/map.cpp
  ${TOOLS_DIR}/owmap/mathlib.cpp
  ${TOOLS_DIR}/owmap/md4.cpp
  ${TOOLS_DIR}/owmap/md5lib.cpp
  ${TOOLS_DIR}/owmap/mesh.cpp
  ${TOOLS_DIR}/owmap/minimap.cpp
  ${TOOLS_DIR}/owmap/model.cpp
  ${TOOLS_DIR}/owmap/patch.cpp
  ${TOOLS_DIR}/owmap/path_init.cpp
  ${TOOLS_DIR}/owmap/polylib.cpp
  ${TOOLS_DIR}/owmap/portals.cpp
  ${TOOLS_DIR}/owmap/prtfile.cpp
  ${TOOLS_DIR}/owmap/ray.cpp
  ${TOOLS_DIR}/owmap/scriplib.cpp
  ${TOOLS_DIR}/owmap/shaders.cpp
  ${TOOLS_DIR}/owmap/surface.cpp
  ${TOOLS_DIR}/owmap/surface_extra.cpp
  ${TOOLS_DIR}/owmap/surface_foliage.cpp
  ${TOOLS_DIR}/owmap/surface_fur.cpp
  ${TOOLS_DIR}/owmap/surface_meta.cpp
  ${TOOLS_DIR}/owmap/threads.cpp
  ${TOOLS_DIR}/owmap/tjunction.cpp
  ${TOOLS_DIR}/owmap/tree.cpp
  ${TOOLS_DIR}/owmap/trilib.cpp
  ${TOOLS_DIR}/owmap/vis.cpp
  ${TOOLS_DIR}/owmap/visflow.cpp
  ${TOOLS_DIR}/owmap/writebsp.cpp
  ${TOOLS_DIR}/owmap/libs/jpeg-compressor/jpgd.cpp
  ${TOOLS_DIR}/owmap/libs/jpeg-compressor/jpge.cpp
  ${TOOLS_DIR}/owmap/libs/lodepng/lodepng.cpp
  ${TOOLS_DIR}/owmap/libs/lodepng/lodepng_util.cpp
  ${TOOLS_DIR}/owmap/libs/picomodel/src/picointernal.c
  ${TOOLS_DIR}/owmap/libs/picomodel/src/picomodel.c
  ${TOOLS_DIR}/owmap/libs/picomodel/src/picomodules.c
  ${TOOLS_DIR}/owmap/libs/picomodel/src/pm_3ds.c
  ${TOOLS_DIR}/owmap/libs/picomodel/src/pm_ase.c
  ${TOOLS_DIR}/owmap/libs/picomodel/src/pm_fm.c
  ${TOOLS_DIR}/owmap/libs/picomodel/src/pm_lwo.c
  ${TOOLS_DIR}/owmap/libs/picomodel/src/pm_md2.c
  ${TOOLS_DIR}/owmap/libs/picomodel/src/pm_md3.c
  ${TOOLS_DIR}/owmap/libs/picomodel/src/pm_mdc.c
  ${TOOLS_DIR}/owmap/libs/picomodel/src/pm_mdl.c
  ${TOOLS_DIR}/owmap/libs/picomodel/src/pm_ms3d.c
  ${TOOLS_DIR}/owmap/libs/picomodel/src/pm_obj.c
  ${TOOLS_DIR}/owmap/libs/picomodel/src/pm_terrain.c
  ${TOOLS_DIR}/owmap/libs/picomodel/src/lwo/clip.c
  ${TOOLS_DIR}/owmap/libs/picomodel/src/lwo/envelope.c
  ${TOOLS_DIR}/owmap/libs/picomodel/src/lwo/list.c
  ${TOOLS_DIR}/owmap/libs/picomodel/src/lwo/lwio.c
  ${TOOLS_DIR}/owmap/libs/picomodel/src/lwo/lwo2.c
  ${TOOLS_DIR}/owmap/libs/picomodel/src/lwo/lwob.c
  ${TOOLS_DIR}/owmap/libs/picomodel/src/lwo/pntspols.c
  ${TOOLS_DIR}/owmap/libs/picomodel/src/lwo/surface.c
  ${TOOLS_DIR}/owmap/libs/picomodel/src/lwo/vecmath.c
  ${TOOLS_DIR}/owmap/libs/picomodel/src/lwo/vmap.c
  ${TOOLS_DIR}/owmap/libs/table-builder/table_builder.cpp
)

set( OWMASTERSERVERLIST_HEADERS
  ${TOOLS_DIR}/master/common.h
  ${TOOLS_DIR}/master/messages.h
  ${TOOLS_DIR}/master/servers.h
)

set( OWMASTERSERVERLIST_SOURCES
  ${TOOLS_DIR}/master/master.cpp
  ${TOOLS_DIR}/master/messages.cpp
  ${TOOLS_DIR}/master/servers.cpp
  ${TOOLS_DIR}/master/stats.cpp
)
####################
# End of file list #
####################

if( WIN32 )
  add_definitions( -DWINVER=0x501 )
  #set( OS_LIBRARIES m winmm wsock32 gdi32 ole32 ws2_32 psapi Iphlpapi advapi32 shell32 user32 )
  set( OS_LIBRARIES winmm ws2_32 psapi )
  set( EXTRALIBS dinput dsound )
elseif( UNIX )
  if( CMAKE_SYSTEM_NAME STREQUAL "FreeBSD" )
    set( OS_LIBRARIES m z )
  elseif( CMAKE_SYSTEM_NAME STREQUAL "Darwin" )
    set( OS_LIBRARIES dl m z )
    # FIXME This looks wrong.
    set( OS_LIBRARIES ${OS_LIBRARIES} "-framework AGL -framework OpenGL -framework Carbon -framework IOKit" )
  else()
    set( OS_LIBRARIES ${CMAKE_DL_LIBS} m z rt )
  endif()
endif()

###############################
# Build Game/Cgame/Ui DLL/.so #
###############################

if( BUILD_GAME )
  add_library( sgame SHARED ${SHAREDLIST_HEADERS} ${SHAREDLIST_SOURCES} ${QAGAMELIST_HEADERS} ${QAGAMELIST_SOURCES} )
  if( WIN32 )
    SOURCE_GROUP( Header\ Files\\SharedList                               FILES ${SHAREDLIST_HEADERS} )
	SOURCE_GROUP( Header\ Files\\GameFiles                                FILES ${QAGAMELIST_HEADERS} )
    SOURCE_GROUP( Source\ Files\\SharedList                               FILES ${SHAREDLIST_SOURCES} )
	SOURCE_GROUP( Source\ Files\\GameFiles                                FILES ${QAGAMELIST_SOURCES} )
	
	target_link_libraries( sgame winmm.lib )
	
	ADD_PRECOMPILED_HEADER( sgame "${TREMGAME_DIR}/sgame/sgame_precompiled.h" "${TREMGAME_DIR}/sgame/sgame_precompiled.cpp" )
	SET_TARGET_PROPERTIES( sgame PROPERTIES LINK_FLAGS "/DEF:\"${TREMGAME_DIR}/sgame/sgame.def" )
  endif( WIN32 )

  TARGET_INCLUDE_DIRECTORIES( sgame PRIVATE ${MOUNT_DIR} ${TREMGAME_DIR} )
if(WIN32)
  set_property( TARGET sgame PROPERTY COMPILE_DEFINITIONS _AMD64_ WIN32 _AMD64 _WIN64 __WIN64__ GAMEDLL )
elseif( UNIX )
  set_property( TARGET sgame PROPERTY COMPILE_DEFINITIONS LINUX __amd64__ __linux__ __x86_64__ __LINUX__ GAMEDLL )
endif()
  set_target_properties( sgame PROPERTIES
    OUTPUT_NAME "sgame${BUILD_ARCH}"
    PREFIX ""
    LIBRARY_OUTPUT_DIRECTORY "main"
  )
  target_link_libraries( sgame )

  add_library( cgame SHARED ${SHAREDLIST_HEADERS} ${SHAREDLIST_SOURCES} ${CGAMELIST_HEADERS} ${CGAMELIST_SOURCES} )
  if( WIN32 )
    SOURCE_GROUP( Header\ Files\\SharedList                               FILES ${SHAREDLIST_HEADERS} )
	SOURCE_GROUP( Header\ Files\\GameFiles                                FILES ${CGAMELIST_HEADERS} )
    SOURCE_GROUP( Source\ Files\\SharedList                               FILES ${SHAREDLIST_SOURCES} )
	SOURCE_GROUP( Source\ Files\\GameFiles                                FILES ${CGAMELIST_SOURCES} )
	
    ADD_PRECOMPILED_HEADER( cgame "${TREMGAME_DIR}/cgame/cgame_precompiled.h" "${TREMGAME_DIR}/cgame/cgame_precompiled.cpp" )	
	SET_TARGET_PROPERTIES( cgame PROPERTIES LINK_FLAGS "/DEF:\"${TREMGAME_DIR}/cgame/cgame.def" )
  endif( WIN32 )
  
  TARGET_INCLUDE_DIRECTORIES( cgame PRIVATE ${MOUNT_DIR} ${TREMGAME_DIR} )
if(WIN32)
  set_property( TARGET cgame PROPERTY COMPILE_DEFINITIONS _AMD64_ NDEBUG _WIN32 WIN32 _AMD64 _WIN64 __WIN64__ CGAMEDLL )
elseif(UNIX)
  set_property( TARGET cgame PROPERTY COMPILE_DEFINITIONS LINUX __amd64__ __linux__ __x86_64__ __LINUX__ CGAMEDLL )
endif()
 
  set_target_properties( cgame PROPERTIES
    OUTPUT_NAME "cgame${BUILD_ARCH}"
    PREFIX ""
    LIBRARY_OUTPUT_DIRECTORY "main"
  )
  target_link_libraries( cgame )
  install( TARGETS sgame cgame
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
  )  
endif()

################
# Build Client #
################

if( BUILD_CLIENT )
  # Setup target client
  
  # Dushan - This is how visual studio directories (or filters) are generated
  if( WIN32 )
    SOURCE_GROUP( Header\ Files\\SharedList                               FILES ${SHAREDLIST_HEADERS} )
    SOURCE_GROUP( Header\ Files\\GPURenderer                              FILES ${GPURENDERER_HEADERS} )
    SOURCE_GROUP( Header\ Files\\GPUWorkers                               FILES ${GPUWORKERS_HEADERS} )
    SOURCE_GROUP( Header\ Files\\Server                                   FILES ${SERVERLIST_HEADERS} )
    SOURCE_GROUP( Header\ Files\\CM                                       FILES ${COLLISIONMODEL_HEADERS} )
    SOURCE_GROUP( Header\ Files\\QCommon                                  FILES ${QCOMMONLIST_HEADERS} )
	SOURCE_GROUP( Header\ Files\\Framework                                FILES ${FRAMEWORKS_HEADERS} )
    SOURCE_GROUP( Header\ Files\\Client                                   FILES ${CLIENTLIST_HEADERS} )
    SOURCE_GROUP( Header\ Files\\Audio                                    FILES ${AUDIO_HEADERS} )
    SOURCE_GROUP( Header\ Files\\Platform                                 FILES ${PLATFORMLIST_HEADERS} )
    SOURCE_GROUP( Header\ Files\\Botlib                                   FILES ${BOTLIBLIST_HEADERS} )
    SOURCE_GROUP( Header\ Files\\Physics                                  FILES ${PHYSICLIB_HEADERS} )
	SOURCE_GROUP( Header\ Files\\GUI                                      FILES ${GUI_HEADERS} )
    SOURCE_GROUP( Header\ Files\\OWLib                                    FILES ${OWLIB_HEADERS} )
    SOURCE_GROUP( Header\ Files\\Database                                 FILES ${DATABASELIST_HEADERS} )
    SOURCE_GROUP( Source\ Files\\SharedList                               FILES ${SHAREDLIST_SOURCES} )
    SOURCE_GROUP( Source\ Files\\GPURenderer                              FILES ${GPURENDERER_SOURCES} )
    SOURCE_GROUP( Source\ Files\\GPUWorkers                               FILES ${GPUWORKERS_SOURCES} )
    SOURCE_GROUP( Source\ Files\\Server                                   FILES ${SERVERLIST_SOURCES} )
    SOURCE_GROUP( Source\ Files\\CM                                       FILES ${COLLISIONMODEL_SOURCES} )
    SOURCE_GROUP( Source\ Files\\QCommon                                  FILES ${QCOMMONLIST_SOURCES} )
	SOURCE_GROUP( Source\ Files\\Framework                                FILES ${FRAMEWORKS_SOURCES} )
    SOURCE_GROUP( Source\ Files\\Client                                   FILES ${CLIENTLIST_SOURCES} )
    SOURCE_GROUP( Source\ Files\\Audio                                    FILES ${AUDIO_SOURCES} )
    SOURCE_GROUP( Source\ Files\\Platform                                 FILES ${PLATFORMLIST_SOURCES} )
    SOURCE_GROUP( Source\ Files\\SDL                                      FILES ${SDLLIST} )
    SOURCE_GROUP( Source\ Files\\Botlib                                   FILES ${BOTLIBLIST_SOURCES} )
    SOURCE_GROUP( Source\ Files\\Physics                                  FILES ${PHYSICLIB_SOURCES} )
	SOURCE_GROUP( Source\ Files\\GUI                                      FILES ${GUI_SOURCES} )
	SOURCE_GROUP( Source\ Files\\OWLib                                    FILES ${OWLIB_SOURCES} )
    SOURCE_GROUP( Source\ Files\\Database                                 FILES ${DATABASELIST_SOURCES} )
  endif( WIN32 )
  
  add_executable( client ${SHAREDLIST_HEADERS} ${GPURENDERER_HEADERS} ${FRAMEWORKS_HEADERS} ${QCOMMONLIST_HEADERS} ${GPUWORKERS_HEADERS} ${SERVERLIST_HEADERS} ${COLLISIONMODEL_HEADERS} ${CLIENTLIST_HEADERS} ${AUDIO_HEADERS} ${BOTLIBLIST_HEADERS} ${PHYSICLIB_HEADERS} ${SHAREDLIST_SOURCES} ${GPURENDERER_SOURCES} ${GPUWORKERS_SOURCES} ${SERVERLIST_SOURCES} ${COLLISIONMODEL_SOURCES} ${QCOMMONLIST_SOURCES} ${CLIENTLIST_SOURCES} ${AUDIO_SOURCES} ${FRAMEWORKS_SOURCES} ${PLATFORMLIST} ${SDLLIST} ${BOTLIBLIST_SOURCES} ${PHYSICLIB_SOURCES} ${DATABASELIST_SOURCES} ${DATABASELIST_HEADERS} ${BOTLIBLIST} ${MOUNT_DIR}/qcommon/dl_main_curl.cpp ${GUI_HEADERS} ${GUI_SOURCES} ${CRYPTOLIST_SOURCES} ${OWLIB_HEADERS} ${OWLIB_SOURCES} ${PLATFORMLIST_SOURCES} ${PLATFORMLIST_HEADERS} ${JPEG_INCLUDE_DIR} ${JPEG_SOURCES} ${MOUNT_DIR}/OWLib/precompiled.h ${MOUNT_DIR}/OWLib/precompiled.cpp )
	    
if(WIN32)
  set_property( TARGET client APPEND PROPERTY COMPILE_DEFINITIONS _AMD64_ WIN32 _AMD64 _WIN64 __WIN64__ NDEBUG _WINDOWS BOTLIB BOTLIB_STATIC USE_CRYPTO HAVE_CONFIG_H USE_INCREASED_ENTITIES  __STDC_CONSTANT_MACROS _PHYSICSLIB BUILD_FREETYPE HAVE_BOOLEAN HAVE_BZIP2 )
  ADD_PRECOMPILED_HEADER( client "${MOUNT_DIR}/OWLib/precompiled.h" "${MOUNT_DIR}/OWLib/precompiled.cpp" )
elseif( UNIX )
  set_property( TARGET client APPEND PROPERTY COMPILE_DEFINITIONS NDEBUG __LINUX__ __linux__ __amd64__ __x86_64__ LINUX BOTLIB BOTLIB_STATIC USE_CRYPTO HAVE_CONFIG_H USE_INCREASED_ENTITIES  __STDC_CONSTANT_MACROS _PHYSICSLIB BUILD_FREETYPE HAVE_BZIP2 )
endif()
  
  set_target_properties( client PROPERTIES OUTPUT_NAME "Tremulous.${BUILD_ARCH}" PREFIX "" LINKER_LANGUAGE CXX )

  install( TARGETS client
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
  )

  # This section finds and provides REQUIRED dependencies to target client
  find_package( OpenGL REQUIRED )
  find_package( JPEG REQUIRED )
  find_package( SDL REQUIRED )
  find_package( Freetype REQUIRED )
  find_package( CURL REQUIRED )
  find_package( Bullet REQUIRED )
  find_package( Zlib REQUIRED )
  find_package( PNG REQUIRED )
  find_package( BZ2 REQUIRED )
  find_package( OpenCL REQUIRED )
  find_package( OpenAL REQUIRED )
  find_package( OggVorbis REQUIRED )
if( WIN32 )
  #Dushan - only for the Windows platform now
  find_package( Sixense REQUIRED )
  find_package( OculusSDK REQUIRED )
  find_package( OculusKernelSDK REQUIRED )
endif( WIN32 )

  TARGET_INCLUDE_DIRECTORIES( client PRIVATE ${OPENGL_INCLUDE_DIR} ${FREETYPE_INCLUDE_DIRS} ${SDL2_INCLUDE_DIR} ${ZLIB_INCLUDE_DIRS} ${CURL_INCLUDE_DIRS} ${BULLET_INCLUDE_DIR} ${BULLET_INCLUDE_COLLISION_DIR} ${BULLET_INCLUDE_DYNAMICS_DIR} ${BULLET_INCLUDE_SOFTBODY_DIR} ${BULLET_INCLUDE_LINEARMATH_DIR} ${BULLET_INCLUDE_FILELOADER_DIR} ${BULLET_INCLUDE_WORLDIMPORTER_DIR} ${PNG_INCLUDE_DIR} ${OpenCL_INCLUDE_DIRS} ${PDCURSES_INCLUDE_DIR} ${MOUNT_DIR} ${JPEG_INCLUDE_DIR} ${OVR_INCLUDE_DIRS} ${OVRKERNEL_INCLUDE_DIR} ${SIXENSE_INCLUDE_DIRS} ${BZ2_INCLUDE_DIR} ${OPENAL_INCLUDE_DIR} ${OGG_INCLUDE_DIR} ${VORBIS_INCLUDE_DIR} ${TREMGAME_DIR} ${TOOLS_DIR}/owmap )

  TARGET_LINK_LIBRARIES( client ${OPENGL_LIBRARIES} ${SDL2_LIBRARY} ${SDL2MAIN_LIBRARY} ${FREETYPE_LIBRARIES} ${ZLIB_LIBRARIES} ${CURL_LIBRARIES} ${PNG_LIBRARIES} ${OpenCL_LIBRARY} ${PDCURSES_LIBRARY} ${JPEG_LIBRARIES} ${LINK_LIBRARY} ${SIXENSE_LIBRARIES} ${SIXENSE_LIBRARIES_UTILS} ${OVR_LIBRARIES} ${OVRKERNEL_LIBRARY_RELEASE} ${BULLET_LIBRARIES} ${BZ2_LIBRARY} ${OPENAL_LIBRARY} ${VORBIS_LIBRARY} ${VORBISFILE_LIBRARY} )

  if( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
    link_directories( /opt/local/lib )
  endif()

 if( ${CMAKE_SYSTEM_NAME} MATCHES "UNIX" )
    link_directories( ${LIB_DIR}/bullet-2.80-rev2531/libs/Linux64/ )
  endif() 
  
set( CURSES_NEED_NCURSES 1 ) # Tells FindCurses that ncurses is required
# Determine platform
if( WIN32 )
	find_package( PDCurses REQUIRED )
elseif( UNIX )
	find_package( Curses REQUIRED )
endif()
set_property( TARGET client APPEND PROPERTY COMPILE_DEFINITIONS USE_CURSES )
TARGET_INCLUDE_DIRECTORIES( client PRIVATE ${CURSES_INCLUDE_DIR} )
target_link_libraries( client ${CURSES_LIBRARIES} )
  
if ( WIN32)
  target_link_libraries( client ${OS_LIBRARIES} )
else ( UNIX )
  target_link_libraries( client ${OS_LIBRARIES} ${BULLET_COLLISION_LIBRARY} ${BULLET_DYNAMICS_LIBRARY} ${BULLET_FILELOADER_LIBRARY} ${BULLET_SOFTBODY_LIBRARY} ${BULLET_WORLDIMPORTER_LIBRARY} ${BULLET_CONVEXDECOMPOSITION_LIBRARY} ${BULLET_LINEARMATH_LIBRARY} -fopenmp)
endif() 

  find_package( MySQL REQUIRED )
  set_property( TARGET client APPEND PROPERTY COMPILE_DEFINITIONS ET_MYSQL )
  TARGET_INCLUDE_DIRECTORIES( client PRIVATE ${MYSQL_INCLUDE_DIR} )
  target_link_libraries( client ${MYSQL_LIBRARIES} )

endif()

################
# Build Server #
################

if( BUILD_SERVER )
  # Setup the target Server
  add_executable( server ${SHAREDLIST_HEADERS} ${CRYPTOLIST_HEADERS} ${FRAMEWORKS_HEADERS} ${SERVERLIST_HEADERS} ${COLLISIONMODEL_HEADERS} ${CLIENTLIST_HEADERS} ${CRYPTOLIST_SOURCES}  ${BOTLIBLIST_HEADERS} ${PHYSICLIB_HEADERS} ${SHAREDLIST_SOURCES} ${SERVERLIST_SOURCES} ${COLLISIONMODEL_SOURCES}  ${QCOMMONLIST_SOURCES} ${FRAMEWORKS_SOURCES} ${BOTLIBLIST_SOURCES} ${PHYSICLIB_SOURCES} ${PLATFORMLIST_HEADERS} ${OWLIB_HEADERS} ${OWLIB_SOURCES} ${PLATFORM_DEDICATED_SOURCES} ${DATABASELIST_SOURCES} ${DATABASELIST_HEADERS} ${NULL_SOURCES} ${NULL_HEADERS} ${MOUNT_DIR}/qcommon/dl_main_stubs.cpp ${MOUNT_DIR}/null/null_precompiled.h ${MOUNT_DIR}/null/null_precompiled.cpp )
	
  target_link_libraries( server ${OS_LIBRARIES} )
if(WIN32)
  set_property( TARGET server APPEND PROPERTY COMPILE_DEFINITIONS _AMD64_ WIN32 _AMD64 _WIN64 __WIN64__ NDEBUG DEDICATED _WINDOWS BOTLIB BOTLIB_STATIC USE_CRYPTO HAVE_CONFIG_H USE_INCREASED_ENTITIES  __STDC_CONSTANT_MACROS _PHYSICSLIB HAVE_BOOLEAN HAVE_BZIP2 )
  ADD_PRECOMPILED_HEADER( client "${MOUNT_DIR}/OWLib/precompiled.h" "${MOUNT_DIR}/OWLib/precompiled.cpp" )
elseif( UNIX )
  set_property( TARGET server APPEND PROPERTY COMPILE_DEFINITIONS NDEBUG __LINUX__ __linux__ __amd64__ __x86_64__ LINUX DEDICATED BOTLIB BOTLIB_STATIC USE_CRYPTO HAVE_CONFIG_H USE_INCREASED_ENTITIES  __STDC_CONSTANT_MACROS _PHYSICSLIB BUILD_FREETYPE HAVE_BZIP2 )
endif()
  
  set_target_properties( server PROPERTIES OUTPUT_NAME "Termulousded.${BUILD_ARCH}" PREFIX "" LINKER_LANGUAGE CXX )
  install( TARGETS server
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
  )
  # The following section finds the REQUIRED dependencies and makes them available to the target
    find_package( SDL REQUIRED )
    find_package( Bullet REQUIRED )
    find_package( CURL REQUIRED )
    find_package( Zlib REQUIRED )
    find_package( BZ2 REQUIRED )

    TARGET_INCLUDE_DIRECTORIES( server PRIVATE ${TOOLS_DIR}/owmap ${SDL2_INCLUDE_DIR} ${ZLIB_INCLUDE_DIRS} ${CURL_INCLUDE_DIRS} ${BULLET_INCLUDE_DIR} ${BULLET_INCLUDE_COLLISION_DIR} ${BULLET_INCLUDE_DYNAMICS_DIR} ${BULLET_INCLUDE_CONVEXDECOMPOSITION_DIR} ${BULLET_INCLUDE_SOFTBODY_DIR} ${BULLET_INCLUDE_LINEARMATH_DIR} ${BULLET_INCLUDE_DIRS} ${BULLET_INCLUDE_FILELOADER_DIR} ${BULLET_INCLUDE_WORLDIMPORTER_DIR} ${PDCURSES_INCLUDE_DIR} ${MOUNT_DIR} ${BZ2_INCLUDE_DIR} ${TREMGAME_DIR} )
    target_link_libraries( server ${SDL2_LIBRARY} ${SDL2MAIN_LIBRARY} ${ZLIB_LIBRARIES} ${CURL_LIBRARIES} ${PDCURSES_LIBRARY} ${BULLET_LIBRARIES} ${PDCURSES_LIBRARY} ${BZ2_LIBRARY} )
    if( WIN32 )
	ADD_PRECOMPILED_HEADER( server "${MOUNT_DIR}/null/null_precompiled.h" "${MOUNT_DIR}/null/null_precompiled.cpp" )
    endif(WIN32)
  # The following section finds the OPTIONAL dependencies and makes them available to the target

    set( CURSES_NEED_NCURSES 1 ) # Tells FindCurses that ncurses is required
    # Determine platform
    if( WIN32 )
        find_package( PDCurses REQUIRED )
    elseif( UNIX )
        find_package( Curses REQUIRED )
    endif()
    set_property( TARGET server APPEND PROPERTY COMPILE_DEFINITIONS USE_CURSES )
    TARGET_INCLUDE_DIRECTORIES( server PRIVATE ${CURSES_INCLUDE_DIR} )
    target_link_libraries( server ${CURSES_LIBRARIES} )
  
  find_package( MySQL REQUIRED )
  set_property( TARGET server APPEND PROPERTY COMPILE_DEFINITIONS ET_MYSQL )
  TARGET_INCLUDE_DIRECTORIES( server PRIVATE ${MYSQL_INCLUDE_DIR} )
  target_link_libraries( server ${MYSQL_LIBRARIES} )

endif()

###########################
# Build Autoupdate Server #
###########################

if( BUILD_AUTOUPDATE_SERVER )
  # Setup the target Server
  add_executable( autoupdateserver ${SHAREDLIST_HEADERS} ${CRYPTOLIST_HEADERS} ${FRAMEWORKS_HEADERS}
    ${SERVERLIST_HEADERS} ${COLLISIONMODEL_HEADERS} ${CLIENTLIST_HEADERS} ${CRYPTOLIST_SOURCES} 
	${BOTLIBLIST_HEADERS} ${PHYSICLIB_HEADERS} ${SHAREDLIST_SOURCES} ${SERVERLIST_SOURCES} ${COLLISIONMODEL_SOURCES} 
	${QCOMMONLIST_SOURCES} ${BOTLIBLIST_SOURCES} ${PHYSICLIB_SOURCES} ${PLATFORMLIST_HEADERS} ${FRAMEWORKS_SOURCES}
	${OWLIB_HEADERS} ${OWLIB_SOURCES} ${PLATFORM_DEDICATED_SOURCES} ${DATABASELIST_SOURCES} ${DATABASELIST_HEADERS} 
    ${MOUNT_DIR}/null/null_client.cpp ${MOUNT_DIR}/null/null_input.cpp ${MOUNT_DIR}/qcommon/dl_main_stubs.cpp ${MOUNT_DIR}/null/null_precompiled.h ${MOUNT_DIR}/null/null_precompiled.cpp)
	
  target_link_libraries( autoupdateserver ${OS_LIBRARIES} )
  set_property( TARGET autoupdateserver APPEND PROPERTY COMPILE_DEFINITIONS BOTLIB BOTLIB_STATIC DEDICATED USE_CRYPTO _AMD64_ WIN32 _AMD64 _WIN64 __WIN64__ UPDATE_SERVER  _PHYSICSLIB )
  set_target_properties( autoupdateserver PROPERTIES OUTPUT_NAME "Termulousupdateded.${BUILD_ARCH}" PREFIX "" LINKER_LANGUAGE CXX )
  if( WIN32 )
  ADD_PRECOMPILED_HEADER( autoupdateserver "${MOUNT_DIR}/null/null_precompiled.h" "${MOUNT_DIR}/null/null_precompiled.cpp" )
  endif(WIN32)
  install( TARGETS autoupdateserver
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
  )
  # The following section finds the REQUIRED dependencies and makes them available to the target
    find_package( SDL REQUIRED )
    find_package( Bullet REQUIRED )
    find_package( CURL REQUIRED )
    find_package( Zlib REQUIRED )

    TARGET_INCLUDE_DIRECTORIES( autoupdateserver PRIVATE ${TOOLS_DIR}/owmap ${SDL2_INCLUDE_DIR} ${ZLIB_INCLUDE_DIRS} ${CURL_INCLUDE_DIRS} ${BULLET_INCLUDE_DIR} ${BULLET_INCLUDE_COLLISION_DIR} ${BULLET_INCLUDE_DYNAMICS_DIR} ${BULLET_INCLUDE_CONVEXDECOMPOSITION_DIR} ${BULLET_INCLUDE_SOFTBODY_DIR} ${BULLET_INCLUDE_LINEARMATH_DIR} ${BULLET_INCLUDE_DIRS} ${BULLET_INCLUDE_FILELOADER_DIR} ${BULLET_INCLUDE_WORLDIMPORTER_DIR} ${PDCURSES_INCLUDE_DIR} ${MOUNT_DIR} ${TREMGAME_DIR} )
    target_link_libraries( autoupdateserver ${SDL2_LIBRARY} ${SDL2MAIN_LIBRARY} ${ZLIB_LIBRARIES} ${CURL_LIBRARIES} ${PDCURSES_LIBRARY} ${BULLET_LIBRARIES} ${PDCURSES_LIBRARY} )
  # The following section finds the OPTIONAL dependencies and makes them available to the target

    set( CURSES_NEED_NCURSES 1 ) # Tells FindCurses that ncurses is required
    # Determine platform
    if( WIN32 )
        find_package( PDCurses REQUIRED )
    elseif( UNIX )
        find_package( Curses REQUIRED )
    endif()
    set_property( TARGET autoupdateserver APPEND PROPERTY COMPILE_DEFINITIONS USE_CURSES )
    TARGET_INCLUDE_DIRECTORIES( autoupdateserver PRIVATE ${CURSES_INCLUDE_DIR} )
    target_link_libraries( autoupdateserver ${CURSES_LIBRARIES} )

  find_package( MySQL REQUIRED )
  set_property( TARGET autoupdateserver APPEND PROPERTY COMPILE_DEFINITIONS ET_MYSQL )
  TARGET_INCLUDE_DIRECTORIES( autoupdateserver PRIVATE ${MYSQL_INCLUDE_DIR} )
  target_link_libraries( autoupdateserver ${MYSQL_LIBRARIES} )  
endif()

################
# Build OWMap  #
################
if( BUILD_OWMAP )
  # Dushan - This is how visual studio directories (or filters) are generated
  if( MSVC )
    SOURCE_GROUP( Header\ Files\\OWMap                                    FILES ${OWMAPLIST_HEADERS} )
    SOURCE_GROUP( Header\ Files\\Physics                                  FILES ${PHYSICLIB_HEADERS} )
	SOURCE_GROUP( Header\ Files\\OWLib                                    FILES ${OWLIB_HEADER} )
    SOURCE_GROUP( Source\ Files\\OWMap                                    FILES ${OWMAPLIST_SOURCES} )
	SOURCE_GROUP( Source\ Files\\Physics                                  FILES ${PHYSICLIB_SOURCES} )
	SOURCE_GROUP( Source\ Files\\OWLib                                    FILES ${OWLIB_SOURCES} )
    SOURCE_GROUP( Header\ Files\\SharedList                               FILES ${SHAREDLIST_HEADERS} )
    SOURCE_GROUP( Source\ Files\\SharedList                               FILES ${SHAREDLIST_SOURCES} )	
  endif( MSVC )

   # Setup OWMap target
  add_executable( OWMap ${OWMAPLIST_SOURCES} ${OWMAPLIST_HEADERS} ${PHYSICLIB_HEADERS} ${PHYSICLIB_SOURCES} ${OWLIB_SOURCES} ${SHAREDLIST_HEADERS} ${SHAREDLIST_SOURCES} )
  #set_property( TARGET OWMap APPEND PROPERTY COMPILE_DEFINITIONS _AMD64_ NDEBUG _WIN32 WIN32 _AMD64 _WIN64 __WIN64__ USE_OPENGL Q3MAP2 _PHYSICSLIB )
 if(WIN32)
  set_property( TARGET OWMap APPEND PROPERTY COMPILE_DEFINITIONS _AMD64_ NDEBUG _WIN32 WIN32 _AMD64 _WIN64 __WIN64__ Q3MAP2 _PHYSICSLIB )
 elseif( UNIX )
  set_property( TARGET OWMap APPEND PROPERTY COMPILE_DEFINITIONS __LINUX__ _LINUX_ LINUX Q3MAP2 _PHYSICSLIB )
 endif() 
  
  # This section finds and provides REQUIRED dependencies to OWMap target
  find_package( OpenGL REQUIRED )
  find_package( JPEG REQUIRED )
  find_package( Bullet REQUIRED )
  find_package( PNG REQUIRED )
  find_package( ZLIB REQUIRED )
  
  TARGET_INCLUDE_DIRECTORIES( OWMap PRIVATE ${TOOLS_DIR}/owmap ${TOOLS_DIR}/owmap/libs/assetsys ${TOOLS_DIR}/owmap/libs/catch2 ${TOOLS_DIR}/owmap/libs/filesystem ${TOOLS_DIR}/owmap/libs/jpeg-compressor ${TOOLS_DIR}/owmap/libs/lodepng ${TOOLS_DIR}/owmap/libs/msdirent ${TOOLS_DIR}/owmap/libs/picomodel/src ${TOOLS_DIR}/owmap/libs/picomodel/lwo ${TOOLS_DIR}/owmap/libs/table-builder ${TOOLS_DIR}/owmap/libs/tinyformat ${BULLET_INCLUDE_DIR} ${BULLET_INCLUDE_COLLISION_DIR} ${BULLET_INCLUDE_DYNAMICS_DIR} ${BULLET_INCLUDE_CONVEXDECOMPOSITION_DIR} ${BULLET_INCLUDE_SOFTBODY_DIR} ${BULLET_INCLUDE_LINEARMATH_DIR} ${BULLET_INCLUDE_DIRS} ${BULLET_INCLUDE_FILELOADER_DIR} ${BULLET_INCLUDE_WORLDIMPORTER_DIR} ${OPENGL_INCLUDE_DIR} ${JPEG_INCLUDE_DIR} ${PNG_INCLUDE_DIR} ${ZLIB_INCLUDE_DIRS} ${MOUNT_DIR} )
  
 if(WIN32)
  target_link_libraries( OWMap ${OPENGL_LIBRARIES} ${JPEG_LIBRARIES} ${PNG_LIBRARIES} ${ZLIB_LIBRARIES} ${BULLET_LIBRARIES} wsock32 ws2_32 )
 elseif( UNIX )
 target_link_libraries( OWMap ${OPENGL_LIBRARIES} ${JPEG_LIBRARIES} ${PNG_LIBRARIES} ${ZLIB_LIBRARIES} ${BULLET_LIBRARIES} ${BULLET_COLLISION_LIBRARY} ${BULLET_DYNAMICS_LIBRARY} ${BULLET_FILELOADER_LIBRARY} ${BULLET_SOFTBODY_LIBRARY} ${BULLET_WORLDIMPORTER_LIBRARY} ${BULLET_CONVEXDECOMPOSITION_LIBRARY} ${BULLET_LINEARMATH_LIBRARY} -lpthread )
 endif() 
 
  set_target_properties( OWMap PROPERTIES OUTPUT_NAME "OWMap.${BUILD_ARCH}" PREFIX "" )

install( TARGETS OWMap
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
  )
endif()

################
# Build OWMap  #
################
if( BUILD_OWMAP )
  # Dushan - This is how visual studio directories (or filters) are generated
  if( MSVC )
    SOURCE_GROUP( Header\ Files\\OWMaster                                 FILES ${OWMASTERSERVERLIST_HEADERS} )
    SOURCE_GROUP( Source\ Files\\OWMaster                                 FILES ${OWMASTERSERVERLIST_SOURCES} )
  endif( MSVC )

   # Setup OWMap target
  add_executable( OWMaster ${OWMASTERSERVERLIST_SOURCES} ${OWMASTERSERVERLIST_HEADERS} )
  if(WIN32)
  set_property( TARGET OWMaster APPEND PROPERTY COMPILE_DEFINITIONS _AMD64_ NDEBUG _WIN32 WIN32 _AMD64 _WIN64 __WIN64__  )
 elseif( UNIX )
  set_property( TARGET OWMaster APPEND PROPERTY COMPILE_DEFINITIONS __LINUX__ _LINUX_ LINUX )
 endif() 
  
  TARGET_INCLUDE_DIRECTORIES( OWMaster PRIVATE ${TOOLS_DIR}/master )
     
if(WIN32)
  target_link_libraries( OWMaster wsock32 ws2_32 )
 elseif( UNIX )
 target_link_libraries( OWMaster -ltdb -lm )
 endif() 

  set_target_properties( OWMaster PROPERTIES OUTPUT_NAME "OWMaster.${BUILD_ARCH}" PREFIX "" )
    
  install( TARGETS OWMaster
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
  )
endif()
