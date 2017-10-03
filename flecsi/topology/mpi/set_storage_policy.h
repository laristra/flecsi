/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Oct 03, 2017
//----------------------------------------------------------------------------//

#ifndef flecsi_topology_mpi_topology_set_storage_policy_h
#define flecsi_topology_mpi_topology_set_storage_policy_h

#include "flecsi/execution/context.h"
#include "flecsi/topology/common/entity_storage.h"
#include "flecsi/topology/index_space.h"
#include "flecsi/topology/set_utils.h"
#include "flecsi/topology/set_types.h"

namespace flecsi{
namespace topology{

template<typename SET_TYPES>
struct mpi_set_topology_storage_policy_t
{
  using id_t = utils::id_t;

  using entity_types_t = typename SET_TYPES::entity_types;

  static const size_t num_index_spaces = 
    std::tuple_size<entity_types_t>::value;

  using index_spaces_t =
    std::array<index_space<set_entity_t*, true, true, true, void,
    topology_storage__>, num_index_spaces>;

  index_spaces_t index_spaces;

  size_t color;

  using index_space_map_t = std::map<size_t, size_t>;

  index_space_map_t index_space_map;

  template<size_t I>
  void map_index_spaces(){
    if(I == num_index_spaces){
      return;
    }

    using element_t = typename std::tuple_element<I, entity_types_t>::type;

    index_space_map[element_t::value] = I;
    map_index_spaces<I + 1>(); 
  }

  mpi_set_topology_storage_policy_t()
  {
    auto & context_ = flecsi::execution::context_t::instance();
    color = context_.color();

    map_index_spaces();
  }

  void
  init_entities(
    size_t index_space,
    set_entity_t* entities,
    utils::id_t* ids,
    size_t size,
    size_t num_entities,
    bool read
  )
  {
    auto itr = index_space_map.find(index_space);
    clog_assert(itr != index_space_map.end(), "invalid index space");
    auto& is = index_spaces[itr->second];
    auto s = is.storage();
    s->set_buffer(entities, num_entities, read);

    auto& id_storage = is.id_storage();
    id_storage.set_buffer(ids, num_entities, true);

    if(!read){
      return;
    }

    is.set_end(num_entities);
  }

  template<
    class T,
    class... S
  >
  T * make(S &&... args)
  {
    constexpr size_t index_space = 
      find_set_index_space__<num_index_spaces, entity_types_t, T>::find(); 

    auto & is = index_spaces[index_space].template cast<T*>();
    size_t entity = is.size();

    auto placement_ptr = static_cast<T*>(is.storage()->buffer()) + entity;
    auto ent = new (placement_ptr) T(std::forward<S>(args)...);

    id_t global_id = id_t::make<T::dimension>(entity, color);
    ent->template set_global_id(global_id);

    auto& id_storage = is.id_storage();

    id_storage[entity] = global_id;

    is.pushed();

    return ent;
  }

  template<
    class T,
    class... S
  >
  T *
  make(
    const id_t& id,
    S && ... args
  )
  {
    constexpr size_t index_space = 
      find_set_index_space__<num_index_spaces, entity_types_t, T>::find(); 
       
    auto & is = index_spaces[index_space].template cast<T*>();

    size_t entity = id.entity();

    auto placement_ptr = static_cast<T*>(is.storage()->buffer()) + entity;
    auto ent = new (placement_ptr) T(std::forward<S>(args)...);

    ent->template set_global_id(id);

    auto& id_storage = is.id_storage();

    id_storage[entity] = id;

    is.pushed();

    return ent;
  }
};

} // namespace topology
} // namespace flecsi

#endif // flecsi_topology_mpi_topology_set_storage_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
