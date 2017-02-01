#~----------------------------------------------------------------------------~#
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
#~----------------------------------------------------------------------------~#

set(execution_HEADERS
	common/function_handle.h
	common/launch.h
	common/processor.h
	common/task_hash.h
	context.h
	default_driver.h
	execution.h
	function.h
	kernel.h
	task.h
)

#------------------------------------------------------------------------------#
# UNIT_POLICY and RUNTIME_DRIVER are set for unit tests that are not
# runtime specific and can be configured for whichever runtime is active.
#------------------------------------------------------------------------------#

if(FLECSI_RUNTIME_MODEL STREQUAL "serial")

  set(execution_HEADERS
    ${execution_HEADERS}
    serial/context_policy.h
    serial/execution_policy.h
    serial/runtime_driver.h
  )

  set(UNIT_POLICY SERIAL)
  set(RUNTIME_DRIVER serial/runtime_driver.cc)

elseif(FLECSI_RUNTIME_MODEL STREQUAL "legion")

  set(execution_HEADERS
    ${execution_HEADERS}
    legion/context_policy.h
    legion/dpd.h
    legion/future.h
    legion/execution_policy.h
    legion/runtime_driver.h
    legion/task_args.h
    legion/task_wrapper.h
    task_ids.h
    PARENT_SCOPE
  )

  set(execution_SOURCES
    legion/dpd.cc
  )

  set(UNIT_POLICY LEGION)
  set(RUNTIME_DRIVER legion/runtime_driver.cc)

elseif(FLECSI_RUNTIME_MODEL STREQUAL "mpilegion")

  set(execution_HEADERS
    ${execution_HEADERS}
    legion/dpd.h
    legion/task_args.h
    legion/task_wrapper.h
    mpilegion/context_policy.h
    mpilegion/execution_policy.h
    mpilegion/legion_handshake.h
    mpilegion/mapper.h
    mpilegion/mpi_legion_interop.h
    mpilegion/runtime_driver.h
    task_ids.h
    mpilegion/init_partitions_task.h 
  )

  set(execution_SOURCES
    legion/dpd.cc
    mpilegion/init_partitions_task.cc
  )

  set(UNIT_POLICY MPILEGION)
  set(RUNTIME_DRIVER mpilegion/runtime_driver.cc)

endif()

set(execution_HEADERS
  ${execution_HEADERS}
  PARENT_SCOPE
)

set(execution_SOURCES
  ${execution_SOURCES}
  PARENT_SCOPE
)

#------------------------------------------------------------------------------#
# Unit tests.
#------------------------------------------------------------------------------#
if((NOT DEFINED ENV{TRAVIS} ) OR (FLECSI_RUNTIME_MODEL STREQUAL "serial")
    OR (FLECSI_RUNTIME_MODEL STREQUAL "legion") )
cinch_add_unit(task
  SOURCES
    test/task.cc
    test/task_driver.h
    ${RUNTIME_DRIVER}
  DEFINES
    FLECSI_DRIVER=flecsi/execution/test/task_driver.h
    TEST_INIT=flecsi/execution/test/task_init.h
  POLICY
    ${UNIT_POLICY}
  LIBRARIES
    flecsi
    ${CINCH_RUNTIME_LIBRARIES}
)

cinch_add_unit(simple_function
  SOURCES
    test/function.cc
    test/simple_function.h
    ${RUNTIME_DRIVER}
  DEFINES
    FLECSI_DRIVER=flecsi/execution/test/simple_function.h
  POLICY
    ${UNIT_POLICY}
  LIBRARIES
    flecsi
    ${CINCH_RUNTIME_LIBRARIES}
)
endif()

if(NOT DEFINED ENV{TRAVIS})
  if(FLECSI_RUNTIME_MODEL STREQUAL "mpilegion")

    cinch_add_unit(mpilegion_task
      SOURCES
        test/task.cc
        test/task_driver.h
        mpilegion/runtime_driver.cc
        mpilegion/init_partitions_task.cc
        legion/dpd.cc
      DEFINES
        FLECSI_DRIVER=flecsi/execution/test/task_driver.h
        TEST_INIT=flecsi/execution/test/task_init.h
      POLICY MPILEGION
      LIBRARIES
      flecsi
      ${CINCH_RUNTIME_LIBRARIES}
    )

#  cinch_add_unit(dpd_sparse
#    SOURCES
#      test/dpd_sparse.cc
#      legion/dpd.cc
#    POLICY
#      MPILEGION
#   THREADS 2
#  )

  # FIXME: Need to talk about this with Irina and Galen.
    cinch_add_unit(test_legion_handshake
      SOURCES test/test_legion_handshake.cc
      POLICY MPILEGION
    )

    cinch_add_unit(test_mpilegion_interop
      SOURCES test/test_mpi_legion_interop.cc
      POLICY MPILEGION
    )

    if(ENABLE_PARTITIONING AND ENABLE_PARMETIS)
      cinch_add_unit(mpilegion_sprint
        SOURCES
          test/task.cc
          test/sprint.h
          mpilegion/runtime_driver.cc
          mpilegion/init_partitions_task.cc
          legion/dpd.cc
        INPUTS
          test/simple2d-8x8.msh
        DEFINES SPRINT_TEST
          FLECSI_DRIVER=flecsi/execution/test/sprint.h
          TEST_INIT=flecsi/execution/test/task_init.h
        LIBRARIES ${PARTITION_LIBRARIES}
        POLICY MPILEGION
        THREADS 2
      )
    endif()

  endif()

endif()

#----------------------------------------------------------------------------~-#
# Formatting options for vim.
# vim: set tabstop=2 shiftwidth=2 expandtab :
#----------------------------------------------------------------------------~-#
