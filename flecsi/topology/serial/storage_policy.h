/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_topology_serial_topology_storage_policy_h
#define flecsi_topology_serial_topology_storage_policy_h

#include "flecsi/topology/mesh_storage.h"

#include <array>
#include <unordered_map>
#include <cassert>
#include <iostream>
#include <vector>

#include "flecsi/data/data_client.h"
#include "flecsi/topology/mesh_utils.h"
#include "flecsi/utils/array_ref.h"
#include "flecsi/utils/reorder.h"
#include "flecsi/topology/index_space.h"

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
/// \class serial_data_handle_policy_t data_handle_policy.h
/// \brief serial_data_handle_policy_t provides...
///

template <size_t ND, size_t NM>
struct serial_topology_storage_policy_t
{
  static constexpr size_t num_partitions = 1;

  using id_t = utils::id_t;

  using index_spaces_t = 
    std::array<index_space<mesh_entity_base_*, true, true, true,
    void/*, entity_storage_t*/>, ND + 1>;

  // array of array of domain_connectivity
  std::array<std::array<domain_connectivity<ND>, NM>, NM> topology;

  std::array<std::array<index_spaces_t, NM>, num_partitions> index_spaces;

  ~serial_topology_storage_policy_t()
  {
    for (size_t m = 0; m < NM; ++m) {
      for (size_t d = 0; d <= ND; ++d) {
        auto & is = index_spaces[all][m][d];
        for (auto ent : is) {
          delete ent;
        }
      }
    }
  }

  template<size_t D, size_t N>
  size_t
  entity_dimension(mesh_entity_t<D, N>*)
  {
    return D;
  }

  template <class T, size_t M = 0, class... S>
  T * make(S &&... args)
  {
    T* ent;
    size_t dim = entity_dimension(ent);
    ent = new T(std::forward<S>(args)...);

    using dtype = domain_entity<M, T>;

    auto & is = index_spaces[all][M][dim].template cast<dtype>();

    id_t global_id = id_t::make<M>(dim, is.size());

    auto typed_ent = static_cast<mesh_entity_base_t<NM>*>(ent);

    typed_ent->template set_global_id<M>(global_id);
    is.push_back(ent);

    return ent;
  } // make

}; // class serial_topology_storage_policy_t

} // namespace topology
} // namespace flecsi

#endif // flecsi_topology_serial_topology_storage_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/

