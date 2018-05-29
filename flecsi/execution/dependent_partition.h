#pragma once

#include <flecsi-config.h>

#if !defined(FLECSI_ENABLE_LEGION)
#error FLECSI_ENABLE_LEGION not defined! This file depends on Legion!
#endif

#include <legion.h>
#include <flecsi/execution/context.h>
#include <flecsi/topology/mesh_definition.h>

namespace flecsi {
namespace execution {
  
class legion_entity;
class legion_adjacency;
class legion_partition;
  
class dependent_partition {
public:
  virtual legion_entity load_entity(int entities_size, int entity_id, int entity_map_id, std::vector<int> &entity_vector, flecsi::topology::mesh_definition_base__ &md) = 0;
  virtual legion_adjacency load_cell_to_entity(legion_entity &cell_region, legion_entity &entity_region, flecsi::topology::mesh_definition_base__ &md) = 0;
  virtual legion_partition partition_by_color(legion_entity &entity) = 0;
  virtual legion_partition partition_by_image(legion_entity &from_entity, legion_entity &to_entity, legion_adjacency &adjacency, legion_partition &from) = 0;
  virtual legion_partition partition_by_difference(legion_entity &entity, legion_partition &par1, legion_partition &par2) = 0;
  virtual legion_partition partition_by_intersection(legion_entity &entity, legion_partition &par1, legion_partition &par2) = 0;
  virtual void output_partition(legion_entity &entity, legion_partition &primary, legion_partition &ghost, legion_partition &shared, legion_partition &exclusive) = 0;
  virtual void min_reduction_by_color(legion_entity &entity, legion_partition &alias_partition) = 0;
};

template<class DEPENDENT_PARTITION_POLICY>
struct dependent_partition_t : public DEPENDENT_PARTITION_POLICY {
  using entity_t = typename DEPENDENT_PARTITION_POLICY::entity_t;
  using adjacency_t = typename DEPENDENT_PARTITION_POLICY::adjacency_t;
  using partition_t = typename DEPENDENT_PARTITION_POLICY::partition_t;
  
  entity_t load_entity(int entities_size, int entity_id, int entity_map_id, std::vector<int> &entity_vector, flecsi::topology::mesh_definition_base__ &md)
  {
    return DEPENDENT_PARTITION_POLICY::load_entity(entities_size, entity_id, entity_map_id, entity_vector, md);
  }
  
  adjacency_t load_cell_to_entity(entity_t &cell_region, entity_t &entity_region, flecsi::topology::mesh_definition_base__ &md)
  {
    return DEPENDENT_PARTITION_POLICY::load_cell_to_entity(cell_region, entity_region, md);
  }
  
  partition_t partition_by_color(entity_t &entity)
  {
    return DEPENDENT_PARTITION_POLICY::partition_by_color(entity);
  }
  
  partition_t partition_by_image(entity_t &from_entity, entity_t &to_entity, adjacency_t &adjacency, partition_t &from)
  {
    return DEPENDENT_PARTITION_POLICY::partition_by_image(from_entity, to_entity, adjacency, from);
  }
  
  partition_t partition_by_difference(entity_t &entity, partition_t &par1, partition_t &par2)
  {
    return DEPENDENT_PARTITION_POLICY::partition_by_difference(entity, par1, par2);
  }
  
  partition_t partition_by_intersection(entity_t &entity, partition_t &par1, partition_t &par2)
  {
    return DEPENDENT_PARTITION_POLICY::partition_by_intersection(entity, par1, par2);
  }
  
  void output_partition(entity_t &entity, partition_t &primary, partition_t &ghost, partition_t &shared, partition_t &exclusive)
  {
     DEPENDENT_PARTITION_POLICY::output_partition(entity, primary, ghost, shared, exclusive);
  }
  
  void min_reduction_by_color(entity_t &entity, partition_t &alias_partition)
  {
    DEPENDENT_PARTITION_POLICY::min_reduction_by_color(entity, alias_partition);
  }
};

/*
template<class DEPENDENT_PARTITION_POLICY>
using entity_t = typename dependent_partition_t<DEPENDENT_PARTITION_POLICY>::entity_t;

template<class DEPENDENT_PARTITION_POLICY>
using adjacency_t = typename dependent_partition_t<DEPENDENT_PARTITION_POLICY>::adjacency_t;

template<class DEPENDENT_PARTITION_POLICY>
using partition_t = typename dependent_partition_t<DEPENDENT_PARTITION_POLICY>::partition_t;
*/
} // namespace execution
} // namespace flecsi