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
#include <flecsi/topology/unstructured_mesh/interface.hh>
#endif

namespace flecsi {
namespace data {

template<storage_label_t STORAGE_CLASS, typename TOPOLOGY_TYPE>
struct storage_class_u {};

/*----------------------------------------------------------------------------*
  Global Topology.
 *----------------------------------------------------------------------------*/

namespace global_topo {

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
struct accessor_u : field_reference<DATA_TYPE> {

  friend void bind<DATA_TYPE, PRIVILEGES>(accessor_u & a, DATA_TYPE * data);

  using Base = field_reference<DATA_TYPE>;
  accessor_u(Base const & ref) : Base(ref) {}

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

} // namespace global_topo

template<>
struct storage_class_u<storage_label_t::dense, topology::global_topology_t> {

  template<typename DATA_TYPE, size_t PRIVILEGES>
  using accessor = global_topo::accessor_u<DATA_TYPE, PRIVILEGES>;

}; // struct storage_class_u

/*----------------------------------------------------------------------------*
  Index Topology.
 *----------------------------------------------------------------------------*/

namespace index_topo {

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
struct accessor_u : field_reference<DATA_TYPE> {

  using Base = field_reference<DATA_TYPE>;
  accessor_u(Base const & ref) : Base(ref) {}

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

} // namespace index_topo

template<>
struct storage_class_u<storage_label_t::dense, topology::index_topology_t> {

  template<typename DATA_TYPE, size_t PRIVILEGES>
  using accessor = index_topo::accessor_u<DATA_TYPE, PRIVILEGES>;

}; // struct storage_class_u

/*----------------------------------------------------------------------------*
  NTree Topology.
 *----------------------------------------------------------------------------*/

namespace ntree_topo {} // namespace ntree_topo

/*----------------------------------------------------------------------------*
  Set Topology.
 *----------------------------------------------------------------------------*/

namespace set_topo {} // namespace set_topo

/*----------------------------------------------------------------------------*
  Structured Mesh Topology.
 *----------------------------------------------------------------------------*/

namespace structured_mesh_topo {} // namespace structured_mesh_topo

/*----------------------------------------------------------------------------*
  Unstructured Mesh Topology.
 *----------------------------------------------------------------------------*/

namespace unstructured_mesh_topo {

/*!
  Forward dense accessor type for bind friend.
 */

template<typename DATA_TYPE, size_t PRIVILEGES>
struct dense_accessor_u;

/*!
  Friend function to bind mapped data into the accessor. This lets init_views
  to set the private data of the accessor.
 */

#if 0
template<typename DATA_TYPE, size_t PRIVILEGES>
void
bind(dense_accessor_u<DATA_TYPE, PRIVILEGES> & a, DATA_TYPE * data) {
  a.data_ = data;
} // bind
#endif

template<typename DATA_TYPE, size_t PRIVILEGES>
struct dense_accessor_u : public field_reference_t {

  dense_accessor_u(field_reference_t const & ref) : field_reference_t(ref) {}

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
  // friend void bind<DATA_TYPE, PRIVILEGES>(dense_accessor_u & a, DATA_TYPE *
  // data);

}; // struct dense_accessor_u

} // namespace unstructured_mesh_topo

template<typename POLICY_TYPE>
struct storage_class_u<storage_label_t::dense,
  topology::unstructured_mesh_topology_u<POLICY_TYPE>> {
}; // struct storage_class_u

#if 0
namespace unstructured_mesh_topo {

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

} // namespace unstructured_mesh_topo
#endif

#if 0
template<typename POLICY_TYPE>
struct storage_class_u<dense, flecsi::topology::mesh_topology_u<POLICY_TYPE>> {

  template<typename DATA_TYPE, size_t PRIVILEGES>
  using handle_t =
    unstructured_mesh_topo::dense_handle_u<DATA_TYPE, PRIVILEGES>

  template<typename DATA_TYPE,
    size_t NAMESPACE,
    size_t NAME,
    size_t VERSION>
  static unstructured_mesh_topo::handle_t<DATA_TYPE, 0> get_reference(
    const client_handle_u<client_t, 0> & client_handle) {
  } // get_reference

}; // struct storage_class_u
#endif

} // namespace data
} // namespace flecsi
