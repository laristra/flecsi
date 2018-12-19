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

#include <flecsi/data/mutator_handle.h>
#include <flecsi/data/sparse_mutator.h>

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
struct mutator_u<data::ragged, T> : public mutator_u<data::sparse, T>,
                                    public ragged_mutator_base_t {

  using base_t = mutator_u<data::sparse, T>;

  using handle_t = typename base_t::handle_t;
  using offset_t = typename base_t::offset_t;
  using entry_value_t = typename base_t::entry_value_t;
  using erase_set_t = typename base_t::erase_set_t;
  using ragged_changes_t = typename mutator_handle_u<T>::ragged_changes_t;
  using ragged_changes_map_t =
    typename mutator_handle_u<T>::ragged_changes_map_t;

  //--------------------------------------------------------------------------//
  //! Copy constructor.
  //--------------------------------------------------------------------------//

  mutator_u(const mutator_handle_u<T> & h) : base_t(h) {
    assert(!base_t::h_.ragged_changes_map_ && "expected null changes map");
    base_t::h_.ragged_changes_map_ = new ragged_changes_map_t;
  }

  T & operator()(size_t index, size_t ragged_index) {
    assert(base_t::h_.offsets_ && "uninitialized ragged_mutator");
    assert(index < base_t::h_.num_entries_);

    offset_t & offset = base_t::h_.offsets_[index];

    size_t n = offset.count();

    if(n >= base_t::h_.num_slots_) {
      if(index < base_t::h_.num_exclusive_) {
        (*base_t::h_.num_exclusive_insertions)++;
      }

      return base_t::h_.spare_map_->emplace(index, entry_value_t(ragged_index))
        ->second.value;
    } // if

    entry_value_t * start = base_t::h_.entries_ + index * base_t::h_.num_slots_;
    entry_value_t * end = start + n;

    entry_value_t * itr =
      std::lower_bound(start, end, entry_value_t(ragged_index),
        [](const entry_value_t & e1, const entry_value_t & e2) -> bool {
          return e1.entry < e2.entry;
        });

    // if we are attempting to create an entry that already exists
    // just over-write the value and exit.
    if(itr != end && itr->entry == ragged_index) {
      return itr->value;
    }

    while(end != itr) {
      *(end) = *(end - 1);
      --end;
    } // while

    itr->entry = ragged_index;

    if(index < base_t::h_.num_exclusive_) {
      (*base_t::h_.num_exclusive_insertions)++;
    }

    offset.set_count(n + 1);

    return itr->value;
  } // operator ()

  void resize(size_t index, size_t size) {
    assert(index < base_t::h_.num_entries_);

    assert(size <= base_t::h_.max_entries_per_index_ &&
           "resize length exceeds max entries per index");

    auto itr = base_t::h_.ragged_changes_map_->find(index);
    if(itr == base_t::h_.ragged_changes_map_->end()) {
      ragged_changes_t changes(size);
      base_t::h_.ragged_changes_map_->emplace(index, std::move(changes));
    }
    else {
      itr->second.size = size;
    }
  }

  void erase(size_t index, size_t ragged_index) {
    assert(index < base_t::h_.num_entries_);

    auto itr = base_t::h_.ragged_changes_map_->find(index);
    if(itr == base_t::h_.ragged_changes_map_->end()) {
      const offset_t & offset = base_t::h_.offsets_[index];
      assert(ragged_index < offset.count());
      ragged_changes_t changes(offset.count() - 1);
      changes.init_erase_set();
      changes.erase_set->insert(ragged_index);
      base_t::h_.ragged_changes_map_->emplace(index, std::move(changes));
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
    assert(index < base_t::h_.num_entries_);

    auto itr = base_t::h_.ragged_changes_map_->find(index);
    if(itr == base_t::h_.ragged_changes_map_->end()) {
      const offset_t & offset = base_t::h_.offsets_[index];
      ragged_changes_t changes(offset.count() + 1);
      changes.init_push_values();
      changes.push_values->push_back(value);
      base_t::h_.ragged_changes_map_->emplace(index, std::move(changes));
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
    assert(index < base_t::h_.num_entries_);

    auto itr = base_t::h_.ragged_changes_map_->find(index);
    if(itr == base_t::h_.ragged_changes_map_->end()) {
      const offset_t & offset = base_t::h_.offsets_[index];
      assert(ragged_index < offset.count());
      ragged_changes_t changes(offset.count() + 1);
      changes.init_insert_values();
      changes.insert_values->emplace(ragged_index, value);
      base_t::h_.ragged_changes_map_->emplace(index, std::move(changes));
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
};

template<typename T>
using ragged_mutator_u = mutator_u<data::ragged, T>;

template<typename T>
using ragged_mutator = ragged_mutator_u<T>;

} // namespace flecsi
