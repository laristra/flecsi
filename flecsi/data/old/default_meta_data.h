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

#ifndef flecsi_default_meta_data_h
#define flecsi_default_meta_data_h

#include "flecsi/utils/bitfield.h"

/*!
 * \file default_meta_data.h
 * \authors bergen
 * \date Initial file creation: Feb 26, 2016
 */

namespace flecsi {
namespace data {

/*----------------------------------------------------------------------------*
 * struct default_data_meta_data_t
 *----------------------------------------------------------------------------*/

/*!
  \brief default_state_user_meta_data_t defines a default meta data type.

  This type should really never get used, i.e., the data specialization
  should provide a meta data type.  So far, this type has mostly been
  useful for testing.
 */
struct default_state_user_meta_data_t {

  void initialize(
    const size_t & site_id_, 
    utils::bitfield_t::field_type_t attributes_
  ) {
    site_id = site_id_;
    attributes = attributes_;
  } // initialize

  size_t site_id;
  utils::bitfield_t::field_type_t attributes;

}; // struct default_state_user_meta_data_t

} // namespace data
} // namespace flecsi

#endif // flecsi_default_meta_data_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
