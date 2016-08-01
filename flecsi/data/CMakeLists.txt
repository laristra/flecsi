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
  data.h
	storage.h
	data_constants.h
	old_data.h
	data_client.h
	old/default_meta_data.h
	old/default_storage_policy.h
	serial/scoped.h
	serial/dense.h
	serial/sparse.h
	serial/tuple.h
	serial/storage_policy.h
	serial/storage_type.h
	serial/meta_data.h
	serial/scalar.h
  data_client.h
  data_constants.h
  PARENT_SCOPE
)

#set(data_SOURCES
#  PARENT_SCOPE
#)

cinch_add_unit(data
  SOURCES test/data.cc)

cinch_add_unit(storage_type
  SOURCES test/storage_type.cc)

#----------------------------------------------------------------------------~-#
# Formatting options for vim.
# vim: set tabstop=2 shiftwidth=2 expandtab :
#----------------------------------------------------------------------------~-#
