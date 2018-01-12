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

#include <flecsi/coloring/crs.h>

namespace flecsi {
namespace coloring {

/*!
 The colorer_t type provides an interface for creating distributed-memory
 colorings from a distributed, compressed-row storage graph representation.

 @ingroup coloring
 */

struct colorer_t {
  /*!
   This method takes a distributed, compressed-row-storage representation
   of a graph and returns the indepdentent coloring on a per
   execution instance basis, e.g., for each rank or task.
  
   @param dcrs A distributed, compressed-row-storage representation
               of the graph to color.
  
   @return The set of indices that belong to the current execution
           instance.
   */

  virtual std::set<size_t> color(const dcrs_t & dcrs) = 0;

}; // class colorer_t

} // namespace coloring
} // namespace flecsi
