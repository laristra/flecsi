/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_execution_h
#define flecsi_execution_execution_h

#include <functional>

#include "flecsi/utils/common.h"
#include "flecsi/execution/common/function_handle.h"
#include "flecsi/execution/task.h"
#include "flecsi/execution/function.h"
#include "flecsi/execution/kernel.h"

///
/// \file execution.h
/// \authors bergen
/// \date Initial file creation: Aug 01, 2016
///

//----------------------------------------------------------------------------//
// Function Interface
//----------------------------------------------------------------------------//

///
/// This macro registers a user function with the FleCSI runtime, which may
/// then be passed as state data and executed in any task address space.
///
/// \param name The function to register. This should be the plain-text
///              name of the function (not a string).
/// \param return_type The function return type.
/// \param ... The signature of the function (arguments).
///
#define flecsi_register_function(name)                                         \
                                                                               \
  /* Function return type (trt) */                                             \
  using name ## _trt_t =                                                       \
    typename flecsi::utils::function_traits__<decltype(name)>::return_type;    \
                                                                               \
  /* Function arguments type (tat) */                                          \
  using name ## _tat_t =                                                       \
    typename flecsi::utils::function_traits__<decltype(name)>::arguments_type; \
                                                                               \
  /* Wrapper to call user function with tuple arguments */                     \
  inline name ## _trt_t name ## _tuple_wrapper(name ## _tat_t && args) {       \
    return flecsi::utils::tuple_function(name, args);                          \
  } /* task ## _tuple_wrapper */                                               \
                                                                               \
  /* Make std::function delegate */                                            \
  std::function<name ## _trt_t(name ## _tat_t)>                                \
    name ## _function_delegate = name ## _tuple_wrapper;                       \
                                                                               \
  /* Define name handle type */                                                \
  using function_handle_ ## name ## _t =                                       \
    flecsi::execution::function_handle__<name ## _trt_t, name ## _tat_t>;      \
                                                                               \
  /* Register the function delegate */                                         \
  bool name ## _function_registered =                                          \
    flecsi::execution::function_t::register_function<                          \
      name ## _trt_t, name ## _tat_t>(                                         \
      EXPAND_AND_STRINGIFY(name), name ## _function_delegate)

///
/// Execute a user function.
///
/// \param handle The function handle.
/// \param ... The function arguments.
///
#define flecsi_execute_function(handle, ...)                                   \
  flecsi::execution::function_t::execute_function(handle, ## __VA_ARGS__)

///
/// FIXME
///
#define flecsi_function_handle(name)                                           \
  function_handle_ ## name ## _t(                                              \
    flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(name)}.hash())

///
/// FIXME
///
#define flecsi_define_function_type(name, return_type, ...)                    \
  using name = flecsi::execution::function_handle__<return_type,               \
    std::tuple<__VA_ARGS__>>

//----------------------------------------------------------------------------//
// Task Interface
//----------------------------------------------------------------------------//

///
/// This macro registers a user task with the FleCSI runtime.
///
/// \param task
/// \param processor
/// \param launch
///
#define flecsi_register_task(task, processor, launch)                          \
                                                                               \
  /* Register the user task in the function table */                           \
  flecsi_register_function(task);                                              \
                                                                               \
  /* Register the user task with the execution policy */                       \
  bool task ## _task_registered =                                              \
    flecsi::execution::task_t::register_task<task ## _trt_t, task ## _tat_t>   \
    (reinterpret_cast<uintptr_t>(&task), flecsi::execution::processor,         \
     flecsi::execution::launch)

///
/// This macro executes a user task.
///
/// \param task The user task to execute.
/// \param processor The processor type on which to execute the task.
/// \param launch The launch  mode for the task.
/// \param ... The arguments to pass to the user task during execution.
///
#define flecsi_execute_task(task, processor, launch, ...)                      \
                                                                               \
  /* Execute the user task */                                                  \
  /* WARNING: This macro returns a future. Don't add terminations! */          \
  flecsi::execution::task_t::execute_task<task ## _trt_t>                      \
    (reinterpret_cast<uintptr_t>(&task), flecsi::execution::processor,         \
    flecsi::execution::launch,                                                 \
    flecsi::utils::const_string_t{__func__}.hash(),                            \
    flecsi::execution::function_handle__<task ## _trt_t, task ## _tat_t>(      \
      flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(task)}.hash()),       \
      ## __VA_ARGS__)

//----------------------------------------------------------------------------//
// Kernel Interface
//----------------------------------------------------------------------------//

///
///
///
#define flecsi_for_each(index, index_space, kernel)                            \
  flecsi::execution::for_each__(index_space, [&](auto * index) kernel)

///
///
///
#define flecsi_reduce_each(index, index_space, variable, kernel)               \
  flecsi::execution::reduce_each__(index_space, variable,                      \
    [&](auto * index, auto & variable) kernel)

#endif // flecsi_execution_execution_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
