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

#include <flecsi/utils/const_string.h>

#include <flecsi/execution/common/processor.h>
#include <flecsi/execution/context.h>
#include <flecsi/execution/legion/context_policy.h>
#include <flecsi/execution/legion/future.h>
#include <flecsi/execution/legion/init_args.h>
#include <flecsi/execution/legion/reduction_wrapper.h>
#include <flecsi/execution/legion/runtime_state.h>
#include <flecsi/execution/legion/task_epilog.h>
#include <flecsi/execution/legion/task_prolog.h>
#include <flecsi/execution/legion/task_wrapper.h>

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
    The future_u type may be used for explicit synchronization of tasks.

    @tparam RETURN The return type of the task.
   */

  template<typename RETURN, launch_type_t launch>
  using future_u = legion_future_u<RETURN, launch>;

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

    @tparam TASK     A hash key identifying the task.
    @tparam RETURN   The return type of the pure Legion task.
    @tparam DELEGATE The function pointer template type of the task.

    @param name The string name for the task. This can be set to any
                valid std::string value.
   */

  template<size_t TASK,
    typename RETURN,
    RETURN (*DELEGATE)(const Legion::Task *,
      const std::vector<Legion::PhysicalRegion> &,
      Legion::Context,
      Legion::Runtime *)>
  static bool register_legion_task(processor_type_t processor,
    launch_t launch,
    std::string name) {
    clog(info) << "Registering legion task " << TASK << " " << name
               << std::endl;

    using wrapper_t = pure_task_wrapper_u<RETURN, DELEGATE>;

    const bool success = context_t::instance().register_task(
      TASK, processor, launch, name, wrapper_t::registration_callback);

    clog_assert(success, "callback registration failed for " << name);

    return true;
  } // register_legion_task

  /*!
    Legion backend task registration. For documentation on this
    method, please see task_u::register_task.
   */

  template<size_t TASK,
    typename RETURN,
    typename ARG_TUPLE,
    RETURN (*DELEGATE)(ARG_TUPLE)>
  static bool
  register_task(processor_type_t processor, launch_t launch, std::string name) {

    using wrapper_t = task_wrapper_u<TASK, RETURN, ARG_TUPLE, DELEGATE>;

    const bool success = context_t::instance().register_task(
      TASK, processor, launch, name, wrapper_t::registration_callback);

    clog_assert(success, "callback registration failed for " << name);

    return true;
  } // register_task

  /*!
    Legion backend task execution. For documentation on this
    method, please see task_u::execute_task.
   */

  template<launch_type_t LAUNCH,
    size_t TASK,
    size_t REDUCTION,
    typename RETURN,
    typename ARG_TUPLE,
    typename... ARGS>
  static decltype(auto) execute_task(ARGS &&... args) {

    using namespace Legion;

    // This will guard the entire method
    clog_tag_guard(execution);

    // Make a tuple from the arugments passed by the user
    ARG_TUPLE task_args = std::make_tuple(args...);

    // Get the FleCSI runtime context
    context_t & context_ = context_t::instance();

    // Get the processor type.
    auto processor_type = context_.processor_type<TASK>();

    // Get the Legion runtime and context from the current task.
    auto legion_runtime = Legion::Runtime::get_runtime();
    auto legion_context = Legion::Runtime::get_context();

    constexpr size_t ZERO =
            flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(0)}.hash();

    // Execute a tuple walker that applies the task epilog operations
    // on the mapped handles
//    task_epilog_t task_epilog(legion_runtime, legion_context);

//IRINA FIXME why epilog is here?
//    task_epilog.walk(task_args);

    //------------------------------------------------------------------------//
    // Single launch
    //------------------------------------------------------------------------//

    if constexpr(LAUNCH == launch_type_t::single) {

      switch(processor_type) {

        case processor_type_t::loc: {
          clog(info) << "Executing single task: " << TASK << std::endl;

          // Execute a tuple walker that initializes the handle arguments
          // that are passed to the task
          init_args_t init_args(legion_runtime, legion_context);
          init_args.walk(task_args);

          // Create a task launcher, passing the task arguments.
          TaskLauncher launcher(context_.task_id<TASK>(),
            TaskArgument(&task_args, sizeof(ARG_TUPLE)));

#ifdef MAPPER_COMPACTION
          launcher.tag = MAPPER_COMPACTED_STORAGE;
#endif

          // Add region requirements and future dependencies to the
          // task launcher
          for(auto & req : init_args.region_reqs) {
            launcher.add_region_requirement(req);
          } // for

          for(auto & future : init_args.futures) {
            launcher.add_future(future);
          } // for

          LegionRuntime::Arrays::Rect<1> launch_bounds(0,1);
          Domain launch_domain = Domain::from_rect<1>(launch_bounds);

          // Execute a tuple walker that applies the task prolog operations
          // on the mapped handles
          {
          task_prolog_t task_prolog(legion_runtime, legion_context,
						launch_domain);
          task_prolog.sparse = false;
          task_prolog.walk(task_args);
          task_prolog.launch_copies();
          } // scope

          {
          task_prolog_t task_prolog(legion_runtime, legion_context,
						launch_domain);
          task_prolog.sparse = true;
          task_prolog.walk(task_args);
          task_prolog.launch_copies();
          } // scope

          // Enqueue the task.
          clog(trace) << "Execute flecsi/legion task " << TASK << " on rank "
                      << legion_runtime->find_local_MPI_rank() << std::endl;
          auto future = legion_runtime->execute_task(legion_context, launcher);

          // Execute a tuple walker that applies the task epilog operations
          // on the mapped handles
          task_epilog_t task_epilog(legion_runtime, legion_context);
          task_epilog.walk(task_args);

          static_assert(REDUCTION == ZERO,
            "reductions are not supported for single tasks");

          return legion_future_u<RETURN, launch_type_t::single>(future);
        } // scope

        case processor_type_t::toc:
          clog_fatal("Invalid processor type (toc is un-implemented)");

        case processor_type_t::mpi:
          clog_fatal("Invalid launch type!"
                     << std::endl
                     << "Legion backend does not support 'single' launch"
                     << " for MPI tasks yet");

        default:
          clog_fatal("Unknown processor type: " << processor_type);
      } // switch
    }

    //------------------------------------------------------------------------//
    // Index launch
    //------------------------------------------------------------------------//

    else {

      switch(processor_type) {

        case processor_type_t::loc: {
          clog(info) << "Executing index task: " << TASK << std::endl;

          // Execute a tuple walker that initializes the handle arguments
          // that are passed to the task
          init_args_t init_args(legion_runtime, legion_context);
          init_args.walk(task_args);

          LegionRuntime::Arrays::Rect<1> launch_bounds(
            LegionRuntime::Arrays::Point<1>(0),
            LegionRuntime::Arrays::Point<1>(context_.colors()-1));
          Domain launch_domain = Domain::from_rect<1>(launch_bounds);

          Legion::ArgumentMap arg_map;
          Legion::IndexLauncher launcher(context_.task_id<TASK>(),
            launch_domain, TaskArgument(&task_args, sizeof(ARG_TUPLE)),
            arg_map);

#ifdef MAPPER_COMPACTION
          launcher.tag = MAPPER_COMPACTED_STORAGE;
#endif
          launcher.tag = MAPPER_FORCE_RANK_MATCH;

          // Add region requirements and future dependencies to the
          // task launcher
          for(auto & req : init_args.region_reqs) {
            launcher.add_region_requirement(req);
          } // for

          for(auto & future : init_args.futures) {
           launcher.add_future(future);
          } // for

          // Execute a tuple walker that applies the task prolog operations
          // on the mapped handles
          {
          task_prolog_t task_prolog(legion_runtime, legion_context,
            launch_domain);
          task_prolog.sparse = false;
          task_prolog.walk(task_args);
          task_prolog.launch_copies();
          } // scope

          {
          task_prolog_t task_prolog(legion_runtime, legion_context,
            launch_domain);
          task_prolog.sparse = true;
          task_prolog.walk(task_args);
          task_prolog.launch_copies();
          } // scope

          if constexpr(REDUCTION != ZERO) {
            clog(info) << "executing reduction logic for " <<
              REDUCTION << std::endl;
            auto reduction_op =
              context_.reduction_operations().find(REDUCTION);

            clog_assert(reduction_op != context_.reduction_operations().end(),
              "invalid reduction operation");

            Legion::Future future;

            size_t reduction_id = context_.reduction_operations()[REDUCTION];
            future = legion_runtime->execute_index_space(legion_context,
              launcher, reduction_id);

            // Enqueue the epilog.
            task_epilog_t task_epilog(legion_runtime, legion_context);
            task_epilog.walk(task_args);

            return legion_future_u<RETURN, launch_type_t::single>(future);
          }
          else {
            // Enqueue the task.
            Legion::FutureMap future_map =
              legion_runtime->execute_index_space(legion_context, launcher);

            // Execute a tuple walker that applies the task epilog operations
            // on the mapped handles
            task_epilog_t task_epilog(legion_runtime, legion_context);
            task_epilog.walk(task_args);
           
            return legion_future_u<RETURN, launch_type_t::index>(future_map);
          }//else
        } // scope

        case processor_type_t::mpi: {
          clog(info) << "Executing MPI task: " << TASK << std::endl;

          // Execute a tuple walker that initializes the handle arguments
          // that are passed to the task
          init_args_t init_args(legion_runtime, legion_context);
          init_args.walk(task_args);

          // FIXME: This will need to change with the new control model
 //         if(context_.execution_state() == SPECIALIZATION_TLT_INIT) {

            LegionRuntime::Arrays::Rect<1> launch_bounds(
              LegionRuntime::Arrays::Point<1>(0),
              LegionRuntime::Arrays::Point<1>(context_.colors()-1));
            Domain launch_domain = Domain::from_rect<1>(launch_bounds);



            ArgumentMap arg_map;
            IndexLauncher launcher(context_.task_id<TASK>(),
              Legion::Domain::from_rect<1>(context_.all_processes()),
              TaskArgument(&task_args, sizeof(ARG_TUPLE)), arg_map);

            launcher.tag = MAPPER_FORCE_RANK_MATCH;

            // Add region requirements and future dependencies to the
            // task launcher
            for(auto & req : init_args.region_reqs) {
              launcher.add_region_requirement(req);
            } // for

            for(auto & future : init_args.futures) {
              launcher.add_future(future);
            } // for

            // Execute a tuple walker that applies the task prolog operations
            // on the mapped handles
            {
            task_prolog_t task_prolog(legion_runtime, legion_context,
              launch_domain);
            task_prolog.sparse = false;
            task_prolog.walk(task_args);
            task_prolog.launch_copies();
            } // scope

            {
            task_prolog_t task_prolog(legion_runtime, legion_context,
              launch_domain);
            task_prolog.sparse = true;
            task_prolog.walk(task_args);
            task_prolog.launch_copies();
            } // scope


            // Launch the MPI task
            auto future = legion_runtime->execute_index_space(
          	  legion_context,launcher);
            // Force synchronization
            future.wait_all_results(true);

            // Handoff to the MPI runtime.
            context_.handoff_to_mpi(legion_context, legion_runtime);

            // Wait for MPI to finish execution (synchronous).
            context_.wait_on_mpi(legion_context, legion_runtime);

            // Reset the calling state to false.
            context_.unset_call_mpi(legion_context, legion_runtime);

            // Execute a tuple walker that applies the task epilog operations
            // on the mapped handles
            task_epilog_t task_epilog(legion_runtime, legion_context);
            task_epilog.walk(task_args);

            if constexpr(REDUCTION != ZERO) {
              clog_fatal("there is no implementation for the mpi"
							 " reduction task");
            } else {
              return legion_future_u<RETURN, launch_type_t::index>(future);
            }
#if 0
          }
          else {

            // Create a task launcher, passing the task arguments.
            TaskLauncher launcher(context_.task_id<TASK>(),
              TaskArgument(&task_args, sizeof(ARG_TUPLE)));

            // Add region requirements and future dependencies to the
            // task launcher
            for(auto & req : init_args.region_reqs) {
              launcher.add_region_requirement(req);
            } // for

            for(auto & future : init_args.futures) {
              future->add_to_single_task_launcher(launcher);
            } // for

            launcher.tag = MAPPER_SUBRANK_LAUNCH;

            // Execute a tuple walker that applies the task prolog
            // operations on the mapped handles
            {
            task_prolog_t task_prolog(legion_runtime, legion_context, launcher);
            task_prolog.sparse = false;
            task_prolog.walk(task_args);
            task_prolog.launch_copies();
            } // scope

            {
            task_prolog_t task_prolog(legion_runtime, legion_context, launcher);
            task_prolog.sparse = true;
            task_prolog.walk(task_args);
            task_prolog.launch_copies();
            } // scope

            // Launch the MPI task
            auto f = legion_runtime->execute_task(legion_context, launcher);

            // Execute a tuple walker that applies the task epilog operations
            // on the mapped handles
            task_epilog_t task_epilog(legion_runtime, legion_context);
            task_epilog.walk(task_args);

            // Wait on the Legion future to complete
            f.wait();

            // Handoff to the MPI runtime.
            context_.handoff_to_mpi();

            // Wait for MPI to finish execution (synchronous).
            context_.wait_on_mpi();

            // Get a future to the task that swaps the runtime states
            auto future =
              context_.unset_call_mpi_index(legion_context, legion_runtime);

            return legion_future_u<RETURN, launch_type_t::index>(future);
          } // if
#endif
        } // scope

        default:
          clog_fatal("Unknown processor type: " << processor_type);
      } // switch

    } // if constexpr
  } // execute_task

  //------------------------------------------------------------------------//
  // Function interface.
  //------------------------------------------------------------------------//

  /*!
    Legion backend function registration. For documentation on this
    method, please see function_u::register_function.
   */

  template<size_t FUNCTION,
    typename RETURN,
    typename ARG_TUPLE,
    RETURN (*DELEGATE)(ARG_TUPLE)>
  static bool register_function() {
    return context_t::instance()
      .template register_function<FUNCTION, RETURN, ARG_TUPLE, DELEGATE>();
  } // register_function

  /*!
    Legion backend function execution. For documentation on this
    method, please see function_u::execute_function.
   */

  template<typename FUNCTION_HANDLE, typename... ARGS>
  static decltype(auto) execute_function(FUNCTION_HANDLE & handle,
    ARGS &&... args) {
    return handle(context_t::instance().function(handle.get_key()),
      std::forward_as_tuple(args...));
  } // execute_function

  //------------------------------------------------------------------------//
  // Reduction interface.
  //------------------------------------------------------------------------//

  /*!
   Legion backend reduction registration. For documentation on this
   method please see task_u::register_reduction_operation.
   */

  template<size_t HASH, typename TYPE>
  static bool register_reduction_operation() {

    using wrapper_t = reduction_wrapper_u<HASH, TYPE>;

    return context_t::instance().register_reduction_operation(
      HASH, wrapper_t::registration_callback);
  } // register_reduction_operation

}; // struct legion_execution_policy_t

} // namespace execution
} // namespace flecsi
