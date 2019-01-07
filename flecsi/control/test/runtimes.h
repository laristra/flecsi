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

#include <flecsi/control/runtime.h>

#include <iostream>

using namespace flecsi::control;

inline int initialize(int argc, char ** argv) {
  std::cout << "Executing initialize" << std::endl;
  return 0;
} // initialize

inline int finalize(int argc, char ** argv, runtime_exit_mode_t mode) {
  std::cout << "Executing finalize with mode " << size_t{mode} << std::endl;
  return 0;
} // initialize

inline bool output(int argc, char ** argv) {
  std::cout << "Executing output" << std::endl;
  return true;
} // output

inline runtime_handler_t handler{ initialize, finalize, output };

flecsi_append_runtime_handler(handler);

inline int runtime_test_runtime_driver(int argc, char ** argv) {
  std::cout << "Executing runtime driver" << std::endl;
  return 0;
} // runtime_test_runtime_driver

flecsi_register_runtime_driver(runtime_test_runtime_driver);
