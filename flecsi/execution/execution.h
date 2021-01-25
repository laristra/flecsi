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

#include <flecsi/utils/const_string.h>
#include <flecsi/utils/function_traits.h>
#include <flecsi/utils/macros.h>

clog_register_tag(execution);

//----------------------------------------------------------------------------//
// Helper Macros
//----------------------------------------------------------------------------//

#define min_redop_id (size_t(1) << 20) - 4096
#define max_redop_id (size_t(1) << 20) - 4095
/*!
  @def flecsi_internal_hash

  This macro returns the hash of constant string version of the given name.

  @param name The string to hash.

  @ingroup execution
 */

#define flecsi_internal_hash(name)                                             \
  flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(name)}.hash()

/*!
  @def flecsi_internal_return_type

  This macro returns the inferred return type for a user task.

  @param task The task to register. This is normally just a function.

  @ingroup execution
 */

#define flecsi_internal_return_type(task)                                      \
  typename flecsi::utils::function_traits_u<decltype(task)>::return_type

/*!
  @def flecsi_internal_arguments_type

  This macro returns the inferred argument type for a user task.

  @param task The task to register. This is normally just a function.

  @ingroup execution
 */

#define flecsi_internal_arguments_type(task)                                   \
  typename flecsi::utils::function_traits_u<decltype(task)>::arguments_type

//----------------------------------------------------------------------------//
// Top-Level Driver Interface
//----------------------------------------------------------------------------//

/*!
  @def flecsi_register_program

  Register a program. This is currently a place holder for more complex
  operations that may be required in the future to register a multiphysics
  program with the runtime. Currently, this interface simply creates a
  boolean symbol so that an actual translation unit is necessary for
  compilation.

  @param program The program name.

  @ingroup execution
 */

#define flecsi_register_program(program)                                       \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  inline bool flecsi_program_registered = true;

//----------------------------------------------------------------------------//
// Top-Level Driver Interface
//----------------------------------------------------------------------------//

/*!
  @def flecsi_register_top_level_driver

  Register the top level driver function.

  @param driver A std::function<int(int, char **)> that shall be invoked by
                the FLeCSI runtime after initialization. Normally, this
                function should be the \em execute method of a
                flecsi::control::control_u<control_policy_t> instance.

  @ingroup execution
 */

#define flecsi_register_top_level_driver(driver)                               \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  inline bool registered_top_level_driver_##driver =                           \
    flecsi::execution::context_t::instance().register_top_level_driver(driver)

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
  inline bool registered_global_object_##nspace##_##index =                    \
    flecsi::execution::context_t::instance()                                   \
      .template register_global_object<flecsi_internal_hash(nspace), index,    \
        type>();

#define flecsi_register_global_object_at_runtime(index, nspace, type)          \
  flecsi::execution::context_t::instance()                                     \
    .template register_global_object<flecsi_internal_hash(nspace), type>(      \
      index);

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
  flecsi::execution::context_t::instance()                                     \
    .template set_global_object<flecsi_internal_hash(nspace), type>(           \
      index, obj);

/*!
  @def flecsi_initialize_global_object

  Call the constructor of a global object that has been registered with the
  FleCSI runtime. Objects constructed with this call are automatically
  deleted when the runtime exits.

  @param index  The index of the global object within the given namespace.
  @param nspace The namespace of the global object.
  @param type   The type of the global object.
  @param ...    A variadic argument list of the runtime arguments to the
                constructor.

  @ingroup execution
 */

#define flecsi_initialize_global_object(index, nspace, type, ...)              \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  flecsi::execution::context_t::instance()                                     \
    .template initialize_global_object<flecsi_internal_hash(nspace), type>(    \
      index, ##__VA_ARGS__);

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
  flecsi::execution::context_t::instance()                                     \
    .template get_global_object<flecsi_internal_hash(nspace), type>(index);

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
  inline flecsi_internal_return_type(task)                                     \
    task##_tuple_delegate(flecsi_internal_arguments_type(task) args) {         \
    return std::apply(task, std::move(args));                                  \
  } /* delegate task */                                                        \
                                                                               \
  /* Call the execution policy to register the task delegate */                \
  inline bool task##_task_registered =                                         \
    flecsi::execution::task_interface_t::register_task<                        \
      flecsi_internal_hash(task), flecsi_internal_return_type(task),           \
      flecsi_internal_arguments_type(task), task##_tuple_delegate>(            \
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
  inline flecsi_internal_return_type(task)                                     \
    task##_tuple_delegate(flecsi_internal_arguments_type(task) args) {         \
    return std::apply(task, std::move(args));                                  \
  } /* delegate task */                                                        \
                                                                               \
  /* Call the execution policy to register the task delegate */                \
  inline bool task##_task_registered =                                         \
    flecsi::execution::task_interface_t::register_task<                        \
      flecsi_internal_hash(nspace::task), flecsi_internal_return_type(task),   \
      flecsi_internal_arguments_type(task), task##_tuple_delegate>(            \
      flecsi::processor, flecsi::launch, {EXPAND_AND_STRINGIFY(nspace::task)})

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

#define flecsi_internal_execute_task(task, launch, operation, ...)             \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  /* Execute the user task */                                                  \
  /* WARNING: This macro returns a future. Don't add terminations! */          \
  flecsi::execution::task_interface_t::execute_task<                           \
    flecsi::execution::launch_type_t::launch, flecsi_internal_hash(task),      \
    flecsi_internal_hash(operation), flecsi_internal_return_type(task),        \
    flecsi_internal_arguments_type(task)>(__VA_ARGS__)

/*!
  @def flecsi_execute_task_simple

  This macro executes a simple user task, i.e., one that is not scoped in
  a namespace. Use of this interface is discouraged.

  @param task   The user task to execute.
  @param launch The launch mode for the task.
  @param ...    The arguments to pass to the user task during execution.

  @ingroup execution
 */

#define flecsi_execute_task_simple(task, launch, ...)                          \
  /* MACRO IMPLEMENTATION */                                                   \
  flecsi_internal_execute_task(task, launch, 0, ##__VA_ARGS__)

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
  flecsi_internal_execute_task(nspace::task, launch, 0, ##__VA_ARGS__)

/*!
  @def flecsi_execute_mpi_task_simple

  This macro executes a simple MPI task, i.e., one that is not scoped
  in a namespace. Use of this interface is discouraged.

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
  flecsi_execute_task(task, nspace, index, ##__VA_ARGS__).get()

//----------------------------------------------------------------------------//
// Reduction Interface
//----------------------------------------------------------------------------//

/*!
  @def flecsi_register_reduction_operation

  This macro registers a custom reduction rule with the runtime.

  @param type     A type that defines static methods \em apply
                  and \em fold. The \em apply method will be used
                  by the runtime for \em exclusive operations, i.e.,
                  the elements are accessed sequentially. The \em fold
                  method is for \em non-exclusive access.
  @param datatype The data type of the custom reduction.

  @ingroup execution
 */

#define flecsi_register_reduction_operation(type, datatype)                    \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  inline bool type##_##datatype##_reduction_operation_registered =             \
    flecsi::execution::task_interface_t::register_reduction_operation<         \
      flecsi::utils::hash::reduction_hash<flecsi_internal_hash(type),          \
        flecsi_internal_hash(datatype)>(),                                     \
      type<datatype>>()

/*!
  @def flecsi_execute_reduction_task
  This macro executes a reduction task.
  @param task      The user task to execute.
  @param nspace    The enclosing namespace of the task.
  @param launch    The launch mode for the task.
  @param type      The reduction operation type.
  @param datatype  The reduction operation data type.
  @param ...       The arguments to pass to the user task during execution.
  @ingroup execution
 */

#define flecsi_execute_reduction_task(                                         \
  task, nspace, launch, type, datatype, ...)                                   \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  flecsi::execution::task_interface_t::execute_task<                           \
    flecsi::execution::launch_type_t::launch,                                  \
    flecsi_internal_hash(nspace::task),                                        \
    flecsi::utils::hash::reduction_hash<flecsi_internal_hash(type),            \
      flecsi_internal_hash(datatype)>(),                                       \
    flecsi_internal_return_type(nspace::task),                                 \
    flecsi_internal_arguments_type(nspace::task)>(__VA_ARGS__)

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
  inline flecsi_internal_return_type(func)                                     \
    func##_tuple_delegate(flecsi_internal_arguments_type(func) args) {         \
    return std::apply(func, std::move(args));                                  \
  } /* delegate func */                                                        \
                                                                               \
  using function_handle_##func##_t =                                           \
    flecsi::execution::function_handle_u<flecsi_internal_return_type(func),    \
      flecsi_internal_arguments_type(func)>;                                   \
                                                                               \
  /* Call the execution policy to register the function delegate */            \
  inline bool func##_func_registered =                                         \
    flecsi::execution::function_interface_t::register_function<                \
      flecsi_internal_hash(nspace::func), flecsi_internal_return_type(func),   \
      flecsi_internal_arguments_type(func), func##_tuple_delegate>()

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
  nspace::function_handle_##func##_t(flecsi_internal_hash(nspace::func))

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
  using func =                                                                 \
    flecsi::execution::function_handle_u<return_type, std::tuple<__VA_ARGS__>>
