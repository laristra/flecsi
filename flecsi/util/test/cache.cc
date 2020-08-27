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
#define __FLECSI_PRIVATE__

#include "flecsi/util/cache.hh"
#include "flecsi/util/unit.hh"

template<typename Type>
struct device {
  std::vector<Type> fetch(std::size_t offset, std::size_t count) {
    (void)offset;
    (void)count;
    return {};
  }
};

using data = flecsi::util::cache_impl::data<1024, 64>;
// using cache = flecsi::util::cache<std::size_t, std::size_t,
// device<std::size_t>, 2048, 128, flecsi::util::cache::fifo>;

int
cache_driver() {
  UNIT {};
} // cache_driver

flecsi::unit::driver<cache_driver> driver;
