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

#include <cinchlog.h>

#include <flecsi/data/accessor.h>
#include <flecsi/data/sparse_data_handle.h>
#include <flecsi/topology/index_space.h>

namespace flecsi {

//----------------------------------------------------------------------------//
//! The ragged_accessor_base_t type provides an empty base type for
//! compile-time identification of data handle objects.
//!
//! @ingroup data
//----------------------------------------------------------------------------//

struct ragged_accessor_base_t {};

//----------------------------------------------------------------------------//
//! The ragged accessor_u type captures information about permissions
//! and specifies a data policy.
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
struct accessor_u<data::ragged,
  T,
  EXCLUSIVE_PERMISSIONS,
  SHARED_PERMISSIONS,
  GHOST_PERMISSIONS> : public accessor_u<data::base,
                         T,
                         EXCLUSIVE_PERMISSIONS,
                         SHARED_PERMISSIONS,
                         GHOST_PERMISSIONS>,
                       public ragged_accessor_base_t {
  using handle_t = ragged_data_handle_u<T,
    EXCLUSIVE_PERMISSIONS,
    SHARED_PERMISSIONS,
    GHOST_PERMISSIONS>;

  using value_t = T;
  using vector_t = typename handle_t::vector_t;

  using index_space_t =
    topology::index_space_u<topology::simple_entry_u<size_t>, true>;

  //--------------------------------------------------------------------------//
  //! Constructor from handle.
  //--------------------------------------------------------------------------//

  accessor_u(const ragged_data_handle_u<T, 0, 0, 0> & h)
    : handle(reinterpret_cast<const handle_t &>(h)) {}

  T & operator()(size_t index, size_t ragged_index) {
    vector_t & row = handle.new_entries[index];
    assert(
      ragged_index < row.size() && "ragged accessor: index out of range");

    return row[ragged_index];
  } // operator ()

  const T & operator()(size_t index, size_t ragged_index) const {
    const vector_t & row = handle.new_entries[index];
    assert(
      ragged_index < row.size() && "ragged accessor: index out of range");

    return row[ragged_index];
  } // operator ()

  //-------------------------------------------------------------------------//
  //! Return max number of entries used over all indices.
  //-------------------------------------------------------------------------//
  size_t size() const {
    size_t max_so_far = 0;

    for(size_t index = 0; index < handle.num_total_; ++index) {
      const vector_t & row = handle.new_entries[index];
      max_so_far = std::max(max_so_far, row.size());
    }

    return max_so_far;
  }

  //-------------------------------------------------------------------------//
  //! Return number of entries used over the specified index.
  //-------------------------------------------------------------------------//
  size_t size(size_t index) const {
    clog_assert(
      index < handle.num_total_, "ragged accessor: index out of bounds");

    const vector_t & row = handle.new_entries[index];
    return row.size();
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
    return handle.max_entries_per_index;
  }

  handle_t handle;
};

template<typename T,
  size_t EXCLUSIVE_PERMISSIONS,
  size_t SHARED_PERMISSIONS,
  size_t GHOST_PERMISSIONS>
using ragged_accessor_u = accessor_u<data::ragged,
  T,
  EXCLUSIVE_PERMISSIONS,
  SHARED_PERMISSIONS,
  GHOST_PERMISSIONS>;

template<typename T,
  size_t EXCLUSIVE_PERMISSIONS,
  size_t SHARED_PERMISSIONS,
  size_t GHOST_PERMISSIONS>
using ragged_accessor = ragged_accessor_u<T,
  EXCLUSIVE_PERMISSIONS,
  SHARED_PERMISSIONS,
  GHOST_PERMISSIONS>;

} // namespace flecsi
