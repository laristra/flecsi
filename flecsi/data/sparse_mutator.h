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
#include <iostream>
#include <unordered_set>

#include <flecsi/data/mutator.h>
#include <flecsi/data/sparse_accessor.h>
#include <flecsi/topology/index_space.h>

namespace flecsi {

//----------------------------------------------------------------------------//
//! The mutator_base_t type provides an empty base type for
//! compile-time
//! identification of data handle objects.
//!
//! @ingroup data
//----------------------------------------------------------------------------//

struct sparse_mutator_base_t {};

//----------------------------------------------------------------------------//
//! The mutator type captures information about permissions
//! and specifies a data policy. operator() is used to insert entries at a given
//! index into these buffers, and keeps entries in sorted order per index.
//! Entries may also be deleted with erase().
//!
//! @tparam T                     The data type referenced by the handle.
//!
//! @ingroup data
//----------------------------------------------------------------------------//

template<typename T>
struct mutator_u<data::sparse, T>
  : public mutator_u<data::base, T>,
    sparse_access<mutator_u<data::ragged, data::sparse_entry_value_u<T>>>,
    public sparse_mutator_base_t {
private:
  using base =
    sparse_access<mutator_u<data::ragged, data::sparse_entry_value_u<T>>>;
  using typename base::entry_value_t;

public:
  using typename base::index_space_t;

  mutator_u(const typename base::ragged_t::handle_t & h) : base{h} {}

  T & operator()(size_t index, size_t entry) {
    auto & r = this->row(index);
    const auto itr = base::lower_bound(r, entry);

    // if we are attempting to create an entry that already exists
    // just over-write the value and exit.
    if(itr != r.end() && itr->entry == entry) {
      return itr->value;
    }

    // otherwise, create a new entry
    auto ritr = r.insert(itr, {entry, T()});
    return ritr->value;

  } // operator ()

  void erase(size_t index, size_t entry) {
    auto & r = this->row(index);
    const auto itr = base::lower_bound(r, entry);

    // if we are attempting to erase an entry that doesn't exist,
    // then just return
    if(itr == r.end() || itr->entry != entry) {
      return;
    }

    // otherwise, erase
    r.erase(itr);

  } // erase
}; // mutator_u

template<typename T>
using sparse_mutator_u = mutator_u<data::sparse, T>;

template<typename T>
using sparse_mutator = sparse_mutator_u<T>;

} // namespace flecsi
