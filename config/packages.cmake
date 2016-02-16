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

# Serial interface
if(FLECSI_RUNTIME_MODEL STREQUAL "serial")

  set(FLECSI_RUNTIME_MAIN script-driver-serial.cc)

# Legion interface
elseif(FLECSI_RUNTIME_MODEL STREQUAL "legion")

  set(FLECSI_RUNTIME_MAIN script-driver-legion.cc)

  # Add legion setup here...
  # include_directories(${Legion_INC_DIRS})
  # set(FLECSI_RUNTIME_LIBRARIES ${Legion_LIBRARIES})

# MPI interface
elseif(FLECSI_RUNTIME_MODEL STREQUAL "mpi")

  set(FLECSI_RUNTIME_MAIN script-driver-mpi.cc)

# Default
else(FLECSI_RUNTIME_MODEL STREQUAL "serial")

  message(FATAL_ERROR "Unrecognized runtime selection")  

endif(FLECSI_RUNTIME_MODEL STREQUAL "serial")

#------------------------------------------------------------------------------#
# Process id bits
#------------------------------------------------------------------------------#

add_definitions(-DFLECSI_ID_PBITS=${FLECSI_ID_PBITS})

execute_process( COMMAND echo "2^${FLECSI_ID_PBITS}" COMMAND bc -l
  OUTPUT_VARIABLE flecsi_partitions OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process( COMMAND echo "2^(60-${FLECSI_ID_PBITS})" COMMAND bc -l
  OUTPUT_VARIABLE flecsi_entities OUTPUT_STRIP_TRAILING_WHITESPACE)

message(STATUS "Set id_t bits to allow ${flecsi_partitions} partitions with ${flecsi_entities} entities each")

#------------------------------------------------------------------------------#
# Enable IO with exodus
#------------------------------------------------------------------------------#

option(ENABLE_IO "Enable I/O with third party libraries." OFF)
if(ENABLE_IO)
  set(IO_LIBRARIES ${TPL_INSTALL_PREFIX}/lib/libexodus.a
    ${TPL_INSTALL_PREFIX}/lib/libnetcdf.a
    ${TPL_INSTALL_PREFIX}/lib/libhdf5_hl.a
    ${TPL_INSTALL_PREFIX}/lib/libhdf5.a
    ${TPL_INSTALL_PREFIX}/lib/libszip.a
    ${TPL_INSTALL_PREFIX}/lib/libz.a
    -ldl)
  include_directories( ${TPL_INSTALL_PREFIX}/include )
  add_definitions( -DHAVE_EXODUS )
endif(ENABLE_IO)

#------------------------------------------------------------------------------#
# Enable LAPACK
#------------------------------------------------------------------------------#
option(ENABLE_LAPACK "Enable use of LAPACK solver." OFF)
if(ENABLE_LAPACK)
  set(LAPACK_LIBRARIES
      ${TPL_INSTALL_PREFIX}/lib/liblapacke.a
      ${TPL_INSTALL_PREFIX}/lib/liblapack.a
      ${TPL_INSTALL_PREFIX}/lib/libcblas.a
      ${TPL_INSTALL_PREFIX}/lib/libblas.a
      -lgfortran
      )
  include_directories( ${TPL_INSTALL_PREFIX}/include )
  add_definitions( -DHAVE_LAPACK )
endif(ENABLE_LAPACK)

#------------------------------------------------------------------------------#
# Enable partitioning with METIS or SCOTCH
#------------------------------------------------------------------------------#

option(ENABLE_PARTITION "Enable partitioning with third party libraries." OFF)

if(ENABLE_PARTITION)

  set( PARTITION_LIBRARIES )

  find_library ( METIS_LIBRARY 
                 NAMES metis 
                 PATHS ${METIS_ROOT} 
                 PATH_SUFFIXES lib
                 NO_DEFAULT_PATH )

  find_path    ( METIS_INCLUDE_DIR 
                 NAMES metis.h 
                 PATHS ${METIS_ROOT} 
                 PATH_SUFFIXES include
                 NO_DEFAULT_PATH )

  find_library ( SCOTCH_LIBRARY 
                 NAMES scotch
                 PATHS ${SCOTCH_ROOT} 
                 PATH_SUFFIXES lib
                 NO_DEFAULT_PATH )

  find_library ( SCOTCH_ERR_LIBRARY 
                 NAMES scotcherr
                 PATHS ${SCOTCH_ROOT} 
                 PATH_SUFFIXES lib
                 NO_DEFAULT_PATH )

  find_path    ( SCOTCH_INCLUDE_DIR 
                 NAMES scotch.h
                 PATHS ${SCOTCH_ROOT} 
                 PATH_SUFFIXES include
                 NO_DEFAULT_PATH )

  if (METIS_LIBRARY AND METIS_INCLUDE_DIR) 
     message(STATUS "Found METIS: ${METIS_ROOT}")
     set( METIS_FOUND TRUE )
     list( APPEND PARTITION_LIBRARIES ${METIS_LIBRARY} )
     include_directories( ${METIS_INCLUDE_DIR} )
     add_definitions( -DHAVE_METIS )
  endif()

  if (SCOTCH_LIBRARY AND SCOTCH_ERR_LIBRARY AND SCOTCH_INCLUDE_DIR) 
     message(STATUS "Found SCOTCH: ${SCOTCH_ROOT}" )
     set( SCOTCH_FOUND TRUE )
     list( APPEND PARTITION_LIBRARIES ${SCOTCH_LIBRARY} ${SCOTCH_ERR_LIBRARY} )
     include_directories( ${SCOTCH_INCLUDE_DIR} )
     add_definitions( -DHAVE_SCOTCH )
  endif()

  if ( NOT PARTITION_LIBRARIES )
     MESSAGE( FATAL_ERROR "Need to specify either SCOTCH or METIS" )
  endif()

endif()

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

#~---------------------------------------------------------------------------~-#
# Formatting options
# vim: set tabstop=2 shiftwidth=2 expandtab :
#~---------------------------------------------------------------------------~-#
