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
  common/command_line_options.hh
  common/function_handle.hh
  common/processor.hh
  context.hh
  execution.hh
  global_object_wrapper.hh
  internal.hh
  kernel_interface.hh
  reduction.hh
  task.hh
  task_interface.hh
)

set(execution_SOURCES
)

if(FLECSI_RUNTIME_MODEL STREQUAL "legion")

  set(execution_HEADERS
    ${execution_HEADERS}
    legion/context_policy.hh
    legion/enactment/bind_accessors.hh
    legion/enactment/task_wrapper.hh
    legion/enactment/unbind_accessors.hh
    legion/execution_policy.hh
    legion/future.hh
    legion/internal_task.hh
    legion/invocation/init_args.hh
    legion/invocation/task_epilogue.hh
    legion/invocation/task_prologue.hh
    legion/mapper.hh
    legion/reduction_wrapper.hh
    legion/runtime.hh
    legion/tasks.hh
  )

  set(execution_SOURCES
    ${execution_SOURCES}
    legion/context_policy.cc
    legion/runtime.cc
  )

elseif(FLECSI_RUNTIME_MODEL STREQUAL "mpi")

  set(execution_HEADERS
    ${execution_HEADERS}
    mpi/context_policy.hh
    mpi/execution_policy.hh
    mpi/reduction_wrapper.hh
    mpi/runtime.hh
  )

  set(execution_SOURCES
    ${execution_SOURCES}
    mpi/context_policy.cc
    mpi/runtime.cc
  )

elseif(FLECSI_RUNTIME_MODEL STREQUAL "hpx")

  set(execution_HEADERS
    ${execution_HEADERS}
    hpx/runtime.hh
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

if(ENABLE_KOKKOS)
  ftest_add_unit(kokkos
    SOURCES
      test/kokkos.cc
  )
endif()

#ftest_add_unit(reduction
#  SOURCES
#    test/reduction.cc
#)

ftest_add_unit(task
  SOURCES
    test/task.cc
)

ftest_add_unit(task_attributes
  SOURCES
    test/task_attributes.cc
)

ftest_add_unit(gpu_task
  SOURCES
    test/gpu_task.cc
)
