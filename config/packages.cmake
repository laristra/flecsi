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

  set(_runtime_path ${CMAKE_SOURCE_DIR}/flecsi/execution/serial)

#
# Legion interface
#
elseif(FLECSI_RUNTIME_MODEL STREQUAL "legion")

  set(_runtime_path ${CMAKE_SOURCE_DIR}/flecsi/execution/legion)

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

  set(_runtime_path ${CMAKE_SOURCE_DIR}/flecsi/execution/mpi)

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
 
  set(_runtime_path ${CMAKE_SOURCE_DIR}/flecsi/execution/mpilegion)

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
# Cereal
#------------------------------------------------------------------------------#

  find_package (Cereal REQUIRED)
  include_directories(${Cereal_INCLUDE_DIRS})

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
option(ENABLE_IO "Enable I/O (uses libexodus)" ${EXODUSII_FOUND})

if(ENABLE_IO)

  if(EXODUSII_FOUND)
    set(IO_LIBRARIES ${EXODUSII_LIBRARIES})
    include_directories(${EXODUSII_INCLUDE_DIRS})
  else()
    MESSAGE(FATAL_ERROR "You need libexodus either from TPL "
      "or system to enable I/O")
  endif()

endif(ENABLE_IO)

#------------------------------------------------------------------------------#
# Enable partitioning with METIS or SCOTCH
#------------------------------------------------------------------------------#

find_package(METIS 5.1)
find_package(SCOTCH)

if(ENABLE_MPI)
  # Counter-intuitive variable: set to TRUE to disable test
  set(PARMETIS_TEST_RUNS TRUE)
  find_package(ParMETIS 4.0)
endif()

if(PARMETIS_FOUND OR SCOTCH_FOUND)
  option(ENABLE_PARTITION
    "Enable partitioning (uses metis/parmetis or scotch)." ON)
else()
  option(ENABLE_PARTITION
    "Enable partitioning (uses metis/parmetis or scotch)." OFF)
endif()

if(ENABLE_PARTITION)

  set(PARTITION_LIBRARIES)

  if(METIS_FOUND)
     list(APPEND PARTITION_LIBRARIES ${METIS_LIBRARIES})
     include_directories(${METIS_INCLUDE_DIRS})
  endif()

  if(PARMETIS_FOUND)
    list(APPEND PARTITION_LIBRARIES ${PARMETIS_LIBRARIES})
    include_directories(${PARMETIS_INCLUDE_DIRS})
  endif()

  if(SCOTCH_FOUND)
     list(APPEND PARTITION_LIBRARIES ${SCOTCH_LIBRARIES})
     include_directories(${SCOTCH_INCLUDE_DIRS})
     if(SCOTCH_VERSION MATCHES ^5)
       #SCOTCH_VERSION from scotch.h is broken in scotch-5
       set(HAVE_SCOTCH_V5 ON)
     endif(SCOTCH_VERSION MATCHES ^5)
  endif()

  if(NOT PARTITION_LIBRARIES)
    MESSAGE(FATAL_ERROR
      "You need scotch, metis or parmetis to enable partitioning" )
  endif()

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
# Static container
#------------------------------------------------------------------------------#

option(ENABLE_STATIC_CONTAINER "Enable static meta container" OFF)

set (MAX_CONTAINER_SIZE 6 CACHE INTEGER  "Set the depth of the container")

#------------------------------------------------------------------------------#
# configure header
#------------------------------------------------------------------------------#

configure_file(${PROJECT_SOURCE_DIR}/config/flecsi.h.in ${CMAKE_BINARY_DIR}/flecsi.h @ONLY)
include_directories(${CMAKE_BINARY_DIR})
install(FILES ${CMAKE_BINARY_DIR}/flecsi.h DESTINATION include)

#~---------------------------------------------------------------------------~-#
# Formatting options
# vim: set tabstop=2 shiftwidth=2 expandtab :
#~---------------------------------------------------------------------------~-#
