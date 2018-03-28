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

  struct global_data_client_t :
  flecsi::topology::global_topology__
  {};

  struct color_data_client_t :
  flecsi::topology::color_topology__
  {};


} // namespace data
} // namespace flecsi
