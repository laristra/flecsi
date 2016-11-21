/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_data_data_types_h
#define flecsi_data_data_types_h

#include <bitset>

///
// \file data_types.h
// \authors bergen
// \date Initial file creation: Sep 28, 2016
///

namespace flecsi {
namespace data {

// Generic bitfield type
using bitset_t = std::bitset<8>;

} // namespace data
} // namespace flecsi

#endif // flecsi_data_data_types_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
