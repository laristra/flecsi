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
#include "flecsi/utils/mpi_type_traits.h"
#include "flecsi/utils/type_traits.h"

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
    if(request_) {
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
      
  void reduce(MPI_Op op)
  {
    local_result_ = std::make_shared<result_t>(*result_);
    request_ = std::make_shared<MPI_Request>();
    if constexpr (utils::is_container_v<result_t>) {
      using value_t = typename result_t::value_type;
      auto datatype = flecsi::utils::mpi_typetraits_u<value_t>::type();
      MPI_Iallreduce(
        local_result_->data(),
        result_->data(),
        local_result_->size(),
        datatype,
        op,
        MPI_COMM_WORLD,
        request_.get());
    }
    else {
      auto datatype = flecsi::utils::mpi_typetraits_u<result_t>::type();
      MPI_Iallreduce(
        local_result_.get(),
        result_.get(),
        1,
        datatype,
        op,
        MPI_COMM_WORLD,
        request_.get());
    }
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
