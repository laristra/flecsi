/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_sparse_data_handle_h
#define flecsi_sparse_data_handle_h

#include "flecsi/data/data_handle.h"

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Oct 06, 2017
//----------------------------------------------------------------------------//

namespace flecsi {

template<typename T>
struct sparse_entry_value__{
  size_t entry;
  T value;
};

template<
  typename T,
  size_t EXCLUSIVE_PERMISSIONS,
  size_t SHARED_PERMISSIONS,
  size_t GHOST_PERMISSIONS,
  typename DATA_POLICY
>
struct sparse_data_handle_base__ : 
  public DATA_POLICY, public data_handle_base_t {

  //--------------------------------------------------------------------------//
  //! Default constructor.
  //--------------------------------------------------------------------------//

  sparse_data_handle_base__()
  {}

  //--------------------------------------------------------------------------//
  //! Copy constructor.
  //--------------------------------------------------------------------------//

  sparse_data_handle_base__(const sparse_data_handle_base__& b)
  : DATA_POLICY(b),
  index_space(b.index_space),
  data_client_hash(b.data_client_hash),
  entries(b.entries),
  indices(b.indices),
  ghost_entries(b.ghost_entries){
  
  }

  size_t index_space;
  size_t data_client_hash;

  sparse_entry_value__<T>* entries = nullptr;
  size_t* indices;

  sparse_entry_value__<T>* ghost_entries = nullptr;
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
