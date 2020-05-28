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
# Compiler version requirements.
#------------------------------------------------------------------------------#

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.0")
    message(FATAL_ERROR "Version 9.0 of gnu compilers required!")
  endif()
endif()

#------------------------------------------------------------------------------#
# Auxiliary Tools
#------------------------------------------------------------------------------#

option(ENABLE_DEVELOPER_MODE "Enable devel targets" OFF)

if(ENABLE_DEVELOPER_MODE)
  find_program(DOT_EXECUTABLE dot)
  if(DOT_EXECUTABLE-NOTFOUND)
    message(FATAL_ERROR "Failed to find dot executable")
  endif()

  find_package(ImageMagick COMPONENTS convert REQUIRED)
endif()

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

add_executable(simple
  2-control/1-simple.cc
  2-control/1-simple.hh
)
target_link_libraries(simple ${FleCSI_LIBRARIES}
  ${FleCSI_LIBRARY_DEPENDENCIES})

add_executable(cycle
  2-control/2-cycle.cc
  2-control/2-cycle.hh
)
target_link_libraries(cycle ${FleCSI_LIBRARIES}
  ${FleCSI_LIBRARY_DEPENDENCIES})

add_executable(subcycle
  2-control/2-cycle.cc
  2-control/2-subcycle.hh
)
target_compile_definitions(subcycle PUBLIC SUBCYCLE)
target_link_libraries(subcycle ${FleCSI_LIBRARIES}
  ${FleCSI_LIBRARY_DEPENDENCIES})

if(ENABLE_DEVELOPER_MODE)
  add_custom_target(pngs
    ${CMAKE_BINARY_DIR}/simple --control-model &&
    ${DOT_EXECUTABLE} -Tpdf simple-control-model.dot > simple.pdf &&
    ${IMAGEMAGICK_CONVERT_EXECUTABLE} -density 600 simple.pdf simple.png &&
    ${CMAKE_BINARY_DIR}/cycle --control-model &&
    ${DOT_EXECUTABLE} -Tpdf cycle-control-model.dot > cycle.pdf &&
    ${IMAGEMAGICK_CONVERT_EXECUTABLE} -density 600 cycle.pdf cycle.png &&
    ${CMAKE_BINARY_DIR}/subcycle --control-model &&
    ${DOT_EXECUTABLE} -Tpdf subcycle-control-model.dot > subcycle.pdf
    ${IMAGEMAGICK_CONVERT_EXECUTABLE} -density 600 subcycle.pdf subcycle.png
  )
endif()

#------------------------------------------------------------------------------#
# 3-tasks
#------------------------------------------------------------------------------#

#add_executable(trivial 2-tasks/1-trivial.cc)
#target_link_libraries(trivial ${FleCSI_LIBRARIES}
#  ${FleCSI_LIBRARY_DEPENDENCIES})
