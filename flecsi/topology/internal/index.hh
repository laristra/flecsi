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
#error Do not include this file directly!
#endif

namespace flecsi {
namespace topology {

/*!
  The \c index type allows users to register data on an
  arbitrarily-sized set of indices that have an implicit one-to-one coloring.

  @ingroup topology
 */

struct index {

  index() = delete;

  struct coloring {
    coloring(size_t size) : size_(size) {}

    size_t size() const {
      return size_;
    }

  private:
    size_t size_;
  };

  static coloring color(size_t size) {
    return {size};
  } // color

}; // struct index

} // namespace topology
} // namespace flecsi
