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
 The global_data_handle_base_t type provides an empty base type for compile-time
 identification of color and global data handle objects.

 @ingroup data
 */

struct global_data_handle_base_t {};

/*!
 The global_data_handle_base_u type captures information about permissions
 and specifies a data policy.

 @tparam T                     The data type referenced by the handle.
 @tparam PERMISSIONS The permissions required on the exclusive
                               indices of the index partition.
 @tparam DATA_POLICY           The data policy for this handle type.

 @ingroup data
 */

template<typename T, size_t PERMISSIONS, typename DATA_POLICY>
struct global_data_handle_base_u : public DATA_POLICY,
                                   public global_data_handle_base_t {

  /*!
    Capture the underlying data type.
   */
  using value_type = T;

  /*!
   Default constructor.
   */

  global_data_handle_base_u() {}

  /*!
    Copy constructor.
   */

  global_data_handle_base_u(const global_data_handle_base_u & b)
    : DATA_POLICY(b) {
    combined_data = b.combined_data;
    combined_size = b.combined_size;
    master = false;
    state = b.state;
    global = b.global;
    color = b.color;
  }

  T * combined_data = nullptr;
  T * color_buf = nullptr;
  size_t color_size = 1;

  size_t combined_size = 0;
  bool master = true;

  size_t state = 0;
  bool global = false;
  bool color = false;
};

} // namespace flecsi

#include <flecsi/runtime/flecsi_runtime_data_handle_policy.h>

namespace flecsi {

/*!
 The global_data_handle_u type is the high-level data handle type.

 @tparam T                     The data type referenced by the handle.
 @tparam PERMISSIONS           The permissions required on the exclusive
                               indices of the index partition.
 @tparam DATA_POLICY           The data policy for this handle type.

 @ingroup data
 */

template<typename T, size_t PERMISSIONS>
using global_data_handle_u = global_data_handle_base_u<T,
  PERMISSIONS,
  FLECSI_RUNTIME_GLOBAL_DATA_HANDLE_POLICY>;

} // namespace flecsi
