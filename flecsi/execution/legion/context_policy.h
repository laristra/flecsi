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

#ifndef flecsi_execution_legion_context_policy_h
#define flecsi_execution_legion_context_policy_h

///
/// \file
/// \date Initial file creation: Aug 4, 2016
///

#include <functional>
#include <memory>
#include <unordered_map>
#include <stack>

#include <cinchlog.h>
#include <legion.h>

#include "flecsi/execution/common/task_hash.h"
#include "flecsi/execution/legion/runtime_driver.h"
#include "flecsi/utils/common.h"
#include "flecsi/utils/const_string.h"
#include "flecsi/utils/tuple_wrapper.h"

clog_register_tag(context);

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
extern thread_local std::unordered_map<size_t,
  std::stack<std::shared_ptr<legion_runtime_state_t>>> state_;

///
/// \class legion_context_policy_t legion/context_policy.h
/// \brief legion_context_policy_t provides...
///
struct legion_context_policy_t
{
  const static LegionRuntime::HighLevel::Processor::Kind lr_loc =
    LegionRuntime::HighLevel::Processor::LOC_PROC;

  const size_t TOP_LEVEL_TASK_ID = 0;

  //--------------------------------------------------------------------------//
  // Initialization.
  //--------------------------------------------------------------------------//

  ///
  /// FleCSI context initialization: registering all tasks and start Legion
  /// runtime
  ///
  int
  initialize(
    int argc,
    char ** argv
  );

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
    {
    clog_tag_guard(context);
    clog(info) << "Pushing state for " << key << std::endl;
    }

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
    {
    clog_tag_guard(context);
    clog(info) << "Popping state for " << key << std::endl;
    }

    state_[key].pop();
  } // set_state

  //--------------------------------------------------------------------------//
  // Task registraiton.
  //--------------------------------------------------------------------------//

  using task_id_t = LegionRuntime::HighLevel::TaskID;
  using register_function_t = std::function<void(size_t)>;
  using unique_tid_t = utils::unique_id_t<task_id_t>;

  ///
  /// Registering FLeCSI task by using task_key and function pointer
  ///
  bool
  register_task(
    task_hash_key_t key,
    processor_type_t variant,
    const register_function_t & f
  )
  {
    {
    clog_tag_guard(context);
    clog(info) << "Registering task " << key << std::endl;
    }

    // Try to get the task entry.
    auto task_entry = task_registry_[key];

    // Check for existence and add if this variant has not been defined.
    if(task_entry.find(variant) == task_entry.end()) {
      task_registry_[key][variant] = { unique_tid_t::instance().next(), f };
      return true;
    }

    return false;
  } // register_task

  ///
  /// return task_id for the task with task_key = key
  ///
  task_id_t
  task_id(
    task_hash_key_t key
  )
  {
    {
    clog_tag_guard(context);
    clog(info) << "Returning task id: " << key << std::endl;
    }

    // There is only one task variant set.
    clog_assert(key.processor().count() == 1,
      "multiple task variants given: " << key.processor());

    // The key exists.
    auto task_entry = task_registry_.find(key);
    clog_assert(task_entry != task_registry_.end(),
      "task key does not exist: " << key);

    auto mask = static_cast<processor_mask_t>(key.processor().to_ulong());
    auto variant = task_entry->second.find(mask_to_type(mask));

    clog_assert(variant != task_entry->second.end(),
      "task variant does not exist: " << key);
    
    return variant->second.first;
  } // task_id

  //--------------------------------------------------------------------------//
  // Function registraiton.
  //--------------------------------------------------------------------------//

  ///
  /// register FLeCSI function
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
  /// return function by it's key
  ///
  std::function<void(void)> *
  function(
    size_t key
  )
  {
    return function_registry_[key];
  } // function

  //--------------------------------------------------------------------------//
  // Legion runtime accessors.
  //--------------------------------------------------------------------------//

  ///
  /// return context corresponding to the taks_key
  ///
  LegionRuntime::HighLevel::Context &
  context(
    size_t task_key
  )
  {
    return state_[task_key].top()->context;
  } // context

  ///
  /// return runtime corresponding to the taks_key
  ///
  LegionRuntime::HighLevel::HighLevelRuntime *
  runtime(
    size_t task_key
  )
  {
    return state_[task_key].top()->runtime;
  } // runtime

  ///
  /// return taks pointer by the  taks_key
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
  /// return PhysicalRegions for the task with the key=task_key
  ///
  const
  std::vector<LegionRuntime::HighLevel::PhysicalRegion> &
  regions(
    size_t task_key
  )
  {
    return state_[task_key].top()->regions;
  } // regions

  // FIXME: Not sure if this is needed...
  //------------------------------------------------------------------------//
  // Data registration
  //------------------------------------------------------------------------//

  using partition_count_map_t = std::map<size_t, size_t>;

  using  copy_task_map_t = std::unordered_map<size_t, task_hash_key_t>;

  // FIXME: I would like to remove/move this.
  struct partitioned_index_space
  {
    // logical region for the entity
    Legion::LogicalRegion entities_lr;
    // index partitions:
    Legion::IndexPartition primary_ip;
    Legion::IndexPartition exclusive_ip;
    Legion::IndexPartition shared_ip;
    Legion::IndexPartition ghost_ip;
    // sizes
    size_t size;
    size_t exclusive_size;
    size_t shared_size;
    size_t ghost_size;
    //number of elements per each part of the partition
    partition_count_map_t exclusive_count_map;
    partition_count_map_t shared_count_map;
    partition_count_map_t ghost_count_map;
    // vector of the PhaseBarrires
    std::vector<Legion::PhaseBarrier> pbs;
    //map for the copy_task ids
    copy_task_map_t copy_task_map;
  };
 
 
private:

  //--------------------------------------------------------------------------//
  // Task registry
  //--------------------------------------------------------------------------//

  // Define the value type for task map.
  using task_value_t =
    std::unordered_map<processor_type_t,
      std::pair<task_id_t, register_function_t>>;

  // Define the map type using the task_hash_t hash function.
  std::unordered_map<
    task_hash_t::key_t, // key
    task_value_t,       // value
    task_hash_t,        // hash function
    task_hash_t         // equivalence operator
  > task_registry_;

  //--------------------------------------------------------------------------//
  // Function registry
  //--------------------------------------------------------------------------//

  std::unordered_map<size_t, std::function<void(void)> *>
    function_registry_;

}; // class legion_context_policy_t

} // namespace execution 
} // namespace flecsi

#endif // flecsi_execution_legion_context_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
