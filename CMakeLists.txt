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
set(BRANCH "gitlab-ci" CACHE STRING
  "Set the repository branch, e.g., gitlab-ci")

#------------------------------------------------------------------------------#
# OS Images
#------------------------------------------------------------------------------#

add_image(centos-7.8
  docker.io/laristra/flecsi-ci:centos-7.8
  os/centos-7.8
)

add_image(centos-8
  docker.io/laristra/flecsi-ci:centos-8
  os/centos-8
)

add_image(fedora-31
  docker.io/laristra/flecsi-ci:fedora-31
  os/fedora-31
)

add_image(fedora-32
  docker.io/laristra/flecsi-ci:fedora-32
  os/fedora-32
)

add_image(ubuntu-20.04
  docker.io/laristra/flecsi-ci:ubuntu-20.04
  os/ubuntu-20.04
)

add_image(clearlinux
  docker.io/laristra/flecsi-ci:clearlinux
  os/clearlinux
)

#------------------------------------------------------------------------------#
# Environment Images
#------------------------------------------------------------------------------#

add_image(devel-centos-8 docker.io/laristra/flecsi-ci:devel-centos-8
  devel/environment
  ARGS
    REPO=${REPOSITORY}
    BRANCH=${BRANCH}
    VERSIONING=devel
    OS=docker.io/laristra/flecsi-ci:centos-8
    DEPENDS push-centos-8
)

add_image(devel-centos-7.8 docker.io/laristra/flecsi-ci:devel-centos-7.8
  devel/environment
  ARGS
    REPO=${REPOSITORY}
    BRANCH=${BRANCH}
    VERSIONING=devel
    OS=docker.io/laristra/flecsi-ci:centos-7.8
    DEPENDS push-centos-7.8
)

add_image(devel-fedora-31 docker.io/laristra/flecsi-ci:devel-fedora-31
  devel/environment
  ARGS
    REPO=${REPOSITORY}
    BRANCH=${BRANCH}
    VERSIONING=devel
    OS=docker.io/laristra/flecsi-ci:fedora-31
  DEPENDS push-fedora-31
)

add_image(devel-fedora-32 docker.io/laristra/flecsi-ci:devel-fedora-32
  devel/environment
  ARGS
    REPO=${REPOSITORY}
    BRANCH=${BRANCH}
    VERSIONING=devel
    OS=docker.io/laristra/flecsi-ci:fedora-32
  DEPENDS push-fedora-32
)

add_image(devel-ubuntu-20.04 docker.io/laristra/flecsi-ci:devel-ubuntu-20.04
  devel/environment
  ARGS
    REPO=${REPOSITORY}
    BRANCH=${BRANCH}
    VERSIONING=devel
    OS=docker.io/laristra/flecsi-ci:ubuntu-20.04
  DEPENDS push-ubuntu-20.04
)

add_image(devel-clearlinux docker.io/laristra/flecsi-ci:devel-clearlinux
  devel/environment
  ARGS
    REPO=${REPOSITORY}
    BRANCH=${BRANCH}
    VERSIONING=devel
    OS=docker.io/laristra/flecsi-ci:clearlinux
  DEPENDS push-clearlinux
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
