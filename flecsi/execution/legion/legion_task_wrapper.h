/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_legion_task_wrapper_h
#define flecsi_legion_task_wrapper_h

#include <flecsi/execution/context.h>

/*!
 * \file legion_task_wrapper.h
 * \authors bergen
 * \date Initial file creation: Jul 24, 2016
 */

namespace flecsi {

template<typename R, typename ... Args>
struct legion_task_wrapper_
{
  using lr_context_t = LegionRuntime::HighLevel::Context;
  using lr_runtime_t = LegionRuntime::HighLevel::HighLevelRuntime;
  using lr_task_t = LegionRuntime::HighLevel::Task;
  using lr_regions_t =
    std::vector<LegionRuntime::HighLevel::PhysicalRegion>;

  using user_function_t = std::function<R(Args ...)>;

  static void runtime_registration(size_t fid)
  {
    std::cout << "runtime registration " << fid << std::endl;
    lr_runtime_t::register_legion_task<execute>(fid, context_t::lr_loc,
      true, false);
  }

  static void execute(const lr_task_t * task, const lr_regions_t & regions,
    lr_context_t context, lr_runtime_t * runtime)
    {
      // User task arguments
      tuple_wrapper_<user_function_t, Args ...> * task_args =
        reinterpret_cast<tuple_wrapper_<
          user_function_t, Args ...> *>(task->args);

      // User task
      user_function_t user_task = task_args->template get<0>();

      std::cout << "Hello from task_wrapper" << std::endl;
      std::cout << "Value from task_wrapper: " <<
        task_args->template get<1>() << std::endl;

      user_task(10.0);
    } // execute

}; // class legion_task_wrapper_

} // namespace flecsi

#endif // flecsi_legion_task_wrapper_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
