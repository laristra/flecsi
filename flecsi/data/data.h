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

#include <flecsi/utils/const_string.h>

/*!
  @def flecsi_register_topology

  Declare a variable of type \em type with namespace \em nspace
  and name \em name.

  This macro registers a data client with the FleCSI runtime. This call
  does not necessarily cause memory to be allocated. It's primary function
  is to describe the client type to the runtime. Memory allocation will
  likely be deferred.

  @param type   The \ref topology type.
  @param nspace The namespace to use to register the variable.
  @param name   The name to use to register the variable.

  @ingroup data
 */

#define flecsi_register_topology(type, nspace, name)                           \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  /* Call the client interface to register the data */                         \
  inline bool client_type##_##nspace##_##name##_data_client_registered =       \
    flecsi::data::data_client_interface_t::register_data_client<client_type,   \
      flecsi_internal_hash(nspace), flecsi_internal_hash(name)>(               \
      {EXPAND_AND_STRINGIFY(name)})
