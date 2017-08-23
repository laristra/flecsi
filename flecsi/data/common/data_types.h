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

struct global_data_client_t
{
  using type_identifier_t = global_data_client_t;
};

// Generic bitfield type
using bitset_t = std::bitset<8>;

#if 0
class global_data_client_t : public data::data_client_t
{
  public:
    static global_data_client_t& instance()
    {
        static global_data_client_t global_data_client_;
        return global_data_client_;
    }

    using type_identifier_t = global_data_client_t;
 
  private:
    global_data_client_t() {}
    global_data_client_t(const global_data_client_t&);
    global_data_client_t& operator=(const global_data_client_t&);
};
#endif

} // namespace data
} // namespace flecsi

#endif // flecsi_data_data_types_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
