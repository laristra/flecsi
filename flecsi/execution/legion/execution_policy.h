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
#include <flecsi/data/data.h>
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

  static runtime_state_t &runtime_state(void *task);

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
   We use "execute_task_u" class in purose to be able to use
   partial template specializations for the execute method
   */

  template <launch_type_t LAUNCH, Legion::ReductionOpID REDUCTION_ID,
     size_t KEY, typename RETURN, typename ARG_TUPLE, typename... ARGS>
  struct execute_task_u {
    static void execute(ARGS &&... args) {
      clog_fatal("invalid launch type" << std::endl);
    }
  };

  /*!
    Partial template specialization for the reduction task (Index launch, 
     REDUCTIONID!=0)
   */
  template <size_t KEY, Legion::ReductionOpID REDUCTION_ID, typename RETURN,	    typename ARG_TUPLE, typename... ARGS>
  struct execute_task_u<launch_type_t::index, REDUCTION_ID,  KEY, RETURN,
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

          return legion_future_u<RETURN, launch_type_t::single>(future);
      }
    }
  };//end execute_task_u

   /*!
    Partial template specialization for the non-reduction task Index task 
     (REDUCTIONID==0)
   */
  template <size_t KEY, typename RETURN, typename ARG_TUPLE, typename... ARGS>
  struct execute_task_u<launch_type_t::index, 0,  KEY, RETURN, ARG_TUPLE,
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

       LegionRuntime::Arrays::Rect<1> launch_bounds(0,context_.colors()-1);
       Domain launch_domain = Domain::from_rect<1>(launch_bounds);

      // Handle MPI and Legion invocations separately.
      if (processor_type == processor_type_t::mpi) {
        {
          clog_tag_guard(execution);
          clog(info) << "Executing MPI task: " << KEY << std::endl;
        }

//FIXME the check
//        if (!(legion_context->is_inner_context())) {

    // This will guard the entire method
    clog_tag_guard(execution);
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

    // Execute a tuple walker that applies the task epilog operations
    // on the mapped handles
    task_epilog_t task_epilog(legion_runtime, legion_context);
    task_epilog.walk(task_args);

    //------------------------------------------------------------------------//
    // Single launch
    //------------------------------------------------------------------------//

    if constexpr(LAUNCH == launch_type_t::single) {

          // Enqueue the epilog.
          task_epilog_t task_epilog(legion_runtime, legion_context);
          task_epilog.walk(task_args);

          return legion_future_u<RETURN, launch_type_t::index>(future);
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
//        LegionRuntime::Arrays::Rect<1> launch_bounds(0,context_.colors()-1);
//        Domain launch_domain = Domain::from_rect<1>(launch_bounds);
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
          } // scope

          task_prolog_t task_prolog_sparse(
              legion_runtime, legion_context, launch_domain);
          task_prolog_sparse.sparse = true;
          task_prolog_sparse.walk(task_args);
          task_prolog_sparse.launch_copies();

          index_task_launcher.tag = MAPPER_FORCE_RANK_MATCH;

          auto future_map = legion_runtime->execute_index_space(
            legion_context, index_task_launcher);

          // Execute a tuple walker that applies the task epilog operations
          // on the mapped handles
          task_epilog_t task_epilog(legion_runtime, legion_context);
          task_epilog.walk(task_args);


          return legion_future_u<RETURN, launch_type_t::index>(future_map);
         
      } // if
    }
  };

  /*!
    Partial template specialization for the reduction Single task 
   */
  template <Legion::ReductionOpID REDUCTION_ID, size_t KEY, typename RETURN,
    typename ARG_TUPLE, typename... ARGS>
  struct execute_task_u<launch_type_t::single, REDUCTION_ID,KEY, RETURN,
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
  struct execute_task_u<launch_type_t::single, 0, KEY, RETURN,
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

        return legion_future_u<RETURN, launch_type_t::single>(future);
      } // if
    }
  };

  /*!
    Legion backend task execution. For documentation on this
    method, please see task_u::execute_task.
   */
  template <launch_type_t LAUNCH, Legion::ReductionOpID REDUCTION_ID,
	 size_t KEY, typename RETURN,  typename ARG_TUPLE, typename... ARGS>
  static decltype(auto) execute_task(ARGS &&... args) {

            for(auto & future : init_args.futures) {
              future->add_to_single_task_launcher(launcher);
            } // for

    return execute_task_u<LAUNCH, REDUCTION_ID, KEY, RETURN, ARG_TUPLE,
                                ARGS...>::execute(task_args_tmp);
  } // execute_task


  //--------------------------------------------------------------------------//
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
