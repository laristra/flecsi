/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2017 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef lax_wendroff_h
#define lax_wendroff_h

#include <iostream>

#include "flecsi/execution/execution.h"
#include "flecsi/partition/weaver.h"

///
// \file lax_wendroff.h
// \authors jgraham, irina, bergen
// \date Initial file creation: Feb 2, 2017
///

using namespace LegionRuntime::HighLevel;
using namespace LegionRuntime::Accessor;
using namespace LegionRuntime::Arrays;

namespace flecsi {
namespace execution {

using index_partition_t = dmp::index_partition__<size_t>;

void
mpi_task(
  double d
)
{
  int rank = 0;
  int size = 0;

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  std::cout << "My rank: " << rank << std::endl;

  flecsi::io::simple_definition_t sd("simple2d-64x64.msh");
  flecsi::dmp::weaver weaver(sd);

  using entry_info_t = flecsi::dmp::entry_info_t;

  index_partition_t ip_cells;

  ip_cells.primary = weaver.get_primary_cells();
  ip_cells.exclusive = weaver.get_exclusive_cells();
  ip_cells.shared = weaver.get_shared_cells();
  ip_cells.ghost  = weaver.get_ghost_cells();
  ip_cells.entities_per_rank = weaver.get_n_cells_per_rank();

  index_partition_t ip_vertices;

  ip_vertices.primary = weaver.get_primary_vertices();
  ip_vertices.exclusive = weaver.get_exclusive_vertices();
  ip_vertices.shared = weaver.get_shared_vertices();
  ip_vertices.ghost  = weaver.get_ghost_vertices();
  ip_vertices.entities_per_rank = weaver.get_n_vertices_per_rank();

  std::vector<std::pair<size_t, size_t>> raw_conns = 
    weaver.get_raw_cell_vertex_conns();


  flecsi::execution::context_t & context_ =
    flecsi::execution::context_t::instance();
  context_.interop_helper_.data_storage_.push_back(
    flecsi::utils::any_t(ip_cells));

  context_.interop_helper_.data_storage_.push_back(
    flecsi::utils::any_t(ip_vertices));

  context_.interop_helper_.data_storage_.push_back(
    flecsi::utils::any_t(raw_conns));
}

  
flecsi_register_task(mpi_task, mpi, single);

void
specialization_driver(
  int argc, 
  char ** argv
)
{
  context_t & context_ = context_t::instance();
  size_t task_key = utils::const_string_t{"specialization_driver"}.hash();
  auto runtime = context_.runtime(task_key);
  auto context = context_.context(task_key);

  legion_helper h(runtime, context);

  using legion_domain = LegionRuntime::HighLevel::Domain;
  field_ids_t & fid_t =field_ids_t::instance();

  flecsi::execution::sprint::parts partitions;
  
  // first execute mpi task to setup initial partitions 
  flecsi_execute_task(mpi_task, mpi, single, 1.0);
  // create a field space to store cells id

  int num_ranks;
  MPI_Comm_size(MPI_COMM_WORLD, &num_ranks);

  //call task to calculate total_num_cells and to get number of 
  //cells per partiotioning

  LegionRuntime::HighLevel::ArgumentMap arg_map;

  LegionRuntime::HighLevel::IndexLauncher get_numbers_of_cells_launcher(
    task_ids_t::instance().get_numbers_of_cells_task_id,
    legion_domain::from_rect<1>(context_.interop_helper_.all_processes_),
    LegionRuntime::HighLevel::TaskArgument(0, 0),
    arg_map);

  get_numbers_of_cells_launcher.tag = MAPPER_FORCE_RANK_MATCH;

  FutureMap fm1 = runtime->execute_index_space(context, 
      get_numbers_of_cells_launcher);

  legion_dpd::partitioned_unstructured cells_part;
  legion_dpd::partitioned_unstructured vertices_part;

  size_t total_num_cells=0;
  std::vector<size_t> cells_primary_start_id;
  std::vector<size_t> cells_num_shared;
  std::vector<size_t> cells_num_ghosts;
  std::vector<size_t> cells_num_exclusive;
  std::vector<size_t> num_vertex_conns;

  size_t total_num_vertices=0;
  std::vector<size_t> vert_primary_start_id;
  std::vector<size_t> vert_num_shared;
  std::vector<size_t> vert_num_ghosts;
  std::vector<size_t> vert_num_exclusive;

  //read dimension information from  get_numbers_of_cells task
  for (size_t i = 0; i < num_ranks; i++) {
    std::cout << "about to call get_results" << std::endl;
    flecsi::execution::sprint::parts received =
      fm1.get_result<flecsi::execution::sprint::parts>(
      DomainPoint::from_point<1>(LegionRuntime::Arrays::make_point(i)));

    cells_primary_start_id.push_back(total_num_cells);
    total_num_cells += received.primary_cells;
    cells_num_shared.push_back(received.shared_cells);
    cells_num_ghosts.push_back(received.ghost_cells);
    cells_num_exclusive.push_back(received.exclusive_cells);

    cells_part.count_map[i] = received.primary_cells;

    vert_primary_start_id.push_back(total_num_vertices);
    total_num_vertices += received.primary_vertices;
    vert_num_shared.push_back(received.shared_vertices);
    vert_num_ghosts.push_back(received.ghost_vertices);
    vert_num_exclusive.push_back(received.exclusive_vertices);
    num_vertex_conns.push_back(received.vertex_conns);

    vertices_part.count_map[i] = received.primary_vertices;

  }//end for

  // create a field space to store cells id
  FieldSpace cells_fs = runtime->create_field_space(context);
  { 
    FieldAllocator allocator = runtime->create_field_allocator(context,
                                             cells_fs);
    allocator.allocate_field(sizeof(size_t), fid_t.fid_cell);
    allocator.allocate_field(sizeof(double), fid_t.fid_data);
//TOFIX
    allocator.allocate_field(sizeof(legion_dpd::ptr_count),
                      legion_dpd::connectivity_field_id(2, 0));
    allocator.allocate_field(sizeof(ptr_t), fid_t.fid_ptr_t);
  }


  //create global IS fnd LR for Cells
  IndexSpace cells_is = runtime->create_index_space(context, total_num_cells);
  {
    IndexAllocator allocator = runtime->create_index_allocator(context,
          cells_is);
    allocator.alloc(total_num_cells);
  }

  LogicalRegion cells_lr=
    runtime->create_logical_region(context,cells_is, cells_fs);
  runtime->attach_name(cells_lr, "cells  logical region");

  LogicalRegion ghost_lr=
    runtime->create_logical_region(context,cells_is, cells_fs);
  runtime->attach_name(ghost_lr, "ghost  logical region");

   //create global IS fnd LR for Vertices

  FieldSpace vertices_fs = runtime->create_field_space(context);
  {
    FieldAllocator allocator = runtime->create_field_allocator(context,
                                             vertices_fs);
    allocator.allocate_field(sizeof(size_t), fid_t.fid_vert);
    allocator.allocate_field(sizeof(ptr_t), fid_t.fid_ptr_t);
  } 

  IndexSpace vertices_is = runtime->create_index_space(context,
          total_num_vertices);
  {
    IndexAllocator allocator = runtime->create_index_allocator(context,
            vertices_is);
    allocator.alloc(total_num_vertices);
  }

  LogicalRegion vertices_lr=
    runtime->create_logical_region(context,vertices_is, vertices_fs);
  runtime->attach_name(vertices_lr, "vertices  logical region");



  //partition cells by number of mpi ranks

  Coloring cells_primary_coloring;
  {
    IndexIterator itr(runtime, context, cells_is);

    for(size_t i = 0; i < num_ranks-1; ++i){
      for (size_t j=cells_primary_start_id[i]; 
          j<cells_primary_start_id[i+1]; j++){
        assert(itr.has_next());
        ptr_t ptr = itr.next();
        cells_primary_coloring[i].points.insert(ptr);
      }//end for
    }//end for

    for (size_t j=cells_primary_start_id[num_ranks-1]; j<total_num_cells; j++){
      assert(itr.has_next());
      ptr_t ptr = itr.next();
      cells_primary_coloring[num_ranks-1].points.insert(ptr);
    }//end for
  }

  IndexPartition cells_primary_ip = 
    runtime->create_index_partition(context, cells_is,
    cells_primary_coloring, true);

  LogicalPartition cells_primary_lp = runtime->get_logical_partition(context,
           cells_lr, cells_primary_ip);

  LogicalPartition ghost_primary_lp = runtime->get_logical_partition(context,
           ghost_lr, cells_primary_ip);


	//partition vertices by number of mpi ranks

  Coloring vert_primary_coloring;
  {
    IndexIterator itr(runtime, context, vertices_is);

    for(size_t i = 0; i < num_ranks-1; ++i){
      for (size_t j=vert_primary_start_id[i];
          j<vert_primary_start_id[i+1]; j++){
        assert(itr.has_next());
        ptr_t ptr = itr.next();
        vert_primary_coloring[i].points.insert(ptr);
      }//end for
    }//end for

    for (size_t j=vert_primary_start_id[num_ranks-1];
				j<total_num_vertices; j++){
      assert(itr.has_next());
      ptr_t ptr = itr.next();
      vert_primary_coloring[num_ranks-1].points.insert(ptr);
    }//end for
  }//end scope

  IndexPartition vert_primary_ip =
    runtime->create_index_partition(context, vertices_is,
    vert_primary_coloring, true);

  LogicalPartition vert_primary_lp = runtime->get_logical_partition(context,
           vertices_lr, vert_primary_ip);
 
  LegionRuntime::Arrays::Rect<1> rank_rect(LegionRuntime::Arrays::Point<1>(0),
    LegionRuntime::Arrays::Point<1>(num_ranks - 1));
  Domain rank_domain = Domain::from_rect<1>(rank_rect);


  LegionRuntime::HighLevel::IndexLauncher initialization_launcher(
    task_ids_t::instance().init_task_id,
    rank_domain,
    LegionRuntime::HighLevel::TaskArgument(0, 0),
    arg_map);
 
  initialization_launcher.tag = MAPPER_FORCE_RANK_MATCH; 

  initialization_launcher.add_region_requirement(
    RegionRequirement(cells_primary_lp, 0,
                      WRITE_DISCARD, EXCLUSIVE, cells_lr));
  initialization_launcher.add_field(0, fid_t.fid_cell);

  initialization_launcher.add_region_requirement(
    RegionRequirement(vert_primary_lp, 0,
                      WRITE_DISCARD, EXCLUSIVE, vertices_lr));
  initialization_launcher.add_field(1, fid_t.fid_vert);

  FutureMap fm2 = runtime->execute_index_space( context,
        initialization_launcher);
  
  fm2.wait_all_results();

  {
	  LegionRuntime::HighLevel::IndexLauncher initialization_launcher(
	    task_ids_t::instance().init_task_id,
	    rank_domain,
	    LegionRuntime::HighLevel::TaskArgument(0, 0),
	    arg_map);

	  initialization_launcher.tag = MAPPER_FORCE_RANK_MATCH;

	  initialization_launcher.add_region_requirement(
	    RegionRequirement(ghost_primary_lp, 0,
	                      WRITE_DISCARD, EXCLUSIVE, ghost_lr));
	  initialization_launcher.add_field(0, fid_t.fid_cell);

	  initialization_launcher.add_region_requirement(
	    RegionRequirement(vert_primary_lp, 0,
	                      WRITE_DISCARD, EXCLUSIVE, vertices_lr));
	  initialization_launcher.add_field(1, fid_t.fid_vert);

	  FutureMap fm2 = runtime->execute_index_space( context,
	        initialization_launcher);

	  fm2.wait_all_results();
  }


  //creating partiotioning for shared and exclusive elements:
  Coloring cells_shared_coloring;

  LegionRuntime::HighLevel::IndexLauncher shared_part_launcher(
    task_ids_t::instance().shared_part_task_id,
    rank_domain,
    LegionRuntime::HighLevel::TaskArgument(0, 0),
    arg_map);

  shared_part_launcher.tag = MAPPER_FORCE_RANK_MATCH;

  shared_part_launcher.add_region_requirement(
    RegionRequirement(cells_primary_lp, 0/*projection ID*/,
      READ_ONLY, EXCLUSIVE, cells_lr));
  shared_part_launcher.add_field(0, fid_t.fid_cell);

  shared_part_launcher.add_region_requirement(
    RegionRequirement(vert_primary_lp, 0/*projection ID*/,
      READ_ONLY, EXCLUSIVE, vertices_lr));
  shared_part_launcher.add_field(1, fid_t.fid_vert);

  FutureMap fm3 = runtime->execute_index_space( context, shared_part_launcher);
  fm3.wait_all_results();

  size_t indx=0;
  for (GenericPointInRectIterator<1> pir(rank_rect); pir; pir++)
  {
    flecsi::execution::sprint::partition_lr sared_lr=
      fm3.get_result<flecsi::execution::sprint::partition_lr>(
      DomainPoint::from_point<1>(pir.p));
    //gett shared partition info for Cells
    {
      LogicalRegion shared_pts_lr = sared_lr.cells;
      LegionRuntime::HighLevel::IndexSpace is = shared_pts_lr.get_index_space();
      LegionRuntime::HighLevel::RegionRequirement req(shared_pts_lr,
        READ_ONLY, EXCLUSIVE, shared_pts_lr);
      req.add_field(fid_t.fid_ptr_t);
      LegionRuntime::HighLevel::InlineLauncher shared_launcher(req);
      LegionRuntime::HighLevel::PhysicalRegion shared_region =
        runtime->map_region(context, shared_launcher);
      shared_region.wait_until_valid();
      LegionRuntime::Accessor::RegionAccessor<
      LegionRuntime::Accessor::AccessorType::Generic, ptr_t> acc =
        shared_region.get_field_accessor(fid_t.fid_ptr_t).typeify<ptr_t>();
      for (size_t j=0; j<cells_num_shared[indx]; j++)
      {
        ptr_t ptr=
          acc.read(LegionRuntime::HighLevel::DomainPoint::from_point<1>(
            LegionRuntime::Arrays::make_point(j)));
        cells_shared_coloring[indx].points.insert(ptr);
      }//end for
      runtime->unmap_region(context, shared_region);
    }//scope

    indx++;
  }//end for

  IndexPartition cells_shared_ip =
    runtime->create_index_partition(context, cells_is, 
      cells_shared_coloring, true);

  LogicalPartition cells_shared_lp = runtime->get_logical_partition(context,
    cells_lr, cells_shared_ip);

  //creating partitioning for exclusive elements in cells_is
  Coloring cells_exclusive_coloring;

  LegionRuntime::HighLevel::IndexLauncher exclusive_part_launcher(
    task_ids_t::instance().exclusive_part_task_id,
    rank_domain,
    LegionRuntime::HighLevel::TaskArgument(0, 0),
    arg_map);

  exclusive_part_launcher.tag = MAPPER_FORCE_RANK_MATCH;

  exclusive_part_launcher.add_region_requirement(
    RegionRequirement(cells_primary_lp, 0/*projection ID*/,
      READ_ONLY, EXCLUSIVE, cells_lr));
  exclusive_part_launcher.add_field(0, fid_t.fid_cell);

  exclusive_part_launcher.add_region_requirement(
    RegionRequirement(vert_primary_lp, 0/*projection ID*/,
      READ_ONLY, EXCLUSIVE, vertices_lr));
  exclusive_part_launcher.add_field(1, fid_t.fid_vert);

  FutureMap fm4 = runtime->execute_index_space(context,exclusive_part_launcher);
  fm4.wait_all_results();

  indx=0;
  for (GenericPointInRectIterator<1> pir(rank_rect); pir; pir++)
  {
    flecsi::execution::sprint::partition_lr exclusive_lr =
      fm4.get_result<flecsi::execution::sprint::partition_lr>(
      DomainPoint::from_point<1>(pir.p)); 
    {
      LogicalRegion exclusive_pts_lr= exclusive_lr.cells;
      LegionRuntime::HighLevel::IndexSpace is =
        exclusive_pts_lr.get_index_space();
      LegionRuntime::HighLevel::RegionRequirement req(exclusive_pts_lr,
        READ_ONLY, EXCLUSIVE, exclusive_pts_lr);
      req.add_field(fid_t.fid_ptr_t);
      LegionRuntime::HighLevel::InlineLauncher exclusive_launcher(req);
      LegionRuntime::HighLevel::PhysicalRegion exclusive_region =
        runtime->map_region(context, exclusive_launcher);
      exclusive_region.wait_until_valid();
      LegionRuntime::Accessor::RegionAccessor<
      LegionRuntime::Accessor::AccessorType::Generic, ptr_t> acc =
        exclusive_region.get_field_accessor(fid_t.fid_ptr_t).typeify<ptr_t>();
      for (size_t j=0; j<cells_num_exclusive[indx]; j++)
      {
        ptr_t ptr=
          acc.read(LegionRuntime::HighLevel::DomainPoint::from_point<1>(
            LegionRuntime::Arrays::make_point(j)));
        cells_exclusive_coloring[indx].points.insert(ptr);
      }//end for
      runtime->unmap_region(context, exclusive_region);
    }//scope

    indx++;
  }//end for

  IndexPartition cells_exclusive_ip =
    runtime->create_index_partition(context, cells_is,
      cells_exclusive_coloring, true);

  LogicalPartition cells_exclusive_lp = runtime->get_logical_partition(context,
    cells_lr, cells_exclusive_ip);

  //creating partitioning for ghost elements in cells_is

  Coloring cells_ghost_coloring;

  LegionRuntime::HighLevel::IndexLauncher ghost_part_launcher(
    task_ids_t::instance().ghost_part_task_id,
    rank_domain,
    LegionRuntime::HighLevel::TaskArgument(0, 0),
    arg_map);

  ghost_part_launcher.tag = MAPPER_FORCE_RANK_MATCH;

  ghost_part_launcher.add_region_requirement(
    RegionRequirement(cells_lr, 0/*projection ID*/,
      READ_ONLY, EXCLUSIVE, cells_lr));
  ghost_part_launcher.add_field(0, fid_t.fid_cell);

  ghost_part_launcher.add_region_requirement(
    RegionRequirement(vertices_lr, 0/*projection ID*/,
      READ_ONLY, EXCLUSIVE, vertices_lr));
  ghost_part_launcher.add_field(1, fid_t.fid_vert);

  FutureMap fm5 = runtime->execute_index_space(context,ghost_part_launcher);
  fm5.wait_all_results();


  indx=0;
  for (GenericPointInRectIterator<1> pir(rank_rect); pir; pir++)
  {
    flecsi::execution::sprint::partition_lr  ghost_lr=
      fm5.get_result<flecsi::execution::sprint::partition_lr >(
      DomainPoint::from_point<1>(pir.p));
    {
      LogicalRegion ghost_pts_lr=ghost_lr.cells;
      LegionRuntime::HighLevel::IndexSpace is =
        ghost_pts_lr.get_index_space();
      LegionRuntime::HighLevel::RegionRequirement req(ghost_pts_lr,
        READ_ONLY, EXCLUSIVE, ghost_pts_lr);
      req.add_field(fid_t.fid_ptr_t);
      LegionRuntime::HighLevel::InlineLauncher ghost_launcher(req);
      LegionRuntime::HighLevel::PhysicalRegion ghost_region =
        runtime->map_region(context, ghost_launcher);
      ghost_region.wait_until_valid();
      LegionRuntime::Accessor::RegionAccessor<
      LegionRuntime::Accessor::AccessorType::Generic, ptr_t> acc =
        ghost_region.get_field_accessor(fid_t.fid_ptr_t).typeify<ptr_t>();
      for (size_t j=0; j<cells_num_ghosts[indx]; j++)
      {
        ptr_t ptr=
          acc.read(LegionRuntime::HighLevel::DomainPoint::from_point<1>(
            LegionRuntime::Arrays::make_point(j)));
        cells_ghost_coloring[indx].points.insert(ptr);
      }//end for
      runtime->unmap_region(context, ghost_region);
    }//end scope

    indx++;
  }//end for

  IndexPartition cells_ghost_ip = runtime->create_index_partition(context,
        cells_is,cells_ghost_coloring, false);

  LogicalPartition cells_ghost_lp = runtime->get_logical_partition(context,
    ghost_lr, cells_ghost_ip);

  //call a legion task that tests ghost cell access
	std::set<Processor> all_procs;
	Realm::Machine::get_machine().get_all_processors(all_procs);
	int num_procs = 0;
	for(std::set<Processor>::const_iterator it = all_procs.begin();
	      it != all_procs.end();
	      it++)
	    if((*it).kind() == Processor::LOC_PROC)
	      num_procs++;
	assert(num_procs == num_ranks);

  // figure out communication pattern

  std::vector<PhaseBarrier> phase_barriers;
  std::vector<std::set<int>> master_colors(num_ranks);
  for (int master_color=0; master_color < num_ranks; ++master_color) {
    std::set<int> slave_colors;
    for (std::set<ptr_t>::iterator it=cells_shared_coloring[master_color].points.begin();
      it!=cells_shared_coloring[master_color].points.end(); ++it) {
      const ptr_t ptr = *it;
      for (int slave_color = 0; slave_color < num_ranks; ++slave_color)
        if (cells_ghost_coloring[slave_color].points.count(ptr)) {
          slave_colors.insert(slave_color);
          master_colors[slave_color].insert(master_color);
        }
    }
    phase_barriers.push_back(runtime->create_phase_barrier(context, 1 + slave_colors.size()));
  }

  // Launch SPMD tasks
  MustEpochLauncher must_epoch_launcher;
  std::vector<execution::sprint::SPMDArgs> spmd_args(num_ranks);
  std::vector<execution::sprint::SPMDArgsSerializer> args_serialized(num_ranks);
  for (int color=0; color < num_ranks; ++color) {
    spmd_args[color].pbarrier_as_master = phase_barriers[color];

    for (std::set<int>::iterator master=master_colors[color].begin();
    		master!=master_colors[color].end(); ++master) {
      spmd_args[color].masters_pbarriers.push_back(phase_barriers[*master]);
    }

    args_serialized[color].archive(&(spmd_args[color]));

    TaskLauncher ghost_access_launcher(task_ids_t::instance().lax_wendroff_task_id,
    		TaskArgument(args_serialized[color].getBitStream(), args_serialized[color].getBitStreamSize()));

    ghost_access_launcher.tag = MAPPER_FORCE_RANK_MATCH;

    LogicalRegion lregion_shared = runtime->get_logical_subregion_by_color(context,
    		cells_shared_lp,color);
    ghost_access_launcher.add_region_requirement(
    		RegionRequirement(lregion_shared,
    				READ_WRITE, SIMULTANEOUS, cells_lr));
    ghost_access_launcher.add_field(0, fid_t.fid_data);
    ghost_access_launcher.add_field(0, fid_t.fid_cell);

    LogicalRegion lregion_exclusive = runtime->get_logical_subregion_by_color(context,
    		cells_exclusive_lp,color);
    ghost_access_launcher.add_region_requirement(
    		RegionRequirement(lregion_exclusive,
    				READ_WRITE, EXCLUSIVE, cells_lr));
    ghost_access_launcher.add_field(1, fid_t.fid_data);
    ghost_access_launcher.add_field(1, fid_t.fid_cell);

    LogicalRegion lregion_ghost = runtime->get_logical_subregion_by_color(context,
    		cells_ghost_lp,color);
    ghost_access_launcher.add_region_requirement(
    		RegionRequirement(lregion_ghost,
    				READ_ONLY, EXCLUSIVE, ghost_lr).add_field(fid_t.fid_cell));

    int req_index = 3;
    for (std::set<int>::iterator master=master_colors[color].begin();
    		master!=master_colors[color].end(); ++master) {
      LogicalRegion lregion_ghost = runtime->get_logical_subregion_by_color(context,
    		  cells_shared_lp,*master);
    ghost_access_launcher.add_region_requirement(
    		RegionRequirement(lregion_ghost,
    				READ_ONLY, SIMULTANEOUS, cells_lr));
      ghost_access_launcher.region_requirements[req_index].add_flags(NO_ACCESS_FLAG);
      ghost_access_launcher.add_field(req_index, fid_t.fid_data);
      req_index++;
    }

    IndexSpace ispace_ghost = runtime->get_index_subspace(context, cells_ghost_ip, color);
    ghost_access_launcher.add_index_requirement(IndexSpaceRequirement(ispace_ghost,
    		NO_MEMORY,cells_is));

    DomainPoint point(color);
    must_epoch_launcher.add_single_task(point,ghost_access_launcher);
  }

  FutureMap fm7 = runtime->execute_must_epoch(context,must_epoch_launcher);
  fm7.wait_all_results();

  for (unsigned idx = 0; idx < phase_barriers.size(); idx++)
    runtime->destroy_phase_barrier(context, phase_barriers[idx]);
  phase_barriers.clear();

  TaskLauncher write_launcher(task_ids_t::instance().lax_write_task_id,
  		TaskArgument(nullptr, 0));
  write_launcher.add_region_requirement(
		  RegionRequirement(cells_lr, READ_ONLY, EXCLUSIVE, cells_lr)
		  .add_field(fid_t.fid_data).add_field(fid_t.fid_cell));
  Future future = runtime->execute_task(context, write_launcher);
  future.get_void_result();

  //TOFIX: free all lr physical regions is
  runtime->destroy_logical_region(context, vertices_lr);
  runtime->destroy_logical_region(context, cells_lr);
  runtime->destroy_field_space(context, vertices_fs);
  runtime->destroy_field_space(context, cells_fs);
  runtime->destroy_index_space(context,cells_is);
  runtime->destroy_index_space(context,vertices_is);

} // specialization_driver

} // namespace execution
} // namespace flecsi

#endif // lax_wendroff_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
