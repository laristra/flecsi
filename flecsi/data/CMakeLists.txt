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
	legion/dense.h
	legion/global.h
	legion/meta_data.h
	legion/scoped.h
	legion/sparse.h
	legion/storage_policy.h
	legion/tuple.h
	old_data.h
	old/default_meta_data.h
	old/default_storage_policy.h
	permissions.h
	serial/dense.h
	serial/global.h
	serial/meta_data.h
	serial/scoped.h
	serial/sparse.h
	serial/storage_policy.h
	serial/tuple.h
	storage.h
	storage_type.h
  PARENT_SCOPE
)

set(data_SOURCES
  data_client.cc
  PARENT_SCOPE
)

cinch_add_unit(data
  SOURCES test/data.cc)

cinch_add_unit(data_client
  SOURCES test/data_client.cc
  LIBRARIES flecsi)

cinch_add_unit(storage_type
  SOURCES test/storage_type.cc
  LIBRARIES flecsi)

#----------------------------------------------------------------------------~-#
# Formatting options for vim.
# vim: set tabstop=2 shiftwidth=2 expandtab :
#----------------------------------------------------------------------------~-#
