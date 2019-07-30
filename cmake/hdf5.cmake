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
  find_package(HDF5 REQUIRED)

  include_directories(${HDF5_C_INCLUDE_DIR})

  if(CMAKE_BUILD_TYPE MATCHES Debug)
    list(APPEND FLECSI_LIBRARY_DEPENDENCIES ${HDF5_hdf5_LIBRARY_DEBUG})
  else()
    list(APPEND FLECSI_LIBRARY_DEPENDENCIES ${HDF5_hdf5_LIBRARY_RELEASE})
  endif()
endif()
