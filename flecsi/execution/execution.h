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
#include <flecsi/execution/reduction.h>
#include <flecsi/execution/task.h>
#include <flecsi/utils/const_string.h>

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

  @param index  The index of the global object within the given namespace.
  @param nspace The namespace of the global object.
  @param type   The type of the global object.
  @param ...    A variadic argument list of the runtime arguments to the
                constructor.

  @ingroup execution
 */

#define flecsi_add_global_object(index, nspace, type, ...)                     \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  flecsi::execution::context_t::instance()                                     \
    .template add_global_object<flecsi_internal_hash(nspace), type>(           \
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
