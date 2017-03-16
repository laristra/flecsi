/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_test_somethingelse_h
#define flecsi_execution_test_somethingelse_h

using namespace LegionRuntime::HighLevel;

#include <legion.h>
///
/// \file
/// \date Initial file creation: Mar 14, 2017
///

#define DH1 1
#undef flecsi_execution_legion_task_wrapper_h
#include "flecsi/execution/legion/task_wrapper.h"

#include <iostream>
#include <vector>

#include "flecsi/utils/common.h"
#include "flecsi/execution/context.h"
#include "flecsi/execution/execution.h"
#include "flecsi/data/data.h"
#include "flecsi/data/data_client.h"
#include "flecsi/data/legion/data_policy.h"
#include "flecsi/execution/legion/helper.h"
#include "flecsi/execution/task_ids.h"

#include "flecsi/execution/test/mpilegion/sprint_common.h"

template<typename T>
using accessor_t = flecsi::data::legion::dense_accessor_t<T, flecsi::data::legion_meta_data_t<flecsi::default_user_meta_data_t> >;

// FIXME remove duplicate task for init_partitions_task.cc
void
check_partitioning_task(
  accessor_t<size_t> acc_cells
// accessor_t<size_t> acc_cells,
)
{
#if 0
  using index_partition_t = flecsi::dmp::index_partition__<size_t>;
  using generic_type = LegionRuntime::Accessor::AccessorType::Generic;
  using field_id = LegionRuntime::HighLevel::FieldID;

  //checking cells:
  {
    LegionRuntime::HighLevel::LogicalRegion lr_shared =
       regions[0].get_logical_region();
   LegionRuntime::HighLevel::IndexSpace is_shared = lr_shared.get_index_space();
    LegionRuntime::HighLevel::IndexIterator itr_shared(runtime, ctx, is_shared);
    field_id fid_shared = *(task->regions[0].privilege_fields.begin());
    LegionRuntime::Accessor::RegionAccessor<generic_type, size_t>
      acc_shared= regions[0].get_field_accessor(fid_shared).typeify<size_t>();

    LegionRuntime::HighLevel::LogicalRegion lr_exclusive =
        regions[1].get_logical_region();
    LegionRuntime::HighLevel::IndexSpace is_exclusive =
        lr_exclusive.get_index_space();
    LegionRuntime::HighLevel::IndexIterator itr_exclusive(runtime,
        ctx, is_exclusive);
    field_id fid_exclusive = *(task->regions[1].privilege_fields.begin());
    LegionRuntime::Accessor::RegionAccessor<generic_type, size_t>
      acc_exclusive=regions[1].get_field_accessor(
          fid_exclusive).typeify<size_t>();


    LegionRuntime::HighLevel::LogicalRegion lr_ghost =
        regions[2].get_logical_region();
    LegionRuntime::HighLevel::IndexSpace is_ghost = lr_ghost.get_index_space();
    LegionRuntime::HighLevel::IndexIterator itr_ghost(runtime, ctx, is_ghost);
    field_id fid_ghost = *(task->regions[2].privilege_fields.begin());
    LegionRuntime::Accessor::RegionAccessor<generic_type, size_t>
      acc_ghost= regions[2].get_field_accessor(fid_ghost).typeify<size_t>();

    flecsi::execution::context_t & context_ =
      flecsi::execution::context_t::instance();
    index_partition_t ip =
      context_.interop_helper_.data_storage_[0];

    size_t indx = 0;
    for (auto shared_cell : ip.shared) {
    assert(itr_shared.has_next());
     ptr_t ptr=itr_shared.next();
     assert(shared_cell.id == acc_shared.read(ptr));
     indx++;
    }
    assert (indx == ip.shared.size());

    indx = 0;
    for (auto exclusive_cell : ip.exclusive) {
     assert(itr_exclusive.has_next());
     ptr_t ptr=itr_exclusive.next();
     assert(exclusive_cell.id == acc_exclusive.read(ptr));
    indx++;
    }
    assert (indx == ip.exclusive.size());


    indx = 0;
    while(itr_ghost.has_next()){
      ptr_t ptr = itr_ghost.next();
      bool found=false;
      size_t ghost_id = acc_ghost.read(ptr);
      for (auto ghost_cell : ip.ghost) {
        if (ghost_cell.id == ghost_id){
          found=true;
        }
      }
      assert(found);
      indx++;
     }
     assert (indx == ip.ghost.size());
    }//scope

    //checking vertices
    {
     LegionRuntime::HighLevel::LogicalRegion lr_shared =
       regions[3].get_logical_region();
    LegionRuntime::HighLevel::IndexSpace is_shared=lr_shared.get_index_space();
    LegionRuntime::HighLevel::IndexIterator itr_shared(runtime, ctx, is_shared);
    field_id fid_shared = *(task->regions[3].privilege_fields.begin());
    LegionRuntime::Accessor::RegionAccessor<generic_type, size_t>
      acc_shared= regions[3].get_field_accessor(fid_shared).typeify<size_t>();

    LegionRuntime::HighLevel::LogicalRegion lr_exclusive =
        regions[4].get_logical_region();
    LegionRuntime::HighLevel::IndexSpace is_exclusive =
        lr_exclusive.get_index_space();
    LegionRuntime::HighLevel::IndexIterator itr_exclusive(runtime,
        ctx, is_exclusive);
    field_id fid_exclusive = *(task->regions[4].privilege_fields.begin());
    LegionRuntime::Accessor::RegionAccessor<generic_type, size_t>
      acc_exclusive=regions[4].get_field_accessor(
      fid_exclusive).typeify<size_t>();


    LegionRuntime::HighLevel::LogicalRegion lr_ghost =
        regions[5].get_logical_region();
    LegionRuntime::HighLevel::IndexSpace is_ghost = lr_ghost.get_index_space();
    LegionRuntime::HighLevel::IndexIterator itr_ghost(runtime, ctx, is_ghost);
    field_id fid_ghost = *(task->regions[5].privilege_fields.begin());
    LegionRuntime::Accessor::RegionAccessor<generic_type, size_t>
      acc_ghost= regions[5].get_field_accessor(fid_ghost).typeify<size_t>();

    flecsi::execution::context_t & context_ =
      flecsi::execution::context_t::instance();
    index_partition_t ip =
      context_.interop_helper_.data_storage_[1];

    size_t indx = 0;
    for (auto shared_cell : ip.shared) {
    assert(itr_shared.has_next());
     ptr_t ptr=itr_shared.next();
     assert(shared_cell.id == acc_shared.read(ptr));
     indx++;
    }
    assert (indx == ip.shared.size());

    indx = 0;
    for (auto exclusive_cell : ip.exclusive) {
     assert(itr_exclusive.has_next());
     ptr_t ptr=itr_exclusive.next();
     assert(exclusive_cell.id == acc_exclusive.read(ptr));
    indx++;
    }
    assert (indx == ip.exclusive.size());

    indx = 0;
    while(itr_ghost.has_next()){
      ptr_t ptr = itr_ghost.next();
      bool found=false;
      size_t ghost_id = acc_ghost.read(ptr);
      for (auto ghost_cell : ip.ghost) {
        if (ghost_cell.id == ghost_id){
          found=true;
        }
      }
      assert(found);
      indx++;
     }
     assert (indx == ip.ghost.size());

    }

  std::cout << "test for shared/ghost/exclusive partitions ... passed"
  << std::endl;
#endif
  }//check_partitioning_task


flecsi_register_task(check_partitioning_task, loc, single);

void
driver(
  int argc,
  char ** argv
)
{


  flecsi::data_client& dc = *((flecsi::data_client*)argv[argc - 1]);

  std::cout << "check PARTITION" << std::endl;

  // PAIR_PROGRAMMING
  // This is where the check partitions code does
  // we need a data handle to be able to pass cell_ids and vert_ids to a flecsi task

  int index_space = 0;
  auto h1 =
    flecsi_get_handle(dc, sprint, cell_ID, size_t, dense, index_space, ro, ro, ro);

  flecsi_execute_task(check_partitioning_task, loc, single, h1);
} //driver



#endif // flecsi_execution_test_somethingelse_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
