/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_legion_task_wrapper_h
#define flecsi_legion_task_wrapper_h

#include "flecsi/execution/context.h"
#include "flecsi/utils/tuple_filter.h"
#include "flecsi/utils/common.h"

/*!
 * \file legion_task_wrapper.h
 * \authors bergen
 * \date Initial file creation: Jul 24, 2016
 */

namespace flecsi {

template<typename R, typename ... Args>
struct legion_task_wrapper_
{
  /*
    Type definition for user task.
   */
  using user_task_t = std::function<R(Args ...)>;

  /*
    This defines a predicate function to pass to tuple_filter that
    will select all tuple elements after the first index, i.e., 0.
   */
  template<typename T>
  using greater_than = std::conditional_t<(T()>0), std::true_type,
    std::false_type>;

  /*
    This function is called by the context singleton to do the actual
    registration of the task wrapper with the Legion runtime. The structure
    of the logic used is really just an object factory pattern.
   */
  static void runtime_registration(size_t fid)
  {
    std::cout << "task wrapper registering task " << fid << std::endl;
    std::cout << type<legion_task_wrapper_>() << fid << std::endl;
    LegionRuntime::HighLevel::HighLevelRuntime::register_legion_task<execute>(
      fid, context_t::lr_loc, true, false);
  } // runtime_registration

  /*
    This method unpacks the task arguments from a tuple
    and calls the user's task.
   */
  template<typename ... As, size_t ... Is>
  static R execute_user_task(user_task_t & f, std::tuple<As ...> & t,
    std::index_sequence<Is ...>)
  {
    return f(std::get<Is>(t) ...);
  } // execute_user_task

  /*
    This method takes a tuple of task argements and creates an integer
    sequence so that the arguments can be unpacked at compile-time,
    calling the user's task.
   */
  template<typename ... As>
  static R execute_user_task(user_task_t & f, std::tuple<As ...> & t)
  {
    return execute_user_task(f, t,
      std::make_integer_sequence<size_t, sizeof ...(As)>{});
  } // execute_user_task

  /*
    This method executes the user's task after processing the arguments
    from the Legion runtime.
   */
  static R execute(const LegionRuntime::HighLevel::Task * task,
    const std::vector<LegionRuntime::HighLevel::PhysicalRegion>& regions,
    LegionRuntime::HighLevel::Context context,
    LegionRuntime::HighLevel::HighLevelRuntime * runtime)
  {
    // Set the context for this execution thread
    context_t::instance().set_state(context, runtime, task, regions);

    // Define a tuple type for the task arguments
    using task_args_t = std::tuple<user_task_t, Args ...>;

    // Get the arguments that were passed to Legion on the task launch
    task_args_t & task_args = *(reinterpret_cast<task_args_t *>(task->args));

    // Get the user task
    user_task_t user_task = std::get<0>(task_args);

    // Get the user task arguments
    auto user_args = tuple_filter_index_<greater_than, task_args_t>(task_args);

// FIXME: Still need to process the data manager arguments here before
// passing them to the user function

    // Execute the user task
    return execute_user_task(user_task, user_args);
  } // execute

}; // class legion_task_wrapper_

} // namespace flecsi

#endif // flecsi_legion_task_wrapper_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
