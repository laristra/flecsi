/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_h
#define flecsi_execution_h

#include <functional>

#include "flecsi/execution/task.h"

/*!
 * \file execution.h
 * \authors bergen
 * \date Initial file creation: Aug 01, 2016
 */

#define register_task(task, processor, return_type, ...)     \
  using user_delegate_ ## task ## _t =                       \
    std::function<return_type(__VA_ARGS__)>;                 \
  bool task ## _registered =                                 \
    flecsi::task_t::register_task<return_type, __VA_ARGS__>( \
    reinterpret_cast<uintptr_t>(&task), processor);

#define execute_task(task, ...)                                    \
  user_delegate_ ## task ## _t task ## _delegate = task;           \
  flecsi::task_t::execute_task(reinterpret_cast<uintptr_t>(&task), \
    task ## _delegate, ## __VA_ARGS__)

#endif // flecsi_execution_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
