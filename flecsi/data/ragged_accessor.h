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
  GHOST_PERMISSIONS> : public accessor_u<data::sparse,
                         T,
                         EXCLUSIVE_PERMISSIONS,
                         SHARED_PERMISSIONS,
                         GHOST_PERMISSIONS>,
                       public ragged_accessor_base_t {

  using base_t = accessor_u<data::sparse,
    T,
    EXCLUSIVE_PERMISSIONS,
    SHARED_PERMISSIONS,
    GHOST_PERMISSIONS>;

  using offset_t = typename base_t::offset_t;

  //--------------------------------------------------------------------------//
  //! Copy constructor.
  //--------------------------------------------------------------------------//

  accessor_u(const sparse_data_handle_u<T, 0, 0, 0> & h) : base_t(h) {}

  T & operator()(size_t index, size_t ragged_index) {
    const offset_t & offset = base_t::handle.offsets[index];
    assert(
      ragged_index < offset.count() && "ragged accessor: index out of range");

    return (base_t::handle.entries + offset.start() + ragged_index)->value;
  } // operator ()
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
