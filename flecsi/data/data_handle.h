/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_data_handle_h
#define flecsi_data_handle_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Aug 01, 2016
//----------------------------------------------------------------------------//

namespace flecsi {

//----------------------------------------------------------------------------//
//! The data_handle_base_t type provides an empty base type for compile-time
//! identification of data handle objects.
//!
//! @ingroup data
//----------------------------------------------------------------------------//

struct data_handle_base_t {};

//----------------------------------------------------------------------------//
//! The data_handle_base__ type captures information about permissions
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
  size_t GHOST_PERMISSIONS,
  typename DATA_POLICY
>
struct data_handle_base__ : public DATA_POLICY, public data_handle_base_t {
  T* exclusive_data;
  size_t exclusive_size;
  
  T* shared_data;
  size_t shared_size;
  
  T* ghost_data;
  size_t ghost_size;
};

} // namespace flecsi

#include "flecsi_runtime_data_handle_policy.h"

namespace flecsi {
  
  //--------------------------------------------------------------------------//
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
  //--------------------------------------------------------------------------//

  template<
    typename T,
    size_t EXCLUSIVE_PERMISSIONS,
    size_t SHARED_PERMISSIONS,
    size_t GHOST_PERMISSIONS
  >
  using data_handle__ = data_handle_base__<
    T,
    EXCLUSIVE_PERMISSIONS,
    SHARED_PERMISSIONS,
    GHOST_PERMISSIONS,
    FLECSI_RUNTIME_DATA_HANDLE_POLICY
  >;

} // namespace flecsi

#endif // flecsi_data_handle_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
