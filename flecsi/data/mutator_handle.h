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
#include <unordered_map>
#include <vector>

#include <flecsi/data/common/data_types.h>

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

  mutator_handle_base_u() = default;

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

    init();
  }

  void init() {
    new_counts_ = new int32_t[num_entries_];
    overflow_map_ = new overflow_map_t;

    std::fill_n(new_counts_, num_entries_, -1);
  }

  size_t commit(commit_info_t * ci) {
    assert(new_counts_ && "uninitialized mutator");

    size_t num_exclusive_entries = ci->entries[1] - ci->entries[0];

    value_t * entries = ci->entries[0];
    offset_t * offsets = ci->offsets;

    value_t * cbuf = new value_t[num_exclusive_entries];

    value_t * cptr = cbuf;
    value_t * eptr = entries;

    size_t offset = 0;

    for(size_t index = 0; index < num_exclusive_; ++index) {
      offset_t & coi = offsets[index];

      size_t num_existing = coi.count();
      size_t count = new_count(index);
      size_t base_count = std::min(count, num_existing);

      std::copy_n(eptr, base_count, cptr);
      cptr += base_count;
      eptr += num_existing;

      if(count > num_existing) {
        size_t overflow_count = count - num_existing;
        // map entry must already exist - use "at" method
        const auto & overflow = overflow_map_->at(index);

        std::copy_n(overflow.begin(), overflow_count, cptr);
        cptr += overflow_count;
      }

      coi.set_offset(offset);
      coi.set_count(count);
      offset += count;
    }

    size_t num_exclusive_filled = cptr - cbuf;

    std::copy_n(cbuf, num_exclusive_entries, entries);
    delete[] cbuf;

    size_t start = num_exclusive_;
    size_t end = start + pi_.count[1] + pi_.count[2];

    for(size_t index = start; index < end; ++index) {
      offset_t & coi = offsets[index];

      size_t num_existing = coi.count();

      size_t count = new_count(index);
      assert(count <= max_entries_per_index_ &&
             "ragged data: exceeded max_entries_per_index in shared/ghost");

      if(count > num_existing) {
        value_t * eptr = entries + coi.start();
        eptr += num_existing;
        size_t overflow_count = count - num_existing;
        // map entry must already exist - use "at" method
        const auto & overflow = overflow_map_->at(index);

        std::copy_n(overflow.begin(), overflow_count, eptr);
      }

      coi.set_count(count);
    } // for index

    entries_ = nullptr;

    offsets_ = nullptr;

    delete[] new_counts_;
    new_counts_ = nullptr;

    delete overflow_map_;
    overflow_map_ = nullptr;

    return num_exclusive_filled;
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
    int nc = new_counts_[index];
    return (nc >= 0 ? nc : offsets_[index].count());
  }

  using overflow_map_t = std::unordered_map<size_t, std::vector<value_t>>;

  partition_info_t pi_;
  size_t num_exclusive_ = 0;
  size_t max_entries_per_index_ = 0;
  size_t num_slots_ = 0;
  size_t num_entries_ = 0;
  offset_t * offsets_ = nullptr;
  value_t * entries_ = nullptr;
  int32_t * new_counts_ = nullptr;
  overflow_map_t * overflow_map_ = nullptr;
  commit_info_t ci_;

}; // mutator_handle_base_u

} // namespace flecsi

#include <flecsi/runtime/flecsi_runtime_data_handle_policy.h>

namespace flecsi {

template<typename T>
using mutator_handle_u =
  mutator_handle_base_u<T, FLECSI_RUNTIME_MUTATOR_HANDLE_POLICY>;

} // namespace flecsi
