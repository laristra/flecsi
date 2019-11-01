#------------------------------------------------------------------------------#
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
#------------------------------------------------------------------------------#

option(ENABLE_HDF5 "Enable HDF5" OFF)

if(ENABLE_HDF5)

  # CMake's FindHDF5 package ignores any path in CMAKE_PREFIX_PATH that
  # doesn't contain the hdf5-config.cmake file in a certain location.
  # Some of our HDF5 builds don't have this.  So we have to give CMake
  # a little extra help...
  if(NOT HDF5_ROOT)
    find_path(HDF5_INCLUDE_DIR hdf5.h
      HINTS ENV HDF5_ROOT
      PATH_SUFFIXES include)
    get_filename_component(HDF5_ROOT ${HDF5_INCLUDE_DIR} DIRECTORY)
  endif()

  find_package(HDF5 REQUIRED)

  include_directories(${HDF5_C_INCLUDE_DIR})

  if(CMAKE_BUILD_TYPE MATCHES Debug)
    list(APPEND FLECSI_LIBRARY_DEPENDENCIES ${HDF5_hdf5_LIBRARY_DEBUG})
  else()
    list(APPEND FLECSI_LIBRARY_DEPENDENCIES ${HDF5_hdf5_LIBRARY_RELEASE})
  endif()
endif()
