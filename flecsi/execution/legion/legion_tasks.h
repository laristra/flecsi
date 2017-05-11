/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_legion_legion_tasks_h
#define flecsi_execution_legion_legion_tasks_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Jul 26, 2016
//----------------------------------------------------------------------------//

#include <legion.h>
#include <legion_utilities.h>

#include "flecsi/execution/legion/internal_task.h"

#define PRIMARY_PART 0
#define GHOST_PART 1
#define EXCLUSIVE_PART 0
#define SHARED_PART 1

clog_register_tag(legion_tasks);

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
//! @def __flecsi_internal_legion_task
//!
//! This macro simplifies pure Legion task definitions by filling in the
//! boiler-plate function arguments.
//!
//! @param task_name   The plain-text task name.
//! @param return_type The return type of the task.
//!
//! @ingroup legion-execution
//----------------------------------------------------------------------------//

#define __flecsi_internal_legion_task(task_name, return_type)                  \
/* MACRO IMPLEMENTATION */                                                     \
                                                                               \
/* Legion task template */                                                     \
inline return_type task_name(                                                  \
  const LegionRuntime::HighLevel::Task * task,                                 \
  const std::vector<LegionRuntime::HighLevel::PhysicalRegion> & regions,       \
  LegionRuntime::HighLevel::Context ctx,                                       \
  LegionRuntime::HighLevel::HighLevelRuntime * runtime                         \
)

//----------------------------------------------------------------------------//
//! Initial SPMD task.
//!
//! @ingroup legion-execution
//----------------------------------------------------------------------------//

__flecsi_internal_legion_task(spmd_task, void) {
  const int my_color = task->index_point.point_data[0];

  {
  clog_tag_guard(legion_tasks);
  clog(info) << "Executing spmd task" << std::endl;
  }

  // Add additional setup.
  context_t & context_ = context_t::instance();

  clog_assert(task->arglen > 0, "spmd_task called without arguments");

  Legion::Deserializer args_deserializer(task->args, task->arglen);
  size_t num_handles;
  args_deserializer.deserialize(&num_handles, sizeof(size_t));

  clog_assert(regions.size() >= num_handles, "fewer regions than data handles");
  clog_assert(task->regions.size() >= num_handles, "fewer regions than data handles");

  Legion::PhaseBarrier* pbarriers_as_master = (Legion::PhaseBarrier*)
      malloc(sizeof(Legion::PhaseBarrier) * num_handles);
  args_deserializer.deserialize((void*)pbarriers_as_master, sizeof(Legion::PhaseBarrier) * num_handles);

  size_t* num_owners = (size_t*)malloc(sizeof(size_t) * num_handles);
  args_deserializer.deserialize((void*)num_owners, sizeof(size_t) * num_handles);

  context_.set_pbarriers_as_masters(pbarriers_as_master);

  size_t region_index = 0;
  for (size_t handle_idx = 0; handle_idx < num_handles; handle_idx++) {

    Legion::PhaseBarrier* ghost_owners_pbarriers_buf = (Legion::PhaseBarrier*)
        malloc(sizeof(Legion::PhaseBarrier) * num_owners[handle_idx]);
    args_deserializer.deserialize((void*)ghost_owners_pbarriers_buf, sizeof(Legion::PhaseBarrier) * num_owners[handle_idx]);

    context_.push_ghost_owners_pbarriers(ghost_owners_pbarriers_buf);

    context_.push_region(regions[region_index].get_logical_region());

    const std::unordered_map<size_t, flecsi::coloring::coloring_info_t> coloring_info_map
      = context_.coloring_info(handle_idx);  // FIX_ME what if the keys are not 0,1,2,...

    auto itr = coloring_info_map.find(my_color);
    clog_assert(itr != coloring_info_map.end(), "Can't find partition info for my color");
    const flecsi::coloring::coloring_info_t coloring_info = itr->second;

    clog(error) << my_color << " handle " << handle_idx <<
        " exclusive " << coloring_info.exclusive <<
        " shared " << coloring_info.shared <<
        " ghost " << coloring_info.ghost << std::endl;

    Legion::IndexSpace color_ispace = regions[region_index].get_logical_region().get_index_space();
    LegionRuntime::Arrays::Rect<1> color_bounds_1D(0,1);
    Legion::Domain color_domain_1D = Legion::Domain::from_rect<1>(color_bounds_1D);

    Legion::DomainColoring primary_ghost_coloring;
    LegionRuntime::Arrays::Rect<2> primary_rect(LegionRuntime::Arrays::make_point(my_color, 0),
        LegionRuntime::Arrays::make_point(my_color, coloring_info.exclusive + coloring_info.shared - 1));
    primary_ghost_coloring[PRIMARY_PART] = Legion::Domain::from_rect<2>(primary_rect);
    LegionRuntime::Arrays::Rect<2> ghost_rect(LegionRuntime::Arrays::make_point(my_color, coloring_info.exclusive + coloring_info.shared),
        LegionRuntime::Arrays::make_point(my_color, coloring_info.exclusive + coloring_info.shared
            + coloring_info.ghost - 1));
    primary_ghost_coloring[GHOST_PART] = Legion::Domain::from_rect<2>(ghost_rect);

    Legion::IndexPartition primary_ghost_ip = runtime->create_index_partition(ctx,
        color_ispace, color_domain_1D, primary_ghost_coloring, true /*disjoint*/);

    context_.push_primary_ghost_ip(primary_ghost_ip);

    Legion::LogicalPartition primary_ghost_lp = runtime->get_logical_partition(ctx,
        regions[region_index].get_logical_region(), primary_ghost_ip);

    context_.push_primary_lr(runtime->get_logical_subregion_by_color(ctx,
      primary_ghost_lp, PRIMARY_PART));

    context_.push_ghost_lr(runtime->get_logical_subregion_by_color(ctx,
      primary_ghost_lp, GHOST_PART));

  } // for handle_idx

  // Get the input arguments from the Legion runtime
  const LegionRuntime::HighLevel::InputArgs & args =
    LegionRuntime::HighLevel::HighLevelRuntime::get_input_args();

  // Set the current task context to the driver
  context_t::instance().push_state(utils::const_string_t{"driver"}.hash(),
    ctx, runtime, task, regions);


#if 0
  const std::unordered_map<size_t, flecsi::coloring::index_coloring_t> coloring_map
    = context_.coloring_map();

  {
  clog_tag_guard(legion_tasks);

  // Create sub-partitions
  for (auto itr : coloring_map) {
    for (auto primary_itr = itr.second.primary.begin(); primary_itr != itr.second.primary.end(); ++primary_itr)
      clog(error) << my_color << " key " << itr.first << " primary " <<
        " " << *primary_itr << std::endl;
    for (auto exclusive_itr = itr.second.exclusive.begin(); exclusive_itr != itr.second.exclusive.end(); ++exclusive_itr)
      clog(error) << my_color << " key " << itr.first << " exclusive " <<
        " " <<  *exclusive_itr << std::endl;
    for (auto shared_itr = itr.second.shared.begin(); shared_itr != itr.second.shared.end(); ++shared_itr)
      clog(error) << my_color << " key " << itr.first << " shared " <<
        " " <<  *shared_itr << std::endl;
    for (auto ghost_itr = itr.second.ghost.begin(); ghost_itr != itr.second.ghost.end(); ++ghost_itr)
      clog(error) << my_color << " key " << itr.first << " ghost " <<
        " " << *ghost_itr << std::endl;
  }
  }
#endif
  // run default or user-defined driver 
  driver(args.argc, args.argv); 

  // Set the current task context to the driver
  context_t::instance().pop_state(utils::const_string_t{"driver"}.hash());
} // spmd_task

//----------------------------------------------------------------------------//
//! Interprocess communication to pass control to MPI runtime.
//!
//! @ingroup legion-execution
//----------------------------------------------------------------------------//

__flecsi_internal_legion_task(handoff_to_mpi_task, void) {
  context_t::instance().handoff_to_mpi();
} // handoff_to_mpi_task

//----------------------------------------------------------------------------//
//! Interprocess communication to wait for control to pass back to the Legion
//! runtime.
//!
//! @ingroup legion-execution
//----------------------------------------------------------------------------//

__flecsi_internal_legion_task(wait_on_mpi_task, void) {
  context_t::instance().wait_on_mpi();
} // wait_on_mpi_task

//----------------------------------------------------------------------------//
//! Interprocess communication to unset mpi execute state.
//!
//! @ingroup legion-execution
//----------------------------------------------------------------------------//

__flecsi_internal_legion_task(unset_call_mpi_task, void) {
  context_t::instance().set_mpi_state(false);
} // unset_call_mpi_task

#undef __flecsi_internal_legion_task

} // namespace execution 
} // namespace flecsi

#endif // flecsi_execution_legion_legion_tasks_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
