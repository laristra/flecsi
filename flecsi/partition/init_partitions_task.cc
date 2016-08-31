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

void init_partitions(const Legion::Task *task, 
        const std::vector<Legion::PhysicalRegion> & regions,
        Legion::Context ctx, Legion::HighLevelRuntime *runtime) 
{
  assert(regions.size() == 1);
  assert(task->regions.size() == 1);
  assert(task->regions[0].privilege_fields.size() == 1);

  flecsi::execution::context_t & context_ =
             flecsi::execution::context_t::instance();
  auto array =
    context_.interop_helper_.data_storage_[0];

  using index_partition_t = index_partition__<size_t>;

#if 1
  //array__<std::shared_ptr<index_partition_t>, 3> array2;
  //array2   = *array;
  index_partition_t ip = (*array)[0];
#endif

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
  int i = 0;
  for(GenericPointInRectIterator<2> pir(rect); pir; pir++) {
    acc_part.write(LegionRuntime::HighLevel::DomainPoint::from_point<2>(pir.p),
                ip.exclusive[i++]);
  }

}

} // namespace dmp
} // namespace flecsi

