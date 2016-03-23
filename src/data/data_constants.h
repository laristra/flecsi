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

#include "../utils/bitfield.h"

/*!
 * \file data_constants.h
 * \authors bergen
 * \date Initial file creation: Feb 26, 2016
 */

namespace flecsi
{

static constexpr size_t __flecsi_internal_data_offset__ =
  std::numeric_limits<size_t>::max()-10;

/*!
  \brief data_name_space_t defines the various data namespaces that
  are available for registering and maintaining data.

  This type is provided as a convenience to avoid naming collisions.
 */
enum class data_name_space_t : size_t {
  user = 0,
  internal = __flecsi_internal_data_offset__
}; // enum class data_name_space_t

static const size_t flecsi_user_space =
  static_cast<size_t>(data_name_space_t::user);
static const size_t flecsi_internal =
  static_cast<size_t>(data_name_space_t::internal);

/*!
  \brief data_runtime_name_space_t defines the various data namespaces that
  are available for registering and maintaining data.

  This type is provided as a convenience to avoid naming collisions.
 */
enum class data_runtime_name_space_t : size_t {
  user = 0
}; // enum class data_runtime_name_space_t

static const size_t flecsi_runtime_user_space =
  static_cast<size_t>(data_runtime_name_space_t::user);

/*!
  \brief data_attribute_t defines different data attributes.

  This type should probably be pushed up in the interface so that users
  can define their own attributes.
 */
enum class data_attribute_t : bitfield_t::field_type_t {
  persistent = 0x0001,
  temporary = 0x0002
}; // enum class data_attribute_t

/*!
  \brief This exposes the persistent attribute so that it can be used
  without specifying the full type information.
 */
static constexpr bitfield_t::field_type_t persistent =
  static_cast<bitfield_t::field_type_t>(data_attribute_t::persistent);

/*!
  \brief This exposes the temporary attribute so that it can be used
  without specifying the full type information.
 */
static constexpr bitfield_t::field_type_t temporary =
  static_cast<bitfield_t::field_type_t>(data_attribute_t::temporary);

} // namespace flecsi

#endif // flecsi_data_constants_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
