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

/*!
  \brief Register data with a data_client_t.

  \param client The data_client_t instance with which to register
    the data.
  \param name The string name of the data variable to register.
  \param versions The number of versions of the data to register. This
    parameter can be used to manage multiple data versions, e.g.,
    for new and old state.
  \param data_type The data type to store, e.g., double or my_type_t.
  \param storage_type The storage type for the data \ref storage_type_t.
  
  Each storage type may have additional parameters that need to be passed
  to this macro.
 */
#define register_data(client, name, versions, data_type, storage_type, ...) \
	storage_t::instance().register_data<storage_type, data_type,              \
		data_name_space_t::user>(client, name, versions, ##__VA_ARGS__)

/*!
  FIXME
 */
#define get_accessor(client, name, version, data_type, storage_type) \
	storage_t::instance().get_accessor<storage_type, data_type,        \
		data_name_space_t::user>(client, name, version)

#define get_mutator(client, name, version, data_type, storage_type, slots) \
  storage_t::instance().get_mutator<storage_type, data_type,               \
    data_name_space_t::user>(client, name, slots, version)

/*!
  FIXME
 */
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
