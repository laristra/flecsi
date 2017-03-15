/*~--------------------------------------------------------------------------~*
 *  @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
 * /@@/////  /@@          @@////@@ @@////// /@@
 * /@@       /@@  @@@@@  @@    // /@@       /@@
 * /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
 * /@@////   /@@/@@@@@@@/@@       ////////@@/@@
 * /@@       /@@/@@//// //@@    @@       /@@/@@
 * /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
 * //       ///  //////   //////  ////////  //
 *
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_mpilegion_context_policy_h
#define flecsi_execution_mpilegion_context_policy_h

///
/// \file mpilegion/context_policy.h
/// \date Initial file creation: Jul 14, 2016
///

#include <functional>
#include <memory>
#include <stack>
#include <unordered_map>

#include <legion.h>

#include "flecsi/utils/common.h"
#include "flecsi/utils/const_string.h"
#include "flecsi/utils/tuple_wrapper.h"
#include "flecsi/execution/mpilegion/runtime_driver.h"

#include "flecsi/execution/common/task_hash.h"
#include "flecsi/execution/mpilegion/legion_handshake.h"
#include "flecsi/execution/mpilegion/mpi_legion_interop.h"
#include "flecsi/execution/mpilegion/init_partitions_task.h"
#include "flecsi/execution/mpilegion/lax_wendroff_task.h"
#include "flecsi/execution/legion/dpd.h"

namespace flecsi {
namespace execution {

///
/// \struct legion_runtime_runtime_state_t legion/context_policy.h
/// \brief legion_runtime_state_t provides storage for Legion runtime
///        information that can be reinitialized as needed to store const
///        data types and references as required by the Legion runtime.
///
struct legion_runtime_state_t {

  legion_runtime_state_t(
    LegionRuntime::HighLevel::Context & context_,
    LegionRuntime::HighLevel::HighLevelRuntime * runtime_,
    const LegionRuntime::HighLevel::Task * task_,
    const std::vector<LegionRuntime::HighLevel::PhysicalRegion> & regions_
  )
  :
    context(context_),
    runtime(runtime_),
    task(task_),
    regions(regions_)
  {}
    
  LegionRuntime::HighLevel::Context & context;
  LegionRuntime::HighLevel::HighLevelRuntime * runtime;
  const LegionRuntime::HighLevel::Task * task;
  const std::vector<LegionRuntime::HighLevel::PhysicalRegion> & regions;

}; // struct legion_runtime_state_t

// Use thread local storage for legion state information. The state_
// is set for each legion task invocation using the task name hash
// as a key. This seems like it should be safe, since multiple concurrent
// invocations of the same task can only occur on seperate threads.
static thread_local std::unordered_map<size_t,
  std::stack<std::shared_ptr<legion_runtime_state_t>>> state_;

///
/// \class mpilegion_context_policy_t mpilegion/context_policy.h
/// \brief mpilegion_context_policy_t provides an interface for passing 
///  legion's context to the FLeCSI tasks
///
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

  mpi_legion_interop_t interop_helper_;

  //-------------------------------------------------------------------------*
  // Initialization.
  //-------------------------------------------------------------------------*/

  ///
  /// Initialization of the legion runtime. Icludes task registration,
  /// legion runtime start and logic for mpi-legion interoperability 
  ///
  int
  initialize(
    int argc,
    char ** argv
  )
  {
    // Register top-level task
    lr_runtime_t::set_top_level_task_id(TOP_LEVEL_TASK_ID);
    lr_runtime_t::register_legion_task<mpilegion_runtime_driver>(
      TOP_LEVEL_TASK_ID, lr_loc, true, false);

    // FIXME
		// This is Galen's hack to get partitioning working for the sprint
    lr_runtime_t::register_legion_task<sprint::parts,
      sprint::get_numbers_of_cells_task>(
      task_ids_t::instance().get_numbers_of_cells_task_id,lr_loc, false, true);

   // FIXME
    // This is Galen's hack to get partitioning working for the sprint
    lr_runtime_t::register_legion_task<sprint::initialization_task>(
      task_ids_t::instance().init_task_id,lr_loc, false, true);

   // FIXME
    // This is Galen's hack to get partitioning working for the sprint
    lr_runtime_t::register_legion_task<sprint::partition_lr,
      sprint::shared_part_task>(
      task_ids_t::instance().shared_part_task_id,lr_loc, false, true); 
 
   // FIXME
    // This is Galen's hack to get partitioning working for the sprint
    lr_runtime_t::register_legion_task<sprint::partition_lr,
      sprint::exclusive_part_task>(
      task_ids_t::instance().exclusive_part_task_id,lr_loc, false, true);

		// FIXME
    // This is Galen's hack to get partitioning working for the sprint
    lr_runtime_t::register_legion_task<sprint::partition_lr,
      sprint::ghost_part_task>(
      task_ids_t::instance().ghost_part_task_id,lr_loc, false, true);
 
   // FIXME
    // This is Galen's hack to get partitioning working for the sprint
    lr_runtime_t::register_legion_task<sprint::check_partitioning_task>(
      task_ids_t::instance().check_partitioning_task_id,lr_loc, false, true);

    lr_runtime_t::register_legion_task<sprint::init_raw_conn_task>(
      task_ids_t::instance().init_raw_conn_task_id,lr_loc, false, true); 

    lr_runtime_t::register_legion_task<
      flecsi::execution::legion_dpd::init_connectivity_task>(
      task_ids_t::instance().dpd_init_connectivity_task_id,lr_loc, false, true); 

    lr_runtime_t::register_legion_task<
      flecsi::execution::sprint::ghost_access_task>(
      task_ids_t::instance().ghost_access_task_id,lr_loc, false, true);

    lr_runtime_t::register_legion_task<
      flecsi::execution::sprint::ghost_check_task>(
      task_ids_t::instance().ghost_check_task_id,lr_loc, false, true);

    lr_runtime_t::register_legion_task<
      flecsi::execution::sprint::ghost_init_task>(
      task_ids_t::instance().ghost_init_task_id,lr_loc, false, true);

    lr_runtime_t::register_legion_task<
      flecsi::execution::sprint::halo_copy_task>(
      task_ids_t::instance().halo_copy_task_id,lr_loc, true, false);

    lr_runtime_t::register_legion_task<
      flecsi::execution::lax_wendroff::lax_wendroff_task>(
      task_ids_t::instance().lax_wendroff_task_id,lr_loc, false, true);

    lr_runtime_t::register_legion_task<
      flecsi::execution::lax_wendroff::lax_halo_task>(
      task_ids_t::instance().lax_halo_task_id,lr_loc, true, false);

    lr_runtime_t::register_legion_task<
      flecsi::execution::lax_wendroff::lax_init_task>(
      task_ids_t::instance().lax_init_task_id,lr_loc, true, false);

    lr_runtime_t::register_legion_task<
      flecsi::execution::lax_wendroff::lax_write_task>(
      task_ids_t::instance().lax_write_task_id,lr_loc, true, false);

    lr_runtime_t::register_legion_task<
      flecsi::execution::lax_wendroff::lax_adv_x_task>(
      task_ids_t::instance().lax_adv_x_task_id,lr_loc, true, false);

    lr_runtime_t::register_legion_task<
      flecsi::execution::lax_wendroff::lax_adv_y_task>(
      task_ids_t::instance().lax_adv_y_task_id,lr_loc, true, false);

    lr_runtime_t::register_legion_task<
      flecsi::execution::lax_wendroff::lax_calc_excl_x_task>(
      task_ids_t::instance().lax_calc_excl_x_task_id,lr_loc, true, false);

    lr_runtime_t::register_legion_task<
      flecsi::execution::lax_wendroff::lax_calc_excl_y_task>(
      task_ids_t::instance().lax_calc_excl_y_task_id,lr_loc, true, false);

    //register spmd task, that call user's driver
    lr_runtime_t::register_legion_task<
      flecsi::execution::spmd_task>(
      task_ids_t::instance().spmd_task_id,lr_loc, false, true);

    // register handoff_to_mpi_task from mpi_legion_interop_t class
    lr_runtime_t::register_legion_task<handoff_to_mpi_task>(
      task_ids_t::instance().handoff_to_mpi_task_id, lr_loc,
      false, true, AUTO_GENERATE_ID,
      LegionRuntime::HighLevel::TaskConfigOptions(true/*leaf*/),
      "handoff_to_mpi_task");

    // register wait_on_mpi_task from mpi_legion_interop_t class
    lr_runtime_t::register_legion_task<wait_on_mpi_task>(
      task_ids_t::instance().wait_on_mpi_task_id, lr_loc,
      false, true, AUTO_GENERATE_ID,
      LegionRuntime::HighLevel::TaskConfigOptions(true/*leaf*/),
      "wait_on_mpi_task");

		// register unset_call_mpi_task from mpi_legion_interop_t class
    lr_runtime_t::register_legion_task<unset_call_mpi_task>(
			task_ids_t::instance().unset_call_mpi_id, lr_loc,
			false, true, AUTO_GENERATE_ID,
			LegionRuntime::HighLevel::TaskConfigOptions(true/*leaf*/),
			"unset_call_mpi_task");

    // Register user tasks
    for(auto f: task_registry_) {
      // funky logic: task_registry_ is a map of std::pair
      // f.first is the uintptr_t that holds the user function address
      // f.second is the pair of unique task id and the registration function
      f.second.second(f.second.first);
    } // for

    //initialize a helper class for mpi-legion interoperability
    interop_helper_.initialize();  

    interop_helper_.legion_configure();

    // Start the runtime
    lr_runtime_t::start(argc, argv, true);

    interop_helper_.handoff_to_legion();

    interop_helper_.wait_on_legion();

    //while loop to do some mpi tasks
    while(ext_legion_handshake_t::instance().call_mpi_) {
      #ifdef LEGIONDEBUG
        int rank;
        MPI_Comm_rank(MPI_COMM_WORLD,&rank);
        std::cout << "inside while loop N " << " rank = "<< rank <<
          "rank_from_handshake = " <<
          ext_legion_handshake_t::instance().rank_ << std::endl;
      #endif

      ext_legion_handshake_t::instance().shared_func_();
      interop_helper_.handoff_to_legion();
      interop_helper_.wait_on_legion();
    } // while

    int version, subversion;
    MPI_Get_version(&version, &subversion);
    if(version==3 && subversion>0)
        Legion::Runtime::wait_for_shutdown();

    return 0;
  } // initialize

  ///
  /// push_state is used to control the state of the legion task with id==key.
  /// Task is considered being completed when it's state is
  /// removed from the state_ object;
  /// Key - is a task-id.
  ///

  void push_state(
    size_t key,
    LegionRuntime::HighLevel::Context & context,
    LegionRuntime::HighLevel::HighLevelRuntime * runtime,
    const LegionRuntime::HighLevel::Task * task,
    const std::vector<LegionRuntime::HighLevel::PhysicalRegion> & regions
  )
  {
    #ifndef NDEBUG
      std::cout << "pushing state for " << key << std::endl;
    #endif

    state_[key].push(std::shared_ptr<legion_runtime_state_t>
      (new legion_runtime_state_t(context, runtime, task, regions)));
  } // set_state

  ///
  /// pops_state(key) is used to control the state of the legion task with
  ///  id=key. It removes the task state from the state_pbject when
  /// the task is completed.
  ///
  
  void pop_state(
    size_t key
  )
  {
    #ifndef NDEBUG
      std::cout << "popping state for " << key << std::endl;
    #endif

    state_[key].pop();
  } // set_state

#if 0
  /*!
    Reset the legion runtime state.
   */
  void
  set_state(
    lr_context_t & context,
    lr_runtime_t * runtime,
    const lr_task_t * task,
    const lr_regions_t & regions
  )
  {
    state_.reset(new legion_runtime_state_t(context, runtime, task, regions));
  } // set_state
#endif

  //--------------------------------------------------------------------------*
  // Task registraiton.
  //--------------------------------------------------------------------------*/

  using task_id_t = LegionRuntime::HighLevel::TaskID;
  using register_function_t = std::function<void(size_t)>;
  using unique_task_id_t = utils::unique_id_t<task_id_t>;

  ///
  /// register_task method generates unique ID for the task and add this ID
  /// to the task_registry_ container 
  ///
  bool
  register_task(
    task_hash_key_t key,
    const register_function_t & f
  )
  {
    if(task_registry_.find(key) == task_registry_.end()) {
      task_registry_[key] = { unique_task_id_t::instance().next(), f };
      return true;
    } // if

    return false;
  } // register_task

  ///
  /// this method return tak_id from the task's key
  ///
  task_id_t
  task_id(
    task_hash_key_t key
  )
  {
    assert(task_registry_.find(key) != task_registry_.end() &&
      "task key does not exist!");

    return task_registry_[key].first;
  } // task_id

  //--------------------------------------------------------------------------*
  // Function registraiton.
  //--------------------------------------------------------------------------*/

  ///
  /// register_function method add fuction pointer and the key to the 
  /// function_registry_ object.
  /// 
  template<typename T>
  bool
  register_function(
    const utils::const_string_t & key,
    T & function
  )
  {
    size_t h = key.hash();
    if(function_registry_.find(h) == function_registry_.end()) {
      function_registry_[h] =
        reinterpret_cast<std::function<void(void)> *>(&function);
      return true;
    } // if

    return false;
  } // register_function
  
  ///
  /// function(key) method return a pointer to the function with the 
  /// function key=key
  ///
  std::function<void(void)> *
  function(
    size_t key
  )
  {
    return function_registry_[key];
  } // function

  //--------------------------------------------------------------------------*
  // Legion runtime accessors.
  //--------------------------------------------------------------------------*/

  ///
  /// return a context that corresponds to the task with the key=task_key
  ///
  LegionRuntime::HighLevel::Context &
  context(
    size_t task_key
  )
  {
    return state_[task_key].top()->context;
  } // context

  ///
  /// return runtime that corresponds to the task with the key=task_key
  ///    
  LegionRuntime::HighLevel::HighLevelRuntime *
  runtime(
    size_t task_key
  )
  {
    return state_[task_key].top()->runtime;
  } // runtime

  ///
  /// return Task object  that corresponds to the task with the key=task_key
  ///  
  const
  LegionRuntime::HighLevel::Task *
  task(
    size_t task_key
  )
  {
    return state_[task_key].top()->task;
  } // task

  ///
  /// return a vector of the PhysicalRegions for the task with the key=task_key
  ///  
  const
  std::vector<LegionRuntime::HighLevel::PhysicalRegion> &
  regions(
    size_t task_key
  )
  {
    return state_[task_key].top()->regions;
  } // regions
  
private:

  //--------------------------------------------------------------------------*
  // Task registry
  //-------------------------------------------------------------------------*/

  // Define the map type using the task_hash_t hash function.
  std::unordered_map<task_hash_t::key_t,
    std::pair<task_id_t, register_function_t>,
    task_hash_t> task_registry_;

  //--------------------------------------------------------------------------*
  // Function registry
  //--------------------------------------------------------------------------*/

  std::unordered_map<size_t, std::function<void(void)> *>
    function_registry_;

}; // class mpilegion_context_policy_t

} // namespace execution 
} // namespace flecsi

#endif // flecsi_execution_mpilegion_context_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
