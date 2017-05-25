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
# Set requirement for C++14
#------------------------------------------------------------------------------#

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#------------------------------------------------------------------------------#
# DBC
#------------------------------------------------------------------------------#
if(FLECSI_DBC_ACTION STREQUAL "throw")
  add_definitions(-DFLECSI_DBC_THROW)
elseif(FLECSI_DBC_ACTION STREQUAL "notify")
  add_definitions(-DFLECSI_DBC_NOTIFY)
endif()

if(FLECSI_DBC_REQUIRE)
  add_definitions(-DFLECSI_REQUIRE_ON)
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
  endif()
endif()

#------------------------------------------------------------------------------#
# Find Legion
#------------------------------------------------------------------------------#

if(FLECSI_RUNTIME_MODEL STREQUAL "legion")

  find_package(Legion REQUIRED)

  message(STATUS "Legion found: ${Legion_FOUND}")

endif()

#------------------------------------------------------------------------------#
# Runtime models
#------------------------------------------------------------------------------#

set(FLECSI_RUNTIME_LIBRARIES)

#
# Serial interface
#
if(FLECSI_RUNTIME_MODEL STREQUAL "serial")

  set(_runtime_path ${PROJECT_SOURCE_DIR}/flecsi/execution/serial)

#
# Legion interface
#
elseif(FLECSI_RUNTIME_MODEL STREQUAL "legion")

  if(NOT ENABLE_MPI)
    message (FATAL_ERROR "MPI is required for the legion runtime model")
  endif()
 
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
# Default
#
else()

  message(FATAL_ERROR "Unrecognized runtime selection")  

endif()

#------------------------------------------------------------------------------#
# Boost
#------------------------------------------------------------------------------#

#find_package(Boost 1.58.0 REQUIRED COMPONENTS serialization)

#include_directories(${Boost_INCLUDE_DIRS})
#set(FLECSI_RUNTIME_LIBRARIES ${FLECSI_RUNTIME_LIBRARIES} ${Boost_LIBRARIES})

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
# Enable partitioning with METIS
#------------------------------------------------------------------------------#

find_package(METIS 5.1)

if(ENABLE_MPI)
  # Counter-intuitive variable: set to TRUE to disable test
  set(PARMETIS_TEST_RUNS TRUE)
  find_package(ParMETIS 4.0)
endif()

option(ENABLE_COLORING
  "Enable partitioning (uses metis/parmetis or scotch)." OFF)

if(ENABLE_COLORING)

  set(COLORING_LIBRARIES)

  if(METIS_FOUND)
    list(APPEND COLORING_LIBRARIES ${METIS_LIBRARIES})
    include_directories(${METIS_INCLUDE_DIRS})
    set(ENABLE_METIS TRUE)
    add_definitions(-DENABLE_METIS)
  endif()

  if(PARMETIS_FOUND)
    list(APPEND COLORING_LIBRARIES ${PARMETIS_LIBRARIES})
    include_directories(${PARMETIS_INCLUDE_DIRS})
    set(ENABLE_PARMETIS TRUE)
    add_definitions(-DENABLE_PARMETIS)
  endif()

  if(NOT COLORING_LIBRARIES)
    MESSAGE(FATAL_ERROR
      "You need parmetis to enable partitioning" )
  endif()

  add_definitions(-DENABLE_COLORING)

endif()

if ( FLECSI_RUNTIME_LIBRARIES OR PARTITION_LIBRARIES )
  cinch_target_link_libraries(
    flecsi ${FLECSI_RUNTIME_LIBRARIES} ${PARTITION_LIBRARIES}
  )
endif()

#------------------------------------------------------------------------------#
# configure header
#------------------------------------------------------------------------------#

configure_file(${PROJECT_SOURCE_DIR}/config/flecsi.h.in
  ${CMAKE_BINARY_DIR}/flecsi.h @ONLY)
include_directories(${CMAKE_BINARY_DIR})
install(FILES ${CMAKE_BINARY_DIR}/flecsi.h DESTINATION include)

#~---------------------------------------------------------------------------~-#
# Formatting options
# vim: set tabstop=2 shiftwidth=2 expandtab :
#~---------------------------------------------------------------------------~-#
