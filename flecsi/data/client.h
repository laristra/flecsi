/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_data_client_h
#define flecsi_data_client_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Jun 21, 2017
//----------------------------------------------------------------------------//

#include "flecsi/data/storage.h"
#include "flecsi/data/data_client_handle.h"
#include "flecsi/execution/context.h"

namespace flecsi {
namespace data {

//----------------------------------------------------------------------------//
//! FIXME: Description of class
//----------------------------------------------------------------------------//

template<
  typename DATA_POLICY
>
struct client_data__
{
  //--------------------------------------------------------------------------//
  //! Register a data client with the FleCSI runtime.
  //!
  //! @tparam DATA_CLIENT_TYPE The data client type.
  //! @tparam NAMESPACE_HASH   The namespace key. Namespaces allow separation
  //!                          of attribute names to avoid collisions.
  //! @tparam NAME_HASH        The attribute name.
  //!
  //! @ingroup data
  //--------------------------------------------------------------------------//

  template<
    typename DATA_CLIENT_TYPE,
    size_t NAMESPACE_HASH,
    size_t NAME_HASH
  >
  static
  bool
  register_data_client(
    std::string const & name
  )
  {
    using wrapper_t = typename DATA_POLICY::template client_wrapper__<
      DATA_CLIENT_TYPE,
      NAMESPACE_HASH,
      NAME_HASH
    >;

    const size_t client_key = typeid(DATA_CLIENT_TYPE).hash_code();
    const size_t key = NAMESPACE_HASH ^ NAME_HASH;

    return storage_t::instance().register_client(client_key, key,
      wrapper_t::register_callback);
  } // register_data_client

  template<
    typename DATA_CLIENT_TYPE,
    size_t NAMESPACE_HASH,
    size_t NAME_HASH
  >
  static
  data_client_handle__<DATA_CLIENT_TYPE>
  get_client_handle()
  {
    auto& context = execution::context_t::instance();
    



    data_client_handle__<DATA_CLIENT_TYPE> h;



    return h;
  }

}; // struct client_data__

} // namespace data
} // namespace flecsi

//----------------------------------------------------------------------------//
// This include file defines the FLECSI_RUNTIME_DATA_POLICY used below.
//----------------------------------------------------------------------------//

#include "flecsi_runtime_data_policy.h"

namespace flecsi {
namespace data {

using client_data_t = client_data__<FLECSI_RUNTIME_DATA_POLICY>;

} // namespace data
} // namespace flecsi

#endif // flecsi_data_client_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
