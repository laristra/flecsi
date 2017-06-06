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

project(flecsi)

#------------------------------------------------------------------------------#
# Set header suffix regular expression
#------------------------------------------------------------------------------#

set(CINCH_HEADER_SUFFIXES "\\.h")

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

#------------------------------------------------------------------------------#
# Enable Boost.Preprocessor
#------------------------------------------------------------------------------#

# This changes the Cinch default
set(ENABLE_BOOST_PREPROCESSOR ON CACHE BOOL "Enable Boost.Preprocessor")


#------------------------------------------------------------------------------#
# cinch_load_extras will try and find legion and mpi.  If we want to
# override the defaults, i.e. ENABLE_MPI=on and ENABLE_LEGION=on, we
# need to do it before cinch_load_extras is called.
#------------------------------------------------------------------------------#

if(FLECSI_RUNTIME_MODEL STREQUAL "mpi")
  set(ENABLE_MPI ON CACHE BOOL "Enable MPI")
elseif(FLECSI_RUNTIME_MODEL STREQUAL "legion")
  set(ENABLE_MPI ON CACHE BOOL "Enable MPI")
  set(ENABLE_LEGION ON CACHE BOOL "Enable Legion")
endif()

#------------------------------------------------------------------------------#
# Add options for design by contract
#------------------------------------------------------------------------------#

set(FLECSI_DBC_ACTIONS throw notify nothing)

if(NOT FLECSI_DBC_ACTION)
  list(GET FLECSI_DBC_ACTIONS 0 FLECSI_DBC_ACTION)
endif()

set(FLECSI_DBC_ACTION "${FLECSI_DBC_ACTION}" CACHE STRING
  "Select the design by contract action")
set_property(CACHE FLECSI_DBC_ACTION PROPERTY STRINGS ${FLECSI_DBC_ACTIONS})

set(FLECSI_DBC_REQUIRE ON CACHE BOOL
  "Enable DBC Pre/Post Condition Assertions")

#------------------------------------------------------------------------------#
# Load the cinch extras
#------------------------------------------------------------------------------#

cinch_load_extras()

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
# Add options for runtime selection
#------------------------------------------------------------------------------#

set(FLECSI_RUNTIME_MODELS serial legion mpi)

if(NOT FLECSI_RUNTIME_MODEL)
  list(GET FLECSI_RUNTIME_MODELS 0 FLECSI_RUNTIME_MODEL)
endif()

set(FLECSI_RUNTIME_MODEL "${FLECSI_RUNTIME_MODEL}" CACHE STRING
  "Select the runtime model")
set_property(CACHE FLECSI_RUNTIME_MODEL
  PROPERTY STRINGS ${FLECSI_RUNTIME_MODELS})

#------------------------------------------------------------------------------#
# Add options for design by contract
#------------------------------------------------------------------------------#

set(FLECSI_DBC_ACTIONS throw notify nothing)

if(NOT FLECSI_DBC_ACTION)
  list(GET FLECSI_DBC_ACTIONS 0 FLECSI_DBC_ACTION)
endif()

set(FLECSI_DBC_ACTION "${FLECSI_DBC_ACTION}" CACHE STRING
  "Select the design by contract action")
set_property(CACHE FLECSI_DBC_ACTION PROPERTY STRINGS ${FLECSI_DBC_ACTIONS})

set(FLECSI_DBC_REQUIRE ON CACHE BOOL
  "Enable DBC Pre/Post Condition Assertions")

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
# Boost
#------------------------------------------------------------------------------#

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

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

  if(NOT MPI_${MPI_LANGUAGE}_FOUND)
    message (FATAL_ERROR "MPI is required for the legion runtime model")
  endif()
 
  if(NOT Legion_FOUND)
    message (FATAL_ERROR "Legion is required for the legion runtime model")
  endif()
 
  set(_runtime_path ${PROJECT_SOURCE_DIR}/flecsi/execution/legion)

  if(NOT APPLE)
    set(FLECSI_RUNTIME_LIBRARIES -ldl ${Legion_LIBRARIES} ${MPI_LIBRARIES})
  else()
    set(FLECSI_RUNTIME_LIBRARIES ${Legion_LIBRARIES} ${MPI_LIBRARIES})
  endif()

  include_directories(${Legion_INCLUDE_DIRS})

  option(ENABLE_LEGION_TLS "Enable TLS storage of runtime state" OFF)
  if(ENABLE_LEGION_TLS)
    add_definitions(-DENABLE_LEGION_TLS)
  endif()

#
# MPI interface
#
elseif(FLECSI_RUNTIME_MODEL STREQUAL "mpi")

  if(NOT MPI_${MPI_LANGUAGE}_FOUND)
    message (FATAL_ERROR "MPI is required for the mpi runtime model")
  endif()

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


#------------------------------------------------------------------------------#
# configure header
#------------------------------------------------------------------------------#

configure_file(${PROJECT_SOURCE_DIR}/config/flecsi.h.in
  ${CMAKE_BINARY_DIR}/flecsi.h @ONLY)
include_directories(${CMAKE_BINARY_DIR})
install(FILES ${CMAKE_BINARY_DIR}/flecsi.h DESTINATION include)

#------------------------------------------------------------------------------#
# Add library targets
#------------------------------------------------------------------------------#

cinch_add_library_target(flecsi flecsi)

#------------------------------------------------------------------------------#
# Link the necessary libraries
#------------------------------------------------------------------------------#

if ( FLECSI_RUNTIME_LIBRARIES OR COLORING_LIBRARIES )
  cinch_target_link_libraries(
    flecsi ${FLECSI_RUNTIME_LIBRARIES} ${COLORING_LIBRARIES}
  )
endif()

#------------------------------------------------------------------------------#
# Set application directory
#------------------------------------------------------------------------------#

cinch_add_application_directory("examples")
cinch_add_application_directory("examples/lax_wendroff")
cinch_add_application_directory("tools")

#------------------------------------------------------------------------------#
# Add distclean target
#------------------------------------------------------------------------------#

add_custom_target(distclean rm -rf ${CMAKE_BINARY_DIR}/*)

#~---------------------------------------------------------------------------~-#
# Formatting options
# vim: set tabstop=2 shiftwidth=2 expandtab :
#~---------------------------------------------------------------------------~-#
