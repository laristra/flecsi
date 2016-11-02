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

#ifndef flecsi_mpilegion_execution_policy_h
#define flecsi_mpilegion_execution_policy_h

#include <functional>

#include "flecsi/utils/const_string.h"
#include "flecsi/execution/context.h"
#include "flecsi/execution/legion/future.h"
#include "flecsi/execution/common/processor.h"
#include "flecsi/execution/common/task_hash.h"
#include "flecsi/execution/mpilegion/context_policy.h"
#include "flecsi/execution/legion/task_wrapper.h"

/*!
 * \file mpilegion/execution_policy.h
 * \authors bergen, demeshko
 * \date Initial file creation: Nov 15, 2015
 */

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
// Execution policy.
//----------------------------------------------------------------------------//

///
// \struct mpilegion_execution_policy mpilegion/execution_policy.h
// \brief mpilegion_execution_policy provides...
///
struct mpilegion_execution_policy_t
{
  template<typename R>
  using future__ = legion_future__<R>;

  /*--------------------------------------------------------------------------*
   * Task interface.
   *--------------------------------------------------------------------------*/

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

    switch(key.processor()) {

      case loc:
      {
        switch(key.launch()) {

          case single:
          {
          return context_t::instance().register_task(key,
            legion_task_wrapper__<loc, 1, 0, R, A>::register_task);
          } // case single

          case index:
          {
          return context_t::instance().register_task(key,
            legion_task_wrapper__<loc, 0, 1, R, A>::register_task);
          } // case single

          case any:
          {
          return context_t::instance().register_task(key,
            legion_task_wrapper__<loc, 1, 1, R, A>::register_task);
          } // case single

        } // switch
      } // case loc

      case toc:
      {
        switch(key.launch()) {

          case single:
          {
          return context_t::instance().register_task(key,
            legion_task_wrapper__<toc, 1, 0, R, A>::register_task);
          } // case single

          case index:
          {
          return context_t::instance().register_task(key,
            legion_task_wrapper__<toc, 0, 1, R, A>::register_task);
          } // case single

          case any:
          {
          return context_t::instance().register_task(key,
            legion_task_wrapper__<toc, 1, 1, R, A>::register_task);
          } // case single

        } // switch
      } // case toc

      case mpi:
      {
        return context_t::instance().register_task(key,
          legion_task_wrapper__<mpi, 0, 1, R, A>::register_task);
      } // case mpi

      default:
        throw std::runtime_error("unsupported processor type");
    } // switch
 
  } // register_task

  template<
    typename R,
    typename T,
    typename A
  >
  static
  decltype(auto)
  execute_task(
    task_hash_key_t key,
    size_t parent,
    T user_task_handle,
    A user_task_args
  )
  {
    using namespace Legion;

    context_t & context_ = context_t::instance();

    using task_args_t = legion_task_args__<R, A>;

    // We can't use std::forward or && references here because
    // the calling state is not guarunteed to exist when the
    // task is invoked, i.e., we have to use copies...
    task_args_t task_args(user_task_handle, user_task_args);

    if(key.processor() == mpi) {

      // Executing Legion task that pass function pointer and 
      // its arguments to every MPI thread
      LegionRuntime::HighLevel::ArgumentMap arg_map;

      LegionRuntime::HighLevel::IndexLauncher index_launcher(
        context_.task_id(key),

      LegionRuntime::HighLevel::Domain::from_rect<2>(
        context_.interop_helper_.all_processes_),
        TaskArgument(&task_args, sizeof(task_args_t)), arg_map);

      index_launcher.tag = MAPPER_ALL_PROC;

      LegionRuntime::HighLevel::FutureMap fm1 =
        context_.runtime(parent)->execute_index_space(context_.context(parent),
        index_launcher);

      fm1.wait_all_results();

      context_.interop_helper_.handoff_to_mpi(context_.context(parent),
         context_.runtime(parent));

// mpi task is running here
//
//      flecsi::execution::future_t future = 

      context_.interop_helper_.wait_on_mpi(context_.context(parent),
        context_.runtime(parent));

    //TOFIX: these flag should be switch iside of the index task
    //as below

      ext_legion_handshake_t::instance().call_mpi_ = false; 

     //context_.interop_helper_.unset_call_mpi(context_.context(),
      //                      context_.runtime());
   
      return 0;
    }
    else {

      switch(key.launch()) {

        case single:
        {
          TaskLauncher task_launcher(context_.task_id(key),
            TaskArgument(&task_args, sizeof(task_args_t)));

          auto future = context_.runtime(parent)->execute_task(
            context_.context(parent), task_launcher);
          //return legion_future__<R>(future);
          return 0;
        } // single

        case index:
        {
          LegionRuntime::HighLevel::ArgumentMap arg_map;
          LegionRuntime::HighLevel::IndexLauncher index_launcher(
            context_.task_id(key),
            LegionRuntime::HighLevel::Domain::from_rect<2>(
              context_.interop_helper_.all_processes_),
            TaskArgument(&task_args, sizeof(task_args_t)), arg_map);

          index_launcher.tag = MAPPER_ALL_PROC;

          auto future = context_.runtime(parent)->execute_index_space(
            context_.context(parent), index_launcher);

          return 0;
        } // index

      } // switch
    } // if
  } // execute_task

  /*--------------------------------------------------------------------------*
   * Function interface.
   *--------------------------------------------------------------------------*/

  /*!
    This method registers a user function with the current
    execution context.
    
    \param key The function identifier.
    \param user_function A reference to the user function as a std::function.

    \return A boolean value indicating whether or not the function was
      successfully registered.
   */
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

  /*!
    This method looks up a function from the \e handle argument
    and executes the associated it with the provided \e args arguments.
    
    \param handle The function handle to execute.
    \param args A variadic argument list of the function parameters.

    \return The return type of the provided function handle.
   */
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

}; // struct mpilegion_execution_policy_t

} // namespace execution 
} // namespace flecsi

#endif // flecsi_mpilegion_execution_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
