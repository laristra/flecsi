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

#include <functional>
#include <string>
#include <vector>

namespace flecsi {
namespace control {

enum exit_mode_t : size_t {
  success,
  unrecognized_option,
  help
}; // enum exit_mode_t

/*!
  Type to define runtime initialization and finalization handlers.
 */

struct runtime_handler_t {
  std::function<int(int, char **)> initialize;
  std::function<int(int, char **, exit_mode_t)> finalize;
  std::function<bool(int, char **)> output;
}; // struct runtime_handler_t

/*!
 */

struct context_t {

  static context_t & instance() {
    static context_t r;
    return r;
  } // instance

  std::string const & program() const { return program_; }
  std::string & program() { return program_; }

  bool register_driver(std::function<int(int, char **)> const & driver) {
    driver_ = driver;
    return true;
  } // register_driver

  std::function<int(int, char **)> const & driver() const {
    return driver_;
  } // driver

  /*!
    Append the given runtime handler to the vector of handlers. Handlers
    will be executed in the order in which they are appended.
   */
  bool append_runtime_handler(runtime_handler_t const & handler) {
    handlers_.push_back(handler);
    return true;
  } // register_runtime_handler

  /*!
    Access the runtime handler vector.
   */

  std::vector<runtime_handler_t> & runtimes() {
    return handlers_;
  } // runtimes

  /*!
    Invoke runtime intiailzation callbacks.
   */

  void initialize_runtimes(int argc, char ** argv) {
    for(auto r: handlers_) {
      r.initialize(argc, argv);
    } // for
  } // initialize_runtimes

  /*!
    Invoke runtime finalization callbacks.
   */

  int finalize_runtimes(int argc, char ** argv, exit_mode_t mode) {
    int result{0};

    for(auto r: handlers_) {
      result |= r.finalize(argc, argv, mode);
    } // for

    return result;
  } // finalize_runtimes

  /*!
    Return a boolean value indicating whether or not this runtime instance
    should participate in output.
   */

  bool participate_in_output(int argc, char ** argv) {
    bool result{true};

    for(auto r: handlers_) {
      result = r.output(argc, argv) ? result : false;
    } // for

    return result;
  } // participate_in_output

private:

  // FIXME: Make the singleton safe.
  context_t() {}

  std::string program_;
  std::function<int(int, char **)> driver_;
  std::vector<runtime_handler_t> handlers_;

}; // context_t

} // namespace flecsi
} // namespace control
