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

#include <array>
#include <cstddef>
#include <utility>

namespace flecsi {
namespace util {

template<class...>
struct types {};

template<auto Value>
struct constant { // like std::integral_constant, but better
  using type = decltype(Value);
  static constexpr const auto & value = Value;
};

namespace detail {
template<auto...>
extern void * const first_constant; // undefined
template<auto V, auto... VV>
constexpr const auto & first_constant<V, VV...> = V;
} // namespace detail

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
  static constexpr decltype(detail::first_constant<VV...>) first =
    detail::first_constant<VV...>;

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

// A std::array<T> indexed by the values in a constants<...>.
template<class T, class C>
struct key_array : std::array<T, C::size> {
  using keys = C;

  template<auto V>
  constexpr T & get() {
    return (*this)[C::template index<V>];
  }
  template<auto V>
  constexpr const T & get() const {
    return (*this)[C::template index<V>];
  }
};

} // namespace util
} // namespace flecsi
