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
  set (Legion_INSTALL_DIRS ${legion_DIR}  CACHE PATH
    "Path to the Legion install directory")
  set(CMAKE_PREFIX_PATH  ${CMAKE_PREFIX_PATH}
     ${LEGION_INSTALL_DIRS})
 # set (legion_DIR ${LEGION_INSTALL_DIRS})
  find_package (legion REQUIRED NO_MODULE)
  if(NOT legion_FOUND)
      message(FATAL_ERROR "Legion is required
                     for this build configuration")
  endif(NOT legion_FOUND)
  include_directories(${LEGION_INCLUDE_DIRS})
  set(FLECSI_RUNTIME_LIBRARIES ${LEGION_LIBRARIES})
  SET_SOURCE_FILES_PROPERTIES(${LEGION_INCLUDE_DIRS}/legion/legion.inl PROPERTIES HEADER_FILE_ONLY 1)

# MPI interface
elseif(FLECSI_RUNTIME_MODEL STREQUAL "mpi")

  set(FLECSI_RUNTIME_MAIN script-driver-mpi.cc)

#MPI+Legion interface
elseif(FLECSI_RUNTIME_MODEL STREQUAL "mpilegion")
   set(FLECSI_RUNTIME_MAIN script-driver-mpilegion.cc)

  # Add legion setup here...
  set (Legion_INSTALL_DIRS ${legion_DIR}  CACHE PATH
    "Path to the Legion install directory")
  set(CMAKE_PREFIX_PATH  ${CMAKE_PREFIX_PATH}
     ${LEGION_INSTALL_DIRS})
  find_package (legion REQUIRED NO_MODULE)
  if(NOT legion_FOUND)
      message(FATAL_ERROR "Legion is required
                     for this build configuration")
  endif(NOT legion_FOUND)
  include_directories(${LEGION_INCLUDE_DIRS})
  set(FLECSI_RUNTIME_LIBRARIES ${LEGION_LIBRARIES})

# Default
else(FLECSI_RUNTIME_MODEL STREQUAL "serial")

  message(FATAL_ERROR "Unrecognized runtime selection")  

endif(FLECSI_RUNTIME_MODEL STREQUAL "serial")

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
    set(IO_LIBRARIES ${TPL_INSTALL_PREFIX}/lib/libexodus.a
      ${TPL_INSTALL_PREFIX}/lib/libnetcdf.a
      ${TPL_INSTALL_PREFIX}/lib/libhdf5_hl.a
      ${TPL_INSTALL_PREFIX}/lib/libhdf5.a
      ${TPL_INSTALL_PREFIX}/lib/libszip.a
      ${TPL_INSTALL_PREFIX}/lib/libz.a
      -ldl)
    include_directories( ${TPL_INSTALL_PREFIX}/include )
  endif()
  add_definitions( -DHAVE_EXODUS )
  message(STATUS "Found EXODUSII: ${IO_LIBRARIES}")
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
  if(ENABLE_MPI)
      find_library( PARMETIS_LIBRARY
                    NAMES parmetis
                    PATHS ${PARMETIS_ROOT}
                    PATH_SUFFIXES lib
                    NO_DEFAULT_PATH )

      find_path( PARMETIS_INCLUDE_DIR
                 NAMES parmetis.h
                 PATHS ${PARMETIS_ROOT}
                 PATH_SUFFIXES include
                 NO_DEFAULT_PATH )
  endif()

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
     message(STATUS "Found METIS: ${METIS_LIBRARY} and ${METIS_INCLUDE_DIR}")
     set( METIS_FOUND TRUE )
     list( APPEND PARTITION_LIBRARIES ${METIS_LIBRARY} )
     include_directories( ${METIS_INCLUDE_DIR} )
     add_definitions( -DHAVE_METIS )
  endif()

  if (ENABLE_MPI AND PARMETIS_LIBRARY AND PARMETIS_INCLUDE_DIR)
     message(STATUS "Found ParMETIS: ${PARMETIS_LIBRARY} and ${PARMETIS_INCLUDE_DIR}")
     set( PARMETIS_FOUND TRUE )
     list( APPEND PARTITION_LIBRARIES ${PARMETIS_LIBRARY} )
     include_directories( ${PARMETIS_INCLUDE_DIR} )
     add_definitions( -DHAVE_PARMETIS )
  endif()

  if (SCOTCH_LIBRARY AND SCOTCH_ERR_LIBRARY AND SCOTCH_INCLUDE_DIR) 
     message(STATUS "Found SCOTCH: ${SCOTCH_LIBRARY}, ${SCOTCH_ERR_LIBRARY} and ${SCOTCH_INCLUDE_DIR}" )
     set( SCOTCH_FOUND TRUE )
     list( APPEND PARTITION_LIBRARIES ${SCOTCH_LIBRARY} ${SCOTCH_ERR_LIBRARY} )
     include_directories( ${SCOTCH_INCLUDE_DIR} )
     add_definitions( -DHAVE_SCOTCH )
  endif()

  if ( NOT PARTITION_LIBRARIES )
     MESSAGE( FATAL_ERROR "Need to specify either SCOTCH or METIS/ParMETIS" )
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
  # need a find_package that can discern between system installs and our TPLs

  # look in tpl's first, then try the system find_package(LAPACK)

  # This is a workaround that finds the TPL install if it's there. The link
  # to -lgfortan is a serious hack, and a documented issue.
  set(LAPACKE_FOUND)
  set(LAPACK_LIBRARIES)
  if(EXISTS ${TPL_INSTALL_PREFIX}/include/lapacke.h
     AND EXISTS ${TPL_INSTALL_PREFIX}/lib/liblapacke.a)
    set(LAPACKE_FOUND 1)
    include_directories(${TPL_INSTALL_PREFIX}/include)
    list( APPEND LAPACK_LIBRARIES
          ${TPL_INSTALL_PREFIX}/lib64/liblapacke.a
          ${TPL_INSTALL_PREFIX}/lib64/liblapack.a
          ${TPL_INSTALL_PREFIX}/lib64/libblas.a
          gfortran)
  else()
    # append lapacke to list of lapack libraries
    find_package(LAPACK)
    find_library(LAPACKE_LIB NAMES lapacke)
    find_path(LAPACKE_INCLUDE_DIRS NAMES lapacke.h PATH_SUFFIXES lapacke)
    if(LAPACKE_INCLUDE_DIRS AND LAPACK_FOUND AND LAPACKE_LIB)
      set(LAPACKE_FOUND TRUE)
      include_directories(${LAPACKE_INCLUDE_DIRS})
      list( APPEND LAPACK_LIBRARIES ${LAPACKE_LIB})
    endif(LAPACKE_INCLUDE_DIRS AND LAPACK_FOUND AND LAPACKE_LIB)
    # want to add ${LAPACK_INCLUDES}/lapacke to the include search path,
    # but FindLAPACK.cmake defines no such variable.
  endif()
  message(STATUS "Found LAPACK: ${LAPACK_LIBRARIES}")

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
