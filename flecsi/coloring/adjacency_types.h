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
namespace coloring {

/*!
 Type for passing adjacency information from the specialization to the
 FleCSI runtime.
 */

struct adjacency_info_t {

  //! The index space of the adjacency information itself.
  size_t index_space;

  //! The index space of the from entities.
  size_t from_index_space;

  //! The index space of the to entities.
  size_t to_index_space;

  //! The size of adjacency array for each color.
  std::vector<size_t> color_sizes;

}; // struct adjacency_info_t

inline std::ostream &
operator<<(std::ostream & stream, const adjacency_info_t & ai) {
  stream << std::endl
         << "index space: " << ai.index_space
         << " from index space: " << ai.from_index_space
         << " to index space: " << ai.to_index_space;

  stream << " color sizes [ ";
  for (auto i : ai.color_sizes) {
    stream << i << " ";
  } // for
  stream << "]";

  return stream;
} // operator <<

} // namespace coloring
} // namespace flecsi
