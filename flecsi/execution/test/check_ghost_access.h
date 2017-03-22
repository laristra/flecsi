/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2017 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_check_ghost_access_h
#define flecsi_execution_check_ghost_access_h

using namespace LegionRuntime::HighLevel;

#include <legion.h>
///
/// \file
/// \date Initial file creation: Mar 22, 2017
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

#include "flecsi/execution/test/mpilegion/sprint_common.h"

#include "flecsi/utils/const_string.h"


template<typename T>
using accessor_t = flecsi::data::legion::dense_accessor_t<T, flecsi::data::legion_meta_data_t<flecsi::default_user_meta_data_t> >;


void
shared_write_task(
  accessor_t<size_t> acc_cells
)
{
  // ndm - how do I pass int cycle?
}

flecsi_register_task(shared_write_task, loc, single);


void
ghost_read_task(
  accessor_t<size_t> acc_cells
)
{
  // ndm - how do I pass int cycle?
}

flecsi_register_task(ghost_read_task, loc, single);


void
driver(
  int argc,
  char ** argv
)
{
  flecsi::execution::context_t & context_ = flecsi::execution::context_t::instance();
  const LegionRuntime::HighLevel::Task *task = context_.task(flecsi::utils::const_string_t{"driver"}.hash());
  const int my_color = task->index_point.point_data[0];
	std::cout << my_color << " check GHOST" << std::endl;

	flecsi::data_client& dc = *((flecsi::data_client*)argv[argc - 1]);

  int index_space = 0;
  auto shared_write_handle =
    flecsi_get_handle(dc, sprint, cell_ID, size_t, dense, index_space, none, rw, none);
  auto ghost_read_handle =
    flecsi_get_handle(dc, sprint, cell_ID, size_t, dense, index_space, none, none, ro);

  for (int cycle = 0; cycle < 2; cycle++) {

    // phase WRITE: masters update their halo regions; slaves may not access data

    flecsi_execute_task(shared_write_task, loc, single, shared_write_handle); // ndm - how do I pass int cycle?

    // phase READ: slaves can read data; masters may not write to data

    //bool read_phase = true;
    //bool write_phase = false;
    /*if (read_phase) {
        if (!is_readable) {
    // as master
    spmd_args->pbarrier_as_master.arrive(1);                                                // phase WRITE
    spmd_args->pbarrier_as_master =
            runtime->advance_phase_barrier(ctx, spmd_args->pbarrier_as_master);               // phase WRITE

    // as slave

    for (int master=0; master < spmd_args->masters_pbarriers.size(); master++) {
      .add_wait_barrier(spmd_args->masters_pbarriers[master]);
      .add_arrival_barrier(spmd_args->masters_pbarriers[master]);
            spmd_args->masters_pbarriers[master] =
              runtime->advance_phase_barrier(ctx, spmd_args->masters_pbarriers[master]);  // phase WRITE
    } // for master as slave

    is_readable = true;
  } // if !is_readable

    */
    //if (write_phase)
    //  write_prologue(task_launcher);

    flecsi_execute_task(ghost_read_task, loc, single, ghost_read_handle); // ndm - how do I pass int cycle?

    //if (write_phase)
    //  write_epilogue(ctx, runtime);
  }

} //driver



#endif // flecsi_execution_check_ghost_access_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
