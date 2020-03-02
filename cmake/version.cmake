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

execute_process(COMMAND ${CMAKE_SOURCE_DIR}/VERSION
  OUTPUT_VARIABLE version_output)
string(REGEX REPLACE "\n$" "" version "${version_output}")

string(REPLACE " " ";" fields ${version_output})

list(GET fields 1 ${PROJECT_NAME}_VERSION)
list(SUBLIST fields 2 -1 rest)
set(${PROJECT_NAME}_COMMITS)
if(rest)
  string(REPLACE ";" "\ " commits "${rest}")
  string(REGEX REPLACE "([()])" "" commits "${commits}")
  string(STRIP commits "${commits}")
  set(${PROJECT_NAME}_COMMITS ${commits})
endif()
