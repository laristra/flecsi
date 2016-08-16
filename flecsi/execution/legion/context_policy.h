/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_legion_context_policy_h
#define flecsi_legion_context_policy_h

/*!
 * \file legion/context_policy.h
 * \authors bergen
 * \date Initial file creation: Jul 14, 2016
 */

#include <memory>
#include <functional>
#include <unordered_map>
#include <legion.h>

#include "flecsi/utils/common.h"
#include "flecsi/utils/const_string.h"
#include "flecsi/utils/tuple_wrapper.h"
#include "flecsi/execution/legion/runtime_driver.h"
#include "flecsi/execution/common/task_hash.h"

namespace flecsi {
namespace execution {

/*!
  \class legion_context_policy_t legion/context_policy.h
  \brief legion_context_policy_t provides...
 */
struct legion_context_policy_t
{

  using lr_context_t = LegionRuntime::HighLevel::Context;
  using lr_runtime_t = LegionRuntime::HighLevel::HighLevelRuntime;
  using lr_task_t = LegionRuntime::HighLevel::Task;
  using lr_regions_t = std::vector<LegionRuntime::HighLevel::PhysicalRegion>;

  const static LegionRuntime::HighLevel::Processor::Kind lr_loc =
    LegionRuntime::HighLevel::Processor::LOC_PROC;

  const size_t TOP_LEVEL_TASK_ID = 0;

  /*--------------------------------------------------------------------------*
   * Initialization.
   *--------------------------------------------------------------------------*/

  int
  initialize(
    int argc,
    char ** argv
  )
  {
    // Register top-level task
    lr_runtime_t::set_top_level_task_id(TOP_LEVEL_TASK_ID);
    lr_runtime_t::register_legion_task<legion_runtime_driver>(
      TOP_LEVEL_TASK_ID, lr_loc, true, false);

    // Register user tasks
    for(auto f: task_registry_) {
      // funky logic: task_registry_ is a map of std::pair
      // f.first is the uintptr_t that holds the user function address
      // f.second is the pair of unique task id and the registration function
      f.second.second(f.second.first);
    } // for
  
    // Start the runtime
    return lr_runtime_t::start(argc, argv);
  } // initialize

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

  /*--------------------------------------------------------------------------*
   * Task registraiton.
   *--------------------------------------------------------------------------*/

  using task_id_t = LegionRuntime::HighLevel::TaskID;
  using register_function_t = std::function<void(size_t)>;
  using unique_fid_t = unique_id_t<task_id_t>;

  /*!
   */
  bool
  register_task(
    task_hash_key_t key,
    const register_function_t & f
  )
  {
    if(task_registry_.find(key) == task_registry_.end()) {
      task_registry_[key] = { unique_fid_t::instance().next(), f };
      return true;
    } // if

    return false;
  } // register_task

  /*!
   */
  task_id_t
  task_id(
    task_hash_key_t key
  )
  {
    assert(task_registry_.find(key) != task_registry_.end() &&
      "task key does not exist!");

    return task_registry_[key].first;
  } // task_id

  /*--------------------------------------------------------------------------*
   * Function registraiton.
   *--------------------------------------------------------------------------*/

  /*!
   */
  template<typename T>
  bool
  register_function(
    const const_string_t & key,
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
  
  /*!
   */
  std::function<void(void)> *
  function(
    size_t key
  )
  {
    return function_registry_[key];
  } // function

  /*--------------------------------------------------------------------------*
   * Legion runtime accessors.
   *--------------------------------------------------------------------------*/

  lr_context_t & context() { return state_->context; }
  lr_runtime_t * runtime() { return state_->runtime; }
  const lr_task_t * task() { return state_->task; }
  const lr_regions_t & regions() { return state_->regions; }
  
private:

  /*!
    \struct legion_runtime_runtime_state_t legion_context_policy.h
    \brief legion_runtime_state_t provides storage for Legion runtime
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

  /*--------------------------------------------------------------------------*
   * Task registry
   *--------------------------------------------------------------------------*/

  // Define the map type using the task_hash_t hash function.
  std::unordered_map<task_hash_t::key_t,
    std::pair<task_id_t, register_function_t>,
    task_hash_t> task_registry_;

  /*--------------------------------------------------------------------------*
   * Function registry
   *--------------------------------------------------------------------------*/

  std::unordered_map<size_t, std::function<void(void)> *>
    function_registry_;

}; // class legion_context_policy_t

} // namespace execution 
} // namespace flecsi

#endif // flecsi_legion_context_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
