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
  common/launch.hh
  common/task_attributes.hh
  execution.hh
  internal.hh
  kernel_interface.hh
  reduction.hh
  backend.hh
)

set(execution_SOURCES
  common/launch.cc
)

if(FLECSI_RUNTIME_MODEL STREQUAL "legion")

  set(execution_HEADERS
    ${execution_HEADERS}
    legion/enactment/bind_accessors.hh
    legion/enactment/task_wrapper.hh
    legion/enactment/unbind_accessors.hh
    legion/execution_policy.hh
    legion/future.hh
    legion/invocation/init_args.hh
    legion/reduction_wrapper.hh
  )

  set(execution_SOURCES
    ${execution_SOURCES}
    legion/runtime.cc
  )

elseif(FLECSI_RUNTIME_MODEL STREQUAL "mpi")

  set(execution_HEADERS
    ${execution_HEADERS}
    mpi/execution_policy.hh
    mpi/reduction_wrapper.hh
  )

  set(execution_SOURCES
    ${execution_SOURCES}
    mpi/runtime.cc
  )

elseif(FLECSI_RUNTIME_MODEL STREQUAL "hpx")

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
