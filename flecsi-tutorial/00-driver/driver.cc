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

/*----------------------------------------------------------------------------*
  Documentation for this example can be found in README.md.
 *----------------------------------------------------------------------------*/

#include <iostream>

namespace flecsi {
namespace execution {

void
driver(int argc, char ** argv) {

  // Print the message

  std::cout << "Hello World" << std::endl;

  // Print the arguments that were passed on the command line

  for(size_t i{1}; i < argc; ++i) {
    std::cout << "\targ(" << i << "): " << argv[i] << std::endl;
  } // for

} // driver

} // namespace execution
} // namespace flecsi
