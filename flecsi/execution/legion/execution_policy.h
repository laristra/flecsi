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

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly!
#else
#include <flecsi/execution/context.h>
#include <flecsi/execution/legion/execution/task_wrapper.h>
#include <flecsi/execution/legion/invocation/init_args.h>
#include <flecsi/execution/legion/invocation/task_epilogue.h>
#include <flecsi/execution/legion/invocation/task_prologue.h>
#include <flecsi/execution/legion/reduction_wrapper.h>
#include <flecsi/utils/const_string.h>
#include <flecsi/utils/flog.h>
#endif

#include <functional>
#include <memory>
#include <type_traits>

#include <flecsi-config.h>

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
    launch_t launch,
    std::string name) {
    flog(internal) << "Registering legion task" << std::endl
                   << "\tname: " << name << std::endl
                   << "\thash: " << TASK << std::endl;

    using wrapper_t = legion::pure_task_wrapper_u<RETURN, DELEGATE>;

    const bool success = context_t::instance().register_task(
      TASK, processor, launch, name, wrapper_t::registration_callback);

    flog_assert(success, "callback registration failed for " << name);

    return true;
  } // register_legion_task

  /*
    Documentation for this interface is in the top-level context type.
   */

  template<size_t TASK,
    typename RETURN,
    typename ARG_TUPLE,
    RETURN (*DELEGATE)(ARG_TUPLE)>
  static bool
  register_task(processor_type_t processor, launch_t launch, std::string name) {

    using wrapper_t = legion::task_wrapper_u<TASK, RETURN, ARG_TUPLE, DELEGATE>;

    const bool success = context_t::instance().register_task(
      TASK, processor, launch, name, wrapper_t::registration_callback);

    flog_assert(success, "callback registration failed for " << name);

    return true;
  } // register_task

  /*
    Documentation for this interface is in the top-level context type.
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
    flog_tag_guard(execution);

    // Make a tuple from the arugments passed by the user
    ARG_TUPLE task_args = std::forward_as_tuple(args...);

    // Get the FleCSI runtime context
    context_t & context_ = context_t::instance();

    // Get the processor type.
    auto processor_type = context_.processor_type<TASK>();

    // Get the Legion runtime and context from the current task.
    auto legion_runtime = Legion::Runtime::get_runtime();
    auto legion_context = Legion::Runtime::get_context();

    constexpr size_t ZERO = flecsi_internal_hash(0);

    legion::init_args_t init_args(legion_runtime, legion_context, LAUNCH);
    init_args.walk(task_args);

    //------------------------------------------------------------------------//
    // Single launch
    //------------------------------------------------------------------------//

    if constexpr(LAUNCH == launch_type_t::single) {

      {
        flog_tag_guard(execution);
        flog(internal) << "Executing single task" << std::endl;
      }

      LegionRuntime::Arrays::Rect<1> launch_bounds(0, 1);
      Domain launch_domain = Domain::from_rect<1>(launch_bounds);

      legion::task_prologue_t prologue(
        legion_runtime, legion_context, launch_domain);
      prologue.walk(task_args);
      prologue.update_state();

      switch(processor_type) {

        case processor_type_t::loc: {
          return 0;
        } // case processor_type_t::loc

        case processor_type_t::toc: {
          flog_fatal("Invalid processor type (toc is un-implemented)");
        } // case processor_type_t::toc

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
        LegionRuntime::Arrays::Point<1>(context_.colors() - 1));
      Domain launch_domain = Domain::from_rect<1>(launch_bounds);

      Legion::ArgumentMap arg_map;
      Legion::IndexLauncher launcher(context_.task_id<TASK>(),
        launch_domain, TaskArgument(&task_args, sizeof(ARG_TUPLE)),
        arg_map);

      switch(processor_type) {

        case processor_type_t::loc: {
          flog(info) << "Executing index launch on loc" << std::endl;
          return 0;
        } // case processor_type_t::loc

        case processor_type_t::toc: {
          flog_fatal("Invalid processor type (toc is un-implemented)");
        } // case processor_type_t::toc

        case processor_type_t::mpi: {
          return 0;
        } // case processor_type_t::mpi

        default:
          flog_fatal("Unknown processor type: " << processor_type);

      } // switch
    } // if constexpr

    return 0;
  } // execute_task

  //------------------------------------------------------------------------//
  // Reduction interface.
  //------------------------------------------------------------------------//

  template<size_t HASH, typename TYPE>
  using reduction_wrapper_u = legion::reduction_wrapper_u<HASH, TYPE>;

}; // struct legion_execution_policy_t

} // namespace execution
} // namespace flecsi
