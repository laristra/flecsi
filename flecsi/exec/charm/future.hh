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

/*! Partial specialization for the charm future

  @tparam Return The return type of the task.

  @ingroup legion-execution
 */
template<typename Return>
struct charm_future<Return, launch_type_t::single> {

  /*!
    Wait on a task result.
   */
  void wait() {
    //charm_future_.wait();
  } // wait

  /*!
    Get a task result.
   */
  Return get(bool silence_warnings = false) {
    if constexpr(std::is_same_v<Return, void>)
      //return charm_future_.get_void_result(silence_warnings);
      return;
    else
      return Return();
      //return charm_future_.get_result<Return>(silence_warnings);
  } // get

  //Legion::Future charm_future_;
}; // charm_future

template<typename Return>
struct charm_future<Return, launch_type_t::index> {

  explicit operator charm_future<Return, launch_type_t::single>() const {
    return {};
  }

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
    if constexpr(std::is_same_v<Return, void>)
      //return charm_future_.get_void_result(index, silence_warnings);
      return;
    else
      //return charm_future_.get_result<Return>(
      //  Legion::DomainPoint::from_point<1>(
      //    LegionRuntime::Arrays::Point<1>(index)),
      //  silence_warnings);
      return Return();
  } // get

  //Legion::FutureMap charm_future_;

}; // struct charm_future

//-----------------------------------------------------------------------


template<typename Return, launch_type_t Launch = launch_type_t::single>
using flecsi_future = charm_future<Return, Launch>;

template<class>
constexpr bool is_index_future = false;
template<class R>
constexpr bool is_index_future<charm_future<R, launch_type_t::index>> = true;

} // namespace exec
} // namespace flecsi
