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
#include <utility>

namespace flecsi {
namespace utils {

template<typename T, typename U>
inline constexpr T
string_hash(U && str, const std::size_t n) {
  if(n == 0)
    return 0;

  // String-to-integer hash function, based on prime numbers.
  // References:
  //    https://stackoverflow.com/questions/8317508/hash-function-for-a-string
  //    https://planetmath.org/goodhashtableprimes

  const T P = 3145739; // prime
  const T Q = 6291469; // prime, a bit less than 2x the first

  T h = 37; // prime
  for(std::size_t i = 0; i < n; ++i)
    h = (h * P) ^ (str[i] * Q);
  return h;
} // string_hash

} // namespace utils
} // namespace flecsi
