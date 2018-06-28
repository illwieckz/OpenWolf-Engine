#
# OpenWolf CMakeLists.txt
#
# cmake required (http://www.cmake.org/)
#

cmake_minimum_required( VERSION 2.8 )

if( POLICY CMP0017 )
  cmake_policy( SET CMP0017 NEW )
endif()

include( CMakeDependentOption )
include( cmake/precompiledHeaders.cmake )

# This line has to appear before 'PROJECT' in order to be able to disable incremental linking
set( MSVC_INCREMENTAL_DEFAULT ON )

set( CMAKE_BUILD_TYPE Release CACHE STRING "Build configuration (Debug, Release, RelWithDebInfo, MinSizeRel)" )

project( OpenWolf C CXX )

if( MSVC )
  set( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /fp:precise -Od" )
else()
  set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse2 -pipe" )
  set( CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g" )
  set( CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} -DNDEBUG -Od" )
  set( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DNDEBUG -Od -ffast-math -fomit-frame-pointer -fno-strict-aliasing" )
  set( CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -DNDEBUG -Od -ffast-math -fomit-frame-pointer -fno-strict-aliasing -g" )
endif()

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_C_FLAGS}" )
set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${CMAKE_C_FLAGS_DEBUG}" )
set( CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} ${CMAKE_C_FLAGS_MINSIZEREL}" )
set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${CMAKE_C_FLAGS_RELEASE}" )
set( CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${CMAKE_C_FLAGS_RELWITHDEBINFO}" )
set( CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "" ) # Need this to get rid of -rdynamic linker flag set by default by cmake
set( CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" )

# Options
option( BUILD_CLIENT                   "Build OpenWolf client"                                 ON )
#option( BUILD_SERVER                   "Build OpenWolfded server"                              ON )
o#ption( BUILD_AUTOUPDATE_SERVER        "Build OpenWolf AutoUpdate server"                      ON )
option( BUILD_GAME                     "Build game logic shared libraries"                     ON )
option( USE_CURSES                     "Use the curses console"                                ON ) 
option( NO_UNTRUSTED_PLUGINS           "Don't load native plugins from the user dir"           ON )

# Package info
set( CPACK_PACKAGE_DESCRIPTION_SUMMARY "OpenWolf client" )
set( CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.txt" )
set( CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/GPL.txt" )
include( CPack )

set( MOUNT_DIR ${CMAKE_SOURCE_DIR}/src/engine )
set( LIB_DIR ${CMAKE_SOURCE_DIR}/src/libs )
set( TOOLS_DIR ${CMAKE_SOURCE_DIR}/src/tools )

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
  set( BUILD_ARCH "x86_64" )
elseif( ${CMAKE_SYSTEM_PROCESSOR} MATCHES "i.86" )
  set( BUILD_ARCH "i386" )
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
  ${MOUNT_DIR}/GPURenderer/r_common.h
  ${MOUNT_DIR}/GPURenderer/r_dsa.h
  ${MOUNT_DIR}/GPURenderer/r_extramath.h
  ${MOUNT_DIR}/GPURenderer/r_extratypes.h
  ${MOUNT_DIR}/GPURenderer/r_fbo.h
  ${MOUNT_DIR}/GPURenderer/r_local.h
  ${MOUNT_DIR}/GPURenderer/r_postprocess.h
  ${MOUNT_DIR}/GPURenderer/r_public.h
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
  ${MOUNT_DIR}/bgame/bg_public.h
  ${MOUNT_DIR}/GUI/gui_api.h
  ${MOUNT_DIR}/GUI/gui_local.h
  ${MOUNT_DIR}/GUI/gui_shared.h
  ${MOUNT_DIR}/GUI/keycodes.h
  ${MOUNT_DIR}/GUI/menudef.h
)

set( GUI_SOURCES
  ${MOUNT_DIR}/bgame/bg_misc.cpp
  ${MOUNT_DIR}/bgame/bg_alloc.cpp
  ${MOUNT_DIR}/bgame/bg_pmove.cpp
  ${MOUNT_DIR}/bgame/bg_slidemove.cpp
  ${MOUNT_DIR}/bgame/bg_voice.cpp
  ${MOUNT_DIR}/GUI/gui_atoms.cpp
  ${MOUNT_DIR}/GUI/gui_gameinfo.cpp
  ${MOUNT_DIR}/GUI/gui_main.cpp
  ${MOUNT_DIR}/GUI/gui_shared.cpp
)

set( SERVERLIST_HEADERS
  ${MOUNT_DIR}/server/server.h
  ${MOUNT_DIR}/sgame/sg_api.h
)

set( SERVERLIST_SOURCES
  ${MOUNT_DIR}/server/sv_bot.cpp
  ${MOUNT_DIR}/server/sv_ccmds.cpp
  ${MOUNT_DIR}/server/sv_client.cpp
  ${MOUNT_DIR}/server/sv_game.cpp
  ${MOUNT_DIR}/server/sv_init.cpp
  ${MOUNT_DIR}/server/sv_main.cpp
  ${MOUNT_DIR}/server/sv_net_chan.cpp
  ${MOUNT_DIR}/server/sv_snapshot.cpp
  ${MOUNT_DIR}/server/sv_world.cpp
)

set( COLLISIONMODEL_HEADERS
  ${MOUNT_DIR}/cm/cm_header.h
  ${MOUNT_DIR}/cm/cm_local.h
  ${MOUNT_DIR}/cm/cm_model.h
  ${MOUNT_DIR}/cm/cm_patch.h
  ${MOUNT_DIR}/cm/cm_polylib.h
  ${MOUNT_DIR}/cm/cm_public.h
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
  ${MOUNT_DIR}/qcommon/dl_public.h
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
  ${MOUNT_DIR}/qcommon/cvar.cpp
  ${MOUNT_DIR}/qcommon/files.cpp
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
  ${MOUNT_DIR}/qcommon/parse.cpp
  ${MOUNT_DIR}/qcommon/puff.cpp
  ${MOUNT_DIR}/qcommon/Unzip.cpp
)

if(USE_OPENSSL)
  find_package(OpenSSL REQUIRED)
  TARGET_INCLUDE_DIRECTORIES(${OPENSSL_INCLUDE_DIR})
  add_definitions( -DUSE_OPENSSL )
endif()

set( DATABASELIST_HEADERS 
  ${MOUNT_DIR}/database/db_local.h
  ${MOUNT_DIR}/database/db_public.h
)
set( DATABASELIST_SOURCES
  ${MOUNT_DIR}/database/db_main.cpp
)

set( CLIENTLIST_HEADERS
  ${MOUNT_DIR}/cgame/cg_api.h
  ${MOUNT_DIR}/client/client.h
  ${MOUNT_DIR}/client/keycodes.h
  ${MOUNT_DIR}/client/keys.h
  ${MOUNT_DIR}/client/cl_HydraManager.h
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
  ${MOUNT_DIR}/audio/snd_local.h
  ${MOUNT_DIR}/audio/snd_public.h
)

set( AUDIO_SOURCES
  ${MOUNT_DIR}/audio/snd_adpcm.cpp
  ${MOUNT_DIR}/audio/snd_dma.cpp
  ${MOUNT_DIR}/audio/snd_mem.cpp
  ${MOUNT_DIR}/audio/snd_mix.cpp
  ${MOUNT_DIR}/audio/snd_wavelet.cpp
)

set( SDLBASELIST_SOURCES
  ${MOUNT_DIR}/sys/sdl_glimp.cpp
  ${MOUNT_DIR}/sys/sdl_gamma.cpp
  ${MOUNT_DIR}/sys/sys_win32.cpp
  ${MOUNT_DIR}/sys/sys_main.cpp
  ${MOUNT_DIR}/sys/con_log.cpp
  ${MOUNT_DIR}/sys/con_win32.cpp
  ${MOUNT_DIR}/sys/con_curses.cpp
)

SET ( SDLBASE_DEDICATED_SOURCES
    ${MOUNT_DIR}/sys/sys_main.cpp
	${MOUNT_DIR}/sys/sys_win32.cpp
	${MOUNT_DIR}/sys/con_win32.cpp
    ${MOUNT_DIR}/sys/con_log.cpp
    ${MOUNT_DIR}/sys/con_curses.cpp
)

SET ( SDLBASELIST_HEADERS
    ${MOUNT_DIR}/sys/resource.h
	${MOUNT_DIR}/sys/sdl_icon.h
	${MOUNT_DIR}/sys/sys_loadlib.h
	${MOUNT_DIR}/sys/sys_local.h
)

set( SDLLIST
  ${MOUNT_DIR}/sys/sdl_input.cpp
  ${MOUNT_DIR}/sys/sdl_snd.cpp
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
  ${MOUNT_DIR}/sgame/chars.h
  ${MOUNT_DIR}/sgame/inv.h
  ${MOUNT_DIR}/sgame/match.h
  ${MOUNT_DIR}/sgame/syn.h

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
  ${MOUNT_DIR}/physicslib/bspfile_abstract.h
  ${MOUNT_DIR}/physicslib/physics_collision.h
  ${MOUNT_DIR}/physicslib/physics_local.h
  ${MOUNT_DIR}/physicslib/physics_public.h
)

set( PHYSICLIB_SOURCES
  ${MOUNT_DIR}/physicslib/physics_main.cpp
  ${MOUNT_DIR}/physicslib/physics_collision.cpp
  ${MOUNT_DIR}/physicslib/physics_debug.cpp
  ${MOUNT_DIR}/physicslib/physics_tracemodel.cpp
)

set ( OWLIB_HEADERS
  ${MOUNT_DIR}/OWLib/precompiled.h
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
  ${MOUNT_DIR}/OWLib/precompiled.cpp
  ${MOUNT_DIR}/OWLib/math_angles.cpp
  ${MOUNT_DIR}/OWLib/math_matrix.cpp
  ${MOUNT_DIR}/OWLib/math_vector.cpp
  ${MOUNT_DIR}/OWLib/q_parse.cpp
  ${MOUNT_DIR}/OWLib/q_shared.cpp
  ${MOUNT_DIR}/OWLib/splines.cpp
  ${MOUNT_DIR}/OWLib/util_str.cpp
)

set( QAGAMELIST_HEADERS
  ${MOUNT_DIR}/sgame/sg_precompiled.h
  ${MOUNT_DIR}/botai/ai_chat.h
  ${MOUNT_DIR}/botai/ai_local.h
  ${MOUNT_DIR}/bgame/bg_local.h
  ${MOUNT_DIR}/bgame/bg_public.h
  ${MOUNT_DIR}/sgame/sg_admin.h
  ${MOUNT_DIR}/sgame/sg_local.h
  ${MOUNT_DIR}/qcommon/surfaceflags.h
  ${MOUNT_DIR}/sgame/sg_api.h
  ${MOUNT_DIR}/sgame/tremulous.h
  ${MOUNT_DIR}/sgame/chars.h
  ${MOUNT_DIR}/sgame/inv.h
  ${MOUNT_DIR}/sgame/match.h
  ${MOUNT_DIR}/sgame/syn.h
)

set( QAGAMELIST_SOURCES
  ${MOUNT_DIR}/sgame/sg_precompiled.cpp
  ${MOUNT_DIR}/botai/ai_alien.cpp
  ${MOUNT_DIR}/botai/ai_chat.cpp
  ${MOUNT_DIR}/botai/ai_human.cpp
  ${MOUNT_DIR}/botai/ai_main.cpp
  ${MOUNT_DIR}/botai/ai_utils.cpp
  ${MOUNT_DIR}/bgame/bg_alloc.cpp
  ${MOUNT_DIR}/bgame/bg_misc.cpp
  ${MOUNT_DIR}/bgame/bg_pmove.cpp
  ${MOUNT_DIR}/bgame/bg_slidemove.cpp
  ${MOUNT_DIR}/bgame/bg_voice.cpp
  ${MOUNT_DIR}/sgame/sg_active.cpp
  ${MOUNT_DIR}/sgame/sg_admin.cpp
  ${MOUNT_DIR}/sgame/sg_api.cpp
  ${MOUNT_DIR}/sgame/sg_bot.cpp
  ${MOUNT_DIR}/sgame/sg_buildable.cpp
  ${MOUNT_DIR}/sgame/sg_client.cpp
  ${MOUNT_DIR}/sgame/sg_cmds.cpp
  ${MOUNT_DIR}/sgame/sg_combat.cpp
  ${MOUNT_DIR}/sgame/sg_main.cpp
  ${MOUNT_DIR}/sgame/sg_maprotation.cpp
  ${MOUNT_DIR}/sgame/sg_mem.cpp
  ${MOUNT_DIR}/sgame/sg_misc.cpp
  ${MOUNT_DIR}/sgame/sg_missile.cpp
  ${MOUNT_DIR}/sgame/sg_mover.cpp
  ${MOUNT_DIR}/sgame/sg_physics.cpp
  ${MOUNT_DIR}/sgame/sg_ptr.cpp
  ${MOUNT_DIR}/sgame/sg_session.cpp
  ${MOUNT_DIR}/sgame/sg_spawn.cpp
  ${MOUNT_DIR}/sgame/sg_svcmds.cpp
  ${MOUNT_DIR}/sgame/sg_target.cpp
  ${MOUNT_DIR}/sgame/sg_team.cpp
  ${MOUNT_DIR}/sgame/sg_trigger.cpp
  ${MOUNT_DIR}/sgame/sg_utils.cpp
  ${MOUNT_DIR}/sgame/sg_weapon.cpp
)

set( CGAMELIST_HEADERS
  ${MOUNT_DIR}/cgame/cg_precompiled.h
  ${MOUNT_DIR}/cgame/cg_api.h
  ${MOUNT_DIR}/GUI/menudef.h
  ${MOUNT_DIR}/bgame/bg_local.h
  ${MOUNT_DIR}/bgame/bg_public.h
  ${MOUNT_DIR}/qcommon/surfaceflags.h
  ${MOUNT_DIR}/GUI/keycodes.h
  ${MOUNT_DIR}/GUI/gui_shared.h
  ${MOUNT_DIR}/cgame/cg_local.h
  ${MOUNT_DIR}/GPURenderer/r_types.h
)

set( CGAMELIST_SOURCES
  ${MOUNT_DIR}/cgame/cg_precompiled.cpp
  ${MOUNT_DIR}/bgame/bg_alloc.cpp
  ${MOUNT_DIR}/bgame/bg_misc.cpp
  ${MOUNT_DIR}/bgame/bg_pmove.cpp
  ${MOUNT_DIR}/bgame/bg_voice.cpp
  ${MOUNT_DIR}/bgame/bg_slidemove.cpp
  ${MOUNT_DIR}/cgame/cg_animation.cpp
  ${MOUNT_DIR}/cgame/cg_animmapobj.cpp
  ${MOUNT_DIR}/cgame/cg_api.cpp
  ${MOUNT_DIR}/cgame/cg_attachment.cpp
  ${MOUNT_DIR}/cgame/cg_buildable.cpp
  ${MOUNT_DIR}/cgame/cg_consolecmds.cpp
  ${MOUNT_DIR}/cgame/cg_draw.cpp
  ${MOUNT_DIR}/cgame/cg_drawtools.cpp
  ${MOUNT_DIR}/cgame/cg_ents.cpp
  ${MOUNT_DIR}/cgame/cg_event.cpp
  ${MOUNT_DIR}/cgame/cg_main.cpp
  ${MOUNT_DIR}/cgame/cg_marks.cpp
  ${MOUNT_DIR}/cgame/cg_mem.cpp
  ${MOUNT_DIR}/cgame/cg_particles.cpp
  ${MOUNT_DIR}/cgame/cg_players.cpp
  ${MOUNT_DIR}/cgame/cg_playerstate.cpp
  ${MOUNT_DIR}/cgame/cg_predict.cpp
  ${MOUNT_DIR}/cgame/cg_ptr.cpp
  ${MOUNT_DIR}/cgame/cg_scanner.cpp
  ${MOUNT_DIR}/cgame/cg_servercmds.cpp
  ${MOUNT_DIR}/cgame/cg_snapshot.cpp
  ${MOUNT_DIR}/cgame/cg_trails.cpp
  ${MOUNT_DIR}/cgame/cg_tutorial.cpp
  ${MOUNT_DIR}/cgame/cg_view.cpp
  ${MOUNT_DIR}/cgame/cg_weapons.cpp
  ${MOUNT_DIR}/GUI/gui_shared.cpp
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

if( NO_UNTRUSTED_PLUGINS )
  add_definitions( -DNO_UNTRUSTED_PLUGINS )
endif()

###############################
# Build Game/Cgame/Ui DLL/.so #
###############################

if( BUILD_GAME )
  add_library( sgame SHARED ${SHAREDLIST_HEADERS} ${SHAREDLIST_SOURCES} ${QAGAMELIST_HEADERS} ${QAGAMELIST_SOURCES} )
  if( MSVC )
    SOURCE_GROUP( Header\ Files\\SharedList                               FILES ${SHAREDLIST_HEADERS} )
	SOURCE_GROUP( Header\ Files\\GameFiles                                FILES ${QAGAMELIST_HEADERS} )
    SOURCE_GROUP( Source\ Files\\SharedList                               FILES ${SHAREDLIST_SOURCES} )
	SOURCE_GROUP( Source\ Files\\GameFiles                                FILES ${QAGAMELIST_SOURCES} )
	
	target_link_libraries( sgame winmm.lib )
	
	ADD_PRECOMPILED_HEADER( sgame "${MOUNT_DIR}/sgame/sg_precompiled.h" "${MOUNT_DIR}/sgame/sg_precompiled.cpp" )
	SET_TARGET_PROPERTIES( sgame PROPERTIES LINK_FLAGS "/DEF:\"${MOUNT_DIR}/sgame/sgame.def" )
  endif( MSVC )

  TARGET_INCLUDE_DIRECTORIES( sgame PRIVATE ${MOUNT_DIR} )
  set_property( TARGET sgame PROPERTY COMPILE_DEFINITIONS _AMD64_ WIN32 _AMD64 _WIN64 __WIN64__ GAMEDLL )
  
  set_target_properties( sgame PROPERTIES
    OUTPUT_NAME "sgame${BUILD_ARCH}"
    PREFIX ""
    LIBRARY_OUTPUT_DIRECTORY "main"
  )
  target_link_libraries( sgame )

  add_library( cgame SHARED ${SHAREDLIST_HEADERS} ${SHAREDLIST_SOURCES} ${CGAMELIST_HEADERS} ${CGAMELIST_SOURCES} )
  if( MSVC )
    SOURCE_GROUP( Header\ Files\\SharedList                               FILES ${SHAREDLIST_HEADERS} )
	SOURCE_GROUP( Header\ Files\\GameFiles                                FILES ${CGAMELIST_HEADERS} )
    SOURCE_GROUP( Source\ Files\\SharedList                               FILES ${SHAREDLIST_SOURCES} )
	SOURCE_GROUP( Source\ Files\\GameFiles                                FILES ${CGAMELIST_SOURCES} )
	
    ADD_PRECOMPILED_HEADER( cgame "${MOUNT_DIR}/cgame/cg_precompiled.h" "${MOUNT_DIR}/cgame/cg_precompiled.cpp" )	
	SET_TARGET_PROPERTIES( cgame PROPERTIES LINK_FLAGS "/DEF:\"${MOUNT_DIR}/cgame/cgame.def" )
  endif( MSVC )
  
  TARGET_INCLUDE_DIRECTORIES( cgame PRIVATE ${MOUNT_DIR} )
  set_property( TARGET cgame PROPERTY COMPILE_DEFINITIONS _AMD64_ NDEBUG _WIN32 WIN32 _AMD64 _WIN64 __WIN64__ CGAMEDLL )
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
  if( MSVC )
    SOURCE_GROUP( Header\ Files\\SharedList                               FILES ${SHAREDLIST_HEADERS} )
    SOURCE_GROUP( Header\ Files\\GPURenderer                              FILES ${GPURENDERER_HEADERS} )
	#SOURCE_GROUP( Header\ Files\\GPURenderer\\OpeneXRList                 FILES ${OPENEXRLIST_HEADERS} )
    SOURCE_GROUP( Header\ Files\\GPUWorkers                               FILES ${GPUWORKERS_HEADERS} )
    SOURCE_GROUP( Header\ Files\\GUI                                      FILES ${GUI_HEADERS} )
    SOURCE_GROUP( Header\ Files\\Server                                   FILES ${SERVERLIST_HEADERS} )
    SOURCE_GROUP( Header\ Files\\CM                                       FILES ${COLLISIONMODEL_HEADERS} )
    SOURCE_GROUP( Header\ Files\\QCommon                                  FILES ${QCOMMONLIST_HEADERS} )
    SOURCE_GROUP( Header\ Files\\Client                                   FILES ${CLIENTLIST_HEADERS} )
    SOURCE_GROUP( Header\ Files\\Audio                                    FILES ${AUDIO_HEADERS} )
    SOURCE_GROUP( Header\ Files\\Platform                                 FILES ${SDLBASELIST_HEADERS} )
    SOURCE_GROUP( Header\ Files\\Botlib                                   FILES ${BOTLIBLIST_HEADERS} )
    SOURCE_GROUP( Header\ Files\\Physics                                  FILES ${PHYSICLIB_HEADERS} )
    SOURCE_GROUP( Header\ Files\\OWLib                                    FILES ${OWLIB_HEADERS} )
    SOURCE_GROUP( Header\ Files\\Database                                 FILES ${DATABASELIST_HEADERS} )
    SOURCE_GROUP( Source\ Files\\SharedList                               FILES ${SHAREDLIST_SOURCES} )
    SOURCE_GROUP( Source\ Files\\GPURenderer                              FILES ${GPURENDERER_SOURCES} )
	#SOURCE_GROUP( Source\ Files\\GPURenderer\\OpeneXRList                 FILES ${OPENEXRLIST_SOURCES} )
    SOURCE_GROUP( Source\ Files\\GPUWorkers                               FILES ${GPUWORKERS_SOURCES} )
    SOURCE_GROUP( Source\ Files\\GUI                                      FILES ${GUI_SOURCES} )
    SOURCE_GROUP( Source\ Files\\Server                                   FILES ${SERVERLIST_SOURCES} )
    SOURCE_GROUP( Source\ Files\\CM                                       FILES ${COLLISIONMODEL_SOURCES} )
    SOURCE_GROUP( Source\ Files\\QCommon                                  FILES ${QCOMMONLIST_SOURCES} )
    SOURCE_GROUP( Source\ Files\\Client                                   FILES ${CLIENTLIST_SOURCES} )
    SOURCE_GROUP( Source\ Files\\Audio                                    FILES ${AUDIO_SOURCES} )
    SOURCE_GROUP( Source\ Files\\Platform                                 FILES ${SDLBASELIST_SOURCES} )
    SOURCE_GROUP( Source\ Files\\SDL                                      FILES ${SDLLIST} )
    SOURCE_GROUP( Source\ Files\\Botlib                                   FILES ${BOTLIBLIST_SOURCES} )
    SOURCE_GROUP( Source\ Files\\Physics                                  FILES ${PHYSICLIB_SOURCES} )
	SOURCE_GROUP( Source\ Files\\OWLib                                    FILES ${OWLIB_SOURCES} )
    SOURCE_GROUP( Source\ Files\\Database                                 FILES ${DATABASELIST_SOURCES} )
  endif( MSVC )
  
  add_executable( client ${SHAREDLIST_HEADERS} ${GPURENDERER_HEADERS} ${QCOMMONLIST_HEADERS} ${GPUWORKERS_HEADERS} ${GUI_HEADERS} ${SERVERLIST_HEADERS} ${COLLISIONMODEL_HEADERS} ${CLIENTLIST_HEADERS} ${AUDIO_HEADERS} ${BOTLIBLIST_HEADERS} ${PHYSICLIB_HEADERS} ${SHAREDLIST_SOURCES} ${GPURENDERER_SOURCES} ${GPUWORKERS_SOURCES} ${GUI_SOURCES} ${SERVERLIST_SOURCES} ${COLLISIONMODEL_SOURCES} ${QCOMMONLIST_SOURCES} ${CLIENTLIST_SOURCES} ${AUDIO_SOURCES} ${SDLBASELIST} ${SDLLIST} ${BOTLIBLIST_SOURCES} ${PHYSICLIB_SOURCES} ${DATABASELIST_SOURCES}
  ${DATABASELIST_HEADERS} ${BOTLIBLIST} ${MOUNT_DIR}/qcommon/dl_main_curl.cpp ${CRYPTOLIST_SOURCES} ${OWLIB_HEADERS} ${OWLIB_SOURCES} ${SDLBASELIST_SOURCES} ${SDLBASELIST_HEADERS} ${JPEG_INCLUDE_DIR} ${JPEG_SOURCES} )
	
  target_link_libraries( client ${OS_LIBRARIES} )
  set_property( TARGET client APPEND PROPERTY COMPILE_DEFINITIONS _AMD64_ WIN32 _AMD64 _WIN64 __WIN64__ NDEBUG _WINDOWS BOTLIB BOTLIB_STATIC USE_CRYPTO HAVE_CONFIG_H USE_INCREASED_ENTITIES  __STDC_CONSTANT_MACROS _PHYSICSLIB BUILD_FREETYPE HAVE_BOOLEAN )
  ADD_PRECOMPILED_HEADER( client "${MOUNT_DIR}/OWLib/precompiled.h" "${MOUNT_DIR}/OWLib/precompiled.cpp" )
  set_target_properties( client PROPERTIES OUTPUT_NAME "OpenWolf.${BUILD_ARCH}" PREFIX "" LINKER_LANGUAGE CXX )
  install( TARGETS client
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
  )

  # This section finds and provides REQUIRED dependencies to target client
  find_package( OpenGL REQUIRED )
  find_package( Jpeg REQUIRED )
  find_package( SDL REQUIRED )
  find_package( Freetype REQUIRED )
  find_package( GLEW REQUIRED )
  find_package( CURL REQUIRED )
  find_package( Bullet REQUIRED )
  find_package( Zlib REQUIRED )
  find_package( PNG REQUIRED )
  find_package( BZ2 REQUIRED )
  find_package( OpenCL REQUIRED )
  find_package( Sixense REQUIRED )
  find_package( OculusSDK REQUIRED )
  find_package( OculusKernelSDK REQUIRED )
  find_package( WebP REQUIRED )
  
  TARGET_INCLUDE_DIRECTORIES( client PRIVATE ${OPENGL_INCLUDE_DIR} ${FREETYPE_INCLUDE_DIRS} ${SDL_INCLUDE_DIR} ${GLEW_INCLUDE_DIR} ${ZLIB_INCLUDE_DIRS} ${CURL_INCLUDE_DIRS} ${BULLET_INCLUDE_DIR} ${BULLET_INCLUDE_COLLISION_DIR} ${BULLET_INCLUDE_DYNAMICS_DIR} ${BULLET_INCLUDE_CONVEXDECOMPOSITION_DIR} ${BULLET_INCLUDE_SOFTBODY_DIR} ${BULLET_INCLUDE_LINEARMATH_DIR} ${BULLET_INCLUDE_DIRS} ${BULLET_INCLUDE_FILELOADER_DIR} ${BULLET_INCLUDE_WORLDIMPORTER_DIR} ${PNG_INCLUDE_DIR} ${OpenCL_INCLUDE_DIRS} ${SIXENSE_INCLUDE_DIRS} ${LIB_DIR}/jpeg ${PDCURSES_INCLUDE_DIR} ${OVR_INCLUDE_DIRS} ${OVRKERNEL_INCLUDE_DIR} ${MOUNT_DIR} ${JPEG_INCLUDE_DIR} )
  target_link_libraries( client ${OPENGL_LIBRARIES} ${SDL_LIBRARY} ${GLEW_LIBRARY} ${FREETYPE_LIBRARIES} ${ZLIB_LIBRARIES} ${CURL_LIBRARIES} ${PNG_LIBRARIES} ${BULLET_LIBRARIES} ${OpenCL_LIBRARY} ${SIXENSE_LIBRARIES} ${SIXENSE_LIBRARIES_UTILS} ${PDCURSES_LIBRARY} ${OVR_LIBRARIES} ${OVRKERNEL_LIBRARY_RELEASE} ${JPEG_LIBRARIES} ${LINK_LIBRARY})

  if( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
    link_directories( /opt/local/lib )
  endif()

  if( USE_CURSES )
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
  add_executable( server ${SHAREDLIST_HEADERS} ${CRYPTOLIST_HEADERS}
    ${SERVERLIST_HEADERS} ${COLLISIONMODEL_HEADERS} ${CLIENTLIST_HEADERS} ${CRYPTOLIST_SOURCES} 
	${BOTLIBLIST_HEADERS} ${PHYSICLIB_HEADERS} ${SHAREDLIST_SOURCES} ${SERVERLIST_SOURCES} ${COLLISIONMODEL_SOURCES} 
	${QCOMMONLIST_SOURCES} ${BOTLIBLIST_SOURCES} ${PHYSICLIB_SOURCES} ${SDLBASELIST_HEADERS}
	${OWLIB_HEADERS} ${OWLIB_SOURCES} ${SDLBASE_DEDICATED_SOURCES} ${DATABASELIST_SOURCES} ${DATABASELIST_HEADERS} 
    ${MOUNT_DIR}/null/null_client.cpp
    ${MOUNT_DIR}/null/null_input.cpp ${MOUNT_DIR}/null/null_snddma.cpp
    ${MOUNT_DIR}/qcommon/dl_main_stubs.cpp )
	
  target_link_libraries( server ${OS_LIBRARIES} )
  set_property( TARGET server APPEND PROPERTY COMPILE_DEFINITIONS BOTLIB BOTLIB_STATIC DEDICATED USE_CRYPTO _AMD64_ WIN32 _AMD64 _WIN64 __WIN64__  _PHYSICSLIB )
  set_target_properties( server PROPERTIES OUTPUT_NAME "OpenWolfded.${BUILD_ARCH}" PREFIX "" LINKER_LANGUAGE CXX )
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

    TARGET_INCLUDE_DIRECTORIES( server PRIVATE ${SDL_INCLUDE_DIR} ${ZLIB_INCLUDE_DIRS} ${CURL_INCLUDE_DIRS} ${BULLET_INCLUDE_DIR} ${BULLET_INCLUDE_COLLISION_DIR} ${BULLET_INCLUDE_DYNAMICS_DIR} ${BULLET_INCLUDE_CONVEXDECOMPOSITION_DIR} ${BULLET_INCLUDE_SOFTBODY_DIR} ${BULLET_INCLUDE_LINEARMATH_DIR} ${BULLET_INCLUDE_DIRS} ${BULLET_INCLUDE_FILELOADER_DIR} ${BULLET_INCLUDE_WORLDIMPORTER_DIR} ${PDCURSES_INCLUDE_DIR} ${MOUNT_DIR} )
    target_link_libraries( server ${SDL_LIBRARY} ${ZLIB_LIBRARIES} ${CURL_LIBRARIES} ${PDCURSES_LIBRARY} ${BULLET_LIBRARIES} ${PDCURSES_LIBRARY} )
	ADD_PRECOMPILED_HEADER( server "${MOUNT_DIR}/null/null_precompiled.h" "${MOUNT_DIR}/null/null_precompiled.cpp" )
  # The following section finds the OPTIONAL dependencies and makes them available to the target

  if( USE_CURSES )
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
  endif()
  
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
  add_executable( autoupdateserver ${SHAREDLIST_HEADERS} ${CRYPTOLIST_HEADERS}
    ${SERVERLIST_HEADERS} ${COLLISIONMODEL_HEADERS} ${CLIENTLIST_HEADERS} ${CRYPTOLIST_SOURCES} 
	${BOTLIBLIST_HEADERS} ${PHYSICLIB_HEADERS} ${SHAREDLIST_SOURCES} ${SERVERLIST_SOURCES} ${COLLISIONMODEL_SOURCES} 
	${QCOMMONLIST_SOURCES} ${BOTLIBLIST_SOURCES} ${PHYSICLIB_SOURCES} ${SDLBASELIST_HEADERS}
	${OWLIB_HEADERS} ${OWLIB_SOURCES} ${SDLBASE_DEDICATED_SOURCES} ${DATABASELIST_SOURCES} ${DATABASELIST_HEADERS} 
    ${MOUNT_DIR}/null/null_client.cpp
    ${MOUNT_DIR}/null/null_input.cpp ${MOUNT_DIR}/null/null_snddma.cpp
    ${MOUNT_DIR}/qcommon/dl_main_stubs.cpp )
	
  target_link_libraries( autoupdateserver ${OS_LIBRARIES} )
  set_property( TARGET autoupdateserver APPEND PROPERTY COMPILE_DEFINITIONS BOTLIB BOTLIB_STATIC DEDICATED USE_CRYPTO _AMD64_ WIN32 _AMD64 _WIN64 __WIN64__ UPDATE_SERVER  _PHYSICSLIB )
  set_target_properties( autoupdateserver PROPERTIES OUTPUT_NAME "OpenWolfupdateded.${BUILD_ARCH}" PREFIX "" LINKER_LANGUAGE CXX )
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

    TARGET_INCLUDE_DIRECTORIES( autoupdateserver PRIVATE ${SDL_INCLUDE_DIR} ${ZLIB_INCLUDE_DIRS} ${CURL_INCLUDE_DIRS} ${BULLET_INCLUDE_DIR} ${BULLET_INCLUDE_COLLISION_DIR} ${BULLET_INCLUDE_DYNAMICS_DIR} ${BULLET_INCLUDE_CONVEXDECOMPOSITION_DIR} ${BULLET_INCLUDE_SOFTBODY_DIR} ${BULLET_INCLUDE_LINEARMATH_DIR} ${BULLET_INCLUDE_DIRS} ${BULLET_INCLUDE_FILELOADER_DIR} ${BULLET_INCLUDE_WORLDIMPORTER_DIR} ${PDCURSES_INCLUDE_DIR} ${MOUNT_DIR} )
    target_link_libraries( autoupdateserver ${SDL_LIBRARY} ${ZLIB_LIBRARIES} ${CURL_LIBRARIES} ${PDCURSES_LIBRARY} ${BULLET_LIBRARIES} ${PDCURSES_LIBRARY} )
  # The following section finds the OPTIONAL dependencies and makes them available to the target

  if( USE_CURSES )
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
  endif()

  find_package( MySQL REQUIRED )
  set_property( TARGET autoupdateserver APPEND PROPERTY COMPILE_DEFINITIONS ET_MYSQL )
  TARGET_INCLUDE_DIRECTORIES( autoupdateserver PRIVATE ${MYSQL_INCLUDE_DIR} )
  target_link_libraries( autoupdateserver ${MYSQL_LIBRARIES} )  
endif()


