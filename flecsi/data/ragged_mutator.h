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
  using value_t = T;
  using overflow_map_t = typename mutator_handle_u<T>::overflow_map_t;

  //--------------------------------------------------------------------------//
  //! Constructor from handle.
  //--------------------------------------------------------------------------//
  
  mutator_u() = default;

  mutator_u(const mutator_handle_u<T> & h) : h_(h) {
    assert(!h_.overflow_map_ && "expected null overflow map");
  }

  T & operator()(size_t index, size_t ragged_index) {
    assert(h_.offsets_ && "uninitialized ragged_mutator");
    assert(index < h_.num_entries_);

    offset_t & offset = h_.offsets_[index];

    size_t n = offset.count();
    size_t nnew = h_.new_count(index);
    assert(ragged_index < nnew);

    if(ragged_index >= n) {
      // value is in overflow area
      auto & overflow = h_.overflow_map_->at(index);
      assert(ragged_index - n < overflow.size());
      return overflow[ragged_index - n];
    }

    // value is in base area
    return h_.entries_[offset.start() + ragged_index];
  } // operator ()

  //-------------------------------------------------------------------------//
  //! Return max number of entries used over all indices.
  //-------------------------------------------------------------------------//
  size_t size() const {
    size_t max_so_far = 0;

    for(size_t index = 0; index < h_.num_total_; ++index) {
      max_so_far = std::max(max_so_far, h_.new_count(index));
    }

    return max_so_far;
  }

  //-------------------------------------------------------------------------//
  //! Return number of entries used over the specified index.
  //-------------------------------------------------------------------------//
  size_t size(size_t index) const {
    assert(index < h_.num_entries_);
    return h_.new_count(index);
  }

  //-------------------------------------------------------------------------//
  //! Return the maximum possible number of entries
  //-------------------------------------------------------------------------//
  auto max_size() const noexcept {
    return h_.max_entries_per_index;
  }

  void resize(size_t index, size_t size) {
    assert(index < h_.num_entries_);

    assert(size <= h_.max_entries_per_index_ &&
           "resize length exceeds max entries per index");

    h_.new_counts_[index] = size;

    offset_t & offset = h_.offsets_[index];
    size_t n = offset.count();
    if(size > n) {
      auto & overflow = (*h_.overflow_map_)[index];
      overflow.resize(size - n);
    }
    else {
      h_.overflow_map_->erase(index);
    }
  } // resize

  void erase(size_t index, size_t ragged_index) {
    assert(index < h_.num_entries_);

    offset_t & offset = h_.offsets_[index];

    size_t n = offset.count();
    size_t nnew = h_.new_count(index);
    assert(ragged_index < nnew);

    h_.new_counts_[index] = nnew - 1;

    if(ragged_index >= n) {
      // erase from overflow area
      auto & overflow = h_.overflow_map_->at(index);
      assert(ragged_index - n < overflow.size());
      overflow.erase(overflow.begin() + (ragged_index - n));
      return;
    }

    // erase from base area
    auto eptr = h_.entries_ + offset.start();
    size_t ncopy = std::min(n, nnew);
    std::copy(&eptr[ragged_index + 1], &eptr[ncopy], &eptr[ragged_index]);
    if(nnew > n) {
      // shift out of overflow area, if needed
      auto & overflow = h_.overflow_map_->at(index);
      eptr[n - 1] = overflow.front();
      overflow.erase(overflow.begin());
    }
  } // erase

  void push_back(size_t index, const T & value) {
    assert(index < h_.num_entries_);

    offset_t & offset = h_.offsets_[index];

    size_t n = offset.count();
    size_t nnew = h_.new_count(index);

    h_.new_counts_[index] = nnew + 1;

    if(nnew >= n) {
      // add to overflow area
      auto & overflow = (*h_.overflow_map_)[index];
      overflow.push_back(value);
      return;
    }

    // add to base area
    auto eptr = h_.entries_ + offset.start();
    eptr[nnew] = value;
  } // push_back

  // insert BEFORE ragged index
  T * insert(size_t index, size_t ragged_index, const T & value) {
    assert(index < h_.num_entries_);

    offset_t & offset = h_.offsets_[index];

    size_t n = offset.count();
    size_t nnew = h_.new_count(index);
    assert(ragged_index <= nnew);

    h_.new_counts_[index] = nnew + 1;

    if(ragged_index >= n) {
      // insert in overflow area
      auto & overflow = (*h_.overflow_map_)[index];
      assert(ragged_index - n <= overflow.size());
      auto itr = overflow.insert(overflow.begin() + (ragged_index - n), value);
      return &(*itr);
    }

    // insert in base area
    auto eptr = h_.entries_ + offset.start();
    if(nnew >= n) {
      // shift into overflow area, if needed
      auto & overflow = (*h_.overflow_map_)[index];
      overflow.insert(overflow.begin(), eptr[n - 1]);
    }
    size_t ncopy = std::min(n - 1, nnew);
    std::copy_backward(&eptr[ragged_index], &eptr[ncopy], &eptr[ncopy + 1]);
    eptr[ragged_index] = value;
    return &eptr[ragged_index];
  } // insert

  handle_t h_;
};

template<typename T>
using ragged_mutator_u = mutator_u<data::ragged, T>;

template<typename T>
using ragged_mutator = ragged_mutator_u<T>;

} // namespace flecsi
