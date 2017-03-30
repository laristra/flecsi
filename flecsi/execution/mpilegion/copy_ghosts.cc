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
 * Copyright (c) 2017 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~--------------------------------------------------------------------------~*/

#include <iostream>

#include "flecsi/execution/context.h"
#include "flecsi/partition/index_partition.h"
#include "flecsi/execution/mpilegion/copy_ghosts.h"

using namespace LegionRuntime::HighLevel;
using namespace LegionRuntime::Accessor;

namespace flecsi {
namespace execution {
namespace mpilegion {

void
size_t_copy_task(
  const Legion::Task *task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx, Legion::HighLevelRuntime *runtime
)
{
    using generic_type = LegionRuntime::Accessor::AccessorType::Generic;
    using field_id = LegionRuntime::HighLevel::FieldID;

    assert(regions.size() == 2);
    assert(task->regions.size() == 2);
    assert(task->regions[0].privilege_fields.size() == 1);
    assert(task->regions[1].privilege_fields.size() == 1);

    field_id fid = *(task->regions[0].privilege_fields.begin());

    LegionRuntime::Accessor::RegionAccessor<generic_type, size_t>
    acc_shared= regions[0].get_field_accessor(fid).typeify<size_t>();
    IndexIterator itr_shared(runtime, ctx, regions[0].get_logical_region());
    std::set<ptr_t> shared_pts;  // TODO profile this or switch to dense storage
    while(itr_shared.has_next())
    	shared_pts.insert(itr_shared.next());

    LegionRuntime::Accessor::RegionAccessor<generic_type, size_t>
    acc_ghost= regions[1].get_field_accessor(fid).typeify<size_t>();
    IndexIterator itr_ghost(runtime, ctx, regions[1].get_logical_region());
    while(itr_ghost.has_next()){
      ptr_t ptr = itr_ghost.next();
      if (shared_pts.count(ptr))
    	  acc_ghost.write(ptr, acc_shared.read(ptr));
    }
}

void
double_copy_task(
  const Legion::Task *task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx, Legion::HighLevelRuntime *runtime
)
{
    using generic_type = LegionRuntime::Accessor::AccessorType::Generic;
    using field_id = LegionRuntime::HighLevel::FieldID;

    assert(regions.size() == 2);
    assert(task->regions.size() == 2);
    assert(task->regions[0].privilege_fields.size() == 1);
    assert(task->regions[1].privilege_fields.size() == 1);

    field_id fid = *(task->regions[0].privilege_fields.begin());

    LegionRuntime::Accessor::RegionAccessor<generic_type, double>
    acc_shared= regions[0].get_field_accessor(fid).typeify<double>();
    IndexIterator itr_shared(runtime, ctx, regions[0].get_logical_region());
    std::set<ptr_t> shared_pts;  // TODO profile this or switch to dense storage
    while(itr_shared.has_next())
      shared_pts.insert(itr_shared.next());

    LegionRuntime::Accessor::RegionAccessor<generic_type, double>
    acc_ghost= regions[1].get_field_accessor(fid).typeify<double>();
    IndexIterator itr_ghost(runtime, ctx, regions[1].get_logical_region());
    while(itr_ghost.has_next()){
      ptr_t ptr = itr_ghost.next();
      if (shared_pts.count(ptr))
        acc_ghost.write(ptr, acc_shared.read(ptr));
    }
}

} // namespace mpilegion
} // namespace execution
} // namespace flecsi

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/

