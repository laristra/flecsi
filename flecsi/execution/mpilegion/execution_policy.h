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
#include "flecsi/execution/processor.h"
#include "flecsi/execution/mpilegion/context_policy.h"
#include "flecsi/execution/mpilegion/task_wrapper.h"

/*!
 * \file mpilegion/execution_policy.h
 * \authors bergen
 * \date Initial file creation: Nov 15, 2015
 */

namespace flecsi {
namespace execution {

/*!
  \struct mpilegion_execution_policy mpilegion/execution_policy.h
  \brief mpilegion_execution_policy provides...
 */
struct mpilegion_execution_policy_t
{
  using task_key_t = uintptr_t;

  /*--------------------------------------------------------------------------*
   * Task interface.
   *--------------------------------------------------------------------------*/

  /*
    To add:
      processor type
      task type (leaf, inner, etc...)
   */
  template<
    typename R,
    typename ... As
  >
  static
  bool
  register_task(
    uintptr_t key,
    processor_t processor
  )
  {
    switch (processor){
     case loc:
      return context_t::instance().register_task(key,
        legion_task_wrapper_<loc, 1, 0, R, As ...>::runtime_registration);
     break;
     case toc:
      return context_t::instance().register_task(key,
        legion_task_wrapper_<toc, 1, 0, R, As ...>::runtime_registration);
     break;
     case mpi:
      return context_t::instance().register_task(key,
        legion_task_wrapper_<mpi, 1, 0, R, As ...>::runtime_registration);
     break;
     default: throw std::runtime_error("unsupported processor type");
   }
  } // register_task

  template<
    typename T,
    typename ... As
  >
  static
  decltype(auto)
  execute_task(
    uintptr_t key,
    processor_t processor, 
    T user_task,
    As ... args
  )
  {
    using namespace Legion;

    context_t & context_ = context_t::instance();

    using task_args_t = std::tuple<T, As ...>;

    // We can't use std::forard or && references here because
    // the calling state is not guarunteed to exist when the
    // task is invoked, i.e., we have to use copies...
    task_args_t task_args(user_task, args ...);

    if (processor==mpi)
    {
     MPILegionInterop *Interop =  MPILegionInterop::instance();
        Interop->shared_func=std::bind(user_task,std::forward<As>(args) ...);
     Interop->call_mpi=true;
     Interop->handoff_to_mpi(context_.context(), context_.runtime());
     //mpi task is running here
     Interop->wait_on_mpi(context_.context(), context_.runtime()); 
    }
    else{
std::cout <<"inside execute task" << std::endl;
     TaskLauncher task_launcher(context_.task_id(key),
       TaskArgument(&task_args, sizeof(task_args_t)));

     return context_.runtime()->execute_task(context_.context(), task_launcher);
    }
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
