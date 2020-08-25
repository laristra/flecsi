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

set(IMAGE_REPOSITORY "https://gitlab.lanl.gov/laristra/flecsi.git" CACHE STRING
  "Set the FleCSI image repository")
set(IMAGE_BRANCH "gitlab-ci" CACHE STRING
  "Set the image repository branch, e.g., gitlab-ci")
set(SOURCE_REPOSITORY "https://gitlab.lanl.gov/laristra/flecsi.git" CACHE STRING
  "Set the FleCSI source repository")
set(SOURCE_BRANCH "devel" CACHE STRING
  "Set the source repository branch, e.g., devel")
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
      IMAGE_REPO=${IMAGE_REPOSITORY}
      IMAGE_BRANCH=${IMAGE_BRANCH}
      SOURCE_REPO=${SOURCE_REPOSITORY}
      SOURCE_BRANCH=${SOURCE_BRANCH}
      SUBDIR=devel
      OS=${REGISTRY}:${image}
    DEPENDS push-${image}
  )

  add_image(util-${image} ${REGISTRY}:util-${image}
    util/environment
    ARGS
      IMAGE_REPO=${IMAGE_REPOSITORY}
      IMAGE_BRANCH=${IMAGE_BRANCH}
      SUBDIR=util
      OS=${REGISTRY}:${image}
    DEPENDS push-${image}
  )

  add_image(flecsi-${image} ${REGISTRY}:flecsi-${image}
    devel/flecsi
    ARGS
      OS=${REGISTRY}:devel-${image}
    DEPENDS push-devel-${image}
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
# Legion unit test image
#------------------------------------------------------------------------------#

add_image(legion-unit ${REGISTRY}:legion-unit
  devel/legion-unit
  ARGS
    OS=${REGISTRY}:devel-ubuntu-20.04
  DEPENDS push-devel-ubuntu-20.04
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

<<<<<<< HEAD
#------------------------------------------------------------------------------#
# Build auxiliary
#------------------------------------------------------------------------------#

if(EXISTS ${CMAKE_SOURCE_DIR}/aux/gnu-centos-7.8.tbz)
  file(COPY ${CMAKE_SOURCE_DIR}/aux/gnu-centos-7.8.tbz
    DESTINATION ${CMAKE_BINARY_DIR}
  )
endif()

if(EXISTS ${CMAKE_SOURCE_DIR}/aux/gnu-clearlinux.tbz)
  file(COPY ${CMAKE_SOURCE_DIR}/aux/gnu-clearlinux.tbz
    DESTINATION ${CMAKE_BINARY_DIR}
  )
endif()

#------------------------------------------------------------------------------#
# Environment files
#------------------------------------------------------------------------------#

||||||| parent of c360ef39... Adding gnu builds
=======
#------------------------------------------------------------------------------#
# Build auxiliary
#------------------------------------------------------------------------------#

file(COPY ${CMAKE_SOURCE_DIR}/aux/gnu-centos-7.8.tbz
  DESTINATION ${CMAKE_BINARY_DIR}
)

file(COPY ${CMAKE_SOURCE_DIR}/aux/gnu-clearlinux.tbz
  DESTINATION ${CMAKE_BINARY_DIR}
)

#------------------------------------------------------------------------------#
# Environment files
#------------------------------------------------------------------------------#

>>>>>>> c360ef39... Adding gnu builds
file(COPY ${CMAKE_SOURCE_DIR}/config/dircolors
  DESTINATION ${CMAKE_BINARY_DIR}
)

file(COPY ${CMAKE_SOURCE_DIR}/config/bashrc
  DESTINATION ${CMAKE_BINARY_DIR}
)

file(COPY ${CMAKE_SOURCE_DIR}/config/vimrc
  DESTINATION ${CMAKE_BINARY_DIR}
)
