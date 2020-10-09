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

#include <cinchlog.h>
#include <string>
#include <tuple>

#include <flecsi/data/common/data_types.h>
#include <flecsi/data/common/row_vector.h>
#include <flecsi/data/common/serdez.h>
#include <flecsi/data/data_constants.h>
#include <flecsi/data/storage.h>
#include <flecsi/execution/context.h>
#include <flecsi/execution/internal_index_space.h>
#include <flecsi/runtime/types.h>
#include <flecsi/topology/color_topology.h>
#include <flecsi/topology/global_topology.h>
#include <flecsi/topology/mesh_topology.h>
#include <flecsi/topology/set_topology.h>
#include <flecsi/utils/common.h>
#include <flecsi/utils/hash.h>

#include <flecsi/utils/const_string.h>
#include <flecsi/utils/demangle.h>
#include <flecsi/utils/tuple_walker.h>

clog_register_tag(registration);

namespace flecsi {
namespace data {

/*!

*/
template<typename DATA_CLIENT_TYPE,
  size_t STORAGE_CLASS,
  typename DATA_TYPE,
  size_t NAMESPACE_HASH,
  size_t NAME_HASH,
  size_t VERSIONS,
  size_t INDEX_SPACE>
struct field_registration_wrapper_u {

  //--------------------------------------------------------------------------//
  //!
  //--------------------------------------------------------------------------//

  static void register_callback(size_t key, field_id_t fid) {
    execution::context_t::field_info_t fi;

    fi.data_client_hash =
      typeid(typename DATA_CLIENT_TYPE::type_identifier_t).hash_code();

    fi.storage_class = STORAGE_CLASS;
    fi.size = (STORAGE_CLASS == sparse ? sizeof(DATA_TYPE) + sizeof(size_t)
                                       : sizeof(DATA_TYPE));
    fi.namespace_hash = NAMESPACE_HASH;
    fi.name_hash = NAME_HASH;
    fi.versions = VERSIONS;
    fi.index_space = INDEX_SPACE;
    fi.fid = fid;
    fi.key = key;

    execution::context_t::instance().register_field_info(fi);

    // register custom serdez op, if applicable
    if constexpr(STORAGE_CLASS == ragged) {
      using serdez_t = serdez_u<row_vector_u<DATA_TYPE>>;
      execution::context_t::instance().register_serdez<serdez_t>(
        static_cast<int32_t>(fid));
    } // if
    else if constexpr(STORAGE_CLASS == sparse) {
      using serdez_t = serdez_u<row_vector_u<sparse_entry_value_u<DATA_TYPE>>>;
      execution::context_t::instance().register_serdez<serdez_t>(
        static_cast<int32_t>(fid));
    }
  } // register_callback

}; // class field_registration_wrapper_u

//----------------------------------------------------------------------------//
//!
//----------------------------------------------------------------------------//

template<typename DATA_CLIENT_TYPE, size_t NAMESPACE_HASH, size_t NAME_HASH>
struct client_registration_wrapper_u {}; // class client_registration_wrapper_u

//----------------------------------------------------------------------------//
//!
//----------------------------------------------------------------------------//

template<typename POLICY_TYPE, size_t NAMESPACE_HASH, size_t NAME_HASH>
struct client_registration_wrapper_u<
  flecsi::topology::mesh_topology_u<POLICY_TYPE>,
  NAMESPACE_HASH,
  NAME_HASH> {
  using CLIENT_TYPE = typename flecsi::topology::mesh_topology_u<POLICY_TYPE>;

  //--------------------------------------------------------------------------//
  //!
  //--------------------------------------------------------------------------//

  struct entity_walker_t
    : public flecsi::utils::tuple_walker_u<entity_walker_t> {

    template<typename T, T V>
    T value(topology::typeify<T, V>) {
      return V;
    }

    template<typename TUPLE_ENTRY_TYPE>
    void handle_type() {
      using INDEX_TYPE = typename std::tuple_element<0, TUPLE_ENTRY_TYPE>::type;
      using DOMAIN_TYPE =
        typename std::tuple_element<1, TUPLE_ENTRY_TYPE>::type;
      using ENTITY_TYPE =
        typename std::tuple_element<2, TUPLE_ENTRY_TYPE>::type;

      constexpr size_t entity_hash =
        utils::hash::client_entity_hash<NAMESPACE_HASH, NAME_HASH,
          INDEX_TYPE::value, DOMAIN_TYPE::value, ENTITY_TYPE::dimension>();

      using wrapper_t = field_registration_wrapper_u<CLIENT_TYPE,
        flecsi::data::dense, ENTITY_TYPE, entity_hash, 0, 1, INDEX_TYPE::value>;

      const size_t type_key =
        typeid(typename CLIENT_TYPE::type_identifier_t).hash_code();

      using flecsi::utils::const_string_t;
      const size_t field_key = utils::hash::client_internal_field_hash<
        const_string_t("flecsi_internal_entity_data").hash(), NAMESPACE_HASH,
        NAME_HASH, INDEX_TYPE::value>();

      {
        clog_tag_guard(registration);
        clog(info) << "registering field for type id: "
                   << flecsi::utils::demangle(
                        typeid(typename CLIENT_TYPE::type_identifier_t).name())
                   << std::endl
                   << " index: " << INDEX_TYPE::value << std::endl
                   << " namespace: " << NAMESPACE_HASH << std::endl
                   << " name: " << NAME_HASH << std::endl;
        clog(info) << "new key: "
                   << utils::hash::client_internal_field_hash<
                        const_string_t("flecsi_internal_entity_data").hash(),
                        NAMESPACE_HASH, NAME_HASH, INDEX_TYPE::value>()
                   << std::endl;
      } // scope

      storage_t::instance().register_field(
        type_key, field_key, wrapper_t::register_callback);

      using id_wrapper_t = field_registration_wrapper_u<CLIENT_TYPE,
        flecsi::data::dense, utils::id_t, entity_hash, 0, 1, INDEX_TYPE::value>;

      const size_t id_key = utils::hash::client_internal_field_hash<
        const_string_t("flecsi_internal_entity_id").hash(), NAMESPACE_HASH,
        NAME_HASH, INDEX_TYPE::value>();

      storage_t::instance().register_field(
        type_key, id_key, id_wrapper_t::register_callback);

    } // handle_type

  }; // struct entity_walker_t

  struct connectivity_walker_u
    : public flecsi::utils::tuple_walker_u<connectivity_walker_u> {

    template<typename T, T V>
    T value(topology::typeify<T, V>) {
      return V;
    }

    template<typename TUPLE_ENTRY_TYPE>
    void handle_type() {
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
        utils::hash::client_adjacency_hash<NAMESPACE_HASH, NAME_HASH,
          INDEX_TYPE::value,
          DOMAIN_TYPE::value, // from and to domains are the same
          DOMAIN_TYPE::value, // for connectivity information
          FROM_ENTITY_TYPE::dimension, TO_ENTITY_TYPE::dimension>();

      using index_wrapper_t =
        field_registration_wrapper_u<CLIENT_TYPE, flecsi::data::dense,
          utils::id_t, adjacency_hash, 0, 1, INDEX_TYPE::value>;

      const size_t type_key =
        typeid(typename CLIENT_TYPE::type_identifier_t).hash_code();

      using flecsi::utils::const_string_t;
      const size_t index_key = utils::hash::client_internal_field_hash<
        const_string_t("flecsi_internal_adjacency_index").hash(),
        NAMESPACE_HASH, NAME_HASH, INDEX_TYPE::value>();
      int ispace = INDEX_TYPE::value;
      storage_t::instance().register_field(
        type_key, index_key, index_wrapper_t::register_callback);

      using offset_wrapper_t =
        field_registration_wrapper_u<CLIENT_TYPE, flecsi::data::dense,
          utils::offset_t, adjacency_hash, 0, 1, from_index_space>;

      // This field resides in the main entities (BLIS) index space, but
      // is unique to an adjacency, so it is registered using the
      // adjacency hash.
      const size_t offset_key = utils::hash::client_internal_field_hash<
        const_string_t("flecsi_internal_adjacency_offset").hash(),
        NAMESPACE_HASH, NAME_HASH, INDEX_TYPE::value>();

      storage_t::instance().register_field(
        type_key, offset_key, offset_wrapper_t::register_callback);
    } // handle_type

  }; // struct connectivity_walker_u

  struct binding_walker_u
    : public flecsi::utils::tuple_walker_u<binding_walker_u> {

    template<typename TUPLE_ENTRY_TYPE>
    void handle_type() {
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
        utils::hash::client_adjacency_hash<NAMESPACE_HASH, NAME_HASH,
          INDEX_TYPE::value, FROM_DOMAIN_TYPE::value, TO_DOMAIN_TYPE::value,
          FROM_ENTITY_TYPE::dimension, TO_ENTITY_TYPE::dimension>();

      using index_wrapper_t =
        field_registration_wrapper_u<CLIENT_TYPE, flecsi::data::dense,
          utils::id_t, adjacency_hash, 0, 1, INDEX_TYPE::value>;

      const size_t type_key =
        typeid(typename CLIENT_TYPE::type_identifier_t).hash_code();

      using flecsi::utils::const_string_t;
      const size_t index_key = utils::hash::client_internal_field_hash<
        const_string_t("flecsi_internal_adjacency_index").hash(),
        NAMESPACE_HASH, NAME_HASH, INDEX_TYPE::value>();
      int ispace = INDEX_TYPE::value;
      storage_t::instance().register_field(
        type_key, index_key, index_wrapper_t::register_callback);

      using offset_wrapper_t =
        field_registration_wrapper_u<CLIENT_TYPE, flecsi::data::dense,
          utils::offset_t, adjacency_hash, 0, 1, from_index_space>;

      // This field resides in the main entities (BLIS) index space, but
      // is unique to an adjacency, so it is registered using the
      // adjacency hash.
      const size_t offset_key = utils::hash::client_internal_field_hash<
        const_string_t("flecsi_internal_adjacency_offset").hash(),
        NAMESPACE_HASH, NAME_HASH, INDEX_TYPE::value>();

      storage_t::instance().register_field(
        type_key, offset_key, offset_wrapper_t::register_callback);

    } // handle_type

  }; // struct binding_walker_u

  struct index_subspaces_walker_u
    : public flecsi::utils::tuple_walker_u<index_subspaces_walker_u> {

    template<typename TUPLE_ENTRY_TYPE>
    void handle_type() {
      using INDEX_TYPE = typename std::tuple_element<0, TUPLE_ENTRY_TYPE>::type;
      using INDEX_SUBSPACE_TYPE =
        typename std::tuple_element<1, TUPLE_ENTRY_TYPE>::type;

      constexpr size_t index_subspace_hash =
        utils::hash::client_index_subspace_hash<NAMESPACE_HASH, NAME_HASH,
          INDEX_TYPE::value, INDEX_SUBSPACE_TYPE::value>();

      using wrapper_t =
        field_registration_wrapper_u<CLIENT_TYPE, flecsi::data::subspace,
          utils::id_t, index_subspace_hash, 0, 1, INDEX_TYPE::value>;

      const size_t type_key =
        typeid(typename CLIENT_TYPE::type_identifier_t).hash_code();

      using flecsi::utils::const_string_t;
      const size_t field_key = utils::hash::client_internal_field_hash<
        const_string_t("flecsi_internal_index_subspace_index").hash(),
        NAMESPACE_HASH, NAME_HASH, INDEX_SUBSPACE_TYPE::value>();

      storage_t::instance().register_field(
        type_key, field_key, wrapper_t::register_callback);

    } // handle_type

  }; // struct index_subspaces_walker_u

  //--------------------------------------------------------------------------//
  //!
  //--------------------------------------------------------------------------//

  static void register_callback(field_id_t fid) {
    using entity_types_t = typename POLICY_TYPE::entity_types;
    using connectivities = typename POLICY_TYPE::connectivities;
    using bindings = typename POLICY_TYPE::bindings;

    auto & storage = storage_t::instance();

    const size_t type_hash =
      typeid(typename CLIENT_TYPE::type_identifier_t).hash_code();
    const size_t instance_hash =
      utils::hash::client_hash<NAMESPACE_HASH, NAME_HASH>();
    auto const & field_registry = storage.field_registry();

    // Only register field attributes if this is the first time
    // that we have seen this type.
    if(storage.register_client_fields(type_hash, instance_hash)) {
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

  } // register_callback

}; // class client_registration_wrapper_u

//----------------------------------------------------------------------------//
//!
//----------------------------------------------------------------------------//

template<typename POLICY_TYPE, size_t NAMESPACE_HASH, size_t NAME_HASH>
struct client_registration_wrapper_u<
  flecsi::topology::set_topology_u<POLICY_TYPE>,
  NAMESPACE_HASH,
  NAME_HASH> {
  using CLIENT_TYPE = typename flecsi::topology::set_topology_u<POLICY_TYPE>;

  //--------------------------------------------------------------------------//
  //!
  //--------------------------------------------------------------------------//

  struct entity_walker_t
    : public flecsi::utils::tuple_walker_u<entity_walker_t> {

    template<typename T, T V>
    T value(topology::typeify<T, V>) {
      return V;
    }

    template<typename TUPLE_ENTRY_TYPE>
    void handle_type() {
      using INDEX_TYPE = typename std::tuple_element<0, TUPLE_ENTRY_TYPE>::type;
      using ENTITY_TYPE =
        typename std::tuple_element<1, TUPLE_ENTRY_TYPE>::type;

      constexpr size_t entity_hash =
        utils::hash::client_entity_hash<NAMESPACE_HASH, NAME_HASH,
          INDEX_TYPE::value, 0, 0>();

      using wrapper_t = field_registration_wrapper_u<CLIENT_TYPE,
        flecsi::data::local, ENTITY_TYPE, entity_hash, 0, 1, INDEX_TYPE::value>;

      const size_t type_key =
        typeid(typename CLIENT_TYPE::type_identifier_t).hash_code();

      using flecsi::utils::const_string_t;
      const size_t field_key = utils::hash::client_internal_field_hash<
        const_string_t("flecsi_internal_entity_data").hash(), NAMESPACE_HASH,
        NAME_HASH, INDEX_TYPE::value>();

      storage_t::instance().register_field(
        type_key, field_key, wrapper_t::register_callback);

      const size_t active_key = utils::hash::client_internal_field_hash<
        const_string_t("flecsi_internal_active_entity_data").hash(),
        NAMESPACE_HASH, NAME_HASH, INDEX_TYPE::value>();

      storage_t::instance().register_field(
        type_key, active_key, wrapper_t::register_callback);

      const size_t migrate_key = utils::hash::client_internal_field_hash<
        const_string_t("flecsi_internal_migrate_entity_data").hash(),
        NAMESPACE_HASH, NAME_HASH, INDEX_TYPE::value>();

      storage_t::instance().register_field(
        type_key, migrate_key, wrapper_t::register_callback);

    } // handle_type

  }; // struct binding_walker_u

  //--------------------------------------------------------------------------//
  //!
  //--------------------------------------------------------------------------//

  static void register_callback(field_id_t fid) {
    using entity_types_t = typename POLICY_TYPE::entity_types;

    auto & storage = storage_t::instance();

    const size_t type_key =
      typeid(typename CLIENT_TYPE::type_identifier_t).hash_code();
    auto const & field_registry = storage.field_registry();

    entity_walker_t entity_walker;
    entity_walker.template walk_types<entity_types_t>();
  } // register_callback

}; // class client_registration_wrapper_u

//----------------------------------------------------------------------------//
//!
//----------------------------------------------------------------------------//

template<size_t NAMESPACE_HASH, size_t NAME_HASH>
struct client_registration_wrapper_u<flecsi::topology::global_topology_u,
  NAMESPACE_HASH,
  NAME_HASH> {

  using CLIENT_TYPE = flecsi::topology::global_topology_u;

  static void register_callback(field_id_t fid) {}

}; // class client_registration_wrapper_u

//----------------------------------------------------------------------------//
//!
//----------------------------------------------------------------------------//

template<size_t NAMESPACE_HASH, size_t NAME_HASH>
struct client_registration_wrapper_u<flecsi::topology::color_topology_u,
  NAMESPACE_HASH,
  NAME_HASH> {

  using CLIENT_TYPE = flecsi::topology::color_topology_u;

  static void register_callback(field_id_t fid) {}

}; // class client_registration_wrapper_u

} // namespace data
} // namespace flecsi
