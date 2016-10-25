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

#include "flecsi/utils/const_string.h"
#include "flecsi/execution/context.h"
#include "flecsi/execution/common/processor.h"
#include "flecsi/execution/common/task_hash.h"
#include "flecsi/execution/legion/context_policy.h"
#include "flecsi/execution/legion/task_wrapper.h"
#include "flecsi/utils/any.h"

///
// \file legion/execution_policy.h
// \authors bergen
// \date Initial file creation: Nov 15, 2015
///

namespace flecsi {
namespace execution {

// Forward.
template<typename R> struct legion_future__;

///
// \struct legion_execution_policy legion_execution_policy.h
// \brief legion_execution_policy provides...
///
struct legion_execution_policy_t
{
  template<typename R>
  using future__ = legion_future__<R>;

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
  // \tparam FIXME: A
  //
  // \param key
  // \param user_task
  // \param args
  ///
  template<
    typename R,
    typename T,
    typename A
  >
  static
  decltype(auto)
  execute_task(
    task_hash_key_t key,
    T user_task,
    A user_task_args
  )
  {
    using namespace Legion;

    context_t & context_ = context_t::instance();

    using task_args_t = legion_task_args__<R, A>;

    // We can't use std::forward or && references here because
    // the calling state is not guarunteed to exist when the
    // task is invoked, i.e., we have to use copies...
    task_args_t task_args(user_task, user_task_args);

    if(std::get<2>(key)==single) {
      TaskLauncher task_launcher(context_.task_id(key),
        TaskArgument(&task_args, sizeof(task_args_t)));
      context_.runtime()->execute_task(context_.context(), task_launcher);

      //FIXME
      return legion_future__<R>();
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

      //FIXME
      return legion_future__<R>();
    } //end if

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


//----------------------------------------------------------------------------//
// Future.
//----------------------------------------------------------------------------//

#if 0
struct legion_future_base_t
{
  virtual ~legion_future_base_t() {}

  virtual void wait() = 0;

  template<typename R>
  virtual const R & get(size_t index = 0) = 0;

}; // struct legion_future_base_t

template<
  typename F
>
struct legion_future_model__
  : public legion_future_base_t
{
  // Member data.
  F future_;  

  legion_future_model__(const F & future)
    : future_(future) {}

  void
  wait()
  {
    future_.wait();
  } // wait

  template<typename R>
  const
  R &
  get(
    size_t index = 0
  )
  {
    return future_.get();
  }

}; // struct legion_future_model__

template<>
struct legion_future_model__<LegionRuntime::HighLevel::FutureMap>
  : public future_base_t
{
  // Member data.
  F future_;  

  legion_future_model__(const F & future)
    : future_(future) {}

  void
  wait()
  {
    future_.wait();
  } // wait

  template<typename R>
  const
  R &
  get(
    size_t index = 0
  )
  {
    // FIXME: What is a domain point?
    return 0;
  }

}; // legion_future_model__
#endif

template<
  typename R
>
struct legion_future_concept__
{
  virtual ~legion_future_concept__() {}

  virtual void wait() = 0;
  virtual const R & get(size_t index = 0) = 0;
}; // struct legion_future_concept__

template<typename R, typename F>
struct legion_future_model__
  : public legion_future_concept__<R>
{

  legion_future_model__(const F & legion_future)
    : legion_future_(legion_future) {}

  void
  wait()
  {
  } // wait

  const
  R &
  get(
    size_t index = 0
  )
  {
  } // get

private:

  F legion_future_;

}; // struct legion_future_model__

template<typename R>
struct legion_future_model__<R, LegionRuntime::HighLevel::FutureMap>
  : public legion_future_concept__<R>
{

  legion_future_model__(
    const LegionRuntime::HighLevel::FutureMap & legion_future
  )
    : legion_future_(legion_future) {}

  void
  wait()
  {
  } // wait

  const
  R &
  get(
    size_t index = 0
  )
  {
  } // get

private:

  LegionRuntime::HighLevel::FutureMap legion_future_;

};

template<
  typename R
>
struct legion_future__
{
  using result_t = R;

  template<typename F>
  legion_future__(const F & future)

  ///
  //
  ///
  void wait()
  {
  } // wait

  ///
  //
  ///
  const result_t &
  get(
    size_t index = 0
  )
  {
    return 0;
  } // get

private:

  // Needed to satisfy static check.
  void set() {}

}; // struct legion_future__

template<>
struct legion_future__<void>
{

  ///
  //
  ///
  void wait()
  {
  } // wait

}; // struct legion_future__

} // namespace execution 
} // namespace flecsi

#endif // flecsi_execution_legion_execution_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
