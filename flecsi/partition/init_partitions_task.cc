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


parts  init_partitions(const Legion::Task *task, 
        const std::vector<Legion::PhysicalRegion> & regions,
        Legion::Context ctx, Legion::HighLevelRuntime *runtime) 
{

  assert(regions.size() == 1);
  assert(task->regions.size() == 1);
  assert(task->regions[0].privilege_fields.size() == 1);
  std::cout << "Here I am in init_partitions" << std::endl; 

  struct parts partitions; 
#if 1
  using index_partition_t = index_partition__<size_t>;

  flecsi::execution::context_t & context_ =
    flecsi::execution::context_t::instance();
  index_partition_t ip =
    context_.interop_helper_.data_storage_[0];
  
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
  LegionRuntime::HighLevel::Domain dom = runtime->get_index_space_domain(ctx, task->regions[0].region.get_index_space());
  LegionRuntime::Arrays::Rect<2> rect = dom.get_rect<2>();
  LegionRuntime::HighLevel::FieldID fid = *(task->regions[0].privilege_fields.begin());
  LegionRuntime::Accessor::RegionAccessor<LegionRuntime::Accessor::AccessorType::Generic, int> acc_part =
    regions[0].get_field_accessor(fid).typeify<int>();

  GenericPointInRectIterator<2> pir(rect);
  for(auto & element : ip.exclusive) {
    if(pir)
      pir++;
    else
      abort();

    acc_part.write(LegionRuntime::HighLevel::DomainPoint::from_point<2>(pir.p), element);
  }
  
#endif
  partitions.exclusive = ip.exclusive.size();
  partitions.shared = ip.shared.size();
  partitions.ghost = ip.ghost.size();

  std::cout << "about to return partitions (exclusive,shared,ghost) ("
            << partitions.exclusive << "," << partitions.shared << "," 
            << partitions.ghost << ")" << std::endl;
  return partitions; 

#if 0

  assert(regions.size() == 1);
  assert(task->regions.size() == 1);
  assert(task->regions[0].privilege_fields.size() == 1);
  std::cout << "Here I am in init_partitions" << std::endl;

//#if 1
  flecsi::execution::context_t & context_ =
             flecsi::execution::context_t::instance();
  auto array =
    context_.interop_helper_.data_storage_[0];

  using index_partition_t = index_partition__<size_t>;

  //array__<std::shared_ptr<index_partition_t>, 3> array2;
  //array2   = *array;
  index_partition_t ip = (*array)[0];
//#endif

//#if 1
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  std::cout << "in init_partitions native legion task rank is " <<
               rank <<  std::endl;



  for(auto & element : ip.exclusive ) {
    std::cout << " Found exclusive elemment: " << element <<
               " on rank: " << rank << std::endl;
  }

  for(auto & element : ip.shared) {
    std::cout << " Found shared elemment: " << element << 
                  " on rank: " << rank << std::endl;
  }

  for(auto & element : ip.ghost) {
    std::cout << " Found ghost elemment: " << element << 
                 " on rank: " << rank << std::endl;
  }

  // Now that we have the index partitioning let's push it into a logical
  //  region that can be used at the top level task (once we return) to
  //  allow creation of the proper index partitions 
  LegionRuntime::HighLevel::Domain dom = 
               runtime->get_index_space_domain(
                ctx, task->regions[0].region.get_index_space());
  LegionRuntime::Arrays::Rect<2> rect = dom.get_rect<2>();
  LegionRuntime::HighLevel::FieldID fid =
                          *(task->regions[0].privilege_fields.begin());
  LegionRuntime::Accessor::RegionAccessor<
    LegionRuntime::Accessor::AccessorType::Generic, int> acc_part =
            regions[0].get_field_accessor(fid).typeify<int>();
 
  GenericPointInRectIterator<2> pir(rect);
  for(auto & element : ip.exclusive) {
    if(pir)
      pir++; 
    else
      abort();
    
    acc_part.write(LegionRuntime::HighLevel::DomainPoint::from_point<2>(pir.p), element);
  }
    
#endif

}

} // namespace dmp
} // namespace flecsi

