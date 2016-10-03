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
#define register_task(task, processor, mode)                                 \
                                                                             \
  /* Task return type (trt) */                                               \
  using task ## _trt_t =                                                     \
    typename flecsi::function_traits__<decltype(task)>::return_type;         \
                                                                             \
  /* Task arguments type (tat) */                                            \
  using task ## _tat_t =                                                     \
    typename flecsi::function_traits__<decltype(task)>::arguments_type;      \
                                                                             \
  /* Wrapper to call user task with tuple arguments */                       \
  inline task ## _trt_t task ## _ttwrapper(task ## _tat_t && args) {         \
    return flecsi::tuple_function(task, args);                               \
  } /* task ## _ttwrapper */                                                 \
                                                                             \
  /* Define user task delegate function type */                              \
  using user_task_delegate_ ## task ## _t =                                  \
    std::function<task ## _trt_t(task ## _tat_t)>;                           \
                                                                             \
  /* Create task delegate function */                                        \
  user_task_delegate_ ## task ## _t task ## _task_delegate =                 \
    task ## _ttwrapper;                                                      \
                                                                             \
  /* Register the user task */                                               \
  bool task ## _task_registered =                                            \
    flecsi::execution::task_t::register_task<task ## _trt_t, task ## _tat_t> \
    (reinterpret_cast<uintptr_t>(&task), processor, mode)

///
//
///
#define execute_task(task, processor, mode, ...)                           \
                                                                           \
  /* Execute the user task */                                              \
  /* WARNING: This macro returns a future. Don't add terminations! */      \
  flecsi::execution::task_t::execute_task<task ## _trt_t>                  \
    (reinterpret_cast<uintptr_t>(&task), processor, mode,                  \
    task ## _task_delegate, ## __VA_ARGS__)

//----------------------------------------------------------------------------//
// Function Interface
//----------------------------------------------------------------------------//

///
// This macro registers a user function with the FleCSI runtime, which may
// then be passed as state data and executed in any task address space.
//
// \param fname The function to register. This should be the plain-text
//              name of the function (not a string).
// \param return_type The function return type.
// \param ... The signature of the function (arguments).
///
#define register_function(fname)                                               \
                                                                               \
  /* Function return type (trt) */                                             \
  using fname ## _trt_t =                                                      \
    typename flecsi::function_traits__<decltype(fname)>::return_type;          \
                                                                               \
  /* Function arguments type (trt) */                                          \
  using fname ## _tat_t =                                                      \
    typename flecsi::function_traits__<decltype(fname)>::arguments_type;       \
                                                                               \
  /* Wrapper to call user function with tuple arguments */                     \
  inline fname ## _trt_t fname ## _tfwrapper(fname ## _tat_t && args) {        \
    return flecsi::tuple_function(fname, args);                                \
  } /* task ## _tfwrapper */                                                   \
                                                                               \
  /* Make std::function delegate */                                            \
  std::function<fname ## _trt_t(fname ## _tat_t)>                              \
    fname ## _function_delegate = fname ## _tfwrapper;                         \
                                                                               \
  /* Define fname handle type */                                               \
  using function_handle_ ## fname ## _t =                                      \
    function_handle__<fname ## _trt_t, fname ## _tat_t>;                       \
                                                                               \
  /* Register the function delegate */                                         \
  bool fname ## _function_registered =                                         \
    flecsi::execution::function_t::register_function<                          \
      fname ## _trt_t, fname ## _tat_t>(                                       \
      EXPAND_AND_STRINGIFY(fname), fname ## _function_delegate)

///
// Execute a user function.
//
// \param handle The function handle.
// \param ... The function arguments.
///
#define execute_function(handle, ...)                                         \
  flecsi::execution::function_t::execute_function(handle, ## __VA_ARGS__)

///
// FIXME
///
#define function_handle(fname)                                                \
  function_handle_ ## fname ## _t(                                            \
    const_string_t{EXPAND_AND_STRINGIFY(fname)}.hash())

///
// FIXME
///
#define define_function_type(name, return_type, ...)                          \
  using name = function_handle__<return_type, std::tuple<__VA_ARGS__>>

#endif // flecsi_execution_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
