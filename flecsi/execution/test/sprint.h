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

  index_partition_t ip;

  ip.primary = weaver.get_primary_cells();
  ip.exclusive = weaver.get_exclusive_cells();
  ip.shared = weaver.get_shared_cells();
  ip.ghost  = weaver.get_ghost_cells();

#if 0
std::cout<<"print primary cells"<<std::endl;
for (std::set<size_t>::iterator it=ip.primary.begin();
        it!=ip.primary.end(); ++it)
      std::cout << ' ' << *it;
std::cout<<"\n";

std::cout<<"shared primary cells"<<std::endl;
for (std::set<entry_info_t>::iterator it=ip.shared.begin(); 
        it!=ip.shared.end(); ++it)
      std::cout << ' ' << *it<<std::endl;
std::cout<<"\n";
#endif

  flecsi::execution::context_t & context_ =
    flecsi::execution::context_t::instance();
  context_.interop_helper_.data_storage_.push_back(
    flecsi::utils::any_t(ip));

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
    allocator.allocate_field(sizeof(int), FID_CELL);
  }

  int num_ranks;
  MPI_Comm_size(MPI_COMM_WORLD, &num_ranks);

  //call task to calculate total_num_cells and to get number of 
  //cells per partiotioning

  LegionRuntime::HighLevel::ArgumentMap arg_map;

  LegionRuntime::HighLevel::IndexLauncher get_numbers_of_cells_launcher(
    task_ids_t::instance().get_numbers_of_cells_task_id,
    legion_domain::from_rect<2>(context_.interop_helper_.all_processes_),
    LegionRuntime::HighLevel::TaskArgument(0, 0),
    arg_map);

  get_numbers_of_cells_launcher.tag = MAPPER_ALL_PROC;

  FutureMap fm1 = runtime->execute_index_space(context, 
      get_numbers_of_cells_launcher);

  size_t total_num_cells=0;
  std::vector<size_t> primary_start_id;
  std::vector<size_t> num_shared;
  std::vector<size_t> num_ghosts;
  std::vector<size_t> num_exclusive;

  for (int i = 0; i < num_ranks; i++) {
    std::cout << "about to call get_results" << std::endl;
    flecsi::dmp::parts received = fm1.get_result<flecsi::dmp::parts>(
      DomainPoint::from_point<2>(make_point(i,0)));

    primary_start_id.push_back(total_num_cells);
    total_num_cells += received.primary;
    num_shared.push_back(received.shared);
    num_ghosts.push_back(received.ghost);
    num_exclusive.push_back(received.exclusive);

#if 1
    std::cout << "From rank " << i << " received (exclusive, shared, ghost) "
              << "(" << received.exclusive << "," << received.shared << ","
              << received.ghost << ")" << std::endl;
#endif
  }//end for


  IndexSpace cells_is = runtime->create_index_space(context, total_num_cells);

  IndexAllocator allocator = runtime->create_index_allocator(context,cells_is);
  allocator.alloc(total_num_cells);

  LogicalRegion cells_lr=
    runtime->create_logical_region(context,cells_is, cells_fs);
  runtime->attach_name(cells_lr, "cells  logical region");

  //partition cells by number of mpi ranks

  Coloring primary_coloring;

  IndexIterator itr(runtime, context, cells_is);

  for(size_t i = 0; i < num_ranks-1; ++i){
    for (size_t j=primary_start_id[i]; j<primary_start_id[i+1]; j++){
      assert(itr.has_next());
      ptr_t ptr = itr.next();
      primary_coloring[i].points.insert(ptr);
    }//end for
  }//end for

  for (size_t j=primary_start_id[num_ranks-1]; j<total_num_cells; j++){
    assert(itr.has_next());
    ptr_t ptr = itr.next();
    primary_coloring[num_ranks-1].points.insert(ptr);
  }//end for

  IndexPartition primary_ip = 
    runtime->create_index_partition(context, cells_is, primary_coloring, true);

  LogicalPartition primary_lp = runtime->get_logical_partition(context,
           cells_lr, primary_ip);
  Rect<1> rank_rect(Point<1>(0), Point<1>(num_ranks - 1));
  Domain rank_domain = Domain::from_rect<1>(rank_rect);

  LegionRuntime::HighLevel::IndexLauncher init_cells_launcher(
    task_ids_t::instance().init_cells_task_id,
    rank_domain,
    LegionRuntime::HighLevel::TaskArgument(0, 0),
    arg_map);
 
  init_cells_launcher.tag = MAPPER_FORCE_RANK_MATCH; 

  init_cells_launcher.add_region_requirement(
    RegionRequirement(primary_lp, 0/*projection ID*/,
                      WRITE_DISCARD, EXCLUSIVE, cells_lr));
  init_cells_launcher.add_field(0, FID_CELL);

  FutureMap fm2 = runtime->execute_index_space( context, init_cells_launcher);
  
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
    RegionAccessor<AccessorType::Generic, int> acc_cell =
      cell_region.get_field_accessor(FID_CELL).typeify<int>();

    IndexIterator itr2(runtime, context, cells_is);
    for (int i=0; i< total_num_cells; i++)
    {
      assert(itr2.has_next());
      ptr_t ptr = itr2.next();
      int value =
          acc_cell.read(ptr);
      std::cout << "cells_global[ " <<i<<" ] = " << value <<std::endl;
    }//end for
  }
#endif

  //creating partiotioning for shared and exclusive elements:
  Coloring shared_coloring;

  LegionRuntime::HighLevel::IndexLauncher shared_part_launcher(
    task_ids_t::instance().shared_part_task_id,
    rank_domain,
    LegionRuntime::HighLevel::TaskArgument(0, 0),
    arg_map);

  shared_part_launcher.tag = MAPPER_FORCE_RANK_MATCH;

  shared_part_launcher.add_region_requirement(
    RegionRequirement(primary_lp, 0/*projection ID*/,
      READ_ONLY, EXCLUSIVE, cells_lr));
  shared_part_launcher.add_field(0, FID_CELL);

  FutureMap fm3 = runtime->execute_index_space( context, shared_part_launcher);
  fm3.wait_all_results();

  int indx=0;
  for (GenericPointInRectIterator<1> pir(rank_rect); pir; pir++)
  {
    LogicalRegion shared_pts_lr= fm3.get_result< LogicalRegion >(
      DomainPoint::from_point<1>(pir.p));
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
    for (int j=0; j<num_shared[indx]; j++)
    {

      ptr_t ptr=
        acc.read(LegionRuntime::HighLevel::DomainPoint::from_point<1>(
          make_point(j)));
      shared_coloring[indx].points.insert(ptr);
    }//end for
    runtime->unmap_region(context, shared_region);
    indx++;
  }//end for

  IndexPartition shared_ip =
    runtime->create_index_partition(context, cells_is, shared_coloring, true);

  LogicalPartition shared_lp = runtime->get_logical_partition(context,
    cells_lr, shared_ip);

  //creating partitioning for exclusive elements in cells_is
  Coloring exclusive_coloring;

  LegionRuntime::HighLevel::IndexLauncher exclusive_part_launcher(
    task_ids_t::instance().exclusive_part_task_id,
    rank_domain,
    LegionRuntime::HighLevel::TaskArgument(0, 0),
    arg_map);

  exclusive_part_launcher.tag = MAPPER_FORCE_RANK_MATCH;

  exclusive_part_launcher.add_region_requirement(
    RegionRequirement(primary_lp, 0/*projection ID*/,
      READ_ONLY, EXCLUSIVE, cells_lr));
  exclusive_part_launcher.add_field(0, FID_CELL);

  FutureMap fm4 = runtime->execute_index_space(context,exclusive_part_launcher);
  fm4.wait_all_results();

  indx=0;
  for (GenericPointInRectIterator<1> pir(rank_rect); pir; pir++)
  {
    LogicalRegion exclusive_pts_lr= fm4.get_result< LogicalRegion >(
      DomainPoint::from_point<1>(pir.p));
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
    for (int j=0; j<num_exclusive[indx]; j++)
    {

      ptr_t ptr=
        acc.read(LegionRuntime::HighLevel::DomainPoint::from_point<1>(
          make_point(j)));
      exclusive_coloring[indx].points.insert(ptr);
    }//end for
    runtime->unmap_region(context, exclusive_region);
    indx++;
  }//end for

  IndexPartition exclusive_ip =
    runtime->create_index_partition(context, cells_is,exclusive_coloring, true);

  LogicalPartition exclusive_lp = runtime->get_logical_partition(context,
    cells_lr, exclusive_ip);

  //creating partitioning for ghost elements in cells_is

  Coloring ghost_coloring;

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

  FutureMap fm5 = runtime->execute_index_space(context,ghost_part_launcher);
  fm5.wait_all_results();


  indx=0;
  for (GenericPointInRectIterator<1> pir(rank_rect); pir; pir++)
  {
    LogicalRegion ghost_pts_lr= fm5.get_result< LogicalRegion >(
      DomainPoint::from_point<1>(pir.p));
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
    for (int j=0; j<num_ghosts[indx]; j++)
    {

      ptr_t ptr=
        acc.read(LegionRuntime::HighLevel::DomainPoint::from_point<1>(
          make_point(j)));
      ghost_coloring[indx].points.insert(ptr);
    }//end for
    runtime->unmap_region(context, ghost_region);
    indx++;
  }//end for

  IndexPartition ghost_ip =
    runtime->create_index_partition(context, cells_is,ghost_coloring, true);

  LogicalPartition ghost_lp = runtime->get_logical_partition(context,
    cells_lr, ghost_ip);


  //call a legion task that checks our partitions
  LegionRuntime::HighLevel::IndexLauncher check_part_launcher(
    task_ids_t::instance().check_partitioning_task_id,
    rank_domain,
    LegionRuntime::HighLevel::TaskArgument(0, 0),
    arg_map);

  check_part_launcher.tag = MAPPER_FORCE_RANK_MATCH;

  check_part_launcher.add_region_requirement(
    RegionRequirement(shared_lp, 0/*projection ID*/,
      READ_ONLY, EXCLUSIVE, cells_lr));
  check_part_launcher.add_field(0, FID_CELL);

  check_part_launcher.add_region_requirement(
    RegionRequirement(exclusive_lp, 0/*projection ID*/,
      READ_ONLY, EXCLUSIVE, cells_lr));
  check_part_launcher.add_field(1, FID_CELL);

  check_part_launcher.add_region_requirement(
    RegionRequirement(ghost_lp, 0/*projection ID*/,
      READ_ONLY, EXCLUSIVE, cells_lr));
  check_part_launcher.add_field(2, FID_CELL);

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
