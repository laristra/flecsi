#include <mpi.h>
#include <unistd.h>

#include <flecsi/execution/legion/dependent_partition_policy.h>
#include <flecsi/execution/legion/legion_tasks.h>
#include <flecsi/io/simple_definition.h>

namespace flecsi {
namespace execution {

legion_entity legion_dependent_partition_policy_t::load_entity(int entities_size, int entity_id, int entity_map_id, std::vector<int> &entity_vector, flecsi::topology::mesh_definition_base__ &md)
{
  if (entity_id == md.get_dimension()) {
    return load_cell(entities_size, entity_map_id, entity_vector, md);
  } else {
    return load_non_cell(entities_size, entity_id, entity_map_id);
  }
}

legion_entity legion_dependent_partition_policy_t::load_cell(int cells_size, int entity_map_id, std::vector<int> &entity_vector, flecsi::topology::mesh_definition_base__ &md)
{
	printf("[Load cells], num_cells %d, total_num_entities %d\n", cells_size, entity_vector.size());
  
  assert(entity_vector[0] == 2);
  
  Legion::Runtime *runtime = Legion::Runtime::get_runtime();
  Legion::Context ctx = Legion::Runtime::get_context();
  context_t & context_ = context_t::instance();
  
  task_mesh_definition_t task_md;
  
  // **************************************************************************
	// Create cell index space, field space and logical region
	LegionRuntime::Arrays::Rect<1> cell_bound(LegionRuntime::Arrays::Point<1>(0),
																						LegionRuntime::Arrays::Point<1>(cells_size-1));
	Legion::Domain cell_dom(Legion::Domain::from_rect<1>(cell_bound));
	Legion::IndexSpace cell_index_space = runtime->create_index_space(ctx, cell_dom);
	Legion::FieldSpace cell_field_space = runtime->create_field_space(ctx);
	Legion::FieldAllocator cell_allocator = runtime->create_field_allocator(ctx, cell_field_space);
	cell_allocator.allocate_field(sizeof(int), FID_CELL_ID);
	runtime->attach_name(cell_field_space, FID_CELL_ID, "FID_CELL_ID");
	cell_allocator.allocate_field(sizeof(LegionRuntime::Arrays::Point<1>), FID_CELL_PARTITION_COLOR);
	runtime->attach_name(cell_field_space, FID_CELL_PARTITION_COLOR, "FID_CELL_PARTITION_COLOR");
	cell_allocator.allocate_field(sizeof(LegionRuntime::Arrays::Rect<1>), FID_CELL_CELL_NRANGE);
	runtime->attach_name(cell_field_space, FID_CELL_CELL_NRANGE, "FID_CELL_CELL_NRANGE");
  for (int i = 1; i < entity_vector.size(); i++) {
    if (entity_vector[i] == 0) {
  	  cell_allocator.allocate_field(sizeof(LegionRuntime::Arrays::Rect<1>), FID_CELL_VERTEX_NRANGE);
  	  runtime->attach_name(cell_field_space, FID_CELL_VERTEX_NRANGE, "FID_CELL_VERTEX_NRANGE");
    }
    if (entity_vector[i] == 1) {
    	cell_allocator.allocate_field(sizeof(LegionRuntime::Arrays::Rect<1>), FID_CELL_EDGE_NRANGE);
    	runtime->attach_name(cell_field_space, FID_CELL_EDGE_NRANGE, "FID_CELL_EDGE_NRANGE");
    }
    task_md.entity_array[i] = entity_vector[i];
  }
	Legion::LogicalRegion cell_lr = runtime->create_logical_region(ctx,cell_index_space,cell_field_space);
  cell_allocator.allocate_field(sizeof(int), FID_CELL_OFFSET);
  runtime->attach_name(cell_field_space, FID_CELL_OFFSET, "FID_CELL_OFFSET");
	runtime->attach_name(cell_lr, "cells_lr");
  
  // **************************************************************************
  // Create color domain the same size of MPI comm size
	MPI_Comm_size(MPI_COMM_WORLD, &num_color);
  
	LegionRuntime::Arrays::Rect<1> color_bound(LegionRuntime::Arrays::Point<1>(0),
																						 LegionRuntime::Arrays::Point<1>(num_color-1));
	color_domain = Legion::Domain(Legion::Domain::from_rect<1>(color_bound));
	partition_index_space = runtime->create_index_space(ctx, color_domain);
  
  
  // **************************************************************************
	// Create equal partition and launch index task to init cell and vertex
  // "Equal partition" for cell is not Legion's create_equal_partition because Legion's 
  // "equal partition" strategy is different from Flecsi make_dcrs, we rely on
  // make_dcrs to create cell to cell connectivity, so here we use
  // the strategy of make_dcrs
	int *cell_count_per_subspace_scan = (int*)malloc(sizeof(int) * (num_color+1));
	int total_cell_count = 0;
	int j;
	int quot = cells_size / num_color;
	int rem = cells_size % num_color;
	int cell_count_per_color = 0;	
	cell_count_per_subspace_scan[0] = 0;
  printf("[Load cells] num_cells_per_rank: ");
	for (j = 0; j < num_color; j++) {
		cell_count_per_color = quot + ((j >= (num_color - rem)) ? 1 : 0);
		printf("%d, ", j, cell_count_per_color);
		total_cell_count += cell_count_per_color;
		cell_count_per_subspace_scan[j+1] = total_cell_count;
	}
  printf("\n");
	
  printf("[Load cells] cell_count_per_subspace_scan: ");
	Legion::DomainColoring cell_equal_color_partitioning;
  for(j = 0; j < num_color; j++){
    LegionRuntime::Arrays::Rect<1> subrect(
      LegionRuntime::Arrays::Point<1>(cell_count_per_subspace_scan[j]), 
			LegionRuntime::Arrays::Point<1>(cell_count_per_subspace_scan[j+1]-1));
    cell_equal_color_partitioning[j] = Legion::Domain::from_rect<1>(subrect);
		printf("(%d, %d), ", cell_count_per_subspace_scan[j], cell_count_per_subspace_scan[j+1]-1);
  }
  printf("\n");
	
	Legion::IndexPartition cell_equal_ip = 
    runtime->create_index_partition(ctx, cell_lr.get_index_space(), color_domain, cell_equal_color_partitioning, true /*disjoint*/);
	Legion::LogicalPartition cell_equal_lp = runtime->get_logical_partition(ctx, cell_lr, cell_equal_ip);
  
  // **************************************************************************
  
  legion_entity cell_region;

  task_md.md_ptr = (intptr_t )&md;
  task_md.total_num_entities = entity_vector.size();

  // Launch index task to init cell and vertex
  const auto init_cell_task_id =
    context_.task_id<__flecsi_internal_task_key(init_cell_task)>();
  
  Legion::IndexLauncher init_cell_launcher(init_cell_task_id,
      																		 partition_index_space, Legion::TaskArgument(&task_md, sizeof(task_mesh_definition_t)),
      																		 Legion::ArgumentMap());
	
	init_cell_launcher.add_region_requirement(
	  Legion::RegionRequirement(cell_equal_lp, 0/*projection ID*/,
	                            READ_WRITE, EXCLUSIVE, cell_lr));
  init_cell_launcher.region_requirements[0].add_field(FID_CELL_ID);
  init_cell_launcher.region_requirements[0].add_field(FID_CELL_PARTITION_COLOR);
			
  init_cell_launcher.tag = MAPPER_FORCE_RANK_MATCH;
  cell_region.task_fm = runtime->execute_index_space(ctx, init_cell_launcher);
  cell_region.task_fm.wait_all_results(true);
  
  cell_region.logical_region = cell_lr;
  cell_region.index_space = cell_index_space;
  cell_region.field_space = cell_field_space;
  cell_region.equal_lp = cell_equal_lp;
  cell_region.color_fid = FID_CELL_PARTITION_COLOR;
  cell_region.id_fid = FID_CELL_ID;
  cell_region.offset_fid = FID_CELL_OFFSET;
  cell_region.id = md.get_dimension();
  cell_region.map_id = entity_map_id;
  
  free(cell_count_per_subspace_scan);
  
  return cell_region;
}

legion_entity legion_dependent_partition_policy_t::load_non_cell(int entities_size, int entity_id, int entity_map_id)
{
  printf("[Load non_cells], num_entities %d, entity_id %d\n", entities_size, entity_id);
  
  Legion::Runtime *runtime = Legion::Runtime::get_runtime();
  Legion::Context ctx = Legion::Runtime::get_context();
  context_t & context_ = context_t::instance();
  
  // **************************************************************************
	// Create vertex index space, field space and logical region
  
  int id_fid, color_fid, offset_fid;
  if(entity_id == 0) {
    id_fid = FID_VERTEX_ID;
    color_fid = FID_VERTEX_PARTITION_COLOR;
    offset_fid = FID_VERTEX_OFFSET;
  } else if (entity_id == 1) {
    id_fid = FID_EDGE_ID;
    color_fid = FID_EDGE_PARTITION_COLOR;
    offset_fid = FID_EDGE_OFFSET;
  } else {
    assert(0);
  }
  
	LegionRuntime::Arrays::Rect<1> vertex_bound(LegionRuntime::Arrays::Point<1>(0),
																						  LegionRuntime::Arrays::Point<1>(entities_size-1));
	Legion::Domain entity_dom(Legion::Domain::from_rect<1>(vertex_bound));
	Legion::IndexSpace entity_index_space = runtime->create_index_space(ctx, entity_dom);
	Legion::FieldSpace entity_field_space = runtime->create_field_space(ctx);
	Legion::FieldAllocator entity_allocator = runtime->create_field_allocator(ctx, entity_field_space);
	entity_allocator.allocate_field(sizeof(int), id_fid);
	entity_allocator.allocate_field(sizeof(LegionRuntime::Arrays::Point<1>), color_fid);
  entity_allocator.allocate_field(sizeof(int), offset_fid);
	Legion::LogicalRegion entity_lr = runtime->create_logical_region(ctx,entity_index_space,entity_field_space);
  
	// We use Legion create_equal_partition for vertex because we do not use Flecsi function to
  // create cell to vertex connectivity. 
  Legion::IndexPartition entity_equal_ip = 
	  runtime->create_equal_partition(ctx, entity_lr.get_index_space(), partition_index_space);
	Legion::LogicalPartition entity_equal_lp = runtime->get_logical_partition(ctx, entity_lr, entity_equal_ip);
  
  // **************************************************************************
  // Launch index task to init cell and vertex
  const auto init_non_cell_task_id =
    context_.task_id<__flecsi_internal_task_key(init_non_cell_task)>();
  
  task_entity_t task_entity;
  task_entity.entity_id = entity_id;
  task_entity.id_fid = id_fid;
  task_entity.color_fid = color_fid;
  
  Legion::IndexLauncher init_non_cell_launcher(init_non_cell_task_id,
      																		 partition_index_space, Legion::TaskArgument(&task_entity, sizeof(task_entity_t)),
      																		 Legion::ArgumentMap());
	
	init_non_cell_launcher.add_region_requirement(
	  Legion::RegionRequirement(entity_equal_lp, 0/*projection ID*/,
	                            READ_WRITE, EXCLUSIVE, entity_lr));
  init_non_cell_launcher.region_requirements[0].add_field(id_fid);
	init_non_cell_launcher.region_requirements[0].add_field(color_fid);
			
  init_non_cell_launcher.tag = MAPPER_FORCE_RANK_MATCH;
  Legion::FutureMap fm_epoch1 = runtime->execute_index_space(ctx, init_non_cell_launcher);
  fm_epoch1.wait_all_results(true);
  
  legion_entity entity_region;
  entity_region.logical_region = entity_lr;
  entity_region.index_space = entity_index_space;
  entity_region.field_space = entity_field_space;
  entity_region.equal_lp = entity_equal_lp;
  entity_region.color_fid = color_fid;
  entity_region.id_fid = id_fid;
  entity_region.offset_fid = offset_fid;
  entity_region.id = entity_id;
  entity_region.map_id = entity_map_id;
  
  return entity_region;
}

legion_adjacency legion_dependent_partition_policy_t::load_cell_to_entity(legion_entity &cell_region, legion_entity &entity_region, flecsi::topology::mesh_definition_base__ &md)
{
  if (cell_region.id == entity_region.id) {
    return load_cell_to_cell(cell_region, md);
  } else {
    return load_cell_to_others(cell_region, entity_region, md);
  }
}

legion_adjacency legion_dependent_partition_policy_t::load_cell_to_cell(legion_entity &cell_region, flecsi::topology::mesh_definition_base__ &md)
{
  Legion::Runtime *runtime = Legion::Runtime::get_runtime();
  Legion::Context ctx = Legion::Runtime::get_context();
  context_t & context_ = context_t::instance();
  
  // **************************************************************************
	// Get the count of cell to cell connectivity from init_cell_task and add them together
	int total_cell_to_cell_count = 0;
	int *task_args_buff = (int*)malloc(sizeof(int) * (num_color+1 + TASK_MD_OFFSET));
  int *cell_to_cell_count_per_subspace_scan = task_args_buff + TASK_MD_OFFSET;
	cell_to_cell_count_per_subspace_scan[0] = 0;
	for (int j = 0; j < num_color; j++) {
		init_mesh_task_rt_t rt_value = cell_region.task_fm.get_result<init_mesh_task_rt_t>(j);
		total_cell_to_cell_count += rt_value.cell_to_cell_count;
		cell_to_cell_count_per_subspace_scan[j+1] = total_cell_to_cell_count;
	}
	printf("[Load cell to cell] total cell to cell count %d\n", total_cell_to_cell_count);
	
	// create cell to cell connectivity index space, field space and logical region with the total_cell_to_cell_count
	LegionRuntime::Arrays::Rect<1> cell_to_cell_bound(LegionRuntime::Arrays::Point<1>(0),
																										LegionRuntime::Arrays::Point<1>(total_cell_to_cell_count-1));
	Legion::Domain cell_to_cell_dom(Legion::Domain::from_rect<1>(cell_to_cell_bound));
	Legion::IndexSpace cell_to_cell_index_space = runtime->create_index_space(ctx, cell_to_cell_dom);
	runtime->attach_name(cell_to_cell_index_space, "cell_to_cell_index_space");
	Legion::FieldSpace cell_to_cell_field_space = runtime->create_field_space(ctx);
	Legion::FieldAllocator cell_to_cell_allocator = runtime->create_field_allocator(ctx, cell_to_cell_field_space);
	cell_to_cell_allocator.allocate_field(sizeof(int), FID_CELL_TO_CELL_ID);
	runtime->attach_name(cell_to_cell_field_space, FID_CELL_TO_CELL_ID, "FID_CELL_TO_CELL_ID");
	cell_to_cell_allocator.allocate_field(sizeof(LegionRuntime::Arrays::Point<1>), FID_CELL_TO_CELL_PTR);
	runtime->attach_name(cell_to_cell_field_space, FID_CELL_TO_CELL_PTR, "FID_CELL_TO_CELL_PTR");
	Legion::LogicalRegion cell_to_cell_lr = runtime->create_logical_region(ctx,cell_to_cell_index_space,cell_to_cell_field_space);
	runtime->attach_name(cell_to_cell_lr, "cell_to_cell_lr");
  
	// Partition the cell to cell connectivity following the pattern of cell_to_cell_count_per_subspace_scan.
	// For example, the cell_to_cell_count_per_subspace_scan is 40,40,50,50, then the partition is also 40,40,50,50
  printf("[Load cell to cell] cell_to_cell_count_per_subspace_scan: ");
	Legion::DomainColoring cell_to_cell_color_partitioning;
  for(int j = 0; j < num_color; j++){
    LegionRuntime::Arrays::Rect<1> subrect(
      LegionRuntime::Arrays::Point<1>(cell_to_cell_count_per_subspace_scan[j]), 
			LegionRuntime::Arrays::Point<1>(cell_to_cell_count_per_subspace_scan[j+1]-1));
    cell_to_cell_color_partitioning[j] = Legion::Domain::from_rect<1>(subrect);
		printf("(%d, %d), ", cell_to_cell_count_per_subspace_scan[j], cell_to_cell_count_per_subspace_scan[j+1]-1);
  }
  printf("\n");
	
	Legion::IndexPartition cell_to_cell_ip = 
    runtime->create_index_partition(ctx, cell_to_cell_lr.get_index_space(), 
                                    color_domain, cell_to_cell_color_partitioning, true /*disjoint*/);
	Legion::LogicalPartition cell_to_cell_lp = runtime->get_logical_partition(ctx, cell_to_cell_lr, cell_to_cell_ip);

  // **************************************************************************
	// Launch index task to init cell to cell and cell to vertex connectivity
  const auto init_cell_to_cell_task_id =
    context_.task_id<__flecsi_internal_task_key(init_cell_to_cell_task)>();
  
  task_mesh_definition_t task_md;
  task_md.md_ptr = (intptr_t )&md;
  memcpy(task_args_buff, &task_md, sizeof(task_mesh_definition_t));
  
	Legion::ArgumentMap arg_map_init_cell_to_cell;
  Legion::IndexLauncher init_cell_to_cell_launcher(init_cell_to_cell_task_id,
      																					partition_index_space, 
																								Legion::TaskArgument(task_args_buff, sizeof(int)*(num_color+1 + TASK_MD_OFFSET)),
      																					arg_map_init_cell_to_cell);
	
	init_cell_to_cell_launcher.add_region_requirement(
	  Legion::RegionRequirement(cell_region.equal_lp, 0/*projection ID*/,
	                            READ_WRITE, EXCLUSIVE, cell_region.logical_region));
  init_cell_to_cell_launcher.region_requirements[0].add_field(FID_CELL_ID);
  init_cell_to_cell_launcher.region_requirements[0].add_field(FID_CELL_CELL_NRANGE);
	
	init_cell_to_cell_launcher.add_region_requirement(
	  Legion::RegionRequirement(cell_to_cell_lp, 0/*projection ID*/,
	                            READ_WRITE, EXCLUSIVE, cell_to_cell_lr));
	init_cell_to_cell_launcher.region_requirements[1].add_field(FID_CELL_TO_CELL_ID);
	init_cell_to_cell_launcher.region_requirements[1].add_field(FID_CELL_TO_CELL_PTR);
	
  init_cell_to_cell_launcher.tag = MAPPER_FORCE_RANK_MATCH;
  Legion::FutureMap fm_epoch2 = runtime->execute_index_space(ctx, init_cell_to_cell_launcher);
  fm_epoch2.wait_all_results(true);
  
  legion_adjacency cell_to_cell_adjacency;
  
  cell_to_cell_adjacency.logical_region = cell_to_cell_lr;
  cell_to_cell_adjacency.index_space = cell_to_cell_index_space;
  cell_to_cell_adjacency.field_space = cell_to_cell_field_space;
  cell_to_cell_adjacency.image_nrange_fid = FID_CELL_CELL_NRANGE;
  cell_to_cell_adjacency.image_fid = FID_CELL_TO_CELL_PTR;
  
  free(task_args_buff);
  
  return cell_to_cell_adjacency;
}

legion_adjacency legion_dependent_partition_policy_t::load_cell_to_others(legion_entity &cell_region, legion_entity &entity_region, flecsi::topology::mesh_definition_base__ &md)
{
  Legion::Runtime *runtime = Legion::Runtime::get_runtime();
  Legion::Context ctx = Legion::Runtime::get_context();
  context_t & context_ = context_t::instance();
  
  int cell_to_others_id_fid, cell_to_others_ptr_fid, image_nrange_fid, image_fid;
  if(entity_region.id == 0) {
    cell_to_others_id_fid = FID_CELL_TO_VERTEX_ID;
    cell_to_others_ptr_fid = FID_CELL_TO_VERTEX_PTR;
    image_nrange_fid = FID_CELL_VERTEX_NRANGE;
    image_fid = FID_CELL_TO_VERTEX_PTR;
  } else if (entity_region.id == 1) {
    cell_to_others_id_fid = FID_CELL_TO_EDGE_ID;
    cell_to_others_ptr_fid = FID_CELL_TO_EDGE_PTR;
    image_nrange_fid = FID_CELL_EDGE_NRANGE;
    image_fid = FID_CELL_TO_EDGE_PTR;
  } else {
    assert(0);
  }
  
 // **************************************************************************
	// Get the count of cell to vertex connectivity from init_mesh_task and add them together
	int total_cell_to_others_count = 0;
	int *task_args_buff = (int*)malloc(sizeof(int) * (num_color+1 + TASK_MD_OFFSET));
  int *cell_to_others_count_per_subspace_scan = task_args_buff + TASK_MD_OFFSET;
	cell_to_others_count_per_subspace_scan[0] = 0;
	for (int j = 0; j < num_color; j++) {
		init_mesh_task_rt_t rt_value = cell_region.task_fm.get_result<init_mesh_task_rt_t>(j);
    //TODO face
    if (entity_region.id == 0) {
		  total_cell_to_others_count += rt_value.cell_to_vertex_count;
    } else if (entity_region.id == 1) {
      total_cell_to_others_count += rt_value.cell_to_edge_count;
    } else {
      assert(0);
    }
		cell_to_others_count_per_subspace_scan[j+1] = total_cell_to_others_count;
	}
  
	// create cell to vertex connectivity index space, field space and logical region with the total_cell_to_vertex_count
	LegionRuntime::Arrays::Rect<1> cell_to_others_bound(LegionRuntime::Arrays::Point<1>(0),
																								  		LegionRuntime::Arrays::Point<1>(total_cell_to_others_count-1));
	Legion::Domain cell_to_others_dom(Legion::Domain::from_rect<1>(cell_to_others_bound));
	Legion::IndexSpace cell_to_others_index_space = runtime->create_index_space(ctx, cell_to_others_dom);
	Legion::FieldSpace cell_to_others_field_space = runtime->create_field_space(ctx);
	Legion::FieldAllocator cell_to_others_allocator = runtime->create_field_allocator(ctx, cell_to_others_field_space);
	cell_to_others_allocator.allocate_field(sizeof(int), cell_to_others_id_fid);
	cell_to_others_allocator.allocate_field(sizeof(LegionRuntime::Arrays::Point<1>), cell_to_others_ptr_fid);
	Legion::LogicalRegion cell_to_others_lr = runtime->create_logical_region(ctx,cell_to_others_index_space,cell_to_others_field_space);
  
	// Partition the cell to vertex connectivity following the pattern of cell_to_vertex_count_per_subspace_scan.
	// For example, the cell_to_vertex_count_per subspace_scan is 40,40,50,50, then the partition is also 40,40,50,50
  printf("[Load cell to others] cell_to_others_count_per_subspace_scan: ");
	Legion::DomainColoring cell_to_others_color_partitioning;
  for(int j = 0; j < num_color; j++){
    LegionRuntime::Arrays::Rect<1> subrect(
      LegionRuntime::Arrays::Point<1>(cell_to_others_count_per_subspace_scan[j]), 
			LegionRuntime::Arrays::Point<1>(cell_to_others_count_per_subspace_scan[j+1]-1));
    cell_to_others_color_partitioning[j] = Legion::Domain::from_rect<1>(subrect);
		printf("(%d, %d), ", cell_to_others_count_per_subspace_scan[j], cell_to_others_count_per_subspace_scan[j+1]-1);
  }
  printf("\n");
	
	Legion::IndexPartition cell_to_others_ip = 
    runtime->create_index_partition(ctx, cell_to_others_lr.get_index_space(), 
                                    color_domain, cell_to_others_color_partitioning, true /*disjoint*/);
	Legion::LogicalPartition cell_to_others_lp = runtime->get_logical_partition(ctx, cell_to_others_lr, cell_to_others_ip);
  
  // **************************************************************************
	// Launch index task to init cell to vertex connectivity
  const auto init_cell_to_others_task_id =
    context_.task_id<__flecsi_internal_task_key(init_cell_to_others_task)>();
  
  task_mesh_definition_t task_md;
  task_md.md_ptr = (intptr_t )&md;
  task_md.entity_id = entity_region.id;
  memcpy(task_args_buff, &task_md, sizeof(task_mesh_definition_t));
  
	Legion::ArgumentMap arg_map_init_cell_to_others;
  Legion::IndexLauncher init_cell_to_others_launcher(init_cell_to_others_task_id,
      																					partition_index_space, 
																								Legion::TaskArgument(task_args_buff, sizeof(int)*(num_color+1 + TASK_MD_OFFSET)),
      																					arg_map_init_cell_to_others);
	
	init_cell_to_others_launcher.add_region_requirement(
	  Legion::RegionRequirement(cell_region.equal_lp, 0/*projection ID*/,
	                            READ_WRITE, EXCLUSIVE, cell_region.logical_region));
  init_cell_to_others_launcher.region_requirements[0].add_field(FID_CELL_ID);
	init_cell_to_others_launcher.region_requirements[0].add_field(image_nrange_fid);
	
	init_cell_to_others_launcher.add_region_requirement(
	  Legion::RegionRequirement(cell_to_others_lp, 0/*projection ID*/,
	                            READ_WRITE, EXCLUSIVE, cell_to_others_lr));
	init_cell_to_others_launcher.region_requirements[1].add_field(cell_to_others_id_fid);
	init_cell_to_others_launcher.region_requirements[1].add_field(cell_to_others_ptr_fid);
			
  init_cell_to_others_launcher.tag = MAPPER_FORCE_RANK_MATCH;
  Legion::FutureMap fm_epoch2 = runtime->execute_index_space(ctx, init_cell_to_others_launcher);
  fm_epoch2.wait_all_results(true);
  
  legion_adjacency cell_to_others_adjacency;
  
  cell_to_others_adjacency.logical_region = cell_to_others_lr;
  cell_to_others_adjacency.index_space = cell_to_others_index_space;
  cell_to_others_adjacency.field_space = cell_to_others_field_space;
  cell_to_others_adjacency.image_nrange_fid = image_nrange_fid;
  cell_to_others_adjacency.image_fid = image_fid;
  
  free(task_args_buff);
  
  return cell_to_others_adjacency;
	
}

void legion_dependent_partition_policy_t::min_reduction_by_color(legion_entity &entity, legion_partition &alias_partition)
{
  Legion::Runtime *runtime = Legion::Runtime::get_runtime();
  Legion::Context ctx = Legion::Runtime::get_context();
  context_t & context_ = context_t::instance();
  
  int color_fid = entity.color_fid;
  
  // **************************************************************************
  // Launch index task to init vertex color
  const auto init_vertex_color_task_id =
    context_.task_id<__flecsi_internal_task_key(init_vertex_color_task)>();
  
  Legion::IndexLauncher init_vertex_color_launcher(init_vertex_color_task_id,
    																			 partition_index_space, Legion::TaskArgument(nullptr, 0),
      																		 Legion::ArgumentMap());
	
	init_vertex_color_launcher.add_region_requirement(
	  Legion::RegionRequirement(alias_partition.logical_partition, 0/*projection ID*/,
	                            MinReductionPointOp::redop_id, SIMULTANEOUS, entity.logical_region));
  init_vertex_color_launcher.region_requirements[0].add_field(color_fid);

  init_vertex_color_launcher.tag = MAPPER_FORCE_RANK_MATCH;
  Legion::FutureMap fm_epoch3 = runtime->execute_index_space(ctx, init_vertex_color_launcher);
  fm_epoch3.wait_all_results(true);
}

legion_partition legion_dependent_partition_policy_t::partition_by_color(legion_entity &entity)
{
  Legion::Runtime *runtime = Legion::Runtime::get_runtime();
  Legion::Context ctx = Legion::Runtime::get_context();
  
  legion_partition color_par;
  
  color_par.index_partition = 
  		runtime->create_partition_by_field(ctx, entity.logical_region,
  	                            				 entity.logical_region, entity.color_fid, partition_index_space);
  color_par.logical_partition = runtime->get_logical_partition(ctx, entity.logical_region, color_par.index_partition);
  return color_par;
}

legion_partition legion_dependent_partition_policy_t::partition_by_image(legion_entity &from_entity, legion_entity &to_entity, legion_adjacency &adjacency, legion_partition &from)
{
  Legion::Runtime *runtime = Legion::Runtime::get_runtime();
  Legion::Context ctx = Legion::Runtime::get_context();
  
  legion_partition image_par;
  
  Legion::IndexPartition primary_image_nrange_ip = 
  		runtime->create_partition_by_image_range(ctx, adjacency.logical_region.get_index_space(),
  	                                           runtime->get_logical_partition(from_entity.logical_region, from.index_partition), 
  	                                           from_entity.logical_region,
  	                                           adjacency.image_nrange_fid,
  	                                           partition_index_space);

	image_par.index_partition = 
		runtime->create_partition_by_image(ctx, to_entity.logical_region.get_index_space(),
                                       runtime->get_logical_partition(adjacency.logical_region, primary_image_nrange_ip), 
                                       adjacency.logical_region,
                                       adjacency.image_fid,
                                       partition_index_space);
  image_par.logical_partition = runtime->get_logical_partition(ctx, to_entity.logical_region, image_par.index_partition);
  return image_par;
}

legion_partition legion_dependent_partition_policy_t::partition_by_difference(legion_entity &entity, legion_partition & par1, legion_partition &par2)
{
  Legion::Runtime *runtime = Legion::Runtime::get_runtime();
  Legion::Context ctx = Legion::Runtime::get_context();
  
  legion_partition diff_par;
  diff_par.index_partition = 
  		runtime->create_partition_by_difference(ctx, entity.logical_region.get_index_space(),
  														                par1.index_partition, par2.index_partition, partition_index_space);
  diff_par.logical_partition = runtime->get_logical_partition(ctx, entity.logical_region, diff_par.index_partition);
  return diff_par;
}

legion_partition legion_dependent_partition_policy_t::partition_by_intersection(legion_entity &entity, legion_partition & par1, legion_partition &par2)
{
  Legion::Runtime *runtime = Legion::Runtime::get_runtime();
  Legion::Context ctx = Legion::Runtime::get_context();
  
  legion_partition intersection_par;
  intersection_par.index_partition = 
  		runtime->create_partition_by_intersection(ctx, entity.logical_region.get_index_space(),
  	                                            par1.index_partition, par2.index_partition, partition_index_space);  
  intersection_par.logical_partition = runtime->get_logical_partition(ctx, entity.logical_region, intersection_par.index_partition);
  return intersection_par;
}


void legion_dependent_partition_policy_t::set_offset(legion_entity &entity, legion_partition &primary)
{
  Legion::Runtime *runtime = Legion::Runtime::get_runtime();
  Legion::Context ctx = Legion::Runtime::get_context();
  context_t & context_ = context_t::instance();
  
  // **************************************************************************
	// Launch index task to init offset
  const auto set_entity_offset_task_id =
    context_.task_id<__flecsi_internal_task_key(set_entity_offset_task)>();
  
  int entity_id = entity.id;
  
  Legion::IndexLauncher set_entity_offset_launcher(set_entity_offset_task_id,
    																			 partition_index_space, Legion::TaskArgument(&entity_id, sizeof(int)),
      																		 Legion::ArgumentMap());
	
	set_entity_offset_launcher.add_region_requirement(
	  Legion::RegionRequirement(primary.logical_partition, 0/*projection ID*/,
	                            READ_WRITE, EXCLUSIVE, entity.logical_region));
  set_entity_offset_launcher.region_requirements[0].add_field(entity.offset_fid);
  
	
  set_entity_offset_launcher.tag = MAPPER_FORCE_RANK_MATCH;
  Legion::FutureMap fm_epoch_set_entity_offset = runtime->execute_index_space(ctx, set_entity_offset_launcher);
  fm_epoch_set_entity_offset.wait_all_results(true);
}


void legion_dependent_partition_policy_t::output_partition(legion_entity &entity, legion_partition &primary, legion_partition &ghost, legion_partition &shared, legion_partition &exclusive)
{
  Legion::Runtime *runtime = Legion::Runtime::get_runtime();
  Legion::Context ctx = Legion::Runtime::get_context();
  context_t & context_ = context_t::instance();
  
  set_offset(entity, primary);
  
  // **************************************************************************
	// Launch index task to verify partition results
  const auto output_partition_task_id =
    context_.task_id<__flecsi_internal_task_key(output_partition_task)>();
  
  task_entity_t task_entity;
  task_entity.entity_map_id = entity.map_id;
  task_entity.entity_id = entity.id;
  task_entity.id_fid = entity.id_fid;
  task_entity.color_fid = entity.color_fid;
  task_entity.offset_fid = entity.offset_fid;
  
  Legion::IndexLauncher output_partition_launcher(output_partition_task_id,
    																			 partition_index_space, Legion::TaskArgument(&task_entity, sizeof(task_entity_t)),
      																		 Legion::ArgumentMap());
	
	output_partition_launcher.add_region_requirement(
	  Legion::RegionRequirement(primary.logical_partition, 0/*projection ID*/,
	                            READ_ONLY, EXCLUSIVE, entity.logical_region));
  output_partition_launcher.region_requirements[0].add_field(entity.id_fid);
  output_partition_launcher.region_requirements[0].add_field(entity.color_fid);
	
	output_partition_launcher.add_region_requirement(
	  Legion::RegionRequirement(ghost.logical_partition, 0/*projection ID*/,
	                            READ_ONLY, EXCLUSIVE, entity.logical_region));
  output_partition_launcher.region_requirements[1].add_field(entity.id_fid);
  output_partition_launcher.region_requirements[1].add_field(entity.offset_fid);
  output_partition_launcher.region_requirements[1].add_field(entity.color_fid);
  
	output_partition_launcher.add_region_requirement(
	  Legion::RegionRequirement(shared.logical_partition, 0/*projection ID*/,
	                            READ_ONLY, EXCLUSIVE, entity.logical_region));
  output_partition_launcher.region_requirements[2].add_field(entity.id_fid);
  output_partition_launcher.region_requirements[2].add_field(entity.offset_fid);
  
	output_partition_launcher.add_region_requirement(
	  Legion::RegionRequirement(exclusive.logical_partition, 0/*projection ID*/,
	                            READ_ONLY, EXCLUSIVE, entity.logical_region));
  output_partition_launcher.region_requirements[3].add_field(entity.id_fid);
  output_partition_launcher.region_requirements[3].add_field(entity.offset_fid);
	

  output_partition_launcher.tag = MAPPER_FORCE_RANK_MATCH;
  Legion::FutureMap fm_epoch4 = runtime->execute_index_space(ctx, output_partition_launcher);
  fm_epoch4.wait_all_results(true);
}

void legion_dependent_partition_policy_t::print_partition(legion_entity &entity, legion_partition &primary, legion_partition &ghost, legion_partition &shared, legion_partition &exclusive, int print_flag)
{
  Legion::Runtime *runtime = Legion::Runtime::get_runtime();
  Legion::Context ctx = Legion::Runtime::get_context();
  context_t & context_ = context_t::instance();
  
 // set_offset(entity, primary);
  
  // **************************************************************************
	// Launch index task to verify partition results
  const auto print_partition_task_id =
    context_.task_id<__flecsi_internal_task_key(print_partition_task)>();
  
  task_entity_t task_entity;
  task_entity.entity_map_id = entity.map_id;
  task_entity.entity_id = entity.id;
  task_entity.id_fid = entity.id_fid;
  task_entity.color_fid = entity.color_fid;
  task_entity.offset_fid = entity.offset_fid;
  task_entity.print_flag = print_flag;
  
  Legion::IndexLauncher print_partition_launcher(print_partition_task_id,
    																			 partition_index_space, Legion::TaskArgument(&task_entity, sizeof(task_entity_t)),
      																		 Legion::ArgumentMap());
	
	print_partition_launcher.add_region_requirement(
	  Legion::RegionRequirement(primary.logical_partition, 0/*projection ID*/,
	                            READ_ONLY, EXCLUSIVE, entity.logical_region));
  print_partition_launcher.region_requirements[0].add_field(entity.id_fid);
  print_partition_launcher.region_requirements[0].add_field(entity.color_fid);
	
	print_partition_launcher.add_region_requirement(
	  Legion::RegionRequirement(ghost.logical_partition, 0/*projection ID*/,
	                            READ_ONLY, EXCLUSIVE, entity.logical_region));
  print_partition_launcher.region_requirements[1].add_field(entity.id_fid);
  print_partition_launcher.region_requirements[1].add_field(entity.offset_fid);
  print_partition_launcher.region_requirements[1].add_field(entity.color_fid);
  
	print_partition_launcher.add_region_requirement(
	  Legion::RegionRequirement(shared.logical_partition, 0/*projection ID*/,
	                            READ_ONLY, EXCLUSIVE, entity.logical_region));
  print_partition_launcher.region_requirements[2].add_field(entity.id_fid);
  print_partition_launcher.region_requirements[2].add_field(entity.offset_fid);
  
	print_partition_launcher.add_region_requirement(
	  Legion::RegionRequirement(exclusive.logical_partition, 0/*projection ID*/,
	                            READ_ONLY, EXCLUSIVE, entity.logical_region));
  print_partition_launcher.region_requirements[3].add_field(entity.id_fid);
  print_partition_launcher.region_requirements[3].add_field(entity.offset_fid);
	

  print_partition_launcher.tag = MAPPER_FORCE_RANK_MATCH;
  Legion::FutureMap fm_epoch4 = runtime->execute_index_space(ctx, print_partition_launcher);
  fm_epoch4.wait_all_results(true);
}

} // namespace execution
} // namespace flecsi
