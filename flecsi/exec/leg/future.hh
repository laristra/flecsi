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

#if !defined(FLECSI_ENABLE_LEGION)
#error FLECSI_ENABLE_LEGION not defined! This file depends on Legion!
#endif

#include <legion.h>

namespace flecsi {

template<typename Return>
struct future<Return> {

  /*!
    Wait on a task result.
   */
  void wait() const {
    legion_future_.wait();
  } // wait

  /*!
    Get a task result.
   */
  Return get(bool silence_warnings = false) const {
    if constexpr(std::is_same_v<Return, void>)
      return legion_future_.get_void_result(silence_warnings);
    else
      return legion_future_.get_result<Return>(silence_warnings);
  } // get

  Legion::Future legion_future_;
};

template<typename Return>
struct future<Return, exec::launch_type_t::index> {

  explicit operator future<Return>() const {
    return {};
  }

  /*!
    Wait on a task result.
  */
  void wait(bool silence_warnings = false) const {
    legion_future_.wait_all_results(silence_warnings);
  } // wait

  /*!
    Get a task result.
   */

  Return get(std::size_t index = 0, bool silence_warnings = false) const {
    if constexpr(std::is_same_v<Return, void>)
      return legion_future_.get_void_result(index, silence_warnings);
    else
      return legion_future_.get_result<Return>(index, silence_warnings);
  } // get

  Legion::FutureMap legion_future_;
};

} // namespace flecsi
