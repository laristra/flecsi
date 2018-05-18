#pragma once

#include <flecsi-config.h>

#if !defined(FLECSI_ENABLE_LEGION)
#error FLECSI_ENABLE_LEGION not defined! This file depends on Legion!
#endif

#include <legion.h>
#include <flecsi/execution/context.h>
#include <flecsi/execution/dependent_partition.h>
#include <flecsi/topology/mesh_definition.h>

namespace flecsi {
namespace execution {

class legion_entity {
public:
  Legion::LogicalRegion logical_region;
  Legion::IndexSpace index_space;
  Legion::FieldSpace field_space;
  Legion::LogicalPartition equal_lp;
  Legion::FutureMap task_fm;
  Legion::FieldID color_fid;
  Legion::FieldID id_fid;
  Legion::FieldID offset_fid;
  int id;
public:
  legion_entity() {}
  ~legion_entity()
  {
    Legion::Runtime *runtime = Legion::Runtime::get_runtime();
    Legion::Context ctx = Legion::Runtime::get_context();
    runtime->destroy_logical_region(ctx, logical_region);
    runtime->destroy_field_space(ctx, field_space);
    runtime->destroy_index_space(ctx, index_space);
  }
};

class legion_adjacency {
public:
  Legion::LogicalRegion logical_region;
  Legion::IndexSpace index_space;
  Legion::FieldSpace field_space;
  Legion::FieldID image_nrange_fid;
  Legion::FieldID image_fid;
public:
  legion_adjacency() {}
  ~legion_adjacency()
  {
    Legion::Runtime *runtime = Legion::Runtime::get_runtime();
    Legion::Context ctx = Legion::Runtime::get_context();
    runtime->destroy_logical_region(ctx, logical_region);
    runtime->destroy_field_space(ctx, field_space);
    runtime->destroy_index_space(ctx, index_space);
  }
};

class legion_partition {
public:
  Legion::IndexPartition index_partition;
  Legion::LogicalPartition logical_partition;
public:
  legion_partition() {}
};

class legion_dependent_partition : public dependent_partition{
public:
  int num_color;
  Legion::IndexSpace partition_index_space;
  Legion::Domain color_domain;
public:
  legion_dependent_partition() {}
  
  virtual dp_entity load_entity(int entities_size, int entity_id, int total_num_entities, flecsi::topology::mesh_definition_base__ &md) override;
  virtual dp_adjacency load_cell_to_entity(dp_entity &cell_region, dp_entity &entity_region, flecsi::topology::mesh_definition_base__ &md) override;
  virtual dp_partition partition_by_color(dp_entity &entity) override;
  virtual dp_partition partition_by_image(dp_entity &from_entity, dp_entity &to_entity, dp_adjacency &adjacency, dp_partition &from) override;
  virtual dp_partition partition_by_difference(dp_entity &entity, dp_partition &par1, dp_partition &par2) override;
  virtual dp_partition partition_by_intersection(dp_entity &entity, dp_partition &par1, dp_partition &par2) override;
  virtual void output_partition(dp_entity &entity, dp_partition &primary, dp_partition &ghost, dp_partition &shared, dp_partition &exclusive) override;
  virtual void min_reduction_by_color(dp_entity &entity, dp_partition &alias_partition) override;

private:
  legion_entity load_cell(int cells_size, int total_num_entities, flecsi::topology::mesh_definition_base__ &md);
  legion_entity load_non_cell(int entities_size, int entity_id);
  legion_adjacency load_cell_to_cell(legion_entity &cell_region, flecsi::topology::mesh_definition_base__ &md);
  legion_adjacency load_cell_to_others(legion_entity &cell_region, legion_entity &other_region, flecsi::topology::mesh_definition_base__ &md);
  void set_offset(legion_entity &entity, legion_partition &primary);
};
  
} // namespace execution
} // namespace flecsi