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

template<
    typename T,
    size_t EXCLUSIVE_PERMISSIONS,
    size_t SHARED_PERMISSIONS,
    size_t GHOST_PERMISSIONS,
    typename DATA_POLICY>
struct sparse_data_handle_base__ : public DATA_POLICY{

  using offset_t = data::sparse_data_offset_t;
  using entry_value_t = data::sparse_entry_value__<T>;

  /*!
    Capture the underlying data type.
   */
  using value_type = T;

  size_t index_space;
  size_t data_client_hash;

  entry_value_t * entries = nullptr;
  offset_t * offsets = nullptr;

  //--------------------------------------------------------------------------//
  //! Default constructor.
  //--------------------------------------------------------------------------//

  sparse_data_handle_base__() {}

  sparse_data_handle_base__(
      size_t num_exclusive,
      size_t num_shared,
      size_t num_ghost)
      : num_exclusive_(num_exclusive), num_shared_(num_shared),
        num_ghost_(num_ghost),
        num_total_(num_exclusive_ + num_shared_ + num_ghost_) {}

  //--------------------------------------------------------------------------//
  //! Copy constructor.
  //--------------------------------------------------------------------------//

  sparse_data_handle_base__(const sparse_data_handle_base__ & b)
      : DATA_POLICY(b), index_space(b.index_space),
        data_client_hash(b.data_client_hash),
        entries(b.entries),
        offsets(b.offsets), num_exclusive_(b.num_exclusive_),
        num_shared_(b.num_shared_), num_ghost_(b.num_ghost_),
        num_total_(b.num_total_){}

  void init(size_t num_exclusive,
            size_t num_shared,
            size_t num_ghost){
    num_exclusive_ = num_exclusive;
    num_shared_ = num_shared;
    num_ghost_ = num_ghost;
    num_total_ = num_exclusive_ + num_shared_ + num_ghost_;
  }

  size_t num_exclusive() const{
    return num_exclusive_;
  }

  size_t num_shared() const{
    return num_shared_;
  }

  size_t num_ghost() const{
    return num_ghost_;
  }

  size_t num_exclusive_;
  size_t num_shared_;
  size_t num_ghost_;
  size_t num_total_;
};



} // namespace flecsi

#include <flecsi/runtime/flecsi_runtime_data_handle_policy.h>

namespace flecsi {

//----------------------------------------------------------------------------//
//! The data_handle__ type is the high-level data handle type.
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
using sparse_data_handle__ = sparse_data_handle_base__<
    T,
    EXCLUSIVE_PERMISSIONS,
    SHARED_PERMISSIONS,
    GHOST_PERMISSIONS,
    FLECSI_RUNTIME_SPARSE_DATA_HANDLE_POLICY>;

} // namespace flecsi
