#------------------------------------------------------------------------------#
#   @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
#  /@@/////  /@@          @@////@@ @@////// /@@
#  /@@       /@@  @@@@@  @@    // /@@       /@@
#  /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
#  /@@////   /@@/@@@@@@@/@@       ////////@@/@@
#  /@@       /@@/@@//// //@@    @@       /@@/@@
#  /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
#  //       ///  //////   //////  ////////  //
#
#  Copyright (c) 2016, Los Alamos National Security, LLC
#  All rights reserved.
#------------------------------------------------------------------------------#

include(CMakeDependentOption)

option(ENABLE_FLOG "Enable FleCSI Logging Utility (FLOG)" OFF)

cmake_dependent_option(FLOG_ENABLE_COLOR_OUTPUT
  "Enable colorized flog logging" ON "ENABLE_FLOG" OFF)
mark_as_advanced(FLOG_ENABLE_COLOR_OUTPUT)

cmake_dependent_option(FLOG_ENABLE_TAGS "Enable tag groups" ON
  "ENABLE_FLOG" OFF)
mark_as_advanced(FLOG_ENABLE_TAGS)

cmake_dependent_option(FLOG_ENABLE_MPI "Enable flog MPI support" ON
  "ENABLE_FLOG;ENABLE_MPI" OFF)
mark_as_advanced(FLOG_ENABLE_MPI)

cmake_dependent_option(FLOG_ENABLE_DEBUG "Enable flog debug mode" OFF
  "ENABLE_FLOG" OFF)
mark_as_advanced(FLOG_ENABLE_DEBUG)

cmake_dependent_option(FLOG_ENABLE_EXTERNAL
  "Enable messages that are defined at namespace scope" OFF "ENABLE_FLOG" ON)
mark_as_advanced(FLOG_ENABLE_EXTERNAL)

cmake_dependent_option(FLOG_ENABLE_DEVELOPER_MODE
  "Enable internal FleCSI developer messages" OFF "ENABLE_FLOG" OFF)
mark_as_advanced(FLOG_ENABLE_DEVELOPER_MODE)

set(FLOG_TAG_BITS "1024" CACHE STRING
  "Select the number of bits to use for tag groups")
mark_as_advanced(FLOG_TAG_BITS)

set(FLOG_SERIALIZATION_INTERVAL "100" CACHE STRING
  "Select the frequency of message serialization in number of tasks")
mark_as_advanced(FLOG_SERIALIZATION_INTERVAL)

set(FLOG_SERIALIZATION_THRESHOLD "1024" CACHE STRING
  "Select the threshold size in number of messages")
mark_as_advanced(FLOG_SERIALIZATION_THRESHOLD)

if(ENABLE_FLOG)
  set(FLOG_STRIP_LEVELS 0 1 2 3 4)

  if(NOT FLOG_STRIP_LEVEL)
    list(GET FLOG_STRIP_LEVELS 0 FLOG_STRIP_LEVEL)
  endif()

  set(FLOG_STRIP_LEVEL ${FLOG_STRIP_LEVEL} CACHE STRING
    "Set the flog strip level (0-4)")
  mark_as_advanced(FLOG_STRIP_LEVEL)

  set_property(CACHE FLOG_STRIP_LEVEL PROPERTY STRINGS ${FLOG_STRIP_LEVELS})
endif()

if(FLOG_ENABLE_MPI)
  find_package(Threads)
  list(APPEND FLECSI_LIBRARY_DEPENDENCIES ${CMAKE_THREAD_LIBS_INIT})
endif()
