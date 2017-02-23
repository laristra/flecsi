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
#include <memory>

#include <legion.h>

#include "flecsi/execution/common/processor.h"
#include "flecsi/execution/common/task_hash.h"
#include "flecsi/execution/context.h"
#include "flecsi/execution/legion/context_policy.h"
#include "flecsi/execution/legion/future.h"
#include "flecsi/execution/legion/task_wrapper.h"
#include "flecsi/utils/const_string.h"

///
/// \file legion/execution_policy.h
/// \authors bergen
/// \date Initial file creation: Nov 15, 2015
///

namespace flecsi {
namespace execution {

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

  // FIXME: add task type (leaf, inner, etc...)
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
    switch(key.processor()) {

      case loc:
      {
        switch(key.launch()) {

          case single:
          {
            return context_t::instance().register_task(key,
              legion_task_wrapper__<loc, 1, 0, R, A>::register_task);
          } // single

          case index:
          {
            return context_t::instance().register_task(key,
              legion_task_wrapper__<loc, 0, 1, R, A>::register_task);
          } // index

          case any:
          {
            return context_t::instance().register_task(key,
              legion_task_wrapper__<loc, 1, 1, R, A>::register_task);
          } // any

        } // switch
      } // loc

      case toc:
      {
        switch(key.launch()) {

          case single:
          {
            return context_t::instance().register_task(key,
              legion_task_wrapper__<toc, 1, 0, R, A>::register_task);
          } // single

          case index:
          {
            return context_t::instance().register_task(key,
              legion_task_wrapper__<toc, 0, 1, R, A>::register_task);
          } // index

          case any:
          {
            return context_t::instance().register_task(key,
              legion_task_wrapper__<toc, 1, 1, R, A>::register_task);
          } // any

        } // switch
      } // toc

      default:
        throw std::runtime_error("unsupported processor type");
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

    auto user_task_args_tuple = std::make_tuple(user_task_args...);
    using user_task_args_tuple_t = decltype( user_task_args_tuple );

    using task_args_t = legion_task_args__<R, user_task_args_tuple_t>;

    // We can't use std::forward or && references here because
    // the calling state is not guarunteed to exist when the
    // task is invoked, i.e., we have to use copies...
    task_args_t task_args(user_task_handle, user_task_args_tuple);

    // Switch on launch type: single or index.
    switch(key.launch()) {

      case single:
      {
        TaskLauncher task_launcher(context_.task_id(key),
          TaskArgument(&task_args, sizeof(task_args_t)));

        auto future = context_.runtime(parent)->execute_task(
          context_.context(parent), task_launcher);

        return legion_future__<R>(future);
      } // single

      case index:
      {
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

        auto future = context_.runtime(parent)->execute_index_space(
          context_.context(parent), index_launcher);

        return legion_future__<R>(future);
      } // index
        
      default:
      {
        throw std::runtime_error("the task can be executed \
                    only as single or index task");
      }

    } // switch
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
