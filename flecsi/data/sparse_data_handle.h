/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_sparse_data_handle_h
#define flecsi_sparse_data_handle_h

#include "flecsi/data/data_handle.h"

#include "flecsi/data/common/data_types.h"

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Oct 06, 2017
//----------------------------------------------------------------------------//

namespace flecsi {

template<
  typename T,
  size_t EXCLUSIVE_PERMISSIONS,
  size_t SHARED_PERMISSIONS,
  size_t GHOST_PERMISSIONS,
  typename DATA_POLICY
>
struct sparse_data_handle_base__ : 
  public DATA_POLICY, public data_handle_base_t {

  using offset_t = data::sparse_data_offset_t;
  using entry_value_t = data::sparse_entry_value__<T>;

  size_t index_space;
  size_t data_client_hash;
  size_t max_entries_per_index;

  entry_value_t* entries = nullptr;
  offset_t* offsets;

  //--------------------------------------------------------------------------//
  //! Default constructor.
  //--------------------------------------------------------------------------//

  sparse_data_handle_base__()
  {}

  sparse_data_handle_base__(
    size_t num_exclusive,
    size_t num_shared,
    size_t num_ghost
  )
  : num_exclusive_(num_exclusive),
  num_shared_(num_shared),
  num_ghost_(num_ghost),
  num_total_(num_exclusive_ + num_shared_ + num_ghost_){}

  //--------------------------------------------------------------------------//
  //! Copy constructor.
  //--------------------------------------------------------------------------//

  sparse_data_handle_base__(const sparse_data_handle_base__& b)
  : DATA_POLICY(b),
  index_space(b.index_space),
  data_client_hash(b.data_client_hash),
  max_entries_per_index(b.max_entries_per_index),
  entries(b.entries),
  offsets(b.offsets),
  num_exclusive_(b.num_exclusive_),
  num_shared_(b.num_shared_),
  num_ghost_(b.num_ghost_){}

  T &
  operator () (
    size_t index,
    size_t entry
  )
  {
    assert(index < num_total_ && "sparse handle: index out of bounds");

    const offset_t& oi = offsets[index];

    entry_value_t * start = entries + oi.start();
    entry_value_t * end = start + oi.count();

    entry_value_t * itr =
      std::lower_bound(start, end, entry_value_t(entry),
        [](const entry_value_t & k1, const entry_value_t & k2) -> bool {
          return k1.entry < k2.entry;
        });

    assert(itr != end && "sparse handle: unmapped entry");

    return itr->value;
  } // operator ()

private:
  size_t num_exclusive_;
  size_t num_shared_;
  size_t num_ghost_;
  size_t num_total_;
};

} // namespace flecsi

#include "flecsi/runtime/flecsi_runtime_data_handle_policy.h"

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
  size_t GHOST_PERMISSIONS
>
using sparse_data_handle__ = sparse_data_handle_base__<
  T,
  EXCLUSIVE_PERMISSIONS,
  SHARED_PERMISSIONS,
  GHOST_PERMISSIONS,
  FLECSI_RUNTIME_SPARSE_DATA_HANDLE_POLICY
>;

} // namespace flecsi

#endif // flecsi_sparse_data_handle_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
