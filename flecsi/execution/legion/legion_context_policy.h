/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef legion_context_policy_h
#define legion_context_policy_h

/*!
 * \file legion_context_policy.h
 * \authors bergen
 * \date Initial file creation: Jul 14, 2016
 */

#include <memory>
#include <legion.h>

#include "flecsi/execution/legion/legion_runtime_driver.h"

namespace flecsi {

#if 0
extern void legion_runtime_driver(
  const LegionRuntime::HighLevel::Task * task,
  const std::vector<LegionRuntime::HighLevel::PhysicalRegion> & regions,
  LegionRuntime::HighLevel::Context ctx,
  LegionRuntime::HighLevel::HighLevelRuntime * runtime);
#endif

/*!
  \class legion_context_policy_t legion_context_policy.h
  \brief legion_context_policy_t provides...
 */
struct legion_context_policy_t
{

  using lr_context_t = LegionRuntime::HighLevel::Context;
  using lr_runtime_t = LegionRuntime::HighLevel::HighLevelRuntime;
  using lr_task_t = LegionRuntime::HighLevel::Task;
  using lr_regions_t =
    std::vector<LegionRuntime::HighLevel::PhysicalRegion>;

  const static LegionRuntime::HighLevel::Processor::Kind lr_loc =
    LegionRuntime::HighLevel::Processor::LOC_PROC;

  const size_t TOP_LEVEL_TASK_ID = 0;

  int initialize(int argc, char ** argv) {
    lr_runtime_t::set_top_level_task_id(TOP_LEVEL_TASK_ID);
    lr_runtime_t::register_legion_task<legion_runtime_driver>(
      TOP_LEVEL_TASK_ID, lr_loc, true, false);
  
    return lr_runtime_t::start(argc, argv);
  } // initialize

  /*!
    Reset the legion runtime state.
   */
  void set_state(lr_context_t & context, lr_runtime_t * runtime,
    const lr_task_t * task, const lr_regions_t & regions)
    {
      state_.reset(new legion_runtime_state_t(context, runtime, task, regions));
    } // set_state

  lr_context_t & context() { return state_->context; }
  lr_runtime_t * runtime() { return state_->runtime; }
  const lr_task_t * task() { return state_->task; }
  const lr_regions_t & regions() { return state_->regions; }
  
private:

  /*!
    \struct legion_runtime_data_t legion_context.h
    \brief legion_runtime_data_t provides storage for Legion runtime
      information that can be reinitialized as needed to store const
      data types and references as required by the Legion runtime.
   */
  struct legion_runtime_state_t {

    legion_runtime_state_t(lr_context_t & context_, lr_runtime_t * runtime_,
      const lr_task_t * task_, const lr_regions_t & regions_)
      : context(context_), runtime(runtime_), task(task_), regions(regions_) {}
      
    lr_context_t & context;
    lr_runtime_t * runtime;
    const lr_task_t * task;
    const lr_regions_t & regions;

  }; // struct legion_runtime_state_t

  std::shared_ptr<legion_runtime_state_t> state_;

}; // class legion_context_policy_t

} // namespace flecsi

#endif // legion_context_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
