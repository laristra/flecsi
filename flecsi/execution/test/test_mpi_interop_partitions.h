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

#include "flecsi/execution/test/mpilegion/data_client_completion.h"


template<typename T>
using accessor_t = flecsi::data::legion::dense_accessor_t<T, flecsi::data::legion_meta_data_t<flecsi::default_user_meta_data_t> >;

void
check_partitioning_task(
  accessor_t<size_t> acc_cells
// accessor_t<size_t> acc_verts,
)
{
  using index_partition_t = flecsi::dmp::index_partition__<size_t>;

  flecsi::execution::context_t & context_ =
    flecsi::execution::context_t::instance();
  index_partition_t ip =
    context_.interop_helper_.data_storage_[0];

  assert (acc_cells.shared_size() == ip.shared.size());
  size_t indx = 0;
  for (auto shared_cell : ip.shared) {
   assert(shared_cell.id == acc_cells.shared(indx));
   indx++;
  }

  assert (acc_cells.size() == ip.exclusive.size());
  indx = 0;
  for (auto exclusive_cell : ip.exclusive) {
    assert(exclusive_cell.id == acc_cells[indx]);
    indx++;
  }

  assert (acc_cells.ghost_size() == ip.ghost.size());
  for (size_t indx = 0; indx < acc_cells.ghost_size(); indx++) {
    bool found = false;
    size_t ghost_id = acc_cells.ghost(indx);
    for (auto ghost_cell : ip.ghost)
      if (ghost_cell.id == ghost_id)
        found=true;
    assert(found);
  }

  // FIXME copy and check vertices too
  #if 0
    //checking vertices
    {
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

#endif
    std::cout << "flecsi test for shared/ghost/exclusive partitions ... passed"
    << std::endl;
  }//check_partitioning_task


flecsi_register_task(check_partitioning_task, loc, single);

void
driver(
  int argc,
  char ** argv
)
{
  flecsi::data_client& dc = *((flecsi::data_client*)argv[argc - 1]);

  int index_space = 0;
  auto h1 =
    flecsi_get_handle(dc, test, cell_ID, size_t, dense, index_space, rw, ro, ro);

  flecsi_execute_task(check_partitioning_task, loc, single, h1);
} //driver



#endif // flecsi_execution_test_somethingelse_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
