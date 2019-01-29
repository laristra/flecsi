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

#include <limits>

#include <flecsi/execution/task.h>
#include <flecsi/utils/const_string.h>
#include <flecsi/utils/hash.h>

/*----------------------------------------------------------------------------*
  Reduction Interface
 *----------------------------------------------------------------------------*/

/*!
  @def flecsi_register_reduction_operation

  This macro registers a custom reduction rule with the runtime.

  @param type     A type that defines static methods \em apply
                  and \em fold. The \em apply method will be used
                  by the runtime for \em exclusive operations, i.e.,
                  the elements are accessed sequentially. The \em fold
                  method is for \em non-exclusive access.
  @param datatype The data type of the custom reduction.

  @ingroup execution
 */

#define flecsi_register_reduction_operation(type, datatype)                    \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  inline bool type##_##datatype##_reduction_operation_registered =             \
    flecsi::execution::task_interface_t::register_reduction_operation<         \
      flecsi::utils::hash::reduction_hash<flecsi_internal_hash(type),          \
        flecsi_internal_hash(datatype)>(),                                     \
      type<datatype>>()

/*!
  @def flecsi_execute_reduction_task
  This macro executes a reduction task.
  @param task      The user task to execute.
  @param nspace    The enclosing namespace of the task.
  @param launch    The launch mode for the task.
  @param type      The reduction operation type.
  @param datatype  The reduction operation data type.
  @param ...       The arguments to pass to the user task during execution.
  @ingroup execution
 */

#define flecsi_execute_reduction_task(                                         \
  task, nspace, launch, type, datatype, ...)                                   \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  flecsi::execution::task_interface_t::execute_task<                           \
    flecsi::execution::launch_type_t::launch,                                  \
    flecsi_internal_hash(nspace::task),                                        \
    flecsi::utils::hash::reduction_hash<flecsi_internal_hash(type),            \
      flecsi_internal_hash(datatype)>(),                                       \
    flecsi_internal_return_type(task), flecsi_internal_arguments_type(task)>(  \
    __VA_ARGS__)

namespace flecsi {
namespace execution {
namespace reduction {

#define flecsi_register_operation_types(operation)                             \
  flecsi_register_reduction_operation(operation, int);                         \
  flecsi_register_reduction_operation(operation, long);                        \
  flecsi_register_reduction_operation(operation, short);                       \
  flecsi_register_reduction_operation(operation, unsigned);                    \
  flecsi_register_reduction_operation(operation, size_t);                      \
  flecsi_register_reduction_operation(operation, float);                       \
  flecsi_register_reduction_operation(operation, double);

/*!
  Minimum reduction type.
 */

template<typename T>
struct min {

  using LHS = T;
  using RHS = T;
  static constexpr T identity{ std::numeric_limits<T>::max()};

  template<bool EXCLUSIVE >
  static void apply(LHS & lhs, RHS rhs) {
    if constexpr (EXCLUSIVE) {
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
        newval.as_T = std::min(oldval.as_T, rhs);
      } while(!__sync_bool_compare_and_swap(target, oldval.as_int,
        newval.as_int));
    } // if
  } // apply

  template<bool EXCLUSIVE >
  static void fold(RHS & rhs1, RHS rhs2) {

    if constexpr (EXCLUSIVE) {
      rhs1 = std::min(rhs1, rhs2);
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
        newval.as_T = std::min(oldval.as_T, rhs2);
      } while(!__sync_bool_compare_and_swap(target, oldval.as_int,
        newval.as_int));
    } // if
  } // fold

}; // struct min

flecsi_register_operation_types(min);

/*!
  Maximum reduction type.
 */

template<typename T>
struct max {

  using LHS = T;
  using RHS = T;
  static constexpr T identity{};

  template<bool EXCLUSIVE = true>
  static void apply(LHS & lhs, RHS rhs) {

    if constexpr (EXCLUSIVE) {
      lhs = lhs > rhs ? lhs : rhs;
    }
    else {
    } // if constexpr

  } // apply

  template<bool EXCLUSIVE = true>
  static void fold(LHS & lhs, RHS rhs) {

    if constexpr (EXCLUSIVE) {
      lhs = lhs > rhs ? lhs : rhs;
    }
    else {
    } // if constexpr

  } // fold

}; // struct max

flecsi_register_operation_types(max);

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

    if constexpr (EXCLUSIVE) {
      lhs += rhs;
    }
    else {
    } // if constexpr

  } // apply

  template<bool EXCLUSIVE = true>
  static void fold(LHS & lhs, RHS rhs) {

    if constexpr (EXCLUSIVE) {
      lhs += rhs;
    }
    else {
    } // if constexpr

  } // fold

}; // struct sum

flecsi_register_operation_types(sum);

/*!
  Product reduction type.
 */

template<typename T>
struct product {

  using LHS = T;
  using RHS = T;
  static constexpr T identity{};

  template<bool EXCLUSIVE = true>
  static void apply(LHS & lhs, RHS rhs) {

    if constexpr (EXCLUSIVE) {
      lhs *= rhs;
    }
    else {
    } // if constexpr

  } // apply

  template<bool EXCLUSIVE = true>
  static void fold(LHS & lhs, RHS rhs) {

    if constexpr (EXCLUSIVE) {
      lhs *= rhs;
    }
    else {
    } // if constexpr

  } // fold

}; // struct product

flecsi_register_operation_types(product);

} // namespace reduction
} // namespace execution
} // namespace flecsi
