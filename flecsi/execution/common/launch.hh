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
#else
#include <flecsi/utils/const_string.hh>
#endif

namespace flecsi {
namespace execution {

enum class launch_type_t : size_t { single, index };

/*!
  Return a launch_identifier from a string literal.
 */

template<size_t CHARACTERS>
inline constexpr size_t
launch_identifier(const char (&str)[CHARACTERS]) {
  return utils::const_string_t(str).hash();
}

void set_launch_domain_size(const size_t identifier, size_t indices);

} // namespace execution

inline constexpr size_t single = execution::launch_identifier("single");
inline constexpr size_t index = execution::launch_identifier("index");

} // namespace flecsi
