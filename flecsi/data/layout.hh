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

/*!
  @file

  This file defines data layouts.
 */

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly!
#endif

#include <cstddef>

namespace flecsi {
namespace data {

/*!
  A FleCSI \c layout provides a specific interface for different
  logical data layouts, e.g., dense vs. sparse. The actual data layout
  is implementation-dependent.
 */

enum layout : size_t {
  dense,
  singular, ///< Access to the single element of an array.
  sparse,
  ragged,
  array,
  subspace
};

} // namespace data
} // namespace flecsi
