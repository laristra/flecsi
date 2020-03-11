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

#include <cstddef>

#include "flecsi/data/field_info.hh"

namespace flecsi {
namespace data {

template<class>
struct topology_id {};

namespace detail {
struct region {
  region(std::size_t, const fields &) {}
};

struct partition {
  template<class F>
  partition(const region &, std::size_t, F) {}
  std::size_t colors() const {
    return 0;
  }
};
} // namespace detail

} // namespace data
} // namespace flecsi
