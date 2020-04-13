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
#include <memory>

#include "flecsi/exec/launch.hh"

namespace flecsi {
namespace exec {

//----------------------------------------------------------------------------//
// Future concept.
//----------------------------------------------------------------------------//

/*!
 Abstract interface type for MPI futures.

 @ingroup legion-execution
 */
template<typename R, launch_type_t launch = launch_type_t::single>
struct mpi_future {
  using result_t = R;

  /*!
    wait() method
   */
  void wait() {}

  /*!
    get() method
   */
  const result_t & get(size_t /*index*/ = 0) const {
    return result_;
  }

  // private:

  /*!
    set method
   */
  void set(const result_t & result) {
    result_ = result;
  }

  operator R &() {
    return result_;
  }

  operator const R &() const {
    return result_;
  }

  result_t result_;

}; // struct mpi_future

/*!
 FIXME documentation
 */
template<launch_type_t launch>
struct mpi_future<void, launch> {
  /*!
   FIXME documentation
   */
  void wait() {}

}; // struct mpi_future

template<typename RETURN, launch_type_t launch>
using flecsi_future = mpi_future<RETURN, launch>;

} // namespace exec
} // namespace flecsi
