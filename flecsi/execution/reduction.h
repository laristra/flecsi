#pragma once

/*! @file */

#include <limits>

#include <flecsi/execution/execution.h>

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

#if defined(FLECSI_ENABLE_LEGION)
// Legion has a number of reduction operations built in.  IF we're using legion,
// lets use it' reductions
template<typename T>
using sum = Legion::SumReduction<T>;
template<typename T>
using min = Legion::MinReduction<T>;
template<typename T>
using max = Legion::MaxReduction<T>;
template<typename T>
using product = Legion::ProdReduction<T>;
// TODO:  Legion Provides additonal reductions that we're not using:
//	diff, div, or, and, xor.
#else

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
  static constexpr T identity{std::numeric_limits<T>::max()};

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
        newval.as_T = std::min(oldval.as_T, rhs);
      } while(
        !__sync_bool_compare_and_swap(target, oldval.as_int, newval.as_int));

    } // if constexpr

  } // apply

  template<bool EXCLUSIVE = true>
  static void fold(RHS & rhs1, RHS rhs2) {

    if constexpr(EXCLUSIVE) {
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
      } while(
        !__sync_bool_compare_and_swap(target, oldval.as_int, newval.as_int));
    } // if constexpr

  } // fold

}; // struct min

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
  static constexpr T identity{std::numeric_limits<T>::max()};

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
        newval.as_T = std::max(oldval.as_T, rhs);
      } while(
        !__sync_bool_compare_and_swap(target, oldval.as_int, newval.as_int));

    } // if constexpr

  } // apply

  template<bool EXCLUSIVE = true>
  static void fold(RHS & rhs1, RHS rhs2) {

    if constexpr(EXCLUSIVE) {
      rhs1 = std::max(rhs1, rhs2);
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
        newval.as_T = std::max(oldval.as_T, rhs2);
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

#endif
flecsi_register_operation_types(sum);
flecsi_register_operation_types(min);
flecsi_register_operation_types(max);
flecsi_register_operation_types(product);
} // namespace reduction
} // namespace execution
} // namespace flecsi
