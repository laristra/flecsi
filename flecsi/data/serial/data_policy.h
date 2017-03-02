/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2017 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_serial_data_policy_h
#define flecsi_serial_data_policy_h

#include "flecsi/partition/index_partition.h"

namespace flecsi {
namespace data {

class serial_data_policy_t
{
public:
 using partitioned_index_space = dmp::index_partition__<size_t>;

};

} // namespace data
} // namespace flecsi

#endif // flecsi_serial_data_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
