/*~--------------------------------------------------------------------------~*
 *  @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
 * /@@/////  /@@          @@////@@ @@////// /@@
 * /@@       /@@  @@@@@  @@    // /@@       /@@
 * /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
 * /@@////   /@@/@@@@@@@/@@       ////////@@/@@
 * /@@       /@@/@@//// //@@    @@       /@@/@@
 * /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
 * //       ///  //////   //////  ////////  //
 *
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~--------------------------------------------------------------------------~*/

#pragma once

//----------------------------------------------------------------------------//
// POLICY_NAMESPACE must be defined before including storage_type.h!!!
// Using this approach allows us to have only one storage_type_t
// definition that can be used by all data policies -> code reuse...
#define POLICY_NAMESPACE hpx
#include <flecsi/data/storage_class.h>
#undef POLICY_NAMESPACE
//----------------------------------------------------------------------------//

#include <algorithm>
#include <memory>

#include <flecsi/data/common/data_types.h>
#include <flecsi/data/common/privilege.h>
#include <flecsi/data/data_client.h>
#include <flecsi/data/dense_data_handle.h>
#include <flecsi/execution/context.h>
#include <flecsi/utils/index_space.h>

///
/// \file
/// \date Initial file creation: Apr 7, 2016
///

#define np(X)                                                                  \
  std::cout << __FILE__ << ":" << __LINE__ << ": " << __PRETTY_FUNCTION__      \
            << ": " << #X << " = " << (X) << std::endl

namespace flecsi {
namespace data {
namespace hpx {

//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=//
// Helper type definitions.
//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=//

//----------------------------------------------------------------------------//
// Dense handle.
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
// Dense accessor.
//----------------------------------------------------------------------------//

///
/// \brief dense_accessor_t provides logically array-based access to data
///        variables that have been registered in the data model.
///
/// \tparam T The type of the data variable. If this type is not
///           consistent with the type used to register the data, bad things
///           can happen. However, it can be useful to reinterpret the type,
///           e.g., when writing raw bytes. This class is part of the
///           low-level \e flecsi interface, so it is assumed that you
///           know what you are doing...
/// \tparam MD The meta data type.
///
template<typename T, size_t EP, size_t SP, size_t GP>
struct dense_handle_t : public dense_data_handle_u<T, EP, SP, GP> {
  //--------------------------------------------------------------------------//
  // Type definitions.
  //--------------------------------------------------------------------------//

  using base = dense_data_handle_u<T, EP, SP, GP>;

  //--------------------------------------------------------------------------//
  // Constructors.
  //--------------------------------------------------------------------------//

  // FIXME: calling to base class constructor?
  ///
  /// Default constructor.
  ///
  dense_handle_t() {}

  template<typename, size_t, size_t, size_t>
  friend struct dense_handle_t;
};

#if 0
  ///
  /// Copy constructor.
  ///
  dense_handle_t(const dense_handle_t & a) : label_(a.label_) {}

  template<size_t EP2, size_t SP2, size_t GP2>
  dense_handle_t(const dense_handle_t<T, EP2, SP2, GP2> & h)
      : label_(h.label_) {}

  //--------------------------------------------------------------------------//
  // Member data interface.
  //--------------------------------------------------------------------------//

  ///
  /// \brief Return a std::string containing the label of the data variable
  ///       reference by this accessor.
  ///
  const std::string & label() const {
    return label_;
  } // label

  ///
  /// \brief Return the index space size of the data variable
  ///        referenced by this accessor.
  ///
  size_t size() const {
    return base::primary_size;
  } // size

  ///
  // \brief Return the index space size of the data variable
  //        referenced by this handle.
  ///
  size_t exclusive_size() const {
    return base::exclusive_size;
  } // size

  ///
  // \brief Return the index space size of the data variable
  //        referenced by this handle.
  ///
  size_t shared_size() const {
    return base::shared_size;
  } // size

  //--------------------------------------------------------------------------//
  // Operators.
  //--------------------------------------------------------------------------//

  ///
  /// \brief Provide logical array-based access to the data for this
  ///        data variable.  This is the const operator version.
  ///
  /// \tparam E A complex index type.
  ///
  /// This version of the operator is provided to support use with
  /// \e flecsi mesh entity types \ref mesh_entity_base_t.
  ///
  template<typename E>
  const T & operator[](E * e) const {
    return this->operator[](e->template id<0>());
  } // operator []

  ///
  /// \brief Provide logical array-based access to the data for this
  ///        data variable.  This is the const operator version.
  ///
  /// \tparam E A complex index type.
  ///
  /// This version of the operator is provided to support use with
  /// \e flecsi mesh entity types \ref mesh_entity_base_t.
  ///
  template<typename E>
  T & operator[](E * e) {
    return this->operator[](e->template id<0>());
  } // operator []

  ///
  /// \brief Provide logical array-based access to the data for this
  ///        data variable.  This is the const operator version.
  ///
  /// \tparam E A complex index type.
  ///
  /// This version of the operator is provided to support use with
  /// \e flecsi mesh entity types \ref mesh_entity_base_t.
  ///
  template<typename E>
  const T & operator()(E * e) const {
    return this->operator[](e->template id<0>());
  } // operator []

  ///
  /// \brief Provide logical array-based access to the data for this
  ///        data variable.  This is the const operator version.
  ///
  /// \tparam E A complex index type.
  ///
  /// This version of the operator is provided to support use with
  /// \e flecsi mesh entity types \ref mesh_entity_base_t.
  ///
  template<typename E>
  T & operator()(E * e) {
    return this->operator[](e->template id<0>());
  } // operator []

  ///
  // \brief Provide logical array-based access to the data for this
  //        data variable.  This is the const operator version.
  //
  // \param index The index of the data variable to return.
  ///
  const T & operator[](size_t index) const {
    assert(index < base::primary_size && "index out of range");
    return base::primary_data[index];
  } // operator []

  ///
  // \brief Provide logical array-based access to the data for this
  //        data variable.  This is the const operator version.
  //
  // \param index The index of the data variable to return.
  ///
  T & operator[](size_t index) {
    assert(index < base::primary_size && "index out of range");
    return base::primary_data[index];
  } // operator []

  ///
  // \brief Provide logical array-based access to the data for this
  //        data variable.  This is the const operator version.
  //
  // \param index The index of the data variable to return.
  ///
  const T & exclusive(size_t index) const {
    assert(index < base::exclusive_size && "index out of range");
    return base::exclusive_data[index];
  } // operator []

  ///
  // \brief Provide logical array-based access to the data for this
  //        data variable.  This is the const operator version.
  //
  // \param index The index of the data variable to return.
  ///
  T & exclusive(size_t index) {
    assert(index < base::exclusive_size && "index out of range");
    return base::exclusive_data[index];
  } // operator []

  ///
  // \brief Provide logical array-based access to the data for this
  //        data variable.  This is the const operator version.
  //
  // \param index The index of the data variable to return.
  ///
  const T & shared(size_t index) const {
    assert(index < base::shared_size && "index out of range");
    return base::shared_data[index];
  } // operator []

  ///
  // \brief Provide logical array-based access to the data for this
  //        data variable.  This is the const operator version.
  //
  // \param index The index of the data variable to return.
  ///
  T & shared(size_t index) {
    assert(index < base::shared_size && "index out of range");
    return base::shared_data[index];
  } // operator []

  ///
  // \brief Provide logical array-based access to the data for this
  //        data variable.  This is the const operator version.
  //
  // \param index The index of the data variable to return.
  ///
  const T & ghost(size_t index) const {
    assert(index < base::ghost_size && "index out of range");
    return base::ghost_data[index];
  } // operator []

  ///
  // \brief Provide logical array-based access to the data for this
  //        data variable.  This is the const operator version.
  //
  // \param index The index of the data variable to return.
  ///
  T & ghost(size_t index) {
    assert(index < base::ghost_size && "index out of range");
    return base::ghost_data[index];
  } // operator []

  //  ///
  //  // \brief Provide logical array-based access to the data for this
  //  //        data variable.  This is the const operator version.
  //  //
  //  // \tparam E A complex index type.
  //  //
  //  // This version of the operator is provided to support use with
  //  // \e flecsi mesh entity types \ref mesh_entity_base_t.
  //  ///
  //  template<typename E>
  //  const T &
  //  operator () (
  //    E * e
  //  ) const
  //  {
  //    return this->operator()(e->template id<0>());
  //  } // operator ()
  //
  //  ///
  //  // \brief Provide logical array-based access to the data for this
  //  //        data variable.  This is the const operator version.
  //  //
  //  // \tparam E A complex index type.
  //  //
  //  // This version of the operator is provided to support use with
  //  // \e flecsi mesh entity types \ref mesh_entity_base_t.
  //  ///
  //  template<typename E>
  //  T &
  //  operator () (
  //    E * e
  //  )
  //  {
  //    return this->operator()(e->template id<0>());
  //  } // operator ()

  ///
  // \brief Provide logical array-based access to the data for this
  //        data variable.  This is the const operator version.
  //
  // \param index The index of the data variable to return.
  ///
  const T & operator()(size_t index) const {
    assert(index < base::primary_size && "index out of range");
    return base::primary_data[index];
  } // operator ()

  ///
  // \brief Provide logical array-based access to the data for this
  //        data variable.  This is the const operator version.
  //
  // \param index The index of the data variable to return.
  ///
  T & operator()(size_t index) {
    assert(index < base::primary_size && "index out of range");
    return base::primary_data[index];
  } // operator ()

  ///
  /// \brief Test to see if this accessor is empty
  ///
  /// \return true if registered.
  ///
  operator bool() const {
    return base::primary_data != nullptr;
  } // operator bool

  template<typename, size_t, size_t, size_t>
  friend struct dense_handle_t;

private:
  std::string label_ = "";
}; // struct dense_handle_t
#endif
//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=//
// Main type definition.
//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=//

//----------------------------------------------------------------------------//
// Dense storage type.
//----------------------------------------------------------------------------//

///
/// FIXME: Dense storage type.
///
template<>
struct storage_class_u<dense> {
  //--------------------------------------------------------------------------//
  // Type definitions.
  //--------------------------------------------------------------------------//

  template<typename T, size_t EP, size_t SP, size_t GP>
  using handle_t = dense_handle_t<T, EP, SP, GP>;

  template<typename DATA_CLIENT_TYPE,
      typename DATA_TYPE,
      size_t NAMESPACE,
      size_t NAME,
    size_t VERSION>
  static handle_t<DATA_TYPE, 0, 0, 0> get_handle(
    const data_client_t & data_client) {
    handle_t<DATA_TYPE, 0, 0, 0> h;
    // FIXME add logic here
    return h;
  }

}; // struct storage_class_u

} // namespace hpx
} // namespace data
} // namespace flecsi
