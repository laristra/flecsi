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

#ifndef flecsi_default_scalar_h
#define flecsi_default_scalar_h

#include "flecsi/utils/const_string.h"
#include "flecsi/data/default/default_storage_type.h"

/*!
 * \file default_scalar.h
 * \authors bergen
 * \date Initial file creation: Apr 17, 2016
 */

namespace flecsi
{
namespace data_model
{
namespace default_storage_policy
{

template<typename T, typename MD>
struct scalar_accessor_t {
}; // struct scalar_accessor_t

template<typename T>
struct scalar_handle_t {
}; // struct scalar_handle_t

/*!
  FIXME: Scalar storage type.
 */
template<typename DS, typename MD>
struct storage_type_t<scalar, DS, MD> {

  /*--------------------------------------------------------------------------*
   * Type definitions.
   *--------------------------------------------------------------------------*/

  using data_store_t = DS;
  using meta_data_t = MD;

  template<typename T>
  using accessor_t = scalar_accessor_t<T, MD>;

  template<typename T>
  using handle_t = scalar_handle_t<T>;

  /*--------------------------------------------------------------------------*
   * Data registration.
   *--------------------------------------------------------------------------*/

  /*!
   */
  template<typename T, size_t NS, typename ... Args>
  static decltype(auto) register_data(data_store_t & data_store,
    uintptr_t runtime_namespace, const const_string_t & key,
    Args && ... args) {
  } // register_data

  /*--------------------------------------------------------------------------*
   * Data accessors.
   *--------------------------------------------------------------------------*/

  /*!
   */
  template<typename T, size_t NS>
  static accessor_t<T> get_accessor(data_store_t & data_store,
    uintptr_t runtime_namespace, const const_string_t & key) {
    return {};
  } // get_accessor

  /*--------------------------------------------------------------------------*
   * Data handles.
   *--------------------------------------------------------------------------*/

  /*!
   */
  template<typename T, size_t NS>
  static handle_t<T> get_handle(data_store_t & data_store,
    uintptr_t runtime_namespace, const const_string_t & key) {
    return {};
  } // get_handle

}; // struct storage_type_t

} // namespace default_storage_policy
} // namespace data_model
} // namespace flecsi

#endif // flecsi_default_scalar_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
