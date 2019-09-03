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
#include "data_reference.hh"
#include "flecsi/runtime/context.hh"
#include <flecsi/topology/internal/global.hh>
#include <flecsi/topology/internal/index.hh>
#include <flecsi/topology/structured_mesh/interface.hh>
#include <flecsi/topology/unstructured_mesh/interface.hh>
#endif

namespace flecsi {
namespace data {

/*!
  The storage_label_t type enumerates the available FleCSI storage classes.
  A FleCSI storage class provides a specific interface for different
  logical data layouts, e.g., dense vs. sparse. The actual data layout
  is implementation-dependent.
 */

enum storage_label_t : size_t {
  dense,
  sparse,
  ragged,
  array,
  subspace
}; // enum storage_label_t

/// A data accessor.
/// \tparam STORAGE_CLASS data layout
/// \tparam TOPOLOGY_TYPE core topology type
/// \tparam T data type
/// \tparam Priv access privileges
template<storage_label_t STORAGE_CLASS,
  typename TOPOLOGY_TYPE,
  typename T,
  std::size_t Priv>
struct accessor;

/*----------------------------------------------------------------------------*
  Global Topology.
 *----------------------------------------------------------------------------*/

template<typename DATA_TYPE, size_t PRIVILEGES>
struct accessor<storage_label_t::dense,
  topology::global_topology_t,
  DATA_TYPE,
  PRIVILEGES> : field_reference<DATA_TYPE> {

  friend void bind(accessor & a, DATA_TYPE * data) {
    a.data_ = data;
  }

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

/*----------------------------------------------------------------------------*
  Index Topology.
 *----------------------------------------------------------------------------*/

template<typename DATA_TYPE, size_t PRIVILEGES>
struct accessor<storage_label_t::dense,
  topology::index_topology_t,
  DATA_TYPE,
  PRIVILEGES> : field_reference<DATA_TYPE> {

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
  friend void bind(accessor & a, DATA_TYPE * data) {
    a.data_ = data;
  }

  DATA_TYPE * data_;

}; // struct accessor

/*----------------------------------------------------------------------------*
  NTree Topology.
 *----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*
  Set Topology.
 *----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*
  Structured Mesh Topology.
 *----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*
  Unstructured Mesh Topology.
 *----------------------------------------------------------------------------*/

// TODO: Do we need POLICY_TYPE, or should we avoid redundant instantiation by
// keying on unstructured_mesh_topology_base_t (or some equivalent enum)?
template<typename POLICY_TYPE, typename DATA_TYPE, size_t PRIVILEGES>
struct accessor<storage_label_t::dense,
  topology::unstructured_mesh_topology<POLICY_TYPE>,
  DATA_TYPE,
  PRIVILEGES> : public field_reference<DATA_TYPE> {

  using Base = field_reference<DATA_TYPE>;
  accessor(Base const & ref) : Base(ref) {}

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
  friend void bind(accessor & a, size_t size, DATA_TYPE * data) {
    a.size_ = size;
    a.data_ = data;
  }

  size_t size_;
  DATA_TYPE * data_;

}; // struct accessor

} // namespace data
} // namespace flecsi
