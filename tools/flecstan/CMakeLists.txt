# -*- CMake -*-

# ---------------------------------------------------------------------------- #
#  @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
# /@@/////  /@@          @@////@@ @@////// /@@
# /@@       /@@  @@@@@  @@    // /@@       /@@
# /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
# /@@////   /@@/@@@@@@@/@@       ////////@@/@@
# /@@       /@@/@@//// //@@    @@       /@@/@@
# /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
# //       ///  //////   //////  ////////  //
#
# Copyright (c) 2019, Triad National Security, LLC
# All rights reserved
# ---------------------------------------------------------------------------- #

project(flecstan LANGUAGES CXX)
cmake_minimum_required(VERSION 3.8)

# Version
# fixme Reformulate for newer cmake
set(flecstan_VERSION_MAJOR 0)
set(flecstan_VERSION_MINOR 0)
set(flecstan_VERSION_PATCH 0)
configure_file (
  "${PROJECT_SOURCE_DIR}/src/flecstan-config.h.in"
  "${PROJECT_BINARY_DIR}/flecstan-config.h"
)

# Use cmake modules from cinch
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../cinch/cmake)

# C++
# fixme Replace hardcoded flags with something better
set(CMAKE_CXX_STANDARD 17)

# compiler flags should be set via command line, but if one must
# override them, make sure they actually work
include(CheckCXXCompilerFlag)

check_cxx_compiler_flag("-pedantic" _pedantic)
if (_pedantic)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic")
endif()

check_cxx_compiler_flag("-Wall" _Wall)
if (_Wall)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
endif()

check_cxx_compiler_flag("-fno-rtti" _fno_rtti)
if (_fno_rtti)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif()

check_cxx_compiler_flag("-s" _s)
if (_s)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s")
endif()

check_cxx_compiler_flag("-O2" _Otwo)
if (_Otwo)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
endif()

# Packages

# need clang/llvm 7
set( _clang_components
    Tooling
    ASTMatchers
    Format
    ToolingInclusions
    Frontend
    Driver
    Parse
    Serialization
    Sema
    Edit
    Analysis
    ToolingCore
    AST
    Rewrite
    Lex
    Basic)


# specifying the versioning in config mode does not work
find_package(Clang CONFIG COMPONENTS ${_clang_components} QUIET)

if (NOT Clang_FOUND)
  # specifying the version here works
  find_package(Clang 7 MODULE REQUIRED
    COMPONENTS ${_clang_components} )
else()
  # not set by config, but set by module mode
  set(CLANG_LIBRARIES)
  foreach(_library ${_clang_components}) 
    list(APPEND CLANG_LIBRARIES clang${_library})
  endforeach()
  # some debug
  MESSAGE( STATUS "Imported cmake config from: ${CLANG_CMAKE_DIR}")
endif()

# Include directories
include_directories(
   ${PROJECT_SOURCE_DIR}/src
   ${PROJECT_BINARY_DIR}
   ${CLANG_INCLUDE_DIRS}
)

# Source files
set(flecstan_SOURCES
   src/flecstan-analysis.cc
   src/flecstan-arg.cc
   src/flecstan-diagnostic.cc
   src/flecstan-main.cc
   src/flecstan-misc.cc
   src/flecstan-prep.cc
   src/flecstan-utility.cc
   src/flecstan-visitor.cc
   src/flecstan-yaml.cc
)

# Executable
add_executable(flecstan ${flecstan_SOURCES})

# Libraries
target_link_libraries(
  flecstan ${CLANG_LIBRARIES}
)
