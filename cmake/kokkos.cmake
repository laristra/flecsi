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

option(ENABLE_KOKKOS "Enable Kokkos" OFF)

if(ENABLE_KOKKOS)

  if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND
    NOT ${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS 8)
    message(FATAL_ERROR "Clang version 8 or greater required for Kokkos")
  endif()

  find_package(Kokkos REQUIRED)

  include_directories(${KOKKOS_INCLUDE_DIR})
  link_directories(${KOKKOS_LIBRARY_DIRS})

  list(APPEND FLECSI_LIBRARY_DEPENDENCIES ${KOKKOS_CORE_LIBRARY})
endif()
