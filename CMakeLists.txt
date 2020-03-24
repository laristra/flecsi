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
# Copyright (c) 2016, Triad National Security, LLC
# All rights reserved
#------------------------------------------------------------------------------#

cmake_minimum_required(VERSION 3.12)

#------------------------------------------------------------------------------#
# Add local module path.
#------------------------------------------------------------------------------#

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

#------------------------------------------------------------------------------#
# Project.
#------------------------------------------------------------------------------#

project(FleCSI LANGUAGES CXX C)

set(CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)

#------------------------------------------------------------------------------#
# Version.
#------------------------------------------------------------------------------#

include(version)

set(FleCSI_VERSION ${${PROJECT_NAME}_VERSION})

#------------------------------------------------------------------------------#
# Add format target.
#------------------------------------------------------------------------------#

include(format)

#------------------------------------------------------------------------------#
# Add spotless target
#------------------------------------------------------------------------------#

add_custom_target(spotless rm -rf ${CMAKE_BINARY_DIR}/*)

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
# These variables are used to collect library and include dependencies
# for the FleCSIConfig file below.
#------------------------------------------------------------------------------#

set(FLECSI_INCLUDE_DEPENDENCIES)
set(FLECSI_LIBRARY_DEPENDENCIES)

#------------------------------------------------------------------------------#
# Coverage.
#------------------------------------------------------------------------------#

include(coverage)

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
# Runtime defaults.
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

mark_as_advanced(ENABLE_MPI ENABLE_LEGION ENABLE_HPX)

#------------------------------------------------------------------------------#
# Legion
#------------------------------------------------------------------------------#

if(ENABLE_LEGION)
  include(legion)
endif()

#------------------------------------------------------------------------------#
# MPI
#------------------------------------------------------------------------------#

if(ENABLE_MPI)
  include(mpi)
endif()

#------------------------------------------------------------------------------#
# HPX
#------------------------------------------------------------------------------#

if(ENABLE_HPX)
  include(hpx)
endif()

#------------------------------------------------------------------------------#
# OpenMP.
#------------------------------------------------------------------------------#

include(openmp)

#------------------------------------------------------------------------------#
# HDF5.
#------------------------------------------------------------------------------#

include(hdf5)

#------------------------------------------------------------------------------#
# Documentation.
#------------------------------------------------------------------------------#

include(documentation)

#------------------------------------------------------------------------------#
# Graphviz
#------------------------------------------------------------------------------#

include(graphviz)

#------------------------------------------------------------------------------#
# Boost.
#------------------------------------------------------------------------------#

set(ENABLE_BOOST ON CACHE BOOL "Enable Boost (required by FleCSI)" FORCE)
mark_as_advanced(ENABLE_BOOST)

include(boost)

#------------------------------------------------------------------------------#
# Kokkos
#------------------------------------------------------------------------------#

include(kokkos)

#------------------------------------------------------------------------------#
# Caliper
#------------------------------------------------------------------------------#

include(caliper)

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
  include_directories(SYSTEM ${METIS_INCLUDE_DIRS})
  set(ENABLE_METIS TRUE)

  list(APPEND FLECSI_INCLUDE_DEPENDENCIES ${METIS_INCLUDE_DIRS})
endif()

if(PARMETIS_FOUND)
  list(APPEND COLORING_LIBRARIES ${PARMETIS_LIBRARIES})
  include_directories(SYSTEM ${PARMETIS_INCLUDE_DIRS})
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

message(STATUS "${FLECSI_Yellow}Set id_t bits to allow:\n"
  "   ${flecsi_partitions} partitions with 2^${FLECSI_ID_EBITS} entities each\n"
  "   ${FLECSI_ID_FBITS} flag bits\n"
  "   ${FLECSI_ID_GBITS} global bits (PBITS*EBITS)${FLECSI_ColorReset}")

#------------------------------------------------------------------------------#
# FLOG and FTEST
#------------------------------------------------------------------------------#

include(flog)
include(ftest)

#------------------------------------------------------------------------------#
# Capture enable settings.
#------------------------------------------------------------------------------#

get_cmake_property(_variableNames VARIABLES)
string (REGEX MATCHALL "(^|;)ENABLE_[A-Za-z0-9_]*"
  _matchedVars "${_variableNames}")

foreach(_variableName ${_matchedVars})
  set(FLECSI_${_variableName} ${${_variableName}})
endforeach()

#------------------------------------------------------------------------------#
# Configure header.
#------------------------------------------------------------------------------#

#set(FLECSI_ENABLE_MPI ${ENABLE_MPI})
#set(FLECSI_ENABLE_LEGION ${ENABLE_LEGION})
#set(FLECSI_ENABLE_METIS ENABLE_METIS)
#set(FLECSI_ENABLE_PARMETIS ENABLE_PARMETIS)
#set(FLECSI_ENABLE_GRAPHVIZ ${ENABLE_GRAPHVIZ})

configure_file(${PROJECT_SOURCE_DIR}/config/flecsi-config.h.in
  ${CMAKE_BINARY_DIR}/flecsi-config.h @ONLY)

include_directories(${CMAKE_BINARY_DIR})

install(
  FILES ${CMAKE_BINARY_DIR}/flecsi-config.h
  DESTINATION include
)

#------------------------------------------------------------------------------#
# FleCSI library.
#------------------------------------------------------------------------------#

include(library)

add_library_target(FleCSI flecsi EXPORT_TARGET FleCSITargets)

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

#------------------------------------------------------------------------------#
# Extract all project options so they can be exported to the
# ProjectConfig.cmake file.
#------------------------------------------------------------------------------#

get_cmake_property(_variableNames VARIABLES)
string (REGEX MATCHALL "(^|;)FLECSI_[A-Za-z0-9_]*"
  _matchedVars "${_variableNames}")

foreach(_variableName ${_matchedVars})
  string(REGEX REPLACE "FLECSI" "FleCSI" _variableName ${_variableName})
  set(FLECSI_CONFIG_CODE
    "${FLECSI_CONFIG_CODE}\nset(${_variableName} \"${${_variableName}}\")")
endforeach()

string(REGEX MATCHALL "(^|;)FLOG_[A-Za-z0-9_]*"
  _matchedVars "${_variableNames}")

foreach(_variableName ${_matchedVars})
  set(FLOG_CONFIG_CODE
    "${FLOG_CONFIG_CODE}\nset(${_variableName} \"${${_variableName}}\")")
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
# Link necessary libraries.
#------------------------------------------------------------------------------#

if(FLECSI_RUNTIME_LIBRARIES OR COLORING_LIBRARIES)
  add_target_link_libraries(
    FleCSI ${FLECSI_RUNTIME_LIBRARIES} ${COLORING_LIBRARIES}
  )
endif()

if(FLECSI_RUNTIME_MODEL STREQUAL "hpx")

  hpx_setup_target(FleCSI NONAMEPREFIX)

endif()

#------------------------------------------------------------------------------#
# Tools.
#------------------------------------------------------------------------------#

#add_subdirectory("tools")

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

include(colors)

macro(summary_header)
  string(APPEND _summary
    "${FLECSI_BoldCyan}"
    "\n"
"
#------------------------------------------------------------------------------#
"
    "# FleCSI Configuration Summary"
"
#------------------------------------------------------------------------------#
"
    "\n"
    "${FLECSI_ColorReset}"
)
endmacro()

macro(summary_info name info allow_split)
  if(NOT ${info} STREQUAL "")
    string(REPLACE " " ";" split ${info})
    list(LENGTH split split_length)
    string(LENGTH ${name} name_length)

    string(APPEND _summary
      "${FLECSI_Plain}"
      "  ${name}:"
      "${FLECSI_Brown} "
    )

    if(${allow_split} AND split_length GREATER 1)
      math(EXPR split_minus "${split_length}-1")
      list(GET split ${split_minus} last)
      list(REMOVE_AT split ${split_minus})

      set(fill " ")
      string(LENGTH ${fill} fill_length)
      while(${fill_length} LESS ${name_length})
        string(APPEND fill " ")
        string(LENGTH ${fill} fill_length)
      endwhile()

      string(APPEND _summary "${FLECSI_Brown}")
      foreach(entry ${split})
        string(APPEND _summary
          "${entry}\n${fill}    "
          )
      endforeach()
      string(APPEND _summary "${last}${FLECSI_ColorReset}\n")
    else()
      string(APPEND _summary
        "${info}"
        "${FLECSI_ColorReset}"
        "\n"
      )
    endif()
  endif()
endmacro()

macro(summary_option name state extra)
  string(APPEND _summary
    "${FLECSI_Plain}"
    "  ${name}:"
    "${FLECSI_ColorReset}"
  )

  if(${state})
    string(APPEND _summary
      "${FLECSI_Green}"
      " ${state}"
      "${FLECSI_ColorReset}"
      "${extra}"
    )
  else()
    string(APPEND _summary
      "${FLECSI_BoldGrey}"
      " ${state}"
      "${FLECSI_ColorReset}"
    )
  endif()
  string(APPEND _summary "\n")
endmacro()

summary_header()
summary_info("CMAKE_BUILD_TYPE" "${CMAKE_BUILD_TYPE}" TRUE)
summary_info("CMAKE_INSTALL_PREFIX" "${CMAKE_INSTALL_PREFIX}" TRUE)
string(APPEND _summary "\n")
summary_info("CMAKE_CXX_COMPILER" "${CMAKE_CXX_COMPILER}" TRUE)
summary_info("CMAKE_CXX_FLAGS" "${CMAKE_CXX_FLAGS}" TRUE)
summary_info("CMAKE_C_COMPILER" "${CMAKE_C_COMPILER}" TRUE)
summary_info("CMAKE_C_FLAGS" "${CMAKE_C_FLAGS}" TRUE)
string(APPEND _summary "\n")
summary_info("FleCSI_VERSION" "${FleCSI_VERSION}" TRUE)
summary_info("FleCSI_DOCUMENTATION_VERSION"
  "${FleCSI_DOCUMENTATION_VERSION}" FALSE)
if(${PROJECT_NAME}_COMMITS)
  summary_info("Repository state" "${${PROJECT_NAME}_COMMITS}" FALSE)
endif()
summary_info("FLECSI_RUNTIME_MODEL" "${FLECSI_RUNTIME_MODEL}" TRUE)
summary_option("ENABLE_FLOG" ${ENABLE_FLOG}
  " (FLOG_STRIP_LEVEL ${FLOG_STRIP_LEVEL})")
summary_option("ENABLE_UNIT_TESTS" ${ENABLE_UNIT_TESTS} "")
string(APPEND _summary "\n")
summary_option("ENABLE_KOKKOS" ${ENABLE_KOKKOS}
  " (${FLECSI_NODE_PLATFORM} platform)")
if(ENABLE_DOCUMENTATION)
  summary_option("ENABLE_SPHINX" ${ENABLE_SPHINX} " (ENABLE_DOCUMENTATION)")
  summary_option("ENABLE_DOXYGEN" ${ENABLE_DOXYGEN} " (ENABLE_DOCUMENTATION)")
endif()
summary_option("ENABLE_GRAPHVIZ" ${ENABLE_GRAPHVIZ} "")
summary_option("ENABLE_OPENMP" ${ENABLE_OPENMP} "")
summary_option("ENABLE_HDF5" ${ENABLE_HDF5} "")

message(STATUS ${_summary})
