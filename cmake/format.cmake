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

option(FORMAT_ONLY
  "Enable format-only mode: no other targets will be create" OFF)
mark_as_advanced(FORMAT_ONLY)

set(ClangFormat_VERSION "8" CACHE STRING
  "Set the required version (major[.minor[.patch]]) of clang-format")
mark_as_advanced(ClangFormat_VERSION)

if(NOT TARGET format)
  add_custom_target(format)
endif()

find_package(ClangFormat ${ClangFormat_VERSION} EXACT)
find_package(Git)

if(ClangFormat_FOUND AND GIT_FOUND AND EXISTS ${PROJECT_SOURCE_DIR}/.git)

  execute_process(COMMAND ${GIT_EXECUTABLE} -C ${PROJECT_SOURCE_DIR}
    ls-files OUTPUT_VARIABLE _FILES OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET)

  string(REGEX REPLACE "\n" ";" _FILES "${_FILES}")

  set(FORMAT_SOURCES)

  foreach(_FILE ${_FILES})
    if(_FILE MATCHES "\\.(hh|cc)$")
      list(APPEND FORMAT_SOURCES "${PROJECT_SOURCE_DIR}/${_FILE}")
    endif()
  endforeach()

  add_custom_target(format-${PROJECT_NAME}
    COMMAND ${ClangFormat_EXECUTABLE} -style=file -i ${FORMAT_SOURCES})

else()

  add_custom_target(format-${PROJECT_NAME}
    COMMAND ${CMAKE_COMMAND} -E echo
      "No clang-format v${ClangFormat_VERSION} or newer found")

endif()

add_dependencies(format format-${PROJECT_NAME})
