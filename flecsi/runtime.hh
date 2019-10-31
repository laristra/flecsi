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

/*! @file */

#include <flecsi-config.h>

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly!
#endif 

#include <boost/program_options.hpp>

#if defined(FLECSI_RUNTIME_DEBUG)
#include <iostream>
#endif

#include <functional>
#include <string>
#include <vector>

#if defined(FLECSI_RUNTIME_DEBUG)
#define runtime_function()                                                     \
  std::cout << __FILE__ << " " << __FUNCTION__ << std::endl;
#else
#define runtime_function()
#endif

namespace flecsi {

enum exit_mode_t : size_t {
  success,
  unrecognized_option,
  help,
  option_exit
}; // enum exit_mode_t

using driver_function_t =
  std::function<int(int, char **, boost::program_options::variables_map &)>;

/*!
  Type to define runtime handlers.
 */

struct runtime_handler_t {
  driver_function_t initialize;
  std::function<int(int, char **, exit_mode_t)> finalize;
  std::function<void(boost::program_options::options_description &)>
    add_options = [](boost::program_options::options_description &) {};
}; // struct runtime_handler_t

/*!
  The runtime_t type provides a stateful interface for registering and
  executing user-defined actions at initialization and finalization
  control points.
 */

struct runtime_t {

  static runtime_t & instance() {
    runtime_function();
    static runtime_t r;
    return r;
  } // instance

  std::string const & program() const {
    return program_;
  }
  std::string & program() {
    return program_;
  }

  using output_function_t = std::function<bool()>;

  bool register_driver(driver_function_t const & driver) {
    runtime_function();
    driver_ = driver;
    return true;
  } // register_driver

  driver_function_t const & driver() const {
    runtime_function();
    return driver_;
  } // driver

  bool register_output_driver(output_function_t const & output_driver) {
    runtime_function();
    output_driver_ = output_driver;
    return true;
  } // register_output_driver

  /*!
    Invoke the registered output driver to determine if the underlying
    process should participate in I/O operations.

    This fuction is mostly useful for limiting output to a single process
    for help messages.
   */

  bool join_output() {
    return output_driver_();
  } // join_output

  /*!
    Append the given runtime handler to the vector of handlers. Handlers
    will be executed in the order in which they are appended.
   */

  bool append_runtime_handler(runtime_handler_t const & handler) {
    runtime_function();
    handlers_.push_back(handler);
    return true;
  } // register_runtime_handler

  /*!
    Access the runtime handler vector.
   */

  std::vector<runtime_handler_t> & runtimes() {
    runtime_function();
    return handlers_;
  } // runtimes

  /*!
    Invoke runtime options callbacks.
   */

  void add_options(boost::program_options::options_description & desc) {
    runtime_function();
    for(auto r : handlers_) {
      r.add_options(desc);
    } // for
  } // add_options

  /*!
    Invoke runtime intiailzation callbacks.
   */

  int initialize_runtimes(int argc,
    char ** argv,
    boost::program_options::variables_map & vm) {
    runtime_function();
    int result{0};

    for(auto r : handlers_) {
      result |= r.initialize(argc, argv, vm);
    } // for

    return result;
  } // initialize_runtimes

  /*!
    Invoke runtime finalization callbacks.
   */

  int finalize_runtimes(int argc, char ** argv, exit_mode_t mode) {
    runtime_function();
    int result{0};

    for(auto r : handlers_) {
      result |= r.finalize(argc, argv, mode);
    } // for

    return result;
  } // finalize_runtimes

private:
  runtime_t() {
    runtime_function();
  }

  ~runtime_t() {
    runtime_function();
  }

  // These are deleted because this type is a singleton, i.e.,
  // we don't want anyone to be able to make copies or references.

  runtime_t(const runtime_t &) = delete;
  runtime_t & operator=(const runtime_t &) = delete;
  runtime_t(runtime_t &&) = delete;
  runtime_t & operator=(runtime_t &&) = delete;

  std::string program_;
  driver_function_t driver_;
  output_function_t output_driver_ = {};
  std::vector<runtime_handler_t> handlers_;

}; // runtime_t

} // namespace flecsi

/*!
  @def flecsi_register_runtime_driver(driver)

  Register the primary runtime driver function.

  @param driver The primary driver with a 'int(int, char **)' signature
                that should be invoked by the Cinch runtime.
 */

#define flecsi_register_runtime_driver(driver)                                 \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  inline bool flecsi_registered_driver_##driver =                              \
    flecsi::runtime_t::instance().register_driver(driver)

/*!
  @def flecsi_register_output_driver(driver)

  Register a function to specify which processes should participate in
  I/O operations.

  @param driver The output driver with a 'bool()' signature
                that should be invoked by the Cinch runtime to determine
                which processes should join output operations.
 */

#define flecsi_register_output_driver(driver)                                  \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  inline bool flecsi_registered_output_driver_##driver =                       \
    flecsi::runtime_t::instance().register_output_driver(driver)

/*!
  @def flecsi_register_runtime_handler(handler)

  Register a runtime handler with the FleCSI runtime. Runtime handlers
  are invoked at fixed control points in the FleCSI control model for
  add options, initialization, and finalization. The finalization function
  has an additional argument that specifies the exit mode.

  @param handler A runtime_handler_t that references the appropriate
                 initialize, finalize, and add_options functions.
 */

#define flecsi_append_runtime_handler(handler)                                 \
  /* MACRO DEFINITION */                                                       \
                                                                               \
  inline bool flecsi_append_runtime_handler_##handler =                        \
    flecsi::runtime_t::instance().append_runtime_handler(handler)
