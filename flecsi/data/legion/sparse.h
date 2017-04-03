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

#ifndef flecsi_legion_sparse_h
#define flecsi_legion_sparse_h

#include <map>
#include <algorithm>

//----------------------------------------------------------------------------//
// POLICY_NAMESPACE must be defined before including storage_type.h!!!
// Using this approach allows us to have only one storage_type_t
// definintion that can be used by all data policies -> code reuse...
#define POLICY_NAMESPACE legion
#include "flecsi/data/storage_type.h"
#undef POLICY_NAMESPACE
//----------------------------------------------------------------------------//

#include "flecsi/utils/const_string.h"

///
// \file legion/sparse.h
// \authors bergen
// \date Initial file creation: Apr 17, 2016
///

namespace flecsi {
namespace data {
namespace legion {

//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=//
// Helper type definitions.
//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=//

//----------------------------------------------------------------------------//
// Sparse accessor.
//----------------------------------------------------------------------------//

#if 0
using index_pair_ = std::pair<size_t, size_t>;

template<typename T>
struct material_value_{
  material_value_(size_t material)
  : material(material){}

  material_value_(size_t material, T value)
  : material(material),
  value(value){}

  material_value_(){}

  size_t material;
  T value;
};

static constexpr size_t INDICES_KEY = 0;
static constexpr size_t MATERIALS_KEY = 1;
#endif

template<typename T, typename MD>
struct sparse_accessor_t {

  //--------------------------------------------------------------------------//
  // Type definitions.
  //--------------------------------------------------------------------------//

  using meta_data_t = MD;
  using user_meta_data_t = typename meta_data_t::user_meta_data_t;
  
  //--------------------------------------------------------------------------//
  // Constructors.
  //--------------------------------------------------------------------------//

  ///
  //
  ///
  sparse_accessor_t() {}

  ///
  //
  ///
  sparse_accessor_t(
    const std::string & label,
    size_t version,
    meta_data_t & meta_data,
    const user_meta_data_t & user_meta_data
  )
  {
  } // sparse_accessor_t

  ///
  //
  ///
  T &
  operator () (
    size_t index,
    size_t material
  )
  {
  } // operator ()

  ///
  //
  ///
  void dump()
  {
  } // dump

  ///
  //
  ///
  T *
  data()
  {
    return nullptr;
  } // data

private:

}; // struct sparse_accessor_t

template<typename T, typename MD>
struct sparse_mutator_t {

  //--------------------------------------------------------------------------//
  // Type definitions.
  //--------------------------------------------------------------------------//
  
  using meta_data_t = MD;
  using user_meta_data_t = typename meta_data_t::user_meta_data_t;

  //--------------------------------------------------------------------------//
  // Constructors.
  //--------------------------------------------------------------------------//

  ///
  //
  ///
  sparse_mutator_t() {}

  ///
  //
  ///
  sparse_mutator_t(
    size_t num_slots,
    const std::string & label,
    size_t version,
    meta_data_t & meta_data,
    const user_meta_data_t & user_meta_data
  )
  {}

  ///
  //
  ///
  ~sparse_mutator_t()
  {
  } // ~sparse_mutator_t

  ///
  //
  ///
  T &
  operator () (
    size_t index,
    size_t material
  )
  {
  } // operator ()

  ///
  //
  ///
  T *
  data()
  {
    return nullptr;
  } // data

  ///
  //
  ///
  void
  commit()
  {
  } // commit

private:

}; // struct sparse_accessor_t

//----------------------------------------------------------------------------//
// Sparse handle.
//----------------------------------------------------------------------------//

template<typename T>
struct sparse_handle_t {
}; // struct sparse_handle_t

//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=//
// Main type definition.
//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=//

///
// FIXME: Sparse storage type.
///
template<typename DS, typename MD>
struct storage_type_t<sparse, DS, MD> {

  //--------------------------------------------------------------------------//
  // Type definitions.
  //--------------------------------------------------------------------------//

  using data_store_t = DS;
  using meta_data_t = MD;

  template<typename T>
  using accessor_t = sparse_accessor_t<T, MD>;

  template<typename T>
  using mutator_t = sparse_mutator_t<T, MD>;

  template<typename T>
  using handle_t = sparse_handle_t<T>;

  //--------------------------------------------------------------------------//
  // Data registration.
  //--------------------------------------------------------------------------//

  ///
  //
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
    size_t indices,
    size_t num_entries,
    Args && ... args
  )
  {
    return {};
  } // register_data

  ///
  //
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

  ///
  //
  ///
  template<
    typename T,
    size_t NS
  >
  static
  mutator_t<T>
  get_mutator(
    const data_client_t & data_client,
    data_store_t & data_store,
    const utils::const_string_t & key,
    size_t slots,
    size_t version
  )
  {
    return {};
  } // get_accessor

  ///
  //
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
    const utils::const_string_t & key,
    size_t version
  )
  {
    return {};
  } // get_handle

}; // struct storage_type_t

} // namespace legion
} // namespace data
} // namespace flecsi

#endif // flecsi_legion_sparse_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
