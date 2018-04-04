/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */
/*! @file */

#include <flecsi/execution/legion/runtime_driver.h>

#include <legion.h>
#include <legion_utilities.h>
#include <limits>

#include <flecsi/data/legion/legion_data.h>
#include <flecsi/data/storage.h>
#include <flecsi/execution/context.h>
#include <flecsi/execution/legion/legion_tasks.h>
#include <flecsi/execution/legion/mapper.h>
#include <flecsi/execution/legion/internal_field.h>
#include <flecsi/runtime/types.h>
#include <flecsi/utils/common.h>
#include <flecsi/data/data_constants.h> 

#include <flecsi/io/simple_definition.h>
#include <mpi.h>
#include <unistd.h>

clog_register_tag(runtime_driver);

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
// Implementation of FleCSI runtime driver task.
//----------------------------------------------------------------------------//

void
runtime_driver(
  const Legion::Task * task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx,
  Legion::Runtime * runtime
)
{
  using namespace data;
  //using data::storage_label_type_t;

  {
  clog_tag_guard(runtime_driver);
  clog(info) << "In Legion runtime driver" << std::endl;
  }

  // Get the input arguments from the Legion runtime
  const Legion::InputArgs & args =
    Legion::Runtime::get_input_args();

  // Initialize MPI Interoperability
  context_t & context_ = context_t::instance();
  context_.connect_with_mpi(ctx, runtime);
  context_.wait_on_mpi(ctx, runtime);

  using field_info_t = context_t::field_info_t;

  //--------------------------------------------------------------------------//
  // Invoke callbacks for entries in the client registry.
  //
  // NOTE: This needs to be called before the field registry below because
  //       The client callbacks register field callbacks with the field
  //       registry.
  //--------------------------------------------------------------------------//

  auto & client_registry =
    flecsi::data::storage_t::instance().client_registry();

  for(auto & c: client_registry) {
    for(auto & d: c.second) {
      d.second.second(d.second.first);
    } // for
  } // for

  //--------------------------------------------------------------------------//
  // Invoke callbacks for entries in the field registry.
  //--------------------------------------------------------------------------//

  auto & field_registry =
    flecsi::data::storage_t::instance().field_registry();

  for(auto & c: field_registry) {
    for(auto & f: c.second) {
      f.second.second(f.first, f.second.first);
    } // for
  } // for

  int num_colors;
  MPI_Comm_size(MPI_COMM_WORLD, &num_colors);
  {
  clog_tag_guard(runtime_driver);
  clog(info) << "MPI num_colors is " << num_colors << std::endl;
  }

  data::legion_data_t data(ctx, runtime, num_colors);
  
  data.init_global_handles();

  size_t number_of_global_fields = 0;
  for(const field_info_t& field_info : context_.registered_fields()){
    context_.put_field_info(field_info);
    if (field_info.storage_class == global)
      number_of_global_fields++;
  }

  if (number_of_global_fields > 0)
  {
    auto& ispace_dmap = context_.index_space_data_map();
    size_t global_index_space =
      execution::internal_index_space::global_is;

    ispace_dmap[global_index_space].entire_region =
        data.global_index_space().logical_region;
  }

#if defined FLECSI_ENABLE_SPECIALIZATION_TLT_INIT
  {
  clog_tag_guard(runtime_driver);
  clog(info) << "Executing specialization top-level-task init" << std::endl;
  }

  // Invoke the specialization top-level task initialization function.
  specialization_tlt_init(args.argc, args.argv);

  context_.advance_state();
  
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
			
  Legion::MustEpochLauncher must_epoch_launcher_init_mesh;
  must_epoch_launcher_init_mesh.add_index_task(init_mesh_launcher);
  Legion::FutureMap fm_epoch1 = runtime->execute_must_epoch(ctx, must_epoch_launcher_init_mesh);
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
			
  Legion::MustEpochLauncher must_epoch_launcher_init_adjacency;
  must_epoch_launcher_init_adjacency.add_index_task(init_adjacency_launcher);
  Legion::FutureMap fm_epoch2 = runtime->execute_must_epoch(ctx, must_epoch_launcher_init_adjacency);
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
  //sleep(20);
  const auto init_vertex_color_task_id =
    context_.task_id<__flecsi_internal_task_key(init_vertex_color_task)>();
  
  Legion::IndexLauncher init_vertex_color_launcher(init_vertex_color_task_id,
    																			 partition_is, Legion::TaskArgument(nullptr, 0),
      																		 Legion::ArgumentMap());
	
	init_vertex_color_launcher.add_region_requirement(
	  Legion::RegionRequirement(vertex_alias_lp, 0/*projection ID*/,
	                            MinReductionPointOp::redop_id, SIMULTANEOUS, vertex_lr));
  init_vertex_color_launcher.region_requirements[0].add_field(FID_VERTEX_PARTITION_COLOR);

  Legion::MustEpochLauncher must_epoch_launcher_init_vertex_color;
  must_epoch_launcher_init_vertex_color.add_index_task(init_vertex_color_launcher);
  Legion::FutureMap fm_epoch3 = runtime->execute_must_epoch(ctx, must_epoch_launcher_init_vertex_color);
  fm_epoch3.wait_all_results(true);
	
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

	  Legion::MustEpochLauncher must_epoch_launcher_verify_vertex_color;
	  must_epoch_launcher_verify_vertex_color.add_index_task(verify_vertex_color_launcher);
	  Legion::FutureMap fm_epoch_test1 = runtime->execute_must_epoch(ctx, must_epoch_launcher_verify_vertex_color);
	  fm_epoch_test1.wait_all_results(true);
	}
	
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
	
	verify_dp_launcher.add_region_requirement(
	  Legion::RegionRequirement(cell_shared_lp, 0/*projection ID*/,
	                            READ_ONLY, EXCLUSIVE, cell_lr));
  verify_dp_launcher.region_requirements[2].add_field(FID_CELL_ID);
	
	verify_dp_launcher.add_region_requirement(
	  Legion::RegionRequirement(cell_execlusive_lp, 0/*projection ID*/,
	                            READ_ONLY, EXCLUSIVE, cell_lr));
  verify_dp_launcher.region_requirements[3].add_field(FID_CELL_ID);
	
	verify_dp_launcher.add_region_requirement(
	  Legion::RegionRequirement(vertex_primary_lp, 0/*projection ID*/,
	                            READ_ONLY, EXCLUSIVE, vertex_lr));
  verify_dp_launcher.region_requirements[4].add_field(FID_VERTEX_ID);
	
	verify_dp_launcher.add_region_requirement(
	  Legion::RegionRequirement(vertex_ghost_lp, 0/*projection ID*/,
	                            READ_ONLY, EXCLUSIVE, vertex_lr));
  verify_dp_launcher.region_requirements[5].add_field(FID_VERTEX_ID);
	
	verify_dp_launcher.add_region_requirement(
	  Legion::RegionRequirement(vertex_shared_lp, 0/*projection ID*/,
	                            READ_ONLY, EXCLUSIVE, vertex_lr));
  verify_dp_launcher.region_requirements[6].add_field(FID_VERTEX_ID);
	
	verify_dp_launcher.add_region_requirement(
	  Legion::RegionRequirement(vertex_exclusive_lp, 0/*projection ID*/,
	                            READ_ONLY, EXCLUSIVE, vertex_lr));
  verify_dp_launcher.region_requirements[7].add_field(FID_VERTEX_ID);
	
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
	
  Legion::MustEpochLauncher must_epoch_launcher_verify_dp;
  must_epoch_launcher_verify_dp.add_index_task(verify_dp_launcher);
  Legion::FutureMap fm_epoch4 = runtime->execute_must_epoch(ctx, must_epoch_launcher_verify_dp);
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
  
#endif // FLECSI_ENABLE_SPECIALIZATION_TLT_INIT


  //--------------------------------------------------------------------------//
  //  Create Legion index spaces and logical regions
  //-------------------------------------------------------------------------//

  auto coloring_info = context_.coloring_info_map();

  data.init_from_coloring_info_map(coloring_info);

  for(auto& itr : context_.adjacency_info()){
    data.add_adjacency(itr.second);
  }

  for(auto& itr : context_.index_subspace_info()){
    data.add_index_subspace(itr.second);
  }

  data.finalize(coloring_info);

  //-------------------------------------------------------------------------//
  //  Create Legion reduction 
  //-------------------------------------------------------------------------//

  //-------------------------------------------------------------------------//
  // Excute Legion task to maps between pre-compacted and compacted
  // data placement
  //-------------------------------------------------------------------------//
  
  auto ghost_owner_pos_fid =
    LegionRuntime::HighLevel::FieldID(internal_field::ghost_owner_pos);
 
  const auto pos_compaction_id =
    context_.task_id<__flecsi_internal_task_key(owner_pos_compaction_task)>();
  
  Legion::IndexLauncher pos_compaction_launcher(pos_compaction_id,
      data.color_domain(), Legion::TaskArgument(nullptr, 0),
      Legion::ArgumentMap());
  
  for(auto is: context_.coloring_map()) {
    size_t idx_space = is.first;
    auto& flecsi_ispace = data.index_space(idx_space);

    Legion::LogicalPartition color_lpart = runtime->get_logical_partition(ctx,
        flecsi_ispace.logical_region, flecsi_ispace.color_partition);
        pos_compaction_launcher.add_region_requirement(
        Legion::RegionRequirement(color_lpart, 0/*projection ID*/,
            WRITE_DISCARD, EXCLUSIVE, flecsi_ispace.logical_region))
                .add_field(ghost_owner_pos_fid);
  } // for idx_space

  Legion::MustEpochLauncher must_epoch_launcher1;
  must_epoch_launcher1.launch_domain = data.color_domain();
  must_epoch_launcher1.add_index_task(pos_compaction_launcher);
  runtime->execute_must_epoch(ctx, must_epoch_launcher1);

  // Fix ghost reference/pointer to point to compacted position of
  // shared that it needs
  const auto pos_correction_id =
    context_.task_id<__flecsi_internal_task_key(owner_pos_correction_task)>();

  Legion::IndexLauncher fix_ghost_refs_launcher(pos_correction_id,
    data.color_domain(), Legion::TaskArgument(nullptr, 0), Legion::ArgumentMap());

  //fix_ghost_refs_launcher.tag = MAPPER_FORCE_RANK_MATCH;

  for(auto is: context_.coloring_map()) {
    size_t idx_space = is.first;
    auto& flecsi_ispace = data.index_space(idx_space);

    Legion::LogicalPartition ghost_lpart = runtime->get_logical_partition(ctx,
    flecsi_ispace.logical_region, flecsi_ispace.ghost_partition);
    fix_ghost_refs_launcher.add_region_requirement(
    Legion::RegionRequirement(ghost_lpart, 0/*projection ID*/,
      READ_WRITE, EXCLUSIVE, flecsi_ispace.logical_region))
      .add_field(ghost_owner_pos_fid);

    Legion::LogicalPartition access_lp =runtime->get_logical_partition(ctx,
        flecsi_ispace.logical_region, flecsi_ispace.access_partition);
    Legion::LogicalRegion primary_lr = runtime->get_logical_subregion_by_color(
        ctx, access_lp, PRIMARY_ACCESS);

    fix_ghost_refs_launcher.add_region_requirement(
       Legion::RegionRequirement(primary_lr, READ_ONLY, EXCLUSIVE,
           flecsi_ispace.logical_region)
            .add_field(ghost_owner_pos_fid));
  } // for idx_space

  Legion::MustEpochLauncher must_epoch_launcher2;
  must_epoch_launcher2.launch_domain = data.color_domain();
  must_epoch_launcher2.add_index_task(fix_ghost_refs_launcher);
  auto fm = runtime->execute_must_epoch(ctx, must_epoch_launcher2);

  fm.wait_all_results(true);

  // map of index space to the field_ids that are mapped to this index space
  std::map<size_t, std::vector<field_id_t>> fields_map;

  size_t number_of_color_fields = 0;

  for(auto is: context_.coloring_map()) {
    size_t idx_space = is.first;
    for(const field_info_t& field_info : context_.registered_fields()){
      switch(field_info.storage_class){
        case global:
          number_of_global_fields++;
          break;
        case color:
          number_of_color_fields++;
          break;
        case subspace:
        case local:
          break;
        default:
          if(field_info.index_space == idx_space){
            fields_map[idx_space].push_back(field_info.fid);
          } // if
          break;
      }
    } // for

    {
      clog_tag_guard(runtime_driver);
      clog(trace) << "fields_map[" <<idx_space<<"] has "<<
        fields_map[idx_space].size()<< " fields"<<std::endl;
    } // scope
  } // for


  //--------------------------------------------------------------------------//
  //   Create Legion must epoch launcher and add Region requirements
  //-------------------------------------------------------------------------//
  
  // Must epoch launch
  Legion::MustEpochLauncher must_epoch_launcher;

  std::map<size_t,Legion::Serializer> args_serializers;

  const auto setup_rank_context_id =
    context_.task_id<__flecsi_internal_task_key(setup_rank_context_task)>();

  // Add colors to must_epoch_launcher
  for(size_t color(0); color<num_colors; ++color) {
   
    //-----------------------------------------------------------------------//
    // data serialization:
    //-----------------------------------------------------------------------//
    
    // #2 serialize field info
    size_t num_fields = context_.registered_fields().size();
    args_serializers[color].serialize(&num_fields, sizeof(size_t));
    args_serializers[color].serialize(
      &context_.registered_fields()[0], num_fields * sizeof(field_info_t));

   //-----------------------------------------------------------------------//
   //add region requirements to the setup_rank_context_launcher
   //-----------------------------------------------------------------------//
   
    Legion::TaskLauncher setup_rank_context_launcher(setup_rank_context_id,
        Legion::TaskArgument(args_serializers[color].get_buffer(),
                             args_serializers[color].get_used_bytes()));
    setup_rank_context_launcher.tag = MAPPER_FORCE_RANK_MATCH;

    Legion::DomainPoint point(color);
    must_epoch_launcher.add_single_task(point, setup_rank_context_launcher);
  } // for color

  // Launch the setup_rank_context tasks
  auto future = runtime->execute_must_epoch(ctx, must_epoch_launcher);
  bool silence_warnings = true;
  future.wait_all_results(silence_warnings);

  // Add additional setup.
  //context_.advance_state();

  auto& ispace_dmap = context_.index_space_data_map();

  //fill ispace_dmap with logical partitions
  for(auto is: context_.coloring_map()) {
    size_t idx_space = is.first;
    auto& flecsi_ispace = data.index_space(idx_space);

    Legion::LogicalPartition access_lp =runtime->get_logical_partition(ctx,
        flecsi_ispace.logical_region, flecsi_ispace.access_partition);
    Legion::LogicalRegion primary_lr = runtime->get_logical_subregion_by_color(
        ctx, access_lp, PRIMARY_ACCESS);
    Legion::LogicalRegion ghost_lr = runtime->get_logical_subregion_by_color(
        ctx, access_lp, GHOST_ACCESS);

    ispace_dmap[idx_space].entire_region = flecsi_ispace.logical_region;
    ispace_dmap[idx_space].color_partition = runtime->get_logical_partition(
        ctx, flecsi_ispace.logical_region, flecsi_ispace.color_partition);
    runtime->attach_name(ispace_dmap[idx_space].color_partition, "color logical partition");
    ispace_dmap[idx_space].primary_lp = runtime->get_logical_partition(
        ctx, primary_lr, flecsi_ispace.primary_partition);
    runtime->attach_name(ispace_dmap[idx_space].primary_lp, "primary logical partition");
    ispace_dmap[idx_space].exclusive_lp = runtime->get_logical_partition(
        ctx, primary_lr, flecsi_ispace.exclusive_partition);
    runtime->attach_name(ispace_dmap[idx_space].exclusive_lp, "exclusive logical partition");
    ispace_dmap[idx_space].shared_lp = runtime->get_logical_partition(
        ctx, primary_lr, flecsi_ispace.shared_partition);
    runtime->attach_name(ispace_dmap[idx_space].shared_lp, "shared logical partition");
    ispace_dmap[idx_space].ghost_lp = runtime->get_logical_partition(
        ctx, ghost_lr, flecsi_ispace.ghost_partition);
    runtime->attach_name(ispace_dmap[idx_space].ghost_lp, "ghost logical partition");

    // Now that ghosts point to post-compacted shared positions, we can
    // partition set of shared positions that ghosts need
    Legion::IndexSpace is_of_colors = runtime->create_index_space(ctx,
        data.color_domain());

    ispace_dmap[idx_space].ghost_owners_ip = runtime->create_partition_by_image(ctx,
        primary_lr.get_index_space(), ispace_dmap[idx_space].ghost_lp,
        flecsi_ispace.logical_region, ghost_owner_pos_fid, is_of_colors);
    runtime->attach_name(ispace_dmap[idx_space].ghost_owners_ip,
        "ghost owners index partition");
    ispace_dmap[idx_space].ghost_owners_lp = runtime->get_logical_partition(
        ctx, primary_lr, ispace_dmap[idx_space].ghost_owners_ip);
    runtime->attach_name(ispace_dmap[idx_space].ghost_owners_lp,
        "ghost owners logical partition");
  } // idx_space

  // initialize read/write flags for task_prolog
  for(auto is: context_.coloring_map()) {
    size_t idx_space = is.first;
    for (const field_id_t& field_id : fields_map[idx_space]){
      ispace_dmap[idx_space].ghost_is_readable[field_id] = true;
      ispace_dmap[idx_space].write_phase_started[field_id] = false;
    }//end field_info
  }//end for idx_space

  for (size_t idx : data.adjacencies()) {
    auto& adjacency = data.adjacency(idx);

    ispace_dmap[idx].entire_region = adjacency.logical_region;
    ispace_dmap[idx].color_partition =
        runtime->get_logical_partition(ctx,
            adjacency.logical_region, adjacency.index_partition);

  }

  //adding information for the global and color handles to the ispace_map

   if (number_of_global_fields>0){

    size_t global_index_space =
      execution::internal_index_space::global_is;

    auto global_ispace = data.global_index_space();

    ispace_dmap[global_index_space].entire_region = global_ispace.logical_region;
  }

  auto color_ispace = data.color_index_space();

  if(number_of_color_fields>0){
    size_t color_index_space =
      execution::internal_index_space::color_is;

    ispace_dmap[color_index_space].entire_region = color_ispace.logical_region;

    ispace_dmap[color_index_space].color_partition =
        runtime->get_logical_partition(ctx, color_ispace.logical_region,
            color_ispace.color_partition);
  }

  context_.advance_state();
  // run default or user-defined driver
  driver(args.argc, args.argv);

  //-----------------------------------------------------------------------//
  // Finish up Legion runtime and fall back out to MPI.
  // ----------------------------------------------------------------------//

  context_.unset_call_mpi(ctx, runtime);
  context_.handoff_to_mpi(ctx, runtime);
} // runtime_driver

void
setup_rank_context_task(
  const Legion::Task * task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx,
  Legion::Runtime * runtime
)
{
  using namespace data;

  const int my_color = task->index_point.point_data[0];

  {
  clog_tag_guard(runtime_driver);
  clog(info) << "Executing setup rank context task " << my_color << std::endl;
  }

  // Add additional setup.
  context_t & context_ = context_t::instance();

  clog_assert(task->arglen > 0, "setup_rank_context_task called without arguments");

  //---------------------------------------------------------------------//
  // Deserialize task arguments
  // --------------------------------------------------------------------//

  Legion::Deserializer args_deserializer(task->args, task->arglen);
 
  // #2 deserialize field info
  size_t num_fields;
  args_deserializer.deserialize(&num_fields, sizeof(size_t));

  using field_info_t = context_t::field_info_t;
  auto field_info_buf = new field_info_t [num_fields];

  args_deserializer.deserialize(field_info_buf,
                                sizeof(field_info_t) * num_fields);

  // add field_info into the context (map between name, hash, is and field)
  if( (context_.field_info_map()).size()==0){
    for(size_t i = 0; i < num_fields; ++i){
      field_info_t& fi = field_info_buf[i];
      context_.put_field_info(fi);
    }//end for i
  }//if

  // Setup maps from mesh to compacted (local) index space and vice versa
  //
  // This depends on the ordering of the BLIS data structure setup.
  // Currently, this is Exclusive - Shared - Ghost.

  for(auto is: context_.coloring_map()) {
    std::map<size_t, size_t> _map;
    size_t counter(0);

    for(auto index: is.second.exclusive) {
      _map[counter++] = index.id;
    } // for

    for(auto index: is.second.shared) {
      _map[counter++] = index.id;
    } // for

    for(auto index: is.second.ghost) {
      _map[counter++] = index.id;
    } // for

    context_.add_index_map(is.first, _map);
  } // for coloring_map
  // FIXME: local_index_space is now problematic in control replication
  // It made sense in SPMD, but now each IndexLaunch starts from a clean slate.
#if 0
  auto& local_index_space_map = context_.local_index_space_map();
  for(auto& itr : local_index_space_map){
    size_t index_space = itr.first;

    legion_data_t::local_index_space_t lis = 
      legion_data_t::create_local_index_space(
      ctx, runtime, index_space, itr.second);
    
    auto& lism = context_.local_index_space_data_map();

    context_t::local_index_space_data_t lis_data;
    lis_data.region = lis.logical_region;
    lism.emplace(index_space, std::move(lis_data));
  }
#endif

} // setup_rank_context_task

} // namespace execution 
} // namespace flecsi
