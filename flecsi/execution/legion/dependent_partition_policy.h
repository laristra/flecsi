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
  int map_id;
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

class legion_dependent_partition_policy_t {
public:
  using space_t = legion_entity;
  using map_t = legion_adjacency;
  using set_t = legion_partition;

  int num_color;
  Legion::IndexSpace partition_index_space;
  Legion::Domain color_domain;
public:
  legion_dependent_partition_policy_t() {}
  
  legion_entity load_entity(int entities_size, int entity_id, int entity_map_id, std::vector<int> &entity_vector, flecsi::topology::mesh_definition_base__ &md);
  legion_adjacency load_cell_to_entity(legion_entity &cell_region, legion_entity &entity_region, flecsi::topology::mesh_definition_base__ &md);
  legion_partition partition_by_color(legion_entity &entity);
  legion_partition partition_by_image(legion_entity &from_entity, legion_entity &to_entity, legion_adjacency &adjacency, legion_partition &from);
  legion_partition partition_by_difference(legion_entity &entity, legion_partition &par1, legion_partition &par2);
  legion_partition partition_by_intersection(legion_entity &entity, legion_partition &par1, legion_partition &par2);
  void output_partition(legion_entity &entity, legion_partition &primary, legion_partition &ghost, legion_partition &shared, legion_partition &exclusive);
  void print_partition(legion_entity &entity, legion_partition &primary, legion_partition &ghost, legion_partition &shared, legion_partition &exclusive, int print_flag);
  void min_reduction_by_color(legion_entity &entity, legion_partition &alias_partition);

public:
  legion_entity load_cell(int cells_size, int entity_map_id, std::vector<int> &entity_vector, flecsi::topology::mesh_definition_base__ &md);
  legion_entity load_non_cell(int entities_size, int entity_id, int entity_map_id);
  legion_adjacency load_cell_to_cell(legion_entity &cell_region, flecsi::topology::mesh_definition_base__ &md);
  legion_adjacency load_cell_to_others(legion_entity &cell_region, legion_entity &other_region, flecsi::topology::mesh_definition_base__ &md);
  void set_offset(legion_entity &entity, legion_partition &primary);
};

} // namespace execution
} // namespace flecsi