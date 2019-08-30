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

option(FLECSI_ENABLE_KOKKOS "Enable Kokkos" OFF)

if(FLECSI_ENABLE_KOKKOS)

  if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND
    NOT ${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS 8)
    message(FATAL_ERROR "Clang version 8 or greater required for Kokkos")
  endif()

  find_package(Kokkos REQUIRED)


  link_directories(${Kokkos_LIBRARY_DIRS})

  list(APPEND FLECSI_LIBRARY_DEPENDENCIES Kokkos::kokkos)
  list(APPEND FLECSI_LIBRARY_DEPENDENCIES Kokkos::kokkoscore)

endif()
