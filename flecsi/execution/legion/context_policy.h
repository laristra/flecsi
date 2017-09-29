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
#include <legion_stl.h>

#if !defined(ENABLE_MPI)
  #error ENABLE_MPI not defined! This file depends on MPI!
#endif

#include <mpi.h>

#include "flecsi/execution/common/launch.h"
#include "flecsi/execution/common/processor.h"
#include "flecsi/execution/legion/internal_field.h"
#include "flecsi/execution/legion/runtime_driver.h"
#include "flecsi/execution/legion/runtime_state.h"
#include "flecsi/execution/legion/future.h"
#include "flecsi/runtime/types.h"
#include "flecsi/utils/common.h"
#include "flecsi/utils/const_string.h"
#include "flecsi/utils/tuple_wrapper.h"

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

#if !defined(ENABLE_LEGION_TLS)
  extern thread_local std::unordered_map<size_t,
    std::stack<std::shared_ptr<legion_runtime_state_t>>> state_;
#endif

//----------------------------------------------------------------------------//
//! mapper tag's IDs
//!
//! @ingroup legion-execution
//----------------------------------------------------------------------------//
//we need to have them here to avoid circular dependency
//FIXME : should we generate theese IDs somewhere?
enum {
  // Use the first 8 bits for storing the rhsf index
  MAPPER_FORCE_RANK_MATCH = 0x00001000,
  MAPPER_COMPACTED_STORAGE = 0x00002000,
  MAPPER_SUBRANK_LAUNCH   = 0x00003000,
  EXCLUSIVE_LR            = 0x00004000,
};


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
  //! The registration_function_t type defines a function type for
  //! registration callbacks.
  //--------------------------------------------------------------------------//

  using registration_function_t =
    std::function<void(task_id_t, processor_type_t, launch_t, std::string &)>;

  //--------------------------------------------------------------------------//
  //! The unique_tid_t type create a unique id generator for registering
  //! tasks.
  //--------------------------------------------------------------------------//

  using unique_tid_t =
    utils::unique_id_t<task_id_t, FLECSI_GENERATED_ID_MAX>;

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
  //! Return the color for which the context was initialized.
  //--------------------------------------------------------------------------//

  size_t
  color()
  const
  {
    return color_;
  } // color

  //--------------------------------------------------------------------------//
  //! Return the number of colors.
  //--------------------------------------------------------------------------//

  size_t
  colors()
  const
  {
    return colors_;
  } // color

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

#if !defined(ENABLE_LEGION_TLS)
  void push_state(
    size_t key,
    Legion::Context & context,
    Legion::Runtime * runtime,
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
#endif

  //--------------------------------------------------------------------------//
  //! Pop Legion runtime state off of the stack.
  //!
  //! @param key The task hash key.
  //--------------------------------------------------------------------------//

#if !defined(ENABLE_LEGION_TLS)
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
#endif

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
    Legion::Runtime * runtime
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
    Legion::Runtime * runtime
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
    Legion::Runtime * runtime
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
    Legion::Runtime * runtime
  );

  //--------------------------------------------------------------------------//
  // Task interface.
  //--------------------------------------------------------------------------//

  //--------------------------------------------------------------------------//
  //! Register a task with the runtime.
  //!
  //! @param key       The task hash key.
  //! @param name      The task name string.
  //! @param callback The registration call back function.
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
  // Legion runtime interface.
  //--------------------------------------------------------------------------//

  //--------------------------------------------------------------------------//
  //! Return the Legion task runtime context for the given task key.
  //!
  //! @param key The task hash key.
  //--------------------------------------------------------------------------//

#if !defined(ENABLE_LEGION_TLS)
  Legion::Context &
  context(
    size_t key
  )
  {
    return state_[key].top()->context;
  } // context
#endif

  //--------------------------------------------------------------------------//
  //! Return the Legion task runtime pointer for the given task key.
  //!
  //! @param key The task hash key.
  //--------------------------------------------------------------------------//

#if !defined(ENABLE_LEGION_TLS)
  Legion::Runtime *
  runtime(
    size_t key
  )
  {
    return state_[key].top()->runtime;
  } // runtime
#endif

  //--------------------------------------------------------------------------//
  //! Collects Legion data associated with a FleCSI index space.
  //--------------------------------------------------------------------------//

  struct index_space_data_t{
    std::map<field_id_t, bool> ghost_is_readable;
    std::map<field_id_t, bool> write_phase_started;
    std::map<field_id_t, Legion::PhaseBarrier> pbarriers_as_owner;
    std::map<field_id_t, std::vector<Legion::PhaseBarrier>>
       ghost_owners_pbarriers;
    std::vector<Legion::LogicalRegion> ghost_owners_lregions;
    std::vector<Legion::LogicalRegion> ghost_owners_subregions;
    Legion::STL::map<LegionRuntime::Arrays::coord_t,
      LegionRuntime::Arrays::coord_t> global_to_local_color_map;
    Legion::LogicalRegion color_region;
    Legion::LogicalRegion primary_lr;
    Legion::LogicalRegion exclusive_lr;
    Legion::LogicalRegion shared_lr;
    Legion::LogicalRegion ghost_lr;
  };

  //------------------------------------------------------------------------//
  //! Get index space data.
  //!
  //! @param index_space FleCSI index space, e.g. cells key
  //------------------------------------------------------------------------//

  auto&
  index_space_data_map()
  {
    return index_space_data_map_;
  }

  //--------------------------------------------------------------------------//
  //! Set DynamicCollective for <double> max reduction
  //!
  //! @param max_reduction Legion DynamicCollective for <double> max reduction
  //--------------------------------------------------------------------------//

  void
  set_max_reduction(Legion::DynamicCollective& max_reduction)
  {
    max_reduction_ = max_reduction;
  }

  //--------------------------------------------------------------------------//
  //! Get DynamicCollective for <double> max reduction
  //--------------------------------------------------------------------------//

  auto&
  max_reduction()
  {
    return max_reduction_;
  }

  //-------------------------------------------------------------------------//
  //! Perform reduction of the maximum value
  //! @param task future
  //-------------------------------------------------------------------------//

  template <typename T>
  auto
  reduce_max(legion_future__<T> & local_future)
  {
    Legion::DynamicCollective& max_reduction = max_reduction_;

    auto legion_runtime = Legion::Runtime::get_runtime();
    auto legion_context = Legion::Runtime::get_context();

    local_future.defer_dynamic_collective_arrival(
      legion_runtime, legion_context, max_reduction
    );

    max_reduction =
      legion_runtime->advance_dynamic_collective(legion_context, max_reduction);

    auto global_future = legion_runtime->get_dynamic_collective_result(
      legion_context, max_reduction
    );

    auto global_max_ = global_future.get_result<double>();

    return global_max_;
  }


  //--------------------------------------------------------------------------//
  //! Set DynamicCollective for <double> in reduction
  //!
  //! @param min_reduction Legion DynamicCollective for <double> max reduction
  //--------------------------------------------------------------------------//

  void
  set_min_reduction(Legion::DynamicCollective& min_reduction)
  {
    min_reduction_ = min_reduction;
  }

  //--------------------------------------------------------------------------//
  //! Get DynamicCollective for <double> max reduction
  //--------------------------------------------------------------------------//

  auto&
  min_reduction()
  {
    return min_reduction_;
  }

  //-------------------------------------------------------------------------//
  //! Perform reduction of the minimum value
  //! @param task future
  //-------------------------------------------------------------------------//

  template <typename T>
  auto
  reduce_min(legion_future__<T> & local_future)
  {
    Legion::DynamicCollective& min_reduction = min_reduction_;

    auto legion_runtime = Legion::Runtime::get_runtime();
    auto legion_context = Legion::Runtime::get_context();

    local_future.defer_dynamic_collective_arrival(
      legion_runtime, legion_context, min_reduction
    );

    min_reduction =
      legion_runtime->advance_dynamic_collective(legion_context, min_reduction);

    auto global_future = legion_runtime->get_dynamic_collective_result(
      legion_context, min_reduction
    );

    auto global_min_ = global_future.get_result<double>();

    return global_min_;
  }

  //--------------------------------------------------------------------------//
  //! Compute internal field id for from/to index space pair for connectivity.
  //! @param from_index_space from index space
  //! @param to_index_space to index space
  //!-------------------------------------------------------------------------//

  size_t
  adjacency_fid(size_t from_index_space, size_t to_index_space)
  const
  {
    return size_t(internal_field::adjacency_pos_start) + 
      from_index_space * 10 + to_index_space;
  }

  size_t
  entity_data_fid(size_t index_space)
  const
  {
    return size_t(internal_field::entity_data_start) + index_space;
  }


private:

  size_t color_ = 0;
  size_t colors_ = 0;

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

  std::map<size_t, index_space_data_t> index_space_data_map_;
  Legion::DynamicCollective max_reduction_;
  Legion::DynamicCollective min_reduction_;

}; // class legion_context_policy_t

} // namespace execution
} // namespace flecsi

#endif // flecsi_execution_legion_context_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
