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

set(ctrl_HEADERS
  point_walker.hh
)

set(ctrl_SOURCES
)

#------------------------------------------------------------------------------#
# Export header list to parent scope.
#------------------------------------------------------------------------------#

set(ctrl_HEADERS
  ${ctrl_HEADERS}
  PARENT_SCOPE
)

#------------------------------------------------------------------------------#
# Export source list to parent scope.
#------------------------------------------------------------------------------#

set(ctrl_SOURCES
  ${ctrl_SOURCES}
  PARENT_SCOPE
)

#------------------------------------------------------------------------------#
# Unit tests.
#
# Note: These do not use the 'add_unit' interface because they are meant
#       to test different low-level control model features. The unit test
#       framework in FleCSI uses the control model itself, and, therefore
#       cannot be used for these tests.
#------------------------------------------------------------------------------#

add_executable(cycle
  test/cycle.cc
  test/cycle_control.hh
  test/package_a.hh
  test/package_b.hh
  test/package_c.hh)
target_link_libraries(cycle FleCSI)
target_link_libraries(cycle ${FLECSI_LIBRARY_DEPENDENCIES}
  ${CMAKE_THREAD_LIBS_INIT})
