#~----------------------------------------------------------------------------~#
# Copyright (c) 2014 Los Alamos National Security, LLC
# All rights reserved.
#~----------------------------------------------------------------------------~#

set(control_HEADERS
  control.h
  phase_walker.h
)

set(control_SOURCES
)

#------------------------------------------------------------------------------#
# Export header list to parent scope.
#------------------------------------------------------------------------------#

set(control_HEADERS
  ${control_HEADERS}
  PARENT_SCOPE
)

#------------------------------------------------------------------------------#
# Export source list to parent scope.
#------------------------------------------------------------------------------#

set(control_SOURCES
  ${control_SOURCES}
  PARENT_SCOPE
)

#------------------------------------------------------------------------------#
# Unit tests.
#------------------------------------------------------------------------------#

cinch_add_unit(control
  SOURCES
    test/control.cc
  LIBRARIES
    ${FLECSI_LIBRARY_DEPENDENCIES}
)

#------------------------------------------------------------------------------#
# Runtime executable
#
# This is used to test the basic runtime logic of the control model. It will
# not work as a normal unit test.
#------------------------------------------------------------------------------#

add_executable(runtime
  runtime_main.cc
  test/runtime_model.cc
)

target_link_libraries(runtime ${CINCH_RUNTIME_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT})
