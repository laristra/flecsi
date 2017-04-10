/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_legion_task_args_h
#define flecsi_execution_legion_task_args_h

#include "flecsi/data/data_handle.h"
#include "flecsi/execution/context.h"
#include "flecsi/execution/common/function_handle.h"
#include "flecsi/utils/common.h"

///
/// \file legion/task_args.h
/// \authors bergen
/// \date Initial file creation: Jul 24, 2016
///

namespace flecsi {
namespace execution {

///
/// Argument type for wrapper task. This type is used to pass the
/// user task and arguments from the call site to the Legion runtime.
///
template<
  typename R,
  typename A
>
struct legion_task_args__
{
  using user_task_handle_t = function_handle__<R, A>;
  using args_t = A;

  legion_task_args__(user_task_handle_t & user_task_handle_,
    args_t & user_args_)
    : user_task_handle(user_task_handle_), user_args(user_args_) {}

  user_task_handle_t user_task_handle;
  args_t user_args;

}; // struct legion_task_args__

} //namespace execution 
} // namespace flecsi

#endif // flecsi_execution_legion_task_args_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
