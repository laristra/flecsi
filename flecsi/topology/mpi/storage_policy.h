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

namespace flecsi {
namespace topology {

///
/// \class mpi_data_handle_policy_t data_handle_policy.h
/// \brief mpi_data_handle_policy_t provides...
///

template<size_t NUM_DIMS, size_t NUM_DOMS, size_t NUM_INDEX_SUBSPACES>
struct mpi_topology_storage_policy_u {
  static constexpr size_t num_partitions = 5;
  using id_t = utils::id_t;

  using storage_t =
    index_space_u<mesh_entity_base_, mesh_entity_base_::id_t, topology_storage_u, entity_storage_t>;
  
  using id_storage_t =
    index_space_u<mesh_entity_base_, utils::indices_t, topology_storage_u, entity_storage_t>;
  
  using index_spaces_t = std::array<storage_t, NUM_DIMS + 1>;

  using index_subspaces_t = std::array<id_storage_t, NUM_INDEX_SUBSPACES>;

  using partition_index_spaces_t =
    std::array<index_space_u<mesh_entity_base_, mesh_entity_base_::id_t, utils::span, entity_storage_t>,
      NUM_DIMS + 1>;

  // array of array of domain_connectivity_u
  std::array<std::array<domain_connectivity_u<NUM_DIMS>, NUM_DOMS>, NUM_DOMS>
    topology;

  std::array<index_spaces_t, NUM_DOMS> index_spaces;

  index_subspaces_t index_subspaces;

  std::array<std::array<partition_index_spaces_t, NUM_DOMS>, num_partitions>
    partition_index_spaces;

  size_t color;

  mpi_topology_storage_policy_u() {
    auto & context_ = flecsi::execution::context_t::instance();
    color = context_.color();
  }

  void init_entities(
    size_t domain,
    size_t dim,
    mesh_entity_base_ * entities,
    utils::id_t * ids,
    size_t size,
    size_t num_entities,
    size_t num_exclusive,
    size_t num_shared,
    size_t num_ghost,
    bool read) {
    auto & is = index_spaces[domain][dim];

    const auto s = &is.data;
    s->set_buffer(entities, num_entities, read);

    auto & id_storage = is.ids;
    id_storage = full_array(ids, num_entities, read);

    for(auto & domain_connectivities : topology) {
      auto & domain_connectivity_u = domain_connectivities[domain];
      for(size_t d = 0; d <= NUM_DIMS; ++d) {
        domain_connectivity_u.get(d, dim).set_entity_storage(*s);
      } // for
    } // for

    if(!read) {
      return;
    }

    size_t shared_end = num_exclusive + num_shared;
    size_t ghost_end = shared_end + num_ghost;

    for(size_t partition = 0; partition < num_partitions; ++partition) {
      auto & isp = partition_index_spaces[partition][domain][dim];
      isp.data = *s;

      switch(partition_t(partition)) {
        case exclusive:
          isp.ids = {ids, num_exclusive};
          break;
        case shared:
          isp.ids = {ids + num_exclusive, ids + shared_end};
          break;
        case ghost:
          isp.ids = {ids + shared_end, ids + ghost_end};
          break;
        case owned:
          isp.ids = {ids, shared_end};
          break;
        default:
          isp.ids = {ids, ghost_end};
          break;
      }
    }
  } // init_entities

  void init_index_subspace(size_t index_space,
    size_t index_subspace,
    size_t domain,
    size_t dim,
    utils::indices_t * ids,
    size_t num_entities,
    bool read) {

    auto & context_ = execution::context_t::instance();
    auto & ssm = context_.index_subspace_info();
    auto itr = ssm.find(index_subspace);
    clog_assert(itr != ssm.end(), "invalid index subspace");
    const execution::context_t::index_subspace_info_t & si = itr->second;

    auto & is = index_spaces[domain][dim];
    auto & iss = index_subspaces[index_subspace];

    iss.data = is.data;

    auto & id_storage = iss.ids;
    id_storage = {{ids, si.capacity}, si.size};

    if(!read) {
      return;
    }
  } // init_index_subspaces

  void init_connectivity(
    size_t from_domain,
    size_t to_domain,
    size_t from_dim,
    size_t to_dim,
    utils::offset_t * offsets,
    size_t num_offsets,
    utils::indices_t * indices,
    size_t num_indices,
    bool read) {
    // TODO - this is an initial implementation for testing purposes.
    // We may wish to store the buffer pointers coming from Legion directly
    // into the connectivity
    auto & conn = topology[from_domain][to_domain].get(from_dim, to_dim);

    auto & is = conn.get_index_space();
    is.ids = full_array(indices, num_indices, read);

    conn.offsets().storage() = full_array(offsets, num_offsets+1, read);
  } // init_connectivities

  template<class T, size_t DOM, class... ARG_TYPES>
  T * make(ARG_TYPES &&... args) {
    auto & is = index_spaces[DOM][T::dimension];
    const std::size_t i = is.ids.size();
    const id_t global = id_t::make<T::dimension, DOM>(i, color);
    is.ids.push_back(global);
    return make2<T, DOM>(i, global, std::forward<ARG_TYPES>(args)...);
  } // make

  template<class T, size_t DOM, class... ARG_TYPES>
  T * make(const id_t & id, ARG_TYPES &&... args) {
    auto & is = index_spaces[DOM][T::dimension];

    size_t entity = id.entity();
    is.ids[entity] = id;
    std::cout << "make2" << std::endl;
    return make2<T, DOM>(entity, id, std::forward<ARG_TYPES>(args)...);
  } // make

private:
  template<class T>
  static utils::vector_ref<T>
  full_array(T * p, std::size_t n, bool full = true) {
    return {{p, n}, full ? n : 0};
  }

  template<class T, size_t DOM, class... ARG_TYPES>
  T * make2(std::size_t index, const id_t & global, ARG_TYPES &&... args) {
    const auto ret = new(
      static_cast<T *>(index_spaces[DOM][T::dimension].data.buffer()) + index)
      T(std::forward<ARG_TYPES>(args)...);
    ret->set_global_id(global);
    return ret;
  }
}; // class mpi_topology_storage_policy_u

} // namespace topology
} // namespace flecsi
