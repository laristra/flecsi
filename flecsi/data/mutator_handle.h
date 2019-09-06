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
#pragma once

/*! @file */

#include <algorithm>
#include <stdint.h>

#include <flecsi/data/common/data_types.h>
#include <flecsi/data/common/simple_vector.h>

namespace flecsi {

//----------------------------------------------------------------------------//
//! This class is used to implement mutators for ragged and sparse data.
//! Its methods implement commit functionality to pack the mutator's
//! existing data and overflow map into the final commit buffer.
//----------------------------------------------------------------------------//

template<typename T, typename MUTATOR_POLICY>
class mutator_handle_base_u : public MUTATOR_POLICY
{
public:
  using value_t = T;

  using offset_t = data::sparse_data_offset_t;

  using index_t = uint64_t;

  using vector_t = data::simple_vector_u<T>;

  size_t * num_exclusive_insertions;

  struct partition_info_t {
    size_t count[3];
    size_t start[3];
    size_t end[3];
  };

  struct commit_info_t {
    offset_t * offsets;
    value_t * entries[3];
  };

  //--------------------------------------------------------------------------//
  //! Default constructor.
  //--------------------------------------------------------------------------//

  mutator_handle_base_u(size_t num_exclusive,
    size_t num_shared,
    size_t num_ghost,
    size_t max_entries_per_index,
    size_t num_slots)
    : num_entries_(num_exclusive + num_shared + num_ghost),
      num_exclusive_(num_exclusive),
      max_entries_per_index_(max_entries_per_index), num_slots_(num_slots) {
    pi_.count[0] = num_exclusive;
    pi_.count[1] = num_shared;
    pi_.count[2] = num_ghost;

    pi_.start[0] = 0;
    pi_.end[0] = num_exclusive;

    pi_.start[1] = num_exclusive;
    pi_.end[1] = num_exclusive + num_shared;

    pi_.start[2] = pi_.end[1];
    pi_.end[2] = pi_.end[1] + num_ghost;
  }

  mutator_handle_base_u(size_t max_entries_per_index, size_t num_slots)
    : max_entries_per_index_(max_entries_per_index), num_slots_(num_slots) {}

  mutator_handle_base_u(const mutator_handle_base_u & b) = default;

  ~mutator_handle_base_u() {}

  void init(const size_t & num_exclusive,
    const size_t & num_shared,
    const size_t & num_ghost) {
    num_entries_ = num_exclusive + num_shared + num_ghost;
    num_exclusive_ = num_exclusive;

    pi_.count[0] = num_exclusive;
    pi_.count[1] = num_shared;
    pi_.count[2] = num_ghost;

    pi_.start[0] = 0;
    pi_.end[0] = num_exclusive;

    pi_.start[1] = num_exclusive;
    pi_.end[1] = num_exclusive + num_shared;

    pi_.start[2] = pi_.end[1];
    pi_.end[2] = pi_.end[1] + num_ghost;
  }

  size_t commit(commit_info_t * ci) {
    assert(new_entries_ && "uninitialized mutator");

    size_t num_exclusive_entries = ci->entries[1] - ci->entries[0];

    // no longer needed - but return a reasonable value just in case

    return num_exclusive_entries;
  } // commit

  size_t num_exclusive() const {
    return pi_.count[0];
  }

  size_t num_shared() const {
    return pi_.count[1];
  }

  size_t num_ghost() const {
    return pi_.count[2];
  }

  size_t max_entries_per_index() const {
    return max_entries_per_index_;
  }

  size_t number_exclusive_entries() const {
    return ci_.entries[1] - ci_.entries[0];
  }

  commit_info_t & commit_info() {
    return ci_;
  }

  const commit_info_t & commit_info() const {
    return ci_;
  }

  size_t new_count(size_t index) const {
    return new_entries_[index].size();
  }

  partition_info_t pi_;
  size_t num_exclusive_;
  size_t max_entries_per_index_;
  size_t num_slots_;
  size_t num_entries_;
  vector_t * new_entries_ = nullptr;
  commit_info_t ci_;

}; // mutator_handle_base_u

} // namespace flecsi

#include <flecsi/runtime/flecsi_runtime_data_handle_policy.h>

namespace flecsi {

template<typename T>
using mutator_handle_u =
  mutator_handle_base_u<T, FLECSI_RUNTIME_MUTATOR_HANDLE_POLICY>;

} // namespace flecsi
