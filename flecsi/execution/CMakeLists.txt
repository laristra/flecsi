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

#------------------------------------------------------------------------------#
# Add header files. Note that these will be "exported" to the parent
# scope below.
#------------------------------------------------------------------------------#

set(execution_HEADERS
  context.h
)

set(execution_SOURCES
)

if(FLECSI_RUNTIME_MODEL STREQUAL "legion")

  set(execution_HEADERS
    ${execution_HEADERS}
    legion/runtime.h
  )

  set(execution_SOURCES
    ${execution_SOURCES}
    legion/runtime.cc
  )

  set(UNIT_POLICY LEGION)
  set(RUNTIME_DRIVER legion/runtime.cc)

elseif(FLECSI_RUNTIME_MODEL STREQUAL "mpi")

  set(execution_HEADERS
    ${execution_HEADERS}
    mpi/runtime.h
  )

  set(execution_SOURCES
    ${execution_SOURCES}
    mpi/runtime.cc
  )

  set(UNIT_POLICY MPI)
  set(RUNTIME_DRIVER mpi/runtime.cc)

elseif(FLECSI_RUNTIME_MODEL STREQUAL "hpx")

  set(execution_HEADERS
    ${execution_HEADERS}
    hpx/runtime.h
  )

  set(execution_SOURCES
    ${execution_SOURCES}
    hpx/runtime.cc
  )

  set(UNIT_POLICY HPX)
  set(RUNTIME_DRIVER hpx/runtime.cc)

endif()

#------------------------------------------------------------------------------#
# Export header list to parent scope.
#------------------------------------------------------------------------------#

set(execution_HEADERS
  ${execution_HEADERS}
  PARENT_SCOPE
)

#------------------------------------------------------------------------------#
# Export source list to parent scope.
#------------------------------------------------------------------------------#

set(execution_SOURCES
  ${execution_SOURCES}
  PARENT_SCOPE
)

#------------------------------------------------------------------------------#
# Unit tests.
#------------------------------------------------------------------------------#

# FIXME: Remove this when unit tests are working again
add_executable(runtime_initialization
  ${CINCH_SOURCE_DIR}/cinch/runtime.cc
  ../control/test/unit.cc
  ../utils/flog/packet.cc
  ${RUNTIME_DRIVER}
)

target_link_libraries(runtime_initialization ${CINCH_RUNTIME_LIBRARIES}
  ${COLORING_LIBRARIES}
)
# // FIXME: Remove to here
