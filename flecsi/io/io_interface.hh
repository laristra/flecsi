/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Triad National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

/*!  @file */

template<typename IO_POLICY>
struct io_interface_u {}; // struct io_interface_u

//----------------------------------------------------------------------------//
// This include file defines the FLECSI_RUNTIME_IO_POLICY used below.
//----------------------------------------------------------------------------//

#include <flecsi/runtime/io_policy.hh>

namespace flecsi {
namespace io {

using io_interface_t = io_interface_u<FLECSI_RUNTIME_IO_POLICY>;

} // namespace io
} // namespace flecsi
