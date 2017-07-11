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
#include "flecsi/utils/tuple_walker.h"
#include "flecsi/topology/mesh_types.h"

namespace flecsi {

namespace topology {
  template<typename>
  class mesh_topology_t;

  template<size_t, size_t>
  class mesh_entity_t;
}

namespace data {

//----------------------------------------------------------------------------//
//! FIXME: Description of class
//----------------------------------------------------------------------------//

template<typename DATA_CLIENT>
struct data_client_policy_handler__{};

template<typename POLICY_TYPE>
struct data_client_policy_handler__<topology::mesh_topology_t<POLICY_TYPE>>{
  
  using field_id_t = Legion::FieldID;

  struct handle_info_t
  {
    size_t index_space;
    size_t from_index_space;
    size_t to_index_space;
  };

  struct entity_walker_t :
    public flecsi::utils::tuple_walker__<entity_walker_t>
  {

    template<
      size_t D,
      size_t N
    >
    size_t
    entity_dimension(topology::mesh_entity_t<D, N>*)
    {
      return D;
    }

    template<typename T, T V>
    T value(topology::typeify<T, V>){
      return V;
    }

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

      entity_index_space_map.emplace(typeid(ENTITY_TYPE).hash_code(),
        value(INDEX_TYPE()));
    } // handle_type

    std::map<size_t, size_t> entity_index_space_map;

  }; // struct entity_walker_t

  struct connectivity_walker_t :
    public flecsi::utils::tuple_walker__<connectivity_walker_t>
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
      using FROM_ENTITY_TYPE =
        typename std::tuple_element<2, TUPLE_ENTRY_TYPE>::type;
      using TO_ENTITY_TYPE =
        typename std::tuple_element<3, TUPLE_ENTRY_TYPE>::type;

      handle_info_t hi;
      
      hi.index_space = INDEX_TYPE::value;
      
      hi.from_index_space = 
        entity_index_space_map[typeid(FROM_ENTITY_TYPE).hash_code()];
      
      hi.to_index_space = 
        entity_index_space_map[typeid(TO_ENTITY_TYPE).hash_code()];

      handles.emplace_back(std::move(hi));
    } // handle_type

    std::vector<handle_info_t> handles;
    std::map<size_t, size_t> entity_index_space_map;

  }; // struct connectivity_walker_t

  struct binding_walker_t :
    public flecsi::utils::tuple_walker__<binding_walker_t>
  {

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
    } // handle_type

    std::vector<handle_info_t> handles;
    std::map<size_t, size_t> entity_index_space_map;

  }; // struct binding_walker_t

  template<
    typename DATA_CLIENT_TYPE,
    size_t NAMESPACE_HASH,
    size_t NAME_HASH
  >
  static
  data_client_handle__<DATA_CLIENT_TYPE>
  get_client_handle()
  {
    using entity_types_t = typename POLICY_TYPE::entity_types;
    using connectivities = typename POLICY_TYPE::connectivities;
    using bindings = typename POLICY_TYPE::bindings;

    entity_walker_t entity_walker;
    entity_walker.template walk_types<entity_types_t>();

    connectivity_walker_t connectivity_walker;
    connectivity_walker.entity_index_space_map = 
      std::move(entity_walker.entity_index_space_map);
    connectivity_walker.template walk_types<connectivities>();

    binding_walker_t binding_walker;
    binding_walker.entity_index_space_map = 
      std::move(connectivity_walker.entity_index_space_map);
    binding_walker.handles = 
      std::move(connectivity_walker.handles);
    binding_walker.template walk_types<bindings>();

    data_client_handle__<DATA_CLIENT_TYPE> h;

    auto& context = execution::context_t::instance();

    auto& ism = context.index_space_data_map();

    const size_t data_client_hash = typeid(DATA_CLIENT_TYPE).hash_code();

    size_t handle_index = 0;

    clog_assert(binding_walker.handles.size() <= h.MAX_ADJACENCIES,
                "handle max adjacencies exceeded");

    h.num_adjacencies = binding_walker.handles.size();

    for(handle_info_t& hi : binding_walker.handles){
      h.adj_index_spaces[handle_index] = hi.index_space;
      h.from_index_spaces[handle_index] = hi.from_index_space;
      h.to_index_spaces[handle_index] = hi.to_index_space;

      auto itr = context.field_info_map().find(
        {data_client_hash, hi.from_index_space});
      clog_assert(itr != context.field_info_map().end(),
        "invalid from index space");

      auto& fm = itr->second;

      for(auto& fitr : fm){
        if(fitr.second.key == 
           utils::hash::client_internal_field_hash(
           utils::const_string_t("__flecsi_internal_adjacency_offset__").
           hash(), hi.from_index_space)){
          h.offset_fids[handle_index] = fitr.second.fid;
          break;
        }
      }

      itr = context.field_info_map().find(
        {data_client_hash, hi.to_index_space});
      clog_assert(itr != context.field_info_map().end(),
        "invalid to index space");

      auto& tm = itr->second;

      for(auto& fitr : tm){
        if(fitr.second.key == 
           utils::hash::client_internal_field_hash(
           utils::const_string_t("__flecsi_internal_entity_data__").
           hash(), hi.to_index_space)){
          h.entity_fids[handle_index] = fitr.second.fid;
          break;
        }
      }

      itr = context.field_info_map().find(
        {data_client_hash, hi.index_space});
      clog_assert(itr != context.field_info_map().end(),
        "invalid index space");

      auto& im = itr->second;

      for(auto& fitr : im){
        if(fitr.second.key == 
           utils::hash::client_internal_field_hash(
           utils::const_string_t("__flecsi_internal_adjacency_index__").
           hash(), hi.index_space)){
          h.index_fids[handle_index] = fitr.second.fid;
          break;
        }
      }

      h.from_color_regions[handle_index] = 
        ism[hi.from_index_space].color_region;
      h.to_color_regions[handle_index] = 
        ism[hi.to_index_space].color_region;

      h.from_primary_regions[handle_index] = 
        ism[hi.from_index_space].primary_lr;
      h.to_primary_regions[handle_index] = 
        ism[hi.to_index_space].primary_lr;
      
      h.adj_regions[handle_index] = ism[hi.index_space].color_region;

      ++handle_index;
    }

    return h;
  }
};


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
    return data_client_policy_handler__<DATA_CLIENT_TYPE>::template 
      get_client_handle<DATA_CLIENT_TYPE, NAMESPACE_HASH, NAME_HASH>();
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
