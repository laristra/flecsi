#~----------------------------------------------------------------------------~#
# Copyright (c) 2014 Los Alamos National Security, LLC
# All rights reserved.
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


#~---------------------------------------------------------------------------~-#
# Formatting options for vim.
# vim: set tabstop=2 shiftwidth=2 expandtab :
#~---------------------------------------------------------------------------~-#
