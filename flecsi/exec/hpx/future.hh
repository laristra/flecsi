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

#if !defined(FLECSI_ENABLE_HPX)
#error FLECSI_ENABLE_HPX not defined! This file depends on Legion!
#endif

#include <hpx/modules/async_combinators.hpp>
#include <hpx/modules/errors.hpp>
#include <hpx/modules/futures.hpp>

#include <vector>

namespace flecsi {

template<typename Return>
struct future<Return> {

  /*!
    Wait on a task result.
   */
  void wait() const {
    hpx_future_.wait();
  } // wait

  /*!
    Get a task result.
   */
  Return get(bool silence_warnings = false) const {
    if(silence_warnings) {
      ::hpx::error_code ec(::hpx::lightwight);
      return hpx_future_.get(ec);
    }
    else {
      return hpx_future_.get();
    }
  } // get

  ::hpx::shared_future<Return> hpx_future_;
};

template<typename Return>
struct future<Return, exec::launch_type_t::index> {
  /*!
    Wait on a task result.
  */
  void wait(bool silence_warnings = false) const {
    ::hpx::wait_all(hpx_futures_);
  } // wait

  /*!
    Get a task result.
   */

  Return get(std::size_t index = 0, bool silence_warnings = false) const {
    if(silence_warnings) {
      ::hpx::error_code ec(::hpx::lightwight);
      return hpx_futures_[index].get(ec);
    }
    else {
      return hpx_futures_[index].get();
    }
  } // get

  std::size_t size() const {
    return hpx_futures_.size();
  }

  std::vector<::hpx::shared_future<Return>> hpx_futures_;
};

} // namespace flecsi
