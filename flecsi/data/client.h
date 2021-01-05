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

#include <flecsi/data/common/data_types.h>
#include <flecsi/data/common/registration_wrapper.h>
#include <flecsi/data/data_client_handle.h>
#include <flecsi/data/internal_client.h>
#include <flecsi/data/storage.h>
#include <flecsi/execution/context.h>
#include <flecsi/runtime/types.h>
#include <flecsi/topology/color_topology.h>
#include <flecsi/topology/global_topology.h>
#include <flecsi/topology/mesh_types.h>
#include <flecsi/topology/mesh_utils.h>

#include <flecsi/utils/const_string.h>
#include <flecsi/utils/tuple_walker.h>

namespace flecsi {
namespace topology {

//----------------------------------------------------------------------------//
// Forward declarations
//----------------------------------------------------------------------------//

template<typename>
class mesh_topology_u;

template<typename>
class set_topology_u;

template<size_t, size_t>
class mesh_entity_u;

} // namespace topology

namespace data {

/*!
  The data client policy handler is responsible for extracting compile-time
  information from various data client types such as index spaces and entity
  types defined on mesh topology or set topology and provides methods such as
  those for obtaining a client handle and populating required fields on the
  data client handle. This class is specialized on a specific data client
  type such as mesh, set, or tree topology.
 */

template<typename DATA_CLIENT>
struct data_client_policy_handler_u {};

/*!
  The data client policy handler for global data client. Populate the
  required fields on the client handle.
 */

template<>
struct data_client_policy_handler_u<topology::global_topology_u> {

  template<typename DATA_CLIENT_TYPE, size_t NAMESPACE_HASH, size_t NAME_HASH>
  static data_client_handle<DATA_CLIENT_TYPE, 0> get_client_handle() {
    data_client_handle<DATA_CLIENT_TYPE, 0> h;

    h.type_hash =
      typeid(typename DATA_CLIENT_TYPE::type_identifier_t).hash_code();
    h.namespace_hash = NAMESPACE_HASH;
    h.name_hash = NAME_HASH;

    return h;
  } // get_client_handle

}; // struct data_client_policy_handler_u

/*!
  The data client policy handler for color data client. Populate the
  required fields on the client handle.
 */

template<>
struct data_client_policy_handler_u<topology::color_topology_u> {

  template<typename DATA_CLIENT_TYPE, size_t NAMESPACE_HASH, size_t NAME_HASH>
  static data_client_handle<DATA_CLIENT_TYPE, 0> get_client_handle() {
    data_client_handle<DATA_CLIENT_TYPE, 0> h;

    h.type_hash =
      typeid(typename DATA_CLIENT_TYPE::type_identifier_t).hash_code();
    h.namespace_hash = NAMESPACE_HASH;
    h.name_hash = NAME_HASH;

    return h;
  } // get_client_handle

}; // struct data_client_policy_handler_u

/*!
  The data client policy handler for mesh topology. This class provides
  tuple walkers for extracting information from the entity types, bindings,
  and connectivity tuples and obtaining information about field IDs in order
  to populate fields on the data client handle so that it can be properly
  processed when passed to a task.
 */

template<typename POLICY_TYPE>
struct data_client_policy_handler_u<topology::mesh_topology_u<POLICY_TYPE>> {

  struct entity_info_t {
    size_t index_space;
    size_t dim;
    size_t domain;
    size_t size;
  }; // struct entity_info_t

  struct adjacency_info_t {
    size_t index_space;
    size_t from_index_space;
    size_t to_index_space;
    size_t from_domain;
    size_t to_domain;
    size_t from_dim;
    size_t to_dim;
  }; // struct adjacency_info_t

  struct index_subspace_info_t {
    size_t index_space;
    size_t index_subspace;
    size_t domain;
    size_t dim;
  }; // struct entity_info_t

  struct entity_walker_t
    : public flecsi::utils::tuple_walker_u<entity_walker_t> {

    template<typename TUPLE_ENTRY_TYPE>
    void handle_type() {
      using INDEX_TYPE = typename std::tuple_element<0, TUPLE_ENTRY_TYPE>::type;
      using DOMAIN_TYPE =
        typename std::tuple_element<1, TUPLE_ENTRY_TYPE>::type;
      using ENTITY_TYPE =
        typename std::tuple_element<2, TUPLE_ENTRY_TYPE>::type;

      entity_info_t ei;

      ei.index_space = INDEX_TYPE::value;
      ei.dim = ENTITY_TYPE::dimension;
      ei.domain = DOMAIN_TYPE::value;
      ei.size = sizeof(ENTITY_TYPE);

      // entity_info.emplace_back(std::move(ei));
      entity_info.push_back(ei);
      entity_index_space_map.emplace(
        typeid(ENTITY_TYPE).hash_code(), INDEX_TYPE::value);
    } // handle_type

    std::vector<entity_info_t> entity_info;
    std::map<size_t, size_t> entity_index_space_map;

  }; // struct entity_walker_t

  template<typename MESH_TYPE>
  struct connectivity_walker_u
    : public flecsi::utils::tuple_walker_u<connectivity_walker_u<MESH_TYPE>> {
    using entity_types_t = typename MESH_TYPE::entity_types;

    template<typename TUPLE_ENTRY_TYPE>
    void handle_type() {
      using INDEX_TYPE = typename std::tuple_element<0, TUPLE_ENTRY_TYPE>::type;
      using DOMAIN_TYPE =
        typename std::tuple_element<1, TUPLE_ENTRY_TYPE>::type;
      using FROM_ENTITY_TYPE =
        typename std::tuple_element<2, TUPLE_ENTRY_TYPE>::type;
      using TO_ENTITY_TYPE =
        typename std::tuple_element<3, TUPLE_ENTRY_TYPE>::type;

      adjacency_info_t hi;

      hi.index_space = INDEX_TYPE::value;

      hi.from_index_space =
        topology::find_index_space_u<std::tuple_size<entity_types_t>::value,
          entity_types_t, FROM_ENTITY_TYPE>::find();

      hi.to_index_space =
        topology::find_index_space_u<std::tuple_size<entity_types_t>::value,
          entity_types_t, TO_ENTITY_TYPE>::find();

      hi.from_domain = DOMAIN_TYPE::value;

      hi.to_domain = DOMAIN_TYPE::value;

      hi.from_dim = FROM_ENTITY_TYPE::dimension;

      hi.to_dim = TO_ENTITY_TYPE::dimension;

      adjacency_info.emplace_back(std::move(hi));
    } // handle_type

    std::vector<adjacency_info_t> adjacency_info;

  }; // struct connectivity_walker_u

  template<typename MESH_TYPE>
  struct binding_walker_u
    : public flecsi::utils::tuple_walker_u<binding_walker_u<MESH_TYPE>> {
    using entity_types_t = typename MESH_TYPE::entity_types;

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

      adjacency_info_t hi;

      hi.index_space = INDEX_TYPE::value;

      hi.from_index_space =
        topology::find_index_space_u<std::tuple_size<entity_types_t>::value,
          entity_types_t, FROM_ENTITY_TYPE>::find();

      hi.to_index_space =
        topology::find_index_space_u<std::tuple_size<entity_types_t>::value,
          entity_types_t, TO_ENTITY_TYPE>::find();

      hi.from_domain = FROM_DOMAIN_TYPE::value;

      hi.to_domain = TO_DOMAIN_TYPE::value;

      hi.from_dim = FROM_ENTITY_TYPE::dimension;

      hi.to_dim = TO_ENTITY_TYPE::dimension;

      adjacency_info.emplace_back(std::move(hi));
    } // handle_type

    std::vector<adjacency_info_t> adjacency_info;
  }; // struct binding_walker_u

  template<typename MESH_TYPE>
  struct index_subspace_walker_u
    : public flecsi::utils::tuple_walker_u<index_subspace_walker_u<MESH_TYPE>> {

    using entity_types_t = typename MESH_TYPE::entity_types;

    template<typename TUPLE_ENTRY_TYPE>
    void handle_type() {
      using INDEX_TYPE = typename std::tuple_element<0, TUPLE_ENTRY_TYPE>::type;
      using INDEX_SUBSPACE_TYPE =
        typename std::tuple_element<1, TUPLE_ENTRY_TYPE>::type;

      index_subspace_info_t si;

      si.index_space = INDEX_TYPE::value;
      si.index_subspace = INDEX_SUBSPACE_TYPE::value;

      constexpr size_t index = topology::find_index_space_from_id_u<
        std::tuple_size<entity_types_t>::value, entity_types_t,
        INDEX_TYPE::value>::find();

      using ENT_TUPLE_ENTRY_TYPE =
        typename std::tuple_element<index, entity_types_t>::type;

      using DOMAIN_TYPE =
        typename std::tuple_element<1, ENT_TUPLE_ENTRY_TYPE>::type;

      using ENTITY_TYPE =
        typename std::tuple_element<2, ENT_TUPLE_ENTRY_TYPE>::type;

      si.domain = DOMAIN_TYPE::value;
      si.dim = ENTITY_TYPE::dimension;

      index_subspace_info.push_back(si);
    } // handle_type

    std::vector<index_subspace_info_t> index_subspace_info;

  }; // struct index_subspace_walker_t

  template<typename DATA_CLIENT_TYPE, size_t NAMESPACE_HASH, size_t NAME_HASH>
  static data_client_handle<DATA_CLIENT_TYPE, 0> get_client_handle() {
    using entity_types = typename POLICY_TYPE::entity_types;
    using connectivities = typename POLICY_TYPE::connectivities;
    using bindings = typename POLICY_TYPE::bindings;
    using index_subspaces =
      typename topology::get_index_subspaces_u<POLICY_TYPE>::type;
    using field_info_t = execution::context_t::field_info_t;

    data_client_handle<DATA_CLIENT_TYPE, 0> h;

    auto & context = execution::context_t::instance();

    auto & ism = context.index_space_data_map();

    h.type_hash =
      typeid(typename DATA_CLIENT_TYPE::type_identifier_t).hash_code();
    h.name_hash = NAME_HASH;
    h.namespace_hash = NAMESPACE_HASH;

    const size_t key = utils::hash::client_hash<NAMESPACE_HASH, NAME_HASH>();

    storage_t::instance().assert_client_exists(
      h.type_hash, utils::hash::client_hash<NAMESPACE_HASH, NAME_HASH>());

    entity_walker_t entity_walker;
    entity_walker.template walk_types<entity_types>();

    h.num_handle_entities = entity_walker.entity_info.size();

    using flecsi::utils::const_string_t;
    size_t entity_index(0);
    for(auto & ei : entity_walker.entity_info) {
      data_client_handle_entity_t & ent = h.handle_entities[entity_index];
      ent.index_space = ei.index_space;
      ent.domain = ei.domain;
      ent.dim = ei.dim;
      ent.size = ei.size;

      const field_info_t * fi = context.get_field_info_from_key(
        h.type_hash, utils::hash::client_internal_field_hash(
                       const_string_t("flecsi_internal_entity_data").hash(),
                       NAMESPACE_HASH, NAME_HASH, ent.index_space));

      if(fi) {
        ent.fid = fi->fid;
        ent.fid_size = fi->size;
      }

      fi = context.get_field_info_from_key(
        h.type_hash, utils::hash::client_internal_field_hash(
                       const_string_t("flecsi_internal_entity_id").hash(),
                       NAMESPACE_HASH, NAME_HASH, ent.index_space));

      if(fi) {
        ent.id_fid = fi->fid;
        ent.id_fid_size = fi->size;
      }

#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_legion
      auto ritr = ism.find(ent.index_space);
      clog_assert(ritr != ism.end(), "invalid index space " << ei.index_space);

      ent.entire_region = ritr->second.entire_region;
      ent.color_partition = ritr->second.color_partition;
      ent.primary_partition = ritr->second.primary_lp;
      ent.exclusive_partition = ritr->second.exclusive_lp;
      ent.shared_partition = ritr->second.shared_lp;
      ent.ghost_partition = ritr->second.ghost_lp;
      ent.ghost_owner_partition = ritr->second.ghost_owners_lp;

      auto init = ritr->second.ghost_is_readable.count(ent.fid) == 0;

      ent.ghost_is_readable = &(ritr->second.ghost_is_readable[ent.fid]);
      ent.write_phase_started = &(ritr->second.write_phase_started[ent.fid]);
      ent.ghost_id_updated = &(ritr->second.ghost_id_updated[ent.fid]);

      if(init) {
        *ent.ghost_is_readable = true;
        *ent.write_phase_started = false;
        *ent.ghost_id_updated = false;
      }
#endif
#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_hpx
      auto ritr = ism.find(ent.index_space);
      clog_assert(ritr != ism.end(), "invalid index space " << ei.index_space);

      // make client handle depend on only one of the fields
      if(h.future == nullptr) {
        h.future = &(ritr->second.future[ent.fid]);
      }
#endif

      ++entity_index;
    } // for

    connectivity_walker_u<POLICY_TYPE> connectivity_walker;
    connectivity_walker.template walk_types<connectivities>();

    binding_walker_u<POLICY_TYPE> binding_walker;
    binding_walker.adjacency_info =
      std::move(connectivity_walker.adjacency_info);
    binding_walker.template walk_types<bindings>();

    size_t handle_index = 0;

    clog_assert(binding_walker.adjacency_info.size() <= h.MAX_ADJACENCIES,
      "handle max adjacencies exceeded");

    h.num_handle_adjacencies = binding_walker.adjacency_info.size();

    for(adjacency_info_t & hi : binding_walker.adjacency_info) {
      data_client_handle_adjacency_t & adj = h.handle_adjacencies[handle_index];

      adj.adj_index_space = hi.index_space;
      adj.from_index_space = hi.from_index_space;
      adj.to_index_space = hi.to_index_space;
      adj.from_domain = hi.from_domain;
      adj.to_domain = hi.to_domain;
      adj.from_dim = hi.from_dim;
      adj.to_dim = hi.to_dim;

      const field_info_t * fi = context.get_field_info_from_key(h.type_hash,
        utils::hash::client_internal_field_hash(
          const_string_t("flecsi_internal_adjacency_offset").hash(),
          NAMESPACE_HASH, NAME_HASH, hi.index_space));

      if(fi) {
        adj.offset_fid = fi->fid;
        adj.offset_fid_size = fi->size;
      }

      fi = context.get_field_info_from_key(
        h.type_hash, utils::hash::client_internal_field_hash(
                       const_string_t("flecsi_internal_adjacency_index").hash(),
                       NAMESPACE_HASH, NAME_HASH, hi.index_space));

      if(fi) {
        adj.index_fid = fi->fid;
        adj.index_fid_size = fi->size;
      }

#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_legion
      auto ritr = ism.find(hi.from_index_space);
      clog_assert(ritr != ism.end(), "invalid from index space");
      adj.from_entire_region = ritr->second.entire_region;
      adj.from_color_partition = ritr->second.color_partition;
      adj.from_primary_partition = ritr->second.primary_lp;

      ritr = ism.find(hi.index_space);
      clog_assert(ritr != ism.end(), "invalid index space");
      adj.adj_region = ritr->second.entire_region;
      adj.adj_color_partition = ritr->second.color_partition;
#endif
      ++handle_index;
    }

#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_legion
    h.ghost_is_readable =
      &(ism[h.handle_adjacencies[0].adj_index_space].ghost_is_readable[0]);
    h.write_phase_started =
      &(ism[h.handle_adjacencies[0].adj_index_space].write_phase_started[0]);
#endif

    auto & issm = context.index_subspace_data_map();

    index_subspace_walker_u<POLICY_TYPE> index_subspace_walker;
    index_subspace_walker.template walk_types<index_subspaces>();

    handle_index = 0;

    clog_assert(
      index_subspace_walker.index_subspace_info.size() <= h.MAX_INDEX_SUBSPACES,
      "handle max index subspaces exceeded");

    h.num_index_subspaces = index_subspace_walker.index_subspace_info.size();

    for(index_subspace_info_t & si :
      index_subspace_walker.index_subspace_info) {

      data_client_handle_index_subspace_t & iss =
        h.handle_index_subspaces[handle_index];

      iss.index_space = si.index_space;
      iss.index_subspace = si.index_subspace;
      iss.domain = si.domain;
      iss.dim = si.dim;

      const field_info_t * fi = context.get_field_info_from_key(h.type_hash,
        utils::hash::client_internal_field_hash(
          const_string_t("flecsi_internal_index_subspace_index").hash(),
          NAMESPACE_HASH, NAME_HASH, si.index_subspace));

      if(fi) {
        iss.index_fid = fi->fid;
        iss.index_fid_size = fi->size;
      }

#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_legion
      auto ritr = issm.find(si.index_subspace);
      clog_assert(ritr != issm.end(), "invalid index subspace");
      iss.logical_region = ritr->second.logical_region;
      iss.logical_partition = ritr->second.logical_partition;
#endif

      ++handle_index;
    }

    return h;
  } // get_client_handle

}; // struct data_client_policy_handler_u

/*!
  The data client policy handler for set topology. This class provides
  tuple walkers for extracting information from the entity types
  and obtaining information about field IDs in order
  to populate fields on the data client handle so that it can be properly
  processed when passed to a task.
 */

template<typename POLICY_TYPE>
struct data_client_policy_handler_u<topology::set_topology_u<POLICY_TYPE>> {

  struct entity_info_t {
    size_t index_space;
    size_t active_migrate_index_space;
    size_t size;
  }; // struct entity_info_t

  struct entity_walker_t
    : public flecsi::utils::tuple_walker_u<entity_walker_t> {

    template<typename TUPLE_ENTRY_TYPE>
    void handle_type() {
      using INDEX_TYPE = typename std::tuple_element<0, TUPLE_ENTRY_TYPE>::type;
      using ENTITY_TYPE =
        typename std::tuple_element<1, TUPLE_ENTRY_TYPE>::type;

      entity_info_t ei;

      ei.index_space = INDEX_TYPE::value;
      // TODO these ranges need to be formalized
      ei.active_migrate_index_space = INDEX_TYPE::value + 1024;
      ei.size = sizeof(ENTITY_TYPE);

      // entity_info.emplace_back(std::move(ei));
      entity_info.push_back(ei);
      entity_index_space_map.emplace(
        typeid(ENTITY_TYPE).hash_code(), INDEX_TYPE::value);
    } // handle_type

    std::vector<entity_info_t> entity_info;
    std::map<size_t, size_t> entity_index_space_map;

  }; // struct entity_walker_t

  template<typename DATA_CLIENT_TYPE, size_t NAMESPACE_HASH, size_t NAME_HASH>
  static data_client_handle<DATA_CLIENT_TYPE, 0> get_client_handle() {
    using entity_types = typename POLICY_TYPE::entity_types;
    using field_info_t = execution::context_t::field_info_t;

    data_client_handle<DATA_CLIENT_TYPE, 0> h;

    auto & context = execution::context_t::instance();

    // auto & ism = context.local_index_space_data_map();
    auto & ism = context.index_space_data_map();

    h.type_hash =
      typeid(typename DATA_CLIENT_TYPE::type_identifier_t).hash_code();
    h.name_hash = NAME_HASH;
    h.namespace_hash = NAMESPACE_HASH;

    storage_t::instance().assert_client_exists(
      h.type_hash, utils::hash::client_hash<NAMESPACE_HASH, NAME_HASH>());

    entity_walker_t entity_walker;
    entity_walker.template walk_types<entity_types>();

    h.num_handle_entities = entity_walker.entity_info.size();

    using flecsi::utils::const_string_t;
    size_t entity_index(0);
    for(auto & ei : entity_walker.entity_info) {
      data_client_handle_entity_t & ent = h.handle_entities[entity_index];
      ent.index_space = ei.index_space;
      ent.index_space2 = ei.active_migrate_index_space;
      ent.size = ei.size;

      const field_info_t * fi = context.get_field_info_from_key(
        h.type_hash, utils::hash::client_internal_field_hash(
                       const_string_t("flecsi_internal_entity_data").hash(),
                       NAMESPACE_HASH, NAME_HASH, ent.index_space));

      if(fi) {
        ent.fid = fi->fid;
      }

#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_legion
      auto ritr = ism.find(ent.index_space);
      clog_assert(ritr != ism.end(), "invalid index space " << ei.index_space);

      ent.entire_region = ritr->second.entire_region;
#endif

      fi = context.get_field_info_from_key(h.type_hash,
        utils::hash::client_internal_field_hash(
          const_string_t("flecsi_internal_active_entity_data").hash(),
          NAMESPACE_HASH, NAME_HASH, ent.index_space));

      if(fi) {
        ent.fid2 = fi->fid;
        ent.fid2_size = fi->size;
      }

      fi = context.get_field_info_from_key(h.type_hash,
        utils::hash::client_internal_field_hash(
          const_string_t("flecsi_internal_migrate_entity_data").hash(),
          NAMESPACE_HASH, NAME_HASH, ent.index_space));

      if(fi) {
        ent.fid3 = fi->fid;
        ent.fid3_size = fi->size;
      }

      ++entity_index;
    } // for

    return h;
  } // get_client_handle

}; // struct data_client_policy_handler_u

/*!
  The data_client_interface_u type defines a high-level data client
  interface that is implemented by the given data policy.

  @tparam DATA_POLICY The backend runtime policy.

  @ingroup data
 */

template<typename DATA_POLICY>
struct data_client_interface_u {

  /*!
    Register a data client with the FleCSI runtime.

    @tparam DATA_CLIENT_TYPE The data client type.
    @tparam NAMESPACE_HASH   The namespace key. Namespaces allow separation
                             of attribute names to avoid collisions.
    @tparam NAME_HASH        The attribute name.
   */

  template<typename DATA_CLIENT_TYPE, size_t NAMESPACE_HASH, size_t NAME_HASH>
  static bool register_data_client(std::string const & name) {
    static_assert(sizeof(DATA_CLIENT_TYPE) ==
                    sizeof(typename DATA_CLIENT_TYPE::type_identifier_t),
      "Data clients may not add data members");

    using wrapper_t = client_registration_wrapper_u<
      typename DATA_CLIENT_TYPE::type_identifier_t, NAMESPACE_HASH, NAME_HASH>;

    const size_t type_key =
      typeid(typename DATA_CLIENT_TYPE::type_identifier_t).hash_code();

    const size_t key = utils::hash::client_hash<NAMESPACE_HASH, NAME_HASH>();

    return storage_t::instance().register_client(
      type_key, key, wrapper_t::register_callback);
  } // register_data_client

  /*!
   */

  template<typename DATA_CLIENT_TYPE, size_t NAMESPACE_HASH, size_t NAME_HASH>
  static data_client_handle<DATA_CLIENT_TYPE, 0> get_client_handle() {
    using data_client_policy_handler_t = data_client_policy_handler_u<
      typename DATA_CLIENT_TYPE::type_identifier_t>;

    return data_client_policy_handler_t::template get_client_handle<
      DATA_CLIENT_TYPE, NAMESPACE_HASH, NAME_HASH>();
  } // get_client_handle

}; // struct data_client_interface_u

} // namespace data
} // namespace flecsi

//----------------------------------------------------------------------------//
// This include file defines the FLECSI_RUNTIME_DATA_POLICY used below.
//----------------------------------------------------------------------------//

#include <flecsi/runtime/flecsi_runtime_data_policy.h>

namespace flecsi {
namespace data {

using data_client_interface_t =
  data_client_interface_u<FLECSI_RUNTIME_DATA_POLICY>;

} // namespace data
} // namespace flecsi
