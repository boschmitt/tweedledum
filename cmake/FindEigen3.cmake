# Distributed under the MIT License (See accompanying file /LICENSE)
#[=======================================================================[.rst:
FindEigen3
-------

Finds the Eigen3 library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``Eigen3::Eigen3``
  The Eigen3 library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``Eigen3_FOUND``
  True if the system has the Eigen3 library.
``Eigen3_VERSION``
  The version of the Eigen3 library which was found.
``Eigen3_INCLUDE_DIRS``
  Include directories needed to use Eigen3.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``Eigen3_INCLUDE_DIR``
  The directory containing ``signature_of_eigen3_matrix_library``.

#]=======================================================================]
find_package(PkgConfig)
pkg_check_modules(PC_Eigen3 QUIET Eigen3)

find_path(Eigen3_INCLUDE_DIR
    NAMES Eigen/signature_of_eigen3_matrix_library
    PATHS
        ${PC_Eigen3_INCLUDE_DIRS}
        ${CMAKE_INSTALL_PREFIX}/include
        ${PROJECT_SOURCE_DIR}/external
    PATH_SUFFIXES 
        eigen3 eigen
)

macro(_Eigen3_get_version)
    file(READ "${Eigen3_INCLUDE_DIR}/Eigen/src/Core/util/Macros.h" _header)
    string(CONCAT VERSION_REGEX
        "#define[ \t]+EIGEN_WORLD_VERSION[ \t]+([0-9]+)[ \n]+"
        "#define[ \t]+EIGEN_MAJOR_VERSION[ \t]+([0-9]+)[ \n]+"
        "#define[ \t]+EIGEN_MINOR_VERSION[ \t]+([0-9]+)")
    string(REGEX MATCH ${VERSION_REGEX} _ "${_header}")
    # Use math to skip leading zeros if any.
    math(EXPR Eigen3_WORLD_VERSION ${CMAKE_MATCH_1})
    math(EXPR Eigen3_MAJOR_VERSION ${CMAKE_MATCH_2})
    math(EXPR Eigen3_MINOR_VERSION ${CMAKE_MATCH_3})
    string(JOIN "." Eigen3_VERSION ${Eigen3_WORLD_VERSION}
                                   ${Eigen3_MAJOR_VERSION}
                                   ${Eigen3_MINOR_VERSION})
endmacro()

set(Eigen3_VERSION ${PC_Eigen3_VERSION})
if (NOT Eigen3_VERSION)
    _Eigen3_get_version()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Eigen3
    FOUND_VAR Eigen3_FOUND
    REQUIRED_VARS
        Eigen3_INCLUDE_DIR
    VERSION_VAR Eigen3_VERSION
)

if(Eigen3_FOUND)
    set(Eigen3_INCLUDE_DIRS ${Eigen3_INCLUDE_DIR})
    set(Eigen3_DEFINITIONS ${PC_Eigen3_CFLAGS_OTHER})
endif()

if(Eigen3_FOUND AND NOT TARGET Eigen3::Eigen3)
    add_library(Eigen3::Eigen3 INTERFACE IMPORTED)
    set_target_properties(Eigen3::Eigen3 PROPERTIES
        INTERFACE_COMPILE_OPTIONS "${PC_Eigen3_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${Eigen3_INCLUDE_DIR}"
    )
endif()

mark_as_advanced(Eigen3_INCLUDE_DIR)
