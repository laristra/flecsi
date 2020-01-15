/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

/*! @file */

#include <functional>
#include <memory>
#include <stack>
#include <unordered_map>

#include <cinchlog.h>
#include <flecsi-config.h>

#if !defined(FLECSI_ENABLE_LEGION)
#error FLECSI_ENABLE_LEGION not defined! This file depends on Legion!
#endif

#include <legion.h>
#include <legion_stl.h>

#if !defined(FLECSI_ENABLE_MPI)
#error FLECSI_ENABLE_MPI not defined! This file depends on MPI!
#endif

#include <mpi.h>

#include <flecsi/execution/common/launch.h>
#include <flecsi/execution/common/processor.h>
#include <flecsi/execution/legion/future.h>
#include <flecsi/execution/legion/internal_field.h>
#include <flecsi/execution/legion/runtime_driver.h>
#include <flecsi/execution/legion/runtime_state.h>
#include <flecsi/runtime/types.h>
#include <flecsi/utils/common.h>

namespace flecsi {
namespace execution {

/*!
  mapper tag's IDs

  @ingroup legion-execution
 */

// we need to have them here to avoid circular dependency
// FIXME : should we generate theese IDs somewhere?
enum {
  // Use the first 8 bits for storing the rhsf index
  MAPPER_FORCE_RANK_MATCH = 0x00001000,
  MAPPER_COMPACTED_STORAGE = 0x00002000,
  MAPPER_SUBRANK_LAUNCH = 0x00003000,
  EXCLUSIVE_LR = 0x00004000,
  PREFER_GPU = 0x11000001,
  PREFER_OMP = 0x11000002,
};

/*!
  The legion_context_policy_t is the backend runtime context policy for
  Legion.

  @ingroup legion-execution
 */

struct legion_context_policy_t {
  const size_t TOP_LEVEL_TASK_ID = 0;

  /*!
     The registration_function_t type defines a function type for
     registration callbacks.
   */

  using registration_function_t =
    std::function<void(task_id_t, processor_type_t, launch_t, std::string &)>;

  /*!
   The unique_tid_t type create a unique id generator for registering
   tasks.
   */

  using unique_tid_t = utils::unique_id_t<task_id_t, FLECSI_GENERATED_ID_MAX>;

  /*!
    The task_info_t type is a convenience type for defining the task
    registration map below.
   */

  using task_info_t = std::tuple<task_id_t,
    processor_type_t,
    launch_t,
    std::string,
    registration_function_t>;

  struct sparse_field_data_t {
    sparse_field_data_t() {}

    sparse_field_data_t(size_t type_size,
      size_t num_exclusive,
      size_t num_shared,
      size_t num_ghost,
      size_t max_entries_per_index)
      : type_size(type_size), num_exclusive(num_exclusive),
        num_shared(num_shared), num_ghost(num_ghost),
        num_total(num_exclusive + num_shared + num_ghost),
        max_entries_per_index(max_entries_per_index) {}

    size_t type_size;

    // total # of exclusive, shared, ghost entries
    size_t num_exclusive = 0;
    size_t num_shared = 0;
    size_t num_ghost = 0;
    size_t num_total = 0;

    size_t max_entries_per_index;
  };

  //--------------------------------------------------------------------------//
  // Runtime state.
  //--------------------------------------------------------------------------//

  /*!
    FleCSI context initialization. This method initializes the FleCSI
    runtime using Legion.

    @param argc The command-line argument count passed from main.
    @param argv The command-line argument values passed from main.

    @return An integer value with a non-zero error code upon failure,
           zero otherwise.
   */

  int initialize(int argc, char ** argv);

  /*!
    Return the color for which the context was initialized.
   */
  // FIXME: As long as MAPPER_FORCE_RANK_MATCH is really only doing RDMA and
  // not forcing the rank match, this will never do what it is supposed to
  // without a setter.

  size_t color() const {
    auto runtime = Legion::Runtime::get_runtime();
    return runtime->find_local_MPI_rank();
  } // color

  void set_color(size_t color) {
    color_ = color;
  }

  /*!
    Return the number of colors.
   */

  size_t colors() const {
    return colors_;
  } // color

  //--------------------------------------------------------------------------//
  //  MPI interoperability.
  //--------------------------------------------------------------------------//

  /*!
    Set the MPI runtime state. When the state is changed to active,
    the handshake interface will begin executing the current MPI task.

    @return A boolean indicating the current MPI runtime state.
   */

  bool set_mpi_state(bool active) {
    {
      clog_tag_guard(context);
      clog(info) << "set_mpi_state " << active << std::endl;
    }

    mpi_active_ = active;
    return mpi_active_;
  } // toggle_mpi_state

  /*!
    Set the MPI user task. When control is given to the MPI runtime
    it will execute whichever function is currently set.
   */

  void set_mpi_task(std::function<void()> & mpi_task) {
    {
      clog_tag_guard(context);
      clog(info) << "set_mpi_task" << std::endl;
    }

    mpi_task_ = mpi_task;
  }

  /*!
    Invoke the current MPI task.
   */

  void invoke_mpi_task() {
    return mpi_task_();
  } // invoke_mpi_task

  /*!
    Set the distributed-memory domain.
   */

  void set_all_processes(const LegionRuntime::Arrays::Rect<1> & all_processes) {
    all_processes_ = all_processes;
  } // all_processes

  /*!
     Return the distributed-memory domain.
   */

  const LegionRuntime::Arrays::Rect<1> & all_processes() const {
    return all_processes_;
  } // all_processes

  /*!
     Handoff to legion runtime from MPI.
   */

  void handoff_to_legion() {
    {
      clog_tag_guard(context);
      clog(info) << "handoff_to_legion" << std::endl;
    }
    MPI_Barrier(MPI_COMM_WORLD);
    handshake_.mpi_handoff_to_legion();
  } // handoff_to_legion

  /*!
    Wait for Legion runtime to complete.
   */

  void wait_on_legion() {
    {
      clog_tag_guard(context);
      clog(info) << "wait_on_legion" << std::endl;
    }

    handshake_.mpi_wait_on_legion();
    MPI_Barrier(MPI_COMM_WORLD);
  } // wait_on_legion

  /*!
    Handoff to MPI from Legion.
   */

  void handoff_to_mpi() {
    {
      clog_tag_guard(context);
      clog(info) << "handoff_to_mpi" << std::endl;
    }

    handshake_.legion_handoff_to_mpi();
  } // handoff_to_mpi

  /*!
    Wait for MPI runtime to complete task execution.
   */

  void wait_on_mpi() {
    {
      clog_tag_guard(context);
      clog(info) << "wait_on_mpi" << std::endl;
    }

    handshake_.legion_wait_on_mpi();
  } // wait_on_legion

  template<typename LAUNCHERTYPE>
  void add_wait_handshake(LAUNCHERTYPE & l) {
    l.add_wait_handshake(handshake_);
  }

  template<typename LAUNCHERTYPE>
  void add_arrival_handshake(LAUNCHERTYPE & l) {
    l.add_arrival_handshake(handshake_);
  }

  void advance_handshake() {
    handshake_.advance_legion_handshake();
  }

  /*!
    Unset the MPI active state to pass execution back to
    the Legion runtime.

    @param ctx The Legion runtime context.
    @param runtime The Legion task runtime pointer.
   */

  void unset_call_mpi(Legion::Context & ctx, Legion::Runtime * runtime);

  Legion::FutureMap unset_call_mpi_single();
  Legion::FutureMap unset_call_mpi_index(Legion::Context & ctx,
    Legion::Runtime * runtime);

  /*!
    Switch execution to the MPI runtime.

    @param ctx The Legion runtime context.
    @param runtime The Legion task runtime pointer.
   */

  void handoff_to_mpi(Legion::Context & ctx, Legion::Runtime * runtime);

  void handoff_to_mpi_single() {
    handshake_.legion_handoff_to_mpi();
  }

  /*!
    Wait on the MPI runtime to finish the current task execution.

    @param ctx The Legion runtime context.
    @param runtime The Legion task runtime pointer.

    @return A future map with the result of the task execution.
   */

  Legion::FutureMap wait_on_mpi(Legion::Context & ctx,
    Legion::Runtime * runtime);

  void wait_on_mpi_single() {
    handshake_.legion_wait_on_mpi();
  }

  /*!
    Connect with the MPI runtime.

    @param ctx The Legion runtime context.
    @param runtime The Legion task runtime pointer.
   */

  void connect_with_mpi(Legion::Context & ctx, Legion::Runtime * runtime);

  //--------------------------------------------------------------------------//
  // Task interface.
  //--------------------------------------------------------------------------//

  /*!
    Register a task with the runtime.

    @param key       The task hash key.
    @param name      The task name string.
    @param callback The registration call back function.
   */

  bool register_task(size_t key,
    processor_type_t processor,
    launch_t launch,
    std::string & name,
    const registration_function_t & callback) {
    clog(info) << "Registering task callback " << name << " with key " << key
               << std::endl;

    clog_assert(task_registry_.find(key) == task_registry_.end(),
      "task key already exists");

    task_registry_[key] = std::make_tuple(
      unique_tid_t::instance().next(), processor, launch, name, callback);

    return true;
  } // register_task

  /*!
    Return the task registration tuple.

    @param key The task hash key.
   */

  template<size_t KEY>
  task_info_t & task_info() {
    auto task_entry = task_registry_.find(KEY);

    clog_assert(task_entry != task_registry_.end(),
      "task key " << KEY << " does not exist");

    return task_entry->second;
  } // task_info

  /*!
    Return the task registration tuple.

    @param key The task hash key.
   */

  task_info_t & task_info(size_t key) {
    auto task_entry = task_registry_.find(key);

    clog_assert(task_entry != task_registry_.end(),
      "task key " << key << " does not exist");

    return task_entry->second;
  } // task_info

  /*!
    FIXME

    @param key The task hash key.
   */

#define task_info_template_method(name, return_type, index)                    \
  template<size_t KEY>                                                         \
  return_type name() {                                                         \
    {                                                                          \
      clog_tag_guard(context);                                                 \
      clog(info) << "Returning " << #name << " for " << KEY << std::endl;      \
    }                                                                          \
    return std::get<index>(task_info<KEY>());                                  \
  }

  /*!
    FIXME

    @param key The task hash key.
   */

#define task_info_method(name, return_type, index)                             \
  return_type name(size_t key) {                                               \
    {                                                                          \
      clog_tag_guard(context);                                                 \
      clog(info) << "Returning " << #name << " for " << key << std::endl;      \
    }                                                                          \
    return std::get<index>(task_info(key));                                    \
  }

  /*!
    FIXME

    @param key The task hash key.
   */

  task_info_template_method(task_id, task_id_t, 0);
  task_info_method(task_id, task_id_t, 0);
  task_info_template_method(processor_type, processor_type_t, 1);
  task_info_method(processor_type, processor_type_t, 1);

  //--------------------------------------------------------------------------//
  // Legion runtime interface.
  //--------------------------------------------------------------------------//

  /*!
    Collects Legion data associated with a FleCSI index space.
   */

  struct index_space_data_t {
    std::map<field_id_t, bool> ghost_is_readable;
    std::map<field_id_t, bool> write_phase_started;
    Legion::IndexPartition ghost_owners_ip; // prevent Destructor call
    Legion::LogicalPartition ghost_owners_lp;
    Legion::LogicalRegion entire_region;
    Legion::LogicalPartition color_partition;
    Legion::LogicalPartition primary_lp;
    Legion::LogicalPartition exclusive_lp;
    Legion::LogicalPartition shared_lp;
    Legion::LogicalPartition ghost_lp;
  };

  struct index_subspace_data_t {
    Legion::LogicalRegion logical_region;
    Legion::LogicalPartition logical_partition;
  };

  struct sparse_metadata_t {
    Legion::LogicalPartition color_partition;
    Legion::LogicalRegion entire_region;
  };

  void set_sparse_metadata(const sparse_metadata_t & sparse_metadata) {
    sparse_metadata_ = sparse_metadata;
  }

  sparse_metadata_t & sparse_metadata() {
    return sparse_metadata_;
  }

  /*!
    Get the index space data map.
   */

  auto & index_space_data_map() {
    return index_space_data_map_;
  }

  /*!
    Get the index subspace data map.
   */

  auto & index_subspace_data_map() {
    return index_subspace_data_map_;
  }

  /*! Perform reduction of the maximum value
  @param task future
 */

  template<typename T, launch_type_t launch>
  auto reduce_max(legion_future_u<T, launch> & global_future) {

    auto global_max_ = global_future.get();

    return global_future;
  }

  /*!
    Perform reduction of the minimum value
    @param task future
   */

  template<typename T, launch_type_t launch>
  auto reduce_min(legion_future_u<T, launch> & global_future) {

    auto global_min_ = global_future.get();

    return global_future;
  }
  /*!
    Return the map of registered reduction operations.
   */

  auto & reduction_operations() {
    return reduction_ops_;
  } // reduction_operations

  /*!
    Compute internal field id for from/to index space pair for connectivity.
    @param from_index_space from index space
    @param to_index_space to index space
   */

  size_t adjacency_fid(size_t from_index_space, size_t to_index_space) const {
    return size_t(internal_field::adjacency_pos_start) + from_index_space * 10 +
           to_index_space;
  }

  size_t entity_data_fid(size_t index_space) const {
    return size_t(internal_field::entity_data_start) + index_space;
  }

private:
  size_t color_ = 0;
  size_t colors_ = 0;

  //--------------------------------------------------------------------------//
  // Task data members.
  //--------------------------------------------------------------------------//

  // Map to store task registration callback methods.
  std::map<size_t, task_info_t> task_registry_;

  //--------------------------------------------------------------------------//
  // Function data members.
  //--------------------------------------------------------------------------//

  std::unordered_map<size_t, void *> function_registry_;

  //--------------------------------------------------------------------------//
  // Reduction operations.
  //--------------------------------------------------------------------------//

  std::map<size_t, size_t> reduction_ops_;

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
  std::map<size_t, index_subspace_data_t> index_subspace_data_map_;
  sparse_metadata_t sparse_metadata_;

}; // class legion_context_policy_t

} // namespace execution
} // namespace flecsi
