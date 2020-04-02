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

#include "flecsi/execution/launch.hh"
#include "flecsi/runtime/backend.hh"

#if !defined(FLECSI_ENABLE_LEGION)
#error FLECSI_ENABLE_LEGION not defined! This file depends on Legion!
#endif

#include <legion.h>

#include <functional>
#include <iostream>
#include <memory>

namespace flecsi {
namespace execution {

/*!
  Base legion future  type.

  @tparam RETURN The return type of the task.
  @tparam FUTURE The Legion runtime future type.

  @ingroup legion-execution
*/
template<typename RETURN, launch_type_t launch>
struct legion_future {};

/*! Partial specialization for the Legion:Future

  @tparam RETURN The return type of the task.

  @ingroup legion-execution
 */
template<typename RETURN>
struct legion_future<RETURN, launch_type_t::single> {

  /*!
    Wait on a task result.
   */
  void wait(bool = false) {
    legion_future_.wait();
  } // wait

  /*!
    Get a task result.
   */
  RETURN
  get(bool silence_warnings = false) {
    if constexpr(std::is_same_v<RETURN, void>)
      return legion_future_.get_void_result(silence_warnings);
    else
      return legion_future_.template get_result<RETURN>(silence_warnings);
  } // get

  Legion::Future legion_future_;
}; // legion_future

template<typename RETURN>
struct legion_future<RETURN, launch_type_t::index> {

  explicit operator legion_future<RETURN, launch_type_t::single>() const {
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
  RETURN
  get(size_t index = 0, bool silence_warnings = false) {
    if constexpr(std::is_same_v<RETURN, void>)
      return legion_future_.get_void_result(silence_warnings);
    else
      return legion_future_.get_result<RETURN>(
        Legion::DomainPoint::from_point<1>(
          LegionRuntime::Arrays::Point<1>(index)),
        silence_warnings);
  } // get

  Legion::FutureMap legion_future_;

}; // struct legion_future

//-----------------------------------------------------------------------

template<typename RETURN, launch_type_t launch = launch_type_t::single>
using flecsi_future = legion_future<RETURN, launch>;

} // namespace execution
} // namespace flecsi
