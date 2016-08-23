/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_h
#define flecsi_execution_h

#include <functional>

#include "flecsi/utils/common.h"
#include "flecsi/execution/common/function_handle.h"
#include "flecsi/execution/task.h"
#include "flecsi/execution/function.h"

///
// \file execution.h
// \authors bergen
// \date Initial file creation: Aug 01, 2016
///

//----------------------------------------------------------------------------//
// Task Interface
//----------------------------------------------------------------------------//

// FIXME: Finish Doxygen

///
//
///
#define register_task(task, processor, return_type, ...)                \
  using user_task_delegate_ ## task ## _t =                             \
    std::function<return_type(__VA_ARGS__)>;                            \
  bool task ## _task_registered =                                       \
    flecsi::execution::task_t::register_task<return_type, __VA_ARGS__>( \
    reinterpret_cast<uintptr_t>(&task), processor);

///
//
///
#define execute_task(task, processor, ...)                         \
  user_task_delegate_ ## task ## _t task ## _task_delegate = task; \
  flecsi::execution::task_t::execute_task(                         \
    reinterpret_cast<uintptr_t>(&task),                            \
    processor, task ## _task_delegate, ## __VA_ARGS__)

//----------------------------------------------------------------------------//
// Function Interface
//----------------------------------------------------------------------------//

///
//
///
#define register_function(fname, return_type, ...)               \
  /* Define fname handle type */                                 \
  using function_handle_ ## fname ## _t =                        \
    function_handle__<return_type, __VA_ARGS__>;                 \
                                                                 \
  /* Make std::function delegate */                              \
  std::function<return_type(__VA_ARGS__)>                        \
    fname ## _function_delegate = fname;                         \
                                                                 \
  /* Register the function delegate */                           \
  bool fname ## _function_registered =                           \
    flecsi::execution::function_t::register_function<            \
      return_type, __VA_ARGS__>(                                 \
      EXPAND_AND_STRINGIFY(fname), fname ## _function_delegate);

///
//
///
#define execute_function(handle, ...)                                     \
  flecsi::execution::function_t::execute_function(handle, ## __VA_ARGS__)

///
//
///
#define function_handle(fname)                          \
  function_handle_ ## fname ## _t(                      \
    const_string_t{EXPAND_AND_STRINGIFY(fname)}.hash())

///
//
///
#define define_function_type(name, return_type, ...)       \
  using name = function_handle__<return_type, __VA_ARGS__>

#endif // flecsi_execution_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
