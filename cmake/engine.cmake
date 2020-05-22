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

set(ENGINE_EXECUTABLE)

if("${CONTAINER_ENGINE}" STREQUAL "docker")
  find_program(DOCKER_EXECUTABLE docker)

  if(DOCKER_EXECUTABLE-NOTFOUND)
    message(FATAL_ERROR "Failed to find docker")
  endif()

  set(ENGINE_EXECUTABLE ${DOCKER_EXECUTABLE})
elseif("${CONTAINER_ENGINE}" STREQUAL "podman")
  find_program(PODMAN_EXECUTABLE podman)
  
  if(PODMAN_EXECUTABLE-NOTFOUND)
    message(FATAL_ERROR "Failed to find podman")
  endif()

  set(ENGINE_EXECUTABLE ${PODMAN_EXECUTABLE})
else()
  message(FATAL_ERROR "invalid container engine")
endif()

message(STATUS
  "Container Engine (${CONTAINER_ENGINE}): found ${ENGINE_EXECUTABLE}")
mark_as_advanced(PODMAN_EXECUTABLE DOCKER_EXECUTABLE)
