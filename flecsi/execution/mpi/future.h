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

#ifndef flecsi_execution_mpi_future_h
#define flecsi_execution_mpi_future_h

//----------------------------------------------------------------------------//
//! \file
//! \date Initial file creation: Nov 15, 2015
//----------------------------------------------------------------------------//

#include <functional>
#include <memory>

#include "flecsi/execution/future.h"

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
class mpi_future__ : public flecsi_future__<R>
{
public:
  using result_t = R;

  ///
  /// wait() method
  ///
  void wait(bool silence_warnings = false) {}

  ///
  /// get() mothod
  ///
  result_t get(size_t index = 0, bool silence_warnings = false) { return result_; }

  ///
  /// set method
  ///
  void set(const result_t & result) { result_ = result; }

private:

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

#endif // flecsi_execution_mpi_future_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
