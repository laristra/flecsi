#pragma once

/*! @file */

#include <limits>

#include <flecsi/execution/execution.h>

namespace flecsi {
namespace execution {
namespace reduction {

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
  static constexpr T identity{};

  template<bool EXCLUSIVE = true>
  static void apply(LHS & lhs, RHS rhs) {

    if constexpr (EXCLUSIVE) {
      lhs = lhs < rhs ? lhs : rhs;
    }
    else {
    } // if constexpr

  } // apply

  template<bool EXCLUSIVE = true>
  static void fold(LHS & lhs, RHS rhs) {

    if constexpr (EXCLUSIVE) {
      lhs = lhs < rhs ? lhs : rhs;
    }
    else {
    } // if constexpr
  } // fold

  static T initial() {
    return T{std::numeric_limits<T>::max()};
  } // initial

}; // struct min

new_flecsi_register_reduction_operation(min, size_t);
new_flecsi_register_reduction_operation(min, float);
new_flecsi_register_reduction_operation(min, double);

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

  static T initial() {
    return T{std::numeric_limits<T>::min()};
  } // initial

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

  static T initial() {
    return T{0};
  } // initial

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

  static T initial() {
    return T{1};
  } // initial

}; // struct product

} // namespace reduction
} // namespace execution
} // namespace flecsi
