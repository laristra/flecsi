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

static const size_t N = 8;

enum 
FieldIDs {
  FID_CELL_PART,
  FID_GLOBAL_CELL,
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
                      
#if 1
  std::cout << "rank: " << rank << " start: " << start <<
    " end: " << end << std::endl;
#endif

  index_partition_t ip;

  for(size_t j(0); j<N; ++j) {
    for(size_t i(start); i<end; ++i) {
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
        ip.shared.push_back(id);
        //std::cout << "rank: " << rank << " shared: " << id << std::endl;

        const size_t ghost_id = j*N+i-1;
        ip.ghost.push_back(ghost_id);
        //std::cout << "rank: " << rank << " ghost: " << ghost_id << std::endl;
      }
      else if(i==end-1) {
        ip.shared.push_back(id);
        //std::cout << "rank: " << rank << " shared: " << id << std::endl;

        const size_t ghost_id = j*N+i+1;
        ip.ghost.push_back(ghost_id);
        //std::cout << "rank: " << rank << " ghost: " << ghost_id << std::endl;
      } // if

    } // for
  } // for
 

  flecsi::execution::context_t & context_ =
             flecsi::execution::context_t::instance();
  context_.interop_helper_.data_storage_.push_back(
        flecsi::utils::any_t(ip));

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
  
  // create a field space to store my cell paritioning
  FieldSpace cells_temp_fs =
      context_.runtime()->create_field_space(context_.context());
  {
    FieldAllocator allocator =
      context_.runtime()->create_field_allocator(context_.context(),
                                             cells_temp_fs);
    allocator.allocate_field(sizeof(int), FID_CELL_PART);
  }

  int num_ranks;
  MPI_Comm_size(MPI_COMM_WORLD, &num_ranks);
  int max_cells = 1000;
  Point<2> elem_rect_lo; elem_rect_lo.x[0] = 0; elem_rect_lo.x[1]=0;
  Point<2> elem_rect_hi;
  elem_rect_hi.x[0] = num_ranks;
  elem_rect_hi.x[1] = max_cells;
  Rect<2> elem_rect( elem_rect_lo, elem_rect_hi );
  
  IndexSpace cells_temp_is = context_.runtime()->create_index_space(
          context_.context(), Domain::from_rect<2>(elem_rect));
  
  LogicalRegion cell_temp_lr =
       context_.runtime()->create_logical_region(context_.context(),
                                      cells_temp_is, cells_temp_fs);
  context_.runtime()->attach_name(cell_temp_lr,
                 "cell partitions logical region");

  // Use blockify to create num_rank slices of the data
  Point<2> cell_temp_color;
  cell_temp_color.x[0] = 1;
  cell_temp_color.x[1] = max_cells;
  Blockify<2> cells_temp_coloring(cell_temp_color);
  
  IndexPartition cells_temp_ip = context_.runtime()->create_index_partition(
                    context_.context(), cells_temp_is, cells_temp_coloring);
  LogicalPartition cell_temp_lp = context_.runtime()->
       get_logical_partition(context_.context(), cell_temp_lr, cells_temp_ip);
  context_.runtime()->attach_name(cell_temp_lp,
            "cell partitions temporary logical partition");
  

  LegionRuntime::HighLevel::ArgumentMap arg_map;

  // this is the init partitions task 
  LegionRuntime::HighLevel::IndexLauncher init_cell_partitions_launcher(
    task_ids_t::instance().init_cell_partitions_task_id,
    legion_domain::from_rect<2>(context_.interop_helper_.all_processes_),
    LegionRuntime::HighLevel::TaskArgument(0, 0),
    arg_map);
  
  
  init_cell_partitions_launcher.add_region_requirement(
    RegionRequirement(cell_temp_lp, 0/*projection ID*/,
                      WRITE_DISCARD, EXCLUSIVE, cell_temp_lr));
  init_cell_partitions_launcher.add_field(0, FID_CELL_PART);
  FutureMap fm = context_.runtime()->execute_index_space(
             context_.context(), init_cell_partitions_launcher);
  
  fm.wait_all_results();

  std::cout << "returned from wait_all_results()" << std::endl;

  for (int i = 0; i < num_ranks; i++) {
    std::cout << "about to call get_results" << std::endl;
    flecsi::dmp::parts received = fm.get_result<flecsi::dmp::parts>(
                           DomainPoint::from_point<2>(make_point(i,0)));
    std::cout << "From rank " << i << " received (exclusive, shared, ghost) "
              << "(" << received.exclusive << "," << received.shared << ","
              << received.ghost << ")" << std::endl; 
  }
 
   //printing results
  { 
  	// GMS: back at the top level, we need to read this partitioning information
  	RegionRequirement req(cell_temp_lr, READ_WRITE, EXCLUSIVE, cell_temp_lr);
  	req.add_field(FID_CELL_PART);

    std::cout << "Back in driver (TTL) and checking values in LR" << std::endl;
    InlineLauncher cell_temp_launcher(req);
    PhysicalRegion cell_temp_region =
      context_.runtime()->map_region(context_.context(), cell_temp_launcher);
    cell_temp_region.wait_until_valid();
    RegionAccessor<AccessorType::Generic, int> acc_cell_temp =
      cell_temp_region.get_field_accessor(FID_CELL_PART).typeify<int>();
    int zeros = 0;
    int non_zeros = 0; 
    int num_cells = 0;
    for (int i = 0; i < num_ranks; i++) {
      flecsi::dmp::parts received = fm.get_result<flecsi::dmp::parts>(
          DomainPoint::from_point<2>(make_point(i,0)));
      int j = 0;
      num_cells += received.exclusive+received.shared;
      for (; j < received.exclusive; j++) {
        double value =
          acc_cell_temp.read(DomainPoint::from_point<2>(make_point(i,j)));
        std::cout << "partition (" << i << "," << j << ") exclusive cell id: "
               << value << std::endl;
       }//end for
       for (; j < received.exclusive+received.shared; j++) {
         double value =
          acc_cell_temp.read(DomainPoint::from_point<2>(make_point(i,j)));
         std::cout << "partition (" << i << "," << j << ") shared cell id: "
               << value << std::endl;
        }//end for
       for (; j < received.exclusive+received.shared+received.ghost; j++) {
        double value =
          acc_cell_temp.read(DomainPoint::from_point<2>(make_point(i,j)));
        std::cout << "partition (" << i << "," << j << ") ghost cell id: "
               << value << std::endl;
        }//end for
    }//end for
  
    std::cout << "partition has " << zeros << " zeros " << non_zeros <<
                " non zeros " << std::endl;
  }//scope
  
  //Creating Global Cells Logical Region
  int num_cells = 0;
  for (int i = 0; i < num_ranks; i++) {
    flecsi::dmp::parts received = fm.get_result<flecsi::dmp::parts>(
        DomainPoint::from_point<2>(make_point(i,0)));
    num_cells += received.exclusive+received.shared;
  }//end for

  Rect<1> elem_rect_cells(Point<1>(0),Point<1>(num_cells-1));
  IndexSpace cells_is = context_.runtime()->create_index_space(
             context_.context(), Domain::from_rect<1>(elem_rect_cells));
  context_.runtime()->attach_name(cells_is, "cells global index space");

  FieldSpace cells_fs =
        context_.runtime()->create_field_space(context_.context());
  {
    FieldAllocator allocator =
      context_.runtime()->create_field_allocator(context_.context(), cells_fs);
    allocator.allocate_field(sizeof(int),FID_GLOBAL_CELL);
  }
  context_.runtime()->attach_name(cells_fs, "cells global field space");

  LogicalRegion cells_lr = context_.runtime()->create_logical_region(
      context_.context(), cells_is , cells_fs);
  context_.runtime()->attach_name(cells_lr, "cells global logical region");

  //partitioning global cells array so each task gets as many cells as
  //original MPI partitioning has
  Rect<1> color_bounds(Point<1>(0),Point<1>(num_ranks-1));
  Domain color_domain = Domain::from_rect<1>(color_bounds);

  IndexPartition cells_ip;
  DomainColoring coloring_cells;

  int index=0;
  for (int i = 0; i < num_ranks; i++) {
    flecsi::dmp::parts received = fm.get_result<flecsi::dmp::parts>(
          DomainPoint::from_point<2>(make_point(i,0)));
    int num_elmts = received.exclusive+received.shared;
    assert((index+num_elmts) <= num_cells);
    Rect<1> subrect(Point<1>(index),Point<1>(index+num_elmts-1));
    coloring_cells[i] = Domain::from_rect<1>(subrect);
    index += num_elmts;
  }//end for

  cells_ip = context_.runtime()->create_index_partition(
           context_.context(),
           cells_is,
           color_domain,
           coloring_cells,
           true/*disjoint*/);

  LogicalPartition cells_lp = context_.runtime()->get_logical_partition(
           context_.context(), cells_lr, cells_ip);
  context_.runtime()->attach_name(cells_lp, "cells_lp");

  //next we create 1d partitioning of the cells_part_lr (by the number
  //of MPI processes

  IndexPartition cells_temp_ip1d;
  DomainColoring coloring_cells_temp;

  for (int i = 0; i < num_ranks; i++) {
    Rect<2> subrect(make_point(i,0),make_point(i,max_cells));
    coloring_cells_temp[i] = Domain::from_rect<2>(subrect);
  }//end for

  cells_temp_ip1d = context_.runtime()->create_index_partition(
           context_.context(),
           cells_temp_is,
           color_domain,
           coloring_cells_temp,
           true/*disjoint*/);

  LogicalPartition cells_temp_lp = context_.runtime()->get_logical_partition(
           context_.context(), cell_temp_lr, cells_temp_ip1d);
  context_.runtime()->attach_name(cells_temp_lp, "cells_temp_lp");


  //index task that fills global cells array from cells_partitioning array
  Domain launch_domain = color_domain;
  LegionRuntime::HighLevel::IndexLauncher fill_cells_launcher(
    task_ids_t::instance().init_cells_global_task_id,
    color_domain,
    LegionRuntime::HighLevel::TaskArgument(0, 0),
    arg_map);
   // launch_domain,
  fill_cells_launcher.add_region_requirement(
      RegionRequirement(cells_lp, 0/*projection ID*/,
                        WRITE_DISCARD, EXCLUSIVE, cells_lr));
  fill_cells_launcher.add_field(0,FID_GLOBAL_CELL);

  fill_cells_launcher.add_region_requirement(
    RegionRequirement(cells_temp_lp, 0,
                      READ_ONLY, EXCLUSIVE, cell_temp_lr));
  fill_cells_launcher.region_requirements[1].add_field(FID_CELL_PART);

  FutureMap fm_cells = context_.runtime()->execute_index_space(
             context_.context(), fill_cells_launcher);

  fm_cells.wait_all_results();

  //printing results
  {
    RegionRequirement req(cells_lr, READ_WRITE, EXCLUSIVE, cells_lr);
    req.add_field(FID_GLOBAL_CELL);
  
    std::cout << "Back in driver (TTL) and checking values in Sells GlobalLR"
       << std::endl;
    InlineLauncher cell_launcher(req);
    PhysicalRegion cell_region =
      context_.runtime()->map_region(context_.context(), cell_launcher);
    cell_region.wait_until_valid();
    RegionAccessor<AccessorType::Generic, int> acc_cell =
      cell_region.get_field_accessor(FID_GLOBAL_CELL).typeify<int>();

    for (int i=0; i< num_cells; i++)
    {
      int value =
          acc_cell.read(DomainPoint::from_point<1>(make_point(i)));
      std::cout << "cells_global[ " <<i<<" ] = " << value <<std::endl;
    }//end for
  }
  
  // Partition the cells based on the values
  // Partitioning for the exclusive cells


  //  Partitioning for the shared cells
  //  Partitioning for the ghosts cells

  
  
  
} // driver

} // namespace execution
} // namespace flecsi

#endif // flecsi_sprint_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
