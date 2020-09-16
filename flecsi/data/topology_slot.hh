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
    return *data;
  }
  const data_t & get() const {
    return *data;
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

// Non-optional slot wrapper for use in other topology types.
template<class T>
struct anti_slot {
  using Slot = topology_slot<T>;
  anti_slot(const typename Slot::coloring & c) {
    s.allocate(c);
  }

  auto & operator*() const {
    return s.get();
  }
  auto & operator*() {
    return s.get();
  }
  auto operator-> () const {
    return &**this;
  }
  auto operator-> () {
    return &**this;
  }

  const Slot & get_slot() const {
    return s;
  }
  Slot & get_slot() {
    return s;
  }

private:
  Slot s;
};

} // namespace data
} // namespace flecsi
