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

#ifndef flecsi_default_storage_type_h
#define flecsi_default_storage_type_h

#include "flecsi/data/data_constants.h"

/*!
 * \file default_storage_type.h
 * \authors bergen
 * \date Initial file creation: Apr 15, 2016
 */

namespace flecsi
{
namespace data_model
{
namespace default_storage_policy
{

  /*!
    \struct storage_type_t

    \tparam T Specialization parameter.
    \tparam DS Data store type.
    \tparam MD Metadata type.
   */
  template<size_t T, typename DS, typename MD>
  struct storage_type_t {};

} // namespace default_storage_policy
} // namespace data_model
} // namespace flecsi

#endif // flecsi_default_storage_type_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
