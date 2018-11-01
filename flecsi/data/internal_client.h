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

#include <flecsi/topology/global_topology.h>

namespace flecsi {
namespace data {

/*!
global_data_client_t is a type used for the global data client registration
*/
struct global_data_client_t : flecsi::topology::global_topology_u {};

/*!
  color_data_client_t is a type used for the color data client registration
*/
struct color_data_client_t : flecsi::topology::color_topology_u {};

} // namespace data
} // namespace flecsi
