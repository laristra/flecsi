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
#include <cstring>
#include <limits>
#include <map>
#include <memory>
#include <set>
#include <unordered_map>
#include <vector>

#include <flecsi/data/common/data_types.h>

namespace flecsi {

//----------------------------------------------------------------------------//
//! This class is used to implement the mutator for sparse data. It contains
//! methods which implement commit functionality to pack the mutator's
//! temporary slots and spare (overflow) map into the final commit buffer.
//! This class implements functionality for both normal sparse data and the
//! ragged sparse data type.
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

  void init(
    const size_t &num_exclusive,
    const size_t &num_shared,
    const size_t &num_ghost
    ){
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
    offsets_ = new offset_t[num_entries_];
    entries_ = new value_t[num_entries_ * num_slots_];
    new_counts_ = new int32_t[num_entries_];
    spare_map_ = new spare_map_t;
    std::fill_n(new_counts_, num_entries_, -1);
  }

  void fill_ragged(const commit_info_t & ci) {
    entries_orig_ = ci.entries[0];
    offsets_orig_ = ci.offsets;
    overflow_map_ = new overflow_map_t;
  } // fill_ragged

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

    delete[] entries_;
    entries_ = nullptr;

    delete[] offsets_;
    offsets_ = nullptr;

    delete[] new_counts_;
    new_counts_ = nullptr;

    delete spare_map_;
    spare_map_ = nullptr;

    entries_orig_ = nullptr;

    offsets_orig_ = nullptr;

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
    return (nc >= 0 ? nc : offsets_orig_[index].count());
  }

  using spare_map_t = std::multimap<size_t, value_t>;
  using erase_set_t = std::set<std::pair<size_t, size_t>>;
  using overflow_map_t = std::unordered_map<size_t, std::vector<value_t>>;

  partition_info_t pi_;
  size_t num_exclusive_;
  size_t max_entries_per_index_;
  size_t num_slots_;
  size_t num_entries_;
  offset_t * offsets_ = nullptr;
  offset_t * offsets_orig_ = nullptr;
  value_t * entries_ = nullptr;
  value_t * entries_orig_ = nullptr;
  int32_t * new_counts_ = nullptr;
  spare_map_t * spare_map_ = nullptr;
  erase_set_t * erase_set_ = nullptr;
  overflow_map_t * overflow_map_ = nullptr;
  commit_info_t ci_;

}; // mutator_handle_base_u

// partial specialization
template<typename T, typename MUTATOR_POLICY>
class mutator_handle_base_u<data::sparse_entry_value_u<T>, MUTATOR_POLICY> : public MUTATOR_POLICY
{
public:
  using entry_value_t = data::sparse_entry_value_u<T>;

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
    entry_value_t * entries[3];
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

  void init(
    const size_t &num_exclusive,
    const size_t &num_shared,
    const size_t &num_ghost
    ){
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
    offsets_ = new offset_t[num_entries_];
    entries_ = new entry_value_t[num_entries_ * num_slots_];
    spare_map_ = new spare_map_t;
  }

  void fill_ragged(const commit_info_t & ci) {
    entries_orig_ = ci.entries[0];
    offsets_orig_ = ci.offsets;
    overflow_map_ = new overflow_map_t;

    for(size_t index = 0; index < num_entries_; ++index) {
      const offset_t & offset = offsets_orig_[index];
      size_t count = offset.count();

      offsets_[index].set_count(count);
    }
  } // fill_ragged

  size_t commit(commit_info_t * ci) {
    if(erase_set_) {
      return commit_<true>(ci);
    }
    else {
      if(overflow_map_) {
        return raggedCommit_(ci);
      }
      else {
        return commit_<false>(ci);
      }
    }
  } // operator ()

  template<bool ERASE>
  size_t commit_(commit_info_t * ci) {
    assert(offsets_ && "uninitialized mutator");
    ci_ = *ci;

    size_t num_exclusive_entries = ci->entries[1] - ci->entries[0];

    entry_value_t * entries = ci->entries[0];
    offset_t * offsets = ci->offsets;

    entry_value_t * cbuf = new entry_value_t[num_exclusive_entries];

    entry_value_t * cptr = cbuf;
    entry_value_t * eptr = entries;

    size_t offset = 0;

    for(size_t i = 0; i < num_exclusive_; ++i) {
      const offset_t & oi = offsets_[i];
      offset_t & coi = offsets[i];

      entry_value_t * sptr = entries_ + i * num_slots_;

      size_t num_existing = coi.count();
      size_t used_slots = oi.count();

      size_t num_merged =
        merge<ERASE>(i, eptr, num_existing, sptr, used_slots, cptr);

      eptr += num_existing;
      coi.set_offset(offset);

      if(num_merged > 0) {
        cptr += num_merged;
        coi.set_count(num_merged);
        offset += num_merged;
      }
    }

    size_t num_exclusive_filled = cptr - cbuf;

    assert(cptr - cbuf <= num_exclusive_entries);
    std::memcpy(entries, cbuf, sizeof(entry_value_t) * num_exclusive_filled);
    delete[] cbuf;

    size_t start = num_exclusive_;
    size_t end = start + pi_.count[1] + pi_.count[2];

    cbuf = new entry_value_t[max_entries_per_index_];

    for(size_t i = start; i < end; ++i) {
      const offset_t & oi = offsets_[i];
      offset_t & coi = offsets[i];

      entry_value_t * eptr = entries + coi.start();

      entry_value_t * sptr = entries_ + i * num_slots_;

      size_t num_existing = coi.count();

      size_t used_slots = oi.count();

      size_t num_merged =
        merge<ERASE>(i, eptr, num_existing, sptr, used_slots, cbuf);

      if(num_merged > 0) {
        assert(num_merged <= max_entries_per_index_);
        std::memcpy(eptr, cbuf, sizeof(entry_value_t) * num_merged);
        coi.set_count(num_merged);
      }
    }

    delete[] cbuf;

    delete[] entries_;
    entries_ = nullptr;

    delete[] offsets_;
    offsets_ = nullptr;

    delete spare_map_;
    spare_map_ = nullptr;

    if(erase_set_) {
      delete erase_set_;
      erase_set_ = nullptr;
    }

    return num_exclusive_filled;
  }

  size_t raggedCommit_(commit_info_t * ci) {
    assert(offsets_ && "uninitialized mutator");

    size_t num_exclusive_entries = ci->entries[1] - ci->entries[0];

    entry_value_t * entries = ci->entries[0];
    offset_t * offsets = ci->offsets;

    entry_value_t * cbuf = new entry_value_t[num_exclusive_entries];

    entry_value_t * cptr = cbuf;
    entry_value_t * eptr = entries;

    size_t offset = 0;

    for(size_t index = 0; index < num_exclusive_; ++index) {
      const offset_t & oi = offsets_[index];
      offset_t & coi = offsets[index];

      size_t num_existing = coi.count();
      size_t count = oi.count();
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
      const offset_t & oi = offsets_[index];
      offset_t & coi = offsets[index];

      size_t num_existing = coi.count();

      size_t count = oi.count();
      assert(count <= max_entries_per_index_ &&
             "ragged data: exceeded max_entries_per_index in shared/ghost");

      if(count > num_existing) {
        entry_value_t * eptr = entries + coi.start();
        eptr += num_existing;
        size_t overflow_count = count - num_existing;
        // map entry must already exist - use "at" method
        const auto & overflow = overflow_map_->at(index);

        std::copy_n(overflow.begin(), overflow_count, eptr);
      }

      coi.set_count(count);
    } // for index

    delete[] entries_;
    entries_ = nullptr;

    delete[] offsets_;
    offsets_ = nullptr;

    delete spare_map_;
    spare_map_ = nullptr;

    entries_orig_ = nullptr;

    offsets_orig_ = nullptr;

    delete overflow_map_;
    overflow_map_ = nullptr;

    return num_exclusive_filled;
  }

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

  using spare_map_t = std::multimap<size_t, entry_value_t>;
  using erase_set_t = std::set<std::pair<size_t, size_t>>;
  using overflow_map_t = std::unordered_map<size_t, std::vector<entry_value_t>>;

  partition_info_t pi_;
  size_t num_exclusive_;
  size_t max_entries_per_index_;
  size_t num_slots_;
  size_t num_entries_;
  offset_t * offsets_ = nullptr;
  offset_t * offsets_orig_ = nullptr;
  entry_value_t * entries_ = nullptr;
  entry_value_t * entries_orig_ = nullptr;
  spare_map_t * spare_map_ = nullptr;
  erase_set_t * erase_set_ = nullptr;
  overflow_map_t * overflow_map_ = nullptr;
  commit_info_t ci_;

  //--------------------------------------------------------------------------//
  //! This is a helper method to commit() to merge both the slots buffer
  //! overflow/spare map, an existing buffer, into the destination buffer,
  //! giving precedence first to the spare map, then slots, then existing.
  //--------------------------------------------------------------------------//

  template<bool ERASE>
  size_t merge(size_t index,
    entry_value_t * existing,
    size_t num_existing,
    entry_value_t * slots,
    size_t num_slots,
    entry_value_t * dest) {

    constexpr size_t end = std::numeric_limits<size_t>::max();
    entry_value_t * existing_end = existing + num_existing;
    entry_value_t * slots_end = slots + num_slots;

    entry_value_t * dest_start = dest;

    auto p = spare_map_->equal_range(index);
    auto itr = p.first;

    size_t spare_entry = itr != p.second ? itr->second.entry : end;
    size_t slot_entry = slots < slots_end ? slots->entry : end;
    size_t existing_entry = existing < existing_end ? existing->entry : end;

    for(;;) {
      if(spare_entry < end && spare_entry <= slot_entry &&
         spare_entry <= existing_entry) {

        dest->entry = spare_entry;
        dest->value = itr->second.value;
        ++dest;

        while(slot_entry == spare_entry) {
          slot_entry = ++slots < slots_end ? slots->entry : end;
        }

        while(existing_entry == spare_entry ||
              (ERASE && erase_set_->find(std::make_pair(
                          index, existing_entry)) != erase_set_->end())) {
          existing_entry = ++existing < existing_end ? existing->entry : end;
        }

        spare_entry = ++itr != p.second ? itr->second.entry : end;
      }
      else if(slot_entry < end && slot_entry <= existing_entry) {
        dest->entry = slot_entry;
        dest->value = slots->value;
        ++dest;

        while(existing_entry == slot_entry ||
              (ERASE && erase_set_->find(std::make_pair(
                          index, existing_entry)) != erase_set_->end())) {
          existing_entry = ++existing < existing_end ? existing->entry : end;
        }

        slot_entry = ++slots < slots_end ? slots->entry : end;
      }
      else if(existing_entry < end) {
        dest->entry = existing_entry;
        dest->value = existing->value;
        ++dest;

        existing_entry = ++existing < existing_end ? existing->entry : end;
      }
      else {
        break;
      }
    }

    return dest - dest_start;
  } // merge

}; // mutator_handle_base_u (sparse)

} // namespace flecsi

#include <flecsi/runtime/flecsi_runtime_data_handle_policy.h>

namespace flecsi {

template<typename T>
using mutator_handle_u =
  mutator_handle_base_u<T, FLECSI_RUNTIME_MUTATOR_HANDLE_POLICY>;

} // namespace flecsi
