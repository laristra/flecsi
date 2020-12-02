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

  This file defines the type identifier type \em reference_base.
 */

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly!
#endif

#include "flecsi/run/types.hh"
#include "flecsi/util/serialize.hh"

namespace flecsi {
namespace data {

struct bind_tag {}; // must be recognized as a task parameter
// Provides a send member function that accepts a function to call with a
// (subsidiary) task parameter and another function to call to transform the
// corresponding task argument (used only on the caller side).
struct send_tag {};

/// An integer used to identify a resource to bind in a task.
struct reference_base : bind_tag {

  reference_base(size_t identifier) : identifier_(identifier) {}

  size_t identifier() const {
    return identifier_;
  } // identifier

protected:
  size_t identifier_;

}; // struct reference_base

} // namespace data
} // namespace flecsi
