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

#ifndef flecsi_legion_execution_policy_h
#define flecsi_legion_execution_policy_h

#include <cstdint>

//#include "flecsi/utils/tuple_for_each.h"

#include "flecsi/execution/context.h"
#include "flecsi/execution/legion/legion_context_policy.h"
#include "flecsi/execution/legion/legion_task_wrapper.h"

/*!
 * \file legion_execution_policy.h
 * \authors bergen
 * \date Initial file creation: Nov 15, 2015
 */

namespace flecsi {

/*!
  \class legion_execution_policy legion_execution_policy.h
  \brief legion_execution_policy provides...
 */
class legion_execution_policy_t
{
public:

  using return_type_t = int32_t;
  using task_key_t = uintptr_t;

  /*
    To add:
      processor type
      task type (leaf, inner, etc...)
   */
  template<typename R, typename ... Args>
  static bool register_task(uintptr_t key) {

      using task_wrapper_t = legion_task_wrapper_<R, Args ...>;

      return context_t::instance().register_task(key,
        task_wrapper_t::runtime_registration);

  } // register_task

  template<typename T, typename ... Args>
  static return_type_t execute_task(uintptr_t key, T user_task,
    Args ... args)
  {
    using namespace Legion;

    context_t & context_ = context_t::instance();

    using task_args_t = std::tuple<T, Args ...>;

    // We can't use std::forard or && references here because
    // the calling state is not guarunteed to exist when the
    // task is invoked, i.e., we have to use copies...
    task_args_t task_args(user_task, args ...);

    TaskLauncher task_launcher(context_.task_id(key),
      TaskArgument(&task_args, sizeof(task_args_t)));

    context_.runtime()->execute_task(context_.context(), task_launcher);
  } // execute_task

#if 0
  kernel_handle_t register_kernel(uintptr_t key, T && kernel,
    Args &&... args)
  {
  } // register_kernel
#endif

}; // class legion_execution_policy_t

} // namespace flecsi

#endif // flecsi_legion_execution_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
