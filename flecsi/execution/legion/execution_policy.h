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
#include <type_traits>

#include <cinchlog.h>
#include <flecsi-config.h>

#if !defined(FLECSI_ENABLE_LEGION)
#error FLECSI_ENABLE_LEGION not defined! This file depends on Legion!
#endif

#include <legion.h>

#include <flecsi/execution/common/processor.h>
#include <flecsi/execution/context.h>
#include <flecsi/execution/legion/context_policy.h>
#include <flecsi/execution/legion/future.h>
#include <flecsi/execution/legion/init_args.h>
#include <flecsi/execution/legion/runtime_state.h>
#include <flecsi/data/data.h>
#include <flecsi/execution/legion/task_epilog.h>
#include <flecsi/execution/legion/task_prolog.h>
#include <flecsi/execution/legion/task_wrapper.h>
#include <flecsi/utils/const_string.h>

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
// Execution policy.
//----------------------------------------------------------------------------//

/*!
  The legion_execution_policy_t is the backend runtime execution policy
  for Legion.

  @ingroup legion-execution
 */

struct legion_execution_policy_t {
  /*!
    The future__ type may be used for explicit synchronization of tasks.

    @tparam RETURN The return type of the task.
   */

  template<typename RETURN>
  using future__ = legion_future__<RETURN>;

  /*!
    The runtime_state_t type identifies a public type for the high-level
    runtime interface to pass state required by the backend.
   */

  using runtime_state_t = legion_runtime_state_t;

  /*!
    Return the runtime state of the calling FleCSI task.

    @param task The calling task.
   */

  static runtime_state_t & runtime_state(void * task);

  //--------------------------------------------------------------------------//
  // Task interface.
  //--------------------------------------------------------------------------//

  /*!
    This method allows the user to register a pure Legion task with
    the runtime. A task id will automatically be generated, and can be
    accessed via legion_context_policy_t::task_id using a valid
    task hash.

    @tparam KEY    A hash key identifying the task.
    @tparam RETURN The return type of the pure Legion task.
    @tparam TASK   The function pointer template type of the task.

    @param name The string name for the task. This can be set to any
                valid std::string value.
   */

  template<
      size_t KEY,
      typename RETURN,
      RETURN (*TASK)(
          const Legion::Task *,
          const std::vector<Legion::PhysicalRegion> &,
          Legion::Context,
          Legion::Runtime *)>
  static bool register_legion_task(
      processor_type_t processor,
      launch_t launch,
      std::string name) {
    clog(info) << "Registering legion task " << KEY << " " << name << std::endl;

    if (!context_t::instance().register_task(
            KEY, processor, launch, name,
            pure_task_wrapper__<RETURN, TASK>::registration_callback)) {
      clog(fatal) << "callback registration failed for " << name << std::endl;
    } // if

    return true;
  } // register_legion_task

  /*!
    Legion backend task registration. For documentation on this
    method, please see task__::register_task.
   */

  template<
      size_t KEY,
      typename RETURN,
      typename ARG_TUPLE,
      RETURN (*DELEGATE)(ARG_TUPLE)>
  static bool
  register_task(processor_type_t processor, launch_t launch, std::string name) {
    using wrapper_t = task_wrapper__<KEY, RETURN, ARG_TUPLE, DELEGATE>;

    if (!context_t::instance().register_task(
            KEY, processor, launch, name, wrapper_t::registration_callback)) {
      clog(fatal) << "callback registration failed for " << name << std::endl;
    } // if

    return true;
  } // register_task

  /*!
    Legion backend task execution. For documentation on this
    method, please see task__::execute_task.
   */

  template<size_t KEY, typename RETURN, typename ARG_TUPLE, typename... ARGS>
  static decltype(auto) execute_task(launch_type_t launch,
      Legion::ReductionOpID redop_id, ARGS &&... args) {
    using namespace Legion;

    // Make a tuple from the task arguments.
    ARG_TUPLE task_args = std::make_tuple(args...);

    context_t & context_ = context_t::instance();

    // Get the processor type.
    auto processor_type = context_.processor_type<KEY>();

    // Get the runtime and context from the current task.
    auto legion_runtime = Legion::Runtime::get_runtime();
    auto legion_context = Legion::Runtime::get_context();

    // Handle MPI and Legion invocations separately.
    if (processor_type == processor_type_t::mpi) {
      {
        clog_tag_guard(execution);
        clog(info) << "Executing MPI task: " << KEY << std::endl;
      }
      
      if (launch==launch_type_t::single){

        init_args_t init_args(legion_runtime, legion_context);
        init_args.walk(task_args);


        ArgumentMap arg_map;
        IndexLauncher launcher(
          context_.task_id<KEY>(),
          Legion::Domain::from_rect<1>(context_.all_processes()),
          TaskArgument(&task_args, sizeof(ARG_TUPLE)), arg_map);

        for (auto & req : init_args.region_reqs) {
            launcher.add_region_requirement(req);
          }

        //! \todo Do we need this comment?
        // Enqueue the MPI task.
        //  auto future =
        //    legion_runtime->execute_index_space(legion_context, launcher);
        // future.wait_all_results();

        Legion::MustEpochLauncher must_epoch_launcher;
        must_epoch_launcher.launch_domain =
            Legion::Domain::from_rect<1>(context_.all_processes());
        must_epoch_launcher.add_index_task(launcher);
        auto future = legion_runtime->execute_must_epoch(
          legion_context, must_epoch_launcher);
        future.wait_all_results(true);

        // Handoff to the MPI runtime.
        context_.handoff_to_mpi(legion_context, legion_runtime);

        // Wait for MPI to finish execution (synchronous).
        context_.wait_on_mpi(legion_context, legion_runtime);

        // Reset the calling state to false.
        context_.unset_call_mpi(legion_context, legion_runtime);

        return legion_future__<RETURN>(future);
      } else { // check for launch

         init_args_t init_args(legion_runtime, legion_context);
         init_args.walk(task_args);

          // Create a task launcher, passing the task arguments.
          TaskLauncher task_launcher(
              context_.task_id<KEY>(),
              TaskArgument(&task_args, sizeof(ARG_TUPLE)));

          for (auto & req : init_args.region_reqs) {
            task_launcher.add_region_requirement(req);
          }

          auto future =
              legion_runtime->execute_task(legion_context, task_launcher);

          future.wait();         
 
           // Handoff to the MPI runtime.
          context_.handoff_to_mpi();

        // Wait for MPI to finish execution (synchronous).
          context_.wait_on_mpi();

         context_.unset_call_mpi_single();

        return legion_future__<RETURN>(future);
      }//if check for execution state
    } else {
      // Initialize the arguments to pass through the runtime.
      init_args_t init_args(legion_runtime, legion_context);
      init_args.walk(task_args);

      LegionRuntime::Arrays::Rect<1> launch_bounds(0,context_.colors()-1);
      Domain launch_domain = Domain::from_rect<1>(launch_bounds);

      // Switch on launch type: single or index.
      switch (launch) {

        case launch_type_t::index: {
          clog_tag_guard(execution);
          clog(info) << "Executing single task: " << KEY << std::endl;

          // Create a task launcher, passing the task arguments.
          TaskLauncher task_launcher(
              context_.task_id<KEY>(),
              TaskArgument(&task_args, sizeof(ARG_TUPLE)));

#ifdef MAPPER_COMPACTION
          task_launcher.tag = MAPPER_COMPACTED_STORAGE;
#endif

          for (auto & req : init_args.region_reqs) {
            task_launcher.add_region_requirement(req);
          }

          // Enqueue the prolog.
          task_prolog_t task_prolog(
              legion_runtime, legion_context, launch_domain);
          task_prolog.walk(task_args);
          task_prolog.launch_copies();

          // Enqueue the task.
          clog(trace) << "Execute flecsi/legion task " << KEY << std::endl;
          auto future =
              legion_runtime->execute_task(legion_context, task_launcher);

          // Enqueue the epilog.
          task_epilog_t task_epilog(legion_runtime, legion_context);
          task_epilog.walk(task_args);

          return legion_future__<RETURN>(future);
        } // scope

        case launch_type_t::single: {
          clog_tag_guard(execution);
          clog(info) << "Executing single task: " << KEY << std::endl;

          // Create a task launcher, passing the task arguments.
          IndexTaskLauncher index_task_launcher(
              context_.task_id<KEY>(), launch_domain,
              TaskArgument(&task_args, sizeof(ARG_TUPLE)), Legion::ArgumentMap());

      //    index_task_launcher.tag = MAPPER_FORCE_RANK_MATCH;

#ifdef MAPPER_COMPACTION
          index_task_launcher.tag = MAPPER_COMPACTED_STORAGE;
#endif

          for (auto & req : init_args.region_reqs) {
            index_task_launcher.add_region_requirement(req);
          }

          // Enqueue the prolog.
          task_prolog_t task_prolog(legion_runtime, legion_context,
              launch_domain);
          task_prolog.walk(task_args);
          task_prolog.launch_copies();

          // Enqueue the task.
          clog(trace) << "Execute flecsi/legion task " << KEY << " on rank "
                      << legion_runtime->find_local_MPI_rank() << std::endl;

          Legion::Future future;
          Legion::FutureMap future_map;

          if (redop_id == 0) {
            Legion::MustEpochLauncher must_epoch_launcher;
            must_epoch_launcher.add_index_task(index_task_launcher);
            must_epoch_launcher.launch_domain = launch_domain;
            future_map = legion_runtime->execute_must_epoch(legion_context,
                must_epoch_launcher);
          }
          else
            future = legion_runtime->execute_index_space(legion_context,
                index_task_launcher, redop_id);

          // Enqueue the epilog.
          task_epilog_t task_epilog(legion_runtime, legion_context);
          task_epilog.walk(task_args);

          if (redop_id == 0)
            return legion_future__<RETURN>(future_map);
          else
            return legion_future__<RETURN>(future);
        } // scope

        default:
          clog(fatal) << "invalid launch type" << std::endl;
          throw std::runtime_error("invalid launch type");

      } // switch
    } // if
  } // execute_task

  //--------------------------------------------------------------------------//
  // Function interface.
  //--------------------------------------------------------------------------//

  /*!
    Legion backend function registration. For documentation on this
    method, please see function__::register_function.
   */

  template<
      size_t KEY,
      typename RETURN,
      typename ARG_TUPLE,
      RETURN (*FUNCTION)(ARG_TUPLE)>
  static bool register_function() {
    return context_t::instance()
        .template register_function<KEY, RETURN, ARG_TUPLE, FUNCTION>();
  } // register_function

  /*!
    Legion backend function execution. For documentation on this
    method, please see function__::execute_function.
   */

  template<typename FUNCTION_HANDLE, typename... ARGS>
  static decltype(auto)
  execute_function(FUNCTION_HANDLE & handle, ARGS &&... args) {
    return handle(
        context_t::instance().function(handle.get_key()),
        std::forward_as_tuple(args...));
  } // execute_function

}; // struct legion_execution_policy_t

} // namespace execution
} // namespace flecsi
