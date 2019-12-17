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

set(runtime_HEADERS
  backend.hh
  context.hh
  program_options.hh
  types.hh
)

set(runtime_SOURCES
)

if(FLECSI_RUNTIME_MODEL STREQUAL "legion")

  set(runtime_HEADERS
    ${runtime_HEADERS}
    legion/context.hh
    legion/mapper.hh
    legion/tasks.hh
  )

  set(runtime_SOURCES
    ${runtime_SOURCES}
    legion/context.cc
  )

elseif(FLECSI_RUNTIME_MODEL STREQUAL "mpi")

  set(runtime_HEADERS
    ${runtime_HEADERS}
    mpi/context.hh
  )

  set(runtime_SOURCES
    ${runtime_SOURCES}
    mpi/context.cc
  )

  elseif(FLECSI_RUNTIME_MODEL STREQUAL "hpx")

  set(runtime_HEADERS
    ${runtime_HEADERS}
    hpx/context.hh
  )

  set(runtime_SOURCES
    ${runtime_SOURCES}
    hpx/context.cc
  )

endif()

#------------------------------------------------------------------------------#
# Export header list to parent scope.
#------------------------------------------------------------------------------#

set(runtime_HEADERS
  ${runtime_HEADERS}
  PARENT_SCOPE
)

#------------------------------------------------------------------------------#
# Export source list to parent scope.
#------------------------------------------------------------------------------#

set(runtime_SOURCES
  ${runtime_SOURCES}
  PARENT_SCOPE
)
