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

#ifndef flecsi_serial_meta_data_h
#define flecsi_serial_meta_data_h

#include <unordered_map>
#include <vector>
#include <memory>
#include <typeinfo>
#include <bitset>

/*!
 * \file serial/meta_data.h
 * \authors bergen
 * \date Initial file creation: Apr 15, 2016
 */

namespace flecsi {
namespace data {

/*-----------------------------------------------------------------------------*
 * struct serial_meta_data_t
 *----------------------------------------------------------------------------*/

/*!
  \brief serial_meta_data_t provides storage for extra information that is
    used to interpret data variable information at different points
    in the low-level runtime.
 
  \tparam user_meta_data_t A user-defined data type that will be carried
    with the meta data.
 */
template<typename T>
struct serial_meta_data_t {

  using user_meta_data_t = T;

  std::string label;
  user_meta_data_t user_data;
  size_t index_space;
  size_t size;
  size_t type_size;
  size_t versions;

  std::bitset<8> attributes;
  std::bitset<8> user_attributes;

  /*!
    \brief type_info_t allows creation of reference information
      to the user-specified type of the data data.

    The std::type_info type requires dynamic initialization.  The
      type_info_t type is designed to allow construction without
      needing a non-trivial default constructor for the
      serial_meta_data_t type.
   */
  struct type_info_t {
    type_info_t(const std::type_info & type_info_) : type_info(type_info_) {}
    const std::type_info & type_info;
  }; // struct type_info_t

  std::shared_ptr<type_info_t> rtti;

  std::unordered_map<size_t, std::vector<uint8_t>> data;
  size_t num_materials;
}; // struct serial_meta_data_t

} // namespace data
} // namespace flecsi

#endif // flecsi_serial_meta_data_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
