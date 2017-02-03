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

#----------------------------------------------------------------------------~-#
# Formatting options for vim.
# vim: set tabstop=2 shiftwidth=2 expandtab :
#----------------------------------------------------------------------------~-#
