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
# Copyright (c) 2016, Triad National Security, LLC
# All rights reserved
#------------------------------------------------------------------------------#

set(CONTAINER_ENGINES docker podman)

if(NOT CONTAINER_ENGINE)
  list(GET CONTAINER_ENGINES 0 CONTAINER_ENGINE)
endif()

set(CONTAINER_ENGINE "${CONTAINER_ENGINE}" CACHE STRING
  "Select the container engine")
set_property(CACHE CONTAINER_ENGINE
  PROPERTY STRINGS ${CONTAINER_ENGINES})

find_program(ENGINE ${CONTAINER_ENGINE})

if(ENGINE-NOTFOUND)
  message(FATAL_ERROR "Failed to find ${CONTAINER_ENGINE}")
else()
  message(STATUS "Container Engine (${CONTAINER_ENGINE}): found ${ENGINE}")
endif()
