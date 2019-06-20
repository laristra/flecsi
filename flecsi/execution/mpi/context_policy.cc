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

#include <flecsi/execution/context.hh>

namespace flecsi {
namespace execution {

int
mpi_context_policy_t::start(int argc, char ** argv) {

  context_t & context_ = context_t::instance();

  /*
    Register reduction operations.
   */

  for(auto & ro : context_.reduction_registry()) {
    ro.second();
  } // for

  return context_.top_level_action()(argc, argv);
}

} // namespace execution
} // namespace flecsi
