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

template<size_t NUM_DIMS, size_t NUM_DOMS>
struct hpx_topology_storage_policy__ {
  static constexpr size_t num_partitions = 5;
  using id_t = utils::id_t;

  using index_spaces_t = std::array<
      index_space__<
          mesh_entity_base_ *,
          true,
          true,
          true,
          void,
          topology_storage__>,
      NUM_DIMS + 1>;

  using partition_index_spaces_t = std::array<
      index_space__<
          mesh_entity_base_ *,
          false,
          false,
          true,
          void,
          topology_storage__>,
      NUM_DIMS + 1>;

  // array of array of domain_connectivity__
  std::array<std::array<domain_connectivity__<NUM_DIMS>, NUM_DOMS>, NUM_DOMS>
      topology;

  std::array<index_spaces_t, NUM_DOMS> index_spaces;

  std::array<std::array<partition_index_spaces_t, NUM_DOMS>, num_partitions>
      partition_index_spaces;

  size_t color;

  hpx_topology_storage_policy__() {
    auto & context_ = flecsi::execution::context_t::instance();
    color = context_.color();
  }

  template<size_t D, size_t N>
  size_t entity_dimension(mesh_entity_t<D, N> *) {
    return D;
  }

  template<class T, size_t M = 0, class... S>
  T * make(S &&... args) {
    T * ent;
    size_t dim = entity_dimension(ent);
    ent = new T(std::forward<S>(args)...);

    using dtype = domain_entity<M, T>;

    auto & is = index_spaces[M][dim].template cast<dtype>();

    id_t global_id = id_t::make<M>(dim, is.size());

    auto typed_ent = static_cast<mesh_entity_base_t<NM> *>(ent);

    typed_ent->template set_global_id<M>(global_id);
    is.push_back(ent);

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
