# Distributed under the MIT License (See accompanying file /LICENSE)
#[=======================================================================[.rst:
Find phmap
-------

Finds the phmap library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``parallel_hashmap``
  The parallel_hashmap library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``PHMAP_FOUND``
  True if the system has the phmap library.
``PHMAP_VERSION``
  The version of the phmap library which was found.
``PHMAP_INCLUDE_DIRS``
  Include directories needed to use phmap.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``PHMAP_INCLUDE_DIR``
  The directory containing ``parallel_hashmap/phmap_config.h``.

#]=======================================================================]
find_package(PkgConfig)
pkg_check_modules(PC_phmap QUIET phmap)

find_path(PHMAP_INCLUDE_DIR
    NAMES parallel_hashmap/phmap_config.h
    PATHS
        ${PC_PHMAP_INCLUDE_DIRS}
        ${CMAKE_INSTALL_PREFIX}/include
        ${PROJECT_SOURCE_DIR}/external
    PATH_SUFFIXES 
        parallel_hashmap
)

macro(_phmap_get_version)
    file(READ "${PHMAP_INCLUDE_DIR}/parallel_hashmap/phmap_config.h" _header)
    string(CONCAT VERSION_REGEX
        "#define[ \t]+PHMAP_VERSION_MAJOR[ \t]+([0-9]+)[ \n]+"
        "#define[ \t]+PHMAP_VERSION_MINOR[ \t]+([0-9]+)[ \n]+"
        "#define[ \t]+PHMAP_VERSION_PATCH[ \t]+([0-9]+)")
    string(REGEX MATCH ${VERSION_REGEX} _ "${_header}")
    # Use math to skip leading zeros if any.
    math(EXPR PHMAP_VERSION_MAJOR ${CMAKE_MATCH_1})
    math(EXPR PHMAP_VERSION_MINOR ${CMAKE_MATCH_2})
    math(EXPR PHMAP_VERSION_PATCH ${CMAKE_MATCH_3})
    string(JOIN "." PHMAP_VERSION ${PHMAP_VERSION_MAJOR}
                                  ${PHMAP_VERSION_MINOR}
                                  ${PHMAP_VERSION_PATCH})
endmacro()

set(PHMAP_VERSION ${PC_PHMAP_VERSION})
if (NOT PHMAP_VERSION)
    _phmap_get_version()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(phmap
    FOUND_VAR PHMAP_FOUND
    REQUIRED_VARS
        PHMAP_INCLUDE_DIR
    VERSION_VAR PHMAP_VERSION
)

if(PHMAP_FOUND)
    set(PHMAP_INCLUDE_DIRS ${PHMAP_INCLUDE_DIR})
    set(PHMAP_DEFINITIONS ${PC_PHMAP_CFLAGS_OTHER})
endif()

if(PHMAP_FOUND AND NOT TARGET phmap)
    add_library(phmap INTERFACE IMPORTED)
    set_target_properties(phmap PROPERTIES
        INTERFACE_COMPILE_OPTIONS "${PC_PHMAP_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${PHMAP_INCLUDE_DIR}"
    )
endif()

mark_as_advanced(PHMAP_INCLUDE_DIR)
