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

#include "context.hh"

using namespace boost::program_options;

namespace flecsi::runtime {

int
context_t::start(int argc, char ** argv, variables_map &) {

  context_t & context_ = context_t::instance();

  /*
    Register reduction operations.
   */

  for(auto & ro : context_.reduction_registry()) {
    ro.second();
  } // for

  return context_.top_level_action()(argc, argv);
}

} // namespace flecsi::runtime
