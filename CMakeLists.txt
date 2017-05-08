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
# Require some version of cmake
#------------------------------------------------------------------------------#

cmake_minimum_required(VERSION 2.8)

#------------------------------------------------------------------------------#
# Set the project name
#------------------------------------------------------------------------------#

project(flecsi)

#------------------------------------------------------------------------------#
# Include project-level CMake configuration file
#------------------------------------------------------------------------------#

if(NOT "$ENV{CINCH_SOURCE_DIR}" STREQUAL "")
    set(CINCH_SOURCE_DIR $ENV{CINCH_SOURCE_DIR})
elseif(EXISTS ${CMAKE_SOURCE_DIR}/cinch)
  set(CINCH_SOURCE_DIR ${CMAKE_SOURCE_DIR}/cinch)
endif()

# This changes the Cinch default
set(ENABLE_BOOST_PREPROCESSOR ON CACHE BOOL
  "Enable Boost.Preprocessor")

include(${CINCH_SOURCE_DIR}/cmake/cinch.cmake)

#------------------------------------------------------------------------------#
# Set the minimum Cinch version
#------------------------------------------------------------------------------#

cinch_minimum_required(1.0)

#------------------------------------------------------------------------------#
# Add options for runtime selection
#------------------------------------------------------------------------------#

set(FLECSI_RUNTIME_MODELS serial mpilegion legion mpi)

if(NOT FLECSI_RUNTIME_MODEL)
  list(GET FLECSI_RUNTIME_MODELS 0 FLECSI_RUNTIME_MODEL)
endif()

set(FLECSI_RUNTIME_MODEL "${FLECSI_RUNTIME_MODEL}" CACHE STRING
  "Select the runtime model")
set_property(CACHE FLECSI_RUNTIME_MODEL
  PROPERTY STRINGS ${FLECSI_RUNTIME_MODELS})

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
# Add option for FleCSIT command-line tool.
#------------------------------------------------------------------------------#

option(ENABLE_FLECSIT "Enable FleCSIT Command-Line Tool" OFF)

#------------------------------------------------------------------------------#
# If a C++14 compiler is available, then set the appropriate flags
#------------------------------------------------------------------------------#

include(cxx14)

check_for_cxx14_compiler(CXX14_COMPILER)

if(CXX14_COMPILER)
    enable_cxx14()
else()
    message(FATAL_ERROR "C++14 compatible compiler not found")
endif()

set(FLECSI_RUNTIME_LIBRARIES)

#------------------------------------------------------------------------------#
# OpenSSL
#------------------------------------------------------------------------------#

option(ENABLE_OPENSSL "Enable OpenSSL Support" OFF)

if(ENABLE_OPENSSL)
  find_package(OpenSSL REQUIRED)

  if(OPENSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
    add_definitions(-DENABLE_OPENSSL)
  endif()
endif()

#------------------------------------------------------------------------------#
# Find Legion
#------------------------------------------------------------------------------#

if(FLECSI_RUNTIME_MODEL STREQUAL "legion" OR
  FLECSI_RUNTIME_MODEL STREQUAL "mpilegion")

  find_package(Legion REQUIRED)

  message(STATUS "Legion found: ${Legion_FOUND}")

endif()

#------------------------------------------------------------------------------#
# Runtime models
#------------------------------------------------------------------------------#

#
# Serial interface
#
if(FLECSI_RUNTIME_MODEL STREQUAL "serial")

  set(_runtime_path ${PROJECT_SOURCE_DIR}/flecsi/execution/serial)

#
# Legion interface
#
elseif(FLECSI_RUNTIME_MODEL STREQUAL "legion")

  set(_runtime_path ${PROJECT_SOURCE_DIR}/flecsi/execution/legion)

  if(NOT APPLE)
    set(FLECSI_RUNTIME_LIBRARIES  -ldl ${Legion_LIBRARIES} ${MPI_LIBRARIES})
  else()
    set(FLECSI_RUNTIME_LIBRARIES  ${Legion_LIBRARIES} ${MPI_LIBRARIES})
  endif()

  include_directories(${Legion_INCLUDE_DIRS})

#
# MPI interface
#
elseif(FLECSI_RUNTIME_MODEL STREQUAL "mpi")

  set(_runtime_path ${PROJECT_SOURCE_DIR}/flecsi/execution/mpi)

  if(NOT APPLE)
    set(FLECSI_RUNTIME_LIBRARIES  -ldl ${MPI_LIBRARIES})
  else()
    set(FLECSI_RUNTIME_LIBRARIES ${MPI_LIBRARIES})
  endif()

#
# MPI+Legion interface
#
elseif(FLECSI_RUNTIME_MODEL STREQUAL "mpilegion")

  if(NOT ENABLE_MPI)
    message (FATAL_ERROR "MPI is required for the mpilegion runtime model")
  endif()
 
  set(_runtime_path ${PROJECT_SOURCE_DIR}/flecsi/execution/mpilegion)

  if(NOT APPLE)
    set(FLECSI_RUNTIME_LIBRARIES  -ldl ${Legion_LIBRARIES} ${MPI_LIBRARIES})
  else()
    set(FLECSI_RUNTIME_LIBRARIES  ${Legion_LIBRARIES} ${MPI_LIBRARIES})
  endif()

  include_directories(${Legion_INCLUDE_DIRS})

#
# Default
#
else(FLECSI_RUNTIME_MODEL STREQUAL "serial")

  message(FATAL_ERROR "Unrecognized runtime selection")  

endif(FLECSI_RUNTIME_MODEL STREQUAL "serial")

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
# Enable IO with exodus
#------------------------------------------------------------------------------#

find_package(EXODUSII)
option(ENABLE_EXODUS "Enable I/O (uses libexodus)" ${EXODUSII_FOUND})

if(ENABLE_EXODUS)

  if(EXODUSII_FOUND)
    set(IO_LIBRARIES ${EXODUSII_LIBRARIES})
    include_directories(${EXODUSII_INCLUDE_DIRS})
  else()
    MESSAGE(FATAL_ERROR "You need libexodus either from TPL "
      "or system to enable I/O")
  endif()

endif(ENABLE_EXODUS)

#------------------------------------------------------------------------------#
# Enable partitioning with METIS
#------------------------------------------------------------------------------#

find_package(METIS 5.1)

if(ENABLE_MPI)
  # Counter-intuitive variable: set to TRUE to disable test
  set(PARMETIS_TEST_RUNS TRUE)
  find_package(ParMETIS 4.0)
endif()

option(ENABLE_PARTITIONING
  "Enable partitioning (uses metis/parmetis or scotch)." OFF)

if(ENABLE_PARTITIONING)

  set(PARTITION_LIBRARIES)

  if(METIS_FOUND)
    list(APPEND PARTITION_LIBRARIES ${METIS_LIBRARIES})
    include_directories(${METIS_INCLUDE_DIRS})
    set(ENABLE_METIS TRUE)
    add_definitions(-DENABLE_METIS)
  endif()

  if(PARMETIS_FOUND)
    list(APPEND PARTITION_LIBRARIES ${PARMETIS_LIBRARIES})
    include_directories(${PARMETIS_INCLUDE_DIRS})
    set(ENABLE_PARMETIS TRUE)
    add_definitions(-DENABLE_PARMETIS)
  endif()

  if(NOT PARTITION_LIBRARIES)
    MESSAGE(FATAL_ERROR
      "You need parmetis to enable partitioning" )
  endif()

  add_definitions(-DENABLE_PARTITIONING)

endif()

#------------------------------------------------------------------------------#
# LAPACK
#------------------------------------------------------------------------------#

# NB: The code that uses lapack actually requires lapacke:
# http://www.netlib.org/lapack/lapacke.html
# If the installation of lapack that this finds does not contain lapacke then
# the build will fail.
if(NOT APPLE)
  find_package(LAPACKE)

  if(LAPACKE_FOUND)
      include_directories( ${LAPACKE_INCLUDE_DIRS} )
  endif(LAPACKE_FOUND)
endif(NOT APPLE)

#------------------------------------------------------------------------------#
# configure header
#------------------------------------------------------------------------------#

configure_file(${PROJECT_SOURCE_DIR}/config/flecsi.h.in ${CMAKE_BINARY_DIR}/flecsi.h @ONLY)
include_directories(${CMAKE_BINARY_DIR})
install(FILES ${CMAKE_BINARY_DIR}/flecsi.h DESTINATION include)

#------------------------------------------------------------------------------#
# Add library targets
#------------------------------------------------------------------------------#

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )
add_subdirectory( flecsi )


#------------------------------------------------------------------------------#
# Set application directory
#------------------------------------------------------------------------------#

# cinch_add_application_directory("examples")
# cinch_add_application_directory("examples/lax_wendroff")
# cinch_add_application_directory("bin")
# cinch_add_application_directory("tools")

#~---------------------------------------------------------------------------~-#
# Formatting options
# vim: set tabstop=2 shiftwidth=2 expandtab :
#~---------------------------------------------------------------------------~-#
