# Distributed under the MIT License (See accompanying file /LICENSE)
#[=======================================================================[.rst:
Find nlohmann_json
-------

Finds the nlohmann_json library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``nlohmann_json``
  The nlohmann_json library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``nlohmann_json_FOUND``
  True if the system has the nlohmann_json library.
``nlohmann_json_VERSION``
  The version of the nlohmann_json library which was found.
``nlohmann_json_INCLUDE_DIRS``
  Include directories needed to use nlohmann_json.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``nlohmann_json_INCLUDE_DIR``
  The directory containing ``nlohmann/json.h``.

#]=======================================================================]
find_package(PkgConfig)
pkg_check_modules(PC_nlohmann_json QUIET nlohmann_json)

find_path(nlohmann_json_INCLUDE_DIR
    NAMES nlohmann/json.hpp
    PATHS
        ${PC_nlohmann_json_INCLUDE_DIRS}
        ${CMAKE_INSTALL_PREFIX}/include
        ${PROJECT_SOURCE_DIR}/external
    PATH_SUFFIXES 
        nlohmann
    
)

macro(_nlohmann_json_get_version)
    file(READ "${nlohmann_json_INCLUDE_DIR}/nlohmann/json.hpp" _header)
    string(CONCAT VERSION_REGEX
        "#define[ \t]+NLOHMANN_JSON_VERSION_MAJOR[ \t]+([0-9]+)[ \n]+"
        "#define[ \t]+NLOHMANN_JSON_VERSION_MINOR[ \t]+([0-9]+)[ \n]+"
        "#define[ \t]+NLOHMANN_JSON_VERSION_PATCH[ \t]+([0-9]+)")
    string(REGEX MATCH ${VERSION_REGEX} _ "${_header}")
    # Use math to skip leading zeros if any.
    math(EXPR nlohmann_json_VERSION_MAJOR ${CMAKE_MATCH_1})
    math(EXPR nlohmann_json_VERSION_MINOR ${CMAKE_MATCH_2})
    math(EXPR nlohmann_json_VERSION_PATCH ${CMAKE_MATCH_3})
    string(JOIN "." nlohmann_json_VERSION ${nlohmann_json_VERSION_MAJOR}
                                          ${nlohmann_json_VERSION_MINOR}
                                          ${nlohmann_json_VERSION_PATCH})
endmacro()

set(nlohmann_json_VERSION ${PC_nlohmann_json_VERSION})
if (NOT nlohmann_json_VERSION)
    _nlohmann_json_get_version()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(nlohmann_json
    FOUND_VAR nlohmann_json_FOUND
    REQUIRED_VARS
        nlohmann_json_INCLUDE_DIR
    VERSION_VAR nlohmann_json_VERSION
)

if(nlohmann_json_FOUND)
    set(nlohmann_json_INCLUDE_DIRS ${nlohmann_json_INCLUDE_DIR})
    set(nlohmann_json_DEFINITIONS ${PC_nlohmann_json_CFLAGS_OTHER})
endif()

if(nlohmann_json_FOUND AND NOT TARGET nlohmann_json)
    add_library(nlohmann_json INTERFACE IMPORTED)
    set_target_properties(nlohmann_json PROPERTIES
        INTERFACE_COMPILE_OPTIONS "${PC_nlohmann_json_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${nlohmann_json_INCLUDE_DIR}"
    )
endif()

mark_as_advanced(nlohmann_json_INCLUDE_DIR)
