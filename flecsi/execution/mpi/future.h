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

template<typename R, launch_type_t launch = launch_type_t::single>
struct mpi_future_u {
  using result_t = R;

  /*!
    wait() method
   */
  void wait() const {
    if (request_) {
      MPI_Status status;
      MPI_Wait(request_.get(), &status);
      request_.reset();
    }
  }

  /*!
    get() mothod
   */
  const result_t & get(size_t index = 0) const {
    wait();
    return *result_;
  }

  // private:

  /*!
    set method
   */
  void set(const result_t & result)
  {
    result_ = std::make_shared<result_t>(result);
  }
  
  void reduce(MPI_Datatype datatype, MPI_Op op)
  {
    local_result_ = std::make_shared<result_t>(*result_);
    request_ = std::make_shared<MPI_Request>();
    MPI_Iallreduce(
        local_result_.get(),
        result_.get(),
        1,
        datatype,
        op,
        MPI_COMM_WORLD,
        request_.get());
  }

  operator R &() {
    wait();
    return *result_;
  }

  operator const R &() const {
    wait();
    return *result_;
  }

  std::shared_ptr<result_t> local_result_;
  std::shared_ptr<result_t> result_;
  mutable std::shared_ptr<MPI_Request> request_;

}; // struct mpi_future_u

/*!
 FIXME documentation
 */
template<launch_type_t launch>
struct mpi_future_u<void, launch> {
  /*!
   FIXME documentation
   */
  void wait() {}

}; // struct mpi_future_u

template<typename RETURN, launch_type_t launch>
using flecsi_future = mpi_future_u<RETURN, launch_type_t::single>;

} // namespace execution
} // namespace flecsi
