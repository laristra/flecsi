#------------------------------------------------------------------------------#
#  @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
# /@@/////  /@@          @@////@@ @@////// /@@
# /@@       /@@  @@@@@  @@    // /@@       /@@
# /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
# /@@////   /@@/@@@@@@@/@@       ////////@@/@@
# /@@       /@@/@@//// //@@    @@       /@@/@@
# /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
# //       ///  //////   //////  ////////  //
#
# Copyright (c) 2016 Los Alamos National Laboratory, LLC
# All rights reserved
#------------------------------------------------------------------------------#

#------------------------------------------------------------------------------#
# Set the minimum Cinch version
#------------------------------------------------------------------------------#

cinch_minimum_required(VERSION v1.0)

#------------------------------------------------------------------------------#
# Set the project name
#------------------------------------------------------------------------------#

project(FleCSI LANGUAGES CXX C)

#------------------------------------------------------------------------------#
# Boost
#------------------------------------------------------------------------------#

set(ENABLE_BOOST ON CACHE BOOL "Enable Boost (required by FleCSI)" FORCE)
mark_as_advanced(ENABLE_BOOST)
list(APPEND FLECSI_LIBRARY_DEPENDENCIES ${Boost_LIBRARIES})

#------------------------------------------------------------------------------#
# Set the FleCSI top-level source directory
#------------------------------------------------------------------------------#

set(FLECSI_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

#------------------------------------------------------------------------------#
# Set header suffix regular expression
#------------------------------------------------------------------------------#

set(CINCH_HEADER_SUFFIXES "\\.h")

#------------------------------------------------------------------------------#
# Set required C++ standard
#------------------------------------------------------------------------------#

set(CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)

#------------------------------------------------------------------------------#
# These variables are used to collect library and include dependencies
# for the FleCSIConfig file below.
#------------------------------------------------------------------------------#

set(FLECSI_INCLUDE_DEPENDENCIES)
set(FLECSI_LIBRARY_DEPENDENCIES)

#------------------------------------------------------------------------------#
# Add options for runtime selection
#------------------------------------------------------------------------------#

set(FLECSI_RUNTIME_MODELS legion mpi hpx)

if(NOT FLECSI_RUNTIME_MODEL)
  list(GET FLECSI_RUNTIME_MODELS 0 FLECSI_RUNTIME_MODEL)
endif()

set(FLECSI_RUNTIME_MODEL "${FLECSI_RUNTIME_MODEL}" CACHE STRING
  "Select the runtime model")
set_property(CACHE FLECSI_RUNTIME_MODEL
  PROPERTY STRINGS ${FLECSI_RUNTIME_MODELS})

#------------------------------------------------------------------------------#
# cinch_load_extras will try and find legion and mpi. If we want to
# override the defaults, i.e. ENABLE_MPI=on and ENABLE_LEGION=on, we
# need to do it before cinch_load_extras is called.
#------------------------------------------------------------------------------#

if(FLECSI_RUNTIME_MODEL STREQUAL "mpi")
  set(ENABLE_MPI ON CACHE BOOL "Enable MPI" FORCE)
  set(ENABLE_LEGION OFF CACHE BOOL "Enable Legion" FORCE)
elseif(FLECSI_RUNTIME_MODEL STREQUAL "legion")
  set(ENABLE_MPI ON CACHE BOOL "Enable MPI" FORCE)
  set(ENABLE_LEGION ON CACHE BOOL "Enable Legion" FORCE)
elseif(FLECSI_RUNTIME_MODEL STREQUAL "hpx")
  set(ENABLE_MPI ON CACHE BOOL "Enable MPI" FORCE)
  set(ENABLE_HPX ON CACHE BOOL "Enable HPX" FORCE)
endif()

mark_as_advanced(ENABLE_MPI ENABLE_LEGION)

#------------------------------------------------------------------------------#
# Load the cinch extras
#------------------------------------------------------------------------------#

cinch_load_extras(MPI LEGION HPX)

#------------------------------------------------------------------------------#
# FLOG and FTEST
#------------------------------------------------------------------------------#

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/config)
include(flog)
include(ftest)

#------------------------------------------------------------------------------#
# Capture settings from build system
#------------------------------------------------------------------------------#

# After we load the cinch options, we need to capture the configuration
# state for the particular Cinch build configuration and set variables that
# are local to this project. FleCSI should never directly use the raw
# options, e.g., ENABLE_OPTION should be captured as FLECSI_ENABLE_OPTION
# and used as such in the code. This will handle collisions between nested
# projects that use Cinch.

# ENABLE options from Cinch
get_cmake_property(_variableNames VARIABLES)
string (REGEX MATCHALL "(^|;)ENABLE_[A-Za-z0-9_]*"
  _matchedVars "${_variableNames}")

foreach(_variableName ${_matchedVars})
  set(FLECSI_${_variableName} ${${_variableName}})
endforeach()

# CLOG options from Cinch
get_cmake_property(_variableNames VARIABLES)
string (REGEX MATCHALL "(^|;)CLOG_[A-Za-z0-9_]*"
  _matchedVars "${_variableNames}")

foreach(_variableName ${_matchedVars})
  set(FLECSI_${_variableName} ${${_variableName}})
endforeach()

#------------------------------------------------------------------------------#
# Add option for setting id bits
#------------------------------------------------------------------------------#

set(FLECSI_ID_PBITS "20" CACHE STRING
  "Select the number of bits to use for partition ids. There will be 62-FLECSI_ID_PBITS-FLECSI_ID_FBITS available for entity ids")
mark_as_advanced(FLECSI_ID_PBITS)

set(FLECSI_ID_FBITS "4" CACHE STRING
  "Select the number of bits to use for id flags. There will be 62-FLECSI_ID_PBITS-FLECSI_ID_FBITS available for entity ids")
mark_as_advanced(FLECSI_ID_FBITS)

#------------------------------------------------------------------------------#
# Add option for counter size
#------------------------------------------------------------------------------#

set(FLECSI_COUNTER_TYPE "int32_t" CACHE STRING
  "Select the type that will be used for loop and iterator values")
mark_as_advanced(FLECSI_COUNTER_TYPE)

#------------------------------------------------------------------------------#
# Add option for FleCSIT command-line tool.
#------------------------------------------------------------------------------#

option(ENABLE_FLECSIT "Enable FleCSIT Command-Line Tool" ON)

#------------------------------------------------------------------------------#
# Globally shared variables
#------------------------------------------------------------------------------#

set(FLECSI_SHARE_DIR ${CMAKE_INSTALL_PREFIX}/share/FleCSI)

#------------------------------------------------------------------------------#
# Graphviz
#------------------------------------------------------------------------------#

if(ENABLE_GRAPHVIZ)
  list(APPEND FLECSI_LIBRARY_DEPENDENCIES ${Graphviz_LIBRARIES})
endif()

#------------------------------------------------------------------------------#
# OpenSSL
#------------------------------------------------------------------------------#

option(ENABLE_OPENSSL "Enable OpenSSL Support" OFF)
mark_as_advanced(ENABLE_OPENSSL)

if(ENABLE_OPENSSL)
  find_package(OpenSSL REQUIRED)

  if(OPENSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})

    list(APPEND FLECSI_INCLUDE_DEPENDENCIES ${OPENSSL_INCLUDE_DIR})
    list(APPEND FLECSI_LIBRARY_DEPENDENCIES ${OPENSSL_LIBRARIES})
  endif()
endif()

#------------------------------------------------------------------------------#
# Caliper
#------------------------------------------------------------------------------#

if(ENABLE_CALIPER)
  list(APPEND FLECSI_LIBRARY_DEPENDENCIES ${Caliper_LIBRARIES})
endif()

#------------------------------------------------------------------------------#
# Runtime models
#------------------------------------------------------------------------------#

set(FLECSI_RUNTIME_LIBRARIES)

set(DL_LIBS)
foreach(dl_lib ${CMAKE_DL_LIBS})
  find_library(DL_LIB ${dl_lib})

  if(DL_LIB STREQUAL "NOTFOUND")
    message(FATAL_ERROR "Dynamic library not found")
  endif()

  list(APPEND DL_LIBS ${DL_LIB})
endforeach()

mark_as_advanced(DL_LIB)

#
# Legion interface
#
if(FLECSI_RUNTIME_MODEL STREQUAL "legion")

  if(NOT MPI_${MPI_LANGUAGE}_FOUND)
    message (FATAL_ERROR "MPI is required for the legion runtime model")
  endif()

  if(NOT Legion_FOUND)
    message (FATAL_ERROR "Legion is required for the legion runtime model")
  endif()

  set(_runtime_path ${PROJECT_SOURCE_DIR}/flecsi/execution/legion)

  set(FLECSI_RUNTIME_LIBRARIES ${DL_LIBS} ${Legion_LIBRARIES}
    ${MPI_LIBRARIES})

  include_directories(${Legion_INCLUDE_DIRS})
  list(APPEND FLECSI_INCLUDE_DEPENDENCIES ${Legion_INCLUDE_DIRS})

  #
  # Compacted storage interface
  #
  option(ENABLE_MAPPER_COMPACTION "Enable Legion Mapper compaction" ON)
  mark_as_advanced(ENABLE_MAPPER_COMPACTION)

#
# MPI interface
#
elseif(FLECSI_RUNTIME_MODEL STREQUAL "mpi")

  if(NOT MPI_${MPI_LANGUAGE}_FOUND)
    message (FATAL_ERROR "MPI is required for the mpi runtime model")
  endif()

  set(_runtime_path ${PROJECT_SOURCE_DIR}/flecsi/execution/mpi)

  set(FLECSI_RUNTIME_LIBRARIES ${DL_LIBS} ${MPI_LIBRARIES})

elseif(FLECSI_RUNTIME_MODEL STREQUAL "hpx")

  if(NOT HPX_FOUND)
    message (FATAL_ERROR "HPX is required for the HPX runtime model")
  endif()

  if(NOT MPI_${MPI_LANGUAGE}_FOUND)
    message (FATAL_ERROR "MPI is required for the hpx runtime model")
  endif()

   set(FLECSI_RUNTIME_LIBRARIES ${DL_LIBS} ${MPI_LIBRARIES})

  set(_runtime_path ${PROJECT_SOURCE_DIR}/flecsi/execution/hpx)

#
# Default
#
else()

  message(FATAL_ERROR "Unrecognized runtime selection")

endif()

list(APPEND FLECSI_LIBRARY_DEPENDENCIES ${FLECSI_RUNTIME_LIBRARIES})

#------------------------------------------------------------------------------#
# Enable partitioning with METIS
#------------------------------------------------------------------------------#

find_package(METIS 5.1)

if(ENABLE_MPI)
  # Counter-intuitive variable: set to TRUE to disable test
  set(PARMETIS_TEST_RUNS TRUE)
  find_package(ParMETIS 4.0)
endif()

set(COLORING_LIBRARIES)

if(METIS_FOUND)
  list(APPEND COLORING_LIBRARIES ${METIS_LIBRARIES})
  include_directories(${METIS_INCLUDE_DIRS})
  set(ENABLE_METIS TRUE)

  list(APPEND FLECSI_INCLUDE_DEPENDENCIES ${METIS_INCLUDE_DIRS})
endif()

if(PARMETIS_FOUND)
  list(APPEND COLORING_LIBRARIES ${PARMETIS_LIBRARIES})
  include_directories(${PARMETIS_INCLUDE_DIRS})
  set(ENABLE_PARMETIS TRUE)

  list(APPEND FLECSI_INCLUDE_DEPENDENCIES ${PARMETIS_INCLUDE_DIRS})
endif()

if(NOT COLORING_LIBRARIES)
  MESSAGE(FATAL_ERROR
    "You need parmetis to enable partitioning" )
endif()

list(APPEND FLECSI_LIBRARY_DEPENDENCIES ${COLORING_LIBRARIES})

#------------------------------------------------------------------------------#
# Process id bits
#------------------------------------------------------------------------------#

# PBITS: possible number of distributed-memory partitions
# EBITS: possible number of entities per partition
# GBITS: possible number of global ids
# FBITS: flag bits
# dimension: dimension 2 bits
# domain: dimension 2 bits

# Make sure that the user set FBITS to an even number
math(EXPR FLECSI_EVEN_FBITS "${FLECSI_ID_FBITS} % 2")
if(NOT ${FLECSI_EVEN_FBITS} EQUAL 0)
  message(FATAL_ERROR "FLECSI_ID_FBITS must be an even number")
endif()

# Get the total number of bits left for ids
math(EXPR FLECSI_ID_BITS "124 - ${FLECSI_ID_FBITS}")

# Global ids use half of the remaining bits
math(EXPR FLECSI_ID_GBITS "${FLECSI_ID_BITS}/2")

# EBITS and PBITS must add up to GBITS
math(EXPR FLECSI_ID_EBITS "${FLECSI_ID_GBITS} - ${FLECSI_ID_PBITS}")

math(EXPR flecsi_partitions "1 << ${FLECSI_ID_PBITS}")
math(EXPR flecsi_entities "1 << ${FLECSI_ID_EBITS}")

message(STATUS "${CINCH_Yellow}Set id_t bits to allow:\n"
  "   ${flecsi_partitions} partitions with 2^${FLECSI_ID_EBITS} entities each\n"
  "   ${FLECSI_ID_FBITS} flag bits\n"
  "   ${FLECSI_ID_GBITS} global bits (PBITS*EBITS)${CINCH_ColorReset}")

#------------------------------------------------------------------------------#
# configure header
#------------------------------------------------------------------------------#

set(FLECSI_ENABLE_MPI ${ENABLE_MPI})
set(FLECSI_ENABLE_LEGION ${ENABLE_LEGION})
set(FLECSI_ENABLE_METIS ENABLE_METIS)
set(FLECSI_ENABLE_PARMETIS ENABLE_PARMETIS)
set(FLECSI_ENABLE_GRAPHVIZ ${ENABLE_GRAPHVIZ})

configure_file(${PROJECT_SOURCE_DIR}/config/flecsi-config.h.in
  ${CMAKE_BINARY_DIR}/flecsi-config.h @ONLY)

include_directories(${CMAKE_BINARY_DIR})

install(
  FILES ${CMAKE_BINARY_DIR}/flecsi-config.h
  DESTINATION include
)

#------------------------------------------------------------------------------#
# Add IO-POC subdirectory
#------------------------------------------------------------------------------#

#add_subdirectory(io-poc)

#------------------------------------------------------------------------------#
# Add library targets
#------------------------------------------------------------------------------#

cinch_add_library_target(FleCSI flecsi EXPORT_TARGET FleCSITargets)

if(FLECSI_RUNTIME_MODEL STREQUAL "hpx")
  option(ENABLE_FLECSI_TUTORIAL
    "Enable library support for the FleCSI tutorial" OFF)
else()
  option(ENABLE_FLECSI_TUTORIAL
    "Enable library support for the FleCSI tutorial" ON)
endif()

#if(ENABLE_FLECSI_TUTORIAL)
#  option(ENABLE_FLECSI_TUTORIAL_VTK "Enable VTK output for tutorial examples"
#    OFF)
#
#  set(FLECSI_TUTORIAL_ENABLE_VTK)
#
#  if(ENABLE_FLECSI_TUTORIAL_VTK)
#    set(FLECSI_TUTORIAL_ENABLE_VTK ":ENABLE_VTK")
#  endif()
#
#  cinch_add_library_target(FleCSI-Tut flecsi-tutorial/specialization)
#endif()

#------------------------------------------------------------------------------#
# Install Tutorial inputs
#------------------------------------------------------------------------------#

install(
  FILES ${PROJECT_SOURCE_DIR}/flecsi-tutorial/specialization/inputs/simple2d-16x16.msh
  DESTINATION share/FleCSI-Tut/inputs)

#------------------------------------------------------------------------------#
# Link the necessary libraries
#------------------------------------------------------------------------------#

if(FLECSI_RUNTIME_LIBRARIES OR COLORING_LIBRARIES)
  cinch_target_link_libraries(
    FleCSI ${FLECSI_RUNTIME_LIBRARIES} ${COLORING_LIBRARIES}
  )
endif()

if(FLECSI_RUNTIME_MODEL STREQUAL "hpx")

  hpx_setup_target(FleCSI NONAMEPREFIX)

endif()

#------------------------------------------------------------------------------#
# Set application directory
#------------------------------------------------------------------------------#

#cinch_add_application_directory("tools")

#------------------------------------------------------------------------------#
# Prepare variables for FleCSIConfig file.
#------------------------------------------------------------------------------#

set(FLECSI_EXTERNAL_INCLUDE_DIRS)

get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  PROPERTY INCLUDE_DIRECTORIES)

foreach(dir ${dirs})
  if(NOT ${dir} MATCHES ${CMAKE_CURRENT_SOURCE_DIR})
    list(APPEND FLECSI_EXTERNAL_INCLUDE_DIRS ${dir})
  endif()
endforeach()

set(FLECSI_LIBRARY_DIR ${CMAKE_INSTALL_PREFIX}/${LIBDIR})
set(FLECSI_INCLUDE_DIRS ${CMAKE_INSTALL_PREFIX}/include
  ${FLECSI_EXTERNAL_INCLUDE_DIRS})

set(FLECSI_CMAKE_DIR ${CMAKE_INSTALL_PREFIX}/${LIBDIR}/cmake/FleCSI)
# FIXME: Refactor
#set(FLECSI_RUNTIME_MAIN ${FLECSI_SHARE_DIR}/runtime/runtime_main.cc)
#set(FLECSI_RUNTIME_DRIVER ${FLECSI_SHARE_DIR}/runtime/runtime_driver.cc)

#------------------------------------------------------------------------------#
# Extract all project options so they can be exported to the
# ProjectConfig.cmake file.
#------------------------------------------------------------------------------#

get_cmake_property(_variableNames VARIABLES)
string (REGEX MATCHALL "(^|;)FLECSI_[A-Za-z0-9_]*"
  _matchedVars "${_variableNames}")

foreach(_variableName ${_matchedVars})
  set(FLECSI_CONFIG_CODE
    "${FLECSI_CONFIG_CODE}\nset(${_variableName} \"${${_variableName}}\")")
endforeach()

#------------------------------------------------------------------------------#
# Export targets and package.
#------------------------------------------------------------------------------#

export(
  TARGETS FleCSI
  FILE ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/FleCSITargets.cmake
)

export(PACKAGE FleCSI)

#------------------------------------------------------------------------------#
# CMake config file: This should be the last thing to happen.
#------------------------------------------------------------------------------#

configure_file(${PROJECT_SOURCE_DIR}/config/FleCSIConfig.cmake.in
  ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/FleCSIConfig.cmake @ONLY)

install(
  FILES ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/FleCSIConfig.cmake
  DESTINATION ${FLECSI_CMAKE_DIR}
)

install(
  EXPORT FleCSITargets
  DESTINATION ${FLECSI_CMAKE_DIR}
  COMPONENT dev
)

#------------------------------------------------------------------------------#
# Output configuration summary.
#------------------------------------------------------------------------------#

macro(summary_header)
  string(APPEND _summary
    "${CINCH_BoldCyan}"
    "\n"
"
#------------------------------------------------------------------------------#
"
    "# FleCSI Configuration Summary"
"
#------------------------------------------------------------------------------#
"
    "\n"
    "${CINCH_ColorReset}"
)
endmacro()

macro(summary_info name info)
  if(NOT ${info} STREQUAL "")
    string(REPLACE " " ";" split ${info})
    list(LENGTH split split_length)
    string(LENGTH ${name} name_length)

    string(APPEND _summary
        "${CINCH_Plain}"
        "  ${name}:"
        "${CINCH_Brown} "
    )

    if(split_length GREATER 1)
      math(EXPR split_minus "${split_length}-1")
      list(GET split ${split_minus} last)
      list(REMOVE_AT split ${split_minus})

      set(fill " ")
      string(LENGTH ${fill} fill_length)
      while(${fill_length} LESS ${name_length})
        string(APPEND fill " ")
        string(LENGTH ${fill} fill_length)
      endwhile()

      string(APPEND _summary "${CINCH_Brown}")
      foreach(entry ${split})
        string(APPEND _summary
          "${entry}\n${fill}    "
          )
      endforeach()
      string(APPEND _summary "${last}${CINCH_ColorReset}\n")
    else()
      string(APPEND _summary
          "${info}"
          "${CINCH_ColorReset}"
          "\n"
      )
    endif()
  endif()
endmacro()

macro(summary_option name state extra)
  string(APPEND _summary
    "${CINCH_Plain}"
    "  ${name}:"
    "${CINCH_ColorReset}"
  )

  if(${state})
    string(APPEND _summary
      "${CINCH_Green}"
      " ${state}"
      "${CINCH_ColorReset}"
      "${extra}"
    )
  else()
    string(APPEND _summary
      "${CINCH_BoldGrey}"
      " ${state}"
      "${CINCH_ColorReset}"
    )
  endif()
  string(APPEND _summary "\n")
endmacro()

summary_header()
summary_info("CMAKE_BUILD_TYPE" "${CMAKE_BUILD_TYPE}")
summary_info("CMAKE_INSTALL_PREFIX" "${CMAKE_INSTALL_PREFIX}")
string(APPEND _summary "\n")
summary_info("CMAKE_CXX_COMPILER" "${CMAKE_CXX_COMPILER}")
summary_info("CMAKE_CXX_FLAGS" "${CMAKE_CXX_FLAGS}")
summary_info("CMAKE_C_COMPILER" "${CMAKE_C_COMPILER}")
summary_info("CMAKE_C_FLAGS" "${CMAKE_C_FLAGS}")
string(APPEND _summary "\n")
summary_info("FLECSI_RUNTIME_MODEL" "${FLECSI_RUNTIME_MODEL}")
summary_option("ENABLE_FLOG" ${ENABLE_FLOG}
  " (FLOG_STRIP_LEVEL ${FLOG_STRIP_LEVEL})")
summary_option("ENABLE_UNIT_TESTS" ${ENABLE_UNIT_TESTS} "")
summary_option("ENABLE_FLECSI_TUTORIAL" ${ENABLE_FLECSI_TUTORIAL} "")
summary_option("ENABLE_FLECSIT" ${ENABLE_FLECSIT} "")
string(APPEND _summary "\n")
summary_option("ENABLE_KOKKOS" ${ENABLE_KOKKOS}
  " (${FLECSI_NODE_PLATFORM} platform)")
summary_option("ENABLE_DOXYGEN" ${ENABLE_DOXYGEN} "")
summary_option("ENABLE_GRAPHVIZ" ${ENABLE_GRAPHVIZ} "")
summary_option("ENABLE_OPENMP" ${ENABLE_OPENMP} "")
summary_option("ENABLE_SPHINX" ${ENABLE_SPHINX} "")

message(STATUS ${_summary})
