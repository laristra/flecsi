/*~--------------------------------------------------------------------------~*
 *  @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
 * /@@/////  /@@          @@////@@ @@////// /@@
 * /@@       /@@  @@@@@  @@    // /@@       /@@
 * /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
 * /@@////   /@@/@@@@@@@/@@       ////////@@/@@
 * /@@       /@@/@@//// //@@    @@       /@@/@@
 * /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
 * //       ///  //////   //////  ////////  //
 *
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~--------------------------------------------------------------------------~*/

#pragma once
//! \file
//! \date Initial file creation: Nov 15, 2015
//----------------------------------------------------------------------------//

#include <functional>
#include <memory>

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
// Future concept.
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
//! Abstract interface type for MPI futures.
//!
//! @ingroup legion-execution
//----------------------------------------------------------------------------//
template<
  typename R
>
struct mpi_future__
{
  using result_t = R;

  ///
  /// wait() method
  ///
  void wait() {}

  ///
  /// get() mothod
  ///
  const result_t & get(size_t index = 0) const { return result_; }

//private:

  ///
  /// set method
  ///
  void set(const result_t & result) { result_ = result; }

  result_t result_;

}; // struct mpi_future__

///
///
///
template<>
struct mpi_future__<void>
{
  ///
  ///
  ///
  void wait() {}

}; // struct mpi_future__

} // namespace execution
} // namespace flecsi

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
