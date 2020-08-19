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

cmake_minimum_required(VERSION 3.12)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

include(engine)
include(image)

project(FleCSI-CI LANGUAGES NONE)

#------------------------------------------------------------------------------#
# Options
#------------------------------------------------------------------------------#

set(REPOSITORY "https://gitlab.lanl.gov/laristra/flecsi.git" CACHE STRING
  "Set the FleCSI CI repository")
set(IMAGE_BRANCH "gitlab-ci" CACHE STRING
  "Set the repository branch, e.g., gitlab-ci")
set(REGISTRY "gitlab.lanl.gov:5050/laristra/flecsi" CACHE STRING "Set the registry")

#------------------------------------------------------------------------------#
# Images
#------------------------------------------------------------------------------#

list(APPEND BASE_IMAGES
  centos-7.8
  centos-8
  fedora-30
  fedora-31
  fedora-32
  ubuntu-20.04
  clearlinux
)

foreach(image ${BASE_IMAGES})

  add_image(${image} ${REGISTRY}:${image} os/${image})

  add_image(devel-${image} ${REGISTRY}:devel-${image}
    devel/environment
    ARGS
      REPO=${REPOSITORY}
      IMAGE_BRANCH=${IMAGE_BRANCH}
      SUBDIR=devel
      OS=${REGISTRY}:${image}
    DEPENDS push-${image}
  )

  add_image(util-${image} ${REGISTRY}:util-${image}
    util/environment
    ARGS
      REPO=${REPOSITORY}
      IMAGE_BRANCH=${IMAGE_BRANCH}
      SUBDIR=util
      OS=${REGISTRY}:${image}
    DEPENDS push-${image}
  )

endforeach()

#------------------------------------------------------------------------------#
# Tutorial image
#------------------------------------------------------------------------------#

add_image(tutorial ${REGISTRY}:tutorial
  devel/tutorial
    ARGS
      OS=${REGISTRY}:devel-centos-7.8
    DEPENDS push-devel-centos-7.8
)

#------------------------------------------------------------------------------#
# Add .dockerignore file to avoid including cmake files in context
#------------------------------------------------------------------------------#

configure_file(${CMAKE_SOURCE_DIR}/config/dockerignore
  ${CMAKE_BINARY_DIR}/.dockerignore)

file(COPY ${CMAKE_SOURCE_DIR}/config/clang_fortran.py
  DESTINATION ${CMAKE_BINARY_DIR}
  FILE_PERMISSIONS
    OWNER_READ
    OWNER_WRITE
    OWNER_EXECUTE
    GROUP_READ
    GROUP_EXECUTE
    WORLD_READ
    WORLD_EXECUTE
)

file(COPY ${CMAKE_SOURCE_DIR}/config/dircolors
  DESTINATION ${CMAKE_BINARY_DIR}
)

file(COPY ${CMAKE_SOURCE_DIR}/config/bashrc
  DESTINATION ${CMAKE_BINARY_DIR}
)

file(COPY ${CMAKE_SOURCE_DIR}/config/vimrc
  DESTINATION ${CMAKE_BINARY_DIR}
)
