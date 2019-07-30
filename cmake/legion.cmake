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

option(ENABLE_LEGION "Enable Legion" OFF)

if(ENABLE_LEGION)

  find_package(Legion REQUIRED)

  if(NOT Legion_FOUND)
    message(FATAL_ERROR "Legion is required for this build configuration")
  endif(NOT Legion_FOUND)

  set(CMAKE_PREFIX_PATH  ${CMAKE_PREFIX_PATH} ${LEGION_INSTALL_DIRS})

  include_directories(${Legion_INCLUDE_DIRS})

  add_definitions(-DLEGION_USE_CMAKE)
  add_definitions(-DREALM_USE_CMAKE)

  list(APPEND FLECSI_LIBRARY_DEPENDENCIES ${Legion_LIBRARIES})

  message(STATUS "Legion found: ${Legion_FOUND}")

endif(ENABLE_LEGION)
