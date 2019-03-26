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

namespace flecsi {

/*!
 The dense_data_handle_base_t type provides an empty base type for compile-time
 identification of data handle objects.

 @ingroup data
 */

struct dense_data_handle_base_t {};

/*!
 The dense_data_handle_base_u type captures information about permissions
 and specifies a data policy.

 @tparam T                     The data type referenced by the handle.
 @tparam EXCLUSIVE_PERMISSIONS The permissions required on the exclusive
                               indices of the index partition.
 @tparam SHARED_PERMISSIONS    The permissions required on the shared
                               indices of the index partition.
 @tparam GHOST_PERMISSIONS     The permissions required on the ghost
                               indices of the index partition.
 @tparam DATA_POLICY           The data policy for this handle type.

 @ingroup data
 */

template<typename T,
  size_t EXCLUSIVE_PERMISSIONS,
  size_t SHARED_PERMISSIONS,
  size_t GHOST_PERMISSIONS,
  typename DATA_POLICY>
struct dense_data_handle_base_u : public DATA_POLICY,
                                  public dense_data_handle_base_t {

  /*!
    Capture the underlying data type.
   */
  using value_type = T;

  T * exclusive_data = nullptr;
  T * exclusive_buf = nullptr;
  size_t exclusive_size = 0;

  T * shared_data = nullptr;
  T * shared_buf = nullptr;
  size_t shared_size = 0;

  T * ghost_data = nullptr;
  T * ghost_buf = nullptr;
  size_t ghost_size = 0;

  T * combined_data = nullptr;
#ifdef COMPACTED_STORAGE_SORT
  T * combined_data_sort = nullptr;
#endif
  size_t combined_size = 0;

  size_t state = 0;
  bool global = false;
  bool color = false;
};

} // namespace flecsi

#include <flecsi/runtime/flecsi_runtime_data_handle_policy.h>

namespace flecsi {

/*!
 The dense_data_handle_u type is the high-level data handle type.

 @tparam T                     The data type referenced by the handle.
 @tparam EXCLUSIVE_PERMISSIONS The permissions required on the exclusive
                               indices of the index partition.
 @tparam SHARED_PERMISSIONS    The permissions required on the shared
                               indices of the index partition.
 @tparam GHOST_PERMISSIONS     The permissions required on the ghost
                               indices of the index partition.
 @tparam DATA_POLICY           The data policy for this handle type.

 @ingroup data
 */

template<typename T,
  size_t EXCLUSIVE_PERMISSIONS,
  size_t SHARED_PERMISSIONS,
  size_t GHOST_PERMISSIONS>
using dense_data_handle_u = dense_data_handle_base_u<T,
  EXCLUSIVE_PERMISSIONS,
  SHARED_PERMISSIONS,
  GHOST_PERMISSIONS,
  FLECSI_RUNTIME_DENSE_DATA_HANDLE_POLICY>;

} // namespace flecsi
