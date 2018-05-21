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
  virtual legion_entity load_entity(int entities_size, int entity_id, int total_num_entities, flecsi::topology::mesh_definition_base__ &md) = 0;
  virtual legion_adjacency load_cell_to_entity(legion_entity &cell_region, legion_entity &entity_region, flecsi::topology::mesh_definition_base__ &md) = 0;
  virtual legion_partition partition_by_color(legion_entity &entity) = 0;
  virtual legion_partition partition_by_image(legion_entity &from_entity, legion_entity &to_entity, legion_adjacency &adjacency, legion_partition &from) = 0;
  virtual legion_partition partition_by_difference(legion_entity &entity, legion_partition &par1, legion_partition &par2) = 0;
  virtual legion_partition partition_by_intersection(legion_entity &entity, legion_partition &par1, legion_partition &par2) = 0;
  virtual void output_partition(legion_entity &entity, legion_partition &primary, legion_partition &ghost, legion_partition &shared, legion_partition &exclusive) = 0;
  virtual void min_reduction_by_color(legion_entity &entity, legion_partition &alias_partition) = 0;
};
  
} // namespace execution
} // namespace flecsi