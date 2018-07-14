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

#include <cinchlog.h>

clog_register_tag(wrapper);

namespace flecsi {
namespace execution {

template<
  size_t NAME,
  typename OPERATION
>
struct reduction_wrapper__ {

  static void registration_callback()
  {
    {
    clog(info) << "Executing reduction wrapper callback for " << NAME <<
      std::endl;
    } // scope

    // Based on the Legion model:
    // OPERATION::LHS
    // OPERATION::RHS
    // OPERATION::identity
    // static void apply(LHS & lhs, RHS rhs)
    // static void fold(LHS & lhs, RHS rhs) (optional)

    // MPI needs to commit the data type if it doesn't exist (We may
    // be able to use type-erasure, i.e., MPI_BYTE)

    // MPI needs to create the operation: MPI_Op_create
    // arguments: function (pointer to function), int commute (boolean)
    // op (reference to fill)

    // State registration may look something like this:
    // context_.register_reduction_operation(NAME, mpi_op, mpi_datatype)

  } // registration_callback

}; // struct reduction_wrapper__

} // namespace execution
} // namespace flecsi
