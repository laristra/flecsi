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
struct legion_set_topology_storage_policy__ {
  using id_t = utils::id_t;

  using entity_types_t = typename SET_TYPE::entity_types;

  static const size_t num_index_spaces = std::tuple_size<entity_types_t>::value;

  using index_spaces_t = std::array<
      index_space__<
          set_entity_t *,
          true,
          true,
          true,
          void,
          identity_storage__,
          topology_storage__>,
      num_index_spaces>;

  index_spaces_t index_spaces;

  size_t color;

  using index_space_map_t = std::map<size_t, size_t>;

  index_space_map_t index_space_map;

  ~legion_set_topology_storage_policy__() {}

  legion_set_topology_storage_policy__() {

    auto & context_ = flecsi::execution::context_t::instance();
    color = context_.color();

    map_set_index_spaces__<
        std::tuple_size<entity_types_t>::value, entity_types_t,
        index_space_map_t>::map(index_space_map);
  }

  void init_entities(
      size_t index_space,
      set_entity_t * entities,
      size_t size,
      size_t num_entities,
      bool read) {
    auto itr = index_space_map.find(index_space);
    clog_assert(itr != index_space_map.end(), "invalid index space");
    auto & is = index_spaces[itr->second];
    auto s = is.storage();

    s->set_buffer(entities, num_entities, read);

    if (!read) {
      return;
    }

    is.set_end(num_entities);
  }

  template<class T, class... S>
  T * make(S &&... args) {
    constexpr size_t index_space =
        find_set_index_space__<num_index_spaces, entity_types_t, T>::find();

    auto & is = index_spaces[index_space].template cast<T *>();
    size_t entity = is.size();

    auto placement_ptr = static_cast<T *>(is.storage()->buffer()) + entity;
    auto ent = new (placement_ptr) T(std::forward<S>(args)...);
    auto storage = is.storage();
    storage->pushed();
    is.pushed();

    return ent;
  }
};

} // namespace topology
} // namespace flecsi
