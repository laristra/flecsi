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

#include <cinchlog.h>

#include <flecsi/data/accessor.h>
#include <flecsi/data/common/data_types.h>
#include <flecsi/data/sparse_data_handle.h>
#include <flecsi/topology/index_space.h>

namespace flecsi {

template<class R> // a ragged accessor or mutator
struct sparse_access { // shared between accessor and mutator
protected:
  using ragged_t = R;
  using entry_value_t = typename ragged_t::value_type;
  using vector_t = typename ragged_t::handle_t::vector_t;
  vector_t & row(std::size_t i) {
    return ragged.handle[i];
  }

public:
  using index_space_t =
    topology::index_space_u<topology::simple_entry_u<size_t>, true>;

  // for row 'index', return pointer to first entry not less
  // than 'entry'
  static typename vector_t::iterator lower_bound(vector_t & row, size_t entry) {
    auto start = row.begin();
    auto end = row.end();

    // find where entry should be
    auto itr = std::lower_bound(start, end, entry_value_t(entry),
      [](const entry_value_t & k1, const entry_value_t & k2) -> bool {
        return k1.entry < k2.entry;
      });

    return itr;

  } // lower_bound

  // for row 'index', return pointer to first entry not less
  // than 'entry'
  static typename vector_t::const_iterator lower_bound(const vector_t & row,
    size_t entry) {
    return lower_bound(const_cast<vector_t &>(row), entry);
  } // lower_bound

  // for row 'index', test whether entry 'entry' is present
  bool contains(size_t index, size_t entry) const {
    auto & r = row(index);
    const auto itr = lower_bound(r, entry);
    return itr != r.end() && itr->entry == entry;
  } // contains

  void dump() {
    auto & h_ = ragged.handle;
    std::size_t i = 0;
    const auto f = [&](const char * l, std::size_t n) {
      std::cout << l << ": \n";

      for(std::size_t end = i + n; i < end; ++i) {
        std::cout << "  index: " << i << std::endl;
        const auto & row = h_.rows[i];
        for(const auto & ev : row) {
          std::cout << "    +" << ev.entry << " = " << ev.value << std::endl;
        }
      }
    };
    f("exclusive", h_.num_exclusive);
    f("shared", h_.num_shared);
    f("ghost", h_.num_ghost);
  } // dump

  //-------------------------------------------------------------------------//
  //! Return all entries used over all indices.
  //-------------------------------------------------------------------------//
  index_space_t entries() const {
    auto & handle = ragged.handle;
    size_t id = 0;
    index_space_t is;
    std::unordered_set<size_t> found;

    for(size_t index = 0; index < handle.num_total_; ++index) {
      const auto & row = handle.rows[index];

      for(const auto & ev : row) {
        size_t entry = ev.entry;
        if(found.find(entry) == found.end()) {
          is.push_back({id++, entry});
          found.insert(entry);
        }
      }
    }

    return is;
  }

  //-------------------------------------------------------------------------//
  //! Return all entries used over the specified index.
  //-------------------------------------------------------------------------//
  index_space_t entries(size_t index) const {
    auto & handle = ragged.handle;
    clog_assert(
      index < handle.num_total_, "sparse accessor: index out of bounds");

    index_space_t is;

    size_t id = 0;
    const auto & row = handle.rows[index];
    for(const auto & ev : row) {
      is.push_back({id++, ev.entry});
    }

    return is;
  }

  //-------------------------------------------------------------------------//
  //! Return all indices allocated.
  //-------------------------------------------------------------------------//
  index_space_t indices() const {
    auto & handle = ragged.handle;
    index_space_t is;
    size_t id = 0;

    for(size_t index = 0; index < handle.num_total_; ++index) {
      const auto & row = handle.rows[index];
      if(row.size() != 0) {
        is.push_back({id++, index});
      }
    }

    return is;
  }

  //-------------------------------------------------------------------------//
  //! Return all indices allocated for a given entry.
  //-------------------------------------------------------------------------//
  index_space_t indices(size_t entry) const {
    auto & handle = ragged.handle;
    index_space_t is;
    size_t id = 0;

    for(size_t index = 0; index < handle.num_total_; ++index) {
      auto & r = row(index);
      const auto itr = lower_bound(r, entry);
      if(itr != r.end() && itr->entry == entry) {
        is.push_back({id++, index});
      }
    }

    return is;
  }

  ragged_t ragged;
};

//----------------------------------------------------------------------------//
//! The sparse_accessor_base_t type provides an empty base type for
//! compile-time identification of data handle objects.
//!
//! @ingroup data
//----------------------------------------------------------------------------//

struct sparse_accessor_base_t {};

//----------------------------------------------------------------------------//
//! The sparse accessor_u type captures information about permissions
//! and specifies a data policy. It provides methods for looking up
//! a data item given an index and entry, for querying allocated indices and
//! entries per index and over all indices.
//!
//! @tparam T                     The data type referenced by the handle.
//! @tparam EXCLUSIVE_PERMISSIONS The permissions required on the exclusive
//!                               indices of the index partition.
//! @tparam SHARED_PERMISSIONS    The permissions required on the shared
//!                               indices of the index partition.
//! @tparam GHOST_PERMISSIONS     The permissions required on the ghost
//!                               indices of the index partition.
//! @tparam DATA_POLICY           The data policy for this handle type.
//!
//! @ingroup data
//----------------------------------------------------------------------------//

template<typename T,
  size_t EXCLUSIVE_PERMISSIONS,
  size_t SHARED_PERMISSIONS,
  size_t GHOST_PERMISSIONS>
struct accessor_u<data::sparse,
  T,
  EXCLUSIVE_PERMISSIONS,
  SHARED_PERMISSIONS,
  GHOST_PERMISSIONS>
  : public accessor_u<data::base,
      T,
      EXCLUSIVE_PERMISSIONS,
      SHARED_PERMISSIONS,
      GHOST_PERMISSIONS>,
    sparse_access<ragged_accessor<data::sparse_entry_value_u<T>,
      EXCLUSIVE_PERMISSIONS,
      SHARED_PERMISSIONS,
      GHOST_PERMISSIONS>>,
    public sparse_accessor_base_t {
private:
  using base = sparse_access<ragged_accessor<data::sparse_entry_value_u<T>,
    EXCLUSIVE_PERMISSIONS,
    SHARED_PERMISSIONS,
    GHOST_PERMISSIONS>>;
  using typename base::entry_value_t; // factor usage?

public:
  using typename base::index_space_t; // unless we can factor the usage?

  accessor_u(const typename base::ragged_t::handle_t & h) : base{h} {}

  //-------------------------------------------------------------------------//
  //! Main accessor
  //!
  //! Access a sparse element.  The element has to exist because we
  //! return a reference to it.
  //-------------------------------------------------------------------------//
  T & operator()(size_t index, size_t entry) {
    auto & r = this->row(index);
    const auto itr = base::lower_bound(r, entry);
    assert(itr != r.end() && itr->entry == entry &&
           "sparse accessor: unmapped entry");

    return itr->value;
  } // operator ()

  const T & operator()(size_t index, size_t entry) const {
    return const_cast<accessor_u &>(*this)(index, entry);
  } // operator ()

  //! a struct used for accessing elements.
  struct result_t {
    const T * value_ptr = nullptr; //!< a pointer to the element
    bool exists = false; //!< a boolean, true if element exists
  };

  //-------------------------------------------------------------------------//
  //! Accessor with boolean
  //!
  //! Access a sparse element.  Returns a pointer to the element (null if not
  //! found), and a boolean specifying whether the element existed.
  //-------------------------------------------------------------------------//
  result_t at(size_t index, size_t entry) const {
    auto & r = this->row(index);
    const auto itr = base::lower_bound(r, entry);

    if(itr != r.end() && itr->entry == entry)
      return result_t{&itr->value, true};
    else
      return result_t{nullptr, false};
  } // at()

  template<typename E>
  T & operator()(E * e, size_t entry) {
    return this->operator()(e->template id<0>(), entry);
  } // operator ()

  //-------------------------------------------------------------------------//
  //! Return the maximum possible entries
  //-------------------------------------------------------------------------//
  auto max_entries() const noexcept {
    auto & handle = this->ragged.handle;
    return handle.max_entries_per_index;
  }
};

template<typename T,
  size_t EXCLUSIVE_PERMISSIONS,
  size_t SHARED_PERMISSIONS,
  size_t GHOST_PERMISSIONS>
using sparse_accessor_u = accessor_u<data::sparse,
  T,
  EXCLUSIVE_PERMISSIONS,
  SHARED_PERMISSIONS,
  GHOST_PERMISSIONS>;

template<typename T,
  size_t EXCLUSIVE_PERMISSIONS,
  size_t SHARED_PERMISSIONS,
  size_t GHOST_PERMISSIONS>
using sparse_accessor = sparse_accessor_u<T,
  EXCLUSIVE_PERMISSIONS,
  SHARED_PERMISSIONS,
  GHOST_PERMISSIONS>;

} // namespace flecsi
