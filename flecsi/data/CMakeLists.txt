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
	new_data.h
	data_constants.h
	data.h
	data_client.h
	default_meta_data.h
	default_storage_policy.h
	default/default_scoped.h
	default/default_dense.h
	default/default_sparse.h
	default/default_tuple.h
	default/default_storage_policy.h
	default/default_storage_type.h
	default/default_meta_data.h
	default/default_scalar.h
  data_client.h
  data_constants.h
  data.h
  default_meta_data.h
  default_storage_policy.h
  PARENT_SCOPE
)

#set(data_SOURCES
#  PARENT_SCOPE
#)

cinch_add_unit(data
  SOURCES test/data.cc)

cinch_add_unit(storage_type
  SOURCES test/storage_type.cc)

cinch_add_unit(new_data
  SOURCES test/new_data.cc)

#----------------------------------------------------------------------------~-#
# Formatting options for vim.
# vim: set tabstop=2 shiftwidth=2 expandtab :
#----------------------------------------------------------------------------~-#
