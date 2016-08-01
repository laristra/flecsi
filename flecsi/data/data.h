/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_data_data_h
#define flecsi_data_data_h

#include "flecsi/data/storage.h"

/*!
 * \file data.h
 * \authors bergen
 * \date Initial file creation: Aug 01, 2016
 */

namespace flecsi {

#define register_data(client, name, versions, data_type, storage_type, ...) \
	storage_t::instance().register_data<storage_type, data_type,              \
		data_name_space_t::user>(client, name, versions, ##__VA_ARGS__)

#define get_accessor(client, name, version, data_type, storage_type) \
	storage_t::instance().get_accessor<storage_type, data_type,        \
		data_name_space_t::user>(client, name, version)

#define get_handle(client, name, version, data_type, storage_type,        \
  privileges)                                                             \
  storage_t::instance().get_handle<storage_type, data_type, privileges,   \
		data_name_space_t::user>(client, name, version)

} // namespace flecsi

#endif // flecsi_data_data_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
