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

#include <functional>

#include "flecsi/utils/const_string.h"
#include "flecsi/execution/context.h"
#include "flecsi/execution/common/processor.h"
#include "flecsi/execution/common/task_hash.h"
#include "flecsi/execution/legion/context_policy.h"
#include "flecsi/execution/legion/task_wrapper.h"

///
// \file legion/execution_policy.h
// \authors bergen
// \date Initial file creation: Nov 15, 2015
///

namespace flecsi {
namespace execution {

///
// \struct legion_execution_policy legion_execution_policy.h
// \brief legion_execution_policy provides...
///
struct legion_execution_policy_t
{

  //--------------------------------------------------------------------------//
  // Task interface.
  //--------------------------------------------------------------------------//

  // FIXME: add task type (leaf, inner, etc...)
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
    // Get the processor and launch types
    const processor_t processor = std::get<1>(key);
    const launch_t launch = std::get<2>(key);

    switch (processor) {

      case loc:
        if(launch == single) {
            return context_t::instance().register_task(key,
              legion_task_registrar__<loc, 1, 0, R, A>::register_task);
        }
        else if(launch == index) {
            return context_t::instance().register_task(key,
              legion_task_registrar__<loc, 0, 1, R, A>::register_task);
        }
        else {
            return context_t::instance().register_task(key,
              legion_task_registrar__<loc, 1, 1, R, A>::register_task);
        } // if

      case toc:
        if(launch == single) {
            return context_t::instance().register_task(key,
              legion_task_registrar__<loc, 1, 0, R, A>::register_task);
        }
        else if(launch == index) {
            return context_t::instance().register_task(key,
              legion_task_registrar__<loc, 0, 1, R, A>::register_task);
        }
        else {
            return context_t::instance().register_task(key,
              legion_task_registrar__<loc, 1, 1, R, A>::register_task);
        } // if

      default:
        throw std::runtime_error("unsupported processor type");

    } // switch
  } // register_task

  ///
  // \tparam R The task return type.
  // \tparam T The user task type.
  // \tparam As The user task argument types.
  //
  // \param key
  // \param user_task
  // \param args
  ///
  template<
    typename R,
    typename T,
    typename ... As
  >
  static
  decltype(auto)
  execute_task(
    task_hash_key_t key,
    T user_task,
    As ... args
  )
  {
    using namespace Legion;

    context_t & context_ = context_t::instance();

    using task_args_t = legion_task_args__<R, std::tuple<As ...>>;

    auto user_task_args = std::make_tuple(args ...);

    // We can't use std::forward or && references here because
    // the calling state is not guarunteed to exist when the
    // task is invoked, i.e., we have to use copies...
    task_args_t task_args(user_task, user_task_args);

      if (std::get<2>(key)==single) {
        TaskLauncher task_launcher(context_.task_id(key),
          TaskArgument(&task_args, sizeof(task_args_t)));
        context_.runtime()->execute_task(context_.context(), task_launcher);
       //FIXME: return Future
       return 0;
      }
      else {
       //FIXME: get launch domain from partitioning of the data used in
       // the task following launch domeing calculation is temporary:
       Rect<1> launch_bounds(Point<1>(0),Point<1>(5));
       Domain launch_domain = Domain::from_rect<1>(launch_bounds);

        LegionRuntime::HighLevel::ArgumentMap arg_map;
        LegionRuntime::HighLevel::IndexLauncher index_launcher(
          context_.task_id(key), launch_domain, TaskArgument(&task_args,
          sizeof(task_args_t)), arg_map);

        context_.runtime()->execute_index_space(context_.context(),
          index_launcher);

        //FIXME: return future_t
        return 0;
        } //end if

    // FIXME: Add region requirements and fields

  } // execute_task

  //--------------------------------------------------------------------------//
  // Function interface.
  //--------------------------------------------------------------------------//

  ///
  // This method registers a user function with the current
  // execution context.
  //
  // \param key The function identifier.
  // \param user_function A reference to the user function as a std::function.
  //
  // \return A boolean value indicating whether or not the function was
  //         successfully registered.
  ///
  template<
    typename R,
    typename ... As
  >
  static
  bool
  register_function(
    const const_string_t & key,
    std::function<R(As ...)> & user_function
  )
  {
    context_t::instance().register_function(key, user_function);
  } // register_function

  ///
  // This method looks up a function from the \e handle argument
  // and executes the associated it with the provided \e args arguments.
  //
  // \param handle The function handle to execute.
  // \param args A variadic argument list of the function parameters.
  //
  // \return The return type of the provided function handle.
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
    return handle(context_t::instance().function(handle.key), t);
  } // execute_function

}; // struct legion_execution_policy_t

} // namespace execution 
} // namespace flecsi

#endif // flecsi_execution_legion_execution_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
