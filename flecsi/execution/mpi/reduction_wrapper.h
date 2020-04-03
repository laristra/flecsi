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

#include <type_traits>

#include <flecsi/execution/context.h>
#include <flecsi/utils/mpi_type_traits.h>

clog_register_tag(reduction_wrapper);

namespace flecsi {
namespace execution {

template<size_t HASH, typename TYPE>
struct reduction_wrapper_u {

  using rhs_t = typename TYPE::RHS;
  using lhs_t = typename TYPE::LHS;

  // MPI does not have support for mixed-type reductions
  static_assert(std::is_same_v<lhs_t, rhs_t>, "type mismatch: LHS != RHS");

  /*!
    Wrapper to convert the type-erased MPI function to the typed C++ method.
   */

  static void
  mpi_wrapper(void * in, void * inout, int * len, MPI_Datatype * dptr) {

    lhs_t * lhs = reinterpret_cast<lhs_t *>(inout);
    rhs_t * rhs = reinterpret_cast<rhs_t *>(in);

    for(size_t i{0}; i < *len; ++i) {
      TYPE::apply(lhs[i], rhs[i]);
    } // for
  } // mpi_wrapper

  /*!
    Register the user-defined reduction operator with the runtime.
   */

  static void registration_callback() {
    {
      clog_tag_guard(reduction_wrapper);
      clog(info) << "Executing reduction wrapper callback for " << HASH
                 << std::endl;
    } // scope

    // Get the runtime context
    auto & context_ = context_t::instance();

    // Get a reference to the operator map
    auto & reduction_ops = context_.reduction_operations();

    // Check if operator has already been registered
    clog_assert(reduction_ops.find(HASH) == reduction_ops.end(),
      typeid(TYPE).name() << " has already been registered with this name");

    // Create the operator and register it with the runtime
    MPI_Op mpiop;
    MPI_Op_create(mpi_wrapper, true, &mpiop);
    reduction_ops[HASH] = mpiop;
  } // registration_callback

}; // struct reduction_wrapper_u

} // namespace execution
} // namespace flecsi
