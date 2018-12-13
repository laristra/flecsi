#~----------------------------------------------------------------------------~#
# Copyright (c) 2014 Los Alamos National Security, LLC
# All rights reserved.
#~----------------------------------------------------------------------------~#

add_executable(io-poc
  io-poc.cc
  ../flecsi/control/runtime_main.cc
)

include_directories(${CMAKE_SOURCE_DIR})

target_link_libraries(io-poc
  ${CINCH_RUNTIME_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
  ${FLECSI_LIBRARY_DEPENDENCIES}
)
