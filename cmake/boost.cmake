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

option(ENABLE_BOOST "Enable Boost" OFF)

if(ENABLE_BOOST)

  set(Boost_NO_BOOST_CMAKE ON)

  find_package(Boost REQUIRED 
    program_options
    atomic
    filesystem
    regex
    serialization
    system
    QUIET)

  include_directories(${Boost_INCLUDE_DIRS})
  link_directories(${Boost_LIBRARY_DIRS})

  list(APPEND CINCH_RUNTIME_LIBRARIES ${Boost_LIBRARIES})

endif()
