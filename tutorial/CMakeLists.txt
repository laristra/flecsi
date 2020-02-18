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

#------------------------------------------------------------------------------#
# Add local module path.
#------------------------------------------------------------------------------#

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/../cmake)

#------------------------------------------------------------------------------#
# Project.
#------------------------------------------------------------------------------#

project(FleCSI-Tutorial LANGUAGES CXX C)

set(CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)

#------------------------------------------------------------------------------#
# FleCSI
#------------------------------------------------------------------------------#

find_package(FleCSI REQUIRED)
include_directories(${FleCSI_INCLUDE_DIRS})

#------------------------------------------------------------------------------#
# FleCSI-Tutorial library.
#------------------------------------------------------------------------------#

add_library(FleCSI-Tutorial
  ${CMAKE_SOURCE_DIR}/specialization/dummy/dummy.cc
)

#------------------------------------------------------------------------------#
# 1-runtime
#------------------------------------------------------------------------------#

add_executable(minimal 1-runtime/1-minimal.cc)
target_link_libraries(minimal ${FleCSI_LIBRARIES}
  ${FleCSI_LIBRARY_DEPENDENCIES})

add_executable(program_options 1-runtime/2-program_options.cc)
target_link_libraries(program_options ${FleCSI_LIBRARIES}
  ${FleCSI_LIBRARY_DEPENDENCIES})

add_executable(flog 1-runtime/3-flog.cc)
target_link_libraries(flog ${FleCSI_LIBRARIES}
  ${FleCSI_LIBRARY_DEPENDENCIES})

#------------------------------------------------------------------------------#
# 2-control
#------------------------------------------------------------------------------#

#------------------------------------------------------------------------------#
# 3-control
#------------------------------------------------------------------------------#

#add_executable(trivial 2-tasks/1-trivial.cc)
#target_link_libraries(trivial ${FleCSI_LIBRARIES}
#  ${FleCSI_LIBRARY_DEPENDENCIES})
