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

#include <tuple>
#include <utility>

namespace flecsi {
namespace utils {

/*!
  Utility to find the most-significant-bit of the given _Bits argument. This
  utility computes the following reccurance relation
  \f$n^{k+1} = n^k | n^k >> 2^k$\f.

  @tparam TYPE  The underlying type of the bit field. This must be an unsigned
                integer type.
  @tparam _Idx  The index counter for the recursion.
  @tparam _Bits The bit field.
 */

template<typename TYPE, TYPE _Idx, TYPE _Bits>
constexpr size_t msb_shift() {
  constexpr size_t shift = 1 << _Idx;
  if constexpr(shift < sizeof(TYPE)*8) {
    return msb_shift<TYPE, _Idx + 1, _Bits>() |
      msb_shift<TYPE, _Idx + 1, _Bits>() >> shift;
  } // if

  return _Bits;
} // msb_shift

/*!
  Utility to find the bit offset of the set bit. This method assumes that
  there is one, and only one, set bit in the field.

  @tparam _Bits The bit field.
 */

template<size_t _Bits>
constexpr size_t msb_place() {
  if constexpr(_Bits > 1) {
    return 1 + msb_place<(_Bits >> 1)>();
  } // if

  return 0;
} // msb_place

/*!
  Return the most-significant-bit of the given _Bits argument.

  @tparam _Bits The bit field.
 */

template<size_t _Bits>
constexpr size_t msb() {
  constexpr size_t msb_bit = msb_shift<size_t, 0, _Bits>();
  return msb_place<((msb_bit + 1) >> 1)>();
} // msb

/*!
  Recursively return or'd bit fields shifted by the width of the field. This
  method is useful for constructing static bit masks.

  @tparam _Idx   Index counter.
  @tparam _Tuple A tuple of size_t bit fields created with
                 flecsi::utils::typeify.
  @tparam _Width The bit width of the fields.
 */

template<size_t _Idx, typename _Tuple, size_t _Width>
constexpr size_t shift_or() {
  if constexpr(_Idx < std::tuple_size_v<_Tuple>) {
    using bit_type_t = typename std::tuple_element<_Idx, _Tuple>::type;

    // The bit_type_t::value depends on the fact that the underlying
    // tuple is created using flecsi::utils::typeify.
    constexpr size_t bits = bit_type_t::value;

    constexpr size_t shift = _Width * ((std::tuple_size_v<_Tuple> - 1) - _Idx);
    return bits << shift | shift_or<_Idx + 1, _Tuple, _Width>();
  }

  return 0;
} // shift_or

} // namespace utils
} // namespace flecsi
