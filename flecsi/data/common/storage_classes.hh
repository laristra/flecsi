/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Triad National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

/*!
  @file

  This file contains implementations of field accessor types.
 */

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly!
#else
#include <flecsi/data/common/data_reference.hh>
#include <flecsi/data/common/storage_label.hh>
#include <flecsi/execution/context.hh>
#include <flecsi/topology/internal/global.hh>
#include <flecsi/topology/internal/index.hh>
#include <flecsi/topology/structured_mesh/interface.hh>
//#include <flecsi/topology/unstructured_mesh/interface.hh>
#endif

namespace flecsi {
namespace data {

/*----------------------------------------------------------------------------*
  Global Topology.
 *----------------------------------------------------------------------------*/

namespace global_topology {

/*!
  Forward accessor type for bind friend.
 */

template<typename DATA_TYPE, size_t PRIVILEGES>
struct accessor_u;

/*!
  Friend function to bind mapped data into the accessor. This lets init_views
  to set the private data of the accessor.
 */

template<typename DATA_TYPE, size_t PRIVILEGES>
void
bind(accessor_u<DATA_TYPE, PRIVILEGES> & a, DATA_TYPE * data) {
  a.data_ = data;
} // bind

/*!
  Global data accessor.
 */

template<typename DATA_TYPE, size_t PRIVILEGES>
struct accessor_u : public field_reference_t {

  friend void bind<DATA_TYPE, PRIVILEGES>(accessor_u & a, DATA_TYPE * data);

  accessor_u(field_reference_t const & ref) : field_reference_t(ref) {}

  operator DATA_TYPE &() {
    return *data_;
  } // value

  operator DATA_TYPE const &() const {
    return *data_;
  } // value

  DATA_TYPE * data() {
    return data_;
  } // data

  accessor_u & operator=(const DATA_TYPE & value) {
    *data_ = value;
    return *this;
  } // operator=

private:
  DATA_TYPE * data_;

}; // struct accessor_u

} // namespace global_topology

/*----------------------------------------------------------------------------*
  Index Topology.
 *----------------------------------------------------------------------------*/

namespace index_topology {

/*!
  Forward accessor type for bind friend.
 */

template<typename DATA_TYPE, size_t PRIVILEGES>
struct accessor_u;

/*!
  Friend function to bind mapped data into the accessor. This lets init_views
  to set the private data of the accessor.
 */

template<typename DATA_TYPE, size_t PRIVILEGES>
void
bind(accessor_u<DATA_TYPE, PRIVILEGES> & a, DATA_TYPE * data) {
  a.data_ = data;
} // bind

template<typename DATA_TYPE, size_t PRIVILEGES>
struct accessor_u : public field_reference_t {

  accessor_u(field_reference_t const & ref) : field_reference_t(ref) {}

  operator DATA_TYPE &() {
    return *data_;
  } // value

  operator DATA_TYPE const &() const {
    return *data_;
  } // value

  DATA_TYPE * data() {
    return data_;
  } // data

  accessor_u & operator=(const DATA_TYPE & value) {
    *data_ = value;
    return *this;
  } // operator=

private:
  friend void bind<DATA_TYPE, PRIVILEGES>(accessor_u & a, DATA_TYPE * data);

  DATA_TYPE * data_;

}; // struct accessor_u

} // namespace index_topology

/*----------------------------------------------------------------------------*
  NTree Topology.
 *----------------------------------------------------------------------------*/

namespace ntree_topology {} // namespace ntree_topology

/*----------------------------------------------------------------------------*
  Set Topology.
 *----------------------------------------------------------------------------*/

namespace set_topology {} // namespace set_topology

/*----------------------------------------------------------------------------*
  Structured Mesh Topology.
 *----------------------------------------------------------------------------*/

namespace structured_mesh_topology {} // namespace structured_mesh_topology

/*----------------------------------------------------------------------------*
  Unstructured Mesh Topology.
 *----------------------------------------------------------------------------*/

#if 0
namespace unstructured_mesh_topology {

struct dense_handle_t {

  dense_handle_t() {}
  ~dense_handle_t() {}

}; // struct dense_handle_t

template<typename DATA_TYPE, size_t PRIVILEGES>
struct dense_accessor_u : public data_reference_t {

  dense_accessor_u(const dense_handle_t & handle) : handle_(handle) {}

  /*!
    Provide logical array-based access to the data referenced by this
    accessor. \em Const version.

    @param index The index of the logical array to access.
   */

  const DATA_TYPE & operator()(size_t index) const {} // operator()

  /*!
    Provide logical array-based access to the data referenced by this
    accessor.

    @param index The index of the logical array to access.
   */

  DATA_TYPE & operator()(size_t index) {} // operator()

private:
  dense_handle_t handle_;

}; // struct dense_accessor_u

struct sparse_handle_t {

  sparse_handle_t() {}
  ~sparse_handle_t() {}

}; // struct sparse_handle_t

template<typename DATA_TYPE, size_t PRIVILEGES>
struct sparse_accessor_u : public data_reference_t {

  sparse_accessor_u(const sparse_handle_t & handle) : handle_(handle) {}

private:
  sparse_handle_t handle_;

}; // struct sparse_accessor_u

template<typename DATA_TYPE, size_t PRIVILEGES>
struct sparse_mutator_u : public data_reference_t {

  sparse_mutator_u(const sparse_handle_t & handle) : handle_(handle) {}

private:
  sparse_handle_t handle_;

}; // struct sparse_mutator_u

} // namespace unstructured_mesh_topology
#endif

#if 0
template<typename POLICY_TYPE>
struct storage_class_u<dense, flecsi::topology::mesh_topology_u<POLICY_TYPE>> {

  template<typename DATA_TYPE, size_t PRIVILEGES>
  using handle_t =
    unstructured_mesh_topology::dense_handle_u<DATA_TYPE, PRIVILEGES>

  template<typename DATA_TYPE,
    size_t NAMESPACE,
    size_t NAME,
    size_t VERSION>
  static unstructured_mesh_topology::handle_t<DATA_TYPE, 0> get_reference(
    const client_handle_u<client_t, 0> & client_handle) {
  } // get_reference

}; // struct storage_class_u
#endif

} // namespace data
} // namespace flecsi
