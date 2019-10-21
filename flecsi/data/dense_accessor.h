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
#include <flecsi/data/common/data_reference.h>
#include <flecsi/data/data_constants.h>
#include <flecsi/data/dense_data_handle.h>

/*!
 @file
 @date Initial file creation: Nov 13, 2017
 */

namespace flecsi {

/*!
 The dense_accessor_base_t type provides an empty base type for compile-time
 identification of data handle objects.

 @ingroup data
 */

struct dense_accessor_base_t {};

/*!
 The dense accessor_u type captures information about permissions
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
  size_t GHOST_PERMISSIONS>
struct accessor_u<data::dense,
  T,
  EXCLUSIVE_PERMISSIONS,
  SHARED_PERMISSIONS,
  GHOST_PERMISSIONS> : public accessor_u<data::base,
                         T,
                         EXCLUSIVE_PERMISSIONS,
                         SHARED_PERMISSIONS,
                         GHOST_PERMISSIONS>,
                       public dense_accessor_base_t {
  using handle_t = dense_data_handle_u<T,
    EXCLUSIVE_PERMISSIONS,
    SHARED_PERMISSIONS,
    GHOST_PERMISSIONS>;

  /*!
   Copy constructor.
   */

  accessor_u() = default;

  accessor_u(const dense_data_handle_u<T, 0, 0, 0> & h)
    : handle(reinterpret_cast<const handle_t &>(h)) {}

  /*!
   \brief Provide logical array-based access to the data for this
          data variable.  This is the const operator version.

   \param index The index of the data variable to return.
   */

  T & operator()(size_t index) {
    assert(index < handle.combined_size && "index out of range");
    return *(handle.combined_data + index);
  }

  /*!
   \brief Provide logical array-based access to the data for this
          data variable.  This is the const operator version.

   \param index The index of the data variable to return.
   */

  const T & operator()(size_t index) const {
    return const_cast<accessor_u &>(*this)(index);
  }

  /*!
   \brief Return the index space size of the data variable
          referenced by this handle.
   */
  size_t size() const {
    return handle.combined_size;
  }

  /*!
   \brief Return the index space size of the data variable
          referenced by this handle.
   */
  size_t exclusive_size() const {
    return handle.exclusive_size;
  } // size

  /*!
   \brief Return the index space size of the data variable
          referenced by this handle.
   */
  size_t shared_size() const {
    return handle.shared_size;
  } // size

  /*!
   \brief Return the index space size of the data variable
          referenced by this handle.
   */
  size_t ghost_size() const {
    return handle.ghost_size;
  } // size

  //--------------------------------------------------------------------------//
  // Operators.
  //--------------------------------------------------------------------------//

  /*!
   \brief Provide logical array-based access to the data for this
          data variable.  This is the const operator version.

   \param index The index of the data variable to return.
   */
  const T & exclusive(size_t index) const {
    assert(index < handle.exclusive_size && "index out of range");
    return handle.exclusive_data[index];
  } // operator []

  /*!
   \brief Provide logical array-based access to the data for this
          data variable.  This is the const operator version.

   \param index The index of the data variable to return.
   */
  T & exclusive(size_t index) {
    assert(index < handle.exclusive_size && "index out of range");
    return handle.exclusive_data[index];
  } // operator []

  /*!
   \brief Provide logical array-based access to the data for this
          data variable.  This is the const operator version.

   \param index The index of the data variable to return.
   */
  const T & shared(size_t index) const {
    assert(index < handle.shared_size && "index out of range");
    return handle.shared_data[index];
  } // operator []

  /*!
   \brief Provide logical array-based access to the data for this
          data variable.  This is the const operator version.

   \param index The index of the data variable to return.
  */
  T & shared(size_t index) {
    assert(index < handle.shared_size && "index out of range");
    return handle.shared_data[index];
  } // operator []

  /*
   \brief Provide logical array-based access to the data for this
          data variable.  This is the const operator version.

   \param index The index of the data variable to return.
  */
  const T & ghost(size_t index) const {
    assert(index < handle.ghost_size && "index out of range");
    return handle.ghost_data[index];
  } // operator []

  /*
   \brief Provide logical array-based access to the data for this
          data variable.  This is the const operator version.

   \param index The index of the data variable to return.
  */
  T & ghost(size_t index) {
    assert(index < handle.ghost_size && "index out of range");
    return handle.ghost_data[index];
  } // operator []

  /*!
   \brief Provide logical array-based access to the data for this
          data variable.  This is the const operator version.

   \tparam E A complex index type.

   This version of the operator is provided to support use with
   \e flecsi mesh entity types \ref mesh_entity_base_t.
   */
  template<typename E>
  const T & operator()(E * e) const {
    return this->operator()(e->id());
  } // operator ()

  /*!
   \brief Provide logical array-based access to the data for this
          data variable.  This is the const operator version.

   \tparam E A complex index type.

   This version of the operator is provided to support use with
   \e flecsi mesh entity types \ref mesh_entity_base_u.
   */
  template<typename E>
  T & operator()(E * e) {
    return this->operator()(e->id());
  } // operator ()

  handle_t handle;
};

template<typename T,
  size_t EXCLUSIVE_PERMISSIONS,
  size_t SHARED_PERMISSIONS,
  size_t GHOST_PERMISSIONS>
using dense_accessor_u = accessor_u<data::dense,
  T,
  EXCLUSIVE_PERMISSIONS,
  SHARED_PERMISSIONS,
  GHOST_PERMISSIONS>;

template<typename T,
  size_t EXCLUSIVE_PERMISSIONS,
  size_t SHARED_PERMISSIONS,
  size_t GHOST_PERMISSIONS>
using dense_accessor = dense_accessor_u<T,
  EXCLUSIVE_PERMISSIONS,
  SHARED_PERMISSIONS,
  GHOST_PERMISSIONS>;

} // namespace flecsi
