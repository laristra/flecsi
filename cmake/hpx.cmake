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

option(ENABLE_HPX "Enable HPX" OFF)

if(ENABLE_HPX)

#------------------------------------------------------------------------------#
# Find HPX
#------------------------------------------------------------------------------#

  find_package(HPX REQUIRED NO_CMAKE_PACKAGE_REGISTRY)

  list(APPEND FLECSI_LIBRARY_DEPENDENCIES HPX::hpx HPX::wrap_main)

  set(CMAKE_PREFIX_PATH  ${CMAKE_PREFIX_PATH} ${HPX_INSTALL_DIRS})

  if (NOT ENABLE_BOOST)
    message(ERROR "Boost is required for the HPX runtime")
  endif()

  add_definitions(-DENABLE_HPX)

  if(MSVC)
    add_definitions(-D_SCL_SECURE_NO_WARNINGS)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_SCL_SECURE_NO_DEPRECATE)
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
    add_definitions(-D_CRT_NONSTDC_NO_WARNINGS)
    add_definitions(-D_HAS_AUTO_PTR_ETC=1)
    add_definitions(-D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)
    add_definitions(-D_SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING)
    add_definitions(-DGTEST_LANG_CXX11=1)
  endif()

  message(STATUS "Found HPX: ${HPX_FOUND}")

endif(ENABLE_HPX)
