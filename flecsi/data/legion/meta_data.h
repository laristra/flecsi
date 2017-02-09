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

///
// \file legion/meta_data.h
// \authors bergen
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

  user_meta_data_t user_data;

  size_t index_space;
  size_t size;
  size_t type_size;
  size_t versions;

  std::unordered_map<size_t, std::vector<uint8_t>> data;

}; // struct legion_meta_data_t

} // namespace data
} // namespace flecsi

#endif // flecsi_legion_meta_data_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
