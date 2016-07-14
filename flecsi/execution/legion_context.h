/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef legion_context_h
#define legion_context_h

/*!
 * \file legion_context.h
 * \authors bergen
 * \date Initial file creation: Jul 14, 2016
 */

#include <legion.h>

/*!
  \class legion_context legion_context.h
  \brief legion_context provides...
 */
struct legion_context_t
{

  using context_t = LegionRuntime::HighLevel::Context;
  using runtime_t = LegionRuntime::HighLevel::HighLevelRuntime;
  using task_t = LegionRuntime::HighLevel::Task;
  using regions_t = std::vector<LegionRuntime::HighLevel::PhysicalRegion>;

  /*!
    Reset the legion runtime state.
   */
  void set_state(context_t & context, runtime_t * runtime,
    const task_t * task, const regions_t & regions)
    {
      state_.reset(new legion_runtime_state_t(context, runtime, task, regions);
    } // set_state

  context_t & context() { return state_->context; }
  runtime_t * runtime() { return state_->runtime; }
  const task_t * task() { return state_->task; }
  const regions_t & regions() { return state_->regions; }
  
private:

  /*!
    \struct legion_runtime_data_t legion_context.h
    \brief legion_runtime_data_t provides storage for Legion runtime
      information that can be reinitialized as needed to store const
      data types and references as required by the Legion runtime.
   */
  struct legion_runtime_state_t {

    legion_data_t(context_t & context_, runtime_t * runtime_,
      const task_t * task_, const regions_t & regions_)
      : context(context_), runtime(runtime_), task(task_), regions(regions_) {}
      
    context_t & context;
    runtime_t * runtime;
    const task_t * task;
    const regions & regions;

  } // struct legion_runtime_state_t

  std::shared_ptr<legion_runtime_state_t> state_;

}; // class legion_context_t

#endif // legion_context_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
