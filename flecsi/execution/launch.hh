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
#error Do not include this file directly
#endif

#include <cstddef>

namespace flecsi {
namespace execution {

enum class launch_type_t : size_t { single, index };

/// A launch domain with a static identity but a runtime size.
struct launch_domain {
  explicit constexpr launch_domain(std::size_t s = 0) : sz(s) {}
  void size(std::size_t s) {
    sz = s;
  }
  constexpr std::size_t size() const {
    return sz;
  }
  constexpr bool operator==(const launch_domain & o) const {
    return this == &o;
  }
  constexpr bool operator!=(const launch_domain & o) const {
    return !(*this == o);
  }

private:
  std::size_t sz;
};

} // namespace execution

inline constexpr execution::launch_domain single(1), index;

} // namespace flecsi
