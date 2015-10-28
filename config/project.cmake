#~----------------------------------------------------------------------------~#
# Copyright (c) 2014 Los Alamos National Security, LLC
# All rights reserved.
#~----------------------------------------------------------------------------~#

project(flexi)

#------------------------------------------------------------------------------#
# Set application directory
#------------------------------------------------------------------------------#

cinch_add_application_directory("examples")

#------------------------------------------------------------------------------#
# Add library targets
#------------------------------------------------------------------------------#

cinch_add_library_target(flexi src)

#------------------------------------------------------------------------------#
# Set header suffix regular expression
#------------------------------------------------------------------------------#

set(CINCH_HEADER_SUFFIXES "\\.h")

#------------------------------------------------------------------------------#
# Add build options
#------------------------------------------------------------------------------#

set( TPL_INSTALL_PREFIX /path/to/third/party/install 
                        CACHE PATH
                        "path to thirdparty install" )
if (NOT TPL_INSTALL_PREFIX STREQUAL "")
  include_directories(${TPL_INSTALL_PREFIX}/include)
  set(METIS_ROOT  ${TPL_INSTALL_PREFIX})
  set(SCOTCH_ROOT ${TPL_INSTALL_PREFIX})
endif()


option(ENABLE_IO "Enable I/O with third party libraries." OFF)
if(ENABLE_IO)
  set(IO_LIBRARIES ${TPL_INSTALL_PREFIX}/lib/libexodus.a
    ${TPL_INSTALL_PREFIX}/lib/libnetcdf.a
    ${TPL_INSTALL_PREFIX}/lib/libhdf5_hl.a
    ${TPL_INSTALL_PREFIX}/lib/libhdf5.a
    ${TPL_INSTALL_PREFIX}/lib/libszip.a
    ${TPL_INSTALL_PREFIX}/lib/libz.a
    -ldl)
endif(ENABLE_IO)

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
     message(STATUS "Found METIS")
     set( METIS_FOUND TRUE )
     include_directories(${METIS_INCLUDE_DIR})
     list( APPEND PARTITION_LIBRARIES ${METIS_LIBRARY} )
  endif()

  if (SCOTCH_LIBRARY AND SCOTCH_ERR_LIBRARY AND SCOTCH_INCLUDE_DIR) 
     message(STATUS "Found SCOTCH" )
     set( SCOTCH_FOUND TRUE )
     include_directories(${SCOTCH_INCLUDE_DIR})
     list( APPEND PARTITION_LIBRARIES ${SCOTCH_LIBRARY} ${SCOTCH_ERR_LIBRARY} )
  endif()

  if ( NOT PARTITION_LIBRARIES )
     MESSAGE( FATAL_ERROR "Need to specify either SCOTCH or METIS" )
  endif()

endif()


#~---------------------------------------------------------------------------~-#
# Formatting options for vim.
# vim: set tabstop=2 shiftwidth=2 expandtab :
#~---------------------------------------------------------------------------~-#
