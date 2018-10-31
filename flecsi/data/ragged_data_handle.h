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

#include <flecsi/data/common/data_types.h>

namespace flecsi {

//----------------------------------------------------------------------------//
//! @brief A ragged data handle contains a variable number of entries for each
//! index.
//!
//! Ragged data can be thought of as a std::vector of entries of type T and
//! variable length for each index. Entries can be removed or added on a
//! per-index basis.
//!
//! @tparam T
//! @tparam EXCLUSIVE_PERMISSIONS
//! @tparam SHARED_PERMISSIONS
//! @tparam GHOST_PERMISSIONS
//! @tparam DATA_POLICY
//----------------------------------------------------------------------------//
template<
    typename T,
    size_t EXCLUSIVE_PERMISSIONS,
    size_t SHARED_PERMISSIONS,
    size_t GHOST_PERMISSIONS,
    typename DATA_POLICY>
struct ragged_data_handle_base_u : public DATA_POLICY {

  using offset_t = data::sparse_data_offset_t;

  /*!
    Capture the underlying data type.
   */
  using value_type = T;

  size_t index_space;
  size_t data_client_hash;

  T * entries = nullptr;
  offset_t * offsets = nullptr;

  //--------------------------------------------------------------------------//
  //! Default constructor.
  //--------------------------------------------------------------------------//

  ragged_data_handle_base_u() {}

  ragged_data_handle_base_u(
      size_t num_exclusive,
      size_t num_shared,
      size_t num_ghost)
      : num_exclusive_(num_exclusive), num_shared_(num_shared),
        num_ghost_(num_ghost),
        num_total_(num_exclusive_ + num_shared_ + num_ghost_) {}

  //--------------------------------------------------------------------------//
  //! Copy constructor.
  //--------------------------------------------------------------------------//

  ragged_data_handle_base_u(const ragged_data_handle_base_u & b)
      : DATA_POLICY(b), index_space(b.index_space),
        data_client_hash(b.data_client_hash), entries(b.entries),
        offsets(b.offsets), num_exclusive_(b.num_exclusive_),
        num_shared_(b.num_shared_), num_ghost_(b.num_ghost_),
        num_total_(b.num_total_) {}

  void init(size_t num_exclusive, size_t num_shared, size_t num_ghost) {
    num_exclusive_ = num_exclusive;
    num_shared_ = num_shared;
    num_ghost_ = num_ghost;
    num_total_ = num_exclusive_ + num_shared_ + num_ghost_;
  }

  size_t num_exclusive() const {
    return num_exclusive_;
  }

  size_t num_shared() const {
    return num_shared_;
  }

  size_t num_ghost() const {
    return num_ghost_;
  }

private:
  size_t num_exclusive_;
  size_t num_shared_;
  size_t num_ghost_;
  size_t num_total_;
};

} // namespace flecsi

#include <flecsi/runtime/flecsi_runtime_data_handle_policy.h>

namespace flecsi {

//----------------------------------------------------------------------------//
//! The data_handle_u type is the high-level data handle type.
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
using ragged_data_handle_u = ragged_data_handle_base_u<
    T,
    EXCLUSIVE_PERMISSIONS,
    SHARED_PERMISSIONS,
    GHOST_PERMISSIONS,
    FLECSI_RUNTIME_RAGGED_DATA_HANDLE_POLICY>;

} // namespace flecsi
