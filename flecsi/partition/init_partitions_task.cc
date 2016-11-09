/*~--------------------------------------------------------------------------~*
 *  @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
 * /@@/////  /@@          @@////@@ @@////// /@@
 * /@@       /@@  @@@@@  @@    // /@@       /@@
 * /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
 * /@@////   /@@/@@@@@@@/@@       ////////@@/@@
 * /@@       /@@/@@//// //@@    @@       /@@/@@
 * /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
 * //       ///  //////   //////  ////////  //
 *
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~--------------------------------------------------------------------------~*/

#include <iostream>

//#include "flecsi/utils/common.h"
#include "flecsi/execution/context.h"
//#include "flecsi/execution/execution.h"
#include "flecsi/partition/index_partition.h"
#include "flecsi/partition/init_partitions_task.h"

namespace flecsi {
namespace dmp {


parts
get_numbers_of_cells_task(
  const Legion::Task *task, 
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx, Legion::HighLevelRuntime *runtime) 
{
  struct parts partitions; 
  using index_partition_t = index_partition__<size_t>;

  flecsi::execution::context_t & context_ =
    flecsi::execution::context_t::instance();
  index_partition_t ip =
    context_.interop_helper_.data_storage_[0];
  
#if 0
  int rank; 
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  std::cout << "in get_numbers_of_cells native legion task rank is " <<
       rank <<  std::endl;

  for(size_t i=0; i<ip.primary.size(); i++) {
    auto element = ip.primary[i];
    std::cout << " Found ghost elemment: " << element <<
        " on rank: " << rank << std::endl;
  }
    
  for(size_t i=0; i< ip.exclusive.size();i++ ) {
    auto element = ip.exclusive[i];
    std::cout << " Found exclusive elemment: " << element << 
        " on rank: " << rank << std::endl;
  }
  
  for(size_t i=0; i< ip.shared.size(); i++) {
    auto element = ip.shared_id(i);
    std::cout << " Found shared elemment: " << element << 
        " on rank: " << rank << std::endl;
  } 

  for(size_t i=0; i<ip.ghost.size(); i++) {
    auto element = ip.ghost_id(i);
    std::cout << " Found ghost elemment: " << element << 
        " on rank: " << rank << std::endl;
  }

#endif
  
  partitions.primary = ip.primary.size();
  partitions.exclusive = ip.exclusive.size();
  partitions.shared = ip.shared.size();
  partitions.ghost = ip.ghost.size();

  std::cout << "about to return partitions (primary,exclusive,shared,ghost) ("
            << partitions.primary << "," 
            <<partitions.exclusive << "," << partitions.shared << "," 
            << partitions.ghost << ")" << std::endl;

  return partitions; 
}//get_numbers_of_cells_task

void
init_cells_task(
  const Legion::Task *task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx, Legion::HighLevelRuntime *runtime
)
{

  assert(regions.size() == 1);
  assert(task->regions.size() == 1);
  assert(task->regions[0].privilege_fields.size() == 1);
  std::cout << "Here I am in init_cells" << std::endl;

  using index_partition_t = index_partition__<size_t>;

  flecsi::execution::context_t & context_ =
    flecsi::execution::context_t::instance();
  index_partition_t ip =
    context_.interop_helper_.data_storage_[0];


  LegionRuntime::HighLevel::LogicalRegion lr = regions[0].get_logical_region();
  LegionRuntime::HighLevel::IndexSpace is = lr.get_index_space();

  LegionRuntime::HighLevel::IndexIterator itr(runtime, ctx, is);

  auto ac = regions[0].get_field_accessor(0).typeify<int>();

  for(size_t i = 0; i <ip.primary.size() ; ++i){
    assert(itr.has_next());
    size_t id = ip.primary[i];
    ptr_t ptr = itr.next();
    ac.write(ptr, id);
  }//end for

}//init_cells_task

} // namespace dmp
} // namespace flecsi

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/

