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

#include <flecsi/coloring/box_types.h>
#include <flecsi/data/data_client.h>
#include <flecsi/execution/context.h>
#include <flecsi/topology/mesh_storage.h>
#include <flecsi/topology/mesh_utils.h>
#include <flecsi/topology/structured_index_space.h>
#include <flecsi/topology/structured_mesh_types.h>

namespace flecsi {
namespace topology {

///
/// \class mpi_data_handle_policy_t data_handle_policy.h
/// \brief mpi_data_handle_policy_t provides...
///

template<size_t NUM_DIMS, size_t NUM_DOMS>
struct mpi_structured_topology_storage_policy__ {
  static constexpr size_t num_partitions = 5;
  
  using box_t       = flecsi::coloring::box_t; 
  using box_tag_t   = flecsi::coloring::box_tag_t; 
  using box_color_t = flecsi::coloring::box_color_t; 

  using index_spaces_t = std::array<
          structured_index_space__<
          structured_mesh_entity_base_ *,
          NUM_DIMS>,
          NUM_DIMS + 1>;

  using partition_index_spaces_t = std::array<
          structured_index_space__<
          structured_mesh_entity_base_ *,
          NUM_DIMS>,
          NUM_DIMS + 1>;

  std::array<index_spaces_t, NUM_DOMS> index_spaces;

  std::array<std::array<partition_index_spaces_t, NUM_DOMS>, num_partitions>
      partition_index_spaces;

  size_t color;

  mpi_structured_topology_storage_policy__() {
    auto & context_ = flecsi::execution::context_t::instance();
    color = context_.color();
  }


  void init(
  size_t domain,
  size_t dim, 
  bool primary,
  size_t primary_dim,
  size_t num_boxes,
  std::vector<box_t> &global_boxes,
  std::vector<std::vector<size_t>> &global_strides,  
  std::vector<box_color_t> exclusive,
  std::vector<std::vector<box_color_t>> shared,
  std::vector<std::vector<box_color_t>> ghost,
  std::vector<std::vector<box_tag_t>> domain_halo
  )
  {
    auto& is = index_spaces[domain][dim];
    is.init(primary, primary_dim, num_boxes, global_boxes, global_strides,
    exclusive, shared, ghost, domain_halo); 

  } //init

/*
  void init(
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

    auto s = is.storage();
    s->set_buffer(entities, num_entities, read);

    auto & id_storage = is.id_storage();
    id_storage.set_buffer(ids, num_entities, true);

    for (auto & domain_connectivities : topology) {
      auto & domain_connectivity__ = domain_connectivities[domain];
      for (size_t d = 0; d <= NUM_DIMS; ++d) {
        domain_connectivity__.get(d, dim).set_entity_storage(s);
      } // for
    }   // for

    if (!read) {
      return;
    }

    is.set_end(num_entities);

    size_t shared_end = num_exclusive + num_shared;
    size_t ghost_end = shared_end + num_ghost;

    for (size_t partition = 0; partition < num_partitions; ++partition) {
      auto & isp = partition_index_spaces[partition][domain][dim];
      isp.set_storage(s);
      isp.set_id_storage(&id_storage);

      switch (partition_t(partition)) {
        case exclusive:
          isp.set_begin(0);
          isp.set_end(num_exclusive);
          break;
        case shared:
          isp.set_begin(num_exclusive);
          isp.set_end(shared_end);
          break;
        case ghost:
          isp.set_begin(shared_end);
          isp.set_end(ghost_end);
          break;
        case owned:
          isp.set_begin(0);
          isp.set_end(shared_end);
          break;
        default:
          break;
      }
    }
  } // init_entities
*/
};  // class mpi_structured_topology_storage_policy__

} // namespace topology
} // namespace flecsi
