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
  backend.hh
  kernel_interface.hh
  launch.hh
  task_attributes.hh
  reduction.hh
)

if(FLECSI_RUNTIME_MODEL STREQUAL "legion")

  set(execution_HEADERS
    ${execution_HEADERS}
    legion/bind_accessors.hh
    legion/task_wrapper.hh
    legion/unbind_accessors.hh
    legion/policy.hh
    legion/future.hh
    legion/task_prologue.hh
    legion/reduction_wrapper.hh
  )

elseif(FLECSI_RUNTIME_MODEL STREQUAL "mpi")

  set(execution_HEADERS
    ${execution_HEADERS}
    mpi/policy.hh
    mpi/reduction_wrapper.hh
  )

elseif(FLECSI_RUNTIME_MODEL STREQUAL "hpx")

endif()

#------------------------------------------------------------------------------#
# Export header list to parent scope.
#------------------------------------------------------------------------------#

set(execution_HEADERS
  ${execution_HEADERS}
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

ftest_add_unit(future
  SOURCES
    test/future.cc
  THREADS 2
)
