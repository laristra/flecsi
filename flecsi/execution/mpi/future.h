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

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
// Future concept.
//----------------------------------------------------------------------------//

/*!
 Abstract interface type for MPI futures.

 @ingroup legion-execution
 */
template<
  typename R,
  typename FUTURE = int
>
struct mpi_future__
{
  using result_t = R;

  /*!
    wait() method
   */
  void wait() {}

  /*!
    get() mothod
   */
  const result_t & get(size_t index = 0) const { return result_; }

//private:

  /*!
    set method
   */
  void set(const result_t & result) { result_ = result; }

  operator R &() {
    return result_;
  }

  operator const R  &() const {
    return result_;
  }

  result_t result_;

}; // struct mpi_future__

/*!
 FIXME documentation
 */
template<typename FUTURE>
struct mpi_future__<void, FUTURE>
{
  /*!
   FIXME documentation
   */
  void wait() {}

}; // struct mpi_future__

template<
    typename RETURN,
    typename FUTURE>
using flecsi_future = mpi_future__<
    RETURN,
    FUTURE>;

template<
    typename RETURN>
using flecsi_single_future = mpi_future__<
    RETURN>;

template<
    typename RETURN>
using flecsi_index_future = mpi_future__<
    RETURN>;

} // namespace execution
} // namespace flecsi
