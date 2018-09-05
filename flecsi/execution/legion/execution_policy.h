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

  template <typename RETURN, launch_type_t launch>
  using future__ = legion_future__<RETURN, launch>;

  /*!
    The runtime_state_t type identifies a public type for the high-level
    runtime interface to pass state required by the backend.
   */

  using runtime_state_t = legion_runtime_state_t;

  /*!
    Return the runtime state of the calling FleCSI task.

    @param task The calling task.
   */

  static runtime_state_t &runtime_state(void *task);

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

  template <size_t KEY, typename RETURN,
            RETURN (*TASK)(const Legion::Task *,
                           const std::vector<Legion::PhysicalRegion> &,
                           Legion::Context, Legion::Runtime *)>
  static bool register_legion_task(processor_type_t processor, launch_t launch,
                                   std::string name) {
    clog(info) << "Registering legion task " << KEY << " " << name << std::endl;

    using wrapper_t = pure_task_wrapper__<RETURN, TASK>;

    const bool success = context_t::instance().register_task(
      KEY, processor, launch, name, wrapper_t::registration_callback);

    clog_assert(success, "callback registration failed for " << name);

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

    const bool success = context_t::instance().register_task(
      KEY, processor, launch, name, wrapper_t::registration_callback);

    clog_assert(success, "callback registration failed for " << name);

    return true;
  } // register_task

  /*!
   We use "execute_task__" class in purose to be able to use
   partial template specializations for the execute method
   */

  template <launch_type_t LAUNCH, Legion::ReductionOpID REDUCTION_ID,
     size_t KEY, typename RETURN, typename ARG_TUPLE, typename... ARGS>
  struct execute_task__ {
    static void execute(ARGS &&... args) {
      clog_fatal("invalid launch type" << std::endl);
    }
  };

  /*!
    Partial template specialization for the reduction task (Index launch, 
     REDUCTIONID!=0)
   */
  template <size_t KEY, Legion::ReductionOpID REDUCTION_ID, typename RETURN,	    typename ARG_TUPLE, typename... ARGS>
  struct execute_task__<launch_type_t::index, REDUCTION_ID,  KEY, RETURN,
	ARG_TUPLE, ARGS...> {
    static decltype(auto) execute(ARG_TUPLE task_args) {
      using namespace Legion;
      context_t &context_ = context_t::instance();

      // Get the processor type.
      auto processor_type = context_.processor_type<KEY>();

      // Get the runtime and context from the current task.
      auto legion_runtime = Legion::Runtime::get_runtime();
      auto legion_context = Legion::Runtime::get_context();

      init_args_t init_args(legion_runtime, legion_context);
      init_args.walk(task_args);

      // Handle MPI and Legion invocations separately.
      if (processor_type == processor_type_t::mpi) {
        //FIXME implement logic for the reduction mpi task
        clog(fatal) << "mpi_reduction task was not implemented yet" <<
		std::endl; 
      } else{
        LegionRuntime::Arrays::Rect<1> launch_bounds(0,context_.colors()-1);
        Domain launch_domain = Domain::from_rect<1>(launch_bounds);
        // Create a task launcher, passing the task arguments.
          IndexTaskLauncher index_task_launcher(
              context_.task_id<KEY>(), launch_domain,
              TaskArgument(&task_args, sizeof(ARG_TUPLE)), Legion::ArgumentMap());

          index_task_launcher.tag = MAPPER_FORCE_RANK_MATCH;

#ifdef MAPPER_COMPACTION
          index_task_launcher.tag = MAPPER_COMPACTED_STORAGE;
#endif

          for (auto & req : init_args.region_reqs) {
            index_task_launcher.add_region_requirement(req);
          }
          for (auto &future : init_args.futures) {
           index_task_launcher.add_future(future);
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

          future = legion_runtime->execute_index_space(legion_context,
                index_task_launcher, REDUCTION_ID);

          // Enqueue the epilog.
          task_epilog_t task_epilog(legion_runtime, legion_context);
          task_epilog.walk(task_args);

          return legion_future__<RETURN, launch_type_t::single>(future);
      }
    }
  };//end execute_task__

   /*!
    Partial template specialization for the non-reduction task Index task 
     (REDUCTIONID==0)
   */
  template <size_t KEY, typename RETURN, typename ARG_TUPLE, typename... ARGS>
  struct execute_task__<launch_type_t::index, 0,  KEY, RETURN, ARG_TUPLE,
                              ARGS...> {
    static decltype(auto) execute(ARG_TUPLE task_args) {
      using namespace Legion;
      // Make a tuple from the task arguments.
      // ARG_TUPLE task_args = std::make_tuple(args...);

      context_t &context_ = context_t::instance();

      // Get the processor type.
      auto processor_type = context_.processor_type<KEY>();

      // Get the runtime and context from the current task.
      auto legion_runtime = Legion::Runtime::get_runtime();
      auto legion_context = Legion::Runtime::get_context();

      init_args_t init_args(legion_runtime, legion_context);
      init_args.walk(task_args);

      // Handle MPI and Legion invocations separately.
      if (processor_type == processor_type_t::mpi) {
        {
          clog_tag_guard(execution);
          clog(info) << "Executing MPI task: " << KEY << std::endl;
        }

//FIXME the check
//        if (!(legion_context->is_inner_context())) {

          ArgumentMap arg_map;
          IndexLauncher launcher(
              context_.task_id<KEY>(),
              Legion::Domain::from_rect<1>(context_.all_processes()),
              TaskArgument(&task_args, sizeof(ARG_TUPLE)), arg_map);

          for (auto &req : init_args.region_reqs) {
            launcher.add_region_requirement(req);
          }
          for (auto &future : init_args.futures) {
            launcher.add_future(future);
          }

        launcher.tag = MAPPER_FORCE_RANK_MATCH;

        task_prolog_t task_prolog(
            legion_runtime, legion_context,  launch_domain);
        task_prolog.sparse = false;
        task_prolog.walk(task_args);
        task_prolog.launch_copies();

        task_prolog_t task_prolog_sparse(
            legion_runtime, legion_context,  launch_domain);
        task_prolog_sparse.sparse = true;
        task_prolog_sparse.walk(task_args);
        task_prolog_sparse.launch_copies();

        auto future = legion_runtime->execute_index_space(
	  legion_context,launcher);
        future.wait_all_results(true);

          // Handoff to the MPI runtime.
          context_.handoff_to_mpi(legion_context, legion_runtime);

          // Wait for MPI to finish execution (synchronous).
          context_.wait_on_mpi(legion_context, legion_runtime);

          // Reset the calling state to false.
          context_.unset_call_mpi(legion_context, legion_runtime);

          // Enqueue the epilog.
          task_epilog_t task_epilog(legion_runtime, legion_context);
          task_epilog.walk(task_args);

          return legion_future__<RETURN, launch_type_t::index>(future);
//FIXME the check
#if 0
        } else { // check for execution_state
          clog(fatal) << " mpi index task can't be executed from  another task"
            << std::endl;
          throw std::runtime_error(" mpi index task can't be executed from  another task");
        }//check for execution state
#endif
      } else {

//FIXME
#if 0 
        if ( if (legion_context->is_inner_context())) {
          clog(fatal) << " loc index task can't be executed from  another task"
            << std::endl;
          throw std::runtime_error(" loc index task can't be executed from"
                "  another task");

        }//end if
#endif
        LegionRuntime::Arrays::Rect<1> launch_bounds(0,context_.colors()-1);
        Domain launch_domain = Domain::from_rect<1>(launch_bounds);
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
          for (auto &future : init_args.futures) {
            index_task_launcher.add_future(future);
          }

          // Enqueue the prolog.
          task_prolog_t task_prolog(
              legion_runtime, legion_context, launch_domain);
          task_prolog.sparse = false;
          task_prolog.walk(task_args);
          task_prolog.launch_copies();

          task_prolog_t task_prolog_sparse(
              legion_runtime, legion_context, launch_domain);
          task_prolog_sparse.sparse = true;
          task_prolog_sparse.walk(task_args);
          task_prolog_sparse.launch_copies();

          index_task_launcher.tag = MAPPER_FORCE_RANK_MATCH;

           future_map = legion_runtime->execute_index_space(
            legion_context, index_task_launcher);

          // Enqueue the epilog.
          task_epilog_t task_epilog(legion_runtime, legion_context);
          task_epilog.walk(task_args);


          return legion_future__<RETURN, launch_type_t::index>(future);
         
      } // if
    }
  };

  /*!
    Partial template specialization for the reduction Single task 
   */
  template <Legion::ReductionOpID REDUCTION_ID, size_t KEY, typename RETURN,
    typename ARG_TUPLE, typename... ARGS>
  struct execute_task__<launch_type_t::single, REDUCTION_ID,KEY, RETURN,
    ARG_TUPLE, ARGS...> {
    static decltype(auto) execute( ARG_TUPLE task_args) {
      using namespace Legion;
      clog(fatal)<<"there is no implementation reduction task with the single task launcher"<<std::endl;
    }
  };

  /*!
    Partial template specialization for the non-reduction Single task 
     (REDUCTIONID==0)
   */
  template < size_t KEY, typename RETURN, typename ARG_TUPLE, typename... ARGS>
  struct execute_task__<launch_type_t::single, 0, KEY, RETURN,
    ARG_TUPLE, ARGS...> {
    static decltype(auto) execute( ARG_TUPLE task_args) {
      using namespace Legion;

      // Make a tuple from the task arguments.
      // ARG_TUPLE task_args = std::make_tuple(args...);

      context_t &context_ = context_t::instance();

      // Get the processor type.
      auto processor_type = context_.processor_type<KEY>();

      // Get the runtime and context from the current task.
      auto legion_runtime = Legion::Runtime::get_runtime();
      auto legion_context = Legion::Runtime::get_context();

      // Handle MPI and Legion invocations separately.
      if (processor_type == processor_type_t::mpi) {
//FIXME
        clog_fatal(" mpi task doesn't have an implementation for the single task execution");
      } else {
        // Initialize the arguments to pass through the runtime.
        init_args_t init_args(legion_runtime, legion_context);
        init_args.walk(task_args);
        clog_tag_guard(execution);
        clog(info) << "Executing single task: " << KEY << std::endl;

        // Create a task launcher, passing the task arguments.
        TaskLauncher task_launcher(context_.task_id<KEY>(),
                                   TaskArgument(&task_args, sizeof(ARG_TUPLE)));

        //FIXME : do I need this tag?
        //task_launcher.tag = MAPPER_SUBRANK_LAUNCH;
#ifdef MAPPER_COMPACTION
        task_launcher.tag = MAPPER_COMPACTED_STORAGE;
#endif

        for (auto &req : init_args.region_reqs) {
          task_launcher.add_region_requirement(req);
        }
        for (auto &future : init_args.futures) {
          task_launcher.add_future(future);
        }

        LegionRuntime::Arrays::Rect<1> launch_bounds(0,1);
        Domain launch_domain = Domain::from_rect<1>(launch_bounds);

        // Enqueue the prolog.
        task_prolog_t task_prolog(
            legion_runtime, legion_context, launch_domain);
        task_prolog.sparse = false;
        task_prolog.walk(task_args);
        task_prolog.launch_copies();

        task_prolog_t task_prolog_sparse(
            legion_runtime, legion_context, launch_domain);
        task_prolog_sparse.sparse = true;
        task_prolog_sparse.walk(task_args);
        task_prolog_sparse.launch_copies();       

        
        // Enqueue the task.
        clog(trace) << "Execute flecsi/legion task " << KEY << " on rank "
                    << legion_runtime->find_local_MPI_rank() << std::endl;
        auto future =
            legion_runtime->execute_task(legion_context, task_launcher);

        // Enqueue the epilog.
        task_epilog_t task_epilog(legion_runtime, legion_context);
        task_epilog.walk(task_args);

        return legion_future__<RETURN, launch_type_t::single>(future);
      } // if
    }
  };

  /*!
    Legion backend task execution. For documentation on this
    method, please see task__::execute_task.
   */
  template <launch_type_t LAUNCH, Legion::ReductionOpID REDUCTION_ID,
	 size_t KEY, typename RETURN,  typename ARG_TUPLE, typename... ARGS>
  static decltype(auto) execute_task(ARGS &&... args) {

    ARG_TUPLE task_args_tmp = std::make_tuple(args...);

    return execute_task__<LAUNCH, REDUCTION_ID, KEY, RETURN, ARG_TUPLE,
                                ARGS...>::execute(task_args_tmp);
  } // execute_task


  //--------------------------------------------------------------------------//
  // Function interface.
  //--------------------------------------------------------------------------//

  /*!
    Legion backend function registration. For documentation on this
    method, please see function__::register_function.
   */

  template <size_t KEY, typename RETURN, typename ARG_TUPLE,
            RETURN (*FUNCTION)(ARG_TUPLE)>
  static bool register_function() {
    return context_t::instance()
        .template register_function<KEY, RETURN, ARG_TUPLE, FUNCTION>();
  } // register_function

  /*!
    Legion backend function execution. For documentation on this
    method, please see function__::execute_function.
   */

  template <typename FUNCTION_HANDLE, typename... ARGS>
  static decltype(auto) execute_function(FUNCTION_HANDLE &handle,
                                         ARGS &&... args) {
    return handle(context_t::instance().function(handle.get_key()),
                  std::forward_as_tuple(args...));
  } // execute_function

}; // struct legion_execution_policy_t

} // namespace execution
} // namespace flecsi
