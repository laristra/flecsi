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

#include <cinch-config.h>

#include <cinch/runtime.h>

#include <iostream>

#if defined(CINCH_ENABLE_BOOST)
  #include <boost/program_options.hpp>
  using namespace boost::program_options;
#endif

using namespace cinch;

#if defined(CINCH_ENABLE_BOOST)
inline int initialize(int argc, char ** argv, parsed_options & parsed) {
#else
inline int initialize(int argc, char ** argv) {
#endif
  std::cout << "Executing initialize" << std::endl;
  return 0;
} // initialize

inline int finalize(int argc, char ** argv, exit_mode_t mode) {
  std::cout << "Executing finalize with mode " << size_t{mode} << std::endl;
  return 0;
} // initialize

#if defined(CINCH_ENABLE_BOOST)
inline void add_options(options_description & desc) {
  std::cout << "Executing add_options" << std::endl;
} // add_options
#endif

inline runtime_handler_t handler{
  initialize,
  finalize
#if defined(CINCH_ENABLE_BOOST)
  , add_options
#endif
};

cinch_append_runtime_handler(handler);

inline int runtime_test_runtime_driver(int argc, char ** argv) {
  std::cout << "Executing runtime driver" << std::endl;
  return 0;
} // runtime_test_runtime_driver

cinch_register_runtime_driver(runtime_test_runtime_driver);
