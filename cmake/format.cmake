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

if(NOT TARGET format)
  add_custom_target(format)
endif()

if(EXISTS ${PROJECT_SOURCE_DIR}/.clang-format)

  set(CLANG-FORMAT_BASE-VERSION "7.0.0")
  set(GIT-BASE-VERSION "1.8.5")

  find_package(CLANG_FORMAT ${CLANG-FORMAT_BASE-VERSION})
  find_package(Git ${GIT-BASE-VERSION})

  if (CLANG_FORMAT_FOUND AND GIT_FOUND AND EXISTS ${PROJECT_SOURCE_DIR}/.git)

    execute_process(COMMAND ${GIT_EXECUTABLE} -C ${PROJECT_SOURCE_DIR} ls-files OUTPUT_VARIABLE _FILES OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)

    string(REGEX REPLACE "\n" ";" _FILES "${_FILES}")

    set(FORMAT_SOURCES)

    foreach(_FILE ${_FILES})
      if(_FILE MATCHES "\\.(hh|cc)$")
        list(APPEND FORMAT_SOURCES "${PROJECT_SOURCE_DIR}/${_FILE}")
      endif()
    endforeach()

    add_custom_target(format-${PROJECT_NAME} COMMAND ${CLANG_FORMAT_EXECUTABLE} -style=file -i ${FORMAT_SOURCES})

  else()

    add_custom_target(format-${PROJECT_NAME} COMMAND ${CMAKE_COMMAND} -E echo "No clang-format v${CLANG-FORMAT_BASE-VERSION} or newer found")

  endif()

else()

  add_custom_target(format-${PROJECT_NAME} COMMAND ${CMAKE_COMMAND} -E echo "No ${PROJECT_SOURCE_DIR}/.clang-format found")

endif()
add_dependencies(format format-${PROJECT_NAME})
