/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_topology_hpx_topology_storage_policy_h
#define flecsi_topology_hpx_topology_storage_policy_h

#include "flecsi/topology/mesh_storage.h"

#include <array>
#include <cassert>
#include <iostream>
#include <unordered_map>
#include <vector>

#include <flecsi/data/data_client.h>
#include <flecsi/execution/context.h>
#include <flecsi/topology/common/entity_storage.h>
#include <flecsi/topology/index_space.h>
#include <flecsi/topology/mesh_storage.h>
#include <flecsi/topology/mesh_types.h>
#include <flecsi/topology/mesh_utils.h>

///
/// \file
/// \date Initial file creation: Apr 04, 2017
///

namespace flecsi {
namespace topology {

class mesh_entity_base_;

template<size_t, size_t>
class mesh_entity_t;

///
/// \class hpx_data_handle_policy_t data_handle_policy.h
/// \brief hpx_data_handle_policy_t provides...
///

template<size_t NUM_DIMS, size_t NUM_DOMS, size_t NUM_INDEX_SUBSPACES>
struct hpx_topology_storage_policy_u {
  static constexpr size_t num_partitions = 5;
  using id_t = utils::id_t;

  using index_spaces_t = std::array<
      index_space_u<
          mesh_entity_base_ *,
          true,
          true,
          true,
          void,
          topology_storage_u>,
      NUM_DIMS + 1>;

  using index_subspaces_t = std::array<
      index_space_u<
          mesh_entity_base_ *,
          false,
          true,
          false,
          void,
          topology_storage_u>,
      NUM_INDEX_SUBSPACES>;

  using partition_index_spaces_t = std::array<
      index_space_u<
          mesh_entity_base_ *,
          false,
          false,
          true,
          void,
          topology_storage_u>,
      NUM_DIMS + 1>;

  // array of array of domain_connectivity_u
  std::array<std::array<domain_connectivity_u<NUM_DIMS>, NUM_DOMS>,
    NUM_DOMS> topology;

  std::array<index_spaces_t, NUM_DOMS> index_spaces;

  std::array<std::array<partition_index_spaces_t, NUM_DOMS>, num_partitions>
      partition_index_spaces;

  size_t color;

  hpx_topology_storage_policy_u() {
    auto & context_ = flecsi::execution::context_t::instance();
    color = context_.color();
  }

  template<size_t D, size_t N>
  size_t entity_dimension(mesh_entity_t<D, N> *) {
    return D;
  }

  template<class T, size_t DOM, class... ARG_TYPES>
  T * make(ARG_TYPES &&... args) {
    using dtype = domain_entity_u<DOM, T>;

    auto & is = index_spaces[DOM][T::dimension].template cast<dtype>();
    size_t entity = is.size();

    auto placement_ptr = static_cast<T *>(is.storage()->buffer()) + entity;
    auto ent = new (placement_ptr) T(std::forward<ARG_TYPES>(args)...);

    id_t global_id = id_t::make<T::dimension, DOM>(entity, color);
    ent->template set_global_id<DOM>(global_id);

    auto & id_storage = is.id_storage();

    id_storage[entity] = global_id;

    is.pushed();

    return ent;
  } // make

  template<class T, size_t DOM, class... ARG_TYPES>
  T * make(const id_t & id, ARG_TYPES &&... args) {
    using dtype = domain_entity_u<DOM, T>;

    auto & is = index_spaces[DOM][T::dimension].template cast<dtype>();

    size_t entity = id.entity();

    auto placement_ptr = static_cast<T *>(is.storage()->buffer()) + entity;
    auto ent = new (placement_ptr) T(std::forward<ARG_TYPES>(args)...);

    ent->template set_global_id<DOM>(id);

    auto & id_storage = is.id_storage();

    id_storage[entity] = id;

    is.pushed();

    return ent;
  } // make

}; // class hpx_topology_storage_policy_t

} // namespace topology
} // namespace flecsi

#endif // flecsi_topology_hpx_topology_storage_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
