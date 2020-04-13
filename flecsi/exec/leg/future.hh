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

#if !defined(FLECSI_ENABLE_LEGION)
#error FLECSI_ENABLE_LEGION not defined! This file depends on Legion!
#endif

#include <legion.h>

#include <functional>
#include <iostream>
#include <memory>

namespace flecsi {
namespace exec {

/*!
  Base legion future  type.

  @tparam Return The return type of the task.
  @tparam Launch FleCSI launch type: single/index.

  @ingroup legion-execution
*/
template<typename Return, launch_type_t Launch>
struct legion_future;

/*! Partial specialization for the Legion:Future

  @tparam Return The return type of the task.

  @ingroup legion-execution
 */
template<typename Return>
struct legion_future<Return, launch_type_t::single> {

  /*!
    Wait on a task result.
   */
  void wait() {
    legion_future_.wait();
  } // wait

  /*!
    Get a task result.
   */
  Return get(bool silence_warnings = false) {
    if constexpr(std::is_same_v<Return, void>)
      return legion_future_.get_void_result(silence_warnings);
    else
      return legion_future_.get_result<Return>(silence_warnings);
  } // get

  Legion::Future legion_future_;
}; // legion_future

template<typename Return>
struct legion_future<Return, launch_type_t::index> {

  explicit operator legion_future<Return, launch_type_t::single>() const {
    return {};
  }

  /*!
    Wait on a task result.
  */
  void wait(bool silence_warnings = false) {
    legion_future_.wait_all_results(silence_warnings);
  } // wait

  /*!
    Get a task result.
   */

  Return get(size_t index = 0, bool silence_warnings = false) {
    if constexpr(std::is_same_v<Return, void>)
      return legion_future_.get_void_result(index, silence_warnings);
    else
      return legion_future_.get_result<Return>(
        Legion::DomainPoint::from_point<1>(
          LegionRuntime::Arrays::Point<1>(index)),
        silence_warnings);
  } // get

  Legion::FutureMap legion_future_;

}; // struct legion_future

//-----------------------------------------------------------------------

template<typename Return, launch_type_t Launch = launch_type_t::single>
using flecsi_future = legion_future<Return, Launch>;

template<class>
constexpr bool is_index_future = false;
template<class R>
constexpr bool is_index_future<legion_future<R, launch_type_t::index>> = true;

} // namespace exec
} // namespace flecsi
