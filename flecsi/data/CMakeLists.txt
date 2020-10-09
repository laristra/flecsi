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

set(data_HEADERS
  accessor.h
  client.h
  common/data_hash.h
  common/data_types.h
  common/data_reference.h
  common/privilege.h
  common/registration_wrapper.h
  common/row_vector.h
  common/serdez.h
  data.h
  data_client.h
  data_client_handle.h
  data_client_handle.h
  data_constants.h
  dense_data_handle.h
  dense_accessor.h
  field.h
  global_accessor.h
  global_data_handle.h
  internal_client.h
  mutator.h
  ragged_accessor.h
  ragged_mutator.h
  sparse_accessor.h
  sparse_mutator.h
  sparse_data_handle.h
  storage.h
  storage_class.h
)

#------------------------------------------------------------------------------#
# Add source files. Note that these will be "exported" to the parent
# scope below.
#------------------------------------------------------------------------------#

set(data_SOURCES
  internal_client.cc
)

set(DRIVER_INITIALIZATION ../execution/driver_initialization.cc)

#------------------------------------------------------------------------------#
# Runtime-specific files.
#
# UNIT_POLICY and RUNTIME_DRIVER are set for unit tests that are not
# runtime specific and can be configured for whichever runtime is active.
#------------------------------------------------------------------------------#

if(FLECSI_RUNTIME_MODEL STREQUAL "legion")

  set(data_HEADERS
    ${data_HEADERS}
    legion/data_client_handle_policy.h
    legion/dense_data_handle_policy.h
    legion/global_data_handle_policy.h
    legion/sparse_data_handle_policy.h
    legion/data_policy.h
    legion/dense.h
    legion/global.h
    legion/color.h
    legion/legion_data.h
    legion/scoped.h
    legion/sparse.h
    legion/storage_policy.h
    legion/tuple.h
  )

  set(UNIT_POLICY LEGION)
  set(RUNTIME_DRIVER ../execution/legion/runtime_driver.cc)

elseif(FLECSI_RUNTIME_MODEL STREQUAL "mpi")

  set(data_HEADERS
    ${data_HEADERS}
    mpi/color.h
    mpi/data_client_handle_policy.h
    mpi/data_handle_policy.h
    mpi/data_policy.h
    mpi/dense.h
    mpi/global.h
    mpi/sparse.h
    mpi/sparse_data_handle_policy.h
    mpi/storage_policy.h
  )

  set(data_SOURCES
    ${data_SOURCES}
  )

  set(UNIT_POLICY MPI)
  set(RUNTIME_DRIVER ../execution/mpi/runtime_driver.cc)

elseif(FLECSI_RUNTIME_MODEL STREQUAL "hpx")

  set(data_HEADERS
    ${data_HEADERS}
    hpx/color.h
    hpx/data_client_handle_policy.h
    hpx/data_handle_policy.h
    hpx/data_policy.h
    hpx/dense.h
    hpx/global.h
    hpx/sparse.h
    hpx/sparse_data_handle_policy.h
    hpx/storage_policy.h
  )

  set(UNIT_POLICY HPX)
  set(RUNTIME_DRIVER ../execution/hpx/runtime_driver.cc)

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

# cinch_add_unit(compaction
#   SOURCES
#     test/legion/compaction.cc
#     ../execution/driver_initialization.cc
#     ${RUNTIME_DRIVER}
#   LIBRARIES
#     FleCSI
#     ${CINCH_RUNTIME_LIBRARIES}
# )

#if(FLECSI_RUNTIME_MODEL STREQUAL "serial")
#  cinch_add_unit(storage_class
#    SOURCES test/storage_class.cc
#    LIBRARIES
#      FleCSI
#      ${CINCH_RUNTIME_LIBRARIES}
#  )
#
#  if(ENABLE_PARMETIS)
#    cinch_add_unit(serial_data_handle
#      SOURCES
#        ../execution/test/task_driver.cc
#        ../execution/serial/runtime_driver.cc
#        test/data-handle-serial.h
#        data_client.cc
#      DEFINES
#        DATA_HANDLE_TEST
#      LIBRARIES
#        FleCSI
#        ${CINCH_RUNTIME_LIBRARIES}
#      POLICY SERIAL
#      THREADS 2
#    )
#  endif()
#
#endif()


#if(FLECSI_RUNTIME_MODEL STREQUAL "mpilegion")
#cinch_add_unit(storage_class
#  SOURCES test/storage_class.h
#          ../execution/mpilegion/runtime_driver.cc
#          ${RUNTIME_DRIVER}
#  DEFINES STORAGE_CLASS_TEST
#        FLECSI_SPECIALIZATION_DRIVER=flecsi/data/test/storage_class.h
#        TEST_INIT=flecsi/execution/test/task_init.h
#  LIBRARIES
#    FleCSI
#    ${CINCH_RUNTIME_LIBRARIES}
#  POLICY MPILEGION
#)
#
#
#  if(ENABLE_PARMETIS)
#    cinch_add_unit(mpilegion_data_handle
#      SOURCES
#        ../execution/test/task_driver.cc
#        test/data-handle.h
#        ../execution/mpilegion/runtime_driver.cc
#        ../execution/mpilegion/copy_ghosts.cc
#        ../execution/test/mpilegion/init_partitions_task.cc
#        data_client.cc
#      DEFINES DATA_HANDLE_TEST
#        FLECSI_SPECIALIZATION_DRIVER=flecsi/data/test/data-handle.h
#        TEST_INIT=flecsi/execution/test/task_init.h
#      LIBRARIES ${CINCH_RUNTIME_LIBRARIES}
#      POLICY MPILEGION
#      THREADS 2
#    )
#  endif()

#endif()

if(ENABLE_PARMETIS)

  cinch_add_unit(client_registration
    SOURCES
      test/client_registration.cc
      ${DRIVER_INITIALIZATION}
      ${RUNTIME_DRIVER}
    DEFINES
      -DCINCH_OVERRIDE_DEFAULT_INITIALIZATION_DRIVER
    LIBRARIES
      FleCSI
      ${CINCH_RUNTIME_LIBRARIES}
    POLICY ${UNIT_POLICY}
    THREADS 2
    NOCI
  )

endif()

