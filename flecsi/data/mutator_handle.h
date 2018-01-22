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
#include <set>
#include <unordered_map>

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
class mutator_handle_base__ : public MUTATOR_POLICY {
public:
  using entry_value_t = data::sparse_entry_value__<T>;

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

  mutator_handle_base__(
      size_t num_exclusive,
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

  mutator_handle_base__(const mutator_handle_base__ & b) = default;

  ~mutator_handle_base__() {}

  void init() {
    offsets_ = new offset_t[num_entries_];
    entries_ = new entry_value_t[num_entries_ * num_slots_];
    spare_map_ = new spare_map_t;
  }

  void commit(commit_info_t * ci) {
    if (erase_set_) {
      commit_<true>(ci);
    } else {
      if (size_map_) {
        raggedCommit_(ci);
      } else {
        commit_<false>(ci);
      }
    }
  } // operator ()

  template<bool ERASE>
  void commit_(commit_info_t * ci) {
    assert(offsets_ && "uninitialized mutator");
    ci_ = *ci;

    size_t num_exclusive_entries = ci->entries[1] - ci->entries[0];

    entry_value_t * entries = ci->entries[0];
    offset_t * offsets = ci->offsets;

    entry_value_t * cbuf = new entry_value_t[num_exclusive_entries];

    entry_value_t * cptr = cbuf;
    entry_value_t * eptr = entries;

    size_t offset = 0;

    for (size_t i = 0; i < num_exclusive_; ++i) {
      const offset_t & oi = offsets_[i];
      offset_t & coi = offsets[i];

      entry_value_t * sptr = entries_ + i * num_slots_;

      size_t num_existing = coi.count();
      size_t used_slots = oi.count();

      size_t num_merged =
          merge<ERASE>(i, eptr, num_existing, sptr, used_slots, cptr);

      eptr += num_existing;
      coi.set_offset(offset);

      if (num_merged > 0) {
        cptr += num_merged;
        coi.set_count(num_merged);
        offset += num_merged;
      }
    }

    assert(cptr - cbuf <= num_exclusive_entries);
    std::memcpy(entries, cbuf, sizeof(entry_value_t) * (cptr - cbuf));
    delete[] cbuf;

    size_t start = num_exclusive_;
    size_t end = start + pi_.count[1] + pi_.count[2];

    cbuf = new entry_value_t[max_entries_per_index_];

    for (size_t i = start; i < end; ++i) {
      const offset_t & oi = offsets_[i];
      offset_t & coi = offsets[i];

      entry_value_t * eptr = entries + coi.start();

      entry_value_t * sptr = entries_ + i * num_slots_;

      size_t num_existing = coi.count();

      size_t used_slots = oi.count();

      size_t num_merged =
          merge<ERASE>(i, eptr, num_existing, sptr, used_slots, cbuf);

      if (num_merged > 0) {
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

    if (erase_set_) {
      delete erase_set_;
      erase_set_ = nullptr;
    }
  }

  void raggedCommit_(commit_info_t * ci) {
    assert(offsets_ && "uninitialized mutator");

    size_t num_exclusive_entries = ci->entries[1] - ci->entries[0];

    entry_value_t * entries = ci->entries[0];
    offset_t * offsets = ci->offsets;

    for (auto & itr : *size_map_) {
      num_exclusive_entries +=
          int64_t(itr.second) - int64_t(offsets[itr.first].count());
    }

    entry_value_t * cbuf = new entry_value_t[num_exclusive_entries];

    entry_value_t * cptr = cbuf;
    entry_value_t * eptr = entries;

    size_t offset = 0;

    for (size_t index = 0; index < num_exclusive_; ++index) {
      const offset_t & oi = offsets_[index];
      offset_t & coi = offsets[index];

      entry_value_t * sptr = entries_ + index * num_slots_;

      size_t num_existing = coi.count();
      size_t used_slots = oi.count();

      for (size_t j = 0; j < num_existing; ++j) {
        cptr[j] = eptr[j];
      }

      for (size_t j = 0; j < used_slots; ++j) {
        size_t k = sptr[j].entry;
        cptr[k].entry = k;
        cptr[k].value = sptr[j].value;
      }

      auto p = spare_map_->equal_range(index);
      auto itr = p.first;
      auto itr_end = p.second;
      while (itr != itr_end) {
        size_t k = itr->second.entry;
        cptr[k].entry = k;
        cptr[k].value = itr->second.value;
        ++itr;
      }

      coi.set_offset(offset);

      auto sitr = size_map_->find(index);

      if (sitr != size_map_->end()) {
        size_t resize = sitr->second;
        coi.set_count(resize);
        offset += resize;
        cptr += resize;
      } else {
        offset += num_existing;
        cptr += num_existing;
      }

      eptr += num_existing;
    }

    std::memcpy(entries, cbuf, sizeof(entry_value_t) * (cptr - cbuf));
    delete[] cbuf;

    size_t start = num_exclusive_;
    size_t end = start + pi_.count[1] + pi_.count[2];

    cbuf = new entry_value_t[max_entries_per_index_];

    for (size_t index = start; index < end; ++index) {
      entry_value_t * eptr = ci->entries[1] + max_entries_per_index_ * index;

      const offset_t & oi = offsets_[index];
      offset_t & coi = offsets[index];

      entry_value_t * sptr = entries_ + index * num_slots_;

      size_t num_existing = coi.count();

      size_t used_slots = oi.count();

      for (size_t j = 0; j < num_existing; ++j) {
        cbuf[j] = eptr[j];
      }

      for (size_t j = 0; j < used_slots; ++j) {
        size_t k = sptr[j].entry;
        cbuf[k].entry = k;
        cbuf[k].value = sptr[j].value;
      }

      auto p = spare_map_->equal_range(index);
      auto itr = p.first;
      auto itr_end = p.second;
      while (itr != itr_end) {
        size_t k = itr->second.entry;
        cbuf[k].entry = k;
        cbuf[k].value = itr->second.value;
        ++itr;
      }

      size_t size;

      auto sitr = size_map_->find(index);

      if (sitr != size_map_->end()) {
        size = sitr->second;
        coi.set_count(size);
      } else {
        size = num_existing;
      }

      std::memcpy(eptr, cbuf, sizeof(entry_value_t) * size);
    }

    delete[] cbuf;

    delete[] entries_;
    entries_ = nullptr;

    delete[] offsets_;
    offsets_ = nullptr;

    delete spare_map_;
    spare_map_ = nullptr;

    delete size_map_;
    size_map_ = nullptr;
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

  commit_info_t & commit_info() {
    return ci_;
  }

  const commit_info_t & commit_info() const {
    return ci_;
  }

  using spare_map_t = std::multimap<size_t, entry_value_t>;
  using erase_set_t = std::set<std::pair<size_t, size_t>>;
  using size_map_t = std::unordered_map<size_t, size_t>;

  partition_info_t pi_;
  size_t num_exclusive_;
  size_t max_entries_per_index_;
  size_t num_slots_;
  size_t num_entries_;
  offset_t * offsets_ = nullptr;
  entry_value_t * entries_ = nullptr;
  spare_map_t * spare_map_ = nullptr;
  erase_set_t * erase_set_ = nullptr;
  size_map_t * size_map_ = nullptr;
  commit_info_t ci_;

  //--------------------------------------------------------------------------//
  //! This is a helper method to commit() to merge both the slots buffer
  //! overflow/spare map, an existing buffer, into the destination buffer,
  //! giving precedence first to the spare map, then slots, then existing.
  //--------------------------------------------------------------------------//

  template<bool ERASE>
  size_t merge(
      size_t index,
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

    for (;;) {
      if (spare_entry < end && spare_entry <= slot_entry &&
          spare_entry <= existing_entry) {

        dest->entry = spare_entry;
        dest->value = itr->second.value;
        ++dest;

        while (slot_entry == spare_entry) {
          slot_entry = ++slots < slots_end ? slots->entry : end;
        }

        while (existing_entry == slot_entry ||
               (ERASE && erase_set_->find(std::make_pair(
                             index, existing_entry)) != erase_set_->end())) {
          existing_entry = ++existing < existing_end ? existing->entry : end;
        }

        spare_entry = ++itr != p.second ? itr->second.entry : end;
      } else if (slot_entry < end && slot_entry <= existing_entry) {
        dest->entry = slot_entry;
        dest->value = slots->value;
        ++dest;

        while (existing_entry == slot_entry ||
               (ERASE && erase_set_->find(std::make_pair(
                             index, existing_entry)) != erase_set_->end())) {
          existing_entry = ++existing < existing_end ? existing->entry : end;
        }

        slot_entry = ++slots < slots_end ? slots->entry : end;
      } else if (existing_entry < end) {
        dest->entry = existing_entry;
        dest->value = existing->value;
        ++dest;

        existing_entry = ++existing < existing_end ? existing->entry : end;
      } else {
        break;
      }
    }

    return dest - dest_start;
  }
};

} // namespace flecsi

#include <flecsi/runtime/flecsi_runtime_data_handle_policy.h>

namespace flecsi {

template<typename T>
using mutator_handle__ =
    mutator_handle_base__<T, FLECSI_RUNTIME_MUTATOR_HANDLE_POLICY>;

} // namespace flecsi
