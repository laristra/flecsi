#~----------------------------------------------------------------------------~#
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
#~----------------------------------------------------------------------------~#

#------------------------------------------------------------------------------#
# Set the minimum Cinch version
#------------------------------------------------------------------------------#

cinch_minimum_required(1.0)

#------------------------------------------------------------------------------#
# Set the project name
#------------------------------------------------------------------------------#

option(ENABLE_CUDA "Enable support for CUDA" OFF)

if(ENABLE_CUDA)
  project(FleCSI LANGUAGES CXX C CUDA)
else()
  project(FleCSI LANGUAGES CXX C)
endif()

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
# This variable is used to collect library and include dependencies for
# the FleCSIConfig file below.
#------------------------------------------------------------------------------#

set(FLECSI_INCLUDE_DEPENDENCIES)
set(FLECSI_LIBRARY_DEPENDENCIES)

#------------------------------------------------------------------------------#
# Enable Boost.Preprocessor
#------------------------------------------------------------------------------#

# This changes the Cinch default
set(ENABLE_BOOST_PREPROCESSOR ON CACHE BOOL "Enable Boost.Preprocessor")

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

# After we load the cinch options, we need to capture the configuration
# state for the particular Cinch build configuration and set variables that
# are local to this project. FleCSI should never directly use the raw
# options, e.g., ENABLE_OPTION should be captured as FLECSI_ENABLE_OPTION
# and used as such in the code. This will handle collisions between nested
# projects that use Cinch.

cinch_load_extras(MPI LEGION HPX)

get_cmake_property(_variableNames VARIABLES)
string (REGEX MATCHALL "(^|;)ENABLE_[A-Za-z0-9_]*"
  _matchedVars "${_variableNames}")

foreach(_variableName ${_matchedVars})
  set(FLECSI_${_variableName} ${${_variableName}})
endforeach()

#------------------------------------------------------------------------------#
# Add option for setting id bits
#------------------------------------------------------------------------------#

set(FLECSI_ID_PBITS "20" CACHE STRING
  "Select the number of bits to use for partition ids. There will be 62-FLECSI_ID_PBITS-FLECSI_ID_FBITS available for entity ids")

set(FLECSI_ID_FBITS "4" CACHE STRING
  "Select the number of bits to use for id flags. There will be 62-FLECSI_ID_PBITS-FLECSI_ID_FBITS available for entity ids")

#------------------------------------------------------------------------------#
# Add option for counter size
#------------------------------------------------------------------------------#

set(FLECSI_COUNTER_TYPE "int32_t" CACHE STRING
  "Select the type that will be used for loop and iterator values")

#------------------------------------------------------------------------------#
# Control Model
#------------------------------------------------------------------------------#

option(ENABLE_DYNAMIC_CONTROL_MODEL "Enable the new FleCSI control model" OFF)

#------------------------------------------------------------------------------#
# Add option for FleCSIT command-line tool.
#------------------------------------------------------------------------------#

option(ENABLE_FLECSIT "Enable FleCSIT Command-Line Tool" ON)

#------------------------------------------------------------------------------#
# Globally shared variables
#------------------------------------------------------------------------------#

set(FLECSI_SHARE_DIR ${CMAKE_INSTALL_PREFIX}/share/FleCSI)

#------------------------------------------------------------------------------#
# RistraLL
#------------------------------------------------------------------------------#

option(ENABLE_RISTRALL "Enable Ristra Low-Level Library Support" OFF)

if(ENABLE_RISTRALL)
  find_package(RistraLL REQUIRED)

  if(RistraLL_FOUND)
    include_directories(${RistraLL_INCLUDE_DIRS})

    list(APPEND FLECSI_INCLUDE_DEPENDENCIES ${RistraLL_INCLUDE_DIRS})
    list(APPEND FLECSI_LIBRARY_DEPENDENCIES ${RistraLL_LIBRARIES})
  endif()
endif()

#------------------------------------------------------------------------------#
# Graphviz
#------------------------------------------------------------------------------#

option(ENABLE_GRAPHVIZ "Enable Graphviz Support" OFF)

if(ENABLE_GRAPHVIZ)
  find_package(Graphviz REQUIRED)

  if(GRAPHVIZ_FOUND)
    include_directories(${GRAPHVIZ_INCLUDE_DIRS})
    add_definitions(-DENABLE_GRAPHVIZ)

    list(APPEND FLECSI_INCLUDE_DEPENDENCIES ${GRAPHVIZ_INCLUDE_DIR})
    list(APPEND FLECSI_LIBRARY_DEPENDENCIES ${GRAPHVIZ_LIBRARIES})
  endif()
endif()

#------------------------------------------------------------------------------#
# OpenSSL
#------------------------------------------------------------------------------#

option(ENABLE_OPENSSL "Enable OpenSSL Support" OFF)

if(ENABLE_OPENSSL)
  find_package(OpenSSL REQUIRED)

  if(OPENSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
    add_definitions(-DENABLE_OPENSSL)

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
# Boost Program Options
#------------------------------------------------------------------------------#

if(ENABLE_BOOST_PROGRAM_OPTIONS)
  list(APPEND FLECSI_LIBRARY_DEPENDENCIES ${Boost_LIBRARIES})
endif()

#------------------------------------------------------------------------------#
# Pthreads
#------------------------------------------------------------------------------#

if(ENABLE_CLOG)
  if(CLOG_ENABLE_MPI)
    list(APPEND FLECSI_LIBRARY_DEPENDENCIES ${CMAKE_THREAD_LIBS_INIT})
  endif()
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

  if(ENABLE_MAPPER_COMPACTION)
    add_definitions(-DMAPPER_COMPACTION)
    set (MAPPER_COMPACTION TRUE)
  else()
    option(COMPACTED_STORAGE_SORT "sort compacted storage according to GIS" ON)

    if(COMPACTED_STORAGE_SORT)
      add_definitions(-DCOMPACTED_STORAGE_SORT)
      set(COMPACTED_STORAGE_SORT TRUE)
    endif()
  endif()

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
set(FLECSI_ENABLE_DYNAMIC_CONTROL_MODEL ${ENABLE_DYNAMIC_CONTROL_MODEL})

configure_file(${PROJECT_SOURCE_DIR}/config/flecsi-config.h.in
  ${CMAKE_BINARY_DIR}/flecsi-config.h @ONLY)

include_directories(${CMAKE_BINARY_DIR})

install(
  FILES ${CMAKE_BINARY_DIR}/flecsi-config.h
  DESTINATION include
)

#------------------------------------------------------------------------------#
# Add library targets
#------------------------------------------------------------------------------#

cinch_add_library_target(FleCSI flecsi EXPORT_TARGET FleCSITargets)

set_target_properties(FleCSI PROPERTIES FOLDER "Core")

if(FLECSI_RUNTIME_MODEL STREQUAL "hpx")
  option(ENABLE_FLECSI_TUTORIAL
    "Enable library support for the FleCSI tutorial" OFF)
else()
  option(ENABLE_FLECSI_TUTORIAL
    "Enable library support for the FleCSI tutorial" ON)
endif()

if(ENABLE_FLECSI_TUTORIAL)
  option(ENABLE_FLECSI_TUTORIAL_VTK "Enable VTK output for tutorial examples"
    OFF)

  set(FLECSI_TUTORIAL_ENABLE_VTK)

  if(ENABLE_FLECSI_TUTORIAL_VTK)
    set(FLECSI_TUTORIAL_ENABLE_VTK ":ENABLE_VTK")
  endif()

  cinch_add_library_target(FleCSI-Tut flecsi-tutorial/specialization)
endif()

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

cinch_add_application_directory("examples")
cinch_add_application_directory("examples/00_simple_drivers")
cinch_add_application_directory("examples/lax_wendroff")
cinch_add_application_directory("examples/02_tasks_and_drivers")
cinch_add_application_directory("tools")

#------------------------------------------------------------------------------#
# Add distclean target
#------------------------------------------------------------------------------#

add_custom_target(distclean rm -rf ${CMAKE_BINARY_DIR}/*)

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
set(FLECSI_RUNTIME_MAIN ${FLECSI_SHARE_DIR}/runtime/runtime_main.cc)
set(FLECSI_RUNTIME_DRIVER ${FLECSI_SHARE_DIR}/runtime/runtime_driver.cc)

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

#~---------------------------------------------------------------------------~-#
# Formatting options
# vim: set tabstop=2 shiftwidth=2 expandtab :
#~---------------------------------------------------------------------------~-#
