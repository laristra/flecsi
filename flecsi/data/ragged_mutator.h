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

#include <flecsi/data/mutator.h>
#include <flecsi/data/mutator_handle.h>

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Nov 14, 2017
//----------------------------------------------------------------------------//

namespace flecsi {

//----------------------------------------------------------------------------//
//! The ragged_mutator_base_t type provides an empty base type for
//! compile-time
//! identification of data handle objects.
//!
//! @ingroup data
//----------------------------------------------------------------------------//

struct ragged_mutator_base_t {};

//----------------------------------------------------------------------------//
//! The ragged mutator_u type captures information about permissions
//! and specifies a data policy. It allows resizing of each ragged array
//! per index, uses the existing sparse data representation, but provides
//! more efficient indexing and insertion.
//!
//! @tparam T                     The data type referenced by the handle.
//!
//! @ingroup data
//----------------------------------------------------------------------------//

template<typename T>
struct mutator_u<data::ragged, T> : public mutator_u<data::base, T>,
                                    public ragged_mutator_base_t {
  using handle_t = mutator_handle_u<T>;
  using offset_t = typename handle_t::offset_t;
  using entry_value_t = typename handle_t::entry_value_t;
  using erase_set_t = typename handle_t::erase_set_t;
  using overflow_map_t = typename mutator_handle_u<T>::overflow_map_t;

  //--------------------------------------------------------------------------//
  //! Constructor from handle.
  //--------------------------------------------------------------------------//

  mutator_u(const mutator_handle_u<T> & h) : h_(h) {
    assert(!h_.overflow_map_ && "expected null overflow map");
  }

  T & operator()(size_t index, size_t ragged_index) {
    assert(h_.offsets_ && "uninitialized ragged_mutator");
    assert(index < h_.num_entries_);

    offset_t & offset = h_.offsets_orig_[index];

    size_t n = offset.count();
    size_t nnew = h_.offsets_[index].count();
    assert(ragged_index < nnew);

    if(ragged_index >= n) {
      auto & overflow = h_.overflow_map_->at(index);
      assert(ragged_index - n < overflow.size());
      return overflow[ragged_index - n].value;
    }

    entry_value_t * start = h_.entries_orig_ + offset.start();
    return start[ragged_index].value;
  } // operator ()

  void resize(size_t index, size_t size) {
    assert(index < h_.num_entries_);

    assert(size <= h_.max_entries_per_index_ &&
           "resize length exceeds max entries per index");

    offset_t & offset_new = h_.offsets_[index];
    offset_new.set_count(size);

    offset_t & offset = h_.offsets_orig_[index];
    size_t n = offset.count();
    if(size > n) {
      auto & overflow = (*h_.overflow_map_)[index];
      overflow.resize(size - n);
    }
    else {
      h_.overflow_map_->erase(index);
    }
  } // resize

#if 0
  void erase(size_t index, size_t ragged_index) {
    assert(index < h_.num_entries_);

    auto itr = h_.ragged_changes_map_->find(index);
    if(itr == h_.ragged_changes_map_->end()) {
      const offset_t & offset = h_.offsets_[index];
      assert(ragged_index < offset.count());
      ragged_changes_t changes(offset.count() - 1);
      changes.init_erase_set();
      changes.erase_set->insert(ragged_index);
      h_.ragged_changes_map_->emplace(index, std::move(changes));
    }
    else {
      ragged_changes_t & changes = itr->second;

      assert(ragged_index < changes.size);

      if(!changes.erase_set) {
        changes.init_erase_set();
      }

      if(changes.erase_set->insert(ragged_index).second) {
        --changes.size;
      }
    }
  }

  void push_back(size_t index, const T & value) {
    assert(index < h_.num_entries_);

    auto itr = h_.ragged_changes_map_->find(index);
    if(itr == h_.ragged_changes_map_->end()) {
      const offset_t & offset = h_.offsets_[index];
      ragged_changes_t changes(offset.count() + 1);
      changes.init_push_values();
      changes.push_values->push_back(value);
      h_.ragged_changes_map_->emplace(index, std::move(changes));
    }
    else {
      ragged_changes_t & changes = itr->second;

      if(!changes.push_values) {
        changes.init_push_values();
      }

      ++changes.size;
      changes.push_values->push_back(value);
    }
  }

  // insert BEFORE ragged index
  void insert(size_t index, size_t ragged_index, const T & value) {
    assert(index < h_.num_entries_);

    auto itr = h_.ragged_changes_map_->find(index);
    if(itr == h_.ragged_changes_map_->end()) {
      const offset_t & offset = h_.offsets_[index];
      assert(ragged_index < offset.count());
      ragged_changes_t changes(offset.count() + 1);
      changes.init_insert_values();
      changes.insert_values->emplace(ragged_index, value);
      h_.ragged_changes_map_->emplace(index, std::move(changes));
    }
    else {
      ragged_changes_t & changes = itr->second;

      assert(ragged_index < changes.size);

      if(!changes.insert_values) {
        changes.init_insert_values();
      }

      auto p = changes.insert_values->emplace(ragged_index, value);
      if(p.second) {
        ++changes.size;
      }
      else {
        p.first->second = value;
      }
    }
  }
#endif

  handle_t h_;
};

template<typename T>
using ragged_mutator_u = mutator_u<data::ragged, T>;

template<typename T>
using ragged_mutator = ragged_mutator_u<T>;

} // namespace flecsi
