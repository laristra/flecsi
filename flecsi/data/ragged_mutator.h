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
#include <flecsi/data/ragged_accessor.h>

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
                                    ragged_access<T>,
                                    public ragged_mutator_base_t {
  using base = ragged_access<T>;

  //--------------------------------------------------------------------------//
  //! Constructor from handle.
  //--------------------------------------------------------------------------//

  mutator_u(const typename base::handle_t & h) : base{h} {}

  T & operator()(size_t index, size_t ragged_index) {
    auto & row = this->handle[index];
    return row[static_cast<uint32_t>(ragged_index)];
  } // operator ()

  void resize(size_t index, size_t size) {
    auto & row = this->handle[index];
    row.resize(static_cast<uint32_t>(size));
  } // resize

  void erase(size_t index, size_t ragged_index) {
    auto & row = this->handle[index];
    row.erase(row.begin() + ragged_index);
  } // erase

  void push_back(size_t index, const T & value) {
    auto & row = this->handle[index];
    row.push_back(value);
  } // push_back

  // insert BEFORE ragged index
  T * insert(size_t index, size_t ragged_index, const T & value) {
    auto & row = this->handle[index];
    assert(ragged_index <= row.size());
    auto itr = row.insert(row.begin() + ragged_index, value);
    return &(*itr);
  } // insert
};

template<typename T>
using ragged_mutator_u = mutator_u<data::ragged, T>;

template<typename T>
using ragged_mutator = ragged_mutator_u<T>;

} // namespace flecsi
