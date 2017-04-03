/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

#include "flecsi/execution/legion/internal_task.h"
#include "cinchlog.h"

///
/// \file
/// \date Initial file creation: Apr 01, 2017
///

namespace flecsi {
namespace execution {

int internal_task_example_1(const LegionRuntime::HighLevel::Task * task,
	const std::vector<LegionRuntime::HighLevel::PhysicalRegion> & regions,
	LegionRuntime::HighLevel::Context context,
	LegionRuntime::HighLevel::HighLevelRuntime * runtime) {
} // internal_task_example

flecsi_register_legion_task(internal_task_example_1, loc, true, false);

int internal_task_example_2(const LegionRuntime::HighLevel::Task * task,
	const std::vector<LegionRuntime::HighLevel::PhysicalRegion> & regions,
	LegionRuntime::HighLevel::Context context,
	LegionRuntime::HighLevel::HighLevelRuntime * runtime) {
} // internal_task_example

flecsi_register_legion_task(internal_task_example_2, toc, false, true);

void driver(int argc, char ** argv) {

	auto key_1 = flecsi_make_legion_task_key(internal_task_example_1,
		loc, true, false);
	auto key_2 = flecsi_make_legion_task_key(internal_task_example_2,
		toc, false, true);

	auto tid_1 = context_t::instance().task_id(key_1);
	auto tid_2 = context_t::instance().task_id(key_2);

	clog(info) << "Task ID: " << tid_1 << std::endl;
	clog(info) << "Task ID: " << tid_2 << std::endl;

} // driver

} // namespace execution
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
