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

option(ENABLE_GRAPHVIZ "Enable Graphviz" OFF)

if(ENABLE_GRAPHVIZ)
    find_package(Graphviz REQUIRED)

    if(NOT Graphviz_FOUND)
        message(FATAL_ERROR "Graphviz is required for this build configuration")
    endif()

    message(STATUS "Found Graphviz: ${Graphviz_INCLUDE_DIRS}")

    include_directories(SYSTEM ${Graphviz_INCLUDE_DIRS})
    list(APPEND FLECSI_LIBRARY_DEPENDENCIES ${Graphviz_LIBRARIES})
endif(ENABLE_GRAPHVIZ)
