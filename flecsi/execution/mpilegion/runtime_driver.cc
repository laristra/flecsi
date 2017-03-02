/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

/*!
 * \file mpilegion/runtime_driver.cc
 * \authors bergen
 * \date Initial file creation: Jul 26, 2016
 */

#include "flecsi/execution/mpilegion/runtime_driver.h"
#include "flecsi/utils/common.h"
#include "flecsi/utils/logging.h"
#include "flecsi/execution/context.h"

#ifndef FLECSI_DRIVER
  #include "flecsi/execution/default_driver.h"
#else
  #include EXPAND_AND_STRINGIFY(FLECSI_DRIVER)
#endif

#ifndef FLECSI_SPECIALIZATION_DRIVER
  #include "flecsi/execution/default_specialization_driver.h"
#else
  #include EXPAND_AND_STRINGIFY(FLECSI_SPECIALIZATION_DRIVER)
#endif

namespace flecsi {
namespace execution {

void
mpilegion_runtime_driver(
  const LegionRuntime::HighLevel::Task * task,
	const std::vector<LegionRuntime::HighLevel::PhysicalRegion> & regions,
	LegionRuntime::HighLevel::Context ctx,
	LegionRuntime::HighLevel::HighLevelRuntime * runtime
)
{
    std::cout << "mpilegion_runtime_driver started" << std::endl;
                
    context_t & context_ = context_t::instance();
    context_.push_state(utils::const_string_t{"specialization_driver"}.hash(),
      ctx, runtime, task, regions);

    const LegionRuntime::HighLevel::InputArgs & args =
      LegionRuntime::HighLevel::HighLevelRuntime::get_input_args();

    // connect legion with MPI
    context_.interop_helper_.connect_with_mpi(ctx, runtime);
    context_.interop_helper_.wait_on_mpi(ctx, runtime);

    // run default or user-defined specialization driver 
    specialization_driver(args.argc, args.argv);

    //creating phace barriers for SPMD launch from partitions created and 
    //registered to the data Client at the specialization_driver 
    int num_ranks;
    MPI_Comm_size(MPI_COMM_WORLD, &num_ranks);
    std::vector<PhaseBarrier> phase_barriers;
    std::vector<std::set<int>> master_colors(num_ranks);
/*    for (int master_color=0; master_color < num_ranks; ++master_color) {
      std::set<int> slave_colors;
      for (std::set<ptr_t>::iterator
         it = cells_shared_coloring[master_color].points.begin();
         it != cells_shared_coloring[master_color].points.end(); ++it) {
         
         const ptr_t ptr = *it;
         for (int slave_color = 0; slave_color < num_ranks; ++slave_color)
           if (cells_ghost_coloring[slave_color].points.count(ptr)) {
             slave_colors.insert(slave_color);
             master_colors[slave_color].insert(master_color);
          }
       }
    phase_barriers.push_back(runtime->create_phase_barrier(ctx,
      1 + slave_colors.size()));
   }
*/
   //FIXME use Legion's serialized instead SPMDArgsSerializer
/*
   std::vector<execution::sprint::SPMDArgs> spmd_args(num_ranks);
   std::vector<execution::sprint::SPMDArgsSerializer>
           args_seriliazed(num_ranks);
   for (int color=0; color < num_ranks; ++color) {
      spmd_args[color].pbarrier_as_master = phase_barriers[color];
      for (std::set<int>::iterator master=master_colors[color].begin();
              master!=master_colors[color].end(); ++master)
        spmd_args[color].masters_pbarriers.push_back(phase_barriers[*master]);
        args_seriliazed[color].archive(&(spmd_args[color]));
        arg_map.set_point(DomainPoint::from_point<1>(
            LegionRuntime::Arrays::Point<1>(color)),
            TaskArgument(args_seriliazed[color].getBitStream(),
            args_seriliazed[color].getBitStreamSize()));
  }
*/
    //execute SPMD launch that execute user-defined driver


#if 1
    MustEpochLauncher must_epoch_launcher; 
    LegionRuntime::HighLevel::ArgumentMap arg_map;
    LegionRuntime::HighLevel::IndexLauncher spmd_launcher(
      task_ids_t::instance().spmd_task_id,
      LegionRuntime::HighLevel::Domain::from_rect<1>(
         context_.interop_helper_.all_processes_),
      TaskArgument(0,0), arg_map);

    spmd_launcher.tag = MAPPER_FORCE_RANK_MATCH;
    must_epoch_launcher.add_index_task(spmd_launcher);
 
    FutureMap fm = runtime->execute_must_epoch(ctx,must_epoch_launcher);
    fm.wait_all_results();

//FIXME add region requirements for logical partitions from data client
#if 0
    spmd_launcher.add_region_requirement(
      RegionRequirement(cells_shared_lp, 0/*projection ID*/,
                    READ_ONLY, SIMULTANEOUS, cells_lr));
    spmd_launcher.add_field(0, fid_t.fid_data);
    spmd_launcher.add_region_requirement(
       RegionRequirement(cells_ghost_lp, 0/*projection ID*/,
                    READ_ONLY, SIMULTANEOUS, cells_lr));
    spmd_launcher.add_field(1, fid_t.fid_data);
#endif

#endif

    //remove phase barriers:

    for (unsigned idx = 0; idx < phase_barriers.size(); idx++)
       runtime->destroy_phase_barrier(ctx, phase_barriers[idx]);
    phase_barriers.clear();


    // finish up legion runtime and handoff to mpi
    context_.interop_helper_.unset_call_mpi(ctx, runtime);
    context_.interop_helper_.handoff_to_mpi(ctx, runtime);

    // Set the current task context to the driver
    context_t::instance().pop_state(
      utils::const_string_t{"specialization_driver"}.hash());
} // mpilegion_runtime_driver



void
spmd_task(
  const Legion::Task *task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx, Legion::HighLevelRuntime *runtime
)
{
  const int my_shard= task->index_point.point_data[0];
  context_t & context_ = context_t::instance();
  context_.push_state(utils::const_string_t{"driver"}.hash(),
      ctx, runtime, task, regions);

  using generic_type = LegionRuntime::Accessor::AccessorType::Generic;
  using field_id = LegionRuntime::HighLevel::FieldID;

  clog(info) << "insude SPMD task, shard# = " << my_shard << std::endl;

  const LegionRuntime::HighLevel::InputArgs & args =
      LegionRuntime::HighLevel::HighLevelRuntime::get_input_args();

  driver(args.argc, args.argv);

  context_.pop_state(utils::const_string_t{"driver"}.hash());
}


} // namespace execution 
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
