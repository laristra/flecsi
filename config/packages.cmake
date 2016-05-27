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

#------------------------------------------------------------------------------#
# Runtime model setup for script interface
#------------------------------------------------------------------------------#

set(FLECSI_RUNTIME_LIBRARIES)

find_package (legion QUIET NO_MODULE)
set (Legion_INSTALL_DIR "" CACHE PATH "Path to the Legion install directory")
if (NOT Legion_INSTALL_DIR STREQUAL "")
  message(WARNING "Legion_INSTALL_DIR is obsolete, use CMAKE_PREFIX_PATH instead (and rebuild the latest version third-party libraries)")
  list(APPEND CMAKE_PREFIX_PATH "${Legion_INSTALL_DIR}")
endif()

# Serial interface
if(FLECSI_RUNTIME_MODEL STREQUAL "serial")

  set(FLECSI_RUNTIME_MAIN script-driver-serial.cc)

# Legion interface
elseif(FLECSI_RUNTIME_MODEL STREQUAL "legion")

  set(FLECSI_RUNTIME_MAIN script-driver-legion.cc)

  if(NOT legion_FOUND)
      message(FATAL_ERROR "Legion is required
                     for this build configuration")
  endif(NOT legion_FOUND)
  
  include_directories(${LEGION_INCLUDE_DIRS})
  set(FLECSI_RUNTIME_LIBRARIES ${LEGION_LIBRARIES} -ldl)
  SET_SOURCE_FILES_PROPERTIES(${LEGION_INCLUDE_DIRS}/legion/legion.inl PROPERTIES HEADER_FILE_ONLY 1)

# MPI interface
elseif(FLECSI_RUNTIME_MODEL STREQUAL "mpi")

  set(FLECSI_RUNTIME_MAIN script-driver-mpi.cc)

#MPI+Legion interface
elseif(FLECSI_RUNTIME_MODEL STREQUAL "mpilegion")
   set(FLECSI_RUNTIME_MAIN script-driver-mpilegion.cc)

  if(NOT legion_FOUND)
      message(FATAL_ERROR "Legion is required
                     for this build configuration")
  endif(NOT legion_FOUND)
  include_directories(${LEGION_INCLUDE_DIRS})
  set(FLECSI_RUNTIME_LIBRARIES ${LEGION_LIBRARIES} dl)

# Default
else(FLECSI_RUNTIME_MODEL STREQUAL "serial")

  message(FATAL_ERROR "Unrecognized runtime selection")  

endif(FLECSI_RUNTIME_MODEL STREQUAL "serial")

#------------------------------------------------------------------------------#
# Hypre
#------------------------------------------------------------------------------#
set (ENABLE_HYPRE OFF CACHE BOOL " do you want to enable HYPRE?")
if (ENABLE_HYPRE)
  find_package (HYPRE)

 if (HYPRE_FOUND)
   include_directories(${HYPRE_INCLUDE_DIRS})
   set(HYPRE_LIBRARY ${HYPRE_LIBRARIES})
 else()
   message (ERROR "HYPRE required for this build is not found")
  endif ()
endif (ENABLE_HYPRE)



#------------------------------------------------------------------------------#
# Process id bits
#------------------------------------------------------------------------------#

math(EXPR FLECSI_ID_EBITS "60 - ${FLECSI_ID_PBITS}")

add_definitions(-DFLECSI_ID_PBITS=${FLECSI_ID_PBITS})
add_definitions(-DFLECSI_ID_EBITS=${FLECSI_ID_EBITS})

math(EXPR flecsi_partitions "1 << ${FLECSI_ID_PBITS}")
math(EXPR flecsi_entities "1 << ${FLECSI_ID_EBITS}")

message(STATUS "Set id_t bits to allow ${flecsi_partitions} partitions with 2^${FLECSI_ID_EBITS} entities each")

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
    MESSAGE( FATAL_ERROR "You need libexodus either from TPL or system to enable I/O" )
  endif()
  add_definitions( -DHAVE_EXODUS )
endif(ENABLE_IO)

#------------------------------------------------------------------------------#
# Enable partitioning with METIS or SCOTCH
#------------------------------------------------------------------------------#

find_package(METIS 5.1)
find_package(SCOTCH)

if(ENABLE_MPI)
  find_package(ParMETIS)
endif()

if(METIS_FOUND OR SCOTCH_FOUND OR PARMETIS_FOUND)
  option(ENABLE_PARTITION "Enable partitioning (uses metis/parmetis or scotch)." ON)
else()
  option(ENABLE_PARTITION "Enable partitioning (uses metis/parmetis or scotch)." OFF)
endif()

if(ENABLE_PARTITION)

  set( PARTITION_LIBRARIES )

  if (METIS_FOUND)
     list( APPEND PARTITION_LIBRARIES ${METIS_LIBRARIES} )
     include_directories( ${METIS_INCLUDE_DIRS} )
     add_definitions( -DHAVE_METIS )
  endif()

  if (PARMETIS_FOUND)
     list( APPEND PARTITION_LIBRARIES ${PARMETIS_LIBRARIES} )
     include_directories( ${PARMETIS_INCLUDE_DIRS} )
     add_definitions( -DHAVE_PARMETIS )
  endif()

  if (SCOTCH_FOUND)
     list( APPEND PARTITION_LIBRARIES ${SCOTCH_LIBRARIES} )
     include_directories( ${SCOTCH_INCLUDE_DIRS} )
     add_definitions( -DHAVE_SCOTCH )
     if(SCOTCH_VERSION MATCHES ^5)
       #SCOTCH_VERSION from scotch.h is broken in scotch-5
       add_definitions( -DHAVE_SCOTCH_V5 )
     endif(SCOTCH_VERSION MATCHES ^5)
  endif()

  if ( NOT PARTITION_LIBRARIES )
     MESSAGE( FATAL_ERROR "You need scotch, metis or parmetis to enable partitioning" )
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
endif(NOT APPLE)

#------------------------------------------------------------------------------#
# Create compile scripts
#------------------------------------------------------------------------------#

# This configures the script that will be installed when 'make install' is
# executed.
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/bin/flecsi.in
  ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/flecsi-install)

# install script
install(FILES ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/flecsi-install
  DESTINATION bin
  RENAME flecsi
  PERMISSIONS
    OWNER_READ OWNER_WRITE OWNER_EXECUTE
    GROUP_READ GROUP_EXECUTE
    WORLD_READ WORLD_EXECUTE
)

# Install auxiliary files
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/driver/script-driver-serial.cc
  DESTINATION ${CMAKE_BINARY_DIR}/share
)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/driver/script-driver-legion.cc
  DESTINATION ${CMAKE_BINARY_DIR}/share
)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/driver/script-driver-mpi.cc
  DESTINATION ${CMAKE_BINARY_DIR}/share
)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/driver/script-driver-serial.cc
  DESTINATION share/flecsi)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/driver/script-driver-legion.cc
  DESTINATION share/flecsi)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/driver/script-driver-mpi.cc
  DESTINATION share/flecsi)

# This configures a locally available script that is suitable for
# testing within the build configuration before the project has been installed.
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/bin/flecsi-local.in
  ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/flecsi)

# copy local script to bin directory and change permissions
file(COPY ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/flecsi
  DESTINATION ${CMAKE_BINARY_DIR}/bin
  FILE_PERMISSIONS
    OWNER_READ OWNER_WRITE OWNER_EXECUTE
    GROUP_READ GROUP_EXECUTE
    WORLD_READ WORLD_EXECUTE
)
#------------------------------------------------------------------------------#
# Check the compiler version and output warnings if it is lower than 5.3.1
#------------------------------------------------------------------------------#

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.3.1)
    message(STATUS "your gcc compiler version is lower than 5.3.1, required for static meta container in FleCSi.  We recommend you to update your compiler. Otherwise static meta container will be turned off")
   set (STATIC_CONTAINER OFF)
  else()
   set (STATIC_CONTAINER ON)
  endif()
else()
    message(STATUS "static meta container has not been tested with your comiler so it will be disabled")
    set (STATIC_CONTAINER OFF)
elseif(...)
# etc.
endif()

#------------------------------------------------------------------------------#
# option for use of Static meta container
#------------------------------------------------------------------------------#

if (STATIC_CONTAINER)
option(ENABLE_STATIC_CONTAINER "Enable static meta container" ON)
else()
option(ENABLE_STATIC_CONTAINER "Enable static meta container" OFF)
endif (STATIC_CONTAINER)

set (MAX_CONTAINER_SIZE 6 CACHE INTEGER  "Set the depth of the container")
add_definitions( -DMAX_COUNTER_SIZE=${MAX_CONTAINER_SIZE} )

#~---------------------------------------------------------------------------~-#
# Formatting options
# vim: set tabstop=2 shiftwidth=2 expandtab :
#~---------------------------------------------------------------------------~-#
