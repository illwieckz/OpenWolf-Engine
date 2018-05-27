# - try to find Oculus VR's SDK for Oculus Rift support
#
# Cache Variables: (probably not for direct use in your scripts)
#  OVRKERNEL_INCLUDE_DIR
#  OVRKERNEL_SOURCE_DIR
#  OVRKERNEL_LIBRARY
#
# Non-cache variables you might use in your CMakeLists.txt:
#  OVR_FOUND
#  OVRKERNEL_INCLUDE_DIRS
#  OVRKERNEL_LIBRARIES
#
# Requires these CMake modules:
#  FindPackageHandleStandardArgs (known included with CMake >=2.6.2)
#
# Original Author:
# 2014 Kevin M. Godby <kevin@godby.org>
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

set(OVR_ROOT_DIR
	"${OVR_ROOT_DIR}"
	${LIB_DIR}/OculusSDK_0.5/OculusSDK/LibOVRKernel
	CACHE
	PATH
	"Directory to search for Oculus SDK")

# The OVR library is built in a directory tree that varies based on platform,
# architecture, and compiler.
#
# The libraries live in one of the following locations:
#
# Lib/Win32/VS2012/libovrd.lib
# Lib/Win32/VS2012/libovr.lib
# Lib/Win32/VS2013/libovrd.lib
# Lib/Win32/VS2013/libovr.lib
# Lib/Win32/VS2010/libovrd.lib
# Lib/Win32/VS2010/libovr.lib
# Lib/x64/VS2012/libovr64d.lib
# Lib/x64/VS2012/libovr64.lib
# Lib/x64/VS2013/libovr64d.lib
# Lib/x64/VS2013/libovr64.lib
# Lib/x64/VS2010/libovr64d.lib
# Lib/x64/VS2010/libovr64.lib
# Lib/Linux/Release/x86_64/libovr.a
# Lib/Linux/Debug/x86_64/libovr.a
# Lib/Linux/Release/i386/libovr.a
# Lib/Linux/Debug/i386/libovr.a
# Lib/Mac/Release/libovr.a
# Lib/Mac/Debug/libovr.a

set(OVRKERNEL_LIBRARY_PATH_SUFFIX "Lib")

# Test compiler
if(MSVC10) # Microsoft Visual Studio 2010
	set(_ovrkernel_library_compiler "VS2010")
elseif(MSVC11) # Microsoft Visual Studio 2012
	set(_ovrkernel_library_compiler "VS2012")
elseif(MSVC12) # Microsoft Visual Studio 2013
	set(_ovrkernel_library_compiler "VS2013")
endif()


# Test 32/64 bits
if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
	set(_ovrkernel_library_arch "x86_64")
	if (WIN32)
		set(_ovrkernel_library_arch "x64")
		set(_ovrkernel_libname_bitsuffix "64")
	endif(WIN32)
else()
	set(_ovrkernel_library_arch "i386")
	if (WIN32)
		set(_ovrkernel_library_arch "Win32")
		set(_ovrkernel_libname_bitsuffix "")
	endif(WIN32)
endif()

# Test platform
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	set(OVRKERNEL_LIBRARY_PATH_SUFFIX_START "Lib/Linux") # needs build type and arch
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	set(OVRKERNEL_LIBRARY_PATH_SUFFIX_START "Lib/Mac") # needs build type
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	set(OVRKERNEL_LIBRARY_PATH_SUFFIX "Lib/${_ovrkernel_library_arch}/${_ovrkernel_library_compiler}")
endif()

find_library(OVRKERNEL_LIBRARY_RELEASE
	NAMES
	ovr${_ovr_libname_bitsuffix}
	LibOVRKernel${_ovr_libname_bitsuffix}
	LibOVRKernel
	PATHS
	"${OVR_ROOT_DIR}"
	"${OVR_ROOT_DIR}/LibOVRKernel"
	c:/tools/oculus-sdk.install/OculusSDK/LibOVRKernel
	${LIB_DIR}/OculusSDK_0.5/OculusSDK/LibOVRKernel/Lib
	${LIB_DIR}/OculusSDK_0.5/OculusSDK/LibOVRKernel
	${LIB_DIR}/OculusSDK_0.5/OculusSDK/LibOVRKernel/Lib/Windows/x64/Release/VS2010
	PATH_SUFFIXES
	${OVRKERNEL_LIBRARY_PATH_SUFFIX}
	${OVRKERNEL_LIBRARY_PATH_SUFFIX_START}/Release
	${OVRKERNEL_LIBRARY_PATH_SUFFIX_START}/Release/${_ovrkernel_library_arch}
	)

find_library(OVRKERNEL_LIBRARY_DEBUG
	NAMES
	ovr${_ovr_libname_bitsuffix}d
	LibOVRKernel${_ovr_libname_bitsuffix}d
	LibOVRKernel
	PATHS
	"${OVR_ROOT_DIR}"
	"${OVR_ROOT_DIR}/LibOVRKernel"
	${LIB_DIR}/OculusSDK_0.5/OculusSDK/LibOVRKernel/Lib
	c:/tools/oculus-sdk.install/OculusSDK/LibOVRKernel
	${LIB_DIR}/OculusSDK_0.5/OculusSDK/LibOVRKernel
	${LIB_DIR}/OculusSDK_0.5/OculusSDK/LibOVRKernel/Lib/Windows/x64/Debug/VS2010
	PATH_SUFFIXES
	${OVRKERNEL_LIBRARY_PATH_SUFFIX}
	${OVRKERNEL_LIBRARY_PATH_SUFFIX_START}/Debug
	${OVRKERNEL_LIBRARY_PATH_SUFFIX_START}/Debug/${_ovrkernel_library_arch})

include(SelectLibraryConfigurations)
select_library_configurations(OVR)

if(OVRKERNEL_LIBRARY_RELEASE)
	get_filename_component(_libdir "${OVRKERNEL_LIBRARY_RELEASE}" PATH)
endif()

find_path(OVRKERNEL_INCLUDE_DIR
	NAMES
	OVR_Array.h
	HINTS
	"${_libdir}"
	"${_libdir}/.."
	"${_libdir}/../.."
	"${_libdir}/../../.."
	${LIB_DIR}/OculusSDK_0.5/OculusSDK/LibOVRKernel/src/kernel
	PATHS
	"${OVR_ROOT_DIR}"
	${LIB_DIR}/OculusSDK_0.5/OculusSDK/LibOVRKernel/src/kernel
	PATH_SUFFIXES
	include
	Include
	)

find_path(OVRKERNEL_SOURCE_DIR
	NAMES
	OVR_File.h
	HINTS
	"${_libdir}"
	"${_libdir}/.."
	"${_libdir}/../.."
	"${_libdir}/../../.."
	${LIB_DIR}/OculusSDK_0.5/OculusSDK/LibOVRKernel/src/kernel
	PATHS
	${LIB_DIR}/OculusSDK_0.5/OculusSDK/LibOVRKernel/src/kernel
	"${OVR_ROOT_DIR}"
	PATH_SUFFIXES
	Src
	)

# Dependencies

set(_ovr_dependency_libraries "")
set(_ovr_dependency_includes "")

if(NOT OPENGL_FOUND)
	find_package(OpenGL)
	list(APPEND _ovr_dependency_libraries ${OPENGL_LIBRARIES})
	list(APPEND _ovr_dependency_includes ${OPENGL_INCLUDE_DIR})
	list(APPEND _ovr_dependencies OPENGL_FOUND)
endif()

if(NOT THREADS_FOUND)
	find_package(Threads)
	list(APPEND _ovr_dependency_libraries ${CMAKE_THREAD_LIBS_INIT})
	list(APPEND _ovr_dependencies THREADS_FOUND)
endif()

# Linux-only dependencies
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	if(NOT X11_FOUND)
		find_package(X11)
		list(APPEND _ovr_dependency_libraries ${X11_LIBRARIES})
		list(APPEND _ovr_dependency_includes ${X11_INCLUDE_DIR})
		list(APPEND _ovr_dependencies X11_FOUND)
	endif()

	if(NOT XRANDR_FOUND)
		find_package(Xrandr)
		list(APPEND _ovr_dependency_libraries ${XRANDR_LIBRARIES})
		list(APPEND _ovr_dependency_includes ${XRANDR_INCLUDE_DIR})
		list(APPEND _ovr_dependencies XRANDR_FOUND)
	endif()

	if(NOT UDEV_FOUND)
		find_package(udev)
		list(APPEND _ovr_dependency_libraries ${UDEV_LIBRARIES})
		list(APPEND _ovr_dependency_includes ${UDEV_INCLUDE_DIR})
		list(APPEND _ovr_dependencies UDEV_FOUND)
	endif()
endif()

if(WIN32)
	#find_library(OVR_WINMM_LIBRARY winmm)
	#find_library(OVR_WS2_LIBRARY ws2_32)
	list(APPEND _ovr_dependency_libraries winmm ws2_32)#${OVR_WINMM_LIBRARY} ${OVR_WS2_LIBRARY})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OVR
	DEFAULT_MSG
	OVRKERNEL_LIBRARY
	OVRKERNEL_INCLUDE_DIR
	OVRKERNEL_SOURCE_DIR
	${_ovr_dependencies}
	)

if(OVR_FOUND)
	#set(OVRKERNEL_LIBRARIES ${OVRKERNEL_LIBRARY} ${_ovr_dependency_libraries})
	#set(OVRKERNEL_INCLUDE_DIRS ${OVRKERNEL_INCLUDE_DIR} ${OVRKERNEL_SOURCE_DIR} ${_ovr_dependency_includes})
	set(OVRKERNEL_LIBRARIES ${OVRKERNEL_LIBRARY_RELEASE})
	set(OVRKERNEL_INCLUDE_DIRS ${OVRKERNEL_INCLUDE_DIR})
	mark_as_advanced(OVR_ROOT_DIR)
endif()

MESSAGE("OVRKERNEL_LIBRARIES is ${OVRKERNEL_LIBRARY_RELEASE}")
MESSAGE("OVRKERNEL_INCLUDE_DIRS is ${OVRKERNEL_INCLUDE_DIR}")
mark_as_advanced(OVRKERNEL_INCLUDE_DIR
	OVRKERNEL_SOURCE_DIR
	OVRKERNEL_LIBRARY
	OVR_WINMM_LIBRARY
	OVR_WS2_LIBRARY)
