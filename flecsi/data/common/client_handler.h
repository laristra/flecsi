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

namespace flecsi {
namespace data {

/*!
  The client policy handler type is responsible for extracting
  compile-time information from various data client types, e.g.,
  mesh or tree topologies, for obtaining a client handle and
  populating required fields on the client handle. This class
  should be specialized on the specific client type.
 */

template<typename DATA_CLIENT>
struct client_handler_u {};

} // namespace data
} // namespace flecsi
