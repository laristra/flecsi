/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_execution_h
#define flecsi_execution_execution_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Aug 01, 2016
//----------------------------------------------------------------------------//

#include <functional>

#include "flecsi/execution/common/function_handle.h"
#include "flecsi/execution/common/task_hash.h"
#include "flecsi/execution/function.h"
#include "flecsi/execution/kernel.h"
#include "flecsi/execution/task.h"
#include "flecsi/utils/common.h"

clog_register_tag(execution);

//----------------------------------------------------------------------------//
// Task Interface
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
//! @def flecsi_register_functor_task
//!
//! This macro registers a functor task with the FleCSI runtime.
//!
//! @param task_t    The task type to register. This is a functor type with
//!                  an execute method implementing the user task.
//! @param processor The \ref processor_t type. This may be an \em or list
//!                  of supported processor types.
//! @param launch    The \ref launch_t type. This may be an \em or list of
//!                  supported launch types and configuration options.
//!
//! @ingroup execution
//----------------------------------------------------------------------------//

#define flecsi_register_functor_task(task_t, processor, launch)                \
/* MACRO IMPLEMENTATION */                                                     \
                                                                               \
  /* Call the execution policy to register the task */                         \
  bool task_t ## _task_registered =                                            \
    flecsi::execution::task_model_t::register_functor_task<task_t>             \
    (flecsi::execution::task_hash_t::make_key(typeid(task_t).hash_code(),     \
    processor, launch), { EXPAND_AND_STRINGIFY(task_t) })

//----------------------------------------------------------------------------//
//! @def flecsi_register_task
//!
//! This macro registers a user task with the FleCSI runtime.
//!
//! @param task      The task to register. This is normally just a function.
//! @param processor The \ref processor_t type. This may be an \em or list
//!                  of supported processor types.
//! @param launch    The \ref launch_t type. This may be an \em or list of
//!                  supported launch types and configuration options.
//!
//! @ingroup execution
//----------------------------------------------------------------------------//

#define flecsi_register_task(task, processor, launch)                          \
/* MACRO IMPLEMENTATION */                                                     \
                                                                               \
  /* Task return type (trt) */                                                 \
  using task ## _trt_t =                                                       \
    typename flecsi::utils::function_traits__<decltype(task)>::return_type;    \
                                                                               \
  /* Task arguments type (tat) */                                              \
  using task ## _tat_t =                                                       \
    typename flecsi::utils::function_traits__<decltype(task)>::arguments_type; \
                                                                               \
  /* Define a delegate function to the user's function that takes a tuple */   \
  /* of the arguments (as opposed to the raw argument pack) */                 \
  inline task ## _trt_t task ## _tuple_delegate(task ## _tat_t args) {         \
    return flecsi::utils::tuple_function(task, args);                          \
  } /* delegate task */                                                        \
                                                                               \
  /* Call the execution policy to register the task delegate */                \
  bool task ## _task_registered =                                              \
    flecsi::execution::task_model_t::register_task<                            \
      task ## _trt_t, task ## _tat_t, task ## _tuple_delegate,                 \
      flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(task)}.hash()>        \
    (flecsi::execution::task_hash_t::make_key(reinterpret_cast<uintptr_t>(     \
    &task), flecsi::execution::processor, flecsi::execution::launch), { EXPAND_AND_STRINGIFY(task) })

//----------------------------------------------------------------------------//
//! @def flecsi_register_mpi_task
//!
//! This macro registers an MPI task with the FleCSI runtime.
//!
//! @param task The MPI task to register. This is normally just a function.
//!
//! @ingroup execution
//----------------------------------------------------------------------------//

#define flecsi_register_mpi_task(task)                                         \
/* MACRO IMPLEMENTATION */                                                     \
                                                                               \
  flecsi_register_task(task, mpi, index)

//----------------------------------------------------------------------------//
//! @def flecsi_execute_task
//!
//! This macro executes a user task.
//!
//! @param task The user task to execute.
//! @param processor The processor type on which to execute the task.
//! @param launch The launch  mode for the task.
//! @param ... The arguments to pass to the user task during execution.
//!
//! @ingroup execution
//----------------------------------------------------------------------------//

#define flecsi_execute_task(task, processor, launch, ...)                      \
/* MACRO IMPLEMENTATION */                                                     \
                                                                               \
  /* Execute the user task */                                                  \
  /* WARNING: This macro returns a future. Don't add terminations! */          \
  flecsi::execution::task_model_t::execute_task<                               \
    typename flecsi::utils::function_traits__<decltype(task)>::return_type     \
  >                                                                            \
  (                                                                            \
    task_hash_t::make_key(                                                     \
      reinterpret_cast<uintptr_t>(&task), processor, launch                    \
    ),                                                                         \
    flecsi::utils::const_string_t{__func__}.hash(),                            \
    ## __VA_ARGS__                                                             \
  )

//----------------------------------------------------------------------------//
//! @def flecsi_execute_mpi_task
//!
//! This macro executes an MPI task.
//!
//! @param task The MPI task to execute.
//! @param ... The arguments to pass to the MPI task during execution.
//!
//! @ingroup execution
//----------------------------------------------------------------------------//

#define flecsi_execute_mpi_task(task, ...)                                     \
/* MACRO IMPLEMENTATION */                                                     \
                                                                               \
  flecsi_execute_task(task, mpi, index, #__VA_ARGS__)

//----------------------------------------------------------------------------//
// Function Interface
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
//! @def flecsi_register_function
//!
//! This macro registers a user function with the FleCSI runtime, which may
//! then be passed as state data and executed in any task address space.
//!
//! @param name The function to register. This should be the plain-text
//!             name of the function (not a string).
//!
//! @ingroup execution
//----------------------------------------------------------------------------//

#define flecsi_register_function(func)                                         \
/* MACRO IMPLEMENTATION */                                                     \
                                                                               \
  /* Function return type (frt) */                                             \
  using func ## _frt_t =                                                       \
    typename flecsi::utils::function_traits__<decltype(func)>::return_type;    \
                                                                               \
  /* Function arguments type (fat) */                                          \
  using func ## _fat_t =                                                       \
    typename flecsi::utils::function_traits__<decltype(func)>::arguments_type; \
                                                                               \
  /* Define a delegate function to the user's function that takes a tuple */   \
  /* of the arguments (as opposed to the raw argument pack) */                 \
  inline func ## _frt_t func ## _tuple_delegate(func ## _fat_t args) {         \
    return flecsi::utils::tuple_function(func, args);                          \
  } /* delegate function */                                                    \
                                                                               \
  using function_handle_ ## func ## _t =                                       \
    flecsi::execution::function_handle__<func ## _frt_t, func ## _fat_t>;      \
                                                                               \
  /* Call the execution policy to register the function delegate */            \
  bool func ## _func_registered =                                              \
    flecsi::execution::function_t::register_function<                          \
      func ## _frt_t, func ## _fat_t, func ## _tuple_delegate,                 \
      flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(func)}.hash()>()


//----------------------------------------------------------------------------//
//! @def flecsi_execute_function
//!
//! Execute a user function.
//!
//! @param handle The function handle.
//! @param ... The function arguments.
//!
//! @ingroup execution
//----------------------------------------------------------------------------//

#define flecsi_execute_function(handle, ...)                                   \
/* MACRO IMPLEMENTATION */                                                     \
                                                                               \
  /* Call the execution policy to execute the function */                      \
  flecsi::execution::function_t::execute_function(handle, ## __VA_ARGS__)

//----------------------------------------------------------------------------//
//! @def flecsi_function_handle
//!
//! Create a function handle.
//!
//! @param func The function name.
//!
//! @ingroup execution
//----------------------------------------------------------------------------//
#define flecsi_function_handle(func)                                           \
/* MACRO IMPLEMENTATION */                                                     \
                                                                               \
  /* Create a function handle instance */                                      \
  function_handle_ ## func ## _t(                                              \
    flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(func)}.hash())

//----------------------------------------------------------------------------//
//! @def flecsi_define_function_type
//!
//! Define a function handle type.
//!
//! @param func The type name to define.
//! @param return_type The return type of the function.
//! @param ... The input parameters to the function.
//!
//! @ingroup execution
//----------------------------------------------------------------------------//

#define flecsi_define_function_type(func, return_type, ...)                    \
/* MACRO IMPLEMENTATION */                                                     \
                                                                               \
  /* Define a function handle type */                                          \
  using func = flecsi::execution::function_handle__<return_type,               \
    std::tuple<__VA_ARGS__>>

//----------------------------------------------------------------------------//
// Kernel Interface
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
//! @def flecsi_for_each
//!
//! Kernel abstraction interface for data-parallel execution.
//!
//! @param index The name of the counter to use, e.g., \em cnt.
//! @param index_space A valid \ref index_space_t instance.
//! @param kernel The kernel logic to execution.
//!
//! Code Example:
//! @code{.cpp}
//! // Print the id of each mesh cell.
//!
//! flecsi_for_each(c, mesh.cells(), {
//!   std::cout << c.id() << std::endl;
//! }); // flecsi_for_each
//! @endcode
//!
//! @ingroup execution
//----------------------------------------------------------------------------//

#define flecsi_for_each(index, index_space, kernel)                            \
/* MACRO IMPLEMENTATION */                                                     \
                                                                               \
  /* Call the execution policy for_each function */                            \
  flecsi::execution::for_each__(index_space, [&](auto * index) kernel)

//----------------------------------------------------------------------------//
//! @def flecsi_reduce_each
//!
//! Kernel abstraction interface for data-parallel reductions.
//!
//! @param index The name of the counter to use, e.g., \em cnt.
//! @param index_space A valid \ref index_space_t instance.
//! @param variable The variable in which to store the result.
//! @param kernel The kernel logic to execution.
//!
//! @ingroup execution
//----------------------------------------------------------------------------//

#define flecsi_reduce_each(index, index_space, variable, kernel)               \
/* MACRO IMPLEMENTATION */                                                     \
                                                                               \
  /* Call the execution policy reduce_each function */                         \
  flecsi::execution::reduce_each__(index_space, variable,                      \
    [&](auto * index, auto & variable) kernel)

#endif // flecsi_execution_execution_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
