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
static mpi_legion_interop_t InteropHelper;

static const size_t N = 8;

enum FieldIDs {
  FID_CELL_PART,
};

  
void mpi_task(double val) {
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
  array__<index_partition_t,1> *array =
    new array__<index_partition_t, 1>();

  (*array)[0] = ip; 

#if 0 
  array__<int,3> *array_2 =
    new array__<int,  3>();
  (*array_2)[0] = 1; 
#endif
  
  InteropHelper.data_storage_.push_back(
    std::shared_ptr<mpi_array_storage_t>(array));
  

} // mpi_task
  
register_task(mpi_task, mpi, single, void, double);
  
void init_part_task(double val) {
  // context_t & context_ = context_t::instance();
  // assert((context_.regions()).size() == 1);
  // assert((context_.task())->regions.size() == 1);

  assert(0); 
  int rank; 
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  std::cout << " legion task rank is " << rank << " val is: " << val << std::endl;
  
  auto array =
    InteropHelper.data_storage_[0];

#if 1
  //array__<std::shared_ptr<index_partition_t>, 3> array2;
  //array2   = *array;
  index_partition_t ip = (*array)[0];
#endif

  
  for(auto & element : ip.exclusive ) {
    std::cout << " Found exclusive element: " << element << " on rank: " << rank << std::endl;
  }
  
  for(auto & element : ip.shared) {
    std::cout << " Found shared element: " << element << " on rank: " << rank << std::endl;
  } 

  for(auto & element : ip.ghost) {
    std::cout << " Found ghost element: " << element << " on rank: " << rank << std::endl;
  } 
  
  
  
}   

register_task(init_part_task, loc, index, void, double);

#if 0
struct parts {
  int exclusive;
  int shared;
  int ghost;
};
#endif
  
parts init_partitions(const Legion::Task *task, const std::vector<Legion::PhysicalRegion> & regions,
                     Legion::Context ctx, Legion::HighLevelRuntime *runtime) {

//void init_partitions(const Task *task, const std::vector<PhysicalRegion> & regions,
//                       Context ctx, HighLevelRuntime *runtime) {

  assert(regions.size() == 1);
  assert(task->regions.size() == 1);
  assert(task->regions[0].privilege_fields.size() == 1);
  std::cout << "Here I am in init_partitions" << std::endl; 

  struct parts partitions; 
#if 1
  auto array =
    InteropHelper.data_storage_[0];

//array__<std::shared_ptr<index_partition_t>, 3> array2;
  //array2   = *array;
  index_partition_t ip = (*array)[0];
#endif

#if 1
  int rank; 
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  std::cout << "in init_partitions native legion task rank is " << rank <<  std::endl;
    

  
  for(auto & element : ip.exclusive ) {
    std::cout << " Found exclusive elemment: " << element << " on rank: " << rank << std::endl;
  }
  
  for(auto & element : ip.shared) {
    std::cout << " Found shared elemment: " << element << " on rank: " << rank << std::endl;
  } 

  for(auto & element : ip.ghost) {
    std::cout << " Found ghost elemment: " << element << " on rank: " << rank << std::endl;
  }

  // Now that we have the index partitioning let's push it into a logical
  //  region that can be used at the top level task (once we return) to
  //  allow creation of the proper index partitions 
  Domain dom = runtime->get_index_space_domain(ctx, task->regions[0].region.get_index_space());
  Rect<2> rect = dom.get_rect<2>();
  FieldID fid = *(task->regions[0].privilege_fields.begin());
  RegionAccessor<AccessorType::Generic, int> acc_part =
    regions[0].get_field_accessor(fid).typeify<int>();

  GenericPointInRectIterator<2> pir(rect);
  for(auto & element : ip.exclusive) {
    if(pir)
      pir++;
    else
      abort();

    acc_part.write(DomainPoint::from_point<2>(pir.p), element);
  }
  
#endif
  partitions.exclusive = ip.exclusive.size();
  partitions.shared = ip.shared.size();
  partitions.ghost = ip.ghost.size();

  std::cout << "about to return partitions (exclusive,shared,ghost) ("
            << partitions.exclusive << "," << partitions.shared << "," 
            << partitions.ghost << ")" << std::endl;
  return partitions; 
  
}


void driver(int argc, char ** argv) {
  context_t & context_ = context_t::instance();
  parts partitions;
  
  // first execute mpi task to setup initial partitions 
  execute_task(mpi_task, mpi, single, 1.0);
#if 0
  execute_task(init_part_task, loc, index, 2.0);
#endif
  

  // create a field space to store my cell paritioning 
  FieldSpace fs = context_.runtime()->create_field_space(context_.context());
  {
    FieldAllocator allocator = 
      context_.runtime()->create_field_allocator(context_.context(), fs);
    allocator.allocate_field(sizeof(double), FID_CELL_PART);
  }

  int num_ranks;
  MPI_Comm_size(MPI_COMM_WORLD, &num_ranks); 
  int max_cells = 1000;
  Point<2> elem_rect_lo; elem_rect_lo.x[0] = 0; elem_rect_lo.x[1]=0;
  Point<2> elem_rect_hi;
  elem_rect_hi.x[0] = num_ranks;
  elem_rect_hi.x[1] = max_cells;
  Rect<2> elem_rect( elem_rect_lo, elem_rect_hi );
  
  IndexSpace is = context_.runtime()->create_index_space(context_.context(), 
                                              Domain::from_rect<2>(elem_rect));
  
  LogicalRegion cell_parts_lr = context_.runtime()->create_logical_region(context_.context(), is, fs);
  context_.runtime()->attach_name(cell_parts_lr, "cell partitions logical region");
  
  Point<2> cell_part_color; cell_part_color.x[0] = 1; cell_part_color.x[1] = max_cells;
  Blockify<2> coloring(cell_part_color);
  
  IndexPartition ip = context_.runtime()->create_index_partition(context_.context(), is, coloring);
  LogicalPartition cell_parts_lp = context_.runtime()->
    get_logical_partition(context_.context(), cell_parts_lr, ip);
  context_.runtime()->attach_name(cell_parts_lp, "cell partitions logical partition");
  
#if 1 
  LegionRuntime::HighLevel::ArgumentMap arg_map;
   
  LegionRuntime::HighLevel::IndexLauncher init_cell_partitions_launcher(
    INIT_CELL_PARTITIONS_TASK_ID,
    LegionRuntime::HighLevel::Domain::from_rect<2>(context_.interop_helper_.all_processes_), 
    LegionRuntime::HighLevel::TaskArgument(0, 0),
    arg_map);
  
  
  init_cell_partitions_launcher.add_region_requirement(
    RegionRequirement(cell_parts_lp, 0/*projection ID*/,
                      WRITE_DISCARD, EXCLUSIVE, cell_parts_lr));
  init_cell_partitions_launcher.add_field(0, FID_CELL_PART);
  FutureMap fm = context_.runtime()->execute_index_space(context_.context(), init_cell_partitions_launcher);
  
  fm.wait_all_results();

  std::cout << "returned from wait_all_results()" << std::endl;

  for (int i = 0; i < num_ranks; i++) {
    std::cout << "about to call get_results" << std::endl; 
    parts received = fm.get_result<parts>(DomainPoint::from_point<2>(make_point(i,0)));
    std::cout << "From rank " << i << " received (exclusive, shared, ghost) "
              << "(" << received.exclusive << "," << received.shared << ","
              << received.ghost << ")" << std::endl; 
      
  }
  
  // GMS: back at the top level, we need to read this partitioning information
  RegionRequirement req(cell_parts_lr, READ_WRITE, EXCLUSIVE, cell_parts_lr);
  req.add_field(FID_CELL_PART);

#endif

#if 1
  
  std::cout << "Back in driver (TTL) and checking values in LR" << std::endl;
  InlineLauncher cell_parts_launcher(req);
  PhysicalRegion cell_parts_region = context_.runtime()->map_region(context_.context(), cell_parts_launcher);
  cell_parts_region.wait_until_valid();
  RegionAccessor<AccessorType::Generic, int> acc_cell_part =
    cell_parts_region.get_field_accessor(FID_CELL_PART).typeify<int>();
  for (GenericPointInRectIterator<2> pir(elem_rect); pir; pir++) {
    double value = acc_cell_part.read(DomainPoint::from_point<2>(pir.p));
    //std::cout << "partition value is: " << value << std::endl; 
  }
#endif
  
  // Now I need to create an Index space based on this data ..

  // Then partition the cells based on the values..

  // etc.. 

  
  
  
} // driver

} // namespace execution
} // namespace flecsi

#endif // flecsi_sprint_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
