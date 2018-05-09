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

#include <unordered_set>

#include <cinchlog.h>

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
//! The sparse accessor__ type captures information about permissions
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

template<
    typename T,
    size_t EXCLUSIVE_PERMISSIONS,
    size_t SHARED_PERMISSIONS,
    size_t GHOST_PERMISSIONS>
struct accessor__<
    data::sparse,
    T,
    EXCLUSIVE_PERMISSIONS,
    SHARED_PERMISSIONS,
    GHOST_PERMISSIONS>
    : public accessor__<
          data::base,
          T,
          EXCLUSIVE_PERMISSIONS,
          SHARED_PERMISSIONS,
          GHOST_PERMISSIONS>,
      public sparse_accessor_base_t {
  using handle_t = sparse_data_handle__<
      T,
      EXCLUSIVE_PERMISSIONS,
      SHARED_PERMISSIONS,
      GHOST_PERMISSIONS>;

  using offset_t = typename handle_t::offset_t;
  using entry_value_t = typename handle_t::entry_value_t;

  using index_space_t =
      topology::index_space__<topology::simple_entry__<size_t>, true>;

  //-------------------------------------------------------------------------//
  //! Copy constructor.
  //-------------------------------------------------------------------------//

  accessor__(const sparse_data_handle__<T, 0, 0, 0> & h)
      : handle(reinterpret_cast<const handle_t &>(h)) {}

  T & operator()(size_t index, size_t entry) {
    assert(index < handle.num_total_ && "sparse accessor: index out of bounds");

    const offset_t & oi = handle.offsets[index];

    entry_value_t * start = handle.entries + oi.start();
    entry_value_t * end = start + oi.count();

    entry_value_t * itr = std::lower_bound(
        start, end, entry_value_t(entry),
        [](const entry_value_t & k1, const entry_value_t & k2) -> bool {
          return k1.entry < k2.entry;
        });

    assert(itr != end && "sparse accessor: unmapped entry");

    return itr->value;
  } // operator ()

  //-------------------------------------------------------------------------//
  //! Return all entries used over all indices.
  //-------------------------------------------------------------------------//
  index_space_t entries() const {
    size_t id = 0;
    index_space_t is;
    std::unordered_set<size_t> found;

    for (size_t index = 0; index < handle.num_total_; ++index) {
      const offset_t & oi = handle.offsets[index];

      entry_value_t * itr = handle.entries + oi.start();
      entry_value_t * end = itr + oi.count();

      while (itr != end) {
        size_t entry = itr->entry;
        if (found.find(entry) == found.end()) {
          is.push_back({id++, entry});
          found.insert(entry);
        }
        ++itr;
      }
    }

    return is;
  }

  //-------------------------------------------------------------------------//
  //! Return all entries used over the specified index.
  //-------------------------------------------------------------------------//
  index_space_t entries(size_t index) const {
    clog_assert(
        index < handle.num_total_, "sparse accessor: index out of bounds");

    const offset_t & oi = handle.offsets[index];

    entry_value_t * itr = handle.entries + oi.start();
    entry_value_t * end = itr + oi.count();

    index_space_t is;

    size_t id = 0;
    while (itr != end) {
      is.push_back({id++, itr->entry});
      ++itr;
    }

    return is;
  }

  //-------------------------------------------------------------------------//
  //! Return all indices allocated.
  //-------------------------------------------------------------------------//
  index_space_t indices() const {
    index_space_t is;
    size_t id = 0;

    for (size_t index = 0; index < handle.num_total_; ++index) {
      const offset_t & oi = handle.offsets[index];

      if (oi.count() != 0) {
        is.push_back({id++, index});
      }
    }

    return is;
  }

  //-------------------------------------------------------------------------//
  //! Return all indices allocated for a given entry.
  //-------------------------------------------------------------------------//
  index_space_t indices(size_t entry) const {
    index_space_t is;
    size_t id = 0;

    for (size_t index = 0; index < handle.num_total_; ++index) {
      const offset_t & oi = handle.offsets[index];

      entry_value_t * start = handle.entries + oi.start();
      entry_value_t * end = start + oi.count();

      if (std::binary_search(
              start, end, entry_value_t(entry),
              [](const auto & k1, const auto & k2) -> bool {
                return k1.entry < k2.entry;
              })) {
        is.push_back({id++, index});
      }
    }

    return is;
  }

  void dump() const {
    for (size_t i = 0; i < handle.num_total_; ++i) {
      const offset_t & offset = handle.offsets[i];
      std::cout << "index: " << i << std::endl;
      // std::cout << "offset: " << offset.start() << std::endl;
      for (size_t j = 0; j < offset.count(); ++j) {
        size_t k = offset.start() + j;
        std::cout << "  " << handle.entries[k].entry << " = "
                  << handle.entries[k].value << std::endl;
      }
    }
  }

  handle_t handle;
};

template<
    typename T,
    size_t EXCLUSIVE_PERMISSIONS,
    size_t SHARED_PERMISSIONS,
    size_t GHOST_PERMISSIONS>
using sparse_accessor__ = accessor__<
    data::sparse,
    T,
    EXCLUSIVE_PERMISSIONS,
    SHARED_PERMISSIONS,
    GHOST_PERMISSIONS>;

template<
    typename T,
    size_t EXCLUSIVE_PERMISSIONS,
    size_t SHARED_PERMISSIONS,
    size_t GHOST_PERMISSIONS>
using sparse_accessor = sparse_accessor__<
    T,
    EXCLUSIVE_PERMISSIONS,
    SHARED_PERMISSIONS,
    GHOST_PERMISSIONS>;

} // namespace flecsi
