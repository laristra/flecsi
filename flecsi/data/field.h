/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

/*!
  @file

  This file contains the C++ data model interface \em field_interface_t
  for FleCSI field data structures. The \em field_interface_t type is a
  specialization of the \em field_interface_u type on the backend runtimne
  policy that is selected at compile time.
 */

#include <flecsi/data/common/field_registration.h>
#include <flecsi/execution/context.h>
#include <flecsi/utils/hash.h>

namespace flecsi {
namespace data {

/*!
  The field_interface_u type provides a high-level field interface that
  is implemented by the given data policy.

  @tparam DATA_POLICY The backend data policy.

  @ingroup data
 */

template<typename DATA_POLICY>
struct field_interface_u {

  /*!
    Register a field with the FleCSI runtime. This method should be thought
    of as registering a field attribute on the given data client type.
    All instances of the client type will have this attribute. However,
    this does not mean that each data client instance will have an
    an instance of the attribute. Attribute instances will be created
    only when they are actually mapped into a task.

    @tparam CLIENT_TYPE    The data client type on which the data
                           attribute should be registered.
    @tparam STORAGE_CLASS  The storage type for the data attribute.
    @tparam DATA_TYPE      The data type, e.g., double. This may be
                           P.O.D. or a user-defined type that is
                           trivially-copyable.
    @tparam NAMESPACE      The namespace key. Namespaces allow separation
                           of attribute names to avoid collisions.
    @tparam NAME           The attribute name.
    @tparam VERSIONS       The number of versions that shall be associated
                           with this attribute.
    @tparam INDEX_SPACE    The index space identifier.

    @param name The string version of the field name.

    @ingroup data
   */

  template<typename CLIENT_TYPE,
    size_t STORAGE_CLASS,
    typename DATA_TYPE,
    size_t NAMESPACE,
    size_t NAME,
    size_t VERSIONS,
    size_t INDEX_SPACE = 0>
  static bool register_field(std::string const & name) {
    static_assert(VERSIONS <= utils::hash::field_max_versions,
      "max field versions exceeded");

    using registration_t = field_registration_u<CLIENT_TYPE, STORAGE_CLASS,
      DATA_TYPE, NAMESPACE, NAME, VERSIONS, INDEX_SPACE>;

    const size_t client_type_key =
      typeid(typename CLIENT_TYPE::type_identifier_t).hash_code();

    for(size_t version(0); version < VERSIONS; ++version) {
      const size_t key = utils::hash::field_hash<NAMESPACE, NAME>(version);

      if(!execution::context_t::instance().register_field(
           client_type_key, key, registration_t::register_callback)) {
        return false;
      } // if
    } // for

    return true;
  } // register_field

  /*!
    Return the handle associated with the given parameters and data client.

    @tparam CLIENT_TYPE   The data client type on which the data
                          attribute should be registered.
    @tparam STORAGE_CLASS The storage type for the data attribute.
    @tparam DATA_TYPE     The data type, e.g., double. This may be
                          P.O.D. or a user-defined type that is
                          trivially-copyable.
    @tparam NAMESPACE     The namespace key. Namespaces allow separation
                          of attribute names to avoid collisions.
    @tparam NAME          The attribute name.
    @tparam INDEX_SPACE   The index space identifier.
    @tparam VERSION       The data version.

    @ingroup data
   */

  template<typename CLIENT_TYPE,
    size_t STORAGE_CLASS,
    typename DATA_TYPE,
    size_t NAMESPACE,
    size_t NAME,
    size_t VERSION = 0,
    size_t PERMISSIONS>
  static decltype(auto) get_handle(
    const client_handle_u<CLIENT_TYPE, PERMISSIONS> & client_handle) {
    static_assert(
      VERSION < utils::hash::field_max_versions, "max field version exceeded");

    using storage_class_t =
      typename DATA_POLICY::template storage_class_u<STORAGE_CLASS,
        CLIENT_TYPE>;

    return storage_class_t::template get_handle<DATA_TYPE, NAMESPACE, NAME,
      VERSION>(client_handle);
  } // get_handle

#if 0
  /*!
    Return the mutator associated with the given parameters and data client.

    @tparam CLIENT_TYPE   The data client type on which the data
                          attribute should be registered.
    @tparam STORAGE_CLASS The storage type for the data attribute.
    @tparam DATA_TYPE     The data type, e.g., double. This may be
                          P.O.D. or a user-defined type that is
                          trivially-copyable.
    @tparam NAMESPACE     The namespace key. Namespaces allow separation
                          of attribute names to avoid collisions.
    @tparam NAME          The attribute name.
    @tparam INDEX_SPACE   The index space identifier.
    @tparam VERSION       The data version.

    @ingroup data
   */

  template<typename CLIENT_TYPE,
    size_t STORAGE_CLASS,
    typename DATA_TYPE,
    size_t NAMESPACE,
    size_t NAME,
    size_t VERSION = 0,
    size_t PERMISSIONS>
  static decltype(auto) get_mutator(
    const client_handle_u<CLIENT_TYPE, PERMISSIONS> & client_handle,
    size_t slots) {
    static_assert(
      VERSION < utils::hash::field_max_versions, "max field version exceeded");

    using storage_class_t =
      typename DATA_POLICY::template storage_class_u<STORAGE_CLASS>;

    return storage_class_t::template get_mutator<CLIENT_TYPE, DATA_TYPE,
      NAMESPACE, NAME, VERSION>(client_handle, slots);
  } // get_mutator
#endif

}; // struct field_interface_u

} // namespace data
} // namespace flecsi

//----------------------------------------------------------------------------//
// This include file defines the FLECSI_RUNTIME_DATA_POLICY used below.
//----------------------------------------------------------------------------//

#include <flecsi/runtime/data_policy.h>

namespace flecsi {
namespace data {

/*!
  The field_interface_t type is the high-level interface to the FleCSI
  field data model.

  @ingroup data
 */

using field_interface_t = field_interface_u<FLECSI_RUNTIME_DATA_POLICY>;

} // namespace data
} // namespace flecsi
