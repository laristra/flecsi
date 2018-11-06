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

namespace flecsi {
namespace utils {

/*!
  Create a C++ type from a non-type value, e.g., an integer literal.

  @tparam T The literal type.
  @tparam M The literal value.

  @ingroup utils
 */

template<typename T, T M>
struct typeify_u {
  using TYPE = T;
  static constexpr T value = M;
};

template<typename T, T M>
constexpr T typeify_u<T, M>::value;

} // namespace utils
} // namespace flecsi
