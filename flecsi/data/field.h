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

/*! @file */

#include <flecsi-config.h>

#include <flecsi/data/common/data_types.h>
#include <flecsi/data/common/registration_wrapper.h>
#include <flecsi/data/common/row_vector.h>
#include <flecsi/data/common/serdez.h>
#include <flecsi/data/storage.h>
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

    @tparam DATA_CLIENT_TYPE The data client type on which the data
                             attribute should be registered.
    @tparam STORAGE_CLASS    The storage type for the data attribute.
    @tparam DATA_TYPE        The data type, e.g., double. This may be
                             P.O.D. or a user-defined type that is
                             trivially-copyable.
    @tparam NAMESPACE_HASH   The namespace key. Namespaces allow separation
                             of attribute names to avoid collisions.
    @tparam NAME_HASH        The attribute name.
    @tparam INDEX_SPACE      The index space identifier.
    @tparam VERSIONS         The number of versions that shall be associated
                             with this attribute.

    @param name The string version of the field name.

    @ingroup data
   */

  template<typename DATA_CLIENT_TYPE,
    size_t STORAGE_CLASS,
    typename DATA_TYPE,
    size_t NAMESPACE_HASH,
    size_t NAME_HASH,
    size_t VERSIONS,
    size_t INDEX_SPACE = 0>
  static bool register_field(std::string const & name) {
    static_assert(VERSIONS <= utils::hash::field_max_versions,
      "max field versions exceeded");

    using wrapper_t =
      field_registration_wrapper_u<DATA_CLIENT_TYPE, STORAGE_CLASS, DATA_TYPE,
        NAMESPACE_HASH, NAME_HASH, VERSIONS, INDEX_SPACE>;

    const size_t client_type_key =
      typeid(typename DATA_CLIENT_TYPE::type_identifier_t).hash_code();

    for(size_t version(0); version < VERSIONS; ++version) {
      const size_t key =
        utils::hash::field_hash<NAMESPACE_HASH, NAME_HASH>(version);

      if(!storage_t::instance().register_field(
           client_type_key, key, wrapper_t::register_callback)) {
        return false;
      } // if
    } // for

    // CRF:  yes, this is ugly, but I can't find any other way to
    //       make it work
#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_legion
    if constexpr(STORAGE_CLASS == ragged || STORAGE_CLASS == sparse) {
      using namespace Legion;
      // CRF hack - for now, use lowest bits of name_hash as serdez id
      int sid = NAME_HASH & 0x7FFFFFFF;
      if constexpr(STORAGE_CLASS == sparse) {
        Runtime::register_custom_serdez_op<
          serdez_u<row_vector_u<sparse_entry_value_u<DATA_TYPE>>>>(sid);
      }
      else {
        Runtime::register_custom_serdez_op<serdez_u<row_vector_u<DATA_TYPE>>>(
          sid);
      }
    } // if
#endif

    return true;
  } // register_field

  /*!
    Return the handle associated with the given parameters and data client.

    @tparam DATA_CLIENT_TYPE The data client type on which the data
                             attribute should be registered.
    @tparam STORAGE_CLASS    The storage type for the data attribute.
    @tparam DATA_TYPE        The data type, e.g., double. This may be
                             P.O.D. or a user-defined type that is
                             trivially-copyable.
    @tparam NAMESPACE_HASH   The namespace key. Namespaces allow separation
                             of attribute names to avoid collisions.
    @tparam NAME_HASH        The attribute name.
    @tparam INDEX_SPACE      The index space identifier.
    @tparam VERSION          The data version.

    @ingroup data
   */

  template<typename DATA_CLIENT_TYPE,
    size_t STORAGE_CLASS,
    typename DATA_TYPE,
    size_t NAMESPACE_HASH,
    size_t NAME_HASH,
    size_t VERSION = 0,
    size_t PERMISSIONS>
  static decltype(auto) get_handle(
    const data_client_handle_u<DATA_CLIENT_TYPE, PERMISSIONS> & client_handle) {
    static_assert(
      VERSION < utils::hash::field_max_versions, "max field version exceeded");

    using storage_class_t =
      typename DATA_POLICY::template storage_class_u<STORAGE_CLASS>;

    return storage_class_t::template get_handle<DATA_CLIENT_TYPE, DATA_TYPE,
      NAMESPACE_HASH, NAME_HASH, VERSION>(client_handle);
  } // get_handle

  /*!
    Return the mutator associated with the given parameters and data client.

    @tparam DATA_CLIENT_TYPE The data client type on which the data
                             attribute should be registered.
    @tparam STORAGE_CLASS    The storage type for the data attribute.
    @tparam DATA_TYPE        The data type, e.g., double. This may be
                             P.O.D. or a user-defined type that is
                             trivially-copyable.
    @tparam NAMESPACE_HASH   The namespace key. Namespaces allow separation
                             of attribute names to avoid collisions.
    @tparam NAME_HASH        The attribute name.
    @tparam INDEX_SPACE      The index space identifier.
    @tparam VERSION          The data version.

    @ingroup data
   */

  template<typename DATA_CLIENT_TYPE,
    size_t STORAGE_CLASS,
    typename DATA_TYPE,
    size_t NAMESPACE_HASH,
    size_t NAME_HASH,
    size_t VERSION = 0,
    size_t PERMISSIONS>
  static decltype(auto) get_mutator(
    const data_client_handle_u<DATA_CLIENT_TYPE, PERMISSIONS> & client_handle,
    size_t slots) {
    static_assert(
      VERSION < utils::hash::field_max_versions, "max field version exceeded");

    using storage_class_t =
      typename DATA_POLICY::template storage_class_u<STORAGE_CLASS>;

    return storage_class_t::template get_mutator<DATA_CLIENT_TYPE, DATA_TYPE,
      NAMESPACE_HASH, NAME_HASH, VERSION>(client_handle, slots);
  } // get_mutator

  /*!
    Return all handles of the given storage type, data type, and
    namespace that satisfy a predicate function.

    @tparam STORAGE_CLASS  The storage type for the data attribute.
    @tparam DATA_TYPE      The data type, e.g., double. This may be
                           P.O.D. or a user-defined type that is
                           trivially-copyable.
    @tparam NAMESPACE_HASH The namespace key. Namespaces allow separation
                           of attribute names to avoid collisions.
    @tparam PREDICATE      The data version.

    @param client    The data client instance.
    @param version   The data version to return.
    @param predicate An instance of the predicate function that will be
                     used to select the individual data elements.
    @param sorted    Put the return data into sorted order.

    @ingroup data
   */

  template<size_t STORAGE_CLASS,
    typename DATA_TYPE,
    size_t NAMESPACE_HASH,
    typename PREDICATE>
  static decltype(auto) get_handles(const data_client_t & client,
    size_t version,
    PREDICATE && predicate,
    bool sorted = true) {
    return DATA_POLICY::template get_handles<STORAGE_CLASS, DATA_TYPE,
      NAMESPACE_HASH, PREDICATE>(
      client, version, std::forward<PREDICATE>(predicate), sorted);
  } // get_handles

  /*!
    Return all handles of the given storage type, and data type that
    satisfy a predicate function.

    @tparam STORAGE_CLASS The storage type for the data attribute.
    @tparam DATA_TYPE     The data type, e.g., double. This may be P.O.D.
                          or a user-defined type that is trivially-copyable.
    @tparam PREDICATE     The data version.

    @param client    The data client instance.
    @param version   The data version to return.
    @param predicate An instance of the predicate function that will be
                     used to select the individual data elements.
    @param sorted    Put the return data into sorted order.

    @ingroup data
   */

  template<size_t STORAGE_CLASS, typename DATA_TYPE, typename PREDICATE>
  static decltype(auto) get_handles(const data_client_t & client,
    size_t version,
    PREDICATE && predicate,
    bool sorted = true) {
    return DATA_POLICY::template get_handles<STORAGE_CLASS, DATA_TYPE,
      PREDICATE>(client, version, std::forward<PREDICATE>(predicate), sorted);
  } // get_handles

}; // struct field_interface_u

} // namespace data
} // namespace flecsi

//----------------------------------------------------------------------------//
// This include file defines the FLECSI_RUNTIME_DATA_POLICY used below.
//----------------------------------------------------------------------------//

#include <flecsi/runtime/flecsi_runtime_data_policy.h>

namespace flecsi {
namespace data {

using field_interface_t = field_interface_u<FLECSI_RUNTIME_DATA_POLICY>;

} // namespace data
} // namespace flecsi
