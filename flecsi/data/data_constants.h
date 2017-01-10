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

#ifndef flecsi_data_constants_h
#define flecsi_data_constants_h

#include <limits>

#include "flecsi/utils/bitfield.h"
#include "flecsi/utils/const_string.h"

///
/// \file
/// \date Initial file creation: Feb 26, 2016
///

namespace flecsi {
namespace data {

///
///
///
enum storage_label_type_t : size_t {
  global,
  dense,
  sparse,
  scoped,
  tuple
}; // enum storage_label_type_t

// EVERYTHING BELOW THIS LINE IS DEPRECATED AND WILL BE REMOVED SOON!

///
/// FIXME: This needs to be removed.
///
static constexpr size_t flecsi_internal =
  utils::const_string_t{"__flecsi_internal__"}.hash();

///
/// \brief data_attribute_old_t defines different data attributes.
///
/// This type should probably be pushed up in the interface so that users
/// can define their own attributes.
///
enum class data_attribute_old_t : utils::bitfield_t::field_type_t {
  persistent = 0x0001,
  temporary = 0x0002
}; // enum class data_attribute_old_t

enum data_attribute_t : size_t {
  persistent = 0x0001, // FIXME: This is just to not break the old model
  temporary
}; // enum data_attribute_t

///
/// \brief This exposes the persistent attribute so that it can be used
/// without specifying the full type information.
///
static constexpr utils::bitfield_t::field_type_t old_persistent =
  static_cast<utils::bitfield_t::field_type_t>(
    data_attribute_old_t::persistent
  );

///
/// \brief This exposes the temporary attribute so that it can be used
/// without specifying the full type information.
///
static constexpr utils::bitfield_t::field_type_t old_temporary =
  static_cast<utils::bitfield_t::field_type_t>(
    data_attribute_old_t::temporary
  );

} // namespace data
} // namespace flecsi

#endif // flecsi_data_constants_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
