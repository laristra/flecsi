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
#include "flecsi/runtime/context.hh"
#include <flecsi/data/common/data_reference.hh>
#include <flecsi/data/common/storage_label.hh>
#include <flecsi/topology/internal/global.hh>
#include <flecsi/topology/internal/index.hh>
#include <flecsi/topology/structured_mesh/interface.hh>
#include <flecsi/topology/unstructured_mesh/interface.hh>
#endif

namespace flecsi {
namespace data {

template<storage_label_t STORAGE_CLASS, typename TOPOLOGY_TYPE>
struct storage_class {};

/*----------------------------------------------------------------------------*
  Global Topology.
 *----------------------------------------------------------------------------*/

namespace global_topo {

/*!
  Forward accessor type for bind friend.
 */

template<typename DATA_TYPE, size_t PRIVILEGES>
struct accessor;

/*!
  Friend function to bind mapped data into the accessor. This lets init_views
  to set the private data of the accessor.
 */

template<typename DATA_TYPE, size_t PRIVILEGES>
void
bind(accessor<DATA_TYPE, PRIVILEGES> & a, DATA_TYPE * data) {
  a.data_ = data;
} // bind

/*!
  Global data accessor.
 */

template<typename DATA_TYPE, size_t PRIVILEGES>
struct accessor : field_reference<DATA_TYPE> {

  friend void bind<DATA_TYPE, PRIVILEGES>(accessor & a, DATA_TYPE * data);

  using Base = field_reference<DATA_TYPE>;
  accessor(Base const & ref) : Base(ref) {}

  operator DATA_TYPE &() {
    return *data_;
  } // value

  operator DATA_TYPE const &() const {
    return *data_;
  } // value

  DATA_TYPE * data() {
    return data_;
  } // data

  accessor & operator=(const DATA_TYPE & value) {
    *data_ = value;
    return *this;
  } // operator=

private:
  DATA_TYPE * data_;

}; // struct accessor

} // namespace global_topo

template<>
struct storage_class<storage_label_t::dense, topology::global_topology_t> {

  template<typename DATA_TYPE, size_t PRIVILEGES>
  using accessor = global_topo::accessor<DATA_TYPE, PRIVILEGES>;

}; // struct storage_class

/*----------------------------------------------------------------------------*
  Index Topology.
 *----------------------------------------------------------------------------*/

namespace index_topo {

/*!
  Forward accessor type for bind friend.
 */

template<typename DATA_TYPE, size_t PRIVILEGES>
struct accessor;

/*!
  Friend function to bind mapped data into the accessor. This lets init_views
  to set the private data of the accessor.
 */

template<typename DATA_TYPE, size_t PRIVILEGES>
void
bind(accessor<DATA_TYPE, PRIVILEGES> & a, DATA_TYPE * data) {
  a.data_ = data;
} // bind

template<typename DATA_TYPE, size_t PRIVILEGES>
struct accessor : field_reference<DATA_TYPE> {

  using Base = field_reference<DATA_TYPE>;
  accessor(Base const & ref) : Base(ref) {}

  operator DATA_TYPE &() {
    return *data_;
  } // value

  operator DATA_TYPE const &() const {
    return *data_;
  } // value

  DATA_TYPE * data() {
    return data_;
  } // data

  accessor & operator=(const DATA_TYPE & value) {
    *data_ = value;
    return *this;
  } // operator=

private:
  friend void bind<DATA_TYPE, PRIVILEGES>(accessor & a, DATA_TYPE * data);

  DATA_TYPE * data_;

}; // struct accessor

} // namespace index_topo

template<>
struct storage_class<storage_label_t::dense, topology::index_topology_t> {

  template<typename DATA_TYPE, size_t PRIVILEGES>
  using accessor = index_topo::accessor<DATA_TYPE, PRIVILEGES>;

}; // struct storage_class

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
struct dense_accessor;

/*!
  Friend function to bind mapped data into the accessor. This lets init_views
  to set the private data of the accessor.
 */

template<typename DATA_TYPE, size_t PRIVILEGES>
void
bind(dense_accessor<DATA_TYPE, PRIVILEGES> & a, size_t size, DATA_TYPE * data) {
  a.size_ = size;
  a.data_ = data;
} // bind

template<typename DATA_TYPE, size_t PRIVILEGES>
struct dense_accessor : public field_reference_t {

  dense_accessor(field_reference_t const & ref) : field_reference_t(ref) {}

  /*!
    Provide logical array-based access to the data referenced by this
    accessor.

    @param index The index of the logical array to access.
   */

  DATA_TYPE & operator()(size_t index) const {
    flog_assert(index < size_, "index out of range");
    return data_[index];
  } // operator()

  DATA_TYPE * data() const {
    return data_;
  } // data

private:
  friend void bind<DATA_TYPE, PRIVILEGES>(dense_accessor & a,
    size_t size,
    DATA_TYPE * data);

  size_t size_;
  DATA_TYPE * data_;

}; // struct dense_accessor

} // namespace unstructured_mesh_topo

template<typename POLICY_TYPE>
struct storage_class<storage_label_t::dense,
  topology::unstructured_mesh_topology<POLICY_TYPE>> {}; // struct storage_class

#if 0
namespace unstructured_mesh_topo {

struct dense_handle_t {

  dense_handle_t() {}
  ~dense_handle_t() {}

}; // struct dense_handle_t

template<typename DATA_TYPE, size_t PRIVILEGES>
struct dense_accessor : public data_reference_t {

  dense_accessor(const dense_handle_t & handle) : handle_(handle) {}

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

}; // struct dense_accessor

struct sparse_handle_t {

  sparse_handle_t() {}
  ~sparse_handle_t() {}

}; // struct sparse_handle_t

template<typename DATA_TYPE, size_t PRIVILEGES>
struct sparse_accessor : public data_reference_t {

  sparse_accessor(const sparse_handle_t & handle) : handle_(handle) {}

private:
  sparse_handle_t handle_;

}; // struct sparse_accessor

template<typename DATA_TYPE, size_t PRIVILEGES>
struct sparse_mutator : public data_reference_t {

  sparse_mutator(const sparse_handle_t & handle) : handle_(handle) {}

private:
  sparse_handle_t handle_;

}; // struct sparse_mutator

} // namespace unstructured_mesh_topo
#endif

#if 0
template<typename POLICY_TYPE>
struct storage_class<dense, flecsi::topology::mesh_topology<POLICY_TYPE>> {

  template<typename DATA_TYPE, size_t PRIVILEGES>
  using handle_t =
    unstructured_mesh_topo::dense_handle<DATA_TYPE, PRIVILEGES>

  template<typename DATA_TYPE,
    size_t NAMESPACE,
    size_t NAME,
    size_t VERSION>
  static unstructured_mesh_topo::handle_t<DATA_TYPE, 0> get_reference(
    const client_handle<client_t, 0> & client_handle) {
  } // get_reference

}; // struct storage_class
#endif

} // namespace data
} // namespace flecsi
