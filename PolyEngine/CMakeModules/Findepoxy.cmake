if (NOT WIN32)
	find_package(PkgConfig)
	pkg_check_modules(PKG_epoxy QUIET epoxy)

	set(epoxy_DEFINITIONS ${PKG_epoxy_CFLAGS})
endif()

find_path(epoxy_INCLUDE_DIR NAMES epoxy/gl.h  HINTS ${PKG_epoxy_INCLUDEDIR} ${PKG_epoxy_INCLUDE_DIRS})
find_library(epoxy_LIBRARY  NAMES epoxy       HINTS ${PKG_epoxy_LIBDIR} ${PKG_epoxy_LIBRARY_DIRS})
find_file(epoxy_GLX_HEADER  NAMES epoxy/glx.h HINTS ${epoxy_INCLUDE_DIR})

if (epoxy_GLX_HEADER STREQUAL "epoxy_GLX_HEADER-NOTFOUND")
	set(epoxy_HAS_GLX FALSE CACHE BOOL "whether epoxy GLX support is available")
else ()
	set(epoxy_HAS_GLX TRUE  CACHE BOOL "whether epoxy GLX support is available")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(epoxy REQUIRED_VARS epoxy_LIBRARY epoxy_INCLUDE_DIR)

if (NOT TARGET epoxy::gl)
	add_library(epoxy::gl UNKNOWN IMPORTED)
	set_target_properties(epoxy::gl PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${epoxy_INCLUDE_DIR}" IMPORTED_LOCATION "${epoxy_LIBRARY}")
endif()

mark_as_advanced(epoxy_INCLUDE_DIR epoxy_LIBRARY epoxy_HAS_GLX)
