/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_data_field_h
#define flecsi_data_field_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Jun 21, 2017
//----------------------------------------------------------------------------//

#include "flecsi/data/common/registration_wrapper.h"
#include "flecsi/data/storage.h"

namespace flecsi {
namespace data {

//----------------------------------------------------------------------------//
//! FIXME: Description of class
//----------------------------------------------------------------------------//

template<
  typename DATA_POLICY
>
struct field_data__
{

  //--------------------------------------------------------------------------//
  //! Register a field with the FleCSI runtime. This method should be thought
  //! of as registering a field attribute on the given data client type.
  //! All instances of the client type will have this attribute. However,
  //! this does not mean that each data client instance will have an
  //! an instance of the attribute. Attribute instances will be created
  //! only when they are actually mapped into a task.
  //!
  //! @tparam DATA_CLIENT_TYPE The data client type on which the data
  //!                          attribute should be registered.
  //! @tparam STORAGE_TYPE     The storage type for the data attribute.
  //! @tparam DATA_TYPE        The data type, e.g., double. This may be
  //!                          P.O.D. or a user-defined type that is
  //!                          trivially-copyable.
  //! @tparam NAMESPACE_HASH   The namespace key. Namespaces allow separation
  //!                          of attribute names to avoid collisions.
  //! @tparam NAME_HASH        The attribute name.
  //! @tparam INDEX_SPACE      The index space identifier.
  //! @tparam VERSIONS         The number of versions that shall be associated
  //!                          with this attribute.
  //!
  //! @param name The string version of the field name.
  //!
  //! @ingroup data
  //--------------------------------------------------------------------------//

  template<
    typename DATA_CLIENT_TYPE,
    size_t STORAGE_TYPE,
    typename DATA_TYPE,
    size_t NAMESPACE_HASH,
    size_t NAME_HASH,
    size_t VERSIONS,
    size_t INDEX_SPACE = 0
  >
  static
  bool
  register_field(
    std::string const & name
  )
  {
    using wrapper_t = field_registration_wrapper__<
      DATA_CLIENT_TYPE,
      STORAGE_TYPE,
      DATA_TYPE,
      NAMESPACE_HASH,
      NAME_HASH,
      VERSIONS,
      INDEX_SPACE
    >;

    const size_t client_key = typeid(DATA_CLIENT_TYPE).hash_code();
    const size_t key = NAMESPACE_HASH ^ NAME_HASH;

    for(size_t i(0); i<VERSIONS; ++i) {
      if(!storage_t::instance().register_field(client_key, key,
        wrapper_t::register_callback)) {
        return false;
      } // if
    } // for

    return true;
  } // register_field

  //--------------------------------------------------------------------------//
  //! Return the handle associated with the given parameters and data client.
  //!
  //! @tparam DATA_CLIENT_TYPE The data client type on which the data
  //!                          attribute should be registered.
  //! @tparam STORAGE_TYPE     The storage type for the data attribute.
  //! @tparam DATA_TYPE        The data type, e.g., double. This may be
  //!                          P.O.D. or a user-defined type that is
  //!                          trivially-copyable.
  //! @tparam NAMESPACE_HASH   The namespace key. Namespaces allow separation
  //!                          of attribute names to avoid collisions.
  //! @tparam NAME_HASH        The attribute name.
  //! @tparam INDEX_SPACE      The index space identifier.
  //! @tparam VERSION          The data version.
  //!
  //! @ingroup data
  //--------------------------------------------------------------------------//

  template<
    typename DATA_CLIENT_TYPE,
    size_t STORAGE_TYPE,
    typename DATA_TYPE,
    size_t NAMESPACE_HASH,
    size_t NAME_HASH,
    size_t VERSION = 0,
    size_t PERMISSIONS
  >
  static
  decltype(auto)
  get_handle(
    const data_client_handle__<DATA_CLIENT_TYPE, PERMISSIONS>& client_handle
  )
  {
    using storage_type_t =
      typename DATA_POLICY::template storage_type__<STORAGE_TYPE>;

    return storage_type_t::template get_handle<
      DATA_CLIENT_TYPE,
      DATA_TYPE,
      NAMESPACE_HASH,
      NAME_HASH,
      VERSION
    >
    (client_handle);
  } // get_handle

  //--------------------------------------------------------------------------//
  //! Return all handles of the given storage type, data type, and
  //! namespace that satisfy a predicate function.
  //!
  //! @tparam STORAGE_TYPE   The storage type for the data attribute.
  //! @tparam DATA_TYPE      The data type, e.g., double. This may be
  //!                        P.O.D. or a user-defined type that is
  //!                        trivially-copyable.
  //! @tparam NAMESPACE_HASH The namespace key. Namespaces allow separation
  //!                        of attribute names to avoid collisions.
  //! @tparam PREDICATE      The data version.
  //!
  //! @param client    The data client instance.
  //! @param version   The data version to return.
  //! @param predicate An instance of the predicate function that will be
  //!                  used to select the individual data elements.
  //! @param sorted    Put the return data into sorted order.
  //!
  //! @ingroup data
  //--------------------------------------------------------------------------//

  template<
    size_t STORAGE_TYPE,
    typename DATA_TYPE,
    size_t NAMESPACE_HASH,
    typename PREDICATE
  >
  static
  decltype(auto)
  get_handles(
    const data_client_t & client,
    size_t version,
    PREDICATE && predicate,
    bool sorted = true
  )
  {
    return DATA_POLICY::template get_handles<
      STORAGE_TYPE,
      DATA_TYPE,
      NAMESPACE_HASH,
      PREDICATE
    >
    (client, version, std::forward<PREDICATE>(predicate), sorted);
  } // get_handles

  //--------------------------------------------------------------------------//
  //! Return all handles of the given storage type, and data type that
  //! satisfy a predicate function.
  //!
  //! @tparam STORAGE_TYPE The storage type for the data attribute.
  //! @tparam DATA_TYPE    The data type, e.g., double. This may be P.O.D.
  //!                      or a user-defined type that is trivially-copyable.
  //! @tparam PREDICATE    The data version.
  //!
  //! @param client    The data client instance.
  //! @param version   The data version to return.
  //! @param predicate An instance of the predicate function that will be
  //!                  used to select the individual data elements.
  //! @param sorted    Put the return data into sorted order.
  //!
  //! @ingroup data
  //--------------------------------------------------------------------------//

  template<
    size_t STORAGE_TYPE,
    typename DATA_TYPE,
    typename PREDICATE
  >
  static
  decltype(auto)
  get_handles(
    const data_client_t & client,
    size_t version,
    PREDICATE && predicate,
    bool sorted = true
  )
  {
    return DATA_POLICY::template get_handles<
      STORAGE_TYPE,
      DATA_TYPE,
      PREDICATE
    >
    (client, version, std::forward<PREDICATE>(predicate), sorted);
  } // get_handles

}; // struct field_data__

} // namespace data
} // namespace flecsi

//----------------------------------------------------------------------------//
// This include file defines the FLECSI_RUNTIME_DATA_POLICY used below.
//----------------------------------------------------------------------------//

#include "flecsi/runtime/flecsi_runtime_data_policy.h"

namespace flecsi {
namespace data {

using field_data_t = field_data__<FLECSI_RUNTIME_DATA_POLICY>;

} // namespace data
} // namespace flecsi

#endif // flecsi_data_field_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
