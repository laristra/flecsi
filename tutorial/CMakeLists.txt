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
# Unit tests
#------------------------------------------------------------------------------#

option(ENABLE_UNIT_TESTS "Enable unit tests" OFF)
mark_as_advanced(ENABLE_UNIT_TESTS)

if(ENABLE_UNIT_TESTS)
  enable_testing()
endif()

function(add_unit name)
  set(options)
  set(one_value_args)
  set(multi_value_args ARGUMENTS)

  cmake_parse_arguments(unit "${options}" "${one_value_args}"
    "${multi_value_args}" ${ARGN})

  add_test(NAME ${name}
    COMMAND
      $<TARGET_FILE:${name}> ${unit_ARGUMENTS}
    WORKING_DIRECTORY
      ${CMAKE_BINARY_DIR}
  )
endfunction()

#------------------------------------------------------------------------------#
# FleCSI
#------------------------------------------------------------------------------#

find_package(FleCSI REQUIRED)
include_directories(SYSTEM ${FleCSI_INCLUDE_DIRS})

#------------------------------------------------------------------------------#
# FleCSI-Tutorial library.
#------------------------------------------------------------------------------#

add_library(FleCSI-Tutorial
  ${CMAKE_SOURCE_DIR}/specialization/dummy/dummy.cc
)

include_directories(${PROJECT_SOURCE_DIR})

#------------------------------------------------------------------------------#
# 1-runtime
#------------------------------------------------------------------------------#

add_executable(runtime-minimal
  1-runtime/1-minimal.cc
)
target_link_libraries(runtime-minimal ${FleCSI_LIBRARIES}
  ${FleCSI_LIBRARY_DEPENDENCIES})
add_unit(runtime-minimal)

add_executable(runtime-program_options
  1-runtime/2-program_options.cc
)
target_link_libraries(runtime-program_options ${FleCSI_LIBRARIES}
  ${FleCSI_LIBRARY_DEPENDENCIES})
file(TOUCH ${CMAKE_BINARY_DIR}/input.txt)
add_unit(runtime-program_options
  ARGUMENTS
    input.txt
)

add_executable(runtime-flog
  1-runtime/3-flog.cc
)
target_link_libraries(runtime-flog ${FleCSI_LIBRARIES}
  ${FleCSI_LIBRARY_DEPENDENCIES})
add_unit(runtime-flog)

#------------------------------------------------------------------------------#
# 2-control
#------------------------------------------------------------------------------#

add_executable(control-simple
  2-control/1-simple.cc
  2-control/1-simple.hh
)
target_link_libraries(control-simple ${FleCSI_LIBRARIES}
  ${FleCSI_LIBRARY_DEPENDENCIES})
add_unit(control-simple)

add_executable(control-cycle
  2-control/2-cycle.cc
  2-control/2-cycle.hh
)
target_link_libraries(control-cycle ${FleCSI_LIBRARIES}
  ${FleCSI_LIBRARY_DEPENDENCIES})
add_unit(control-cycle)

add_executable(control-subcycle
  2-control/2-cycle.cc
  2-control/2-subcycle.hh
)
target_compile_definitions(control-subcycle PUBLIC SUBCYCLE)
target_link_libraries(control-subcycle ${FleCSI_LIBRARIES}
  ${FleCSI_LIBRARY_DEPENDENCIES})
add_unit(control-subcycle)

if(ENABLE_DEVELOPER_MODE)
  add_custom_target(pngs
    ${CMAKE_BINARY_DIR}/control-simple --control-model &&
    ${DOT_EXECUTABLE} -Tpdf simple-control-model.dot > simple.pdf &&
    ${IMAGEMAGICK_CONVERT_EXECUTABLE} -density 600 simple.pdf simple.png &&
    ${CMAKE_BINARY_DIR}/control-cycle --control-model &&
    ${DOT_EXECUTABLE} -Tpdf cycle-control-model.dot > cycle.pdf &&
    ${IMAGEMAGICK_CONVERT_EXECUTABLE} -density 600 cycle.pdf cycle.png &&
    ${CMAKE_BINARY_DIR}/control-subcycle --control-model &&
    ${DOT_EXECUTABLE} -Tpdf subcycle-control-model.dot > subcycle.pdf
    ${IMAGEMAGICK_CONVERT_EXECUTABLE} -density 600 subcycle.pdf subcycle.png
  )
endif()
