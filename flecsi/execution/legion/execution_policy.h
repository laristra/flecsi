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

struct init_args_ : public utils::tuple_walker__<init_args_>{
  init_args_(Legion::Runtime* runtime, Legion::Context context)
  : runtime(runtime),
  context(context){}

  template<typename T, size_t EP, size_t SP, size_t GP>
  void handle(data_handle__<T, EP, SP, GP>& h){
  
  }

  template<typename T>
  static
  typename std::enable_if_t<!std::is_base_of<data_handle_base, T>::
    value>
  handle(T&){}

  Legion::Runtime* runtime;
  Legion::Context context;
  std::vector<Legion::RegionRequirement> reqs;
};

struct task_prolog_ : public utils::tuple_walker__<init_args_>{
  task_prolog_(Legion::Runtime* runtime,
               Legion::Context context,
               Legion::TaskLauncher& launcher)
  : runtime(runtime),
  context(context),
  launcher(launcher){}

  template<typename T, size_t EP, size_t SP, size_t GP>
  void handle(data_handle__<T, EP, SP, GP>& h){
  
  }

  template<typename T>
  static
  typename std::enable_if_t<!std::is_base_of<data_handle_base, T>::
    value>
  handle(T&){}

  Legion::Runtime* runtime;
  Legion::Context context;
  Legion::TaskLauncher& launcher;
};

struct task_epilog_ : public utils::tuple_walker__<init_args_>{
  task_epilog_(Legion::Runtime* runtime,
               Legion::Context context)
  : runtime(runtime),
  context(context){}

  template<typename T, size_t EP, size_t SP, size_t GP>
  void handle(data_handle__<T, EP, SP, GP>& h){
  
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
  template<typename R>
  /// future
  using future__ = legion_future__<R>;

  //--------------------------------------------------------------------------//
  // Task interface.
  //--------------------------------------------------------------------------//

  ///
  /// register FLeCSI task depending on the tasks's processor and launch types
  ///
  template<
    typename R,
    typename A
  >
  static
  bool
  register_task(
    task_hash_key_t key
  )
  {
    const launch_t launch = key.launch();

    switch(key.processor()) {

      case loc:
      {
        if(launch_single(launch) && launch_index(launch)) {
          return context_t::instance().register_task(key,
            legion_task_wrapper__<loc, 1, 1, R, A>::register_callback);
        }
        else if(launch_single(launch)) {
          return context_t::instance().register_task(key,
            legion_task_wrapper__<loc, 1, 0, R, A>::register_callback);
        }
        else if(launch_index(launch)) {
          return context_t::instance().register_task(key,
            legion_task_wrapper__<loc, 0, 1, R, A>::register_callback);
        } // if
      } // loc

      case toc:
      {
        if(launch_single(launch) && launch_index(launch)) {
          return context_t::instance().register_task(key,
            legion_task_wrapper__<toc, 1, 1, R, A>::register_callback);
        }
        else if(launch_single(launch)) {
          return context_t::instance().register_task(key,
            legion_task_wrapper__<toc, 1, 0, R, A>::register_callback);
        }
        else if(launch_index(launch)) {
          return context_t::instance().register_task(key,
            legion_task_wrapper__<toc, 0, 1, R, A>::register_callback);
        } // if
      } // toc

      default:
        clog(fatal) << "unsupported processor type" << std::endl;
    } // switch

  } // register_task

  ///
  /// Execute FLeCSI task.
  /// \tparam R The task return type.
  /// \tparam T The user task type.
  /// \tparam FIXME: A
  ///
  /// \param key
  /// \param user_task_handle
  /// \param args
  ///
  template<
    typename R,
    typename T,
    typename...As
  >
  static
  decltype(auto)
  execute_task(
    task_hash_key_t key,
    size_t parent,
    T user_task_handle,
    As && ... user_task_args
  )
  {
    using namespace Legion;

    context_t & context_ = context_t::instance();

    {
    clog_tag_guard(execution);
    clog(info) << __PRETTY_FUNCTION__ << std::endl;
    }

    auto user_task_args_tuple = std::make_tuple(user_task_args...);
    using user_task_args_tuple_t = decltype( user_task_args_tuple );

    using task_args_t =
      legion_task_args__<R,typename T::args_t, user_task_args_tuple_t>;

    auto legion_runtime = context_.runtime(parent);
    auto legion_context = context_.context(parent);

    init_args_ init_args(legion_runtime, legion_context);
    init_args.walk(user_task_args_tuple);

    // We can't use std::forward or && references here because
    // the calling state is not guaranteed to exist when the
    // task is invoked, i.e., we have to use copies...
    task_args_t task_args(user_task_handle, user_task_args_tuple);

    const launch_t launch = key.launch();

    // Switch on launch type: single or index.
    if(launch_single(launch)) {
      TaskLauncher task_launcher(context_.task_id(key),
        TaskArgument(&task_args, sizeof(task_args_t)));

      task_prolog_
        task_prolog(legion_runtime, legion_context, task_launcher);
      task_prolog.walk(user_task_args_tuple);

      auto future = context_.runtime(parent)->execute_task(
        context_.context(parent), task_launcher);

      task_epilog_
        task_epilog(legion_runtime, legion_context);
      task_epilog.walk(user_task_args_tuple);

      return legion_future__<R>(future);
    }
    else if(launch_index(launch)) {
      //FIXME: get launch domain from partitioning of the data used in
      // the task following launch domeing calculation is temporary:
      LegionRuntime::Arrays::Rect<1> launch_bounds(
        LegionRuntime::Arrays::Point<1>(0),
        LegionRuntime::Arrays::Point<1>(5));
      Domain launch_domain = Domain::from_rect<1>(launch_bounds);

      LegionRuntime::HighLevel::ArgumentMap arg_map;
      LegionRuntime::HighLevel::IndexLauncher index_launcher(
        context_.task_id(key), launch_domain, TaskArgument(&task_args,
        sizeof(task_args_t)), arg_map);

      // !!! needs to do what legion does for data handle

      auto future = context_.runtime(parent)->execute_index_space(
        context_.context(parent), index_launcher);

      return legion_future__<R>(future);
    }
    else {
      clog(fatal) << "unsupported task type" << std::endl;
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
    typename R,
    typename ... As
  >
  static
  bool
  register_function(
    const utils::const_string_t & key,
    std::function<R(As ...)> & user_function
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
    typename T,
    typename ... As
  >
  static
  decltype(auto)
  execute_function(
    T & handle,
    As && ... args
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
