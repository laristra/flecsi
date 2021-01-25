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

  // HPX does not have support for mixed-type reductions
  static_assert(std::is_same_v<lhs_t, rhs_t>, "type mismatch: LHS != RHS");

  /*!
    Wrapper to convert the type-erased HPX function to the typed C++ method.
   */

  static void
  hpx_wrapper(void * in, void * inout, int * len, MPI_Datatype * dptr) {

    lhs_t * lhs = reinterpret_cast<lhs_t *>(inout);
    rhs_t * rhs = reinterpret_cast<rhs_t *>(in);

    for(size_t i{0}; i < *len; ++i) {
      TYPE::apply(lhs[i], rhs[i]);
    } // for
  } // hpx_wrapper

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
    MPI_Op_create(hpx_wrapper, true, &mpiop);
    reduction_ops[HASH] = mpiop;
  } // registration_callback

}; // struct reduction_wrapper_u

//----------------------------------------------------------------------------//
// Min
//----------------------------------------------------------------------------//

/*!
  Minimum reduction type.
 */

template<typename T>
struct min {

  using LHS = T;
  using RHS = T;
  static constexpr T identity{(std::numeric_limits<T>::max)()};

  template<bool EXCLUSIVE = true>
  static void apply(LHS & lhs, RHS rhs) {
    if constexpr(EXCLUSIVE) {
      lhs = lhs < rhs ? lhs : rhs;
    }
    else {
      int64_t * target = (int64_t *)&lhs;
      union
      {
        int64_t as_int;
        T as_T;
      } oldval, newval;
      do {
        oldval.as_int = *target;
        newval.as_T = (std::min)(oldval.as_T, rhs);
      } while(
        !__sync_bool_compare_and_swap(target, oldval.as_int, newval.as_int));

    } // if constexpr

  } // apply

  template<bool EXCLUSIVE = true>
  static void fold(RHS & rhs1, RHS rhs2) {

    if constexpr(EXCLUSIVE) {
      rhs1 = (std::min)(rhs1, rhs2);
    }
    else {
      int64_t * target = (int64_t *)&rhs1;
      union
      {
        int64_t as_int;
        T as_T;
      } oldval, newval;
      do {
        oldval.as_int = *target;
        newval.as_T = (std::min)(oldval.as_T, rhs2);
      } while(
        !__sync_bool_compare_and_swap(target, oldval.as_int, newval.as_int));
    } // if constexpr

  } // fold

}; // struct min

namespace reduction {
//----------------------------------------------------------------------------//
// Max
//----------------------------------------------------------------------------//

/*!
  Maximum reduction type.
 */

template<typename T>
struct max {
  using LHS = T;
  using RHS = T;
  static constexpr T identity{(std::numeric_limits<T>::max)()};

  template<bool EXCLUSIVE = true>
  static void apply(LHS & lhs, RHS rhs) {
    if constexpr(EXCLUSIVE) {
      lhs = lhs > rhs ? lhs : rhs;
    }
    else {
      int64_t * target = (int64_t *)&lhs;
      union
      {
        int64_t as_int;
        T as_T;
      } oldval, newval;
      do {
        oldval.as_int = *target;
        newval.as_T = (std::max)(oldval.as_T, rhs);
      } while(
        !__sync_bool_compare_and_swap(target, oldval.as_int, newval.as_int));

    } // if constexpr

  } // apply

  template<bool EXCLUSIVE = true>
  static void fold(RHS & rhs1, RHS rhs2) {

    if constexpr(EXCLUSIVE) {
      rhs1 = (std::max)(rhs1, rhs2);
    }
    else {
      int64_t * target = (int64_t *)&rhs1;
      union
      {
        int64_t as_int;
        T as_T;
      } oldval, newval;
      do {
        oldval.as_int = *target;
        newval.as_T = (std::max)(oldval.as_T, rhs2);
      } while(
        !__sync_bool_compare_and_swap(target, oldval.as_int, newval.as_int));
    } // if constexpr

  } // fold
}; // struct max

//----------------------------------------------------------------------------//
// Sum
//----------------------------------------------------------------------------//

/*!
  Sum reduction type.
 */

template<typename T>
struct sum {

  using LHS = T;
  using RHS = T;
  static constexpr T identity{};

  template<bool EXCLUSIVE = true>
  static void apply(LHS & lhs, RHS rhs) {

    if constexpr(EXCLUSIVE) {
      lhs += rhs;
    }
    else {
      int64_t * target = (int64_t *)&lhs;
      union
      {
        int64_t as_int;
        T as_T;
      } oldval, newval;
      do {
        oldval.as_int = *target;
        newval.as_T = oldval.as_T + rhs;
      } while(
        !__sync_bool_compare_and_swap(target, oldval.as_int, newval.as_int));

    } // if constexpr
  } // apply

  template<bool EXCLUSIVE = true>
  static void fold(LHS & lhs, RHS rhs) {
    if constexpr(EXCLUSIVE) {
      lhs += rhs;
    }
    else {
      int64_t * target = (int64_t *)&lhs;
      union
      {
        int64_t as_int;
        T as_T;
      } oldval, newval;
      do {
        oldval.as_int = *target;
        newval.as_T = oldval.as_T + rhs;
      } while(
        !__sync_bool_compare_and_swap(target, oldval.as_int, newval.as_int));
    } // if constexpr

  } // fold

}; // struct sum

/*!
  Product reduction type.
 */

//----------------------------------------------------------------------------//
// Product
//----------------------------------------------------------------------------//

template<typename T>
struct product {
  using LHS = T;
  using RHS = T;
  static constexpr T identity{};

  template<bool EXCLUSIVE = true>
  static void apply(LHS & lhs, RHS rhs) {

    if constexpr(EXCLUSIVE) {
      lhs *= rhs;
    }
    else {
      int64_t * target = (int64_t *)&lhs;
      union
      {
        int64_t as_int;
        T as_T;
      } oldval, newval;
      do {
        oldval.as_int = *target;
        newval.as_T = oldval.as_T * rhs;
      } while(
        !__sync_bool_compare_and_swap(target, oldval.as_int, newval.as_int));

    } // if constexpr
  } // apply

  template<bool EXCLUSIVE = true>
  static void fold(LHS & lhs, RHS rhs) {
    if constexpr(EXCLUSIVE) {
      lhs *= rhs;
    }
    else {
      int64_t * target = (int64_t *)&lhs;
      union
      {
        int64_t as_int;
        T as_T;
      } oldval, newval;
      do {
        oldval.as_int = *target;
        newval.as_T = oldval.as_T * rhs;
      } while(
        !__sync_bool_compare_and_swap(target, oldval.as_int, newval.as_int));
    } // if constexpr

  } // fold
}; // struct product

} // namespace reduction
} // namespace execution
} // namespace flecsi
