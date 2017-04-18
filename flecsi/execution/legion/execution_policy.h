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

#ifndef flecsi_execution_legion_execution_policy_h
#define flecsi_execution_legion_execution_policy_h

///
/// \file legion/execution_policy.h
/// \authors bergen
/// \date Initial file creation: Nov 15, 2015
///

#include <functional>
#include <memory>
#include <type_traits>

#include <cinchlog.h>
#include <legion.h>

#include "flecsi/execution/common/processor.h"
#include "flecsi/execution/common/task_hash.h"
#include "flecsi/execution/context.h"
#include "flecsi/execution/legion/context_policy.h"
#include "flecsi/execution/legion/future.h"
#include "flecsi/execution/legion/task_wrapper.h"
#include "flecsi/utils/const_string.h"
#include "flecsi/utils/tuple_walker.h"
#include "flecsi/data/data_handle.h"

clog_register_tag(execution);

namespace flecsi {
namespace execution {

// This is called to walk the task args before the task launcher
// is created for the purposes of gathering region requirements and
// setting any state on the data handle BEFORE Legion gets the task
// args tuple to be serialized.
struct init_args_ : public utils::tuple_walker__<init_args_>{
  init_args_(
    Legion::Runtime* runtime,
    Legion::Context context
  )
  : runtime(runtime),
  context(context){}

  template<
    typename T,
    size_t EP,
    size_t SP,
    size_t GP
  >
  void
  handle(
    data_handle__<T, EP, SP, GP>& h
  )
  {

  }

  template<typename T>
  static
  typename std::enable_if_t<!std::is_base_of<data_handle_base, T>::
    value>
  handle(
    T&
  ){}

  Legion::Runtime* runtime;
  Legion::Context context;
  std::vector<Legion::RegionRequirement> reqs;
};

// This is called to walk the task args after the task launcher
// is created but before the task runs for the purposes of 
// handling synchronization
struct task_prolog_ : public utils::tuple_walker__<task_prolog_>{
  task_prolog_(
    Legion::Runtime* runtime,
    Legion::Context context,
    Legion::TaskLauncher& launcher
  )
  : runtime(runtime),
  context(context),
  launcher(launcher){}

  template<
    typename T,
    size_t EP,
    size_t SP,
    size_t GP
  >
  void
  handle(
    data_handle__<T, EP, SP, GP>& h
  )
  {
  
  }

  template<
    typename T
  >
  static
  typename std::enable_if_t<!std::is_base_of<data_handle_base, T>::value>
  handle(
    T&
  )
  {

  }

  Legion::Runtime* runtime;
  Legion::Context context;
  Legion::TaskLauncher& launcher;
};

// This is called to walk the task args after the task has run 
// for the purpose of handling synchronization
struct task_epilog_ : public utils::tuple_walker__<task_epilog_>{
  task_epilog_(
    Legion::Runtime* runtime,
    Legion::Context context
  )
  : runtime(runtime),
  context(context){}

  template<
    typename T,
    size_t EP,
    size_t SP,
    size_t GP
  >
  void handle(
    data_handle__<T, EP, SP, GP>& h
  ){
  
  }

  template<typename T>
  static
  typename std::enable_if_t<!std::is_base_of<data_handle_base, T>::
    value>
  handle(T&){}

  Legion::Runtime* runtime;
  Legion::Context context;
};

//----------------------------------------------------------------------------//
// Execution policy.
//----------------------------------------------------------------------------//

///
/// \struct legion_execution_policy legion_execution_policy.h
/// \brief legion_execution_policy provides...
///
struct legion_execution_policy_t
{
  template<typename RETURN>
  /// future
  using future__ = legion_future__<RETURN>;

  //--------------------------------------------------------------------------//
  // Task interface.
  //--------------------------------------------------------------------------//

  /// User task registration.
  template<
    typename RETURN,
    typename ARG_TUPLE,
    RETURN (*DELEGATE)(ARG_TUPLE),
    size_t KEY
  >
  static
  bool
  register_task(
    task_hash_key_t key,
    std::string task_name
  )
  {
    // Processor type can be an or-list of values, each of which should
    // be register as a different variant.
    const processor_t processor = key.processor();

    using wrapper_t = task_wrapper__<RETURN, ARG_TUPLE, DELEGATE, KEY>;

    // Register mpi task variant
    if(processor_mpi(processor)) {
      if(!context_t::instance().register_task(
        key, processor_type_t::mpi, task_name,
        wrapper_t::registration_callback)) {
        clog(fatal) << "mpi callback registration failed" << std::endl;
      } // if
    }
    else {
      // Register loc task variant
      if(processor_loc(processor)) {
        if(!context_t::instance().register_task(
          key, processor_type_t::loc, task_name,
          wrapper_t::registration_callback)) {
          clog(fatal) << "loc callback registration failed" << std::endl;
        } // if
      } // if

      // Register loc task variant
      if(processor_loc(processor)) {
        if(!context_t::instance().register_task(
          key, processor_type_t::toc, task_name,
          wrapper_t::registration_callback)) {
          clog(fatal) << "toc callback registration failed" << std::endl;
        } // if
      } // if
    } // if

    return true;
  } // register_task

  /// Legion task registration.
  template<
    typename RETURN,
    RETURN (*TASK)(
      const Legion::Task *,
      const std::vector<Legion::PhysicalRegion> &,
      Legion::Context,
      Legion::Runtime *
    )
  >
  static
  bool
  register_legion_task(
    task_hash_key_t key,
    std::string task_name
  )
  {
    // Processor type can be an or-list of values, each of which should
    // be register as a different variant.
    const processor_t processor = key.processor();

    // Register loc task variant
    if(processor_loc(processor)) {
      if(!context_t::instance().register_task(
        key, processor_type_t::loc, task_name,
        pure_task_wrapper__<RETURN, TASK>::registration_callback)) {
        clog(fatal) << "pure loc callback registration failed" << std::endl;
      } // if
    } // if

    // Register toc task variant
    if(processor_toc(processor)) {
      if(!context_t::instance().register_task(
        key, processor_type_t::toc, task_name,
        pure_task_wrapper__<RETURN, TASK>::registration_callback)) {
        clog(fatal) << "pure toc callback registration failed" << std::endl;
      } // if
    } // if

    return true;
  } // register_legion_task

  ///
  /// Execute FLeCSI task.
  ///
  /// \tparam RETURN The task return type.
  /// \tparam FIXME: A
  ///
  /// \param key
  /// \param parent
  /// \param args
  ///
  template<
    typename RETURN,
    typename ... ARGS
  >
  static
  decltype(auto)
  execute_task(
    task_hash_key_t key,
    size_t parent,
    ARGS && ... args
  )
  {
    using namespace Legion;

    // Make a tuple from the task arguments.
    auto user_task_args = std::make_tuple(args ...);
    using user_task_args_t = decltype(user_task_args);

    // Get the runtime and context from the calling task.
    context_t & context_ = context_t::instance();
    auto legion_runtime = context_.runtime(parent);
    auto legion_context = context_.context(parent);

    // Handle MPI and Legion invocations separately.
    if(processor_mpi(key.processor())) {
      {
      clog_tag_guard(execution);
      clog(info) << "Executing MPI task: " << key << std::endl;
      }

      ArgumentMap arg_map;
      IndexLauncher launcher(
        context_.task_id(key),
        Legion::Domain::from_rect<1>(context_.all_processes()),
        TaskArgument(&user_task_args, sizeof(user_task_args_t)),
        arg_map
      );

      // Enqueue the MPI task.
      auto future =
        legion_runtime->execute_index_space(legion_context, launcher);
      future.wait_all_results();

      // Handoff to the MPI runtime.
      context_.handoff_to_mpi(legion_context, legion_runtime);

      // Wait for MPI to finish execution (synchronous).
      context_.wait_on_mpi(legion_context, legion_runtime);
      
      // Reset the calling state to false.
      context_.unset_call_mpi(legion_context, legion_runtime);

      return legion_future__<RETURN>(future);
    }
    else {
      // Initialize the arguments to pass through the runtime.
      init_args_ init_args(legion_runtime, legion_context);
      init_args.walk(user_task_args);

      const launch_t launch = key.launch();

      // Switch on launch type: single or index.
      if(launch_single(launch)) {
        {
        clog_tag_guard(execution);
        clog(info) << "Executing single task: " << key << std::endl;
        }

        // Create a task launcher, passing the task arguments.
        TaskLauncher task_launcher(context_.task_id(key),
          TaskArgument(&user_task_args, sizeof(user_task_args_t)));

        // Enqueue the prolog.
        task_prolog_
          task_prolog(legion_runtime, legion_context, task_launcher);
        task_prolog.walk(user_task_args);

        // Enqueue the task.
        auto future = context_.runtime(parent)->execute_task(
          context_.context(parent), task_launcher);

        // Enqueue the epilog.
        task_epilog_
          task_epilog(legion_runtime, legion_context);
        task_epilog.walk(user_task_args);

        return legion_future__<RETURN>(future);
      }
      else if(launch_index(launch)) {
        {
        clog_tag_guard(execution);
        clog(info) << "Executing index task: " << key << std::endl;
        }

        // FIXME:
        // FIXME: This looks incomplete!
        // FIXME:
        //FIXME: get launch domain from partitioning of the data used in
        // the task following launch domeing calculation is temporary:
        LegionRuntime::Arrays::Rect<1> launch_bounds(
          LegionRuntime::Arrays::Point<1>(0),
          LegionRuntime::Arrays::Point<1>(5));
        Domain launch_domain = Domain::from_rect<1>(launch_bounds);

        LegionRuntime::HighLevel::ArgumentMap arg_map;
        LegionRuntime::HighLevel::IndexLauncher index_launcher(
          context_.task_id(key), launch_domain,
          TaskArgument(&user_task_args, sizeof(user_task_args_t)), arg_map);

        // Enqueue the task.
        auto future = context_.runtime(parent)->execute_index_space(
          context_.context(parent), index_launcher);

        return legion_future__<RETURN>(future);
      }
      else {
        clog(fatal) << "unsupported task type" << std::endl;
      } // if
    } // if
  } // execute_task

  //--------------------------------------------------------------------------//
  // Function interface.
  //--------------------------------------------------------------------------//

  ///
  /// This method registers a user function with the current
  /// execution context.
  ///
  /// \param key The function identifier.
  /// \param user_function A reference to the user function as a std::function.
  ///
  /// \return A boolean value indicating whether or not the function was
  ///         successfully registered.
  ///
  template<
    typename RETURN,
    typename ... ARGS
  >
  static
  bool
  register_function(
    const utils::const_string_t & key,
    std::function<RETURN(ARGS ...)> & user_function
  )
  {
    return context_t::instance().register_function(key, user_function);
  } // register_function

  ///
  /// This method looks up a function from the \e handle argument
  /// and executes the associated it with the provided \e args arguments.
  ///
  /// \param handle The function handle to execute.
  /// \param args A variadic argument list of the function parameters.
  ///
  /// \return The return type of the provided function handle.
  ///
  template<
    typename FUNCTION_HANDLE,
    typename ... ARGS
  >
  static
  decltype(auto)
  execute_function(
    FUNCTION_HANDLE & handle,
    ARGS && ... args
  )
  {
    auto t = std::make_tuple(args ...);
    return handle(context_t::instance().function(handle.key()), t);
  } // execute_function

}; // struct legion_execution_policy_t

} // namespace execution 
} // namespace flecsi

#endif // flecsi_execution_legion_execution_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
