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

set(exec_HEADERS
  backend.hh
  fold.hh
  kernel_interface.hh
  launch.hh
  task_attributes.hh
)

if(FLECSI_RUNTIME_MODEL STREQUAL "legion")

  set(exec_HEADERS
    ${exec_HEADERS}
    leg/bind_accessors.hh
    leg/task_wrapper.hh
    leg/unbind_accessors.hh
    leg/policy.hh
    leg/future.hh
    leg/task_prologue.hh
    leg/reduction_wrapper.hh
  )

elseif(FLECSI_RUNTIME_MODEL STREQUAL "mpi")

  set(exec_HEADERS
    ${exec_HEADERS}
    mpi/policy.hh
    mpi/reduction_wrapper.hh
  )

elseif(FLECSI_RUNTIME_MODEL STREQUAL "hpx")

elseif(FLECSI_RUNTIME_MODEL STREQUAL "charm")

  set(execution_HEADERS
    ${execution_HEADERS}
    charm/bind_accessors.hh
    charm/task_wrapper.hh
    charm/unbind_accessors.hh
    charm/policy.hh
    charm/future.hh
    charm/task_prologue.hh
    charm/reduction_wrapper.hh
  )

endif()

#------------------------------------------------------------------------------#
# Export header list to parent scope.
#------------------------------------------------------------------------------#

set(exec_HEADERS
  ${exec_HEADERS}
  PARENT_SCOPE
)

#------------------------------------------------------------------------------#
# Unit tests.
#------------------------------------------------------------------------------#

if(ENABLE_KOKKOS)
  add_unit(kokkos
    SOURCES
      test/kokkos.cc
  )
endif()

add_unit(task
  SOURCES
    test/task.cc
  THREADS
    4 # Warning: This number is hard-coded into the the test.
)

if(FLECSI_RUNTIME_MODEL STREQUAL "legion")
  add_unit(future
    SOURCES
      test/future.cc
    THREADS 2
  )
endif()
