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

#include <flecsi/data/backend.hh>
#include <flecsi/data/data_reference.hh>
#include <flecsi/execution.hh>
#include <flecsi/runtime/types.hh>

#include <mpi.h>

namespace flecsi {
namespace data {

template<typename TOPOLOGY_TYPE, typename... ARGS>
void
coloring_task(size_t identifier, ARGS &&... args) {
  auto & coloring_info =
    runtime::context_t::instance().coloring<TOPOLOGY_TYPE>(identifier);
} // coloring_task

template<typename TOPOLOGY_TYPE>
struct coloring_reference : public data_reference_base_t {

  coloring_reference()
    : data_reference_base_t(unique_cid_t::instance().next()) {}

  /*!
    @note The MPI task launched by this method is always mapped to the process
          launch domain.
   */

  template<typename... ARGS>
  void allocate(ARGS &&... args) {
    execute<coloring_task<TOPOLOGY_TYPE, ARGS...>, index, mpi>(
      identifier_, std::forward<ARGS>(args)...);
    allocated_ = true;
  } // allocate

  void deallocate() {
    data_policy_t::deallocate_coloring<TOPOLOGY_TYPE>(identifier_);
    allocated_ = false;
  } // deallocate

private:
  bool allocated_ = false;

}; // struct coloring_reference

} // namespace data
} // namespace flecsi
