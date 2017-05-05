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

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Aug 4, 2016
//----------------------------------------------------------------------------//

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

#include "flecsi/execution/legion/runtime_state.h"

clog_register_tag(context);
clog_register_tag(interop);

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
//! The legion_runtime_state_t type provides storage for Legion runtime
//! information that can be reinitialized as needed to store const
//! data types and references as required by the Legion runtime.
//!
//! @ingroup legion-execution
//----------------------------------------------------------------------------//

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

//----------------------------------------------------------------------------//
//! Context state uses thread-local storage (TLS). The state is set for
//! each Legion task invocation using the task name hash of the plain-text
//! task name as a key. This should give sufficient isolation from naming
//! collisions. State should only be pushed or popped from the FleCSI
//! task wrapper, or from top-level driver calls.
//!
//! FleCSI developers should be extremely careful with how this state
//! is used. In particular, you should not rely on any particular
//! initialization of this state, i.e., it may be uninitialized on any
//! given thread. The current implementation does not make any assumptions
//! about what tasks may have been invoked from a particular thread before
//! pushing context information onto this state. It is safe for this state
//! to be uninitialized when it is encountered by a task executing in
//! the FleCSI runtime. This property \em must be maintained.
//!
//! @ingroup legion-execution
//----------------------------------------------------------------------------//

extern thread_local std::unordered_map<size_t,
  std::stack<std::shared_ptr<legion_runtime_state_t>>> state_;

//----------------------------------------------------------------------------//
//! The legion_context_policy_t is the backend runtime context policy for
//! Legion.
//!
//! @ingroup legion-execution
//----------------------------------------------------------------------------//

struct legion_context_policy_t
{
  const size_t TOP_LEVEL_TASK_ID = 0;

  //--------------------------------------------------------------------------//
  // Runtime state.
  //--------------------------------------------------------------------------//

  //--------------------------------------------------------------------------//
  //! FleCSI context initialization. This method initializes the FleCSI
  //! runtime using Legion.
  //!
  //! @param argc The command-line argument count passed from main.
  //! @param argv The command-line argument values passed from main.
  //!
  //! @return An integer value with a non-zero error code upon failure,
  //!         zero otherwise.
  //--------------------------------------------------------------------------//

  int
  initialize(
    int argc,
    char ** argv
  );

  //--------------------------------------------------------------------------//
  //! Push Legion runtime state onto a task specific stack. In this case,
  //! \em task is the plain-text name of the user's task.
  //!
  //! @param key     The task hash key.
  //! @param context The Legion task context reference.
  //! @param runtime The Legion task runtime pointer.
  //! @param task    The Legion task pointer.
  //! @param regions The Legion physical regions.
  //--------------------------------------------------------------------------//

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
  } // push_state

  //--------------------------------------------------------------------------//
  //! Pop Legion runtime state off of the stack.
  //!
  //! @param key The task hash key.
  //--------------------------------------------------------------------------//

  void pop_state(
    size_t key
  )
  {
    {
    clog_tag_guard(context);
    clog(info) << "Popping state for " << key << std::endl;
    }

    state_[key].pop();
  } // pop_state

  //--------------------------------------------------------------------------//
  // MPI interoperability.
  //--------------------------------------------------------------------------//

  //--------------------------------------------------------------------------//
  //! Set the MPI runtime state. When the state is changed to active,
  //! the handshake interface will begin executing the current MPI task.
  //!
  //! @return A boolean indicating the current MPI runtime state.
  //--------------------------------------------------------------------------//

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

  //--------------------------------------------------------------------------//
  //! Set the MPI user task. When control is given to the MPI runtime
  //! it will execute whichever function is currently set.
  //--------------------------------------------------------------------------//

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

  //--------------------------------------------------------------------------//
  //! Invoke the current MPI task.
  //--------------------------------------------------------------------------//

  void
  invoke_mpi_task()
  {
    return mpi_task_();
  } // invoke_mpi_task

  //--------------------------------------------------------------------------//
  //! Set the distributed-memory domain.
  //--------------------------------------------------------------------------//

  void
  set_all_processes(const LegionRuntime::Arrays::Rect<1> & all_processes)
  {
    all_processes_ = all_processes;
  } // all_processes

  //--------------------------------------------------------------------------//
  //! Return the distributed-memory domain.
  //--------------------------------------------------------------------------//

  const
  LegionRuntime::Arrays::Rect<1> &
  all_processes()
  const
  {
    return all_processes_;
  } // all_processes

  //--------------------------------------------------------------------------//
  //! Handoff to legion runtime from MPI.
  //--------------------------------------------------------------------------//

  void
  handoff_to_legion()
  {
    {
    clog_tag_guard(interop);
    clog(info) << "handoff_to_legion" << std::endl;
    }

    handshake_.mpi_handoff_to_legion();
  } // handoff_to_legion

  //--------------------------------------------------------------------------//
  //! Wait for Legion runtime to complete.
  //--------------------------------------------------------------------------//

  void
  wait_on_legion()
  {
    {
    clog_tag_guard(interop);
    clog(info) << "wait_on_legion" << std::endl;
    }

    handshake_.mpi_wait_on_legion();
  } // wait_on_legion

  //--------------------------------------------------------------------------//
  //! Handoff to MPI from Legion.
  //--------------------------------------------------------------------------//

  void
  handoff_to_mpi()
  {
    {
    clog_tag_guard(interop);
    clog(info) << "handoff_to_mpi" << std::endl;
    }

    handshake_.legion_handoff_to_mpi();
  } // handoff_to_mpi

  //--------------------------------------------------------------------------//
  //! Wait for MPI runtime to complete task execution.
  //--------------------------------------------------------------------------//

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
  //! Unset the MPI active state to pass execution back to
  //! the Legion runtime.
  //!
  //! @param ctx The Legion runtime context.
  //! @param runtime The Legion task runtime pointer.
  //--------------------------------------------------------------------------//

  void
  unset_call_mpi(
    Legion::Context & ctx,
    Legion::HighLevelRuntime * runtime
  );

  //--------------------------------------------------------------------------//
  //! Switch execution to the MPI runtime.
  //!
  //! @param ctx The Legion runtime context.
  //! @param runtime The Legion task runtime pointer.
  //--------------------------------------------------------------------------//

  void
  handoff_to_mpi(
    Legion::Context & ctx,
    Legion::HighLevelRuntime * runtime
  );

  //--------------------------------------------------------------------------//
  //! Wait on the MPI runtime to finish the current task execution.
  //!
  //! @param ctx The Legion runtime context.
  //! @param runtime The Legion task runtime pointer.
  //!
  //! @return A future map with the result of the task execution.
  //--------------------------------------------------------------------------//

  Legion::FutureMap
  wait_on_mpi(
    Legion::Context & ctx,
    Legion::HighLevelRuntime * runtime
  );

  //--------------------------------------------------------------------------//
  //! Connect with the MPI runtime.
  //!
  //! @param ctx The Legion runtime context.
  //! @param runtime The Legion task runtime pointer.
  //--------------------------------------------------------------------------//

  void
  connect_with_mpi(
    Legion::Context & ctx,
    Legion::HighLevelRuntime * runtime
  );

  //--------------------------------------------------------------------------//
  // Task interface.
  //--------------------------------------------------------------------------//

  //--------------------------------------------------------------------------//
  //! The task_id_t type is used to uniquely identify tasks that have
  //! been registered with the runtime.
  //!
  //! @ingroup execution
  //--------------------------------------------------------------------------//

  using task_id_t = Legion::TaskID;

  //--------------------------------------------------------------------------//
  //! The registration_function_t type defines a function type for
  //! registration callbacks.
  //--------------------------------------------------------------------------//

  using registration_function_t =
    std::function<void(task_id_t, processor_type_t, launch_t, std::string &)>;

  //--------------------------------------------------------------------------//
  //! The unique_tid_t type create a unique id generator for registering
  //! tasks.
  //--------------------------------------------------------------------------//

  using unique_tid_t = utils::unique_id_t<task_id_t>;

  //--------------------------------------------------------------------------//
  //! Register a task variant with the runtime.
  //!
  //! @param key       The task hash key.
  //! @param variant   The processor variant of the task.
  //! @param name      The task name string.
  //! @param call_back The registration call back function.
  //--------------------------------------------------------------------------//

  bool
  register_task(
    task_hash_key_t & key,
    processor_type_t variant,
    std::string & name,
    const registration_function_t & call_back
  )
  {
    // Get the task entry. It is ok to create a new entry, and to have
    // multiple variants for each entry, i.e., we don't need to check
    // that the entry is empty.
    auto task_entry = task_registry_[key];

    // Add the variant only if it has not been defined.
    if(task_entry.find(variant) == task_entry.end()) {
      task_registry_[key][variant] =
        std::make_tuple(unique_tid_t::instance().next(), call_back, name);
      return true;
    }

    return false;
  } // register_task

  //--------------------------------------------------------------------------//
  //! Return the task id for the task identified by \em key.
  //!
  //! @param key The task hash key.
  //--------------------------------------------------------------------------//

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
  // Function interface.
  //--------------------------------------------------------------------------//

  //--------------------------------------------------------------------------//
  //! FIXME: This interface needs to be updated.
  //--------------------------------------------------------------------------//

  template<
    typename RETURN,
    typename ARG_TUPLE
  >
  bool
  register_function(
    const utils::const_string_t & key,
    std::function<RETURN(ARG_TUPLE)> & user_function
  )
  {
    const size_t h = key.hash();
    if(function_registry_.find(h) == function_registry_.end()) {
      function_registry_[h] =
        reinterpret_cast<std::function<void(void)> *>(&user_function);
      return true;
    } // if

    return false;
  } // register_function

  //--------------------------------------------------------------------------//
  //! FIXME: This interface needs to be updated.
  //--------------------------------------------------------------------------//

  template<
    typename RETURN,
    typename ARG_TUPLE
  >
  std::function<RETURN(ARG_TUPLE)> *
  function(
    size_t key
  )
  {
    return reinterpret_cast<std::function<RETURN(ARG_TUPLE)> *>
      (function_registry_[key]);
  } // function

  //--------------------------------------------------------------------------//
  // Legion runtime interface.
  //--------------------------------------------------------------------------//

  //--------------------------------------------------------------------------//
  //! Return the Legion task runtime context for the given task key.
  //!
  //! @param key The task hash key.
  //--------------------------------------------------------------------------//

  Legion::Context &
  context(
    size_t key
  )
  {
    return state_[key].top()->context;
  } // context

  //--------------------------------------------------------------------------//
  //! Return the Legion task runtime pointer for the given task key.
  //!
  //! @param key The task hash key.
  //--------------------------------------------------------------------------//

  Legion::HighLevelRuntime *
  runtime(
    size_t key
  )
  {
    return state_[key].top()->runtime;
  } // runtime

  //--------------------------------------------------------------------------//
  //! Return the Legion task pointer for the given task key.
  //!
  //! @param key The task hash key.
  //--------------------------------------------------------------------------//

  const
  Legion::Task *
  task(
    size_t key
  )
  {
    return state_[key].top()->task;
  } // task

  //--------------------------------------------------------------------------//
  //! Return a reference to the Legion task physical regions vector.
  //!
  //! @param key The task hash key.
  //--------------------------------------------------------------------------//

  const
  std::vector<Legion::PhysicalRegion> &
  regions(
    size_t key
  )
  {
    return state_[key].top()->regions;
  } // regions

  //--------------------------------------------------------------------------//
  // FIXME: Not sure if this is needed...
  //--------------------------------------------------------------------------//

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
  // Task data members.
  //--------------------------------------------------------------------------//

  struct task_value_hash_t
  {

    std::size_t
    operator () (
      const processor_type_t & key
    )
    const
    {
      return size_t(key);
    } // operator ()

  }; // struct task_value_hash_t

  struct task_value_equal_t
  {

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
      std::tuple<
        task_id_t,
        registration_function_t,
        std::string
      >,
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
  // Function data members.
  //--------------------------------------------------------------------------//

  std::unordered_map<size_t, std::function<void(void)> *>
    function_registry_;

  //--------------------------------------------------------------------------//
  // Legion data members.
  //--------------------------------------------------------------------------//

  Legion::MPILegionHandshake handshake_;
  LegionRuntime::Arrays::Rect<1> all_processes_;

  //--------------------------------------------------------------------------//
  // MPI data members.
  //--------------------------------------------------------------------------//

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
