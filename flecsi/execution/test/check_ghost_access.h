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
  accessor_t<size_t> acc_cells,
  int my_color,
  int cycle
)
{
  std::cout << my_color << " as master writes data; phase 1 of cycle " << cycle << std::endl;
  // ndm - how do I pass int cycle?
}

flecsi_register_task(shared_write_task, loc, single);


void
ghost_read_task(
  accessor_t<size_t> acc_cells,
  int my_color,
  int cycle
)
{
  std::cout << my_color << " as slave reads data; phase 2 of cycle " << cycle <<  std::endl;
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
    flecsi_get_handle(dc, sprint, cell_ID, size_t, dense, index_space, none, ro, none);  // FIXME rw
  auto ghost_read_handle =
    flecsi_get_handle(dc, sprint, cell_ID, size_t, dense, index_space, none, none, ro);

  for (int cycle = 0; cycle < 2; cycle++) {

    // phase WRITE: masters update their halo regions; slaves may not access data

    std::cout << my_color << " write phase " << cycle << std::endl;
    flecsi_execute_task(shared_write_task, loc, single, shared_write_handle, my_color, cycle);

    // phase READ: slaves can read data; masters may not write to data

    std::cout << my_color << " read phase " << cycle << std::endl;
    flecsi_execute_task(ghost_read_task, loc, single, ghost_read_handle, my_color, cycle);

  }

} //driver



#endif // flecsi_execution_check_ghost_access_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
