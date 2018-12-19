#
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
#

option(FLECSI_ENABLE_FLOG "Enable FleCSI Logging Utility (FLOG)" OFF)

if(FLECSI_ENABLE_FLOG)

  set(FLOG_STRIP_LEVEL "0" CACHE STRING "Set the clog strip level")
  option(FLOG_COLOR_OUTPUT "Enable colorized clog logging" ON)
  option(FLOG_ENABLE_TAGS "Enable tag groups" ${ENABLE_BOOST})

  if(FLOG_ENABLE_TAGS)
    set(FLOG_TAG_BITS "64" CACHE STRING
      "Select the number of bits to use for tag groups")
  endif()

  option(FLOG_ENABLE_EXTERNAL
    "Enable messages that are defined at external scope" OFF)

  if(MPI_${MPI_LANGUAGE}_FOUND)
      option(FLOG_ENABLE_MPI "Enable clog MPI functions" ${ENABLE_MPI})

      if(FLOG_ENABLE_MPI)
        find_package(Threads)
      endif()
  endif()

  option(FLOG_DEBUG "Enable clog debugging" OFF)

  list(APPEND CINCH_RUNTIME_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})
endif()
