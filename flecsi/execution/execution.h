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

#if !defined(__FLECSI_PRIVATE__)
#define __FLECSI_PRIVATE__
#endif

#include <flecsi/execution/context.h>
#include <flecsi/execution/internal.h>
#include <flecsi/execution/reduction.h>

/*----------------------------------------------------------------------------*
  Basic runtime interface
 *----------------------------------------------------------------------------*/

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

/*----------------------------------------------------------------------------*
  Global object interface.
 *----------------------------------------------------------------------------*/

/*!
  @def flecsi_add_global_object

  Add a global object to the context. Global objects cannot be added
  from within a task. Attempts to do so will generate a runtime error.

  @param index  The size_t index of the global object within the given scope.
  @param scope  The string scope of the global object.
  @param type   The C++ type of the global object.
  @param ...    A variadic argument list of the runtime arguments to the
                constructor.

  @ingroup execution
 */

#define flecsi_add_global_object(index, scope, type, ...)                      \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  flecsi::execution::context_t::instance()                                     \
    .template add_global_object<flecsi_internal_string_hash(scope), type>(     \
      index, ##__VA_ARGS__);

/*!
  @def flecsi_get_global_object

  Get a global object instance.

  @param index  The size_t index of the global object within the given scope.
  @param scope  The string scope of the global object.
  @param type   The type of the global object.

  @ingroup execution
 */

#define flecsi_get_global_object(index, scope, type)                           \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  flecsi::execution::context_t::instance()                                     \
    .template get_global_object<flecsi_internal_string_hash(scope), type>(     \
      index);

//----------------------------------------------------------------------------//
// Task Registration Interface
//----------------------------------------------------------------------------//

/*!
  @def flecsi_register_task

  This macro registers a user task with the FleCSI runtime. This interface
  requires that the task be scoped in a namespace. This is best practice to
  avoid the possiblity of naming collisions.

  @param task      The task to register. This is normally just a function.
  @param nspace    The enclosing C++ namespace of the task.
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
    return flecsi::utils::tuple_function(task, args);                          \
  } /* delegate task */                                                        \
                                                                               \
  /* Call the execution policy to register the task delegate */                \
  inline bool task##_task_registered =                                         \
    flecsi::execution::task_interface_t::register_task<flecsi_internal_hash(   \
                                                         nspace::task),        \
      flecsi_internal_return_type(task),                                       \
      flecsi_internal_arguments_type(task),                                    \
      task##_tuple_delegate>(flecsi::processor,                                \
      flecsi::launch,                                                          \
      {flecsi_internal_stringify(nspace::task)})

//----------------------------------------------------------------------------//
// Task Execution Interface
//----------------------------------------------------------------------------//

/*!
  @def flecsi_execute_task

  This macro executes a user task.

  @param task   The user task to execute.
  @param nspace The enclosing C++ namespace of the task.
  @param launch The launch mode for the task.
  @param ...    The arguments to pass to the user task during execution.

  @ingroup execution
 */

#define flecsi_execute_task(task, nspace, launch, ...)                         \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  /* Execute the user task */                                                  \
  flecsi_internal_execute_task(nspace::task, launch, 0, ##__VA_ARGS__)
