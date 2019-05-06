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

#------------------------------------------------------------------------------#
# Add header files. Note that these will be "exported" to the parent
# scope below.
#------------------------------------------------------------------------------#

set(execution_HEADERS
  common/function_handle.h
  common/command_line_options.h
  common/launch.h
  common/processor.h
  common/topology_instance.h
  context.h
  internal.h
  execution.h
  global_object_wrapper.h
  reduction.h
  task.h
)

set(execution_SOURCES
  common/launch.cc
)

if(FLECSI_RUNTIME_MODEL STREQUAL "legion")

  set(execution_HEADERS
    ${execution_HEADERS}
    legion/context_policy.h
    legion/execution_policy.h
    legion/internal_task.h
    legion/mapper.h
    legion/reduction_wrapper.h
    legion/runtime.h
    legion/enactment/task_wrapper.h
    legion/enactment/finalize_views.h
    legion/enactment/init_views.h
    legion/tasks.h
  )

  set(execution_SOURCES
    ${execution_SOURCES}
    legion/context_policy.cc
    legion/runtime.cc
  )

elseif(FLECSI_RUNTIME_MODEL STREQUAL "mpi")

  set(execution_HEADERS
    ${execution_HEADERS}
    mpi/context_policy.h
    mpi/execution_policy.h
    mpi/reduction_wrapper.h
    mpi/runtime.h
  )

  set(execution_SOURCES
    ${execution_SOURCES}
    mpi/context_policy.cc
    mpi/runtime.cc
  )

elseif(FLECSI_RUNTIME_MODEL STREQUAL "hpx")

  set(execution_HEADERS
    ${execution_HEADERS}
    hpx/runtime.h
  )

  set(execution_SOURCES
    ${execution_SOURCES}
    hpx/runtime.cc
  )

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
    4 # Warning: This number is hard-coded into the the test.
)

ftest_add_unit(global-object
  SOURCES
    test/global-object.cc
)

ftest_add_unit(task
  SOURCES
    test/task.cc
)
