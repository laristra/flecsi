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

#include <flecsi/data/data_reference.hh>
#include <flecsi/runtime/types.hh>

namespace flecsi {
namespace data {

template<typename TOPOLOGY_TYPE>
struct coloring_reference : public data_reference_base_t {

  coloring_reference()
    : data_reference_base_t(unique_cid_t::instance().next()) {}

}; // struct coloring_reference

} // namespace data
} // namespace flecsi
