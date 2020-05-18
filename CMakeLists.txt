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

add_image(centos-8 docker.io/tuxfan/flecsi-ci:centos-8 os/centos-8)
add_image(fedora-31 docker.io/tuxfan/flecsi-ci:fedora-31 os/fedora-31)
#add_image(fedora-32 docker.io/tuxfan/flecsi-ci:fedora-32 os/fedora-31)

#------------------------------------------------------------------------------#
# Environment Images
#------------------------------------------------------------------------------#

add_image(devel-centos-8 docker.io/tuxfan/flecsi-ci:devel-centos-8
  devel/environment
  ARGS
    REPO=${REPOSITORY}
    BRANCH=${BRANCH}
    VERSIONING=devel
    OS=docker.io/tuxfan/flecsi-ci:centos-8
  DEPENDS centos-8
)
