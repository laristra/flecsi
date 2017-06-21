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

///
// \file legion/global.h
// \authors bergen
// \date Initial file creation: Apr 17, 2016
///

namespace flecsi {
namespace data {
namespace legion {

//----------------------------------------------------------------------------//
// Global handle.
//----------------------------------------------------------------------------//

template<typename T>
struct global_handle_t {
}; // struct global_handle_t

//----------------------------------------------------------------------------//
// Global storage type.
//----------------------------------------------------------------------------//

///
// FIXME: Global storage type.
///
template<>
struct storage_type_t<global> {

  //--------------------------------------------------------------------------//
  // Type definitions.
  //--------------------------------------------------------------------------//

  template<typename T>
  using handle_t = global_handle_t<T>;

  //--------------------------------------------------------------------------//
  // Data handles.
  //--------------------------------------------------------------------------//

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
  std::vector<handle_t<T>>
  get_handles(
    const data_client_t & data_client,
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
  std::vector<handle_t<T>>
  get_handles(
    const data_client_t & data_client,
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
  std::vector<handle_t<T>>
  get_handles(
    const data_client_t & data_client,
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
  std::vector<handle_t<T>>
  get_handles(
    const data_client_t & data_client,
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
