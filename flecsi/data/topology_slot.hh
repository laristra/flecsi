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

#include "flecsi/flog.hh"
#include <optional>

namespace flecsi {
namespace data {

struct convert_tag {}; // must be recognized as a task argument

/// A slot that holds a topology, constructed upon request.
/// Declare a task parameter as a \c topology_accessor to use the topology.
/// \note A \c specialization provides aliases for both these types.
template<typename Topo>
struct topology_slot : convert_tag {
  using core = typename Topo::core;
  using coloring = typename Topo::coloring;

  template<typename... AA>
  core & allocate(const coloring & coloring_reference, AA &&... aa) {
    data.emplace(coloring_reference);
    Topo::initialize(*this, std::forward<AA>(aa)...);
    return get();
  }

  void deallocate() {
    // data.reset();
  } // deallocate

  core & get() {
    flog_assert(data, "topology not allocated");
    return *data;
  }
  const core & get() const {
    return const_cast<topology_slot &>(*this).get();
  }

  core * operator->() {
    return &*data;
  }
  const core * operator->() const {
    return &*data;
  }

private:
  std::optional<core> data;
}; // struct topology_slot

} // namespace data
} // namespace flecsi
