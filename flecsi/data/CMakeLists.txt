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

#------------------------------------------------------------------------------#
# Add header files. Note that these will be "exported" to the parent
# scope below.
#------------------------------------------------------------------------------#

set(data_HEADERS
  accessor.h
  common/data_hash.h
  common/data_types.h
  data_client.h
  data_constants.h
  data.h
  data_handle.h
  default_user_meta_data.h
  storage.h
  storage_type.h
)

#------------------------------------------------------------------------------#
# Add source files. Note that these will be "exported" to the parent
# scope below.
#------------------------------------------------------------------------------#

set(data_SOURCES
  data_client.cc
)

#------------------------------------------------------------------------------#
# Runtime-specific files.
#
# UNIT_POLICY and RUNTIME_DRIVER are set for unit tests that are not
# runtime specific and can be configured for whichever runtime is active.
#------------------------------------------------------------------------------#

if(FLECSI_RUNTIME_MODEL STREQUAL "serial")

  set(data_HEADERS
    ${data_HEADERS}
    serial/data_handle_policy.h
    serial/data_policy.h
    serial/dense.h
    serial/global.h
    serial/handle_policy.h
    serial/meta_data.h
    serial/scoped.h
    serial/sparse.h
    serial/storage_policy.h
    serial/tuple.h
  )

  set(UNIT_POLICY SERIAL)
  set(RUNTIME_DRIVER ../execution/serial/runtime_driver.cc)

elseif(FLECSI_RUNTIME_MODEL STREQUAL "legion")

  set(data_HEADERS
    ${data_HEADERS}
    legion/data_handle_policy.h
    legion/data_policy.h
    legion/dense.h
    legion/dpd.h
    legion/global.h
    legion/handle_policy.h
    legion/meta_data.h
    legion/registration_wrapper.h
    legion/scoped.h
    legion/sparse.h
    legion/storage_policy.h
    legion/tuple.h
  )

#  set(data_SOURCES
#    ${data_SOURCES}
#    legion/dpd.cc
#  )

  set(UNIT_POLICY LEGION)
  set(RUNTIME_DRIVER ../execution/legion/runtime_driver.cc)

elseif(FLECSI_RUNTIME_MODEL STREQUAL "mpi")

  set(execution_HEADERS
    ${execution_HEADERS}
  )

  set(execution_SOURCES
    ${execution_SOURCES}
  )

  set(UNIT_POLICY LEGION)
  set(RUNTIME_DRIVER mpi/runtime_driver.cc)

endif()

#------------------------------------------------------------------------------#
# Export header list to parent scope.
#------------------------------------------------------------------------------#

set(data_HEADERS
    ${data_HEADERS}
    PARENT_SCOPE
)

#------------------------------------------------------------------------------#
# Export source list to parent scope.
#------------------------------------------------------------------------------#

set(data_SOURCES
    ${data_SOURCES}
    PARENT_SCOPE
)

#------------------------------------------------------------------------------#
# Unit tests.
#------------------------------------------------------------------------------#

cinch_add_unit(data_client
  SOURCES
    test/data_client.cc
    ${RUNTIME_DRIVER}
  LIBRARIES
    flecsi
    ${CINCH_RUNTIME_LIBRARIES}
)

cinch_add_unit(compaction
  SOURCES
    test/legion/compaction.cc
    ${RUNTIME_DRIVER}
  LIBRARIES
    flecsi
    ${CINCH_RUNTIME_LIBRARIES}
)

if(FLECSI_RUNTIME_MODEL STREQUAL "serial")
  cinch_add_unit(storage_type
    SOURCES test/storage_type.cc
    LIBRARIES
      flecsi
      ${CINCH_RUNTIME_LIBRARIES}
  )

  if(ENABLE_PARTITIONING AND ENABLE_PARMETIS)
    cinch_add_unit(serial_data_handle
      SOURCES
        ../execution/test/task.cc
        ../execution/test/task_driver.cc
        ../execution/serial/runtime_driver.cc
        test/data-handle-serial.h
        data_client.cc
      DEFINES DATA_HANDLE_TEST
        FLECSI_SPECIALIZATION_DRIVER=flecsi/data/test/data-handle-serial.h
        TEST_INIT=flecsi/execution/test/task_init.h
      LIBRARIES
        flecsi
        ${CINCH_RUNTIME_LIBRARIES}
      POLICY SERIAL
      THREADS 2
    )
  endif()

endif()


#if(FLECSI_RUNTIME_MODEL STREQUAL "mpilegion")
#cinch_add_unit(storage_type
#  SOURCES test/storage_type.h
#          ../execution/mpilegion/runtime_driver.cc
#          ${RUNTIME_DRIVER}
#  DEFINES STORAGE_TYPE_TEST
#        FLECSI_SPECIALIZATION_DRIVER=flecsi/data/test/storage_type.h
#        TEST_INIT=flecsi/execution/test/task_init.h
#  LIBRARIES
#    flecsi
#    ${CINCH_RUNTIME_LIBRARIES}
#  POLICY MPILEGION
#)
#
#
#  if(ENABLE_PARTITIONING AND ENABLE_PARMETIS)
#    cinch_add_unit(mpilegion_data_handle
#      SOURCES
#        ../execution/test/task.cc
#        ../execution/test/task_driver.cc
#        test/data-handle.h
#        ../execution/mpilegion/runtime_driver.cc
#        ../execution/mpilegion/copy_ghosts.cc
#        ../execution/test/mpilegion/init_partitions_task.cc
#        data_client.cc
#        legion/dpd.cc
#      DEFINES DATA_HANDLE_TEST
#        FLECSI_SPECIALIZATION_DRIVER=flecsi/data/test/data-handle.h
#        TEST_INIT=flecsi/execution/test/task_init.h
#      LIBRARIES ${CINCH_RUNTIME_LIBRARIES}
#      POLICY MPILEGION
#      THREADS 2
#    )
#  endif()

#endif()

if(FLECSI_RUNTIME_MODEL STREQUAL "legion")

  if(ENABLE_PARTITIONING AND ENABLE_PARMETIS)

    cinch_add_unit(legion_dh1
      SOURCES
        ../execution/test/task.cc
        ../execution/test/context_runtime.cc
        ../execution/legion/runtime_driver.cc
        ../execution/legion/context_policy.cc
        test/dh1.cc
        data_client.cc
      LIBRARIES
        flecsi
        ${CINCH_RUNTIME_LIBRARIES}
      POLICY LEGION
      THREADS 2
    )
  endif()

endif()

#----------------------------------------------------------------------------~-#
# Formatting options for vim.
# vim: set tabstop=2 shiftwidth=2 expandtab :
#----------------------------------------------------------------------------~-#
