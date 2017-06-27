/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_topology_legion_topology_storage_policy_h
#define flecsi_topology_legion_topology_storage_policy_h

#include <legion.h>
#include <legion_stl.h>
#include <legion_utilities.h>
#include <arrays.h>

#include "flecsi/topology/mesh_storage.h"

#include <array>

#include "flecsi/topology/index_space.h"
#include "flecsi/utils/id.h"

///
/// \file
/// \date Initial file creation: Apr 04, 2017
///

namespace flecsi {
namespace topology {

template<size_t, size_t>
class mesh_entity_t;

///
/// \class legion_data_handle_policy_t data_handle_policy.h
/// \brief legion_data_handle_policy_t provides...
///

template<
  size_t ND,
  size_t NM
>
struct legion_topology_storage_policy_t
{
  using id_t = utils::id_t;

  using index_spaces_t = 
    std::array<index_space<mesh_entity_base_*, true, true, true>, ND + 1>;

  // array of array of domain_connectivity
  std::array<std::array<domain_connectivity<ND>, NM>, NM> topology;

  std::array<index_spaces_t, NM> index_spaces;

  template<
    size_t D,
    size_t M,
    typename ET
  >
  void
  add_entity(
    ET* ent,
    size_t partition_id = 0
  )
  {
    using dtype = domain_entity<M, ET>;

    auto & is = index_spaces[M][D].template cast<dtype>();

    id_t global_id = id_t::make<D, M>(is.size(), partition_id);

    auto typed_ent = static_cast<mesh_entity_base_t<NM>*>(ent);

    typed_ent->template set_global_id<M>(global_id);
    is.push_back(ent);
  }

  void
  init_entities(
    size_t domain,
    size_t dim,
    mesh_entity_base_* entities,
    size_t num_entities
  )
  {
    auto& is = index_spaces[domain][dim];
    is.storage()->set_buffer(entities, num_entities);
    
    for(auto& from_domain : topology){
      auto& to_domain_connectivty = from_domain[domain];
      for(size_t from_dim = 0; from_dim <= ND; ++from_dim){
        auto& conn = to_domain_connectivty.get(from_dim, dim);
        conn->entity_storage()->set_buffer(entities, num_entities);
      }
    }
  }

  void
  init_connectivity(
    size_t from_domain,                
    size_t to_domain,                
    size_t from_dim,                
    size_t to_dim,
    LegionRuntime::Arrays::Point<2>* positions,
    uint64_t* indices,
    size_t num_positions
  )
  {
    // TODO - this is an initial implementation for testing purposes.
    // We may wish to store the buffer pointers coming from Legion directly
    // into the connectivity

    auto& conn = topology[from_domain][to_domain].get(from_dim, to_dim);
    size_t index_offset = 0;
    for(size_t i = 0; i < num_positions; ++i){
      auto& pi = positions[i]; 
      size_t offset = pi.x[0];
      size_t count = pi.x[1];

      for(size_t j = index_offset; j < index_offset + count; ++j){
        conn.push(indices[j]);
      }

      conn.end_from();

      index_offset += count;
    }
  }

  template<
    size_t D,
    size_t N
  >
  size_t
  entity_dimension(mesh_entity_t<D, N>*)
  {
    return D;
  }

  void
  set_entity_storage(
    size_t domain,
    size_t dim,
    mesh_entity_base_* entities,
    size_t num_entities
  )
  {
    auto& is = index_spaces[domain][dim];
    is.storage()->set_buffer(entities, num_entities);
  }

  template <class T, size_t M, class... S>
  T * make(S &&... args)
  {    
    T* entity;
    size_t dim = entity_dimension(entity);
    entity = new T(std::forward<S>(args)...);
    return entity;
  } // make

}; // class legion_topology_storage_policy_t

} // namespace topology
} // namespace flecsi

#endif // flecsi_topology_legion_topology_storage_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
