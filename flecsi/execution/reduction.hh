/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Triad National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

/*! @file */

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly!
#endif

#include "../runtime/backend.hh"

#include <limits>

/*----------------------------------------------------------------------------*
  Reduction Interface
 *----------------------------------------------------------------------------*/

namespace flecsi {
namespace exec::fold {

/*!
  Minimum reduction type.
 */

template<typename T>
struct min {

  using LHS = T;
  using RHS = T;
  static constexpr T identity{std::numeric_limits<T>::max()};

  template<bool EXCLUSIVE>
  static void apply(LHS & lhs, RHS rhs) {
    if constexpr(EXCLUSIVE) {
      lhs = lhs < rhs ? lhs : rhs;
    }
    else {
      int64_t * target = reinterpret_cast<int64_t *>(&lhs);

      union
      {
        int64_t as_int;
        T as_T;
      } oldval, newval;

      do {
        std::memcpy(&oldval.as_int, target, sizeof(int64_t));
        newval.as_T = std::min(oldval.as_T, rhs);
      } while(
        !__sync_bool_compare_and_swap(target, oldval.as_int, newval.as_int));
    } // if
  } // apply

  template<bool EXCLUSIVE>
  static void fold(RHS & rhs1, RHS rhs2) {

    if constexpr(EXCLUSIVE) {
      rhs1 = std::min(rhs1, rhs2);
    }
    else {
      int64_t * target = reinterpret_cast<int64_t *>(&rhs1);

      union
      {
        int64_t as_int;
        T as_T;
      } oldval, newval;

      do {
        std::memcpy(&oldval.as_int, target, sizeof(int64_t));
        newval.as_T = std::min(oldval.as_T, rhs2);
      } while(
        !__sync_bool_compare_and_swap(target, oldval.as_int, newval.as_int));
    } // if
  } // fold

}; // struct min

/*!
  Maximum reduction type.
 */

template<typename T>
struct max {

  using LHS = T;
  using RHS = T;
  static constexpr T identity{std::numeric_limits<T>::min()};

  template<bool EXCLUSIVE = true>
  static void apply(LHS & lhs, RHS rhs) {
    if constexpr(EXCLUSIVE) {
      lhs = lhs > rhs ? lhs : rhs;
    }
    else {
      int64_t * target = reinterpret_cast<int64_t *>(&lhs);

      union
      {
        int64_t as_int;
        T as_T;
      } oldval, newval;

      do {
        std::memcpy(&oldval.as_int, target, sizeof(int64_t));
        newval.as_T = std::max(oldval.as_T, rhs);
      } while(
        !__sync_bool_compare_and_swap(target, oldval.as_int, newval.as_int));
    } // if
  } // apply

  template<bool EXCLUSIVE = true>
  static void fold(RHS & rhs1, RHS rhs2) {

    if constexpr(EXCLUSIVE) {
      rhs1 = std::max(rhs1, rhs2);
    }
    else {
      int64_t * target = reinterpret_cast<int64_t *>(&rhs1);

      union
      {
        int64_t as_int;
        T as_T;
      } oldval, newval;

      do {
        std::memcpy(&oldval.as_int, target, sizeof(int64_t));
        newval.as_T = std::max(oldval.as_T, rhs2);
      } while(
        !__sync_bool_compare_and_swap(target, oldval.as_int, newval.as_int));
    } // if
  } // fold

}; // struct max

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
      int64_t * target = reinterpret_cast<int64_t *>(&lhs);

      union
      {
        int64_t as_int;
        T as_T;
      } oldval, newval;

      do {
        std::memcpy(&oldval.as_int, target, sizeof(int64_t));
        newval.as_T = oldval.as_T + rhs;
      } while(
        !__sync_bool_compare_and_swap(target, oldval.as_int, newval.as_int));
    } // if constexpr

  } // apply

  template<bool EXCLUSIVE = true>
  static void fold(RHS & rhs1, RHS rhs2) {

    if constexpr(EXCLUSIVE) {
      rhs1 += rhs2;
    }
    else {
      int64_t * target = reinterpret_cast<int64_t *>(&rhs1);

      union
      {
        int64_t as_int;
        T as_T;
      } oldval, newval;

      do {
        std::memcpy(&oldval.as_int, target, sizeof(int64_t));
        newval.as_T = oldval.as_T + rhs2;
      } while(
        !__sync_bool_compare_and_swap(target, oldval.as_int, newval.as_int));
    } // if constexpr

  } // fold

}; // struct sum

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

    if constexpr(EXCLUSIVE) {
      lhs *= rhs;
    }
    else {
      int64_t * target = reinterpret_cast<int64_t *>(&lhs);

      union
      {
        int64_t as_int;
        T as_T;
      } oldval, newval;

      do {
        std::memcpy(&oldval.as_int, target, sizeof(int64_t));
        newval.as_T = oldval.as_T * rhs;
      } while(
        !__sync_bool_compare_and_swap(target, oldval.as_int, newval.as_int));
    } // if constexpr

  } // apply

  template<bool EXCLUSIVE = true>
  static void fold(RHS & rhs1, RHS rhs2) {

    if constexpr(EXCLUSIVE) {
      rhs1 *= rhs2;
    }
    else {
      int64_t * target = reinterpret_cast<int64_t *>(&rhs1);

      union
      {
        int64_t as_int;
        T as_T;
      } oldval, newval;

      do {
        std::memcpy(&oldval.as_int, target, sizeof(int64_t));
        newval.as_T = oldval.as_T * rhs2;
      } while(
        !__sync_bool_compare_and_swap(target, oldval.as_int, newval.as_int));
    } // if constexpr

  } // fold

}; // struct product

} // namespace exec::fold
} // namespace flecsi
