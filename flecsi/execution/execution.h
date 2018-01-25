/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

/*! @file */

#include <functional>

#include <flecsi/execution/common/function_handle.h>
#include <flecsi/execution/common/launch.h>
#include <flecsi/execution/common/processor.h>
#include <flecsi/execution/function.h>
#include <flecsi/execution/kernel.h>
#include <flecsi/execution/task.h>
#include <flecsi/utils/common.h>

clog_register_tag(execution);

//----------------------------------------------------------------------------//
// Helper Macros
//----------------------------------------------------------------------------//

/*!
  @def __flecsi_internal_return_type

  This macro returns the inferred return type for a user task.

  @param task The task to register. This is normally just a function.

  @ingroup execution
 */

#define __flecsi_internal_return_type(task)                                    \
  typename flecsi::utils::function_traits__<decltype(task)>::return_type

/*!
  @def __flecsi_internal_arguments_type

  This macro returns the inferred argument type for a user task.

  @param task The task to register. This is normally just a function.

  @ingroup execution
 */

#define __flecsi_internal_arguments_type(task)                                 \
  typename flecsi::utils::function_traits__<decltype(task)>::arguments_type

//----------------------------------------------------------------------------//
// Object Registration Interface
//----------------------------------------------------------------------------//

/*!
  @def flecsi_register_global_object

  Register a global object with the runtime. A global object must be
  intitialized and mutated consistently by all colors.

  @param index  The index of the global object within the given namespace.
  @param nspace The namespace of the global object.
  @param type   The type of the global object.

  @ingroup execution
 */

#define flecsi_register_global_object(index, nspace, type)                     \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  static bool registered_global_object_ ## nspace ## _ ## index =              \
    context_t::instance().template register_global_object<                     \
      flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(nspace)}.hash(),      \
      index, type>();

/*!
  @def flecsi_set_global_object

  Set the address of a global object that has been registered with the
  FleCSI runtime.

  @param index  The index of the global object within the given namespace.
  @param nspace The namespace of the global object.
  @param type   The type of the global object.
  @param obj    The address of the global object. Normally, this is just
                the pointer to the object, which will be converted into
                a uintptr_t.

  @ingroup execution
 */

#define flecsi_set_global_object(index, nspace, type, obj)                     \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  context_t::instance().template set_global_object<                            \
    flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(nspace)}.hash(),        \
    type>(index, obj);

/*!
  @def flecsi_get_global_object

  Get a global object instance.

  @param index  The index of the global object within the given namespace.
  @param nspace The namespace of the global object.
  @param type   The type of the global object.

  @ingroup execution
 */

#define flecsi_get_global_object(index, nspace, type)                          \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  context_t::instance().template get_global_object<                            \
    flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(nspace)}.hash(),        \
    type>(index);

//----------------------------------------------------------------------------//
// Task Registration Interface
//----------------------------------------------------------------------------//

/*!
  @def flecsi_register_task_simple

  This macro registers a user task with the FleCSI runtime. This is the
  basic form without support for tasks that are defined within a namespace.
  Best practice is to use the flecsi_register_task interface that requires
  that the task be defined in a namespace to scope the task and avoid naming
  collisions.

  @param task      The task to register. This is normally just a function.
  @param processor The \ref processor_type_t type.
  @param launch    The \ref launch_t type. This may be an \em or list of
                   supported launch types and configuration options.

  @ingroup execution
 */

#define flecsi_register_task_simple(task, processor, launch)                   \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  /* Define a delegate function to the user's function that takes a tuple */   \
  /* of the arguments (as opposed to the raw argument pack). This is */        \
  /* necessary because we cannot infer the argument type without using */      \
  /* a tuple. */                                                               \
  inline __flecsi_internal_return_type(task)                                   \
      task##_tuple_delegate(__flecsi_internal_arguments_type(task) args) {     \
    return flecsi::utils::tuple_function(task, args);                          \
  } /* delegate task */                                                        \
                                                                               \
  /* Call the execution policy to register the task delegate */                \
  bool task##_task_registered =                                                \
      flecsi::execution::task_interface_t::register_task<                      \
          flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(task)}.hash(),    \
          __flecsi_internal_return_type(task),                                 \
          __flecsi_internal_arguments_type(task), task##_tuple_delegate>(      \
          flecsi::processor, flecsi::launch, {EXPAND_AND_STRINGIFY(task)})

/*!
  @def flecsi_register_task

  This macro registers a user task with the FleCSI runtime. This interface
  requires that the task be scoped in a namespace. This is best practice to
  avoid the possiblity of naming collisions.

  @param task      The task to register. This is normally just a function.
  @param nspace    The enclosing namespace of the task.
  @param processor The \ref processor_type_t type.
  @param launch    The \ref launch_t type. This may be an \em or list of
                   supported launch types and configuration options.

  @ingroup execution
 */

#define flecsi_register_task(task, nspace, processor, launch)                  \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  /* Define a delegate function to the user's function that takes a tuple */   \
  /* of the arguments (as opposed to the raw argument pack). This is */        \
  /* necessary because we cannot infer the argument type without using */      \
  /* a tuple. */                                                               \
  inline __flecsi_internal_return_type(task)                                   \
      task##_tuple_delegate(__flecsi_internal_arguments_type(task) args) {     \
    return flecsi::utils::tuple_function(task, args);                          \
  } /* delegate task */                                                        \
                                                                               \
  /* Call the execution policy to register the task delegate */                \
  bool task##_task_registered =                                                \
      flecsi::execution::task_interface_t::register_task<                      \
          flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(nspace::task)}    \
              .hash(),                                                         \
          __flecsi_internal_return_type(task),                                 \
          __flecsi_internal_arguments_type(task), task##_tuple_delegate>(      \
          flecsi::processor, flecsi::launch,                                   \
          {EXPAND_AND_STRINGIFY(nspace::task)})

/*!
  @def flecsi_register_mpi_task_simple

  This macro registers an MPI task with the FleCSI runtime. This is the
  basic form without support for tasks that are defined within a namespace.
  Best practice is to use the flecsi_register_mpi_task interface that requires
  that the task be defined in a namespace to scope the task and avoid naming
  collisions.

  @param task   The MPI task to register. This is normally just a function.

  @ingroup execution
 */

#define flecsi_register_mpi_task_simple(task)                                  \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  flecsi_register_task_simple(task, mpi, index | flecsi::leaf)

/*!
  @def flecsi_register_mpi_task

  This macro registers an MPI task with the FleCSI runtime.

  @param task   The MPI task to register. This is normally just a function.
  @param nspace The enclosing namespace of the task.

  @ingroup execution
 */

#define flecsi_register_mpi_task(task, nspace)                                 \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  flecsi_register_task(task, nspace, mpi, index | flecsi::leaf)

//----------------------------------------------------------------------------//
// Task Execution Interface
//----------------------------------------------------------------------------//

/*!
  @def flecsi_color

  Return the index of the currently executing color.

  @ingroup execution
 */

#define flecsi_color()                                                         \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  flecsi::execution::context_t::instance().color()

/*!
  @def flecsi_colors

  Return the number of colors in the currently executing code.

  @ingroup execution
 */

#define flecsi_colors()                                                        \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  flecsi::execution::context_t::instance().colors()

/*!
  @def flecsi_execute_task_simple

  This macro executes a simple user task, i.e., one that is not scoped in
  a namespace.

  @param task   The user task to execute.
  @param launch The launch mode for the task.
  @param ...    The arguments to pass to the user task during execution.

  @ingroup execution
 */

#define flecsi_execute_task_simple(task, launch, ...)                          \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  /* Execute the user task */                                                  \
  /* WARNING: This macro returns a future. Don't add terminations! */          \
  flecsi::execution::task_interface_t::execute_task<                           \
      flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(task)}.hash(),        \
      __flecsi_internal_return_type(task),                                     \
      __flecsi_internal_arguments_type(task)>(                                 \
      flecsi::execution::mask_to_type(flecsi::launch), ##__VA_ARGS__)

/*!
  @def flecsi_execute_task

  This macro executes a user task.

  @param task   The user task to execute.
  @param nspace The enclosing namespace of the task.
  @param launch The launch mode for the task.
  @param ...    The arguments to pass to the user task during execution.

  @ingroup execution
 */

#define flecsi_execute_task(task, nspace, launch, ...)                         \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  /* Execute the user task */                                                  \
  flecsi_execute_task_simple(nspace::task, launch, ##__VA_ARGS__)

/*!
  @def flecsi_execute_mpi_task_simple

  This macro executes a simple MPI task, i.e., one that is not scoped
  in a namespace.

  @param task The MPI task to execute.
  @param ...  The arguments to pass to the MPI task during execution.

  @ingroup execution
 */

#define flecsi_execute_mpi_task_simple(task, ...)                              \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  flecsi_execute_task_simple(task, index, ##__VA_ARGS__)

/*!
  @def flecsi_execute_mpi_task

  This macro executes an MPI task.

  @param task The MPI task to execute.
  @param nspace The enclosing namespace of the task.
  @param ...  The arguments to pass to the MPI task during execution.

  @ingroup execution
 */

#define flecsi_execute_mpi_task(task, nspace, ...)                             \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  flecsi_execute_task(task, nspace, index, ##__VA_ARGS__)

//----------------------------------------------------------------------------//
// Function Interface
//----------------------------------------------------------------------------//

/*!
  @def flecsi_register_function

  This macro registers a user function with the FleCSI runtime, which may
  then be passed as state data and executed in any task address space.

  @param func The function to register. This should be the plain-text

  @ingroup execution
 */

#define flecsi_register_function(func, nspace)                                 \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  /* Define a delegate function to the user's function that takes a tuple */   \
  /* of the arguments (as opposed to the raw argument pack). This is */        \
  /* necessary because we cannot infer the argument type without using */      \
  /* a tuple. */                                                               \
  inline __flecsi_internal_return_type(func)                                   \
      func##_tuple_delegate(__flecsi_internal_arguments_type(func) args) {     \
    return flecsi::utils::tuple_function(func, args);                          \
  } /* delegate func */                                                        \
                                                                               \
  using function_handle_##func##_t =                                           \
      flecsi::execution::function_handle__<                                    \
        __flecsi_internal_return_type(func),                                   \
        __flecsi_internal_arguments_type(func)                                 \
      >;                                                                       \
                                                                               \
  /* Call the execution policy to register the function delegate */            \
  bool func##_func_registered =                                                \
      flecsi::execution::function_interface_t::register_function<              \
        flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(nspace::func)}      \
          .hash(), \
        __flecsi_internal_return_type(func), \
        __flecsi_internal_arguments_type(func), \
        func##_tuple_delegate \
      >()

/*!
  @def flecsi_execute_function

  Execute a user function.

  @param handle The function handle.
  @param ...    The function arguments.

  @ingroup execution
 */

#define flecsi_execute_function(handle, ...)                                   \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  /* Call the execution policy to execute the function */                      \
  flecsi::execution::function_interface_t::execute_function(                   \
      handle, ##__VA_ARGS__)

/*!
  @def flecsi_function_handle

  Create a function handle.

  @param func The function name.

  @ingroup execution
 */

#define flecsi_function_handle(func, nspace)                                   \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  /* Create a function handle instance */                                      \
  nspace::function_handle_##func##_t(                                          \
    flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(nspace::func)}.hash())

/*!
  @def flecsi_define_function_type

  Define a function handle type.

  @param func The type name to define.
  @param return_type The return type of the function.
  @param ... The input parameters to the function.

  @ingroup execution
 */

#define flecsi_define_function_type(func, return_type, ...)                    \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  /* Define a function handle type */                                          \
  using func = flecsi::execution::function_handle__<                           \
      return_type, std::tuple<__VA_ARGS__>>
