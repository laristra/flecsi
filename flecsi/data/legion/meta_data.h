/*~--------------------------------------------------------------------------~*
 *  @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
 * /@@/////  /@@          @@////@@ @@////// /@@
 * /@@       /@@  @@@@@  @@    // /@@       /@@
 * /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
 * /@@////   /@@/@@@@@@@/@@       ////////@@/@@
 * /@@       /@@/@@//// //@@    @@       /@@/@@
 * /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
 * //       ///  //////   //////  ////////  //
 *
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_legion_meta_data_h
#define flecsi_legion_meta_data_h

#include "flecsi/data/common/data_types.h"

///
// \file legion/meta_data.h
// \authors bergen, nickm
// \date Initial file creation: Apr 15, 2016
///

namespace flecsi {
namespace data {

//----------------------------------------------------------------------------//
// struct legion_meta_data_t
//----------------------------------------------------------------------------//

///
// \brief legion_meta_data_t provides storage for extra information that is
//        used to interpret data variable information at different points
//        in the low-level runtime.
// 
// \tparam T A user-defined data type that will be carried with the meta data.
///
template<typename T>
struct legion_meta_data_t
{

  using user_meta_data_t = T;

  std::string label;
  user_meta_data_t user_data;
  size_t index_space;
  size_t size;
  size_t type_size;
  size_t versions;

  ///
  /// \brief type_info_t allows creation of reference information
  ///        to the user-specified type of the data data.
  ///
  /// The std::type_info type requires dynamic initialization.  The
  /// type_info_t type is designed to allow construction without
  /// needing a non-trivial default constructor for the
  /// serial_meta_data_t type.
  ///
  struct type_info_t {
    type_info_t(const std::type_info & type_info_) : type_info(type_info_) {}
    const std::type_info & type_info;
  }; // struct type_info_t

  struct legion_data{
    Legion::LogicalRegion lr;
    Legion::FieldSpace fs;
    Legion::IndexSpace is;
    Legion::IndexPartition exclusive_ip;
    Legion::IndexPartition shared_ip;
    Legion::IndexPartition ghost_ip;
    Legion::LogicalRegion exclusive_lr;
    Legion::LogicalRegion shared_lr;
    Legion::LogicalRegion ghost_lr;
  };

  legion_data& get_legion_data(size_t version) const{
    auto itr = data.find(version);
    assert(itr != data.end() && "invalid data version");
    return const_cast<legion_data&>(itr->second);
  }

  legion_data create_legion_data() const{
    return legion_data();
  }

  void put_legion_data(size_t version, legion_data& d){
    data.emplace(version, std::move(d));
  }

  std::shared_ptr<type_info_t> rtti;
  std::unordered_map<size_t, bitset_t> attributes;
  size_t num_entries;

  std::unordered_map<size_t, legion_data> data;
}; // struct legion_meta_data_t

} // namespace data
} // namespace flecsi

#endif // flecsi_legion_meta_data_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
