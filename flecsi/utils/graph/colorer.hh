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

#include <flecsi/utils/crs.hh>

namespace flecsi {
namespace util {
namespace graph {

/*!
 The colorer type provides an interface for creating distributed-memory
 colorings from a distributed, compressed-row storage graph representation.

 @ingroup coloring
 */

struct colorer {
  /*!
   This method takes a distributed, compressed-row-storage representation
   of a graph and returns the indepdentent coloring on a per color basis.

   @param naive An initial distributed, compressed-row-storage representation
                of the graph to color.

   @return The set of indices that belong to the current execution
           instance.
   */

  virtual std::set<size_t> color(const dcrs & naive) = 0;
  virtual std::vector<size_t> new_color(const dcrs & naive) = 0;

}; // class colorer

} // namespace graph
} // namespace util
} // namespace flecsi
