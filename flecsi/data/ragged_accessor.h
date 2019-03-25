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

  using offset_t = typename handle_t::offset_t;
  using value_t = T;

  //--------------------------------------------------------------------------//
  //! Constructor from handle.
  //--------------------------------------------------------------------------//

  accessor_u() = default;

  accessor_u(const ragged_data_handle_u<T, 0, 0, 0> & h)
    : handle(reinterpret_cast<const handle_t &>(h)) {}

  T & operator()(size_t index, size_t ragged_index) {
    const offset_t & offset = handle.offsets[index];
    assert(
      ragged_index < offset.count() && "ragged accessor: index out of range");

    return handle.entries[offset.start() + ragged_index];
  } // operator ()

  //-------------------------------------------------------------------------//
  //! Return max number of entries used over all indices.
  //-------------------------------------------------------------------------//
  size_t size() const {
    size_t max_so_far = 0;

    for(size_t index = 0; index < handle.num_total_; ++index) {
      const offset_t & oi = handle.offsets[index];
      max_so_far = std::max(max_so_far, oi.count());
    }

    return max_so_far;
  }

  //-------------------------------------------------------------------------//
  //! Return number of entries used over the specified index.
  //-------------------------------------------------------------------------//
  size_t size(size_t index) const {
    clog_assert(
      index < handle.num_total_, "ragged accessor: index out of bounds");

    const offset_t & oi = handle.offsets[index];
    return oi.count();
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
