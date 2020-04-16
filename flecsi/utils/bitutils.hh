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

#include <cstddef>

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

  This utility is a static implementation of part of the highestOneBit method
  in Java. The runtime version has constant time complexity. I guess this
  has O(0) time complexity, since it is computed by the compiler... ;-)
 */

template<typename TYPE, TYPE _Idx, TYPE _Bits>
constexpr TYPE
msb_shift() {
  constexpr TYPE shift = 1 << _Idx;

  if constexpr(shift < sizeof(TYPE) * 8) {
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
constexpr size_t
msb_place() {
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
constexpr size_t
msb() {
  constexpr size_t msb_bit = msb_shift<size_t, 0, _Bits>();
  return msb_place<((msb_bit + 1) >> 1)>();
} // msb

} // namespace utils
} // namespace flecsi
