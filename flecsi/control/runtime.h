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

#include <flecsi/control/context.h>

/*!
  @def flecsi_register_runtime_driver(driver)

  Register the primary runtime driver function.

  @param driver The primary driver with a 'int(int, char **)' signature
                that should be invoked by the FleCSI runtime.
 */

#define flecsi_register_runtime_driver(driver)                                 \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  inline bool flecsi_registered_driver_##driver =                              \
    flecsi::control::context_t::instance().register_driver(driver)

/*!
  @def flecsi_register_runtime_handler(handler)

  Register a runtime handler with the FleCSI runtime. Runtime handlers
  are invoked at fixed control points in the FleCSI control model for
  initialization, finalization, and output participation. The finalization
  function has an additional argument that specifies the exit mode.

  @param handler A runtime_handler_t that references the appropriate
                 initialize, finalize, and output functions.                
 */

#define flecsi_append_runtime_handler(handler)                                 \
  /* MACRO DEFINITION */                                                       \
                                                                               \
  inline bool flecsi_append_runtime_handler_##handler =                        \
    flecsi::control::context_t::instance().append_runtime_handler(handler)
