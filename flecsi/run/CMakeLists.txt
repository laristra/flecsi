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

set(run_HEADERS
  backend.hh
  context.hh
  types.hh
)

set(run_SOURCES
)

if(FLECSI_RUNTIME_MODEL STREQUAL "legion")

  set(run_HEADERS
    ${run_HEADERS}
    leg/context.hh
    leg/mapper.hh
  )

  set(run_SOURCES
    ${run_SOURCES}
    leg/context.cc
  )

elseif(FLECSI_RUNTIME_MODEL STREQUAL "mpi")

  set(run_HEADERS
    ${run_HEADERS}
    mpi/context.hh
  )

  set(run_SOURCES
    ${run_SOURCES}
    mpi/context.cc
  )

elseif(FLECSI_RUNTIME_MODEL STREQUAL "charm")

  set(runtime_HEADERS
    ${runtime_HEADERS}
    charm/context.hh
    charm/mapper.hh
    charm/tasks.hh
  )

  set(runtime_SOURCES
    ${runtime_SOURCES}
    charm/context.cc
    charm/context.decl.h
  )

endif()

#------------------------------------------------------------------------------#
# Export header list to parent scope.
#------------------------------------------------------------------------------#

set(run_HEADERS
  ${run_HEADERS}
  PARENT_SCOPE
)

#------------------------------------------------------------------------------#
# Export source list to parent scope.
#------------------------------------------------------------------------------#

set(run_SOURCES
  ${run_SOURCES}
  PARENT_SCOPE
)

#------------------------------------------------------------------------------#
# Unit tests.
#------------------------------------------------------------------------------#

add_unit(program-options
  SOURCES
    test/program-options.cc
)
