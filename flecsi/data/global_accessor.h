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

#include <flecsi/data/accessor.h>
#include <flecsi/data/data_constants.h>
#include <flecsi/data/dense_data_handle.h>
#include <flecsi/data/global_data_handle.h>
#include <flecsi/utils/target.h>

/*!
 @file
 @date Initial file creation: Nov 13, 2017
 */

namespace flecsi {

/*!
 The global_accessor_base_t type provides an empty base type for compile-time
 identification of data handle objects.

 @ingroup data
 */

struct global_accessor_base_t {};

/*!
 The global accessor_u type captures information about permissions
 and specifies a data policy.

 @tparam T                     The data type referenced by the handle.
 @tparam PERMISSIONS           The permissions required on the exclusive
                               indices of the index partition.
 @tparam DATA_POLICY           The data policy for this handle type.

 @ingroup data
 */

template<typename T, size_t PERMISSIONS>
struct accessor_u<data::global, T, PERMISSIONS, 0, 0>
  : public accessor_u<data::base, T, PERMISSIONS, 0, 0>,
    public global_accessor_base_t {

  using handle_t = global_data_handle_u<T, PERMISSIONS>;

  accessor_u(const global_data_handle_u<T, 0> & h)
    : handle(reinterpret_cast<const handle_t &>(h)) {}

  FLECSI_INLINE_TARGET
  operator T &() {
    return data();
  }

  FLECSI_INLINE_TARGET
  operator const T &() const {
    return data();
  }

  FLECSI_INLINE_TARGET
  T & data() const {
    return *handle.combined_data;
  } // data

  FLECSI_INLINE_TARGET
  size_t size() {
    return 1;
  } // size

  FLECSI_INLINE_TARGET
  accessor_u & operator=(const T & x) {
    data() = x;
    return *this;
  } // operator =

  FLECSI_INLINE_TARGET
  T * operator&() const {
    return handle.combined_data;
  }

  //--------------------------------------------------------------------------/
  //
  // Operators.
  //--------------------------------------------------------------------------/

  /*!
   \brief Provide logical array-based access to the data for this
          data variable.  This is the const operator version.

   \tparam E A complex index type.

   This version of the operator is provided to support use with
   \e flecsi mesh entity types \ref mesh_entity_base_t.
   */
  template<typename E>
  const T & operator()(E * e) const {
    return this->operator()(e->template id<0>());
  } // operator ()

  /*!
   \brief Provide logical array-based access to the data for this
          data variable.  This is the const operator version.

   \tparam E A complex index type.

   This version of the operator is provided to support use with
   \e flecsi mesh entity types \ref mesh_entity_base_t.
   */

  template<typename E>
  T & operator()(E * e) {
    return this->operator()(e->template id<0>());
  } // operator ()

  handle_t handle;
};

template<typename T, size_t PERMISSIONS>
struct accessor_u<data::color, T, PERMISSIONS, 0, 0>
  : public accessor_u<data::base, T, PERMISSIONS, 0, 0> {

  using handle_t = global_data_handle_u<T, PERMISSIONS>;

  accessor_u(const global_data_handle_u<T, 0> & h)
    : handle(reinterpret_cast<const handle_t &>(h)) {}

  FLECSI_INLINE_TARGET
  operator T &() {
    return data();
  }

  FLECSI_INLINE_TARGET
  operator const T &() const {
    return data();
  }

  FLECSI_INLINE_TARGET
  T & data() const {
    return *handle.combined_data;
  }

  FLECSI_INLINE_TARGET
  size_t size() {
    return 1;
  }

  FLECSI_INLINE_TARGET
  accessor_u & operator=(const T & x) {
    data() = x;
    return *this;
  }

  FLECSI_INLINE_TARGET
  T & operator=(accessor_u) {
    return *handle.combined_data;
  }

  FLECSI_INLINE_TARGET
  T * operator&() const {
    return handle.combined_data;
  }

  //--------------------------------------------------------------------------/
  //
  // Operators.
  //--------------------------------------------------------------------------/
  /*!
   \brief Provide logical array-based access to the data for this
          data variable.  This is the const operator version.

   \tparam E A complex index type.

   This version of the operator is provided to support use with
   \e flecsi mesh entity types \ref mesh_entity_base_t.
   */
  template<typename E>
  const T & operator()(E * e) const {
    return this->operator()(e->template id<0>());
  } // operator ()

  /*!
   \brief Provide logical array-based access to the data for this
          data variable.  This is the const operator version.

   \tparam E A complex index type.

   This version of the operator is provided to support use with
   \e flecsi mesh entity types \ref mesh_entity_base_t.
   */
  template<typename E>
  T & operator()(E * e) {
    return this->operator()(e->template id<0>());
  } // operator ()

  handle_t handle;
};

template<typename T, size_t PERMISSIONS>
using color_accessor_u = accessor_u<data::color, T, PERMISSIONS, 0, 0>;

template<typename T, size_t PERMISSIONS>
using global_accessor_u = accessor_u<data::global, T, PERMISSIONS, 0, 0>;

// TODO: these definitions need to be part of the specialization
template<typename T, size_t PERMISSIONS>
using global_accessor = global_accessor_u<T, PERMISSIONS>;

template<typename T, size_t PERMISSIONS>
using color_accessor = color_accessor_u<T, PERMISSIONS>;

} // namespace flecsi
