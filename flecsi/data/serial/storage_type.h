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

#ifndef flecsi_serial_storage_type_h
#define flecsi_serial_storage_type_h

#include "flecsi/data/data_constants.h"

/*!
 * \file serial/storage_type.h
 * \authors bergen
 * \date Initial file creation: Apr 15, 2016
 */

namespace flecsi {
namespace serial_storage_policy {

  /*!
    \struct storage_type_t

    \tparam T Specialization parameter.
    \tparam DS Data store type.
    \tparam MD Metadata type.
   */
  template<size_t T, typename DS, typename MD>
  struct storage_type_t {};

} // namespace serial_storage_policy
} // namespace flecsi

#endif // flecsi_serial_storage_type_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
