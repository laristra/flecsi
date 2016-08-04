/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_mpilegion_context_policy_h
#define flecsi_mpilegion_context_policy_h

/*!
 * \file legion/context_policy.h
 * \authors bergen
 * \date Initial file creation: Jul 14, 2016
 */

#include <memory>
#include <functional>
#include <unordered_map>
#include <legion.h>
#include <mpi.h>

#include "flecsi/utils/common.h"
#include "flecsi/utils/tuple_wrapper.h"
#include "flecsi/execution/mpilegion/runtime_driver.h"


namespace flecsi {
namespace execution {

/*!
  \class mpilegion_context_policy_t mpilegion_context_policy.h
  \brief mpilegion_context_policy_t provides...
 */
struct mpilegion_context_policy_t
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

    MPI_Init(&argc, &argv);

    // Register top-level task
    lr_runtime_t::set_top_level_task_id(TOP_LEVEL_TASK_ID);
    lr_runtime_t::register_legion_task<mpilegion_runtime_driver>(
      TOP_LEVEL_TASK_ID, lr_loc, true, false);

    // Register user tasks
    for(auto f: registration_) {
      // funky logic: registration_ is a map of std::pair
      // f.first is the uintptr_t that holds the user function address
      // f.second is the pair of unique task id and the registration function
      f.second.second(f.second.first);
    } // for
 
    MPILegionInterop *Interop =  MPILegionInterop::initialize();
   
    // Start the runtime
    return lr_runtime_t::start(argc, argv);

    Interop->legion_configure();

    std::cout<<"back in MPI, do something here"<<std::endl;

    Interop->handoff_to_legion();

    //legion is running

    Interop->wait_on_legion();

    //while loop to do some mpi tasks

     while(Interop->call_mpi)
     {
       Interop->shared_func();
       Interop->handoff_to_legion();
       Interop->wait_on_legion();
      }

      MPI_Finalize();
   
   return 0;

  } // initialize

  /*!
    Reset the legion runtime state.
   */
  void set_state(lr_context_t & context, lr_runtime_t * runtime,
    const lr_task_t * task, const lr_regions_t & regions)
    {
      state_.reset(new mpilegion_runtime_state_t(context, runtime, task, regions));
    } // set_state

  /*--------------------------------------------------------------------------*
   * Task registraiton.
   *--------------------------------------------------------------------------*/

  using task_id_t = LegionRuntime::HighLevel::TaskID;
  using register_function_t = std::function<void(size_t)>;
  using unique_fid_t = unique_id_t<task_id_t>;

  bool register_task(uintptr_t key, const register_function_t & f)
    {
      if(registration_.find(key) == registration_.end()) {
        registration_[key] = { unique_fid_t::instance().next(), f };
        return true;
      }

      return false;
    } // register_task

  task_id_t task_id(uintptr_t key)
    {
      assert(registration_.find(key) != registration_.end() &&
        "task key does not exist!");

      return registration_[key].first;
    } // task_id

  /*--------------------------------------------------------------------------*
   * Legion runtime accessors.
   *--------------------------------------------------------------------------*/

  lr_context_t & context() { return state_->context; }
  lr_runtime_t * runtime() { return state_->runtime; }
  const lr_task_t * task() { return state_->task; }
  const lr_regions_t & regions() { return state_->regions; }
  
private:

  /*!
    \struct mpilegion_runtime_runtime_state_t mpilegion_context_policy.h
    \brief mpilegion_runtime_state_t provides storage for Legion runtime
      information that can be reinitialized as needed to store const
      data types and references as required by the Legion runtime.
   */
  struct mpilegion_runtime_state_t {

    mpilegion_runtime_state_t(lr_context_t & context_, lr_runtime_t * runtime_,
      const lr_task_t * task_, const lr_regions_t & regions_)
      : context(context_), runtime(runtime_), task(task_), regions(regions_) {}
      
    lr_context_t & context;
    lr_runtime_t * runtime;
    const lr_task_t * task;
    const lr_regions_t & regions;

  }; // struct mpilegion_runtime_state_t

  std::shared_ptr<mpilegion_runtime_state_t> state_;
  std::unordered_map<uintptr_t,
    std::pair<task_id_t, register_function_t>> registration_;

}; // class mpilegion_context_policy_t

} //namespace execution
} // namespace flecsi

#endif // flecsi_mpilegion_context_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
