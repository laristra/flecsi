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

/*! @file */

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly!
#else
#include <flecsi/data/common/data_reference.h>
#include <flecsi/data/legion/topologies.h>
#include <flecsi/topology/internal/color.h>
#include <flecsi/topology/internal/global.h>
#endif

#define POLICY_NAMESPACE legion
#include <flecsi/data/common/storage_class.h>
#undef POLICY_NAMESPACE

namespace flecsi {
namespace data {
namespace legion {

/*----------------------------------------------------------------------------*
  Global Topology.
 *----------------------------------------------------------------------------*/

namespace global_topology {

template<typename DATA_TYPE, size_t PRIVILEGES>
struct accessor_u : public field_reference_t {

  accessor_u(field_reference_t const & ref)
    : field_reference_t(ref) {}

  /*!
    Return a raw reference to the data of this accessor.
   */

  DATA_TYPE * data() {
    return data_;
  } // data

  /*!
    Assignment operator.

    @param value The value to assign to this accessor.
   */

  accessor_u & operator=(const DATA_TYPE & value) {
    *data_ = value;
    return *this;
  } // operator=

private:
  DATA_TYPE * data_;

}; // struct accessor_u

} // namespace global_topology

template<>
struct storage_class_u<global, flecsi::topology::global_topology_t> {

  using global_topology_t = flecsi::topology::global_topology_t;
  using topology_reference_t = topology_reference_u<global_topology_t>;

  template<typename DATA_TYPE, size_t NAMESPACE, size_t NAME, size_t VERSION>
  static field_reference_t get_reference(topology_reference_t const & topology) {
    field_id_t fid;
    size_t identifier;
    field_reference_t ref(fid, identifier, topology.identifier());
    return ref;
  } // get_reference

}; // struct storage_class_u

/*----------------------------------------------------------------------------*
  Color Topology.
 *----------------------------------------------------------------------------*/

namespace color_topology {

template<typename DATA_TYPE, size_t PRIVILEGES>
struct accessor_u : public field_reference_t {

  accessor_u(field_reference_t const & ref)
    : field_reference_t(ref) {}

  /*!
    Return a raw reference to the data of this accessor.
   */

  DATA_TYPE * data() {
    return data_;
  } // data

  /*!
    Assignment operator.

    @param value The value to assign to this accessor.
   */

  accessor_u & operator=(const DATA_TYPE & value) {
    *data_ = value;
    return *this;
  } // operator=

private:
  DATA_TYPE * data_;

}; // struct accessor_u

} // namespace color_topology

template<>
struct storage_class_u<color, flecsi::topology::color_topology_t> {

  using color_topology_t = flecsi::topology::color_topology_t;
  using topology_reference_t = topology_reference_u<color_topology_t>;

  template<typename DATA_TYPE, size_t NAMESPACE, size_t NAME, size_t VERSION>
  static field_reference_t get_reference(topology_reference_t const & topology) {
    field_id_t fid;
    size_t identifier;
    field_reference_t ref(fid, identifier, topology.identifier());
    return ref;
  } // get_reference

}; // struct storage_class_u

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

} // namespace legion
} // namespace data
} // namespace flecsi
