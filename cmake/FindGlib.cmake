  find_path(GLIB_INCLUDE_DIR  NAMES glib.h PATH_SUFFIXES glib-2.0
    PATHS
    ${GLIB_PKG_INCLUDE_DIRS}
    /usr/include/glib-2.0
    /usr/include
    /usr/local/include
	${LIB_DIR}/glib
	${LIB_DIR}/glib/include/glib-2.0
    )
  find_path(GLIB_CONFIG_INCLUDE_DIR NAMES glibconfig.h PATHS ${LIB_DIR}/glib/lib/x64/glib-2.0/include ${LIB_DIR}/glib/lib/x64 PATH_SUFFIXES glib-2.0/include)
  find_library(GLIB_LIBRARIES2 NAMES glib-2.0
    PATHS
    ${GLIB_PKG_LIBRARY_DIRS}
    /usr/lib
    /usr/local/lib
	${LIB_DIR}/glib/lib/x64
    )

  find_library(GOBJECT_LIBRARIES NAMES gobject-2.0
    PATHS
    ${GLIB_PKG_LIBRARY_DIRS}
    /usr/lib
    /usr/local/lib
	${LIB_DIR}/glib/lib/x64
    )

  find_library(GTHREAD_LIBRARIES NAMES gthread-2.0
    PATHS
    ${GLIB_PKG_LIBRARY_DIRS}
    /usr/lib
    /usr/local/lib
	${LIB_DIR}/glib/lib/x64
    )

  set(GLIB_LIBRARIES ${GLIB_LIBRARIES2} ${GOBJECT_LIBRARIES} ${GTHREAD_LIBRARIES}) 

if (GLIB_INCLUDE_DIR AND GLIB_CONFIG_INCLUDE_DIR AND GLIB_LIBRARIES)
  set(GLIB_INCLUDE_DIRS ${GLIB_INCLUDE_DIR} ${GLIB_CONFIG_INCLUDE_DIR})
endif (GLIB_INCLUDE_DIR AND GLIB_CONFIG_INCLUDE_DIR AND GLIB_LIBRARIES)

if(GLIB_INCLUDE_DIRS AND GLIB_LIBRARIES)
  set(GLIB_FOUND TRUE CACHE INTERNAL "glib-2.0 found")
  message(STATUS "Found glib-2.0: ${GLIB_INCLUDE_DIR}, ${GLIB_LIBRARIES}")
else(GLIB_INCLUDE_DIRS AND GLIB_LIBRARIES)
  set(GLIB_FOUND FALSE CACHE INTERNAL "glib-2.0 found")
  message(STATUS "glib-2.0 not found.")
endif(GLIB_INCLUDE_DIRS AND GLIB_LIBRARIES)

mark_as_advanced(GLIB_INCLUDE_DIR GLIB_CONFIG_INCLUDE_DIR GLIB_INCLUDE_DIRS GLIB_LIBRARIES)