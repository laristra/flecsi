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

#include <flecsi/execution/context.h>
#include <flecsi/execution/task.h>

/*----------------------------------------------------------------------------*
  Helper interface.
 *----------------------------------------------------------------------------*/

/*!
  @def flecsi_internal_hash

  This macro returns the hash of constant string version of the given name.

  @param name The string to hash.

  @ingroup execution
 */

#define flecsi_internal_hash(name)                                             \
  flecsi::utils::const_string_t{EXPAND_AND_STRINGIFY(name)}.hash()

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
  Reduction Interface
 *----------------------------------------------------------------------------*/

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
      flecsi::utils::hash::reduction_hash<                                     \
        flecsi_internal_hash(type),                                            \
        flecsi_internal_hash(datatype)                                         \
      >(),                                                                     \
      type<datatype>                                                           \
    >()

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

#define flecsi_execute_reduction_task(task, nspace, launch, type,              \
  datatype, ...)                                                               \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  flecsi::execution::task_interface_t::execute_task<                           \
    flecsi::execution::launch_type_t::launch,                                  \
    flecsi_internal_hash(nspace::task),                                        \
    flecsi::utils::hash::reduction_hash<                                       \
      flecsi_internal_hash(type),                                              \
      flecsi_internal_hash(datatype)                                           \
    >(),                                                                       \
    flecsi_internal_return_type(task),                                         \
    flecsi_internal_arguments_type(task)>(__VA_ARGS__)

/*----------------------------------------------------------------------------*
  Global object interface.
 *----------------------------------------------------------------------------*/

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
      .template register_global_object<                                        \
        flecsi_internal_hash(nspace),                                          \
        index,                                                                 \
        type                                                                   \
      >();

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
    .template initialize_global_object<                                        \
      flecsi_internal_hash(nspace),                                            \
      type                                                                     \
    >(index, ##__VA_ARGS__);

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
    .template get_global_object<                                               \
      flecsi_internal_hash(nspace),                                            \
      type                                                                     \
      >(index);

