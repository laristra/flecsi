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
  GHOST_PERMISSIONS> : public accessor_u<data::ragged,
                         data::sparse_entry_value_u<T>,
                         EXCLUSIVE_PERMISSIONS,
                         SHARED_PERMISSIONS,
                         GHOST_PERMISSIONS>,
                       public sparse_accessor_base_t {
  using handle_t = sparse_data_handle_u<T,
    EXCLUSIVE_PERMISSIONS,
    SHARED_PERMISSIONS,
    GHOST_PERMISSIONS>;

  using entry_value_t = data::sparse_entry_value_u<T>;
  using vector_t = typename handle_t::vector_t;

  using base_t = accessor_u<data::ragged,
    entry_value_t,
    EXCLUSIVE_PERMISSIONS,
    SHARED_PERMISSIONS,
    GHOST_PERMISSIONS>;

  using index_space_t =
    topology::index_space_u<topology::simple_entry_u<size_t>, true>;

  //-------------------------------------------------------------------------//
  //! Copy constructor.
  //-------------------------------------------------------------------------//

  accessor_u(const accessor_u & a) : base_t(a) {}

  accessor_u(const typename sparse_data_handle_u<T, 0, 0, 0>::base_t & h)
    : base_t(h) {}

  //-------------------------------------------------------------------------//
  //! Main accessor
  //!
  //! Access a sparse element.  The element has to exist because we
  //! return a reference to it.
  //-------------------------------------------------------------------------//
  T & operator()(size_t index, size_t entry) {
    auto itr = lower_bound(index, entry);
    assert(itr && itr->entry == entry && "sparse accessor: unmapped entry");

    return itr->value;
  } // operator ()

  //-------------------------------------------------------------------------//
  //! Main accessor (const version)
  //!
  //! Access a sparse element.  Return an emtpy value if not found.  The empty
  //! value is specified by the default constructor of the underlying type.
  //-------------------------------------------------------------------------//
  const T & operator()(size_t index, size_t entry) const {
    auto itr = lower_bound(index, entry);
    assert(itr && itr->entry == entry && "sparse accessor: unmapped entry");

    if(itr && itr->entry == entry)
      return itr->value;
    else
      return T{};
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
    auto itr = lower_bound(index, entry);

    if(itr && itr->entry == entry)
      return result_t{&itr->value, true};
    else
      return result_t{nullptr, false};
  } // at()

  template<typename E>
  T & operator()(E * e, size_t entry) {
    return this->operator()(e->template id<0>(), entry);
  } // operator ()

  // for row 'index', return pointer to first entry not less
  // than 'entry'
  entry_value_t * lower_bound(size_t index, size_t entry) {
    auto & handle = base_t::handle;
    assert(index < handle.num_total_ && "sparse accessor: index out of bounds");

    vector_t & row = handle.new_entries[index];
    auto start = row.begin();
    auto end = row.end();

    // find where entry should be
    auto itr = std::lower_bound(start, end, entry_value_t(entry),
      [](const entry_value_t & k1, const entry_value_t & k2) -> bool {
        return k1.entry < k2.entry;
      });

    return (itr == end ? nullptr : itr);

  } // lower_bound

  // for row 'index', return pointer to first entry not less
  // than 'entry'
  const entry_value_t * lower_bound(size_t index, size_t entry) const {
    auto & handle = base_t::handle;
    assert(index < handle.num_total_ && "sparse accessor: index out of bounds");

    const vector_t & row = handle.new_entries[index];
    auto start = row.begin();
    auto end = row.end();

    // find where entry should be
    auto itr = std::lower_bound(start, end, entry_value_t(entry),
      [](const entry_value_t & k1, const entry_value_t & k2) -> bool {
        return k1.entry < k2.entry;
      });

    return (itr == end ? nullptr : itr);

  } // lower_bound

  // for row 'index', test whether entry 'entry' is present
  bool contains(size_t index, size_t entry) const {
    auto itr = lower_bound(index, entry);
    return (itr && itr->entry == entry);
  } // contains

  //-------------------------------------------------------------------------//
  //! Return all entries used over all indices.
  //-------------------------------------------------------------------------//
  index_space_t entries() const {
    auto & handle = base_t::handle;
    size_t id = 0;
    index_space_t is;
    std::unordered_set<size_t> found;

    for(size_t index = 0; index < handle.num_total_; ++index) {
      const auto & row = handle.new_entries[index];

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
    auto & handle = base_t::handle;
    clog_assert(
      index < handle.num_total_, "sparse accessor: index out of bounds");

    index_space_t is;

    size_t id = 0;
    const auto & row = handle.new_entries[index];
    for(const auto & ev : row) {
      is.push_back({id++, ev.entry});
    }

    return is;
  }

  //-------------------------------------------------------------------------//
  //! Return all indices allocated.
  //-------------------------------------------------------------------------//
  index_space_t indices() const {
    auto & handle = base_t::handle;
    index_space_t is;
    size_t id = 0;

    for(size_t index = 0; index < handle.num_total_; ++index) {
      const auto & row = handle.new_entries[index];
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
    auto & handle = base_t::handle;
    index_space_t is;
    size_t id = 0;

    for(size_t index = 0; index < handle.num_total_; ++index) {
      auto itr = lower_bound(index, entry);
      if(itr && itr->entry == entry) {
        is.push_back({id++, index});
      }
    }

    return is;
  }

  void dump() const {
    auto & handle = base_t::handle;
    for(size_t i = 0; i < handle.num_total_; ++i) {
      const auto & row = handle.new_entries[index];
      std::cout << "index: " << i << std::endl;
      for(size_t j = 0; j < row.size(); ++j) {
        std::cout << "  " << row[j].entry << " = " << row[j].value << std::endl;
      }
    }
  }

  //-------------------------------------------------------------------------//
  //! Return the maximum possible entries
  //-------------------------------------------------------------------------//
  auto max_entries() const noexcept {
    auto & handle = base_t::handle;
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
