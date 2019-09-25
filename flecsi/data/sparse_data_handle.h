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

#include <flecsi/data/common/row_vector.h>

namespace flecsi {

template<typename T, typename DATA_POLICY>
struct ragged_data_handle_base_u : public DATA_POLICY {

  /*!
    Capture the underlying data type.
   */
  using value_type = T;

  using vector_t = data::row_vector_u<T>;

  //--------------------------------------------------------------------------//
  //! Default constructor.
  //--------------------------------------------------------------------------//

  ragged_data_handle_base_u() {}

  ragged_data_handle_base_u(size_t num_exclusive,
    size_t num_shared,
    size_t num_ghost)
    : num_exclusive_(num_exclusive), num_shared_(num_shared),
      num_ghost_(num_ghost),
      num_total_(num_exclusive_ + num_shared_ + num_ghost_) {}

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

  size_t num_exclusive_;
  size_t num_shared_;
  size_t num_ghost_;
  size_t num_total_;

  size_t index_space;
  size_t data_client_hash;
  std::size_t max_entries_per_index;

  vector_t * new_entries = nullptr;

}; // ragged_data_handle_base_u

} // namespace flecsi

#include <flecsi/runtime/flecsi_runtime_data_handle_policy.h>

namespace flecsi {

//----------------------------------------------------------------------------//
//! The data_handle_u type is the high-level data handle type.
//!
//! @tparam T                     The data type referenced by the handle.
//! @tparam DATA_POLICY           The data policy for this handle type.
//!
//! @ingroup data
//----------------------------------------------------------------------------//

template<typename T>
using ragged_data_handle_u =
  ragged_data_handle_base_u<T, FLECSI_RUNTIME_SPARSE_DATA_HANDLE_POLICY>;

template<typename T, typename DATA_POLICY>
struct sparse_data_handle_base_u
  : public ragged_data_handle_base_u<data::sparse_entry_value_u<T>,
      FLECSI_RUNTIME_SPARSE_DATA_HANDLE_POLICY> {

  using entry_value_t = data::sparse_entry_value_u<T>;
  using base_t = ragged_data_handle_base_u<entry_value_t,
    FLECSI_RUNTIME_SPARSE_DATA_HANDLE_POLICY>;

  /*!
    Capture the underlying data type.
   */
  using value_type = T;

  //--------------------------------------------------------------------------//
  //! Default constructor.
  //--------------------------------------------------------------------------//

  sparse_data_handle_base_u() {}

  sparse_data_handle_base_u(size_t num_exclusive,
    size_t num_shared,
    size_t num_ghost)
    : base_t(num_exclusive, num_shared, num_ghost) {}

  //--------------------------------------------------------------------------//
  //! Copy constructor.
  //--------------------------------------------------------------------------//

  sparse_data_handle_base_u(const sparse_data_handle_base_u & b) : base_t(b){};

}; // sparse_data_handle_base_u

template<typename T>
using sparse_data_handle_u =
  sparse_data_handle_base_u<T, FLECSI_RUNTIME_SPARSE_DATA_HANDLE_POLICY>;

} // namespace flecsi
