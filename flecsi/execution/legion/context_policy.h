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

#if !defined(ENABLE_MPI)
  #error ENABLE_MPI not defined! This file depends on MPI!
#endif

#include <mpi.h>

#include "flecsi/execution/common/task_hash.h"
#include "flecsi/execution/legion/runtime_driver.h"
#include "flecsi/utils/common.h"
#include "flecsi/utils/const_string.h"
#include "flecsi/utils/tuple_wrapper.h"

clog_register_tag(context);
clog_register_tag(interop);

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
    Legion::Context & context_,
    Legion::HighLevelRuntime * runtime_,
    const Legion::Task * task_,
    const std::vector<Legion::PhysicalRegion> & regions_
  )
  :
    context(context_),
    runtime(runtime_),
    task(task_),
    regions(regions_)
  {}
    
  Legion::Context & context;
  Legion::HighLevelRuntime * runtime;
  const Legion::Task * task;
  const std::vector<Legion::PhysicalRegion> & regions;

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
    Legion::Context & context,
    Legion::HighLevelRuntime * runtime,
    const Legion::Task * task,
    const std::vector<Legion::PhysicalRegion> & regions
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
  // MPI interoperabiliy.
  //--------------------------------------------------------------------------//

  ///
  /// Set the MPI runtime state. When the state is changed to active,
  /// the handshake interface will begin executing the current MPI task.
  ///
  /// \return A boolean indicating the current MPI runtime state.
  ///
  bool
  set_mpi_state(bool active)
  {
    {
    clog_tag_guard(interop);
    clog(info) << "set_mpi_state " << active << std::endl;
    }

    mpi_active_ = active;
    return mpi_active_;
  } // toggle_mpi_state

  ///
  /// Set the MPI user task. When control is given to the MPI runtime
  /// it will execute whichever function is currently set.
  ///
  void
  set_mpi_task(
    std::function<void()> & mpi_task
  )
  {
    {
    clog_tag_guard(interop);
    clog(info) << "set_mpi_task" << std::endl;
    }

    mpi_task_ = mpi_task;
  }

  void
  invoke_mpi_task()
  {
    return mpi_task_();
  } // invoke_mpi_task

  ///
  /// Set distributed-memory domain.
  ///
  void
  set_all_processes(const LegionRuntime::Arrays::Rect<1> & all_processes)
  {
    all_processes_ = all_processes;
  } // all_processes

  ///
  /// Return distributed-memory domain.
  ///
  const LegionRuntime::Arrays::Rect<1> &
  all_processes()
  const
  {
    return all_processes_;
  } // all_processes

  ///
  /// Handoff to legion runtime from MPI.
  ///
  void
  handoff_to_legion()
  {
    {
    clog_tag_guard(interop);
    clog(info) << "handoff_to_legion" << std::endl;
    }

    handshake_.mpi_handoff_to_legion();
  } // handoff_to_legion

  ///
  /// Wait for Legion runtime to complete.
  ///
  void
  wait_on_legion()
  {
    {
    clog_tag_guard(interop);
    clog(info) << "wait_on_legion" << std::endl;
    }

    handshake_.mpi_wait_on_legion();
  } // wait_on_legion

  ///
  /// Handoff to MPI from Legion.
  ///
  void
  handoff_to_mpi()
  {
    {
    clog_tag_guard(interop);
    clog(info) << "handoff_to_mpi" << std::endl;
    }

    handshake_.legion_handoff_to_mpi();
  } // handoff_to_mpi

  ///
  /// Wait for MPI runtime to complete.
  ///
  void
  wait_on_mpi()
  {
    {
    clog_tag_guard(interop);
    clog(info) << "wait_on_mpi" << std::endl;
    }

    handshake_.legion_wait_on_mpi();
  } // wait_on_legion

  //--------------------------------------------------------------------------//
  // FIXME: These all seem to be the same calling side below. Need to
  //        understand what they are doing.
  //--------------------------------------------------------------------------//

  ///
  /// FIXME: Comment
  ///
  void
  unset_call_mpi(
    Legion::Context ctx,
    Legion::HighLevelRuntime * runtime
  );

  ///
  /// FIXME: Comment
  ///
  void
  handoff_to_mpi(
    Legion::Context ctx,
    Legion::HighLevelRuntime * runtime
  );

  ///
  /// FIXME: Comment
  ///
  Legion::FutureMap
  wait_on_mpi(
    Legion::Context ctx,
    Legion::HighLevelRuntime * runtime
  );

  ///
  /// FIXME: Comment
  ///
  void
  connect_with_mpi(
    Legion::Context ctx,
    Legion::HighLevelRuntime * runtime
  );

  //--------------------------------------------------------------------------//
  // Task registraiton.
  //--------------------------------------------------------------------------//

  using task_id_t = Legion::TaskID;
  using register_function_t =
    std::function<void(task_id_t, processor_type_t, launch_t, std::string &)>;
  using unique_tid_t = utils::unique_id_t<task_id_t>;

  bool
  register_task(
    task_hash_key_t & key,
    processor_type_t variant,
    std::string & name,
    const register_function_t & f
  )
  {
    // Get the task entry. It is ok to create a new entry, and to have
    // multiple variants for each entry, i.e., we don't need to check
    // that the entry is empty.
    auto task_entry = task_registry_[key];

    // Add the variant only if it has not been defined.
    if(task_entry.find(variant) == task_entry.end()) {
      task_registry_[key][variant] = 
        std::make_tuple(unique_tid_t::instance().next(), f, name);
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
    
    return std::get<0>(variant->second);
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
  Legion::Context &
  context(
    size_t task_key
  )
  {
    return state_[task_key].top()->context;
  } // context

  ///
  /// return runtime corresponding to the taks_key
  ///
  Legion::HighLevelRuntime *
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
  Legion::Task *
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
  std::vector<Legion::PhysicalRegion> &
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

  struct task_value_hash_t{
    std::size_t
    operator () (
      const processor_type_t & key
    )
    const
    {
      return size_t(key);
    } // operator ()
  };

  struct task_value_equal_t{
    bool
    operator () (
      const processor_type_t & key1,
      const processor_type_t & key2
    )
    const
    {
      return size_t(key1) == size_t(key2);
    } // operator ()
  };

  // Define the value type for task map.
  using task_value_t =
    std::unordered_map<
      processor_type_t,
      std::tuple<task_id_t, register_function_t, std::string>,
      task_value_hash_t,
      task_value_equal_t
    >;

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

  //--------------------------------------------------------------------------//
  // MPI Interoperability
  //--------------------------------------------------------------------------//

  Legion::MPILegionHandshake handshake_;
  LegionRuntime::Arrays::Rect<1> all_processes_;
  std::function<void()> mpi_task_;
  bool mpi_active_ = false;

}; // class legion_context_policy_t

} // namespace execution 
} // namespace flecsi

#endif // flecsi_execution_legion_context_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
