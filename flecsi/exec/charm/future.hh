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

#include <flecsi-config.h>

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly!
#endif

#include "flecsi/exec/launch.hh"
#include "flecsi/run/backend.hh"

#if !defined(FLECSI_ENABLE_CHARM)
#error FLECSI_ENABLE_CHARM not defined! This file depends on Charm!
#endif

#include <functional>
#include <iostream>
#include <memory>

namespace flecsi {
namespace exec {

/*!
  Base charm future  type.

  @tparam Return The return type of the task.
  @tparam Launch FleCSI launch type: single/index.

  @ingroup legion-execution
*/
template<typename Return, launch_type_t Launch>
struct charm_future;

template<typename Return, launch_type_t Launch>
struct charm_future {
  charm_future(Return r) : return_(r) {}

  /*!
    Wait on a task result.
  */
  void wait(bool silence_warnings = false) {
    //charm_future_.wait_all_results(silence_warnings);
  } // wait

  /*!
    Get a task result.
   */

  Return get(size_t index = 0, bool silence_warnings = false) {
    return return_;
  } // get

  Return return_;
}; // struct charm_future

/*! Partial specialization for the charm future

  @tparam Return The return type of the task.

  @ingroup legion-execution
 */
template <launch_type_t Launch>
struct charm_future<void, Launch> {

  /*!
    Wait on a task result.
   */
  void wait() {
    //charm_future_.wait();
  } // wait

  /*!
    Get a task result.
   */
  void get(bool silence_warnings = false) {
    return;
  } // get
}; // charm_future

//-----------------------------------------------------------------------


template<typename Return, launch_type_t Launch = launch_type_t::single>
using flecsi_future = charm_future<Return, Launch>;

template<class>
constexpr bool is_index_future = false;
template<class R>
constexpr bool is_index_future<charm_future<R, launch_type_t::index>> = true;

} // namespace exec
} // namespace flecsi
