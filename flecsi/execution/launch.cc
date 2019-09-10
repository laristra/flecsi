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

#if !defined(__FLECSI_PRIVATE__)
#define __FLECSI_PRIVATE__
#endif

#include "launch.hh"
#include "flecsi/runtime/backend.hh"

namespace flecsi {
namespace execution {

void
set_launch_domain_size(const size_t hash, size_t indices) {
  runtime::context_t::instance().set_launch_domain_size(hash, indices);
} // set_launch_domain_size

} // namespace execution
} // namespace flecsi
