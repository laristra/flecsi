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
namespace util {

template<auto Value>
struct constant { // like std::integral_constant, but better
  using type = decltype(Value);
  static constexpr const auto & value = Value;
};

// Non-type template parameters must be fixed-size through C++20, so we must
// use a type to hold an arbitrary amount of information, but there's no need
// to convert each to a type separately.
template<auto... VV>
struct constants {
  static constexpr std::size_t size = sizeof...(VV);
  // NB: not SFINAE-friendly:
  static constexpr
    typename decltype((constant<0>(), ..., constant<VV>()))::type value =
      size == 1 ? (VV, ...) : throw;

private:
  template<auto V, std::size_t... II>
  static constexpr std::size_t find(std::index_sequence<II...>) {
    std::size_t ret = size;
    ((V == VV ? void(ret = II) : void()), ...);
    if(ret < size)
      return ret;
    // otherwise UB which is rejected by the constexpr initialization of index
  }

public:
  // V must be comparable to each of VV.
  template<auto V>
  static constexpr std::size_t index = find<V>(
    std::make_index_sequence<size>());
};

} // namespace util
} // namespace flecsi
