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
  OUTPUT_VARIABLE version)
string(REGEX REPLACE "\n$" "" version "${version}")

string(REGEX MATCH "^devel-.*" devel ${version})

if(devel)
  string(REPLACE "-" ";" version_list ${version})
  list(GET version_list 1 ${PROJECT_NAME}_MAJOR)
  list(GET version_list 2 ${PROJECT_NAME}_MINOR)
  list(GET version_list 3 ${PROJECT_NAME}_PATCH)
  set(${PROJECT_NAME}_VERSION "devel-${${PROJECT_NAME}_MAJOR}.${${PROJECT_NAME}_MINOR}.${${PROJECT_NAME}_PATCH}")
else()
  # Count the dots
  string(REGEX MATCHALL "\\." dots ${version})
  list(LENGTH dots dots)

  # Convert the version into a list
  string(REPLACE "." ";" version_list ${version})

  list(GET version_list 0 ${PROJECT_NAME}_MAJOR)

  if(${dots} STREQUAL "0")
    set(${PROJECT_NAME}_MINOR "0")
    set(${PROJECT_NAME}_PATCH "0")
  elseif(${dots} STREQUAL "1")
    list(GET version_list 1 ${PROJECT_NAME}_MINOR)
    set(${PROJECT_NAME}_PATCH "0")
  else()
    list(GET version_list 1 ${PROJECT_NAME}_MINOR)
    list(GET version_list 2 ${PROJECT_NAME}_PATCH)
  endif()

  set(${PROJECT_NAME}_VERSION "${${PROJECT_NAME}_MAJOR}.${${PROJECT_NAME}_MINOR}.${${PROJECT_NAME}_PATCH}")

endif()
