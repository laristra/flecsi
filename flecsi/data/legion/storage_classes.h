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

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly!
#else
  #include <flecsi/topology/internal.h>
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

template<typename DATA_TYPE, size_t PRIVLEGES>
struct handle_u {

  handle_u() {}
  ~handle_u() {}

}; // struct handle_u

template<typename DATA_TYPE, size_t PRIVLEGES>
struct accessor_u {

  using handle_t = handle_u<DATA_TYPE, PRIVLEGES>;

  accessor_u(const handle_u<DATA_TYPE, 0> & handle)
    : handle_(reinterpret_cast<const handle_t &>(handle)) {}

  /*!
    Provide logical array-based access to the data referenced by this
    accessor. \em Const version.

    @param index The index of the logical array to access.
   */

  const DATA_TYPE & operator()(size_t index) const {
  } // operator()

  /*!
    Provide logical array-based access to the data referenced by this
    accessor.

    @param index The index of the logical array to access.
   */

  DATA_TYPE & operator()(size_t index) {
  } // operator()

private:

  handle_t handle_;

}; // struct accessor_u

} // namespace global_topology

template<>
struct storage_class_u<global, flecsi::topology::global_topology_t> {

  using client_t = flecsi::topology::global_topology_t;
  using client_handle_t = client_handle_u<client_t, 0>;
  template<typename DATA_TYPE, size_t PRIVLEGES>
  using handle_t = global_topology::handle_u<DATA_TYPE, PRIVLEGES>;

  template<typename DATA_TYPE, size_t NAMESPACE, size_t NAME, size_t VERSION>
  static handle_t<DATA_TYPE, 0> get_handle(const client_handle_t & client_handle) {
    handle_t<DATA_TYPE, 0> h;
    return h;
  } // get_handle

}; // struct storage_class_u

/*----------------------------------------------------------------------------*
  Color Topology.
 *----------------------------------------------------------------------------*/

namespace color_topology {

template<typename TYPE, size_t PRIVLEGES>
struct handle_u {

  handle_u() {}
  ~handle_u() {}

}; // struct handle_u

template<typename DATA_TYPE, size_t PRIVLEGES>
struct accessor_u {

  using handle_t = handle_u<DATA_TYPE, PRIVLEGES>;

  accessor_u(const handle_u<DATA_TYPE, 0> & handle)
    : handle_(reinterpret_cast<const handle_t &>(handle)) {}

  /*!
    Provide logical array-based access to the data referenced by this
    accessor. \em Const version.

    @param index The index of the logical array to access.
   */

  const DATA_TYPE & operator()(size_t index) const {
  } // operator()

  /*!
    Provide logical array-based access to the data referenced by this
    accessor.

    @param index The index of the logical array to access.
   */

  DATA_TYPE & operator()(size_t index) {
  } // operator()

private:

  handle_t handle_;

}; // struct accessor_u

} // namespace color_topology

template<>
struct storage_class_u<color, flecsi::topology::color_topology_t> {

  using client_t = flecsi::topology::global_topology_t;
  using client_handle_t = client_handle_u<client_t, 0>;
  template<typename DATA_TYPE, size_t PRIVLEGES>
  using handle_t = color_topology::handle_u<DATA_TYPE, PRIVLEGES>;

  template<typename DATA_TYPE, size_t NAMESPACE, size_t NAME, size_t VERSION>
  static handle_t<DATA_TYPE, 0> get_handle(const client_handle_t & client_handle) {
    handle_t<DATA_TYPE, 0> h;
    return h;
  } // get_handle

}; // struct storage_class_u

/*----------------------------------------------------------------------------*
  Unstructured Mesh Topology.
 *----------------------------------------------------------------------------*/

namespace unstructured_mesh_topology {

template<typename TYPE, size_t PRIVLEGES>
struct dense_handle_u {

  dense_handle_u() {}
  ~dense_handle_u() {}

}; // struct dense_handle_unstructured_mesh_u

template<typename DATA_TYPE, size_t PRIVLEGES>
struct dense_accessor_u {

  using handle_t = dense_handle_u<DATA_TYPE, PRIVLEGES>;

  dense_accessor_u(const dense_handle_u<DATA_TYPE, 0> & handle)
    : handle_(reinterpret_cast<const handle_t &>(handle)) {}

  /*!
    Provide logical array-based access to the data referenced by this
    accessor. \em Const version.

    @param index The index of the logical array to access.
   */

  const DATA_TYPE & operator()(size_t index) const {
  } // operator()

  /*!
    Provide logical array-based access to the data referenced by this
    accessor.

    @param index The index of the logical array to access.
   */

  DATA_TYPE & operator()(size_t index) {
  } // operator()

private:

  handle_t handle_;

}; // struct dense_accessor_u

template<typename TYPE, size_t PRIVLEGES>
struct sparse_handle_u {

  sparse_handle_u() {}
  ~sparse_handle_u() {}

}; // struct sparse_handle_unstructured_mesh_u

template<typename DATA_TYPE, size_t PRIVLEGES>
struct sparse_accessor_u {

  using handle_t = sparse_handle_u<DATA_TYPE, PRIVLEGES>;

  sparse_accessor_u(const sparse_handle_u<DATA_TYPE, 0> & handle)
    : handle_(reinterpret_cast<const handle_t &>(handle)) {}

private:

  handle_t handle_;

}; // struct sparse_accessor_u

template<typename DATA_TYPE, size_t PRIVLEGES>
struct sparse_mutator_u {

  using handle_t = sparse_handle_u<DATA_TYPE, PRIVLEGES>;

  sparse_mutator_u(const sparse_handle_u<DATA_TYPE, 0> & handle)
    : handle_(reinterpret_cast<const handle_t &>(handle)) {}

private:

  handle_t handle_;

}; // struct sparse_mutator_u

} // namespace unstructured_mesh_topology

#if 0
template<typename POLICY_TYPE>
struct storage_class_u<dense, flecsi::topology::mesh_topology_u<POLICY_TYPE>> {

  using client_t = flecsi::topology::mesh_topology_u<POLICY_TYPE>;
  template<typename DATA_TYPE, size_t PRIVLEGES>
  using handle_t =
    unstructured_mesh_topology::dense_handle_u<DATA_TYPE, PRIVLEGES>

  template<typename DATA_TYPE,
    size_t NAMESPACE,
    size_t NAME,
    size_t VERSION>
  static unstructured_mesh_topology::handle_t<DATA_TYPE, 0> get_handle(
    const client_handle_u<client_t, 0> & client_handle) {
  } // get_handle

}; // struct storage_class_u
#endif

} // namespace legion
} // namespace data
} // namespace flecsi
