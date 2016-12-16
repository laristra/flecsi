/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_sprint_h
#define flecsi_sprint_h

#include <iostream>

#include "flecsi/utils/common.h"
#include "flecsi/execution/context.h"
#include "flecsi/execution/execution.h"
#include "flecsi/partition/index_partition.h"
#include "flecsi/partition/init_partitions_task.h"
#include "flecsi/partition/weaver.h"

///
// \file sprint.h
// \authors bergen
// \date Initial file creation: Aug 23, 2016
///

using namespace LegionRuntime::HighLevel;
using namespace LegionRuntime::Accessor;
using namespace LegionRuntime::Arrays;

namespace flecsi {
namespace execution {

static const size_t N = 8;

using index_partition_t = dmp::index_partition__<size_t>;

enum
FieldIDs {
  FID_CELL,
  FID_VERT,
  FID_GHOST_CELL_ID,
};

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

  flecsi::io::simple_definition_t sd("simple2d-8x8.msh");
  flecsi::dmp::weaver weaver(sd);

  using entry_info_t = flecsi::dmp::entry_info_t;

  index_partition_t ip_cells;

  ip_cells.primary = weaver.get_primary_cells();
  ip_cells.exclusive = weaver.get_exclusive_cells();
  ip_cells.shared = weaver.get_shared_cells();
  ip_cells.ghost  = weaver.get_ghost_cells();

  index_partition_t ip_vertices;

//  ip_vertices.primary = weaver.get_primary_vertices();
  ip_vertices.exclusive = weaver.get_exclusive_vertices();
  ip_vertices.shared = weaver.get_shared_vertices();
  ip_vertices.ghost  = weaver.get_ghost_vertices();

  for (auto vert_s : ip_vertices.shared)
   {
     ip_vertices.primary.insert(vert_s.id);
   }
  for (auto vert_e : ip_vertices.exclusive)
   {
     ip_vertices.primary.insert(vert_e.id);
   }

#if 0
   std::cout <<"DEBUG CELLS"<<std::endl;
   size_t i=0;
   for (auto cells_p : ip_cells.primary)
   {
    std::cout<<"primary["<<i<<"] = " <<cells_p<<std::endl;
    i++;
   }
   i=0;
   for (auto cells_s : ip_cells.shared)
   {
    std::cout<<"shared["<<i<<"] = " <<cells_s.id<< ", offset = "<< 
       cells_s.offset<<std::endl;
    i++;
   }


   std::cout <<"DEBUG VERTICES"<<std::endl;
   i=0;
   for (auto vert_p : ip_vertices.primary)
   {
    std::cout<<"primary["<<i<<"] = " <<vert_p<<std::endl;
    i++;
   }
   i=0;
   for (auto vert_s : ip_vertices.shared)
   {
    std::cout<<"shared["<<i<<"] = " <<vert_s.id<< ", offset = "<<
       vert_s.offset<<std::endl;
    i++;
   }
   i=0;
   for (auto vert_e : ip_vertices.exclusive)
   {
    std::cout<<"exclusive["<<i<<"] = " <<vert_e.id<< ", offset = "<<
       vert_e.offset<<std::endl;
    i++;
   }

#endif

  flecsi::execution::context_t & context_ =
    flecsi::execution::context_t::instance();
  context_.interop_helper_.data_storage_.push_back(
    flecsi::utils::any_t(ip_cells));

  context_.interop_helper_.data_storage_.push_back(
    flecsi::utils::any_t(ip_vertices));
}

  
register_task(mpi_task, mpi, single);

void
driver(
  int argc, 
  char ** argv
)
{
  context_t & context_ = context_t::instance();
  size_t task_key = const_string_t{"driver"}.hash();
  auto runtime = context_.runtime(task_key);
  auto context = context_.context(task_key);

  using legion_domain = LegionRuntime::HighLevel::Domain;

  flecsi::dmp::parts partitions;
  
  // first execute mpi task to setup initial partitions 
  execute_task(mpi_task, mpi, single, 1.0);
  // create a field space to store cells id
  FieldSpace cells_fs = runtime->create_field_space(context);
  {
    FieldAllocator allocator = runtime->create_field_allocator(context,
                                             cells_fs);
    allocator.allocate_field(sizeof(size_t), FID_CELL);
  }

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

  size_t total_num_cells=0;
  std::vector<size_t> cells_primary_start_id;
  std::vector<size_t> cells_num_shared;
  std::vector<size_t> cells_num_ghosts;
  std::vector<size_t> cells_num_exclusive;

  size_t total_num_vertices=0;
  std::vector<size_t> vert_primary_start_id;
  std::vector<size_t> vert_num_shared;
  std::vector<size_t> vert_num_ghosts;
  std::vector<size_t> vert_num_exclusive;

  //read dimension information from  get_numbers_of_cells task
  for (size_t i = 0; i < num_ranks; i++) {
    std::cout << "about to call get_results" << std::endl;
    flecsi::dmp::parts received = fm1.get_result<flecsi::dmp::parts>(
      DomainPoint::from_point<1>(make_point(i)));

    cells_primary_start_id.push_back(total_num_cells);
    total_num_cells += received.primary_cells;
    cells_num_shared.push_back(received.shared_cells);
    cells_num_ghosts.push_back(received.ghost_cells);
    cells_num_exclusive.push_back(received.exclusive_cells);

    vert_primary_start_id.push_back(total_num_vertices);
    total_num_vertices += received.primary_vertices;
    vert_num_shared.push_back(received.shared_vertices);
    vert_num_ghosts.push_back(received.ghost_vertices);
    vert_num_exclusive.push_back(received.exclusive_vertices);

#if 0
    std::cout << "From rank " << i 
              << " received cells (exclusive, shared, ghost) "
              << "(" << received.exclusive_cells << "," 
              << received.shared_cells << ","
              << received.ghost_cells << ")" << std::endl;
#endif
  }//end for


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
    allocator.allocate_field(sizeof(size_t), FID_VERT);
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
 

  Rect<1> rank_rect(Point<1>(0), Point<1>(num_ranks - 1));
  Domain rank_domain = Domain::from_rect<1>(rank_rect);

  LegionRuntime::HighLevel::IndexLauncher initialization_launcher(
    task_ids_t::instance().init_task_id,
    rank_domain,
    LegionRuntime::HighLevel::TaskArgument(0, 0),
    arg_map);
 
  initialization_launcher.tag = MAPPER_FORCE_RANK_MATCH; 

  initialization_launcher.add_region_requirement(
    RegionRequirement(cells_primary_lp, 0/*projection ID*/,
                      WRITE_DISCARD, EXCLUSIVE, cells_lr));
  initialization_launcher.add_field(0, FID_CELL);

  initialization_launcher.add_region_requirement(
    RegionRequirement(vert_primary_lp, 0/*projection ID*/,
                      WRITE_DISCARD, EXCLUSIVE, vertices_lr));
  initialization_launcher.add_field(1, FID_VERT);

  FutureMap fm2 = runtime->execute_index_space( context,
        initialization_launcher);
  
  fm2.wait_all_results();

#if 0 
  //printing cell_lr results
  {
    RegionRequirement req(cells_lr, READ_WRITE, EXCLUSIVE, cells_lr);
    req.add_field(FID_CELL);

    std::cout << "Back in driver (TTL) and checking values in Cells GlobalLR"
       << std::endl;
    InlineLauncher cell_launcher(req);
    PhysicalRegion cell_region = runtime->map_region(context, cell_launcher);
    cell_region.wait_until_valid();
    RegionAccessor<AccessorType::Generic, size_t> acc_cell =
      cell_region.get_field_accessor(FID_CELL).typeify<size_t>();

    IndexIterator itr2(runtime, context, cells_is);
    for (size_t i=0; i< total_num_cells; i++)
    {
      assert(itr2.has_next());
      ptr_t ptr = itr2.next();
      size_t value =
          acc_cell.read(ptr);
      std::cout << "cells_global[ " <<i<<" ] = " << value <<std::endl;
    }//end for
  }
#endif

  //creating partiotioning for shared and exclusive elements:
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
  shared_part_launcher.add_field(0, FID_CELL);

  shared_part_launcher.add_region_requirement(
    RegionRequirement(vert_primary_lp, 0/*projection ID*/,
      READ_ONLY, EXCLUSIVE, vertices_lr));
  shared_part_launcher.add_field(1, FID_VERT);

  FutureMap fm3 = runtime->execute_index_space( context, shared_part_launcher);
  fm3.wait_all_results();

  size_t indx=0;
  for (GenericPointInRectIterator<1> pir(rank_rect); pir; pir++)
  {
    flecsi::dmp::partition_lr sared_lr=
      fm3.get_result<flecsi::dmp::partition_lr>(
      DomainPoint::from_point<1>(pir.p));
    //gett shared partition info for Cells
    {
      LogicalRegion shared_pts_lr = sared_lr.cells;
      LegionRuntime::HighLevel::IndexSpace is = shared_pts_lr.get_index_space();
      LegionRuntime::HighLevel::RegionRequirement req(shared_pts_lr,
        READ_ONLY, EXCLUSIVE, shared_pts_lr);
      req.add_field(FID_SHARED);
      LegionRuntime::HighLevel::InlineLauncher shared_launcher(req);
      LegionRuntime::HighLevel::PhysicalRegion shared_region =
        runtime->map_region(context, shared_launcher);
      shared_region.wait_until_valid();
      LegionRuntime::Accessor::RegionAccessor<
      LegionRuntime::Accessor::AccessorType::Generic, ptr_t> acc =
        shared_region.get_field_accessor(FID_SHARED).typeify<ptr_t>();
      for (size_t j=0; j<cells_num_shared[indx]; j++)
      {
        ptr_t ptr=
          acc.read(LegionRuntime::HighLevel::DomainPoint::from_point<1>(
            make_point(j)));
        cells_shared_coloring[indx].points.insert(ptr);
      }//end for
      runtime->unmap_region(context, shared_region);
    }//scope

   //gett shared partition info for Vertices
    {
      LogicalRegion shared_pts_lr = sared_lr.vert;
      LegionRuntime::HighLevel::IndexSpace is = shared_pts_lr.get_index_space();
      LegionRuntime::HighLevel::RegionRequirement req(shared_pts_lr,
        READ_ONLY, EXCLUSIVE, shared_pts_lr);
      req.add_field(FID_SHARED);
      LegionRuntime::HighLevel::InlineLauncher shared_launcher(req);
      LegionRuntime::HighLevel::PhysicalRegion shared_region =
        runtime->map_region(context, shared_launcher);
      shared_region.wait_until_valid();
      LegionRuntime::Accessor::RegionAccessor<
      LegionRuntime::Accessor::AccessorType::Generic, ptr_t> acc =
        shared_region.get_field_accessor(FID_SHARED).typeify<ptr_t>();
      for (size_t j=0; j<vert_num_shared[indx]; j++)
      {
        ptr_t ptr=
          acc.read(LegionRuntime::HighLevel::DomainPoint::from_point<1>(
            make_point(j)));
        vert_shared_coloring[indx].points.insert(ptr);
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

  IndexPartition vert_shared_ip =
    runtime->create_index_partition(context, vertices_is,
      vert_shared_coloring, true);

  LogicalPartition vert_shared_lp = runtime->get_logical_partition(context,
    vertices_lr, vert_shared_ip);

  //creating partitioning for exclusive elements in cells_is
  Coloring cells_exclusive_coloring;
  Coloring vert_exclusive_coloring;

  LegionRuntime::HighLevel::IndexLauncher exclusive_part_launcher(
    task_ids_t::instance().exclusive_part_task_id,
    rank_domain,
    LegionRuntime::HighLevel::TaskArgument(0, 0),
    arg_map);

  exclusive_part_launcher.tag = MAPPER_FORCE_RANK_MATCH;

  exclusive_part_launcher.add_region_requirement(
    RegionRequirement(cells_primary_lp, 0/*projection ID*/,
      READ_ONLY, EXCLUSIVE, cells_lr));
  exclusive_part_launcher.add_field(0, FID_CELL);

  exclusive_part_launcher.add_region_requirement(
    RegionRequirement(vert_primary_lp, 0/*projection ID*/,
      READ_ONLY, EXCLUSIVE, vertices_lr));
  exclusive_part_launcher.add_field(1, FID_VERT);

  FutureMap fm4 = runtime->execute_index_space(context,exclusive_part_launcher);
  fm4.wait_all_results();

  indx=0;
  for (GenericPointInRectIterator<1> pir(rank_rect); pir; pir++)
  {
    flecsi::dmp::partition_lr exclusive_lr =
      fm4.get_result<flecsi::dmp::partition_lr>(
      DomainPoint::from_point<1>(pir.p)); 
    {
      LogicalRegion exclusive_pts_lr= exclusive_lr.cells;
      LegionRuntime::HighLevel::IndexSpace is =
        exclusive_pts_lr.get_index_space();
      LegionRuntime::HighLevel::RegionRequirement req(exclusive_pts_lr,
        READ_ONLY, EXCLUSIVE, exclusive_pts_lr);
      req.add_field(FID_EXCLUSIVE);
      LegionRuntime::HighLevel::InlineLauncher exclusive_launcher(req);
      LegionRuntime::HighLevel::PhysicalRegion exclusive_region =
        runtime->map_region(context, exclusive_launcher);
      exclusive_region.wait_until_valid();
      LegionRuntime::Accessor::RegionAccessor<
      LegionRuntime::Accessor::AccessorType::Generic, ptr_t> acc =
        exclusive_region.get_field_accessor(FID_EXCLUSIVE).typeify<ptr_t>();
      for (size_t j=0; j<cells_num_exclusive[indx]; j++)
      {
        ptr_t ptr=
          acc.read(LegionRuntime::HighLevel::DomainPoint::from_point<1>(
            make_point(j)));
        cells_exclusive_coloring[indx].points.insert(ptr);
      }//end for
      runtime->unmap_region(context, exclusive_region);
    }//scope
 
    {
      LogicalRegion exclusive_pts_lr= exclusive_lr.vert;
      LegionRuntime::HighLevel::IndexSpace is =
        exclusive_pts_lr.get_index_space();
      LegionRuntime::HighLevel::RegionRequirement req(exclusive_pts_lr,
        READ_ONLY, EXCLUSIVE, exclusive_pts_lr);
      req.add_field(FID_EXCLUSIVE);
      LegionRuntime::HighLevel::InlineLauncher exclusive_launcher(req);
      LegionRuntime::HighLevel::PhysicalRegion exclusive_region =
        runtime->map_region(context, exclusive_launcher);
      exclusive_region.wait_until_valid();
      LegionRuntime::Accessor::RegionAccessor<
      LegionRuntime::Accessor::AccessorType::Generic, ptr_t> acc =
        exclusive_region.get_field_accessor(FID_EXCLUSIVE).typeify<ptr_t>();
      for (size_t j=0; j<vert_num_exclusive[indx]; j++)
      {
        ptr_t ptr=
          acc.read(LegionRuntime::HighLevel::DomainPoint::from_point<1>(
            make_point(j)));
        vert_exclusive_coloring[indx].points.insert(ptr);
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

  IndexPartition vert_exclusive_ip =
    runtime->create_index_partition(context, vertices_is,
      vert_exclusive_coloring, true);

  LogicalPartition vert_exclusive_lp = runtime->get_logical_partition(context,
    vertices_lr, vert_exclusive_ip);


  //creating partitioning for ghost elements in cells_is

  Coloring cells_ghost_coloring;
  Coloring vert_ghost_coloring;

  LegionRuntime::HighLevel::IndexLauncher ghost_part_launcher(
    task_ids_t::instance().ghost_part_task_id,
    rank_domain,
    LegionRuntime::HighLevel::TaskArgument(0, 0),
    arg_map);

  ghost_part_launcher.tag = MAPPER_FORCE_RANK_MATCH;

  ghost_part_launcher.add_region_requirement(
    RegionRequirement(cells_lr, 0/*projection ID*/,
      READ_ONLY, EXCLUSIVE, cells_lr));
  ghost_part_launcher.add_field(0, FID_CELL);

  ghost_part_launcher.add_region_requirement(
    RegionRequirement(vertices_lr, 0/*projection ID*/,
      READ_ONLY, EXCLUSIVE, vertices_lr));
  ghost_part_launcher.add_field(1, FID_VERT);

  FutureMap fm5 = runtime->execute_index_space(context,ghost_part_launcher);
  fm5.wait_all_results();


  indx=0;
  for (GenericPointInRectIterator<1> pir(rank_rect); pir; pir++)
  {
    flecsi::dmp::partition_lr  ghost_lr=
      fm5.get_result<flecsi::dmp::partition_lr >(
      DomainPoint::from_point<1>(pir.p));
    {
      LogicalRegion ghost_pts_lr=ghost_lr.cells;
      LegionRuntime::HighLevel::IndexSpace is =
        ghost_pts_lr.get_index_space();
      LegionRuntime::HighLevel::RegionRequirement req(ghost_pts_lr,
        READ_ONLY, EXCLUSIVE, ghost_pts_lr);
      req.add_field(FID_GHOST);
      LegionRuntime::HighLevel::InlineLauncher ghost_launcher(req);
      LegionRuntime::HighLevel::PhysicalRegion ghost_region =
        runtime->map_region(context, ghost_launcher);
      ghost_region.wait_until_valid();
      LegionRuntime::Accessor::RegionAccessor<
      LegionRuntime::Accessor::AccessorType::Generic, ptr_t> acc =
        ghost_region.get_field_accessor(FID_GHOST).typeify<ptr_t>();
      for (size_t j=0; j<cells_num_ghosts[indx]; j++)
      {
        ptr_t ptr=
          acc.read(LegionRuntime::HighLevel::DomainPoint::from_point<1>(
            make_point(j)));
        cells_ghost_coloring[indx].points.insert(ptr);
      }//end for
      runtime->unmap_region(context, ghost_region);
    }//end scope

    {
      LogicalRegion ghost_pts_lr=ghost_lr.vert;
      LegionRuntime::HighLevel::IndexSpace is =
        ghost_pts_lr.get_index_space();
      LegionRuntime::HighLevel::RegionRequirement req(ghost_pts_lr,
        READ_ONLY, EXCLUSIVE, ghost_pts_lr);
      req.add_field(FID_GHOST);
      LegionRuntime::HighLevel::InlineLauncher ghost_launcher(req);
      LegionRuntime::HighLevel::PhysicalRegion ghost_region =
        runtime->map_region(context, ghost_launcher);
      ghost_region.wait_until_valid();
      LegionRuntime::Accessor::RegionAccessor<
      LegionRuntime::Accessor::AccessorType::Generic, ptr_t> acc =
        ghost_region.get_field_accessor(FID_GHOST).typeify<ptr_t>();
      for (size_t j=0; j<vert_num_ghosts[indx]; j++)
      {
        ptr_t ptr=
          acc.read(LegionRuntime::HighLevel::DomainPoint::from_point<1>(
            make_point(j)));
        vert_ghost_coloring[indx].points.insert(ptr);
      }//end for
      runtime->unmap_region(context, ghost_region);
    }//end scope


    indx++;
  }//end for

  IndexPartition cells_ghost_ip = runtime->create_index_partition(context,
        cells_is,cells_ghost_coloring, true);

  LogicalPartition cells_ghost_lp = runtime->get_logical_partition(context,
    cells_lr, cells_ghost_ip);

  IndexPartition vert_ghost_ip = runtime->create_index_partition(context,
        vertices_is,vert_ghost_coloring, true);

  LogicalPartition vert_ghost_lp = runtime->get_logical_partition(context,
    vertices_lr, vert_ghost_ip);


  //call a legion task that checks our partitions
  LegionRuntime::HighLevel::IndexLauncher check_part_launcher(
    task_ids_t::instance().check_partitioning_task_id,
    rank_domain,
    LegionRuntime::HighLevel::TaskArgument(0, 0),
    arg_map);

  check_part_launcher.tag = MAPPER_FORCE_RANK_MATCH;

  check_part_launcher.add_region_requirement(
    RegionRequirement(cells_shared_lp, 0/*projection ID*/,
      READ_ONLY, EXCLUSIVE, cells_lr));
  check_part_launcher.add_field(0, FID_CELL);

  check_part_launcher.add_region_requirement(
    RegionRequirement(cells_exclusive_lp, 0/*projection ID*/,
      READ_ONLY, EXCLUSIVE, cells_lr));
  check_part_launcher.add_field(1, FID_CELL);

  check_part_launcher.add_region_requirement(
    RegionRequirement(cells_ghost_lp, 0/*projection ID*/,
      READ_ONLY, EXCLUSIVE, cells_lr));
  check_part_launcher.add_field(2, FID_CELL);

   check_part_launcher.add_region_requirement(
    RegionRequirement(vert_shared_lp, 0/*projection ID*/,
      READ_ONLY, EXCLUSIVE, vertices_lr));
  check_part_launcher.add_field(3, FID_VERT);

  check_part_launcher.add_region_requirement(
    RegionRequirement(vert_exclusive_lp, 0/*projection ID*/,
      READ_ONLY, EXCLUSIVE, vertices_lr));
  check_part_launcher.add_field(4, FID_VERT);

  check_part_launcher.add_region_requirement(
    RegionRequirement(vert_ghost_lp, 0/*projection ID*/,
      READ_ONLY, EXCLUSIVE, vertices_lr));
  check_part_launcher.add_field(5, FID_VERT);

  FutureMap fm6 = runtime->execute_index_space(context,check_part_launcher);
  fm6.wait_all_results();



} // driver

} // namespace execution
} // namespace flecsi

#endif // flecsi_sprint_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
