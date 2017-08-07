if (NOT WIN32)
	find_package(PkgConfig)
	pkg_check_modules(PKG_epoxy QUIET epoxy)

	set(epoxy_DEFINITIONS ${PKG_epoxy_CFLAGS})

	find_path(epoxy_INCLUDE_DIR NAMES epoxy/gl.h HINTS ${PKG_epoxy_INCLUDEDIR} ${PKG_epoxy_INCLUDE_DIRS})
	find_library(epoxy_LIBRARY  NAMES epoxy      HINTS ${PKG_epoxy_LIBDIR} ${PKG_epoxy_LIBRARY_DIRS})
	find_file(epoxy_GLX_HEADER NAMES epoxy/glx.h HINTS ${epoxy_INCLUDE_DIR})

	if (epoxy_GLX_HEADER STREQUAL "epoxy_GLX_HEADER-NOTFOUND")
		set(epoxy_HAS_GLX FALSE CACHE BOOL "whether glx is available")
	else ()
		set(epoxy_HAS_GLX TRUE CACHE BOOL "whether glx is available")
	endif()

	include(FindPackageHandleStandardArgs)
	find_package_handle_standard_args(epoxy DEFAULT_MSG epoxy_LIBRARY epoxy_INCLUDE_DIR)

	mark_as_advanced(epoxy_INCLUDE_DIR epoxy_LIBRARY epoxy_HAS_GLX)
endif()
