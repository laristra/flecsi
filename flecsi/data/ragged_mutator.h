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
#include <flecsi/topology/index_space.h>

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

  using index_space_t =
    topology::index_space_u<topology::simple_entry_u<size_t>, true>;

  //--------------------------------------------------------------------------//
  //! Constructor from handle.
  //--------------------------------------------------------------------------//

  mutator_u(const mutator_handle_u<T> & h) : h_(h) {
    assert(!h_.new_entries_ && "expected null new_entries");
  }

  T & operator()(size_t index, size_t ragged_index) {
    assert(h_.new_entries_ && "uninitialized ragged_mutator");
    assert(index < h_.num_entries_);

    size_t nnew = h_.new_count(index);
    assert(ragged_index < nnew);

    auto & overflow = h_.new_entries_[index];
    assert(ragged_index < overflow.size());
    return overflow[ragged_index];

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
  //! Return all entries used over all indices.
  //-------------------------------------------------------------------------//
  index_space_t entries() const {
    size_t id = 0;
    index_space_t is;

    const size_t max_size = size();
    for(size_t entry = 0; entry < max_size; ++entry) {
      is.push_back({id++, entry});
    }

    return is;
  }

  //-------------------------------------------------------------------------//
  //! Return all entries used over the specified index.
  //-------------------------------------------------------------------------//
  index_space_t entries(size_t index) const {
    size_t id = 0;
    index_space_t is;

    const size_t my_size = size(index);
    for(size_t entry = 0; entry < my_size; ++entry) {
      is.push_back({id++, entry});
    }

    return is;
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

    auto & overflow = h_.new_entries_[index];
    overflow.resize(size);
  } // resize

  void erase(size_t index, size_t ragged_index) {
    assert(index < h_.num_entries_);

    auto & overflow = h_.new_entries_[index];
    assert(ragged_index < overflow.size());
    overflow.erase(overflow.begin() + ragged_index);
  } // erase

  void push_back(size_t index, const T & value) {
    assert(index < h_.num_entries_);

    auto & overflow = h_.new_entries_[index];
    overflow.push_back(value);
  } // push_back

  // insert BEFORE ragged index
  T * insert(size_t index, size_t ragged_index, const T & value) {
    assert(index < h_.num_entries_);

    auto & overflow = h_.new_entries_[index];
    assert(ragged_index <= overflow.size());
    auto itr = overflow.insert(overflow.begin() + ragged_index, value);
    return &(*itr);
  } // insert

  handle_t h_;
};

template<typename T>
using ragged_mutator_u = mutator_u<data::ragged, T>;

template<typename T>
using ragged_mutator = ragged_mutator_u<T>;

} // namespace flecsi
