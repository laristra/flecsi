/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

/*! @file */

#include <bitset>

#include <flecsi/utils/offset.h>

namespace flecsi {
namespace data {

struct global_data_client_t {
  using type_identifier_t = global_data_client_t;
};

using sparse_data_offset_t = utils::offset__<16>;

template<typename T>
struct sparse_entry_value__ {
  using index_t = uint64_t;

  sparse_entry_value__(index_t entry) : entry(entry) {}

  sparse_entry_value__(index_t entry, T value) : entry(entry), value(value) {}

  sparse_entry_value__() {}

  bool operator<(const sparse_entry_value__ & ev) const {
    return entry < ev.entry;
  }

  index_t entry;
  T value;
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

/*~-------------------------------------------------------------------------~-*
 *~-------------------------------------------------------------------------~-*/
