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

#include <flecsi-config.h>

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly!
#else
#include <flecsi/execution/context.hh>
#include <flecsi/execution/legion/enactment/task_wrapper.hh>
#include <flecsi/execution/legion/future.hh>
#include <flecsi/execution/legion/invocation/init_args.hh>
#include <flecsi/execution/legion/invocation/task_epilogue.hh>
#include <flecsi/execution/legion/invocation/task_prologue.hh>
#include <flecsi/execution/legion/reduction_wrapper.hh>
#include <flecsi/utils/const_string.hh>
#include <flecsi/utils/flog.hh>
#include <flecsi/utils/flog/utils.hh>
#endif

#include <functional>
#include <memory>
#include <type_traits>

#if !defined(FLECSI_ENABLE_LEGION)
#error FLECSI_ENABLE_LEGION not defined! This file depends on Legion!
#endif

#include <legion.h>

flog_register_tag(execution);

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
    task_execution_type_t execution,
    std::string name) {
    flog(internal) << "Registering legion task" << std::endl
                   << "\tname: " << name << std::endl
                   << "\thash: " << TASK << std::endl;

    using wrapper_t = legion::pure_task_wrapper_u<RETURN, DELEGATE>;

    const bool success = context_t::instance().register_task(
      TASK, processor, execution, name, wrapper_t::registration_callback);

    flog_assert(success, "callback registration failed for " << name);

    return true;
  } // register_legion_task

  /*
    Documentation for this interface is in the top-level context type.
   */

  template<typename RETURN, typename ARG_TUPLE, RETURN (*DELEGATE)(ARG_TUPLE)>
  static bool register_task(std::size_t TASK,
    processor_type_t processor,
    task_execution_type_t execution,
    std::string name) {

    using wrapper_t = legion::task_wrapper_u<RETURN, ARG_TUPLE, DELEGATE>;

    const bool success = context_t::instance().register_task(
      TASK, processor, execution, name, wrapper_t::registration_callback);

    flog_assert(success, "callback registration failed for " << name);

    return true;
  } // register_task

  /*
    Documentation for this interface is in the top-level context type.
   */

  template<size_t LAUNCH_DOMAIN,
    size_t REDUCTION,
    typename RETURN,
    typename ARG_TUPLE,
    typename... ARGS>
  static decltype(auto) execute_task(std::size_t TASK, ARGS &&... args) {

    using namespace Legion;

    // This will guard the entire method
    flog_tag_guard(execution);

    // Make a tuple from the arugments passed by the user
    ARG_TUPLE task_args = std::forward_as_tuple(args...);

    // Get the FleCSI runtime context
    context_t & flecsi_context = context_t::instance();

    // Get the processor type.
    auto processor_type = flecsi_context.processor_type(TASK);

    // Get the Legion runtime and context from the current task.
    auto legion_runtime = Legion::Runtime::get_runtime();
    auto legion_context = Legion::Runtime::get_context();

    constexpr size_t ZERO = flecsi_internal_hash(0);

    size_t domain_size = flecsi_context.get_domain(LAUNCH_DOMAIN);
    domain_size = domain_size == 0 ? flecsi_context.processes() *
                                       flecsi_context.threads_per_process()
                                   : domain_size;

#if defined(FLOG_ENABLE_MPI)
    const size_t tasks_executed = flecsi_context.tasks_executed();
    if((tasks_executed > 0) &&
       (tasks_executed % FLOG_SERIALIZATION_INTERVAL == 0)) {

      size_t processes = flecsi_context.processes();
      LegionRuntime::Arrays::Rect<1> launch_bounds(
        LegionRuntime::Arrays::Point<1>(0),
        LegionRuntime::Arrays::Point<1>(processes - 1));
      Domain launch_domain = Domain::from_rect<1>(launch_bounds);

      const auto reduction_tid =
        context_t::instance()
          .task_id<flecsi_internal_hash(flog_reduction_task)>();

      Legion::ArgumentMap arg_map;
      Legion::IndexLauncher reduction_launcher(
        reduction_tid, launch_domain, Legion::TaskArgument(NULL, 0), arg_map);

      constexpr size_t max_hash =
        utils::hash::reduction_hash<flecsi_internal_hash(max),
          flecsi_internal_hash(size_t)>();
      size_t reduction_id = flecsi_context.reduction_operations()[max_hash];

      Legion::Future future = legion_runtime->execute_index_space(
        legion_context, reduction_launcher, reduction_id);

      std::cerr << "max: " << future.get_result<size_t>() << std::endl;
      if(future.get_result<size_t>() > FLOG_SERIALIZATION_SIZE) {
        const auto flog_mpi_tid =
          context_t::instance().task_id<flecsi_internal_hash(flog_mpi_task)>();

        Legion::IndexLauncher flog_mpi_launcher(
          flog_mpi_tid, launch_domain, Legion::TaskArgument(NULL, 0), arg_map);

        flog_mpi_launcher.tag = FLECSI_MAPPER_FORCE_RANK_MATCH;

        // Launch the MPI task
        auto future_mpi = legion_runtime->execute_index_space(
          legion_context, flog_mpi_launcher);

        // Force synchronization
        future_mpi.wait_all_results(true);

        // Handoff to the MPI runtime.
        flecsi_context.handoff_to_mpi(legion_context, legion_runtime);

        // Wait for MPI to finish execution (synchronous).
        flecsi_context.wait_on_mpi(legion_context, legion_runtime);

        // Reset the calling state to false.
        flecsi_context.unset_call_mpi(legion_context, legion_runtime);
      } // if
    } // if
#endif // FLECSI_ENABLE_FLOG

    context_t::instance().tasks_executed()++;

    legion::init_args_t init_args(legion_runtime, legion_context, domain_size);
    init_args.walk(task_args);

    //------------------------------------------------------------------------//
    // Single launch
    //------------------------------------------------------------------------//

    if constexpr(flecsi_internal_hash(single) == LAUNCH_DOMAIN) {

      static_assert(
        REDUCTION == ZERO, "reductions are not supported for single tasks");

      {
        flog_tag_guard(execution);
        flog(internal) << "Executing single task" << std::endl;
      }

      TaskLauncher launcher(flecsi_context.task_id(TASK),
        TaskArgument(&task_args, sizeof(ARG_TUPLE)));

      for(auto & req : init_args.region_requirements()) {
        launcher.add_region_requirement(req);
      } // for

      LegionRuntime::Arrays::Rect<1> launch_bounds(0, 1);
      Domain launch_domain = Domain::from_rect<1>(launch_bounds);

      legion::task_prologue_t prologue(
        legion_runtime, legion_context, launch_domain);
      prologue.walk(task_args);
      prologue.update_state();

      switch(processor_type) {

        case processor_type_t::toc:
        case processor_type_t::loc: {
          auto future = legion_runtime->execute_task(legion_context, launcher);

          return legion_future_u<RETURN, launch_type_t::single>(future);
        } // case processor_type_t::loc

        case processor_type_t::mpi: {
          flog_fatal("Invalid launch type!"
                     << std::endl
                     << "Legion backend does not support 'single' launch"
                     << " for MPI tasks yet");
        } // case processor_type_t::mpi

        default:
          flog_fatal("Unknown processor type: " << processor_type);
      } // switch

      legion::task_epilogue_t epilogue(legion_runtime, legion_context);
      epilogue.walk(task_args);
    }

    //------------------------------------------------------------------------//
    // Index launch
    //------------------------------------------------------------------------//

    else {

      {
        flog_tag_guard(execution);
        flog(internal) << "Executing index task" << std::endl;
      }

      LegionRuntime::Arrays::Rect<1> launch_bounds(
        LegionRuntime::Arrays::Point<1>(0),
        LegionRuntime::Arrays::Point<1>(domain_size - 1));
      Domain launch_domain = Domain::from_rect<1>(launch_bounds);

      Legion::ArgumentMap arg_map;
      Legion::IndexLauncher launcher(flecsi_context.task_id(TASK),
        launch_domain,
        TaskArgument(&task_args, sizeof(ARG_TUPLE)),
        arg_map);

      for(auto & req : init_args.region_requirements()) {
        launcher.add_region_requirement(req);
      } // for

      legion::task_prologue_t prologue(
        legion_runtime, legion_context, launch_domain);
      prologue.walk(task_args);
      prologue.update_state();

      switch(processor_type) {

        case processor_type_t::toc:
        case processor_type_t::loc: {
          flog(info) << "Executing index launch on loc" << std::endl;

          if constexpr(REDUCTION != ZERO) {
            flog(info) << "executing reduction logic for " << REDUCTION
                       << std::endl;
            auto reduction_op =
              flecsi_context.reduction_operations().find(REDUCTION);

            flog_assert(
              reduction_op != flecsi_context.reduction_operations().end(),
              "invalid reduction operation");

            Legion::Future future;

            size_t reduction_id =
              flecsi_context.reduction_operations()[REDUCTION];
            future = legion_runtime->execute_index_space(
              legion_context, launcher, reduction_id);

            // Enqueue the epilog.
            legion::task_epilogue_t task_epilogue(
              legion_runtime, legion_context);
            task_epilogue.walk(task_args);
            return 0;

            // FIXME
            // return legion_future_u<RETURN, launch_type_t::single>(future);
            return 0;
          }
          else {
            // Enqueue the task.
            Legion::FutureMap future_map =
              legion_runtime->execute_index_space(legion_context, launcher);

            // Execute a tuple walker that applies the task epilog operations
            // on the mapped handles
            legion::task_epilogue_t task_epilogue(
              legion_runtime, legion_context);
            task_epilogue.walk(task_args);

            // FIXME
            // return legion_future_u<RETURN, launch_type_t::index>(future_map);
            return 0;
          } // else

        } // case processor_type_t::loc

        case processor_type_t::mpi: {
          launcher.tag = FLECSI_MAPPER_FORCE_RANK_MATCH;

          // Launch the MPI task
          auto future =
            legion_runtime->execute_index_space(legion_context, launcher);
          // Force synchronization
          future.wait_all_results(true);

          // Handoff to the MPI runtime.
          flecsi_context.handoff_to_mpi(legion_context, legion_runtime);

          // Wait for MPI to finish execution (synchronous).
          flecsi_context.wait_on_mpi(legion_context, legion_runtime);

          // Reset the calling state to false.
          flecsi_context.unset_call_mpi(legion_context, legion_runtime);

          // Execute a tuple walker that applies the task epilog operations
          // on the mapped handles
          legion::task_epilogue_t task_epilogue(legion_runtime, legion_context);
          task_epilogue.walk(task_args);

          if constexpr(REDUCTION != ZERO) {
            // FIXME implement logic for reduction MPI task
            flog_fatal("there is no implementation for the mpi"
                       " reduction task");
          }
          else {
            // FIXME
            // return legion_future_u<RETURN, launch_type_t::index>(future);
            return 0;
          }

        } // case processor_type_t::mpi

        default:
          flog_fatal("Unknown processor type: " << processor_type);

      } // switch
    } // if constexpr

  } // execute_task

  //------------------------------------------------------------------------//
  // Reduction interface.
  //------------------------------------------------------------------------//

  template<size_t HASH, typename TYPE>
  using reduction_wrapper_u = legion::reduction_wrapper_u<HASH, TYPE>;

}; // struct legion_execution_policy_t

} // namespace execution
} // namespace flecsi
