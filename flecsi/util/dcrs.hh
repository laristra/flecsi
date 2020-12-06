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

#include "flecsi/flog.hh"

#include <algorithm>
#include <iterator>
#include <ostream>
#include <vector>

namespace flecsi {
namespace util {

template<typename T, typename U>
std::vector<T>
as(std::vector<U> const & v) {
  return {v.begin(), v.end()};
} // as

template<typename ForwardIt, typename T>
auto
distribution_offset(ForwardIt const & distribution, T index) {
  auto it = std::upper_bound(distribution.begin(), distribution.end(), index);
  flog_assert(it != distribution.end(), "index out of range");
  return std::distance(distribution.begin(), it) - 1;
}

struct dcrs {
  std::vector<size_t> offsets;
  std::vector<size_t> indices;
  std::vector<size_t> distribution;

  /*
    Return the number of entries. This is the number of nodes in the graph,
    or the number of rows in the compressed matrix.
   */

  size_t entries() const {
    flog_assert(
      !offsets.empty(), "attempted to call colors() on empty offsets");
    return offsets.size() - 1;
  }

  void clear() {
    offsets.clear();
    indices.clear();
    distribution.clear();
  }
}; // struct dcrs

inline std::ostream &
operator<<(std::ostream & stream, dcrs const & graph) {
  stream << "distribution: ";
  for(auto o : graph.distribution) {
    stream << o << " ";
  }
  stream << std::endl;

  stream << "offsets: ";
  for(auto o : graph.offsets) {
    stream << o << " ";
  }
  stream << std::endl;

  stream << "indices: ";
  for(auto o : graph.indices) {
    stream << o << " ";
  }
  stream << std::endl;

  return stream;
} // operator<<

} // namespace util
} // namespace flecsi
