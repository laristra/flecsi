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

#include "flecsi/execution/legion/runtime_driver.h"
#include "flecsi/execution/common/launch.h"
#include "flecsi/execution/common/processor.h"
#include "flecsi/utils/common.h"
#include "flecsi/utils/const_string.h"
#include "flecsi/utils/tuple_wrapper.h"

#include "flecsi/execution/legion/runtime_state.h"

namespace flecsi {
namespace execution {

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
  //! The task_info_t type is a convenience type for defining the task
  //! registration map below.
  //--------------------------------------------------------------------------//

  using task_info_t =
    std::tuple<
      task_id_t,
      processor_type_t,
      launch_t,
      std::string,
      registration_function_t
    >;

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
    clog_tag_guard(context);
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
    clog_tag_guard(context);
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
    clog_tag_guard(context);
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
    clog_tag_guard(context);
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
    clog_tag_guard(context);
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
    clog_tag_guard(context);
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
  //! Register a task with the runtime.
  //!
  //! @param key       The task hash key.
  //! @param name      The task name string.
  //! @param call_back The registration call back function.
  //--------------------------------------------------------------------------//

  bool
  register_task(
    size_t key,
    processor_type_t processor,
    launch_t launch,
    std::string & name,
    const registration_function_t & callback
  )
  {
    clog(info) << "Registering task callback " << name << " with key " <<
      key << std::endl;

    clog_assert(task_registry_.find(key) == task_registry_.end(),
      "task key already exists");

    task_registry_[key] = std::make_tuple(unique_tid_t::instance().next(),
      processor, launch, name, callback);

    return true;
  } // register_task

  //--------------------------------------------------------------------------//
  //! Return the task registration tuple.
  //!
  //! @param key The task hash key.
  //--------------------------------------------------------------------------//

  template<
    size_t KEY
  >
  task_info_t & 
  task_info()
  {
    auto task_entry = task_registry_.find(KEY);

    clog_assert(task_entry != task_registry_.end(),
      "task key " << KEY << " does not exist");

    return task_entry->second;
  } // task_info

  //--------------------------------------------------------------------------//
  //! Return the task registration tuple.
  //!
  //! @param key The task hash key.
  //--------------------------------------------------------------------------//

  task_info_t & 
  task_info(size_t key)
  {
    auto task_entry = task_registry_.find(key);

    clog_assert(task_entry != task_registry_.end(),
      "task key " << key << " does not exist");

    return task_entry->second;
  } // task_info

  //--------------------------------------------------------------------------//
  //! FIXME
  //!
  //! @param key The task hash key.
  //--------------------------------------------------------------------------//

  #define task_info_template_method(name, return_type, index)                  \
    template<size_t KEY>                                                       \
    return_type                                                                \
    name()                                                                     \
    {                                                                          \
      {                                                                        \
      clog_tag_guard(context);                                                 \
      clog(info) << "Returning " << #name << " for " << KEY << std::endl;      \
      }                                                                        \
      return std::get<index>(task_info<KEY>());                                \
    }

  //--------------------------------------------------------------------------//
  //! FIXME
  //!
  //! @param key The task hash key.
  //--------------------------------------------------------------------------//

  #define task_info_method(name, return_type, index)                           \
    return_type                                                                \
    name(size_t key)                                                           \
    {                                                                          \
      {                                                                        \
      clog_tag_guard(context);                                                 \
      clog(info) << "Returning " << #name << " for " << key << std::endl;      \
      }                                                                        \
      return std::get<index>(task_info(key));                                  \
    }

  //--------------------------------------------------------------------------//
  //! FIXME
  //!
  //! @param key The task hash key.
  //--------------------------------------------------------------------------//

  task_info_template_method(task_id, task_id_t, 0);
  task_info_method(task_id, task_id_t, 0);
  task_info_template_method(processor_type, processor_type_t, 1);
  task_info_method(processor_type, processor_type_t, 1);

  //--------------------------------------------------------------------------//
  // Function interface.
  //--------------------------------------------------------------------------//

  //--------------------------------------------------------------------------//
  //! FIXME: This interface needs to be updated.
  //--------------------------------------------------------------------------//

  template<
    typename RETURN,
    typename ARG_TUPLE,
    RETURN (*FUNCTION)(ARG_TUPLE),
    size_t KEY
  >
  bool
  register_function(
  )
  {
    clog_assert(function_registry_.find(KEY) == function_registry_.end(),
      "function has already been registered");

    clog(info) << "Registering function: " << FUNCTION << std::endl;

    function_registry_[KEY] =
      reinterpret_cast<void *>(FUNCTION);
    return true;
  } // register_function

  //--------------------------------------------------------------------------//
  //! FIXME: Add description.
  //--------------------------------------------------------------------------//

  void *
  function(
    size_t key
  )
  {
    return function_registry_[key];
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
  //! Set phase barriers as masters.
  //!
  //! @param pbarriers_as_masters phase barriers buffer
  //--------------------------------------------------------------------------//

  void
  set_pbarriers_as_masters(
    Legion::PhaseBarrier* pbarriers_as_masters
  )
  {
    pbarriers_as_masters_ = pbarriers_as_masters;
  }

  //--------------------------------------------------------------------------//
  //! Return phase barriers as masters for index space.
  //!
  //! @param index_space virtual index space
  //--------------------------------------------------------------------------//

  Legion::PhaseBarrier&
  get_pbarriers_as_master(
    size_t index_space
  )
  const
  {
    return pbarriers_as_masters_[index_space];
  }

  //--------------------------------------------------------------------------//
  //! Push ghost owners phase barriers buffer corresponding to a virtual
  //! index space.
  //!
  //! @param ghost_owners_pbarriers ghost owners phase barriers buffer 
  //--------------------------------------------------------------------------//

  void
  push_ghost_owners_pbarriers(
    Legion::PhaseBarrier* ghost_owners_pbarriers
  )
  {
    ghost_owners_pbarriers_.push_back(ghost_owners_pbarriers);
  }

  //--------------------------------------------------------------------------//
  //! Push ghost owners logical regions (virtual
  //! index space).
  //!
  //! @param ghost_owners_lregions ghost owners logical regions
  //--------------------------------------------------------------------------//

  void
  push_ghost_owners_lregions(
    std::vector<Legion::LogicalRegion> ghost_owners_lregions
  )
  {
    ghost_owners_lregions_.push_back(ghost_owners_lregions);
  }

  //--------------------------------------------------------------------------//
  //! Return phase barriers for ghost owners.
  //!
  //! @param index_space virtual index space
  //--------------------------------------------------------------------------//

  Legion::PhaseBarrier&
  get_ghost_owners_pbarriers(
    size_t index_space,
    size_t owner
  )
  const
  {
    return ghost_owners_pbarriers_[index_space][owner];
  }

  //--------------------------------------------------------------------------//
  //! Push logical region corresponding to a virtual index space.
  //!
  //! @param region logical region 
  //--------------------------------------------------------------------------//

  void
  push_color_region(
    Legion::LogicalRegion region
  )
  {
    color_regions_.push_back(region);
  }

  //--------------------------------------------------------------------------//
  //! Return main logical region.
  //!
  //! @param index_space virtual index space
  //--------------------------------------------------------------------------//

  Legion::LogicalRegion
  get_color_region(
    size_t index_space
  )
  const
  {
    return color_regions_[index_space];
  }

  //--------------------------------------------------------------------------//
  //! Push primary ghost index partition corresponding to a virtual 
  //! index space.
  //!
  //! @param primary_ghost_ip primary ghost index partition 
  //--------------------------------------------------------------------------//

  void
  push_primary_ghost_ip(
    Legion::IndexPartition primary_ghost_ip
  ){
    primary_ghost_ips_.push_back(primary_ghost_ip);
  }

  //--------------------------------------------------------------------------//
  //! Return primary ghost index partition for virtual index space.
  //!
  //! @param index_space virtual index space
  //--------------------------------------------------------------------------//

  Legion::IndexPartition
  get_primary_ghost_ip(
    size_t index_space
  )
  const
  {
    return primary_ghost_ips_[index_space];
  }

  //--------------------------------------------------------------------------//
  //! Push exclusive shared index partition corresponding to a virtual
  //! index space.
  //!
  //! @param ip exclusive shared index partition
  //--------------------------------------------------------------------------//

  void
  push_excl_shared_ip(
    Legion::IndexPartition ip
  ){
    excl_shared_ips_.push_back(ip);
  }

  //--------------------------------------------------------------------------//
  //! Return exclusive ghost index partition for virtual index space.
  //!
  //! @param index_space virtual index space
  //--------------------------------------------------------------------------//

  Legion::IndexPartition
  get_excl_shared_ip(
    size_t index_space
  )
  const
  {
    return excl_shared_ips_[index_space];
  }

  //--------------------------------------------------------------------------//
  //! Push primary logical region corresponding to a virtual index space.
  //!
  //! @param region primary logical region 
  //--------------------------------------------------------------------------//

  void
  push_primary_lr(
    Legion::LogicalRegion primary_lr
  )
  {
    primary_lrs_.push_back(primary_lr);
  }

  //--------------------------------------------------------------------------//
  //! Return primary logical region for virtual index space.
  //!
  //! @param index_space virtual index space
  //--------------------------------------------------------------------------//

  Legion::LogicalRegion
  get_primary_lr(
    size_t index_space
  )
  const
  {
    return primary_lrs_[index_space];
  }

  //--------------------------------------------------------------------------//
  //! Push ghost logical region corresponding to a virtual index space.
  //!
  //! @param region ghost logical region 
  //--------------------------------------------------------------------------//

  void
  push_ghost_lr(
    Legion::LogicalRegion ghost_lr
  )
  {
    ghost_lrs_.push_back(ghost_lr);
  }

  //--------------------------------------------------------------------------//
  //! Return ghost logical region for virtual index space.
  //!
  //! @param index_space virtual index space
  //--------------------------------------------------------------------------//

  Legion::LogicalRegion
  get_ghost_lr(
    size_t index_space
  )
  const
  {
    return ghost_lrs_[index_space];
  }

  //--------------------------------------------------------------------------//
  //! Push exclusive logical region corresponding to a virtual index space.
  //!
  //! @param region exclusive logical region
  //--------------------------------------------------------------------------//

  void
  push_exclusive_lr(
    Legion::LogicalRegion region
  )
  {
    exclusive_lrs_.push_back(region);
  }

  //--------------------------------------------------------------------------//
  //! Return exclusive logical region for virtual index space.
  //!
  //! @param index_space virtual index space
  //--------------------------------------------------------------------------//

  Legion::LogicalRegion
  get_exclusive_lr(
    size_t index_space
  )
  const
  {
    return exclusive_lrs_[index_space];
  }

  //--------------------------------------------------------------------------//
  //! Push shared logical region corresponding to a virtual index space.
  //!
  //! @param region shared logical region
  //--------------------------------------------------------------------------//

  void
  push_shared_lr(
    Legion::LogicalRegion region
  )
  {
    shared_lrs_.push_back(region);
  }

  //--------------------------------------------------------------------------//
  //! Return shared logical region for virtual index space.
  //!
  //! @param index_space virtual index space
  //--------------------------------------------------------------------------//

  Legion::LogicalRegion
  get_shared_lr(
    size_t index_space
  )
  const
  {
    return shared_lrs_[index_space];
  }

  //--------------------------------------------------------------------------//
  // FIXME: Not sure if this is needed...
  //--------------------------------------------------------------------------//

private:

  //--------------------------------------------------------------------------//
  // Task data members.
  //--------------------------------------------------------------------------//

  // Map to store task registration callback methods.
  std::map<
    size_t,
    task_info_t
  > task_registry_;

  //--------------------------------------------------------------------------//
  // Function data members.
  //--------------------------------------------------------------------------//

  std::unordered_map<size_t, void *>
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

  //--------------------------------------------------------------------------//
  // Legion data members within SPMD task.
  //--------------------------------------------------------------------------//

  Legion::PhaseBarrier* pbarriers_as_masters_;
  std::vector<Legion::PhaseBarrier*> ghost_owners_pbarriers_;
  std::vector<std::vector<Legion::LogicalRegion>> ghost_owners_lregions_;
  std::vector<Legion::LogicalRegion> color_regions_;
  std::vector<Legion::IndexPartition> primary_ghost_ips_;
  std::vector<Legion::LogicalRegion> primary_lrs_;
  std::vector<Legion::LogicalRegion> ghost_lrs_;
  std::vector<Legion::IndexPartition> excl_shared_ips_;
  std::vector<Legion::LogicalRegion> exclusive_lrs_;
  std::vector<Legion::LogicalRegion> shared_lrs_;

}; // class legion_context_policy_t

} // namespace execution
} // namespace flecsi

#endif // flecsi_execution_legion_context_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
