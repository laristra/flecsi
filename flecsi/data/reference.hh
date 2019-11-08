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

#include <flecsi/runtime/types.hh>
#include <flecsi/utils/serialize.hh>

namespace flecsi {
namespace data {

#if 0
template<typename TOPOLOGY_TYPE>
struct topology_instance;
#endif

/*!
  The reference_base type is the base of all FleCSI data model types.
  It is used to identify FleCSI data model types, and to store basic handle
  information.
 */

struct reference_base {

  reference_base(size_t identifier) : identifier_(identifier) {}

  size_t identifier() const {
    return identifier_;
  } // identifier

protected:
  size_t identifier_;

}; // struct reference_base

} // namespace data
} // namespace flecsi
