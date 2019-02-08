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

/*!
  @file

  This file defines the base \em client_handler_u type that is customized
  for each specific data client type using partial specialization.
 */

#ifndef POLICY_NAMESPACE
#error You must define a data policy namespace before including this file.
#endif

namespace flecsi {
namespace data {
namespace POLICY_NAMESPACE {

/*!
  The client policy handler type is responsible for extracting
  compile-time information from various data client types, e.g.,
  mesh or tree topologies, for obtaining a client handle and
  populating required fields on the client handle. This class
  should be specialized on the specific client type.
 */

template<typename CLIENT_TYPE>
struct client_handle_specialization_u {};

} // namespace POLICY_NAMESPACE
} // namespace data
} // namespace flecsi
