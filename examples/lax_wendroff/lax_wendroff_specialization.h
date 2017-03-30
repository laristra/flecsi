/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2017 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef lax_wendroff_specialization_h
#define lax_wendroff_specialization_h

#include <iostream>

#include "flecsi/execution/execution.h"
#include "flecsi/partition/weaver.h"
#include "flecsi/execution/test/mpilegion/init_partitions_task.h"

///
// \file lax_wendroff.h
// \authors jgraham, irina, bergen
// \date Initial file creation: Feb 2, 2017
///

using namespace LegionRuntime::HighLevel;
using namespace LegionRuntime::Accessor;
using namespace LegionRuntime::Arrays;
using namespace flecsi::execution::test;

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

  flecsi::io::simple_definition_t sd("simple2d-32x32.msh");
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

  data_client& dc = *((data_client*)argv[argc - 1]);


  using legion_domain = LegionRuntime::HighLevel::Domain;
  field_ids_t & fid_t =field_ids_t::instance();

  flecsi::execution::test::parts partitions;
  
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
    flecsi::execution::test::parts received =
      fm1.get_result<flecsi::execution::test::parts>(
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

  execution::mpilegion_context_policy_t::partitioned_index_space cells_parts;
  cells_parts.size = total_num_cells;
  cells_parts.entities_lr = cells_lr;

  execution::mpilegion_context_policy_t::partitioned_index_space verts_parts;
  verts_parts.size = total_num_vertices;
  verts_parts.entities_lr = vertices_lr;

  //creating partitioning for shared and exclusive elements:
  Coloring cells_shared_coloring;
  Coloring vert_shared_coloring;

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
    flecsi::execution::test::partition_lr sared_lr=
      fm3.get_result<flecsi::execution::test::partition_lr>(
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
      cells_parts.shared_count_map[indx] =  cells_num_shared[indx];
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
    flecsi::execution::test::partition_lr exclusive_lr =
      fm4.get_result<flecsi::execution::test::partition_lr>(
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
      cells_parts.exclusive_count_map[indx] =  cells_num_exclusive[indx];
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
    flecsi::execution::test::partition_lr  ghost_lr=
      fm5.get_result<flecsi::execution::test::partition_lr >(
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
      cells_parts.ghost_count_map[indx] =  cells_num_ghosts[indx];
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
    cells_lr, cells_ghost_ip);

// copy cell_id to flecsi data structures

  cells_parts.shared_ip = cells_shared_ip;
  cells_parts.ghost_ip = cells_ghost_ip;
  cells_parts.exclusive_ip = cells_exclusive_ip;

  const int versions = 1;
  int index_id = 0;

  dc.put_index_space(index_id, cells_parts);

  flecsi_register_data(dc, lax, cell_ID, size_t, dense, versions, index_id);
  flecsi_register_data(dc, lax, phi, double, dense, versions, index_id);
  flecsi_register_data(dc, lax, phi_update, double, dense, versions, index_id);

  auto cell_handle =
    flecsi_get_handle(dc, lax, cell_ID, size_t, dense, index_id, rw, rw, ro);


  LegionRuntime::HighLevel::IndexLauncher copy_legion_to_flecsi_launcher(
    task_ids_t::instance().copy_legion_to_flecsi_task_id,
    rank_domain,
    LegionRuntime::HighLevel::TaskArgument(0, 0),
    arg_map);

  copy_legion_to_flecsi_launcher.tag = MAPPER_FORCE_RANK_MATCH;

  copy_legion_to_flecsi_launcher.add_region_requirement(
    RegionRequirement(cells_shared_lp, 0/*projection ID*/,
      READ_ONLY, EXCLUSIVE, cells_lr));
  copy_legion_to_flecsi_launcher.add_field(0, fid_t.fid_cell);

  copy_legion_to_flecsi_launcher.add_region_requirement(
    RegionRequirement(cells_exclusive_lp, 0/*projection ID*/,
      READ_ONLY, EXCLUSIVE, cells_lr));
  copy_legion_to_flecsi_launcher.add_field(1, fid_t.fid_cell);

  LogicalPartition flecsi_exclusive_lp = runtime->get_logical_partition(context,
           cell_handle.lr, cell_handle.exclusive_ip);
  copy_legion_to_flecsi_launcher.add_region_requirement(
    RegionRequirement(flecsi_exclusive_lp, 0/*projection ID*/,
      READ_WRITE, EXCLUSIVE, cell_handle.lr));
  copy_legion_to_flecsi_launcher.add_field(2, fid_t.fid_value);

  LogicalPartition flecsi_shared_lp = runtime->get_logical_partition(context,
           cell_handle.lr, cell_handle.shared_ip);
  copy_legion_to_flecsi_launcher.add_region_requirement(
    RegionRequirement(flecsi_shared_lp, 0/*projection ID*/,
      READ_WRITE, EXCLUSIVE, cell_handle.lr));
  copy_legion_to_flecsi_launcher.add_field(3, fid_t.fid_value);

  FutureMap fm_copy = runtime->execute_index_space(context,copy_legion_to_flecsi_launcher);
  fm_copy.wait_all_results();


  //call a legion task that tests ghost cell access
  //be careful not to destroy any index spaces that we will still be using
  runtime->destroy_logical_region(context, vertices_lr);
  runtime->destroy_logical_region(context, cells_lr);
  runtime->destroy_field_space(context, vertices_fs);
  runtime->destroy_field_space(context, cells_fs);
  runtime->destroy_index_space(context,vertices_is);

} // specialization_driver

} // namespace execution
} // namespace flecsi

#endif // lax_wendroff_specialization_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
