/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Triad National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

/*!
  @file

  This file contains specializations of the \em topology_registration_u
  type for the various FleCSI topology types. In general, the registration
  type provides a mechanism for topologies to register meta data with
  the runtime.
 */

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly!
#else
#include "flecsi/runtime/context_policy.hh"
#include "flecsi/topology/common/core.hh"
#include <flecsi/runtime/types.hh>
#include <flecsi/topology/internal/global.hh>
#include <flecsi/topology/internal/index.hh>
//#include <flecsi/topology/mesh_topology.hh>
#include <flecsi/utils/common.hh>
#include <flecsi/utils/const_string.hh>
#include <flecsi/utils/demangle.hh>
#include <flecsi/utils/flog.hh>
#include <flecsi/utils/hash.hh>
#include <flecsi/utils/tuple_walker.hh>
#endif

#include <string>
#include <tuple>

flog_register_tag(registration);

namespace flecsi {
namespace data {

/*!

 */

template<typename TOPOLOGY_TYPE, size_t NAMESPACE, size_t NAME>
struct topology_registration_u;

#if 0
/*!

 */

template<typename POLICY_TYPE, size_t NAMESPACE, size_t NAME>
struct topology_registration_u<
  flecsi::topology::mesh_topology_u<POLICY_TYPE>,
  NAMESPACE,
  NAME> {
  using CLIENT_TYPE = typename flecsi::topology::mesh_topology_u<POLICY_TYPE>;

  /*!

   */

  struct entity_walker_t
      : public flecsi::utils::tuple_walker_u<entity_walker_t> {

    template<typename T, T V>
    T value(utils::typeify_u<T, V>) {
      return V;
    }

    template<typename TUPLE_ENTRY_TYPE>
    void visit_type() {
      using INDEX_TYPE = typename std::tuple_element<0, TUPLE_ENTRY_TYPE>::type;
      using DOMAIN_TYPE =
        typename std::tuple_element<1, TUPLE_ENTRY_TYPE>::type;
      using ENTITY_TYPE =
        typename std::tuple_element<2, TUPLE_ENTRY_TYPE>::type;

      constexpr size_t entity_hash =
        utils::hash::client_entity_hash<NAMESPACE, NAME,
          INDEX_TYPE::value, DOMAIN_TYPE::value, ENTITY_TYPE::dimension>();

      using registration_t = field_registration_u<CLIENT_TYPE,
        flecsi::data::dense, ENTITY_TYPE, entity_hash, 0, 1, INDEX_TYPE::value>;

      const size_t type_key =
        typeid(CLIENT_TYPE).hash_code();

      using flecsi::utils::const_string_t;
      const size_t field_key =
        utils::hash::client_internal_field_hash<
          const_string_t("flecsi_internal_entity_data").hash(),
          NAMESPACE, NAME, INDEX_TYPE::value
        >();

      {
      flog_tag_guard(registration);
      flog(info) << "registering field for type id: " <<
        flecsi::utils::demangle(
          typeid(CLIENT_TYPE).name()
        ) << std::endl <<
        " index: " << INDEX_TYPE::value << std::endl <<
        " namespace: " << NAMESPACE << std::endl <<
        " name: " << NAME << std::endl;
      flog(info) << "new key: " <<
        utils::hash::client_internal_field_hash<
          const_string_t("flecsi_internal_entity_data").hash(),
          NAMESPACE, NAME, INDEX_TYPE::value>() << std::endl;
      } // scope

      execution::context_t::instance().register_field(
        type_key, field_key, registration_t::register_callback);

      using id_registration_t = field_registration_u<CLIENT_TYPE,
        flecsi::data::dense, utils::id_t, entity_hash, 0, 1, INDEX_TYPE::value>;

      const size_t id_key =
        utils::hash::client_internal_field_hash<
          const_string_t("flecsi_internal_entity_id").hash(),
          NAMESPACE, NAME, INDEX_TYPE::value
        >();

      execution::context_t::instance().register_field(
        type_key, id_key, id_registration_t::register_callback);

    } // visit_type

  }; // struct entity_walker_t

  struct connectivity_walker_u
      : public flecsi::utils::tuple_walker_u<connectivity_walker_u> {

    template<typename T, T V>
    T value(utils::typeify_u<T, V>) {
      return V;
    }

    template<typename TUPLE_ENTRY_TYPE>
    void visit_type() {
      using INDEX_TYPE = typename std::tuple_element<0, TUPLE_ENTRY_TYPE>::type;
      using DOMAIN_TYPE =
        typename std::tuple_element<1, TUPLE_ENTRY_TYPE>::type;
      using FROM_ENTITY_TYPE =
        typename std::tuple_element<2, TUPLE_ENTRY_TYPE>::type;
      using TO_ENTITY_TYPE =
        typename std::tuple_element<3, TUPLE_ENTRY_TYPE>::type;

      using entity_types_t = typename POLICY_TYPE::entity_types;

      constexpr size_t from_index_space =
        topology::find_index_space_u<std::tuple_size<entity_types_t>::value,
          entity_types_t, FROM_ENTITY_TYPE>::find();

      constexpr size_t to_index_space =
        topology::find_index_space_u<std::tuple_size<entity_types_t>::value,
          entity_types_t, TO_ENTITY_TYPE>::find();

      constexpr size_t adjacency_hash =
        utils::hash::client_adjacency_hash<NAMESPACE, NAME,
          INDEX_TYPE::value,
          DOMAIN_TYPE::value, // from and to domains are the same
          DOMAIN_TYPE::value, // for connectivity information
          FROM_ENTITY_TYPE::dimension, TO_ENTITY_TYPE::dimension>();

      using index_registration_t =
        field_registration_u<CLIENT_TYPE, flecsi::data::dense,
          utils::id_t, adjacency_hash, 0, 1, INDEX_TYPE::value>;

      const size_t type_key =
        typeid(CLIENT_TYPE).hash_code();

      using flecsi::utils::const_string_t;
      const size_t index_key =
        utils::hash::client_internal_field_hash<
          const_string_t("flecsi_internal_adjacency_index").hash(),
          NAMESPACE,
          NAME,
          INDEX_TYPE::value
        >();
      int ispace = INDEX_TYPE::value;
      execution::context_t::instance().register_field(
        type_key, index_key, index_registration_t::register_callback);

      using offset_registration_t =
        field_registration_u<CLIENT_TYPE, flecsi::data::dense,
          utils::offset_t, adjacency_hash, 0, 1, from_index_space>;

      // This field resides in the main entities (BLIS) index space, but
      // is unique to an adjacency, so it is registered using the
      // adjacency hash.
      const size_t offset_key =
        utils::hash::client_internal_field_hash<
          const_string_t("flecsi_internal_adjacency_offset").hash(),
          NAMESPACE,
          NAME,
          INDEX_TYPE::value
        >();

      execution::context_t::instance().register_field(
        type_key, offset_key, offset_registration_t::register_callback);
    } // visit_type

  }; // struct connectivity_walker_u

  struct binding_walker_u
      : public flecsi::utils::tuple_walker_u<binding_walker_u> {

    template<typename TUPLE_ENTRY_TYPE>
    void visit_type() {
      using INDEX_TYPE = typename std::tuple_element<0, TUPLE_ENTRY_TYPE>::type;
      using FROM_DOMAIN_TYPE =
        typename std::tuple_element<1, TUPLE_ENTRY_TYPE>::type;
      using TO_DOMAIN_TYPE =
        typename std::tuple_element<2, TUPLE_ENTRY_TYPE>::type;
      using FROM_ENTITY_TYPE =
        typename std::tuple_element<3, TUPLE_ENTRY_TYPE>::type;
      using TO_ENTITY_TYPE =
        typename std::tuple_element<4, TUPLE_ENTRY_TYPE>::type;

      using entity_types_t = typename POLICY_TYPE::entity_types;

      constexpr size_t from_index_space =
        topology::find_index_space_u<std::tuple_size<entity_types_t>::value,
          entity_types_t, FROM_ENTITY_TYPE>::find();

      constexpr size_t to_index_space =
        topology::find_index_space_u<std::tuple_size<entity_types_t>::value,
          entity_types_t, TO_ENTITY_TYPE>::find();

      constexpr size_t adjacency_hash =
        utils::hash::client_adjacency_hash<NAMESPACE, NAME,
          INDEX_TYPE::value, FROM_DOMAIN_TYPE::value, TO_DOMAIN_TYPE::value,
          FROM_ENTITY_TYPE::dimension, TO_ENTITY_TYPE::dimension>();

      using index_registration_t =
        field_registration_u<CLIENT_TYPE, flecsi::data::dense,
          utils::id_t, adjacency_hash, 0, 1, INDEX_TYPE::value>;

      const size_t type_key =
        typeid(CLIENT_TYPE).hash_code();

      using flecsi::utils::const_string_t;
      const size_t index_key =
        utils::hash::client_internal_field_hash<
          const_string_t("flecsi_internal_adjacency_index").hash(),
          NAMESPACE,
          NAME,
          INDEX_TYPE::value
        >();
      int ispace = INDEX_TYPE::value;
      execution::context_t::instance().register_field(
        type_key, index_key, index_registration_t::register_callback);

      using offset_registration_t =
        field_registration_u<CLIENT_TYPE, flecsi::data::dense,
          utils::offset_t, adjacency_hash, 0, 1, from_index_space>;

      // This field resides in the main entities (BLIS) index space, but
      // is unique to an adjacency, so it is registered using the
      // adjacency hash.
      const size_t offset_key =
        utils::hash::client_internal_field_hash<
          const_string_t("flecsi_internal_adjacency_offset").hash(),
          NAMESPACE,
          NAME,
          INDEX_TYPE::value
        >();

      execution::context_t::instance().register_field(
        type_key, offset_key, offset_registration_t::register_callback);

    } // visit_type

  }; // struct binding_walker_u

  struct index_subspaces_walker_u
      : public flecsi::utils::tuple_walker_u<index_subspaces_walker_u> {

    template<typename TUPLE_ENTRY_TYPE>
    void visit_type() {
      using INDEX_TYPE = typename std::tuple_element<0, TUPLE_ENTRY_TYPE>::type;
      using INDEX_SUBSPACE_TYPE =
        typename std::tuple_element<1, TUPLE_ENTRY_TYPE>::type;

      constexpr size_t index_subspace_hash =
        utils::hash::client_index_subspace_hash<NAMESPACE, NAME,
          INDEX_TYPE::value, INDEX_SUBSPACE_TYPE::value>();

      using registration_t =
        field_registration_u<CLIENT_TYPE, flecsi::data::subspace,
          utils::id_t, index_subspace_hash, 0, 1, INDEX_TYPE::value>;

      const size_t type_key =
        typeid(CLIENT_TYPE).hash_code();

      using flecsi::utils::const_string_t;
      const size_t field_key =
        utils::hash::client_internal_field_hash<
          const_string_t("flecsi_internal_index_subspace_index").hash(),
          NAMESPACE,
          NAME,
          INDEX_SUBSPACE_TYPE::value
        >();

      execution::context_t::instance().register_field(
        type_key, field_key, registration_t::register_callback);

    } // visit_type

  }; // struct index_subspaces_walker_u

  /*!

   */

  static void register_fields(field_id_t fid) {
    using entity_types_t = typename POLICY_TYPE::entity_types;
    using connectivities = typename POLICY_TYPE::connectivities;
    using bindings = typename POLICY_TYPE::bindings;

    const size_t type_hash =
      typeid(CLIENT_TYPE).hash_code();
    const size_t instance_hash =
      utils::hash::client_hash<NAMESPACE, NAME>();

    // Only register field attributes if this is the first time
    // that we have seen this type.
    if(!execution::context_t::instance().client_fields_registered(type_hash, instance_hash)) {
      entity_walker_t entity_walker;
      entity_walker.template walk_types<entity_types_t>();

      connectivity_walker_u connectivity_walker;
      connectivity_walker.template walk_types<connectivities>();

      binding_walker_u binding_walker;
      binding_walker.template walk_types<bindings>();

      using index_subspaces =
        typename topology::get_index_subspaces_u<POLICY_TYPE>::type;

      index_subspaces_walker_u index_subspaces_walker;
      index_subspaces_walker.template walk_types<index_subspaces>();
    } // if

  } // register_fields

}; // class topology_registration_u

/*!

 */

template<typename POLICY_TYPE, size_t NAMESPACE, size_t NAME>
struct topology_registration_u<
  flecsi::topology::set_topology_u<POLICY_TYPE>,
  NAMESPACE,
  NAME> {
  using CLIENT_TYPE = typename flecsi::topology::set_topology_u<POLICY_TYPE>;

  /*!

   */

  struct entity_walker_t
      : public flecsi::utils::tuple_walker_u<entity_walker_t> {

    template<typename T, T V>
    T value(utils::typeify_u<T, V>) {
      return V;
    }

    template<typename TUPLE_ENTRY_TYPE>
    void visit_type() {
      using INDEX_TYPE = typename std::tuple_element<0, TUPLE_ENTRY_TYPE>::type;
      using ENTITY_TYPE =
        typename std::tuple_element<1, TUPLE_ENTRY_TYPE>::type;

      constexpr size_t entity_hash = utils::hash::client_entity_hash<
        NAMESPACE, NAME, INDEX_TYPE::value, 0, 0>();

      using registration_t = field_registration_u<
        CLIENT_TYPE, flecsi::data::local, ENTITY_TYPE, entity_hash, 0, 1,
        INDEX_TYPE::value>;

      const size_t type_key =
        typeid(CLIENT_TYPE).hash_code();

      using flecsi::utils::const_string_t;
      const size_t field_key = utils::hash::client_internal_field_hash<
          const_string_t("flecsi_internal_entity_data").hash(),
          NAMESPACE, NAME, INDEX_TYPE::value
        >();

      execution::context_t::instance().register_field(
        type_key, field_key, registration_t::register_callback);

      const size_t active_key = utils::hash::client_internal_field_hash<
          const_string_t("flecsi_internal_active_entity_data").hash(),
          NAMESPACE, NAME, INDEX_TYPE::value
        >();

      execution::context_t::instance().register_field(
        type_key, active_key, registration_t::register_callback);

      const size_t migrate_key = utils::hash::client_internal_field_hash<
          const_string_t("flecsi_internal_migrate_entity_data").hash(),
          NAMESPACE, NAME, INDEX_TYPE::value
        >();

      execution::context_t::instance().register_field(
        type_key, migrate_key, registration_t::register_callback);

    } // visit_type

  }; // struct binding_walker_u

  /*!

   */

  static void register_fields(field_id_t fid) {
    using entity_types_t = typename POLICY_TYPE::entity_types;
    const size_t type_key =
      typeid(CLIENT_TYPE).hash_code();

    entity_walker_t entity_walker;
    entity_walker.template walk_types<entity_types_t>();
  } // register_fields

}; // class topology_registration_u
#endif

//----------------------------------------------------------------------------//
// Global.
//----------------------------------------------------------------------------//

/*!
  The global topology doesn't need to register an additional meta data, so this
  type is essentially empty.
 */

template<size_t NAMESPACE, size_t NAME>
struct topology_registration_u<flecsi::topology::global_topology_t,
  NAMESPACE,
  NAME> {

  using TOPOLOGY_TYPE = flecsi::topology::global_topology_t;

  static void register_fields() {} // register_fields

}; // class topology_registration_u

//----------------------------------------------------------------------------//
// Color.
//----------------------------------------------------------------------------//

/*!

 */

template<size_t NAMESPACE, size_t NAME>
struct topology_registration_u<flecsi::topology::index_topology_t,
  NAMESPACE,
  NAME> {

  using TOPOLOGY_TYPE = flecsi::topology::index_topology_t;

  static void register_fields() {} // register_fields

}; // class topology_registration_u

} // namespace data
} // namespace flecsi
