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

#include <ostream>
#include <vector>

namespace flecsi {
namespace util {

struct dcrs {
  std::vector<size_t> offsets;
  std::vector<size_t> indices;
  std::vector<size_t> distribution;

#define define_as(member)                                                      \
  template<typename T>                                                         \
  std::vector<T> member##_as() const {                                         \
    return {member.begin(), member.end()};                                     \
  }

  define_as(offsets);
  define_as(indices);
  define_as(distribution);
#undef dcrs_as

  size_t entries() const {
    flog_assert(
      !offsets.empty(), "attempted to call colors() on empty offsets");
    return offsets.size() - 1;
  }

  size_t global_colors() const {
    flog_assert(!distribution.empty(),
      "attempted to call colors() on empty distribution");
    return distribution.size() - 1;
  }

  size_t global_indices() const {
    return distribution.back();
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
