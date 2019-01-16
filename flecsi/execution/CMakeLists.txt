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
  common/function_handle.h
  common/launch.h
  common/processor.h
  context.h
  execution.h
  global_object_wrapper.h
  task.h
)

set(execution_SOURCES
)

if(FLECSI_RUNTIME_MODEL STREQUAL "legion")

  set(execution_HEADERS
    ${execution_HEADERS}
    legion/context_policy.h
    legion/execution_policy.h
    legion/internal_task.h
    legion/mapper.h
    legion/runtime.h
    legion/task_wrapper.h
    legion/tasks.h
  )

  set(execution_SOURCES
    ${execution_SOURCES}
    legion/context_policy.cc
    legion/runtime.cc
  )

  set(UNIT_POLICY LEGION)
  set(RUNTIME_DRIVER legion/runtime.cc)

elseif(FLECSI_RUNTIME_MODEL STREQUAL "mpi")

  set(execution_HEADERS
    ${execution_HEADERS}
    mpi/context_policy.h
    mpi/execution_policy.h
    mpi/runtime.h
  )

  set(execution_SOURCES
    ${execution_SOURCES}
    mpi/context_policy.cc
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

ftest_add_unit(color
  SOURCES
    test/color.cc
  THREADS
    4
)
