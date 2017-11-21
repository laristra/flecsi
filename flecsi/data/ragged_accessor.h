/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_ragged_accessor_h
#define flecsi_ragged_accessor_h

#include "flecsi/data/sparse_data_handle.h"

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Nov 21, 2017
//----------------------------------------------------------------------------//

namespace flecsi {

//----------------------------------------------------------------------------//
//! The ragged_accessor_base_t type provides an empty base type for 
//! compile-time identification of data handle objects.
//!
//! @ingroup data
//----------------------------------------------------------------------------//

struct ragged_accessor_base_t {};

//----------------------------------------------------------------------------//
//! The ragged_accessor type captures information about permissions
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

template<
  typename T,
  size_t EXCLUSIVE_PERMISSIONS,
  size_t SHARED_PERMISSIONS,
  size_t GHOST_PERMISSIONS
>
struct ragged_accessor : public ragged_accessor_base_t {
  using handle_t = 
    sparse_data_handle__<
      T,
      EXCLUSIVE_PERMISSIONS,
      SHARED_PERMISSIONS,
      GHOST_PERMISSIONS
    >;

  using offset_t = typename handle_t::offset_t;
  using entry_value_t = typename handle_t::entry_value_t;

  //--------------------------------------------------------------------------//
  //! Copy constructor.
  //--------------------------------------------------------------------------//

  ragged_accessor(const sparse_data_handle__<T, 0, 0, 0>& h)
  : handle(reinterpret_cast<const handle_t&>(h)){

  }

  T &
  operator () (
    size_t index,
    size_t ragged_index
  )
  {
    assert(index < handle.num_total_ && 
      "sparse accessor: index out of bounds");

    const offset_t& oi = handle.offsets[index];

    entry_value_t * start = handle.entries + oi.start();
    assert(ragged_index < oi.count());

    return (start + ragged_index)->value;
  } // operator ()

  handle_t handle;  
};

} // namespace flecsi

#endif // flecsi_ragged_accessor_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
