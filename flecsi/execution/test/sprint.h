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

using index_partition_t = dmp::index_partition__<size_t>;
using ghost_info_t = index_partition_t::ghost_info_t;
using shared_info_t = index_partition_t::shared_info_t;

static const size_t N = 8;

enum 
FieldIDs {
  FID_CELL,
  FID_GHOST_CELL_ID,
};

  
void 
mpi_task(
  double val
) 
{
  int rank = 0;
  int size = 0;

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  //std::cout << "My rank: " << rank << std::endl;

  size_t part = N/size;
  size_t rem = N%size;

  size_t start = rank*(part + (rem > 0 ? 1 : 0));
  size_t end = rank < rem ? start + part+1 : start + part;

  std::vector<size_t> start_global_id;
  size_t global_end=0;
  for (int i=0; i<size; i++)
  {
    size_t start_i = rank*(part + (rem > 0 ? 1 : 0));
    size_t end_i = rank < rem ? start_i + part+1 : start + part;
    start_global_id.push_back(global_end);
    global_end +=N*(end_i-start_i);
  }//end for

#if 1
  std::cout << "rank: " << rank << " start: " << start <<
    " end: " << end << std::endl;
#endif

  index_partition_t ip;

  for(size_t j=0; j<N; ++j) {
    for(size_t i(start); i<end; ++i) {
      ghost_info_t ghost;
      const size_t id = j*N+i;
      // exclusive
      if(i>start && i<end-1) {
        ip.exclusive.push_back(id);
        //std::cout << "rank: " << rank << " exclusive: " << id << std::endl;
      }
      else if(rank == 0 && i==start) {
        ip.exclusive.push_back(id);
        //std::cout << "rank: " << rank << " exclusive: " << id << std::endl;
       }
      else if(rank == size-1 && i==end-1) {
        ip.exclusive.push_back(id);
        //std::cout << "rank: " << rank << " exclusive: " << id << std::endl;
      }
      else if(i==start) {
          shared_info_t shared;
          shared.mesh_id = id;
          shared.global_id =0;
          shared.dependent_ranks.push_back(rank - 1);
          ip.shared.push_back(shared);

          const size_t ghost_id = j*N+i-1;
          ghost.mesh_id =ghost_id ;
          ghost.global_id = start_global_id[rank-1]+(end-start)*j+1;
          ghost.rank =rank -1;
          ip.ghost.push_back(ghost);
      }
      else if(i==end-1) {
          shared_info_t shared;
          shared.mesh_id = id;
          shared.global_id = 0; 
          shared.dependent_ranks.push_back(rank + 1);
          ip.shared.push_back(shared);

          const size_t ghost_id = j*N+i+1;
          ghost.mesh_id =ghost_id ;
          ghost.rank = rank+1 ;
          ghost.global_id = start_global_id[rank+1]+(end-start)*j;
          ip.ghost.push_back(ghost);
      } // if
    } // for
  } // for

  //creating primary partitioning and filling global_id's for shared elements:
  int start_indx=0;
  for (int i=0; i<ip.exclusive.size(); i++){
    for (int j=start_indx; j<ip.shared.size(); j++){
        if (ip.exclusive[i]<ip.shared_id(j))
        {
          ip.primary.push_back(ip.exclusive[i]);
          j=ip.shared.size()+1;
        }//end if
        else 
        {
          ip.primary.push_back(ip.shared_id(j));
          ip.shared[j].global_id = start_global_id[rank]+ip.primary.size()-1;
        }//end else
        start_indx=ip.primary.size()-i-1;
      }//end for
  }//end_for
  for (int i = start_indx; i< ip.shared.size(); i++)
  { 
      ip.primary.push_back(ip.shared_id(i));
      ip.shared[i].global_id = start_global_id[rank]+ip.primary.size()-1;
  }//end for


  if (size>1)
    assert (ip.primary.size() == (ip.exclusive.size()+ip.shared.size()));   

  flecsi::execution::context_t & context_ =
             flecsi::execution::context_t::instance();
  context_.interop_helper_.data_storage_.push_back(
        flecsi::utils::any_t(ip));
#if 0 
   //check the mpi output

  for (int i =0; i< ip.primary.size(); i++)
  {
   std::cout<< " rank = " << rank <<" global_id = " << start_global_id[rank]+i<< 
    " primary = " << ip.primary[i]<< std::endl;
  }
#if 0
  for (int i =0; i< ip.shared.size(); i++)
  {
    std::cout<< " rank = " << rank << " shared_mesh_id  = " << ip.shared_id(i) 
          << std::endl;
    std::cout<< " rank = " << rank << " shared_global_id  = " 
        << ip.shared[i].global_id << std::endl;
    std::cout<< " rank = " << rank << "dependent_ranks" << 
         ip.shared[i].dependent_ranks[0] << std::endl;
  }

  for (int i =0; i< ip.ghost.size(); i++)
  {
    std::cout<< " rank = " << rank << " gost_mesh_id  = " << ip.ghost_id(i)
          << std::endl;
    std::cout<< " rank = " << rank << " ghost_global_id  = " 
        << ip.ghost[i].global_id << std::endl;
    std::cout<< " rank = " << rank << "ghost rank" <<
         ip.ghost[i].rank << std::endl;
  }
   
#endif
#endif

} // mpi_task
  
register_task(mpi_task, mpi, single);
  

void
driver(
  int argc,
  char ** argv
)
{
  using legion_domain = LegionRuntime::HighLevel::Domain;

  context_t & context_ = context_t::instance();
  flecsi::dmp::parts partitions;
  
  // first execute mpi task to setup initial partitions
  execute_task(mpi_task, mpi, single, 1.0);
  
  // create a field space to store cells id
  FieldSpace cells_fs =
      context_.runtime()->create_field_space(context_.context());
  {
    FieldAllocator allocator =
      context_.runtime()->create_field_allocator(context_.context(),
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

  FutureMap fm1 = context_.runtime()->execute_index_space(
             context_.context(), get_numbers_of_cells_launcher);

  size_t total_num_cells=0;
  std::vector<size_t> primary_start_id;
  for (int i = 0; i < num_ranks; i++) {
    std::cout << "about to call get_results" << std::endl;
    flecsi::dmp::parts received = fm1.get_result<flecsi::dmp::parts>(
                           DomainPoint::from_point<2>(make_point(i,0)));

    primary_start_id.push_back(total_num_cells);
    total_num_cells += received.primary;

#if 1
    std::cout << "From rank " << i << " received (exclusive, shared, ghost) "
              << "(" << received.exclusive << "," << received.shared << ","
              << received.ghost << ")" << std::endl;
#endif
  }//end for

  
  IndexSpace cells_is = context_.runtime()->create_index_space(
          context_.context(), total_num_cells);

  IndexAllocator allocator = context_.runtime()->create_index_allocator( 
                                          context_.context(),cells_is);
  for(size_t i = 0; i < total_num_cells; ++i){
    ptr_t  ptr_i= allocator.alloc(1);
    assert(!ptr_i.is_null());
  }

  LogicalRegion cells_lr=
       context_.runtime()->create_logical_region(context_.context(),
                                      cells_is, cells_fs);
  context_.runtime()->attach_name(cells_lr,
                 "cells  logical region");

  //partition cells by number of mpi ranks

  Coloring primary_coloring;

  IndexIterator itr(context_.runtime(), context_.context(), cells_is);
  
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
    context_.runtime()->create_index_partition(context_.context(),
          cells_is, primary_coloring, true);

  LogicalPartition primary_lp =
    context_.runtime()->get_logical_partition(context_.context(),
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

  FutureMap fm2 = context_.runtime()->execute_index_space(
             context_.context(), init_cells_launcher);
  
  fm2.wait_all_results();

#if 0
  //printing cell_lr results
  {
    RegionRequirement req(cells_lr, READ_WRITE, EXCLUSIVE, cells_lr);
    req.add_field(FID_CELL);

    std::cout << "Back in driver (TTL) and checking values in Cells GlobalLR"
       << std::endl;
    InlineLauncher cell_launcher(req);
    PhysicalRegion cell_region =
      context_.runtime()->map_region(context_.context(), cell_launcher);
    cell_region.wait_until_valid();
    RegionAccessor<AccessorType::Generic, int> acc_cell =
      cell_region.get_field_accessor(FID_CELL).typeify<int>();

    IndexIterator itr2(context_.runtime(), context_.context(), cells_is);
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

} // driver

} // namespace execution
} // namespace flecsi

#endif // flecsi_sprint_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
