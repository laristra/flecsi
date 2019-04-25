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
