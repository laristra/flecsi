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

#include <cstddef>

namespace flecsi {
namespace coloring {

template<
  size_t INDEX_SPACE,
  size_t DIMENSION,
  size_t THRU_DIMENSION,
  size_t DEPTH = 1
>
struct primary_independent_u {
  static constexpr size_t index_space = INDEX_SPACE;
  static constexpr size_t dimension = DIMENSION;
  static constexpr size_t thru_dimension = THRU_DIMENSION;
  static constexpr size_t depth = DEPTH;
}; // struct primary_independent_u

template<
  size_t INDEX_SPACE,
  size_t DIMENSION,
  size_t PRIMARY_DIMENSION
>
struct auxiliary_independent_u {
  static constexpr size_t index_space = INDEX_SPACE;
  static constexpr size_t dimension = DIMENSION;
  static constexpr size_t primary_dimension = PRIMARY_DIMENSION;
}; // struct auxiliary_independent_u

} // namespace coloring
} // namespace flecsi
