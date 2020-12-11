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

#include <flecsi/execution/context.h>
#include <flecsi/topology/common/entity_storage.h>
#include <flecsi/topology/index_space.h>
#include <flecsi/topology/set_types.h>
#include <flecsi/topology/set_utils.h>
#include <flecsi/topology/types.h>

namespace flecsi {
namespace topology {

template<typename SET_TYPE>
struct mpi_set_topology_storage_policy_u {

  using id_t = utils::id_t;

  using entity_types_t = typename SET_TYPE::entity_types;

  static const size_t num_index_spaces = std::tuple_size<entity_types_t>::value;

  using index_spaces_t = std::array<
    index_space_u<set_entity_t, set_entity_t::id_t, identity_storage_u, entity_storage_t>,
    num_index_spaces>;

  index_spaces_t index_spaces;

  size_t color;

  using index_space_map_t = std::map<size_t, size_t>;

  index_space_map_t index_space_map;

  ~mpi_set_topology_storage_policy_u() {}

  mpi_set_topology_storage_policy_u() {

    auto & context_ = flecsi::execution::context_t::instance();
    color = context_.color();

    map_set_index_spaces_u<std::tuple_size<entity_types_t>::value,
      entity_types_t, index_space_map_t>::map(index_space_map);
  }

  void init_entities(size_t index_space,
    size_t active_migrate_index_space,
    set_entity_t * entities,
    size_t num_entities,
    set_entity_t * active_entities,
    size_t num_active_entities,
    set_entity_t * migrate_entities,
    size_t num_migrate_entities,
    size_t size,
    bool read) {

    auto itr = index_space_map.find(index_space);
    clog_assert(itr != index_space_map.end(), "invalid index space");
    auto & is = index_spaces[itr->second];
    auto s = is.storage();

    *s = {{entities, num_entities}, read ? num_entities : 0};

    itr = index_space_map.find(active_migrate_index_space);
    clog_assert(
      itr != index_space_map.end(), "invalid active migrate index space");
    auto & amis = index_spaces[itr->second];
    auto s2 = amis.storage();

    // how to handle migration buffer?
    s2->set_buffer(active_entities, num_entities, read);

    if(!read) {
      return;
    }

    is.set_end(num_entities);
  }

  void finalize_storage() {
    auto & context = execution::context_t::instance();

    /*
    auto& im = context.local_index_space_data_map();
    for(auto& itr : im){
      size_t index_space = itr.first;

      auto sitr = index_space_map.find(index_space);
      clog_assert(sitr != index_space_map.end(), "invalid index space");
      auto& is = index_spaces[sitr->second];
      execution::context_t::local_index_space_data_t& isd = itr.second;
      isd.size = is.size();
    }
    */
  }

  template<class T, class... ARG_TYPES>
  T * make(ARG_TYPES &&... args) {
    constexpr size_t index_space =
      find_set_index_space_u<num_index_spaces, entity_types_t, T>::find();

    auto & is = index_spaces[index_space];
    auto placement_ptr = static_cast<T *>(is.data.buffer()) + is.ids.size();
    auto ent = new(placement_ptr) T(std::forward<ARG_TYPES>(args)...);
    return ent;
  }
};

} // namespace topology
} // namespace flecsi
