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

#include "flecsi/data/common/data_types.h"
#include "flecsi/data/common/registration_wrapper.h"
#include "flecsi/data/storage.h"
#include "flecsi/data/data_client_handle.h"
#include "flecsi/execution/context.h"
#include "flecsi/utils/tuple_walker.h"
#include "flecsi/topology/mesh_types.h"
#include "flecsi/runtime/types.h"

namespace flecsi {
namespace topology {

//----------------------------------------------------------------------------//
// Forward declaration
//----------------------------------------------------------------------------//

template<typename>
class mesh_topology_t;

template<typename>
class set_topology_t;

//----------------------------------------------------------------------------//
// Forward declaration
//----------------------------------------------------------------------------//

template<size_t, size_t>
class mesh_entity_t;

} // namespace topology

namespace data {

//----------------------------------------------------------------------------//
//! FIXME: Description of class
//----------------------------------------------------------------------------//

template<typename DATA_CLIENT>
struct data_client_policy_handler__{};

//----------------------------------------------------------------------------//
//! FIXME: Description of class
//----------------------------------------------------------------------------//

template<>
struct data_client_policy_handler__<global_data_client_t>
{
  
  template<
    typename DATA_CLIENT_TYPE,
    size_t NAMESPACE_HASH,
    size_t NAME_HASH
  >
  static
  data_client_handle__<DATA_CLIENT_TYPE, 0>
  get_client_handle()
  {
    data_client_handle__<DATA_CLIENT_TYPE, 0> h;

    h.client_hash = 
      typeid(typename DATA_CLIENT_TYPE::type_identifier_t).hash_code();
    h.namespace_hash = NAMESPACE_HASH;
    h.name_hash = NAME_HASH;

    return h;
  } // get_client_handle

}; // struct data_client_policy_handler__

//----------------------------------------------------------------------------//
//! FIXME: Description of class
//----------------------------------------------------------------------------//

template<typename POLICY_TYPE>
struct data_client_policy_handler__<topology::mesh_topology_t<POLICY_TYPE>>
{

  struct entity_info_t
  {
    size_t index_space;
    size_t dim;
    size_t domain;
    size_t size;
  }; // struct entity_info_t

  struct adjacency_info_t
  {
    size_t index_space;
    size_t from_index_space;
    size_t to_index_space;
    size_t from_domain;
    size_t to_domain;
    size_t from_dim;
    size_t to_dim;
  }; // struct adjacency_info_t

  struct entity_walker_t :
    public flecsi::utils::tuple_walker__<entity_walker_t>
  {

    template<
      typename TUPLE_ENTRY_TYPE
    >
    void
    handle_type()
    {
      using INDEX_TYPE =
        typename std::tuple_element<0, TUPLE_ENTRY_TYPE>::type;
      using DOMAIN_TYPE =
        typename std::tuple_element<1, TUPLE_ENTRY_TYPE>::type;
      using ENTITY_TYPE =
        typename std::tuple_element<2, TUPLE_ENTRY_TYPE>::type;

      entity_info_t ei;

      ei.index_space = INDEX_TYPE::value;
      ei.dim = ENTITY_TYPE::dimension;
      ei.domain = DOMAIN_TYPE::value;
      ei.size = sizeof(ENTITY_TYPE);

      //entity_info.emplace_back(std::move(ei));
      entity_info.push_back(ei);
      entity_index_space_map.emplace(typeid(ENTITY_TYPE).hash_code(),
        INDEX_TYPE::value);
    } // handle_type

    std::vector<entity_info_t> entity_info;
    std::map<size_t, size_t> entity_index_space_map;

  }; // struct entity_walker_t

  template<typename MESH_TYPE>
  struct connectivity_walker_t :
    public flecsi::utils::tuple_walker__<connectivity_walker_t<MESH_TYPE>>
  {
    using entity_types_t = typename MESH_TYPE::entity_types;

    template<
      typename TUPLE_ENTRY_TYPE
    >
    void
    handle_type()
    {
      using INDEX_TYPE =
        typename std::tuple_element<0, TUPLE_ENTRY_TYPE>::type;
      using DOMAIN_TYPE =
        typename std::tuple_element<1, TUPLE_ENTRY_TYPE>::type;
      using FROM_ENTITY_TYPE =
        typename std::tuple_element<2, TUPLE_ENTRY_TYPE>::type;
      using TO_ENTITY_TYPE =
        typename std::tuple_element<3, TUPLE_ENTRY_TYPE>::type;

      adjacency_info_t hi;
      
      hi.index_space = INDEX_TYPE::value;
      
      hi.from_index_space = 
        topology::find_index_space__<std::tuple_size<entity_types_t>::value,
        entity_types_t, FROM_ENTITY_TYPE>::find();
      
      hi.to_index_space = 
        topology::find_index_space__<std::tuple_size<entity_types_t>::value,
        entity_types_t, TO_ENTITY_TYPE>::find();

      hi.from_domain = DOMAIN_TYPE::value;

      hi.to_domain = DOMAIN_TYPE::value;

      hi.from_dim = FROM_ENTITY_TYPE::dimension;

      hi.to_dim = TO_ENTITY_TYPE::dimension;

      adjacency_info.emplace_back(std::move(hi));
    } // handle_type

    std::vector<adjacency_info_t> adjacency_info;

  }; // struct connectivity_walker_t

  template<typename MESH_TYPE>
  struct binding_walker_t :
    public flecsi::utils::tuple_walker__<binding_walker_t<MESH_TYPE>>
  {
    using entity_types_t = typename MESH_TYPE::entity_types;

    template<
      typename TUPLE_ENTRY_TYPE
    >
    void
    handle_type()
    {
      using INDEX_TYPE =
        typename std::tuple_element<0, TUPLE_ENTRY_TYPE>::type;
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
        topology::find_index_space__<std::tuple_size<entity_types_t>::value,
        entity_types_t, FROM_ENTITY_TYPE>::find();
      
      hi.to_index_space = 
        topology::find_index_space__<std::tuple_size<entity_types_t>::value,
        entity_types_t, TO_ENTITY_TYPE>::find();

      hi.from_domain = FROM_DOMAIN_TYPE::value;

      hi.to_domain = TO_DOMAIN_TYPE::value;

      hi.from_dim = FROM_ENTITY_TYPE::dimension;

      hi.to_dim = TO_ENTITY_TYPE::dimension;

      adjacency_info.emplace_back(std::move(hi));
    } // handle_type

    std::vector<adjacency_info_t> adjacency_info;
  }; // struct binding_walker_t

  template<
    typename DATA_CLIENT_TYPE,
    size_t NAMESPACE_HASH,
    size_t NAME_HASH
  >
  static
  data_client_handle__<DATA_CLIENT_TYPE, 0>
  get_client_handle()
  {
    using entity_types = typename POLICY_TYPE::entity_types;
    using connectivities = typename POLICY_TYPE::connectivities;
    using bindings = typename POLICY_TYPE::bindings;

    data_client_handle__<DATA_CLIENT_TYPE, 0> h;

    auto& context = execution::context_t::instance();

    auto& ism = context.index_space_data_map();

    h.client_hash = 
      typeid(typename DATA_CLIENT_TYPE::type_identifier_t).hash_code();
    h.name_hash = NAME_HASH;
    h.namespace_hash = NAMESPACE_HASH;

    entity_walker_t entity_walker;
    entity_walker.template walk_types<entity_types>();

    h.num_handle_entities = entity_walker.entity_info.size();

    size_t entity_index(0);
    for(auto & ei: entity_walker.entity_info) {
      data_client_handle_entity_t & ent = h.handle_entities[entity_index];
      ent.index_space = ei.index_space;
      ent.domain = ei.domain;
      ent.dim = ei.dim;
      ent.size = ei.size;

      auto itr = context.field_info_map().find(
        { h.client_hash, ent.index_space });

      clog_assert(itr != context.field_info_map().end(),
        "invalid entity index space");

      auto & tm = itr->second;

      for(auto & fitr : tm){
        if(fitr.second.key == 
          utils::hash::client_internal_field_hash(
          utils::const_string_t("__flecsi_internal_entity_data__").
          hash(), ent.index_space)) {
            ent.fid = fitr.second.fid;
        } // if
        else if(fitr.second.key == 
          utils::hash::client_internal_field_hash(
          utils::const_string_t("__flecsi_internal_entity_id__").
          hash(), ent.index_space)) {
            ent.id_fid = fitr.second.fid;
        }
      } // for
#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_legion
      auto ritr = ism.find(ent.index_space);
      clog_assert(ritr != ism.end(), "invalid index space " << ei.index_space);
      
      ent.color_region = ritr->second.color_region;
      ent.exclusive_region = ritr->second.exclusive_lr;
      ent.shared_region = ritr->second.shared_lr;
      ent.ghost_region = ritr->second.ghost_lr;
#endif

      ++entity_index;
    } // for

    connectivity_walker_t<POLICY_TYPE> connectivity_walker;
    connectivity_walker.template walk_types<connectivities>();

    binding_walker_t<POLICY_TYPE> binding_walker;
    binding_walker.adjacency_info = 
      std::move(connectivity_walker.adjacency_info);
    binding_walker.template walk_types<bindings>();

    size_t handle_index = 0;

    clog_assert(binding_walker.adjacency_info.size() <= h.MAX_ADJACENCIES,
      "handle max adjacencies exceeded");

    h.num_handle_adjacencies = binding_walker.adjacency_info.size();

    for(adjacency_info_t& hi : binding_walker.adjacency_info){
      data_client_handle_adjacency_t & adj = 
        h.handle_adjacencies[handle_index];

      adj.adj_index_space = hi.index_space;
      adj.from_index_space = hi.from_index_space;
      adj.to_index_space = hi.to_index_space;
      adj.from_domain = hi.from_domain;
      adj.to_domain = hi.to_domain;
      adj.from_dim = hi.from_dim;
      adj.to_dim = hi.to_dim;

      auto itr = context.field_info_map().find(
        {h.client_hash, hi.from_index_space});
      clog_assert(itr != context.field_info_map().end(),
        "invalid from index space");

      auto& fm = itr->second;

      // This field resides in the main entities (BLIS) index space, but
      // is unique to an adjacency, so it is registered using the
      // adjacency hash.      
      for(auto& fitr : fm){
        if(fitr.second.key == 
           utils::hash::client_internal_field_hash(
           utils::const_string_t("__flecsi_internal_adjacency_offset__").
           hash(), hi.index_space)){
          adj.offset_fid = fitr.second.fid;
          break;
        }
      }

      itr = context.field_info_map().find(
        {h.client_hash, hi.index_space});
      clog_assert(itr != context.field_info_map().end(),
        "invalid index space");

      auto& im = itr->second;

      for(auto& fitr : im){
        if(fitr.second.key == 
          utils::hash::client_internal_field_hash(
          utils::const_string_t("__flecsi_internal_adjacency_index__").
          hash(), hi.index_space)){
            adj.index_fid = fitr.second.fid;
            break;
        }
      }
#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_legion
      auto ritr = ism.find(hi.from_index_space);
      clog_assert(ritr != ism.end(), "invalid from index space");
      adj.from_color_region = ritr->second.color_region;
      adj.from_primary_region = ritr->second.primary_lr;

      ritr = ism.find(hi.index_space);
      clog_assert(ritr != ism.end(), "invalid index space");
      adj.adj_region = ritr->second.color_region;
#endif
      ++handle_index;
    }

    return h;
  } // get_client_handle

}; // struct data_client_policy_handler__

//----------------------------------------------------------------------------//
//! FIXME: Description of class
//----------------------------------------------------------------------------//

template<typename POLICY_TYPE>
struct data_client_policy_handler__<topology::set_topology_t<POLICY_TYPE>>
{

  struct entity_info_t
  {
    size_t index_space;
    size_t size;
  }; // struct entity_info_t

  struct entity_walker_t :
    public flecsi::utils::tuple_walker__<entity_walker_t>
  {

    template<
      typename TUPLE_ENTRY_TYPE
    >
    void
    handle_type()
    {
      using INDEX_TYPE =
        typename std::tuple_element<0, TUPLE_ENTRY_TYPE>::type;
      using ENTITY_TYPE =
        typename std::tuple_element<1, TUPLE_ENTRY_TYPE>::type;

      entity_info_t ei;

      ei.index_space = INDEX_TYPE::value;
      ei.size = sizeof(ENTITY_TYPE);

      //entity_info.emplace_back(std::move(ei));
      entity_info.push_back(ei);
      entity_index_space_map.emplace(typeid(ENTITY_TYPE).hash_code(),
        INDEX_TYPE::value);
    } // handle_type

    std::vector<entity_info_t> entity_info;
    std::map<size_t, size_t> entity_index_space_map;

  }; // struct entity_walker_t

  template<
    typename DATA_CLIENT_TYPE,
    size_t NAMESPACE_HASH,
    size_t NAME_HASH
  >
  static
  data_client_handle__<DATA_CLIENT_TYPE, 0>
  get_client_handle()
  {
    using entity_types = typename POLICY_TYPE::entity_types;

    data_client_handle__<DATA_CLIENT_TYPE, 0> h;

    auto& context = execution::context_t::instance();

    auto& ism = context.index_space_data_map();

    h.client_hash = 
      typeid(typename DATA_CLIENT_TYPE::type_identifier_t).hash_code();
    h.name_hash = NAME_HASH;
    h.namespace_hash = NAMESPACE_HASH;

    entity_walker_t entity_walker;
    entity_walker.template walk_types<entity_types>();

    h.num_handle_entities = entity_walker.entity_info.size();

    size_t entity_index(0);
    for(auto & ei: entity_walker.entity_info) {
      data_client_handle_entity_t & ent = h.handle_entities[entity_index];
      ent.index_space = ei.index_space;
      ent.size = ei.size;

      auto itr = context.field_info_map().find(
        { h.client_hash, ent.index_space });

      clog_assert(itr != context.field_info_map().end(),
        "invalid entity index space");

      auto & tm = itr->second;

      for(auto & fitr : tm){
        if(fitr.second.key == 
          utils::hash::client_internal_field_hash(
          utils::const_string_t("__flecsi_internal_entity_data__").
          hash(), ent.index_space)) {
            ent.fid = fitr.second.fid;
        } // if
        else if(fitr.second.key == 
          utils::hash::client_internal_field_hash(
          utils::const_string_t("__flecsi_internal_entity_id__").
          hash(), ent.index_space)) {
            ent.id_fid = fitr.second.fid;
        }
      } // for
#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_legion
      auto ritr = ism.find(ent.index_space);
      clog_assert(ritr != ism.end(), "invalid index space " << ei.index_space);
      
      ent.color_region = ritr->second.color_region;
      ent.exclusive_region = ritr->second.exclusive_lr;
      ent.shared_region = ritr->second.shared_lr;
      ent.ghost_region = ritr->second.ghost_lr;
#endif

      ++entity_index;
    } // for

    return h;
  } // get_client_handle

}; // struct data_client_policy_handler__

//----------------------------------------------------------------------------//
//! The data_client_interface__ type defines a high-level data client
//! interface that is implemented by the given data policy.
//!
//! @tparam DATA_POLICY The backend runtime policy.
//!
//! @ingroup data
//----------------------------------------------------------------------------//

template<
  typename DATA_POLICY
>
struct data_client_interface__
{
  //--------------------------------------------------------------------------//
  //! Register a data client with the FleCSI runtime.
  //!
  //! @tparam DATA_CLIENT_TYPE The data client type.
  //! @tparam NAMESPACE_HASH   The namespace key. Namespaces allow separation
  //!                          of attribute names to avoid collisions.
  //! @tparam NAME_HASH        The attribute name.
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
    static_assert(sizeof(DATA_CLIENT_TYPE) ==
      sizeof(typename DATA_CLIENT_TYPE::type_identifier_t),
      "Data clients may not add data members");

    using wrapper_t = client_registration_wrapper__<
      typename DATA_CLIENT_TYPE::type_identifier_t,
      NAMESPACE_HASH,
      NAME_HASH
    >;

    const size_t client_key = 
      typeid(typename DATA_CLIENT_TYPE::type_identifier_t).hash_code();
    //! \todo move to hash.h
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
  data_client_handle__<DATA_CLIENT_TYPE, 0>
  get_client_handle()
  {
    using data_client_policy_handler_t = 
      data_client_policy_handler__<
        typename DATA_CLIENT_TYPE::type_identifier_t
      >;

    return data_client_policy_handler_t::template 
      get_client_handle<DATA_CLIENT_TYPE, NAMESPACE_HASH, NAME_HASH>();
  } // get_client_handle

}; // struct data_client_interface__

} // namespace data
} // namespace flecsi

//----------------------------------------------------------------------------//
// This include file defines the FLECSI_RUNTIME_DATA_POLICY used below.
//----------------------------------------------------------------------------//

#include "flecsi/runtime/flecsi_runtime_data_policy.h"

namespace flecsi {
namespace data {

using data_client_interface_t =
  data_client_interface__<FLECSI_RUNTIME_DATA_POLICY>;

} // namespace data
} // namespace flecsi

#endif // flecsi_data_client_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
