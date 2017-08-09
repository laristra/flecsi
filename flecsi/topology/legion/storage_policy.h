/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_topology_legion_topology_storage_policy_h
#define flecsi_topology_legion_topology_storage_policy_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Apr 04, 2017
//----------------------------------------------------------------------------//

#include <array>
#include <arrays.h>
#include <legion.h>
#include <legion_stl.h>
#include <legion_utilities.h>

#include "flecsi/topology/mesh_storage.h"

#include "flecsi/topology/mesh_types.h"
#include "flecsi/topology/legion/entity_storage.h"

#include "flecsi/topology/index_space.h"
#include "flecsi/utils/id.h"

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
  static constexpr size_t num_partitions = 5;

  using id_t = utils::id_t;

  using index_spaces_t = 
    std::array<index_space<mesh_entity_base_*, true, true, true,
      void, topology_storage__>, ND + 1>;

  using partition_index_spaces_t = 
    std::array<index_space<mesh_entity_base_*, false, false, true,
      void, topology_storage__>, ND + 1>;

  // array of array of domain_connectivity
  std::array<std::array<domain_connectivity<ND>, NM>, NM> topology;

  std::array<index_spaces_t, NM> index_spaces;

  std::array<std::array<partition_index_spaces_t, NM>, num_partitions> 
    partition_index_spaces;

  void
  init_entities(
    size_t domain,
    size_t dim,
    mesh_entity_base_* entities,
    utils::id_t* ids,
    size_t size,
    size_t num_entities,
    size_t num_exclusive,
    size_t num_shared,
    size_t num_ghost,
    bool read
  )
  {
    auto& is = index_spaces[domain][dim];

    auto s = is.storage();
    s->set_buffer(entities, num_entities, read);

    if(read){
      is.set_end(num_entities);
    }

    size_t shared_end = num_exclusive + num_shared;
    size_t ghost_end = shared_end + num_ghost;

    auto& id_storage = is.id_storage();
    id_storage.set_buffer(ids, num_entities, read);

    for(size_t partition = 0; partition < num_partitions; ++partition){
      auto& isp = partition_index_spaces[partition][domain][dim];
      isp.set_storage(s);
      isp.set_id_storage(&id_storage);

      switch(partition_t(partition)){
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

    for(auto& domain_connectivities : topology) {
      auto& domain_connectivity = domain_connectivities[domain];
      for(size_t d = 0; d <= ND; ++d) {
        domain_connectivity.get(dim, d).set_entity_storage(s);
        domain_connectivity.get(d, dim).set_entity_storage(s);
      } // for
    } // for      
  } // init_entities

  void
  init_connectivity(
    size_t from_domain,
    size_t to_domain,
    size_t from_dim,
    size_t to_dim,
    LegionRuntime::Arrays::Point<2>* offsets,
    size_t num_offsets,
    utils::id_t* indices,
    size_t num_indices,
    bool read 
  )
  {
    // TODO - this is an initial implementation for testing purposes.
    // We may wish to store the buffer pointers coming from Legion directly
    // into the connectivity

    auto& conn = topology[from_domain][to_domain].get(from_dim, to_dim);

    auto& id_storage = conn.get_index_space().id_storage();
    id_storage.set_buffer(indices, num_indices, read);

    if(read){
      conn.get_index_space().set_end(num_indices);
    }

    conn.offsets().storage().set_buffer(offsets, num_offsets, read);
    conn.set_enabled(true);

  } // init_connectivities

  template<
    size_t D,
    size_t N
  >
  size_t
  entity_dimension(mesh_entity_t<D, N>*)
  {
    return D;
  }

  template<
    class T,
    size_t M,
    class... S
  >
  T * make(S &&... args)
  {
    using dtype = domain_entity<M, T>;

    T* ent;
    size_t dim = entity_dimension(ent);
    auto & is = index_spaces[M][dim].template cast<dtype>();
    size_t entity_id = is.size();

    auto placement_ptr = static_cast<T*>(is.storage()->buffer()) + entity_id;
    ent = new (placement_ptr) T(std::forward<S>(args)...);

    id_t global_id = id_t::make<M>(dim, entity_id);
    ent->template set_global_id<M>(global_id);

    auto& id_storage = is.id_storage();
    id_storage.push_back(global_id);

    is.pushed();

    return ent;
  } // make

  template<
    class T,
    size_t M,
    class... S
  >
  T *
  make(
    id_t const & id,
    S && ... args
  )
  {
    using dtype = domain_entity<M, T>;

    T* ent;
    size_t dim = entity_dimension(ent);
    auto & is = index_spaces[M][dim].template cast<dtype>();

    auto placement_ptr = static_cast<T*>(is.storage()->buffer()) + id.entity();
    ent = new (placement_ptr) T(std::forward<S>(args)...);

    ent->template set_global_id<M>(id);

    auto& id_storage = is.id_storage();
    id_storage.push_back(id);

    is.pushed();

    return ent;
  } // make

}; // class legion_topology_storage_policy_t

} // namespace topology
} // namespace flecsi

#endif // flecsi_topology_legion_topology_storage_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
