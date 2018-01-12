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

#include <flecsi/data/data_constants.h>
#include <flecsi/data/storage.h>
#include <flecsi/execution/context.h>
#include <flecsi/execution/internal_index_space.h>
#include <flecsi/runtime/types.h>
#include <flecsi/topology/mesh_topology.h>
#include <flecsi/topology/set_topology.h>
#include <flecsi/utils/common.h>
#include <flecsi/utils/hash.h>
#include <flecsi/utils/tuple_walker.h>

// clog_register_tag(registration);

namespace flecsi {
namespace data {

//----------------------------------------------------------------------------//
//!
//----------------------------------------------------------------------------//

template<
    typename DATA_CLIENT_TYPE,
    size_t STORAGE_CLASS,
    typename DATA_TYPE,
    size_t NAMESPACE_HASH,
    size_t NAME_HASH,
    size_t VERSIONS,
    size_t INDEX_SPACE>
struct field_registration_wrapper__ {
  //--------------------------------------------------------------------------//
  //!
  //--------------------------------------------------------------------------//

  static void register_callback(size_t key, field_id_t fid) {
    execution::context_t::field_info_t fi;

    fi.data_client_hash =
        typeid(typename DATA_CLIENT_TYPE::type_identifier_t).hash_code();

    fi.storage_class = STORAGE_CLASS;
    fi.size = sizeof(DATA_TYPE);
    fi.namespace_hash = NAMESPACE_HASH;
    fi.name_hash = NAME_HASH;
    fi.versions = VERSIONS;

    // This seems like it could be improved to get rid of the
    // conditional logic
    if (STORAGE_CLASS == global)
      fi.index_space = execution::internal_index_space::global_is;
    else if (STORAGE_CLASS == color)
      fi.index_space = execution::internal_index_space::color_is;
    else
      fi.index_space = INDEX_SPACE;

    fi.fid = fid;
    fi.key = key;

    execution::context_t::instance().register_field_info(fi);
  } // register_callback

}; // class field_registration_wrapper__

//----------------------------------------------------------------------------//
//!
//----------------------------------------------------------------------------//

template<typename DATA_CLIENT_TYPE, size_t NAMESPACE_HASH, size_t NAME_HASH>
struct client_registration_wrapper__ {}; // class client_registration_wrapper__

//----------------------------------------------------------------------------//
//!
//----------------------------------------------------------------------------//

template<typename POLICY_TYPE, size_t NAMESPACE_HASH, size_t NAME_HASH>
struct client_registration_wrapper__<
    flecsi::topology::mesh_topology__<POLICY_TYPE>,
    NAMESPACE_HASH,
    NAME_HASH> {
  using CLIENT_TYPE = typename flecsi::topology::mesh_topology__<POLICY_TYPE>;

  //--------------------------------------------------------------------------//
  //!
  //--------------------------------------------------------------------------//

  struct entity_walker_t
      : public flecsi::utils::tuple_walker__<entity_walker_t> {

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

      constexpr size_t entity_hash = utils::hash::client_entity_hash<
          NAMESPACE_HASH, NAME_HASH, INDEX_TYPE::value, DOMAIN_TYPE::value,
          ENTITY_TYPE::dimension>();

      using wrapper_t = field_registration_wrapper__<
          CLIENT_TYPE, flecsi::data::dense, ENTITY_TYPE, entity_hash, 0, 1,
          INDEX_TYPE::value>;

      const size_t client_key =
          typeid(typename CLIENT_TYPE::type_identifier_t).hash_code();

      const size_t key = utils::hash::client_internal_field_hash<
          utils::const_string_t("__flecsi_internal_entity_data__").hash(),
          INDEX_TYPE::value>();

      storage_t::instance().register_field(
          client_key, key, wrapper_t::register_callback);

      using id_wrapper_t = field_registration_wrapper__<
          CLIENT_TYPE, flecsi::data::dense, utils::id_t, entity_hash, 0, 1,
          INDEX_TYPE::value>;

      const size_t id_key = utils::hash::client_internal_field_hash<
          utils::const_string_t("__flecsi_internal_entity_id__").hash(),
          INDEX_TYPE::value>();

      storage_t::instance().register_field(
          client_key, id_key, id_wrapper_t::register_callback);

    } // handle_type

  }; // struct entity_walker_t

  struct connectivity_walker__
      : public flecsi::utils::tuple_walker__<connectivity_walker__> {

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

      constexpr size_t from_index_space = topology::find_index_space__<
          std::tuple_size<entity_types_t>::value, entity_types_t,
          FROM_ENTITY_TYPE>::find();

      constexpr size_t to_index_space = topology::find_index_space__<
          std::tuple_size<entity_types_t>::value, entity_types_t,
          TO_ENTITY_TYPE>::find();

      constexpr size_t adjacency_hash = utils::hash::client_adjacency_hash<
          NAMESPACE_HASH, NAME_HASH, INDEX_TYPE::value,
          DOMAIN_TYPE::value, // from and to domains are the same
          DOMAIN_TYPE::value, // for connectivity information
          FROM_ENTITY_TYPE::dimension, TO_ENTITY_TYPE::dimension>();

      using index_wrapper_t = field_registration_wrapper__<
          CLIENT_TYPE, flecsi::data::dense, utils::id_t, adjacency_hash, 0, 1,
          INDEX_TYPE::value>;

      const size_t client_key =
          typeid(typename CLIENT_TYPE::type_identifier_t).hash_code();

      const size_t index_key = utils::hash::client_internal_field_hash<
          utils::const_string_t("__flecsi_internal_adjacency_index__").hash(),
          INDEX_TYPE::value>();
      int ispace = INDEX_TYPE::value;
      storage_t::instance().register_field(
          client_key, index_key, index_wrapper_t::register_callback);

      using offset_wrapper_t = field_registration_wrapper__<
          CLIENT_TYPE, flecsi::data::dense, utils::offset_t, adjacency_hash, 0,
          1, from_index_space>;

      // This field resides in the main entities (BLIS) index space, but
      // is unique to an adjacency, so it is registered using the
      // adjacency hash.
      const size_t offset_key = utils::hash::client_internal_field_hash<
          utils::const_string_t("__flecsi_internal_adjacency_offset__").hash(),
          INDEX_TYPE::value>();

      storage_t::instance().register_field(
          client_key, offset_key, offset_wrapper_t::register_callback);
    } // handle_type

  }; // struct connectivity_walker__

  struct binding_walker__
      : public flecsi::utils::tuple_walker__<binding_walker__> {

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

      constexpr size_t adjacency_hash = utils::hash::client_adjacency_hash<
          NAMESPACE_HASH, NAME_HASH, INDEX_TYPE::value, FROM_DOMAIN_TYPE::value,
          TO_DOMAIN_TYPE::value, FROM_ENTITY_TYPE::dimension,
          TO_ENTITY_TYPE::dimension>();

      using wrapper_t = field_registration_wrapper__<
          CLIENT_TYPE, flecsi::data::dense, size_t, adjacency_hash, 0, 1,
          INDEX_TYPE::value>;

      const size_t client_key =
          typeid(typename CLIENT_TYPE::type_identifier_t).hash_code();
      const size_t key = utils::hash::client_internal_field_hash<
          utils::const_string_t("__flecsi_internal_field_hash_base__").hash(),
          INDEX_TYPE::value>();
      int ispace = INDEX_TYPE::value;
      storage_t::instance().register_field(
          client_key, key, wrapper_t::register_callback);
    } // handle_type

  }; // struct binding_walker__

  //--------------------------------------------------------------------------//
  //!
  //--------------------------------------------------------------------------//

  static void register_callback(field_id_t fid) {
    using entity_types_t = typename POLICY_TYPE::entity_types;
    using connectivities = typename POLICY_TYPE::connectivities;
    using bindings = typename POLICY_TYPE::bindings;

    auto & storage = storage_t::instance();

    const size_t client_key =
        typeid(typename CLIENT_TYPE::type_identifier_t).hash_code();
    auto const & field_registry = storage.field_registry();

    // Only register field attributes if this is the first time
    // that we have seen this type.
    if (storage.register_client_fields(client_key)) {
      entity_walker_t entity_walker;
      entity_walker.template walk_types<entity_types_t>();

      connectivity_walker__ connectivity_walker;
      connectivity_walker.template walk_types<connectivities>();

      binding_walker__ binding_walker;
      binding_walker.template walk_types<bindings>();
    } // if

  } // register_callback

}; // class client_registration_wrapper__

//----------------------------------------------------------------------------//
//!
//----------------------------------------------------------------------------//

template<typename POLICY_TYPE, size_t NAMESPACE_HASH, size_t NAME_HASH>
struct client_registration_wrapper__<
    flecsi::topology::set_topology__<POLICY_TYPE>,
    NAMESPACE_HASH,
    NAME_HASH> {
  using CLIENT_TYPE = typename flecsi::topology::set_topology__<POLICY_TYPE>;

  //--------------------------------------------------------------------------//
  //!
  //--------------------------------------------------------------------------//

  struct entity_walker_t
      : public flecsi::utils::tuple_walker__<entity_walker_t> {

    template<typename T, T V>
    T value(topology::typeify<T, V>) {
      return V;
    }

    template<typename TUPLE_ENTRY_TYPE>
    void handle_type() {
      using INDEX_TYPE = typename std::tuple_element<0, TUPLE_ENTRY_TYPE>::type;
      using ENTITY_TYPE =
          typename std::tuple_element<1, TUPLE_ENTRY_TYPE>::type;

      constexpr size_t entity_hash = utils::hash::client_entity_hash<
          NAMESPACE_HASH, NAME_HASH, INDEX_TYPE::value, 0, 0>();

      using wrapper_t = field_registration_wrapper__<
          CLIENT_TYPE, flecsi::data::local, ENTITY_TYPE, entity_hash, 0, 1,
          INDEX_TYPE::value>;

      const size_t client_key =
          typeid(typename CLIENT_TYPE::type_identifier_t).hash_code();

      const size_t key = utils::hash::client_internal_field_hash<
          utils::const_string_t("__flecsi_internal_entity_data__").hash(),
          INDEX_TYPE::value>();

      storage_t::instance().register_field(
          client_key, key, wrapper_t::register_callback);

    } // handle_type

  }; // struct binding_walker__

  //--------------------------------------------------------------------------//
  //!
  //--------------------------------------------------------------------------//

  static void register_callback(field_id_t fid) {
    using entity_types_t = typename POLICY_TYPE::entity_types;

    auto & storage = storage_t::instance();

    const size_t client_key =
        typeid(typename CLIENT_TYPE::type_identifier_t).hash_code();
    auto const & field_registry = storage.field_registry();

    entity_walker_t entity_walker;
    entity_walker.template walk_types<entity_types_t>();
  } // register_callback

}; // class client_registration_wrapper__

} // namespace data
} // namespace flecsi
