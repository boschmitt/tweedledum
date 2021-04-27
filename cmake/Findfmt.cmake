# Distributed under the MIT License (See accompanying file /LICENSE)
#[=======================================================================[.rst:
Find fmt
-------

Finds the fmt library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``fmt::fmt``
  The fmt library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``fmt_FOUND``
  True if the system has the fmt library.
``fmt_VERSION``
  The version of the fmt library which was found.
``fmt_INCLUDE_DIRS``
  Include directories needed to use fmt.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``fmt_INCLUDE_DIR``
  The directory containing ``fmt/format.h``.

#]=======================================================================]
find_package(PkgConfig)
pkg_check_modules(PC_fmt QUIET fmt)

find_path(fmt_INCLUDE_DIR
    NAMES fmt/format.h
    PATHS
        ${PC_fmt_INCLUDE_DIRS}
        ${CMAKE_INSTALL_PREFIX}/include
        ${PROJECT_SOURCE_DIR}/external
    PATH_SUFFIXES 
        fmt/include
)

macro(_fmt_get_version)
    # Get version from core.h
    file(READ "${fmt_INCLUDE_DIR}/fmt/core.h" core_h)
    if (NOT core_h MATCHES "FMT_VERSION ([0-9]+)([0-9][0-9])([0-9][0-9])")
        message(FATAL_ERROR "Cannot get FMT_VERSION from core.h.")
    endif ()
    # Use math to skip leading zeros if any.
    math(EXPR fmt_VERSION_MAJOR ${CMAKE_MATCH_1})
    math(EXPR fmt_VERSION_MINOR ${CMAKE_MATCH_2})
    math(EXPR fmt_VERSION_PATCH ${CMAKE_MATCH_3})
    string(JOIN "." fmt_VERSION ${fmt_VERSION_MAJOR}
                                ${fmt_VERSION_MINOR}
                                ${fmt_VERSION_PATCH})
endmacro()

set(fmt_VERSION ${PC_fmt_VERSION})
if (NOT fmt_VERSION)
    _fmt_get_version()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(fmt
    FOUND_VAR fmt_FOUND
    REQUIRED_VARS
        fmt_INCLUDE_DIR
    VERSION_VAR fmt_VERSION
)

if(fmt_FOUND)
    set(fmt_INCLUDE_DIRS ${fmt_INCLUDE_DIR})
    set(fmt_DEFINITIONS ${PC_fmt_CFLAGS_OTHER})
endif()

if(fmt_FOUND AND NOT TARGET fmt::fmt-header-only)
    add_library(fmt::fmt-header-only INTERFACE IMPORTED)
    set_target_properties(fmt::fmt-header-only PROPERTIES
        INTERFACE_COMPILE_OPTIONS "${PC_fmt_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${fmt_INCLUDE_DIR}"
    )
    target_compile_definitions(fmt::fmt-header-only INTERFACE FMT_HEADER_ONLY=1)
endif()

mark_as_advanced(fmt_INCLUDE_DIR)
