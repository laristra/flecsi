/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Triad National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

/*! @file */

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly!
#else
#include <flecsi/data/legion/runtime_data_types.h>
#include <flecsi/execution/common/launch.h>
#include <flecsi/execution/common/processor.h>
#include <flecsi/runtime/types.h>
#include <flecsi/utils/common.h>
#endif

#if !defined(FLECSI_ENABLE_LEGION)
#error FLECSI_ENABLE_LEGION not defined! This file depends on Legion!
#endif

#include <legion.h>

#if !defined(FLECSI_ENABLE_MPI)
#error FLECSI_ENABLE_MPI not defined! This file depends on MPI!
#endif

#include <boost/program_options.hpp>
#include <mpi.h>

#include <map>
#include <unordered_map>

namespace flecsi {
namespace execution {

using namespace boost::program_options;

const size_t FLECSI_TOP_LEVEL_TASK_ID = 0;
const size_t FLECSI_MAPPER_FORCE_RANK_MATCH = 0x00001000;
const size_t FLECSI_MAPPER_COMPACTED_STORAGE = 0x00002000;
const size_t FLECSI_MAPPER_SUBRANK_LAUNCH = 0x00003000;
const size_t FLECSI_MAPPER_EXCLUSIVE_LR = 0x00004000;

// Forward declarations
void top_level_task(const Legion::Task * task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx,
  Legion::Runtime * runtime);
void handoff_to_mpi_task(const Legion::Task * task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx,
  Legion::Runtime * runtime);
void wait_on_mpi_task(const Legion::Task * task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx,
  Legion::Runtime * runtime);
void unset_call_mpi_task(const Legion::Task * task,
  const std::vector<Legion::PhysicalRegion> & regions,
  Legion::Context ctx,
  Legion::Runtime * runtime);

struct legion_context_policy_t {

  friend void top_level_task(const Legion::Task * task,
    const std::vector<Legion::PhysicalRegion> & regions,
    Legion::Context ctx,
    Legion::Runtime * runtime);
  friend void handoff_to_mpi_task(const Legion::Task * task,
    const std::vector<Legion::PhysicalRegion> & regions,
    Legion::Context ctx,
    Legion::Runtime * runtime);
  friend void wait_on_mpi_task(const Legion::Task * task,
    const std::vector<Legion::PhysicalRegion> & regions,
    Legion::Context ctx,
    Legion::Runtime * runtime);
  friend void unset_call_mpi_task(const Legion::Task * task,
    const std::vector<Legion::PhysicalRegion> & regions,
    Legion::Context ctx,
    Legion::Runtime * runtime);

  /*!
     The registration_function_t type defines a function type for
     registration callbacks.
   */

  using registration_function_t = std::function<
    void(task_id_t, processor_type_t, task_execution_type_t, std::string &)>;

  /*!
    The task_info_t type is a convenience type for defining the task
    registration map below.
   */

  using task_info_t = std::tuple<task_id_t,
    processor_type_t,
    task_execution_type_t,
    std::string,
    registration_function_t>;

  /*
    Documentation for this interface is in the top-level context type.
   */

  int start(int argc, char ** argv, variables_map & vm);

  /*
    Documentation for this interface is in the top-level context type.
   */

  size_t process() const {
    return process_;
  } // process

  /*
    Documentation for this interface is in the top-level context type.
   */

  size_t processes() const {
    return processes_;
  } // processes

  size_t threads_per_process() const {
    return threads_per_process_;
  } // threads_per_process

  size_t threads() const {
    return threads_;
  } // threads

  /*
    Documentation for this interface is in the top-level context type.
   */

  static size_t task_depth() {
    return Legion::Runtime::get_runtime()
      ->get_current_task(Legion::Runtime::get_context())
      ->get_depth();
  } // task_depth

  /*
    Documentation for this interface is in the top-level context type.
   */

  static size_t color() {
    flog_assert(
      task_depth() > 0, "this method can only be called from within a task");
    return Legion::Runtime::get_runtime()
      ->get_current_task(Legion::Runtime::get_context())
      ->index_point.point_data[0];
  } // color

  /*
    Documentation for this interface is in the top-level context type.
   */

  static size_t colors() {
    flog_assert(
      task_depth() > 0, "this method can only be called from within a task");
    return Legion::Runtime::get_runtime()
      ->get_current_task(Legion::Runtime::get_context())
      ->index_domain.get_volume();
  } // colors

  /*!
   */

  data::legion::global_runtime_data_t & global_topology_instance() {
    return global_topology_instance_;
  } // global_topology_instance

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
      flog_tag_guard(context);
      flog(internal) << "In set_mpi_state " << active << std::endl;
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
      flog_tag_guard(context);
      flog(internal) << "In set_mpi_task" << std::endl;
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
      flog_tag_guard(context);
      flog(internal) << "In handoff_to_legion" << std::endl;
    }
    MPI_Barrier(MPI_COMM_WORLD);
    handshake_.mpi_handoff_to_legion();
  } // handoff_to_legion

  /*!
    Wait for Legion runtime to complete.
   */

  void wait_on_legion() {
    {
      flog_tag_guard(context);
      flog(internal) << "In wait_on_legion" << std::endl;
    }

    handshake_.mpi_wait_on_legion();
    MPI_Barrier(MPI_COMM_WORLD);
  } // wait_on_legion

  /*!
    Handoff to MPI from Legion.
   */

  void handoff_to_mpi() {
    {
      flog_tag_guard(context);
      flog(internal) << "In handoff_to_mpi" << std::endl;
    }

    handshake_.legion_handoff_to_mpi();
  } // handoff_to_mpi

  /*!
    Wait for MPI runtime to complete task execution.
   */

  void wait_on_mpi() {
    {
      flog_tag_guard(context);
      flog(internal) << "In wait_on_mpi" << std::endl;
    }

    handshake_.legion_wait_on_mpi();
  } // wait_on_legion

  /*!
    Unset the MPI active state to pass execution back to
    the Legion runtime.

    @param ctx The Legion runtime context.
    @param runtime The Legion task runtime pointer.
   */

  void unset_call_mpi(Legion::Context & ctx, Legion::Runtime * runtime);

  /*!
    Switch execution to the MPI runtime.

    @param ctx The Legion runtime context.
    @param runtime The Legion task runtime pointer.
   */

  void handoff_to_mpi(Legion::Context & ctx, Legion::Runtime * runtime);

  /*!
    Wait on the MPI runtime to finish the current task execution.

    @param ctx The Legion runtime context.
    @param runtime The Legion task runtime pointer.

    @return A future map with the result of the task execution.
   */

  Legion::FutureMap wait_on_mpi(Legion::Context & ctx,
    Legion::Runtime * runtime);

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

    @param key      The task hash key.
    @param name     The task name string.
    @param callback The registration call back function.
   */

  bool register_task(size_t key,
    processor_type_t processor,
    task_execution_type_t execution,
    std::string & name,
    const registration_function_t & callback) {
    flog(internal) << "Registering task callback" << std::endl
                   << "\tname: " << name << std::endl
                   << "\thash: " << key << std::endl;

    flog_assert(task_registry_.find(key) == task_registry_.end(),
      "task key already exists");

    task_registry_[key] = std::make_tuple(
      unique_tid_t::instance().next(), processor, execution, name, callback);

    return true;
  } // register_task

  /*!
    Return the task registration tuple.

    @param key The task hash key.
   */

  template<size_t KEY>
  task_info_t & task_info() {
    auto task_entry = task_registry_.find(KEY);

    flog_assert(task_entry != task_registry_.end(),
      "task key " << KEY << " does not exist");

    return task_entry->second;
  } // task_info

  /*!
    Return the task registration tuple.

    @param key The task hash key.
   */

  task_info_t & task_info(size_t key) {
    auto task_entry = task_registry_.find(key);

    flog_assert(task_entry != task_registry_.end(),
      "task key " << key << " does not exist");

    return task_entry->second;
  } // task_info

  /*!
    Return task key information.

    @param key The task hash key.
   */

#define task_info_template_method(name, return_type, index)                    \
  template<size_t KEY>                                                         \
  return_type name() {                                                         \
    {                                                                          \
      flog_tag_guard(context);                                                 \
      flog(internal) << "Returning task info" << std::endl                     \
                     << "\tname: " << #name << std::endl                       \
                     << "\thash: " << KEY << std::endl;                        \
    }                                                                          \
    return std::get<index>(task_info<KEY>());                                  \
  }

  /*!
    Return task key information.

    @param key The task hash key.
   */

#define task_info_method(name, return_type, index)                             \
  return_type name(size_t key) {                                               \
    {                                                                          \
      flog_tag_guard(context);                                                 \
      flog(internal) << "Returning task info" << std::endl                     \
                     << "\tname: " << #name << std::endl                       \
                     << "\thash: " << key << std::endl;                        \
    }                                                                          \
    return std::get<index>(task_info(key));                                    \
  }

  // clang-format off
  task_info_template_method(task_id, task_id_t, 0);
  task_info_method(task_id, task_id_t, 0);
  task_info_template_method(processor_type, processor_type_t, 1);
  task_info_method(processor_type, processor_type_t, 1);
  // clang-format on

  //--------------------------------------------------------------------------//
  // Reduction interface.
  //--------------------------------------------------------------------------//

  /*!
    Return the map of registered reduction operations.
   */

  auto & reduction_operations() {
    return reduction_ops_;
  } // reduction_operations

private:
  /*--------------------------------------------------------------------------*
    Backend initialization.
   *--------------------------------------------------------------------------*/

  void initialize_global_topology();
  void initialize_index_topology();

  /*--------------------------------------------------------------------------*
    Runtime data.
   *--------------------------------------------------------------------------*/

  data::legion::global_runtime_data_t global_topology_instance_;
  std::unordered_map<size_t, data::legion::index_runtime_data_t>
    index_topology_instances_;

  size_t process_ = std::numeric_limits<size_t>::max();
  size_t processes_ = std::numeric_limits<size_t>::max();
  size_t threads_per_process_ = std::numeric_limits<size_t>::max();
  size_t threads_ = std::numeric_limits<size_t>::max();

  /*--------------------------------------------------------------------------*
    Interoperability data members.
   *--------------------------------------------------------------------------*/

  std::function<void()> mpi_task_;
  bool mpi_active_ = false;
  Legion::MPILegionHandshake handshake_;
  LegionRuntime::Arrays::Rect<1> all_processes_;

  /*--------------------------------------------------------------------------*
    Task data members.
   *--------------------------------------------------------------------------*/

  std::map<size_t, task_info_t> task_registry_;

  /*--------------------------------------------------------------------------*
    Function data members.
   *--------------------------------------------------------------------------*/

  std::unordered_map<size_t, void *> function_registry_;

  /*--------------------------------------------------------------------------*
    Reduction data members.
   *--------------------------------------------------------------------------*/

  std::map<size_t, size_t> reduction_ops_;

}; // struct legion_context_policy_t

} // namespace execution
} // namespace flecsi
