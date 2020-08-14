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

template<typename Return>
struct future<Return, exec::launch_type_t::single> {

  /*!
    Wait on a task result.
  */
  void wait(bool silence_warnings = false) const {
  } // wait

  /*!
    Get a task result.
   */
  Return get(bool silence_warnings = false) const {
    //return return_;
    return return_;
  } // get

  Return return_;
}; // struct future

template<>
struct future<void, exec::launch_type_t::single> {

  /*!
    Wait on a task result.
  */
  void wait(bool silence_warnings = false) const {
  } // wait

  /*!
    Get a task result.
   */
  void get(bool silence_warnings = false) const {
    return;
  } // get
}; // struct future

template <typename Return>
struct future<Return, exec::launch_type_t::index> {

  /*!
    Wait on a task result.
   */
  void wait() const {
  } // wait

  /*!
    Get a task result.
   */
  Return get(std::size_t index = 0, bool silence_warnings = false) const {
    return return_;
  } // get

  std::size_t size() const { return 1; }

  Return return_;
}; // future

template <>
struct future<void, exec::launch_type_t::index> {

  /*!
    Wait on a task result.
   */
  void wait() const {
  } // wait

  /*!
    Get a task result.
   */
  void get(std::size_t index = 0, bool silence_warnings = false) const {
    return;
  } // get

  std::size_t size() const { return 1; }

}; // future

//-----------------------------------------------------------------------

} // namespace flecsi
