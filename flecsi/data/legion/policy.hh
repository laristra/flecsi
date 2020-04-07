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

#include <flecsi/data/legion/types.hh>
#include <flecsi/flog.hh>
#include <flecsi/runtime/backend.hh>

#if !defined(FLECSI_ENABLE_LEGION)
#error FLECSI_ENABLE_LEGION not defined! This file depends on Legion!
#endif

#include <legion.h>

namespace flecsi {

inline flog::devel_tag topologies_tag("topologies");

namespace data {

template<class C>
struct topology_id {
  // NB: C-style cast supports private inheritance
  topology_id() : id(runtime::context_t::instance().record(*(C *)this)) {}
  topology_id(const topology_id &) : topology_id() {}
  ~topology_id() {
    runtime::context_t::instance().forget(id);
  }
  topology_id & operator=(const topology_id &) noexcept {
    return *this;
  }

  std::size_t id;
};

} // namespace data
} // namespace flecsi
