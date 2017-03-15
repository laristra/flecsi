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

#ifndef flecsi_legion_global_h
#define flecsi_legion_global_h

//----------------------------------------------------------------------------//
// POLICY_NAMESPACE must be defined before including storage_type.h!!!
// Using this approach allows us to have only one storage_type_t
// definintion that can be used by all data policies -> code reuse...
#define POLICY_NAMESPACE legion
#include "flecsi/data/storage_type.h"
#undef POLICY_NAMESPACE
//----------------------------------------------------------------------------//

#include "flecsi/utils/const_string.h"
#include "flecsi/data/data_client.h"
#include "flecsi/data/data_handle.h"
#include "flecsi/data/common/data_types.h"

///
/// \file legion/global.h
/// \authors bergen
/// \date Initial file creation: Apr 17, 2016
///

namespace flecsi {
namespace data {
namespace legion {

//----------------------------------------------------------------------------//
// Global accessor.
//----------------------------------------------------------------------------//

///
/// FIXME documentation
///
template<typename T, typename MD>
struct global_accessor_t {

  //--------------------------------------------------------------------------//
  // Type definitions.
  //--------------------------------------------------------------------------//

  using meta_data_t = MD;
  using user_meta_data_t = typename meta_data_t::user_meta_data_t;

  //--------------------------------------------------------------------------//
  // Constructors.
  //--------------------------------------------------------------------------//

  ///
  /// FIXME documentation
  ///
  global_accessor_t() {}


  ///
  /// FIXME documentation
  ///
  global_accessor_t(
    const std::string & label,
    T * data,
    const user_meta_data_t & meta_data
  )
  :
    label_(label),
    data_(data),
    meta_data_(meta_data)
  {}

  ///
  /// FIXME documentation
  ///
  const T *
  operator -> () const
  {
    return data_;
  } // operator ->

  ///
  /// FIXME documentation
  ///
  T *
  operator -> ()
  {
    return data_;
  } // operator ->

  ///
  /// \brief Test to see if this accessor is empty.
  ///
  /// \return true if registered.
  ///
  operator bool() const
  {
    return data_ != nullptr;
  } // operator bool

  ///
  /// FIXME documentation
  ///
  const std::string &
  label() const
  {
    return label_;
  } // label

  ///
  /// FIXME documentation
  ///
  bitset_t &
  attributes()
  {
  } // attributes

  ///
  /// FIXME documentation
  ///
  const bitset_t &
  attributes() const
  {
  } // attributes

private:

  std::string label_ = "";
  T * data_ = nullptr;
  const user_meta_data_t & meta_data_ = {};
  bitset_t * user_attributes_ = nullptr;
}; // struct global_accessor_t

//----------------------------------------------------------------------------//
// Global handle.
//----------------------------------------------------------------------------//
///
/// FIXME documentation
///
template<typename T>
struct global_handle_t {
}; // struct global_handle_t

//----------------------------------------------------------------------------//
// Global storage type.
//----------------------------------------------------------------------------//

///
/// FIXME: Global storage type.
///
template<typename DS, typename MD>
struct storage_type_t<global, DS, MD> {

  //--------------------------------------------------------------------------//
  // Type definitions.
  //--------------------------------------------------------------------------//

  using data_store_t = DS;
  using meta_data_t = MD;

  template<typename T>
  using accessor_t = global_accessor_t<T, MD>;

  template<typename T>
  using handle_t = global_handle_t<T>;

  //--------------------------------------------------------------------------//
  // Data registration.
  //--------------------------------------------------------------------------//

  ///
  /// \tparam T Data type to register.
  /// \tparam NS Namespace.
  /// \tparam Args Variadic arguments that are passed to
  ///              metadata initialization.
  ///
  /// \param data_store A reference for accessing the low-level data.
  /// \param key A const string instance containing the variable name.
  /// \param runtime_namespace The runtime namespace to be used.
  /// \param The number of variable versions for this datum.
  ///
  template< 
    typename T,
    size_t NS,
    typename ... Args
  >
  static
  handle_t<T>
  register_data(
    const data_client_t & data_client,
    data_store_t & data_store,
    const utils::const_string_t & key,
    size_t versions,
    Args && ... args
  )
  {
    return {};    
  } // register_data

  //--------------------------------------------------------------------------//
  // Data accessors.
  //--------------------------------------------------------------------------//

  ///
  /// FIXME documentation
  ///
  template<
    typename T,
    size_t NS
  >
  static
  accessor_t<T>
  get_accessor(
    const data_client_t & data_client,
    data_store_t & data_store,
    const utils::const_string_t & key,
    size_t version
  )
  {
    return {};
  } // get_accessor

  //--------------------------------------------------------------------------//
  // Data handles.
  //--------------------------------------------------------------------------//

  ///
  /// FIXME documentation
  ///
  template<
    typename T,
    size_t NS
  >
  static
  handle_t<T>
  get_handle(
    const data_client_t & data_client,
    data_store_t & data_store,
    const utils::const_string_t & key
  )
  {
    return {};
  } // get_handle

  ///
  /// FIXME documentation
  ///
  template<
    typename T,
    size_t NS,
    typename Predicate
  >
  static
  std::vector<accessor_t<T>>
  get_accessors(
    const data_client_t & data_client,
    data_store_t & data_store,
    size_t version,
    Predicate && predicate,
    bool sorted
  )
  {

  }

  ///
  /// FIXME documentation
  ///
  template<
    typename T,
    typename Predicate
  >
  static
  std::vector<accessor_t<T>>
  get_accessors(
    const data_client_t & data_client,
    data_store_t & data_store,
    size_t version,
    Predicate && predicate,
    bool sorted
  )
  {

  }

  ///
  /// FIXME documentation
  ///
  template<
    typename T,
    size_t NS
  >
  static
  std::vector<accessor_t<T>>
  get_accessors(
    const data_client_t & data_client,
    data_store_t & data_store,
    size_t version,
    bool sorted
  )
  {

  }

  ///
  /// FIXME documentation
  ///
  template<
    typename T
  >
  static
  std::vector<accessor_t<T>>
  get_accessors(
    const data_client_t & data_client,
    data_store_t & data_store,
    size_t version,
    bool sorted
  )
  {

  }

}; // struct storage_type_t

} // namespace legion
} // namespace data
} // namespace flecsi

#endif // flecsi_legion_global_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
