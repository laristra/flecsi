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

option(ENABLE_CALIPER "Enable Caliper" OFF)
mark_as_advanced(ENABLE_CALIPER)

if(ENABLE_CALIPER)
    find_package(Caliper REQUIRED)

    if(NOT Caliper_FOUND)
	    message(FATAL_ERROR "Caliper is required for this build configuration")
    endif()

    message(STATUS "Found Caliper: ${Caliper_INCLUDE_DIRS}")

    include_directories(SYSTEM ${Caliper_INCLUDE_DIRS})
    add_definitions(-DENABLE_CALIPER)
    list(APPEND FLECSI_LIBRARY_DEPENDENCIES ${Caliper_LIBRARIES})
    if(ENABLE_MPI)
      list(APPEND FLECSI_LIBRARY_DEPENDENCIES ${Caliper_MPI_LIBRARIES})
    endif(ENABLE_MPI)
endif(ENABLE_CALIPER)
