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
  T* exclusive_data = nullptr;
  T* exclusive_buf = nullptr;
  size_t exclusive_size = 0;
  
  T* shared_data = nullptr;
  T* shared_buf = nullptr;
  size_t shared_size = 0;
  
  T* ghost_data = nullptr;
  T* ghost_buf = nullptr;
  size_t ghost_size = 0;

  T* combined_data = nullptr;
  size_t combined_size = 0;
  bool master = true;

  template<
    size_t EXCLUSIVE_PERMISSIONS2,
    size_t SHARED_PERMISSIONS2,
    size_t GHOST_PERMISSIONS2
  >
  void
  copy_data(
    const data_handle_base__<
      T,
      EXCLUSIVE_PERMISSIONS2,
      SHARED_PERMISSIONS2,
      GHOST_PERMISSIONS2,
      DATA_POLICY
    >& b
  )
  {
    exclusive_data = b.exclusive_data;
    shared_data = b.shared_data;
    ghost_data = b.ghost_data;
    combined_data = b.combined_data;
    exclusive_size = b.exclusive_size;
    shared_size = b.shared_size;
    ghost_size = b.ghost_size;
    master = false;
  }   
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
