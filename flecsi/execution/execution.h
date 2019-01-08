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
