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
#include "../launch.hh"
#include "flecsi/runtime/backend.hh"
#include "flecsi/runtime/legion/tasks.hh"
#include "flecsi/utils/demangle.hh"
#include "flecsi/utils/function_traits.hh"
#include <flecsi/execution/legion/enactment/task_wrapper.hh>
#include <flecsi/execution/legion/future.hh>
#include <flecsi/execution/legion/invocation/init_args.hh>
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

namespace detail {
// Remove const from under a reference, if there is one.
template<class T>
struct nonconst_ref {
  using type = T;
};
template<class T>
struct nonconst_ref<const T &> {
  using type = T &;
};
template<class T>
using nonconst_ref_t = typename nonconst_ref<T>::type;

// Serialize a tuple of converted arguments (or references to existing
// arguments where possible).  Note that is_constructible_v<const
// float&,const double&> is true, so we have to check
// is_constructible_v<float&,double&> instead.
template<class... PP, class... AA>
auto
serial_arguments(std::tuple<PP...> * /* to deduce PP */, AA &&... aa) {
  static_assert((std::is_const_v<std::remove_reference_t<const PP>> && ...),
    "Tasks cannot accept non-const references");
  return utils::serial_put(std::tuple<std::conditional_t<
      std::is_constructible_v<nonconst_ref_t<PP> &, nonconst_ref_t<AA>>,
      const PP &,
      std::decay_t<PP>>...>(std::forward<AA>(aa)...));
}
} // namespace detail

template<auto & F,
  size_t LAUNCH_DOMAIN,
  size_t REDUCTION,
  size_t ATTRIBUTES,
  typename... ARGS>
decltype(auto)
reduce(ARGS &&... args) {
  using namespace Legion;
  using namespace execution;

  using traits_t = utils::function_traits<decltype(F)>;
  using RETURN = typename traits_t::return_type;
  using ARG_TUPLE = typename traits_t::arguments_type;

  // This will guard the entire method
  flog_tag_guard(execution);

  // Get the FleCSI runtime context
  auto & flecsi_context = runtime::context_t::instance();

  // Get the processor type.
  constexpr auto processor_type = mask_to_processor_type(ATTRIBUTES);

  // Get the Legion runtime and context from the current task.
  auto legion_runtime = Legion::Runtime::get_runtime();
  auto legion_context = Legion::Runtime::get_context();

#if defined(FLECSI_ENABLE_FLOG)
  const size_t tasks_executed = flecsi_context.tasks_executed();
  if((tasks_executed > 0) &&
     (tasks_executed % FLOG_SERIALIZATION_INTERVAL == 0)) {

    size_t processes = flecsi_context.processes();
    LegionRuntime::Arrays::Rect<1> launch_bounds(
      LegionRuntime::Arrays::Point<1>(0),
      LegionRuntime::Arrays::Point<1>(processes - 1));
    Domain launch_domain = Domain::from_rect<1>(launch_bounds);

    Legion::ArgumentMap arg_map;
    Legion::IndexLauncher reduction_launcher(
      legion::task_id<runtime::flog_reduction_task>,
      launch_domain,
      Legion::TaskArgument(NULL, 0),
      arg_map);

    constexpr size_t max_hash =
      utils::hash::reduction_hash<flecsi_internal_hash(max),
        flecsi_internal_hash(size_t)>();
    size_t reduction_id = flecsi_context.reduction_operations()[max_hash];

    Legion::Future future = legion_runtime->execute_index_space(
      legion_context, reduction_launcher, reduction_id);

    if(future.get_result<size_t>() > FLOG_SERIALIZATION_THRESHOLD) {
      Legion::IndexLauncher flog_mpi_launcher(
        legion::task_id<runtime::flog_mpi_task>,
        launch_domain,
        Legion::TaskArgument(NULL, 0),
        arg_map);

      flog_mpi_launcher.tag = runtime::FLECSI_MAPPER_FORCE_RANK_MATCH;

      // Launch the MPI task
      auto future_mpi =
        legion_runtime->execute_index_space(legion_context, flog_mpi_launcher);

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

  constexpr size_t ZERO = flecsi_internal_hash(0);

  size_t domain_size = flecsi_context.get_launch_domain_size(LAUNCH_DOMAIN);
  domain_size = domain_size == 0 ? flecsi_context.processes() *
                                     flecsi_context.threads_per_process()
                                 : domain_size;

  ++flecsi_context.tasks_executed();

  legion::init_args_t init_args(legion_runtime, legion_context, domain_size);
  init_args.walk<ARG_TUPLE>(args...);
  auto buf = detail::serial_arguments(
    static_cast<ARG_TUPLE *>(nullptr), std::forward<ARGS>(args)...);

  //------------------------------------------------------------------------//
  // Single launch
  //------------------------------------------------------------------------//

  using wrap = legion::task_wrapper<F, processor_type>;
  const auto task = legion::task_id<wrap::execute,
    (ATTRIBUTES & ~mpi) | 1 << static_cast<std::size_t>(wrap::LegionProcessor)>;

  if constexpr(LAUNCH_DOMAIN == launch_identifier("single")) {

    static_assert(
      REDUCTION == ZERO, "reductions are not supported for single tasks");

    {
      flog_tag_guard(execution);
      flog_devel(info) << "Executing single task" << std::endl;
    }

    TaskLauncher launcher(task, TaskArgument(buf.data(), buf.size()));

    for(auto & req : init_args.region_requirements()) {
      launcher.add_region_requirement(req);
    } // for

    LegionRuntime::Arrays::Rect<1> launch_bounds(0, 1);
    Domain launch_domain = Domain::from_rect<1>(launch_bounds);

    if constexpr(processor_type == task_processor_type_t::toc ||
                 processor_type == task_processor_type_t::loc) {
      auto future = legion_runtime->execute_task(legion_context, launcher);

      return legion_future<RETURN, launch_type_t::single>(future);
    }
    else {
      static_assert(
        processor_type == task_processor_type_t::mpi, "Unknown launch type");
      flog_fatal("Invalid launch type!"
                 << std::endl
                 << "Legion backend does not support 'single' launch"
                 << " for MPI tasks yet");
    }
  }

  //------------------------------------------------------------------------//
  // Index launch
  //------------------------------------------------------------------------//

  else {

    {
      flog_tag_guard(execution);
      flog_devel(info) << "Executing index task" << std::endl;
    }

    LegionRuntime::Arrays::Rect<1> launch_bounds(
      LegionRuntime::Arrays::Point<1>(0),
      LegionRuntime::Arrays::Point<1>(domain_size - 1));
    Domain launch_domain = Domain::from_rect<1>(launch_bounds);

    Legion::ArgumentMap arg_map;
    Legion::IndexLauncher launcher(
      task, launch_domain, TaskArgument(buf.data(), buf.size()), arg_map);

    for(auto & req : init_args.region_requirements()) {
      launcher.add_region_requirement(req);
    } // for

    if constexpr(processor_type == task_processor_type_t::toc ||
                 processor_type == task_processor_type_t::loc) {
      flog_devel(info) << "Executing index launch on loc" << std::endl;

      if constexpr(REDUCTION != ZERO) {
        flog_devel(info) << "executing reduction logic for " << REDUCTION
                         << std::endl;
        auto reduction_op =
          flecsi_context.reduction_operations().find(REDUCTION);

        flog_assert(reduction_op != flecsi_context.reduction_operations().end(),
          "invalid reduction operation");

        Legion::Future future;

        size_t reduction_id = flecsi_context.reduction_operations()[REDUCTION];
        future = legion_runtime->execute_index_space(
          legion_context, launcher, reduction_id);

        // FIXME
        // return legion_future<RETURN, launch_type_t::single>(future);
        return 0;
      }
      else {
        // Enqueue the task.
        Legion::FutureMap future_map =
          legion_runtime->execute_index_space(legion_context, launcher);

        // FIXME
        // return legion_future<RETURN, launch_type_t::index>(future_map);
        return 0;
      } // else
    }
    else {
      static_assert(
        processor_type == task_processor_type_t::mpi, "Unknown launch type");
      launcher.tag = runtime::FLECSI_MAPPER_FORCE_RANK_MATCH;

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

      if constexpr(REDUCTION != ZERO) {
        // FIXME implement logic for reduction MPI task
        flog_fatal("there is no implementation for the mpi"
                   " reduction task");
      }
      else {
        // FIXME
        // return legion_future<RETURN, launch_type_t::index>(future);
        return 0;
      }
    }
  } // if constexpr

  // return 0;
} // execute_task

namespace execution {

//------------------------------------------------------------------------//
// Reduction interface.
//------------------------------------------------------------------------//

template<size_t HASH, typename TYPE>
using reduction_wrapper = legion::reduction_wrapper<HASH, TYPE>;

} // namespace execution
} // namespace flecsi
