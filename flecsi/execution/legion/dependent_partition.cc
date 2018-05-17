#include <mpi.h>
#include <unistd.h>

#include <flecsi/execution/legion/dependent_partition.h>
#include <flecsi/execution/legion/legion_tasks.h>
#include <flecsi/io/simple_definition.h>

namespace flecsi {
namespace execution {

legion_entity dependent_partition::load_cell(int num_cells)
{
	printf("Load cells, num_cells %d\n", num_cells);
  
  Legion::Runtime *runtime = Legion::Runtime::get_runtime();
  Legion::Context ctx = Legion::Runtime::get_context();
  context_t & context_ = context_t::instance();
  
  // **************************************************************************
	// Create cell index space, field space and logical region
	LegionRuntime::Arrays::Rect<1> cell_bound(LegionRuntime::Arrays::Point<1>(0),
																						LegionRuntime::Arrays::Point<1>(num_cells-1));
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
	cell_allocator.allocate_field(sizeof(LegionRuntime::Arrays::Rect<1>), FID_CELL_VERTEX_NRANGE);
	runtime->attach_name(cell_field_space, FID_CELL_VERTEX_NRANGE, "FID_CELL_VERTEX_NRANGE");
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
	int quot = num_cells / num_color;
	int rem = num_cells % num_color;
	int cell_count_per_color = 0;	
	cell_count_per_subspace_scan[0] = 0;
  printf("[TOP] num_cells_per_rank: ");
	for (j = 0; j < num_color; j++) {
		cell_count_per_color = quot + ((j >= (num_color - rem)) ? 1 : 0);
		printf("%d, ", j, cell_count_per_color);
		total_cell_count += cell_count_per_color;
		cell_count_per_subspace_scan[j+1] = total_cell_count;
	}
  printf("\n");
	
  printf("[TOP] cell_count_per_subspace_scan: ");
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
    
  // Launch index task to init cell and vertex
  const auto init_cell_task_id =
    context_.task_id<__flecsi_internal_task_key(init_cell_task)>();
  
  Legion::IndexLauncher init_cell_launcher(init_cell_task_id,
      																		 partition_index_space, Legion::TaskArgument(nullptr, 0),
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
  cell_region.id = 0;
  
  free(cell_count_per_subspace_scan);
  
  return cell_region;
}

legion_adjacency dependent_partition::load_cell_to_cell(legion_entity &cell_region)
{
  Legion::Runtime *runtime = Legion::Runtime::get_runtime();
  Legion::Context ctx = Legion::Runtime::get_context();
  context_t & context_ = context_t::instance();
  
  // **************************************************************************
	// Get the count of cell to cell connectivity from init_cell_task and add them together
	int total_cell_to_cell_count = 0;
	int *cell_to_cell_count_per_subspace_scan = (int*)malloc(sizeof(int) * (num_color+1));
	cell_to_cell_count_per_subspace_scan[0] = 0;
	for (int j = 0; j < num_color; j++) {
		init_mesh_task_rt_t rt_value = cell_region.task_fm.get_result<init_mesh_task_rt_t>(j);
		total_cell_to_cell_count += rt_value.cell_to_cell_count;
		cell_to_cell_count_per_subspace_scan[j+1] = total_cell_to_cell_count;
	}
	printf("[TOP] total cell to cell count %d\n", total_cell_to_cell_count);
	
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
  printf("[TOP] cell_to_cell_count_per_subspace_scan: ");
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
  
	Legion::ArgumentMap arg_map_init_cell_to_cell;
  Legion::IndexLauncher init_cell_to_cell_launcher(init_cell_to_cell_task_id,
      																					partition_index_space, 
																								Legion::TaskArgument(cell_to_cell_count_per_subspace_scan, sizeof(int)*(num_color+1)),
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
  
  return cell_to_cell_adjacency;
}

legion_entity dependent_partition::load_non_cell(int num_entities, int entity_id)
{
  printf("Load non_cells, num_entities %d\n", num_entities);
  
  Legion::Runtime *runtime = Legion::Runtime::get_runtime();
  Legion::Context ctx = Legion::Runtime::get_context();
  context_t & context_ = context_t::instance();
  
  // **************************************************************************
	// Create vertex index space, field space and logical region
  
  int id_fid, color_fid, offset_fid;
  if(entity_id == 1) {
    id_fid = FID_VERTEX_ID;
    color_fid = FID_VERTEX_PARTITION_COLOR;
    offset_fid = FID_VERTEX_OFFSET;
  } else if (entity_id == 2) {
    
  } else {
    assert(0);
  }
  
	LegionRuntime::Arrays::Rect<1> vertex_bound(LegionRuntime::Arrays::Point<1>(0),
																						  LegionRuntime::Arrays::Point<1>(num_entities-1));
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
  
  return entity_region;
}

legion_adjacency dependent_partition::load_cell_to_others(legion_entity &cell_region, legion_entity &entity_region)
{
  Legion::Runtime *runtime = Legion::Runtime::get_runtime();
  Legion::Context ctx = Legion::Runtime::get_context();
  context_t & context_ = context_t::instance();
  
  int cell_to_others_id_fid, cell_to_others_ptr_fid, image_nrange_fid, image_fid;
  if(entity_region.id == 1) {
    cell_to_others_id_fid = FID_CELL_TO_VERTEX_ID;
    cell_to_others_ptr_fid = FID_CELL_TO_VERTEX_PTR;
    image_nrange_fid = FID_CELL_VERTEX_NRANGE;
    image_fid = FID_CELL_TO_VERTEX_PTR;
  } else if (entity_region.id == 2) {
    
  } else {
    assert(0);
  }
  
 // **************************************************************************
	// Get the count of cell to vertex connectivity from init_mesh_task and add them together
	int total_cell_to_others_count = 0;
	// a array to store cell2cell and cell2vertex, first num_color-1 is cell2cell, second one is cell2vertex
	int *cell_to_others_count_per_subspace_scan = (int*)malloc(sizeof(int) * (num_color+1));
	cell_to_others_count_per_subspace_scan[0] = 0;
	for (int j = 0; j < num_color; j++) {
		init_mesh_task_rt_t rt_value = cell_region.task_fm.get_result<init_mesh_task_rt_t>(j);
		total_cell_to_others_count += rt_value.cell_to_vertex_count;
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
  printf("[TOP] cell_to_others_count_per_subspace_scan: ");
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
  
	Legion::ArgumentMap arg_map_init_cell_to_others;
  Legion::IndexLauncher init_cell_to_others_launcher(init_cell_to_others_task_id,
      																					partition_index_space, 
																								Legion::TaskArgument(cell_to_others_count_per_subspace_scan, sizeof(int)*(num_color+1)),
      																					arg_map_init_cell_to_others);
	
	init_cell_to_others_launcher.add_region_requirement(
	  Legion::RegionRequirement(cell_region.equal_lp, 0/*projection ID*/,
	                            READ_WRITE, EXCLUSIVE, cell_region.logical_region));
  init_cell_to_others_launcher.region_requirements[0].add_field(FID_CELL_ID);
	init_cell_to_others_launcher.region_requirements[0].add_field(FID_CELL_VERTEX_NRANGE);
	
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
  
  return cell_to_others_adjacency;
	
}

void dependent_partition::min_reduction_color(legion_entity &entity, legion_partition &alias_partition)
{
  Legion::Runtime *runtime = Legion::Runtime::get_runtime();
  Legion::Context ctx = Legion::Runtime::get_context();
  context_t & context_ = context_t::instance();
  
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
  init_vertex_color_launcher.region_requirements[0].add_field(FID_VERTEX_PARTITION_COLOR);

  init_vertex_color_launcher.tag = MAPPER_FORCE_RANK_MATCH;
  Legion::FutureMap fm_epoch3 = runtime->execute_index_space(ctx, init_vertex_color_launcher);
  fm_epoch3.wait_all_results(true);
}

legion_partition dependent_partition::partition_by_color(legion_entity &entity)
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

legion_partition dependent_partition::partition_by_image(legion_entity &from_entity, legion_entity &to_entity, legion_adjacency &adjacency, legion_partition &from)
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

legion_partition dependent_partition::partition_by_difference(legion_entity &entity, legion_partition & par1, legion_partition &par2)
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

legion_partition dependent_partition::partition_by_intersection(legion_entity &entity, legion_partition & par1, legion_partition &par2)
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


void dependent_partition::set_offset(legion_entity &entity, legion_partition &primary)
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


void dependent_partition::output_partition(legion_entity &entity, legion_partition &primary, legion_partition &ghost, legion_partition &shared, legion_partition &exclusive)
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

void
dependent_partition_tlt_init(
  Legion::Context ctx,
  Legion::Runtime * runtime,
  context_t & context_
)    
{
  printf("start DP\n");
	flecsi::io::simple_definition_t sd("simple2d-8x8.msh");
  
	int num_cells = sd.num_entities(1);
	int num_vertices = sd.num_entities(0);
	printf("num_cells %d, num_vertex %d\n", num_cells, num_vertices);
	
  // **************************************************************************
	// Create cell index space, field space and logical region
	LegionRuntime::Arrays::Rect<1> cell_bound(LegionRuntime::Arrays::Point<1>(0),
																						LegionRuntime::Arrays::Point<1>(num_cells-1));
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
	cell_allocator.allocate_field(sizeof(LegionRuntime::Arrays::Rect<1>), FID_CELL_VERTEX_NRANGE);
	runtime->attach_name(cell_field_space, FID_CELL_VERTEX_NRANGE, "FID_CELL_VERTEX_NRANGE");
	Legion::LogicalRegion cell_lr = runtime->create_logical_region(ctx,cell_index_space,cell_field_space);
  cell_allocator.allocate_field(sizeof(int), FID_CELL_OFFSET);
  runtime->attach_name(cell_field_space, FID_CELL_OFFSET, "FID_CELL_OFFSET");
	runtime->attach_name(cell_lr, "cells_lr");
	
  // **************************************************************************
	// Create vertex index space, field space and logical region
	LegionRuntime::Arrays::Rect<1> vertex_bound(LegionRuntime::Arrays::Point<1>(0),
																						  LegionRuntime::Arrays::Point<1>(num_vertices-1));
	Legion::Domain vertex_dom(Legion::Domain::from_rect<1>(vertex_bound));
	Legion::IndexSpace vertex_index_space = runtime->create_index_space(ctx, vertex_dom);
	Legion::FieldSpace vertex_field_space = runtime->create_field_space(ctx);
	Legion::FieldAllocator vertex_allocator = runtime->create_field_allocator(ctx, vertex_field_space);
	vertex_allocator.allocate_field(sizeof(int), FID_VERTEX_ID);
	runtime->attach_name(vertex_field_space, FID_VERTEX_ID, "FID_VERTEX_ID");
	vertex_allocator.allocate_field(sizeof(LegionRuntime::Arrays::Point<1>), FID_VERTEX_PARTITION_COLOR);
	runtime->attach_name(vertex_field_space, FID_VERTEX_PARTITION_COLOR, "FID_VERTEX_PARTITION_COLOR");
	vertex_allocator.allocate_field(sizeof(double), FID_VERTEX_PARTITION_COLOR_ID);
	runtime->attach_name(vertex_field_space, FID_VERTEX_PARTITION_COLOR_ID, "FID_VERTEX_PARTITION_COLOR_ID");
  vertex_allocator.allocate_field(sizeof(int), FID_VERTEX_OFFSET);
  runtime->attach_name(vertex_field_space, FID_VERTEX_OFFSET, "FID_VERTEX_OFFSET");
	Legion::LogicalRegion vertex_lr = runtime->create_logical_region(ctx,vertex_index_space,vertex_field_space);
	runtime->attach_name(vertex_lr, "vertex_lr");
	
  // **************************************************************************
  // Create color domain the same size of MPI comm size
	int num_color;
	MPI_Comm_size(MPI_COMM_WORLD, &num_color);
  
	LegionRuntime::Arrays::Rect<1> color_bound(LegionRuntime::Arrays::Point<1>(0),
																						 LegionRuntime::Arrays::Point<1>(num_color-1));
	Legion::Domain color_domain(Legion::Domain::from_rect<1>(color_bound));
	Legion::IndexSpace partition_is = runtime->create_index_space(ctx, color_domain);
	
  // **************************************************************************
	// Create equal partition and launch index task to init cell and vertex
  // "Equal partition" for cell is not Legion's create_equal_partition because Legion's 
  // "equal partition" strategy is different from Flecsi make_dcrs, we rely on
  // make_dcrs to create cell to cell connectivity, so here we use
  // the strategy of make_dcrs
	int *cell_count_per_subspace_scan = (int*)malloc(sizeof(int) * (num_color+1));
	int total_cell_count = 0;
	int j;
	int quot = num_cells / num_color;
	int rem = num_cells % num_color;
	int cell_count_per_color = 0;	
	cell_count_per_subspace_scan[0] = 0;
  printf("[TOP] num_cells_per_rank: ");
	for (j = 0; j < num_color; j++) {
		cell_count_per_color = quot + ((j >= (num_color - rem)) ? 1 : 0);
		printf("%d, ", j, cell_count_per_color);
		total_cell_count += cell_count_per_color;
		cell_count_per_subspace_scan[j+1] = total_cell_count;
	}
  printf("\n");
	
  printf("[TOP] cell_count_per_subspace_scan: ");
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
	// We use Legion create_equal_partition for vertex because we do not use Flecsi function to
  // create cell to vertex connectivity. 
  Legion::IndexPartition vertex_equal_ip = 
	  runtime->create_equal_partition(ctx, vertex_lr.get_index_space(), partition_is);
	Legion::LogicalPartition vertex_equal_lp = runtime->get_logical_partition(ctx, vertex_lr, vertex_equal_ip);

  // **************************************************************************
  // Launch index task to init cell and vertex
  const auto init_mesh_task_id =
    context_.task_id<__flecsi_internal_task_key(init_mesh_task)>();
  
  Legion::IndexLauncher init_mesh_launcher(init_mesh_task_id,
      																		 partition_is, Legion::TaskArgument(nullptr, 0),
      																		 Legion::ArgumentMap());
	
	init_mesh_launcher.add_region_requirement(
	  Legion::RegionRequirement(cell_equal_lp, 0/*projection ID*/,
	                            READ_WRITE, EXCLUSIVE, cell_lr));
  init_mesh_launcher.region_requirements[0].add_field(FID_CELL_ID);
  init_mesh_launcher.region_requirements[0].add_field(FID_CELL_PARTITION_COLOR);
  init_mesh_launcher.region_requirements[0].add_field(FID_CELL_CELL_NRANGE);
	
	init_mesh_launcher.add_region_requirement(
	  Legion::RegionRequirement(vertex_equal_lp, 0/*projection ID*/,
	                            READ_WRITE, EXCLUSIVE, vertex_lr));
  init_mesh_launcher.region_requirements[1].add_field(FID_VERTEX_ID);
  init_mesh_launcher.region_requirements[1].add_field(FID_VERTEX_PARTITION_COLOR_ID);
	init_mesh_launcher.region_requirements[1].add_field(FID_VERTEX_PARTITION_COLOR);
			
 // Legion::MustEpochLauncher must_epoch_launcher_init_mesh;
//  must_epoch_launcher_init_mesh.add_index_task(init_mesh_launcher);
  //Legion::FutureMap fm_epoch1 = runtime->execute_must_epoch(ctx, must_epoch_launcher_init_mesh);
  init_mesh_launcher.tag = MAPPER_FORCE_RANK_MATCH;
  Legion::FutureMap fm_epoch1 = runtime->execute_index_space(ctx, init_mesh_launcher);
  fm_epoch1.wait_all_results(true);
	
  // **************************************************************************
	// Get the count of cell to cell connectivity from init_mesh_task and add them together
	int total_cell_to_cell_count = 0;
	int total_cell_to_vertex_count = 0;
	// a array to store cell2cell and cell2vertex, first num_color-1 is cell2cell, second one is cell2vertex
	int *cell_to_cell_vertex_count_per_subspace_scan = (int*)malloc(sizeof(int) * (num_color+1) * 2);
	int *cell_to_cell_count_per_subspace_scan = cell_to_cell_vertex_count_per_subspace_scan;
	int *cell_to_vertex_count_per_subspace_scan = &(cell_to_cell_vertex_count_per_subspace_scan[num_color+1]);
	cell_to_cell_count_per_subspace_scan[0] = 0;
	cell_to_vertex_count_per_subspace_scan[0] = 0;
	for (j = 0; j < num_color; j++) {
		init_mesh_task_rt_t rt_value = fm_epoch1.get_result<init_mesh_task_rt_t>(j);
		total_cell_to_cell_count += rt_value.cell_to_cell_count;
		cell_to_cell_count_per_subspace_scan[j+1] = total_cell_to_cell_count;
		total_cell_to_vertex_count += rt_value.cell_to_vertex_count;
		cell_to_vertex_count_per_subspace_scan[j+1] = total_cell_to_vertex_count;
	}
	printf("[TOP] total cell to cell count %d\n", total_cell_to_cell_count);
	
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
	
	// create cell to vertex connectivity index space, field space and logical region with the total_cell_to_vertex_count
	LegionRuntime::Arrays::Rect<1> cell_to_vertex_bound(LegionRuntime::Arrays::Point<1>(0),
																								  		LegionRuntime::Arrays::Point<1>(total_cell_to_vertex_count-1));
	Legion::Domain cell_to_vertex_dom(Legion::Domain::from_rect<1>(cell_to_vertex_bound));
	Legion::IndexSpace cell_to_vertex_index_space = runtime->create_index_space(ctx, cell_to_vertex_dom);
	runtime->attach_name(cell_to_vertex_index_space, "cell_to_vertex_index_space");
	Legion::FieldSpace cell_to_vertex_field_space = runtime->create_field_space(ctx);
	Legion::FieldAllocator cell_to_vertex_allocator = runtime->create_field_allocator(ctx, cell_to_vertex_field_space);
	cell_to_vertex_allocator.allocate_field(sizeof(int), FID_CELL_TO_VERTEX_ID);
	runtime->attach_name(cell_to_vertex_field_space, FID_CELL_TO_VERTEX_ID, "FID_CELL_TO_VERTEX_ID");
	cell_to_vertex_allocator.allocate_field(sizeof(LegionRuntime::Arrays::Point<1>), FID_CELL_TO_VERTEX_PTR);
	runtime->attach_name(cell_to_vertex_field_space, FID_CELL_TO_VERTEX_PTR, "FID_CELL_TO_VERTEX_PTR");
	Legion::LogicalRegion cell_to_vertex_lr = runtime->create_logical_region(ctx,cell_to_vertex_index_space,cell_to_vertex_field_space);
	runtime->attach_name(cell_to_vertex_lr, "cell_to_vertex_lr");
	
	// Partition the cell to cell connectivity following the pattern of cell_to_cell_count_per_subspace_scan.
	// For example, the cell_to_cell_count_per_subspace_scan is 40,40,50,50, then the partition is also 40,40,50,50
  printf("[TOP] cell_to_cell_count_per_subspace_scan: ");
	Legion::DomainColoring cell_to_cell_color_partitioning;
  for(j = 0; j < num_color; j++){
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
	
	// Partition the cell to vertex connectivity following the pattern of cell_to_vertex_count_per_subspace_scan.
	// For example, the cell_to_vertex_count_per subspace_scan is 40,40,50,50, then the partition is also 40,40,50,50
  printf("[TOP] cell_to_vertex_count_per_subspace_scan: ");
	Legion::DomainColoring cell_to_vertex_color_partitioning;
  for(j = 0; j < num_color; j++){
    LegionRuntime::Arrays::Rect<1> subrect(
      LegionRuntime::Arrays::Point<1>(cell_to_vertex_count_per_subspace_scan[j]), 
			LegionRuntime::Arrays::Point<1>(cell_to_vertex_count_per_subspace_scan[j+1]-1));
    cell_to_vertex_color_partitioning[j] = Legion::Domain::from_rect<1>(subrect);
		printf("(%d, %d), ", cell_to_vertex_count_per_subspace_scan[j], cell_to_vertex_count_per_subspace_scan[j+1]-1);
  }
  printf("\n");
	
	Legion::IndexPartition cell_to_vertex_ip = 
    runtime->create_index_partition(ctx, cell_to_vertex_lr.get_index_space(), 
                                    color_domain, cell_to_vertex_color_partitioning, true /*disjoint*/);
	Legion::LogicalPartition cell_to_vertex_lp = runtime->get_logical_partition(ctx, cell_to_vertex_lr, cell_to_vertex_ip);
	
  // **************************************************************************
	// Launch index task to init cell to cell and cell to vertex connectivity
  const auto init_adjacency_task_id =
    context_.task_id<__flecsi_internal_task_key(init_adjacency_task)>();
  
	Legion::ArgumentMap arg_map_init_adjacency;
  Legion::IndexLauncher init_adjacency_launcher(init_adjacency_task_id,
      																					partition_is, 
																								Legion::TaskArgument(cell_to_cell_vertex_count_per_subspace_scan, sizeof(int)*(num_color+1)*2),
      																					arg_map_init_adjacency);
	
	init_adjacency_launcher.add_region_requirement(
	  Legion::RegionRequirement(cell_equal_lp, 0/*projection ID*/,
	                            READ_WRITE, EXCLUSIVE, cell_lr));
  init_adjacency_launcher.region_requirements[0].add_field(FID_CELL_ID);
  init_adjacency_launcher.region_requirements[0].add_field(FID_CELL_CELL_NRANGE);
	init_adjacency_launcher.region_requirements[0].add_field(FID_CELL_VERTEX_NRANGE);
	
	init_adjacency_launcher.add_region_requirement(
	  Legion::RegionRequirement(cell_to_cell_lp, 0/*projection ID*/,
	                            READ_WRITE, EXCLUSIVE, cell_to_cell_lr));
	init_adjacency_launcher.region_requirements[1].add_field(FID_CELL_TO_CELL_ID);
	init_adjacency_launcher.region_requirements[1].add_field(FID_CELL_TO_CELL_PTR);
	
	init_adjacency_launcher.add_region_requirement(
	  Legion::RegionRequirement(cell_to_vertex_lp, 0/*projection ID*/,
	                            READ_WRITE, EXCLUSIVE, cell_to_vertex_lr));
	init_adjacency_launcher.region_requirements[2].add_field(FID_CELL_TO_VERTEX_ID);
	init_adjacency_launcher.region_requirements[2].add_field(FID_CELL_TO_VERTEX_PTR);
			
  //Legion::MustEpochLauncher must_epoch_launcher_init_adjacency;
  //must_epoch_launcher_init_adjacency.add_index_task(init_adjacency_launcher);
  //Legion::FutureMap fm_epoch2 = runtime->execute_must_epoch(ctx, must_epoch_launcher_init_adjacency);
  init_adjacency_launcher.tag = MAPPER_FORCE_RANK_MATCH;
  Legion::FutureMap fm_epoch2 = runtime->execute_index_space(ctx, init_adjacency_launcher);
  fm_epoch2.wait_all_results(true);
	
  // **************************************************************************
	// Partition cell by color
	Legion::IndexPartition cell_primary_ip = 
		runtime->create_partition_by_field(ctx, cell_lr,
	                            				 cell_lr, FID_CELL_PARTITION_COLOR, partition_is);
  runtime->attach_name(cell_primary_ip, "cell_primary_ip");
  
	// Ghost cell
	Legion::IndexPartition cell_primary_image_nrange_ip = 
		runtime->create_partition_by_image_range(ctx, cell_to_cell_lr.get_index_space(),
	                                           runtime->get_logical_partition(cell_lr, cell_primary_ip), 
	                                           cell_lr,
	                                           FID_CELL_CELL_NRANGE,
	                                           partition_is);

	Legion::IndexPartition cell_reachable_ip = 
		runtime->create_partition_by_image(ctx, cell_lr.get_index_space(),
                                       runtime->get_logical_partition(cell_to_cell_lr, cell_primary_image_nrange_ip), 
                                       cell_to_cell_lr,
                                       FID_CELL_TO_CELL_PTR,
                                       partition_is);

	Legion::IndexPartition cell_ghost_ip = 
		runtime->create_partition_by_difference(ctx, cell_lr.get_index_space(),
														                cell_reachable_ip, cell_primary_ip, partition_is);
	runtime->attach_name(cell_ghost_ip, "cell_ghost_ip");
  // Shared cell	
#if 1	
	Legion::IndexPartition cell_ghost_preimage_ip = 
		runtime->create_partition_by_image_range(ctx, cell_to_cell_lr.get_index_space(),
	                                           runtime->get_logical_partition(cell_lr, cell_ghost_ip), 
	                                           cell_lr,
	                                           FID_CELL_CELL_NRANGE,
	                                           partition_is);

  Legion::IndexPartition cell_ghost_preimage_preimage_nrange_ip = 
		runtime->create_partition_by_image(ctx, cell_lr.get_index_space(),
                                       runtime->get_logical_partition(cell_to_cell_lr, cell_ghost_preimage_ip), 
                                       cell_to_cell_lr,
                                       FID_CELL_TO_CELL_PTR,
                                       partition_is); 
#else
	Legion::IndexPartition cell_ghost_preimage_ip = 
		runtime->create_partition_by_preimage(ctx, cell_ghost_ip , 
                                          cell_to_cell_lr, cell_to_cell_lr,
                                          FID_CELL_TO_CELL_PTR,
                                          partition_is);

  Legion::IndexPartition cell_ghost_preimage_preimage_nrange_ip = 
		runtime->create_partition_by_preimage_range(ctx, cell_ghost_preimage_ip , 
                                                cell_lr, cell_lr,
                                                FID_CELL_CELL_NRANGE,
                                                partition_is);																																												
#endif
	Legion::IndexPartition cell_shared_ip = 
		runtime->create_partition_by_intersection(ctx, cell_lr.get_index_space(),
	                                            cell_ghost_preimage_preimage_nrange_ip, cell_primary_ip, partition_is);  
	runtime->attach_name(cell_shared_ip, "cell_shared_ip");
  
	// Execlusive cell
	Legion::IndexPartition cell_execlusive_ip = 
		runtime->create_partition_by_difference(ctx, cell_lr.get_index_space(),
	                                          cell_primary_ip, cell_shared_ip, partition_is); 
	runtime->attach_name(cell_execlusive_ip, "cell_execlusive_ip");
	
	// Non-disjoint vertex
	Legion::IndexPartition vertex_alias_image_nrange_ip = 
		runtime->create_partition_by_image_range(ctx, cell_to_vertex_lr.get_index_space(),
	                                           runtime->get_logical_partition(cell_lr, cell_primary_ip), 
	                                           cell_lr,
	                                           FID_CELL_VERTEX_NRANGE,
	                                           partition_is);

	Legion::IndexPartition vertex_alias_ip = 
		runtime->create_partition_by_image(ctx, vertex_lr.get_index_space(),
                                       runtime->get_logical_partition(cell_to_vertex_lr, vertex_alias_image_nrange_ip), 
                                       cell_to_vertex_lr,
                                       FID_CELL_TO_VERTEX_PTR,
                                       partition_is);
	
	// Get logical region
	Legion::LogicalPartition cell_primary_lp = runtime->get_logical_partition(ctx, cell_lr, cell_primary_ip);
  runtime->attach_name(cell_primary_lp, "cell_primary_lp");
	Legion::LogicalPartition cell_ghost_lp = runtime->get_logical_partition(ctx, cell_lr, cell_ghost_ip);
  runtime->attach_name(cell_ghost_lp, "cell_ghost_lp");
	Legion::LogicalPartition cell_shared_lp = runtime->get_logical_partition(ctx, cell_lr, cell_shared_ip);
  runtime->attach_name(cell_shared_lp, "cell_shared_lp");
	Legion::LogicalPartition cell_execlusive_lp = runtime->get_logical_partition(ctx, cell_lr, cell_execlusive_ip);
  runtime->attach_name(cell_execlusive_lp, "cell_execlusive_lp");
	Legion::LogicalPartition vertex_alias_lp = runtime->get_logical_partition(ctx, vertex_lr, vertex_alias_ip);
  runtime->attach_name(vertex_alias_lp, "vertex_alias_lp");
  
  // **************************************************************************
  // Launch index task to init vertex color
  const auto init_vertex_color_task_id =
    context_.task_id<__flecsi_internal_task_key(init_vertex_color_task)>();
  
  Legion::IndexLauncher init_vertex_color_launcher(init_vertex_color_task_id,
    																			 partition_is, Legion::TaskArgument(nullptr, 0),
      																		 Legion::ArgumentMap());
	
	init_vertex_color_launcher.add_region_requirement(
	  Legion::RegionRequirement(vertex_alias_lp, 0/*projection ID*/,
	                            MinReductionPointOp::redop_id, SIMULTANEOUS, vertex_lr));
  init_vertex_color_launcher.region_requirements[0].add_field(FID_VERTEX_PARTITION_COLOR);

  //Legion::MustEpochLauncher must_epoch_launcher_init_vertex_color;
  //must_epoch_launcher_init_vertex_color.add_index_task(init_vertex_color_launcher);
  //Legion::FutureMap fm_epoch3 = runtime->execute_must_epoch(ctx, must_epoch_launcher_init_vertex_color);
  init_vertex_color_launcher.tag = MAPPER_FORCE_RANK_MATCH;
  Legion::FutureMap fm_epoch3 = runtime->execute_index_space(ctx, init_vertex_color_launcher);
  fm_epoch3.wait_all_results(true);

#if 0	
	{
	  const auto verify_vertex_color_task_id =
	    context_.task_id<__flecsi_internal_task_key(verify_vertex_color_task)>();
  
	  Legion::IndexLauncher verify_vertex_color_launcher(verify_vertex_color_task_id,
	    																			 partition_is, Legion::TaskArgument(nullptr, 0),
	      																		 Legion::ArgumentMap());
	
		verify_vertex_color_launcher.add_region_requirement(
		  Legion::RegionRequirement(vertex_alias_lp, 0/*projection ID*/,
		                            READ_ONLY, EXCLUSIVE, vertex_lr));
		verify_vertex_color_launcher.region_requirements[0].add_field(FID_VERTEX_ID);
	  verify_vertex_color_launcher.region_requirements[0].add_field(FID_VERTEX_PARTITION_COLOR);

	  //Legion::MustEpochLauncher must_epoch_launcher_verify_vertex_color;
	 // must_epoch_launcher_verify_vertex_color.add_index_task(verify_vertex_color_launcher);
	 // Legion::FutureMap fm_epoch_test1 = runtime->execute_must_epoch(ctx, must_epoch_launcher_verify_vertex_color);
    verify_vertex_color_launcher.tag = MAPPER_FORCE_RANK_MATCH;
	  Legion::FutureMap fm_epoch_test1 = runtime->execute_index_space(ctx, verify_vertex_color_launcher);
    fm_epoch_test1.wait_all_results(true);
	}
#endif
	
  // **************************************************************************
	// Primary vertex
	Legion::IndexPartition vertex_primary_ip = 
		runtime->create_partition_by_field(ctx, vertex_lr,
                                       vertex_lr,
                                       FID_VERTEX_PARTITION_COLOR,
                                       partition_is);
	runtime->attach_name(vertex_primary_ip, "vertex_primary_ip");
  
	// Ghost vertex
	 Legion::IndexPartition cell_ghost_c2v_image_nrange_ip = 
		 runtime->create_partition_by_image_range(ctx, cell_to_vertex_lr.get_index_space(),
                                              runtime->get_logical_partition(cell_lr, cell_ghost_ip), 
                                              cell_lr,
                                              FID_CELL_VERTEX_NRANGE,
                                              partition_is);																																	 
	
	 Legion::IndexPartition vertex_of_ghost_cell_ip = 
		 runtime->create_partition_by_image(ctx, vertex_lr.get_index_space(),
                                        runtime->get_logical_partition(cell_to_vertex_lr, cell_ghost_c2v_image_nrange_ip), 
                                        cell_to_vertex_lr,
                                        FID_CELL_TO_VERTEX_PTR,
                                        partition_is);
	Legion::IndexPartition vertex_ghost_ip = 
		runtime->create_partition_by_difference(ctx, vertex_lr.get_index_space(),
	                                          vertex_of_ghost_cell_ip, vertex_primary_ip, partition_is); 
  runtime->attach_name(vertex_ghost_ip, "vertex_ghost_ip");
  
	// Shared vertex
	Legion::IndexPartition cell_shared_c2v_image_nrange_ip = 
		runtime->create_partition_by_image_range(ctx, cell_to_vertex_lr.get_index_space(),
	                                           runtime->get_logical_partition(cell_lr, cell_shared_ip), 
                                             cell_lr,
                                             FID_CELL_VERTEX_NRANGE,
                                             partition_is);

	Legion::IndexPartition vertex_of_shared_cell_ip = 
		runtime->create_partition_by_image(ctx, vertex_lr.get_index_space(),
                                       runtime->get_logical_partition(cell_to_vertex_lr, cell_shared_c2v_image_nrange_ip), 
                                       cell_to_vertex_lr,
                                       FID_CELL_TO_VERTEX_PTR,
                                       partition_is);


	Legion::IndexPartition vertex_shared_ip = runtime->create_partition_by_intersection(ctx, vertex_lr.get_index_space(),
	                                                           vertex_of_shared_cell_ip, vertex_primary_ip, partition_is); 
	runtime->attach_name(vertex_shared_ip, "vertex_shared_ip");
	
	// Exclusive vertex
	Legion::IndexPartition vertex_exclusive_ip = runtime->create_partition_by_difference(ctx, vertex_lr.get_index_space(),
	                                                           vertex_primary_ip, vertex_shared_ip, partition_is); 
	runtime->attach_name(vertex_exclusive_ip, "vertex_exclusive_ip");
	
	// Get logical region
	Legion::LogicalPartition vertex_primary_lp = runtime->get_logical_partition(ctx, vertex_lr, vertex_primary_ip);
  runtime->attach_name(vertex_primary_lp, "vertex_primary_lp");
  Legion::LogicalPartition vertex_ghost_lp = runtime->get_logical_partition(ctx, vertex_lr, vertex_ghost_ip);
  runtime->attach_name(vertex_ghost_lp, "vertex_ghost_lp");
  Legion::LogicalPartition vertex_shared_lp = runtime->get_logical_partition(ctx, vertex_lr, vertex_shared_ip);
  runtime->attach_name(vertex_shared_lp, "vertex_shared_lp");
  Legion::LogicalPartition vertex_exclusive_lp = runtime->get_logical_partition(ctx, vertex_lr, vertex_exclusive_ip);
  runtime->attach_name(vertex_exclusive_lp, "vertex_exclusive_lp");
	
	//test
  Legion::LogicalPartition vertex_of_ghost_cell_lp = runtime->get_logical_partition(ctx, vertex_lr, vertex_of_ghost_cell_ip);
  runtime->attach_name(vertex_of_ghost_cell_lp, "vertex_of_ghost_cell_lp");
	
  Legion::LogicalPartition cell_reachable_lp = runtime->get_logical_partition(ctx, cell_lr, cell_reachable_ip);
  runtime->attach_name(cell_reachable_lp, "cell_reachable_lp");
	
  Legion::LogicalPartition cell_primary_image_nrange_lp = runtime->get_logical_partition(ctx, cell_to_cell_lr, cell_primary_image_nrange_ip);
  runtime->attach_name(cell_primary_image_nrange_lp, "cell_primary_image_nrange_lp");
  
  // All shared cell and vertex
#if 0
	LegionRuntime::Arrays::Rect<1> pending_partition_bound(LegionRuntime::Arrays::Point<1>(0),
																						LegionRuntime::Arrays::Point<1>(0));
	Legion::Domain pending_partition_dom(Legion::Domain::from_rect<1>(pending_partition_bound));
	Legion::IndexSpace pending_partition_color_space = runtime->create_index_space(ctx, pending_partition_dom);
  Legion::DomainPoint pending_color_point(0);
  Legion::IndexPartition cell_shared_pending_ip = runtime->create_pending_partition(ctx, cell_index_space, pending_partition_color_space);
  Legion::IndexSpace cell_all_shared_is = runtime->create_index_space_union(ctx, cell_shared_pending_ip, pending_color_point, cell_shared_ip);
	//Legion::LogicalRegion cell_all_shared_lr = runtime->get_logical_subregion(ctx, runtime->get_logical_partition(ctx, cell_lr, cell_shared_pending_ip), cell_all_shared_is);
  Legion::LogicalRegion cell_all_shared_lr = runtime->get_logical_subregion_by_tree(ctx,cell_all_shared_is,cell_field_space, cell_lr.get_tree_id());
	runtime->attach_name(cell_all_shared_lr, "cell_all_shared_lr");
  
  Legion::IndexPartition vertex_shared_pending_ip = runtime->create_pending_partition(ctx, vertex_index_space, pending_partition_color_space);
  Legion::IndexSpace vertex_all_shared_is = runtime->create_index_space_union(ctx, vertex_shared_pending_ip, pending_color_point, vertex_shared_ip);
	//Legion::LogicalRegion vertex_all_shared_lr = runtime->get_logical_subregion(ctx, runtime->get_logical_partition(ctx, vertex_lr, vertex_shared_pending_ip), vertex_all_shared_is);
  Legion::LogicalRegion vertex_all_shared_lr = runtime->get_logical_subregion_by_tree(ctx, vertex_all_shared_is, vertex_field_space, vertex_lr.get_tree_id());
	runtime->attach_name(vertex_all_shared_lr, "vertex_all_shared_lr");
#endif  
  // **************************************************************************
	// Launch index task to init offset
  const auto init_entity_offset_task_id =
    context_.task_id<__flecsi_internal_task_key(init_entity_offset_task)>();
  
  Legion::IndexLauncher init_entity_offset_launcher(init_entity_offset_task_id,
    																			 partition_is, Legion::TaskArgument(nullptr, 0),
      																		 Legion::ArgumentMap());
	
	init_entity_offset_launcher.add_region_requirement(
	  Legion::RegionRequirement(cell_primary_lp, 0/*projection ID*/,
	                            READ_WRITE, EXCLUSIVE, cell_lr));
  init_entity_offset_launcher.region_requirements[0].add_field(FID_CELL_ID);
  init_entity_offset_launcher.region_requirements[0].add_field(FID_CELL_OFFSET);
  
	init_entity_offset_launcher.add_region_requirement(
	  Legion::RegionRequirement(vertex_primary_lp, 0/*projection ID*/,
	                            READ_WRITE, EXCLUSIVE, vertex_lr));
  init_entity_offset_launcher.region_requirements[1].add_field(FID_VERTEX_ID);
  init_entity_offset_launcher.region_requirements[1].add_field(FID_VERTEX_OFFSET);
	
  //Legion::MustEpochLauncher must_epoch_launcher_init_entity_offset;
  //must_epoch_launcher_init_entity_offset.add_index_task(init_entity_offset_launcher);
  //Legion::FutureMap fm_epoch_init_entity_offset = runtime->execute_must_epoch(ctx, must_epoch_launcher_init_entity_offset);
  init_entity_offset_launcher.tag = MAPPER_FORCE_RANK_MATCH;
  Legion::FutureMap fm_epoch_init_entity_offset = runtime->execute_index_space(ctx, init_entity_offset_launcher);
  fm_epoch_init_entity_offset.wait_all_results(true);

  // **************************************************************************
	// Launch index task to verify partition results
  const auto verify_dp_task_id =
    context_.task_id<__flecsi_internal_task_key(verify_dp_task)>();
  
  Legion::IndexLauncher verify_dp_launcher(verify_dp_task_id,
    																			 partition_is, Legion::TaskArgument(nullptr, 0),
      																		 Legion::ArgumentMap());
	
	verify_dp_launcher.add_region_requirement(
	  Legion::RegionRequirement(cell_primary_lp, 0/*projection ID*/,
	                            READ_ONLY, EXCLUSIVE, cell_lr));
  verify_dp_launcher.region_requirements[0].add_field(FID_CELL_ID);
  verify_dp_launcher.region_requirements[0].add_field(FID_CELL_PARTITION_COLOR);
	
	verify_dp_launcher.add_region_requirement(
	  Legion::RegionRequirement(cell_ghost_lp, 0/*projection ID*/,
	                            READ_ONLY, EXCLUSIVE, cell_lr));
  verify_dp_launcher.region_requirements[1].add_field(FID_CELL_ID);
  verify_dp_launcher.region_requirements[1].add_field(FID_CELL_OFFSET);
  verify_dp_launcher.region_requirements[1].add_field(FID_CELL_PARTITION_COLOR);
	
	verify_dp_launcher.add_region_requirement(
	  Legion::RegionRequirement(cell_shared_lp, 0/*projection ID*/,
	                            READ_ONLY, EXCLUSIVE, cell_lr));
  verify_dp_launcher.region_requirements[2].add_field(FID_CELL_ID);
  verify_dp_launcher.region_requirements[2].add_field(FID_CELL_OFFSET);
	
	verify_dp_launcher.add_region_requirement(
	  Legion::RegionRequirement(cell_execlusive_lp, 0/*projection ID*/,
	                            READ_ONLY, EXCLUSIVE, cell_lr));
  verify_dp_launcher.region_requirements[3].add_field(FID_CELL_ID);
  verify_dp_launcher.region_requirements[3].add_field(FID_CELL_OFFSET);
	
	verify_dp_launcher.add_region_requirement(
	  Legion::RegionRequirement(vertex_primary_lp, 0/*projection ID*/,
	                            READ_ONLY, EXCLUSIVE, vertex_lr));
  verify_dp_launcher.region_requirements[4].add_field(FID_VERTEX_ID);
	
	verify_dp_launcher.add_region_requirement(
	  Legion::RegionRequirement(vertex_ghost_lp, 0/*projection ID*/,
	                            READ_ONLY, EXCLUSIVE, vertex_lr));
  verify_dp_launcher.region_requirements[5].add_field(FID_VERTEX_ID);
  verify_dp_launcher.region_requirements[5].add_field(FID_VERTEX_OFFSET);
  verify_dp_launcher.region_requirements[5].add_field(FID_VERTEX_PARTITION_COLOR);
	
	verify_dp_launcher.add_region_requirement(
	  Legion::RegionRequirement(vertex_shared_lp, 0/*projection ID*/,
	                            READ_ONLY, EXCLUSIVE, vertex_lr));
  verify_dp_launcher.region_requirements[6].add_field(FID_VERTEX_ID);
  verify_dp_launcher.region_requirements[6].add_field(FID_VERTEX_OFFSET);
	
	verify_dp_launcher.add_region_requirement(
	  Legion::RegionRequirement(vertex_exclusive_lp, 0/*projection ID*/,
	                            READ_ONLY, EXCLUSIVE, vertex_lr));
  verify_dp_launcher.region_requirements[7].add_field(FID_VERTEX_ID);
  verify_dp_launcher.region_requirements[7].add_field(FID_VERTEX_OFFSET);
	
	verify_dp_launcher.add_region_requirement(
	  Legion::RegionRequirement(vertex_of_ghost_cell_lp, 0/*projection ID*/,
	                            READ_ONLY, EXCLUSIVE, vertex_lr));
  verify_dp_launcher.region_requirements[8].add_field(FID_VERTEX_ID);
	
	verify_dp_launcher.add_region_requirement(
	  Legion::RegionRequirement(cell_reachable_lp, 0/*projection ID*/,
	                            READ_ONLY, EXCLUSIVE, cell_lr));
  verify_dp_launcher.region_requirements[9].add_field(FID_CELL_ID);
	
	verify_dp_launcher.add_region_requirement(
	  Legion::RegionRequirement(cell_primary_image_nrange_lp, 0/*projection ID*/,
	                            READ_ONLY, EXCLUSIVE, cell_to_cell_lr));
  verify_dp_launcher.region_requirements[10].add_field(FID_CELL_TO_CELL_ID);

#if 0  
	verify_dp_launcher.add_region_requirement(
	  Legion::RegionRequirement(cell_all_shared_lr, 0/*projection ID*/,
	                            READ_ONLY, EXCLUSIVE, cell_lr));
  verify_dp_launcher.region_requirements[11].add_field(FID_CELL_ID);
  verify_dp_launcher.region_requirements[11].add_field(FID_CELL_PARTITION_COLOR);
  
	verify_dp_launcher.add_region_requirement(
	  Legion::RegionRequirement(vertex_all_shared_lr, 0/*projection ID*/,
	                            READ_ONLY, EXCLUSIVE, vertex_lr));
  verify_dp_launcher.region_requirements[12].add_field(FID_VERTEX_ID);
  verify_dp_launcher.region_requirements[12].add_field(FID_VERTEX_PARTITION_COLOR);
#endif
  
  //Legion::MustEpochLauncher must_epoch_launcher_verify_dp;
  //must_epoch_launcher_verify_dp.add_index_task(verify_dp_launcher);
  //Legion::FutureMap fm_epoch4 = runtime->execute_must_epoch(ctx, must_epoch_launcher_verify_dp);
  verify_dp_launcher.tag = MAPPER_FORCE_RANK_MATCH;
  Legion::FutureMap fm_epoch4 = runtime->execute_index_space(ctx, verify_dp_launcher);
  fm_epoch4.wait_all_results(true);

	
	// Clean up
	free(cell_to_cell_vertex_count_per_subspace_scan);
	cell_to_cell_vertex_count_per_subspace_scan = NULL;
	free(cell_count_per_subspace_scan);
	cell_count_per_subspace_scan = NULL;
	
	runtime->destroy_logical_region(ctx, cell_lr);
	runtime->destroy_logical_region(ctx, vertex_lr);
	runtime->destroy_logical_region(ctx, cell_to_cell_lr);
  runtime->destroy_logical_region(ctx, cell_to_vertex_lr);
	runtime->destroy_field_space(ctx, cell_field_space);
	runtime->destroy_field_space(ctx, vertex_field_space);
	runtime->destroy_field_space(ctx, cell_to_cell_field_space);
	runtime->destroy_field_space(ctx, cell_to_vertex_field_space);
	runtime->destroy_index_space(ctx, cell_index_space);
	runtime->destroy_index_space(ctx, vertex_index_space);
	runtime->destroy_index_space(ctx, cell_to_cell_index_space);
	runtime->destroy_index_space(ctx, cell_to_vertex_index_space);
	runtime->destroy_index_space(ctx, partition_is);
}

} // namespace execution
} // namespace flecsi