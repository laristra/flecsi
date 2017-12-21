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
//! The ragged mutator__ type captures information about permissions
//! and specifies a data policy. It allows resizing of each ragged array
//! per index, uses the existing sparse data representation, but provides
//! more efficient indexing and insertion.
//!
//! @tparam T                     The data type referenced by the handle.
//!
//! @ingroup data
//----------------------------------------------------------------------------//

template<typename T>
struct mutator__<data::ragged, T> : public mutator__<data::sparse, T>,
                                    public ragged_mutator_base_t {

  using base_t = mutator__<data::sparse, T>;

  using handle_t = typename base_t::handle_t;
  using offset_t = typename base_t::offset_t;
  using entry_value_t = typename base_t::entry_value_t;
  using erase_set_t = typename base_t::erase_set_t;
  using size_map_t = typename handle_t::size_map_t;

  //--------------------------------------------------------------------------//
  //! Copy constructor.
  //--------------------------------------------------------------------------//

  mutator__(const mutator_handle__<T> & h) : base_t(h) {
    assert(!base_t::h_.size_map_ && "expected null size map");
    base_t::h_.size_map_ = new size_map_t;
  }

  T & operator()(size_t index, size_t ragged_index) {
    assert(base_t::h_.offsets_ && "uninitialized ragged_mutator");
    assert(index < base_t::h_.num_entries_);

    offset_t & offset = base_t::h_.offsets_[index];

    size_t n = offset.count();

    if (n >= base_t::h_.num_slots_) {
      if (index < base_t::h_.num_exclusive_) {
        (*base_t::h_.num_exclusive_insertions)++;
      }

      return base_t::h_.spare_map_->emplace(index, entry_value_t(ragged_index))
          ->second.value;
    } // if

    entry_value_t * start = base_t::h_.entries_ + index * base_t::h_.num_slots_;
    entry_value_t * end = start + n;

    entry_value_t * itr = std::lower_bound(
        start, end, entry_value_t(ragged_index),
        [](const entry_value_t & e1, const entry_value_t & e2) -> bool {
          return e1.entry < e2.entry;
        });

    // if we are attempting to create an entry that already exists
    // just over-write the value and exit.
    if (itr != end && itr->entry == ragged_index) {
      return itr->value;
    }

    while (end != itr) {
      *(end) = *(end - 1);
      --end;
    } // while

    itr->entry = ragged_index;

    if (index < base_t::h_.num_exclusive_) {
      (*base_t::h_.num_exclusive_insertions)++;
    }

    offset.set_count(n + 1);

    return itr->value;
  } // operator ()

  void resize(size_t index, size_t length) {
    assert(
        length <= base_t::h_.max_entries_per_index_ &&
        "resize length exceeds max entries per index");
    base_t::h_.size_map_->emplace(index, length);
  }
};

template<typename T>
using ragged_mutator__ = mutator__<data::ragged, T>;

template<typename T>
using ragged_mutator = ragged_mutator__<T>;

} // namespace flecsi
