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
#include <limits>
#include <type_traits>

namespace flecsi {
namespace util {

/// Simple emulation of std::bit_width from C++20.
template<class T>
constexpr T
bit_width(T x) noexcept {
  static_assert(std::is_unsigned_v<T>);
  T ret = 0, d = std::numeric_limits<T>::digits;
  const auto high = [&] { return x >= T(1) << d - 1; };
  if(high())
    return d; // avoid overwide >>= below
  // Perform a binary search for the first 1.  The static number of iterations
  // makes this significantly faster (at runtime) than a traditional search.
  while(d >>= 1)
    if(high()) {
      x >>= d;
      ret += d;
    }
  return ret;
}

} // namespace util
} // namespace flecsi
