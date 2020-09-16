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
template<typename TOPOLOGY_TYPE>
struct topology_slot : convert_tag {
  using data_t = typename TOPOLOGY_TYPE::core;
  using coloring = typename TOPOLOGY_TYPE::coloring;

  data_t & allocate(const coloring & coloring_reference) {
    return data.emplace(coloring_reference);
  } // allocate

  void deallocate() {
    // data.reset();
  } // deallocate

  data_t & get() {
    flog_assert(data, "topology not allocated");
    return *data;
  }
  const data_t & get() const {
    return const_cast<topology_slot &>(*this).get();
  }

  data_t * operator->() {
    return &*data;
  }
  const data_t * operator->() const {
    return &*data;
  }

private:
  std::optional<data_t> data;
}; // struct topology_slot

} // namespace data
} // namespace flecsi
