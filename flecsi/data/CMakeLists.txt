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

set(data_HEADERS
	common/data_types.h
	data_client.h
	data_constants.h
	data.h
	data_handle.h
	default_user_meta_data.h
		old/old_data.h
	old/default_meta_data.h
	old/default_storage_policy.h
	permissions.h
	storage.h
	storage_type.h
)

set(data_SOURCES
  data_client.cc
)

if(FLECSI_RUNTIME_MODEL STREQUAL "serial")

  set(data_HEADERS
    ${data_HEADERS}
    serial/dense.h
    serial/global.h
    serial/meta_data.h
    serial/scoped.h
    serial/sparse.h
    serial/storage_policy.h
    serial/tuple.h
  )

elseif(FLECSI_RUNTIME_MODEL STREQUAL "legion")

  set(data_HEADERS
    ${data_HEADERS}
    legion/dense.h
    legion/global.h
    legion/meta_data.h
    legion/scoped.h
    legion/sparse.h
    legion/storage_policy.h
    legion/tuple.h
  )

elseif(FLECSI_RUNTIME_MODEL STREQUAL "mpilegion")

  set(data_HEADERS
    ${data_HEADERS}
    legion/dense.h
    legion/global.h
    legion/meta_data.h
    legion/scoped.h
    legion/sparse.h
    legion/storage_policy.h
    legion/tuple.h
  )

endif()

set(data_HEADERS
    ${data_HEADERS}
    PARENT_SCOPE
)

set(data_SOURCES
    ${data_SOURCES}
    PARENT_SCOPE
)
#------------------------------------------------------------------------------#
# Unit tests.
#------------------------------------------------------------------------------#

cinch_add_unit(old_data
  SOURCES test/old_data.cc
)

cinch_add_unit(data_client
  SOURCES test/data_client.cc
  LIBRARIES
    flecsi
    ${CINCH_RUNTIME_LIBRARIES}
)

cinch_add_unit(storage_type
  SOURCES test/storage_type.cc
  LIBRARIES
    flecsi
    ${CINCH_RUNTIME_LIBRARIES}
)

# if(FLECSI_RUNTIME_MODEL STREQUAL "mpilegion")

#   if(ENABLE_PARTITIONING AND ENABLE_PARMETIS)
#     cinch_add_unit(mpilegion_data_handle
#       SOURCES
#         ../execution/test/task.cc
#         ../execution/test/task_driver.h
#         test/data-handle.h
#         ../execution/mpilegion/runtime_driver.cc
#         ../execution/mpilegion/init_partitions_task.cc
#         data_client.cc
#         ../execution/legion/dpd.cc
#         ../execution/mpilegion/lax_wendroff_task.cc
#       DEFINES DATA_HANDLE_TEST
#         FLECSI_DRIVER=flecsi/data/test/data-handle.h
#         TEST_INIT=flecsi/execution/test/task_init.h
#       LIBRARIES ${PARTITION_LIBRARIES}
#       POLICY MPILEGION
#       THREADS 2
#     )
#   endif()

# endif()

# if(FLECSI_RUNTIME_MODEL STREQUAL "serial")

#   if(ENABLE_PARTITIONING AND ENABLE_PARMETIS)
#     cinch_add_unit(serial_data_handle
#       SOURCES
#         ../execution/test/task.cc
#         ../execution/test/task_driver.h
#         ../execution/serial/runtime_driver.cc
#         test/data-handle-serial.h
#         data_client.cc
#       DEFINES DATA_HANDLE_TEST
#         FLECSI_DRIVER=flecsi/data/test/data-handle-serial.h
#         TEST_INIT=flecsi/execution/test/task_init.h
#       LIBRARIES ${PARTITION_LIBRARIES}
#       POLICY SERIAL
#       THREADS 2
#     )
#   endif()

# endif()

#----------------------------------------------------------------------------~-#
# Formatting options for vim.
# vim: set tabstop=2 shiftwidth=2 expandtab :
#----------------------------------------------------------------------------~-#
