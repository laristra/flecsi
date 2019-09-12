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

//----------------------------------------------------------------------------//
// POLICY_NAMESPACE must be defined before including storage_class.h!!!
// Using this approach allows us to have only one storage_class_u
// definintion that can be used by all data policies -> code reuse...
#define POLICY_NAMESPACE legion
#include <flecsi/data/storage_class.h>
#undef POLICY_NAMESPACE
//----------------------------------------------------------------------------//

#include <flecsi/data/common/privilege.h>
#include <flecsi/data/data_client.h>
#include <flecsi/data/dense_data_handle.h>
#include <flecsi/data/storage.h>
#include <flecsi/execution/context.h>
#include <flecsi/utils/hash.h>

namespace flecsi {
namespace data {
namespace legion {

/*!
 The dense_handle_t type provides logically array-based access to data
 variables that have been registered in the data model.

 @tparam T                     The type of the data variable. If this type
                               is not consistent with the type used to
                               register the data, bad things can happen.
                               However, it can be useful to reinterpret
                               the type, e.g., when writing raw bytes.
                               This class is part of the low-level
                               \e flecsi interface, so it is assumed that
                               you know what you are doing...
 @tparam EXCLUSIVE_PERMISSIONS The permissions to the exclusive indices.
 @tparam SHARED_PERMISSIONS    The permissions to the shared indices.
 @tparam GHOST_PERMISSIONS     The permissions to the ghost indices.

 @ingroup data
*/

template<typename T,
  size_t EXCLUSIVE_PERMISSIONS,
  size_t SHARED_PERMISSIONS,
  size_t GHOST_PERMISSIONS>
struct dense_handle_t : public dense_data_handle_u<T,
                          EXCLUSIVE_PERMISSIONS,
                          SHARED_PERMISSIONS,
                          GHOST_PERMISSIONS> {
  /*!
   Type definitions.
   */

  using base_t = dense_data_handle_u<T,
    EXCLUSIVE_PERMISSIONS,
    SHARED_PERMISSIONS,
    GHOST_PERMISSIONS>;

  /*!
   Constructors.
   */

  dense_handle_t() {}

  /*!
    Destructor.
   */

  ~dense_handle_t() {}

  template<typename, size_t, size_t, size_t>
  friend class dense_handle_t;

}; // struct dense_handle_t

//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=//
// Main type definition.
//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=//

//----------------------------------------------------------------------------//
// Dense storage type.
//----------------------------------------------------------------------------//

/*!
 FIXME: Dense storage type.
 */

template<>
struct storage_class_u<dense> {

  /*!
    Type definitions.
   */

  template<typename T,
    size_t EXCLUSIVE_PERMISSIONS,
    size_t SHARED_PERMISSIONS,
    size_t GHOST_PERMISSIONS>
  using handle_t = dense_handle_t<T,
    EXCLUSIVE_PERMISSIONS,
    SHARED_PERMISSIONS,
    GHOST_PERMISSIONS>;

  //--------------------------------------------------------------------------//
  // Data handles.
  //--------------------------------------------------------------------------//

  /*!
    FIXME documentation
   */

  template<typename T, size_t NAMESPACE, typename Predicate>
  static decltype(auto) get_handles(const data_client_t & data_client,
    size_t version,
    Predicate && predicate,
    bool sorted) {}

  template<typename T, typename Predicate>
  static decltype(auto) get_handles(const data_client_t & data_client,
    size_t version,
    Predicate && predicate,
    bool sorted) {}

  template<typename T, size_t NAMESPACE>
  static decltype(auto)
  get_handles(const data_client_t & data_client, size_t version, bool sorted) {}

  template<typename T>
  static decltype(auto)
  get_handles(const data_client_t & data_client, size_t version, bool sorted) {}

  /*!
    Obtain a dense data handle to a field that resides on the specified data
    client.

    @param client_handle the data client that owns the field.

    @tparam DATA_CLIENT_TYPE The data client type.
    @tparam NAMESPACE_HASH   The namespace key. Namespaces allow separation
                             of attribute names to avoid collisions.
    @tparam NAME             The field name.
    @tparam VERSION          The field version.
    @tparam PERMISSIONS      The data client permissions.
   */
  template<typename DATA_CLIENT_TYPE,
    typename DATA_TYPE,
    size_t NAMESPACE,
    size_t NAME,
    size_t VERSION,
    size_t PERMISSIONS>
  static handle_t<DATA_TYPE, 0, 0, 0> get_handle(
    const data_client_handle_u<DATA_CLIENT_TYPE, PERMISSIONS> & client_handle) {
    static_assert(
      VERSION < utils::hash::field_max_versions, "max field version exceeded");

    handle_t<DATA_TYPE, 0, 0, 0> h;

    auto & context = execution::context_t::instance();

    auto & field_info = context.get_field_info_from_name(
      typeid(typename DATA_CLIENT_TYPE::type_identifier_t).hash_code(),
      utils::hash::field_hash<NAMESPACE, NAME>(VERSION));

    size_t index_space = field_info.index_space;
    auto & ism = context.index_space_data_map();

    h.data_client_hash = field_info.data_client_hash;
    h.primary_lp = ism[index_space].primary_lp;
    h.exclusive_lp = ism[index_space].exclusive_lp;
    h.shared_lp = ism[index_space].shared_lp;
    h.ghost_owners_lp = ism[index_space].ghost_owners_lp;
    h.ghost_lp = ism[index_space].ghost_lp;
    h.ghost_is_readable = &(ism[index_space].ghost_is_readable[field_info.fid]);
    h.write_phase_started =
      &(ism[index_space].write_phase_started[field_info.fid]);

    h.entire_region = ism[index_space].entire_region;
    h.color_partition = ism[index_space].color_partition;
    h.fid = field_info.fid;
    h.index_space = field_info.index_space;
    h.state = context.execution_state();

    return h;
  } // get_handle

}; // struct storage_class_u

} // namespace legion
} // namespace data
} // namespace flecsi
