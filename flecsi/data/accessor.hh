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
#endif

#include <flecsi/data/field.hh>
#include <flecsi/topology/structured/interface.hh>
#include <flecsi/topology/unstructured/interface.hh>

namespace flecsi {
namespace data {

/*----------------------------------------------------------------------------*
  Global Topology.
 *----------------------------------------------------------------------------*/

template<typename DATA_TYPE, size_t PRIVILEGES>
struct accessor<storage_label_t::dense, topology::global, DATA_TYPE, PRIVILEGES>
  : reference_base {

  friend void bind(accessor & a, DATA_TYPE * data) {
    a.data_ = data;
  }

  using value_type = DATA_TYPE;
  using topology_t = topology::global;

  accessor(field_reference<DATA_TYPE, topology_t> const & ref)
    : accessor(ref.fid()) {}
  explicit accessor(std::size_t f) : reference_base(f) {}

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
struct accessor<storage_label_t::dense, topology::index, DATA_TYPE, PRIVILEGES>
  : reference_base {
  using value_type = DATA_TYPE;
  using topology_t = topology::index;

  accessor(field_reference<DATA_TYPE, topology_t> const & ref)
    : accessor(ref.fid()) {}
  explicit accessor(std::size_t f) : reference_base(f) {}

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
// keying on unstructured_base (or some equivalent enum)?
template<typename POLICY_TYPE, typename DATA_TYPE, size_t PRIVILEGES>
struct accessor<storage_label_t::dense,
  topology::unstructured<POLICY_TYPE>,
  DATA_TYPE,
  PRIVILEGES> : reference_base {
  using value_type = DATA_TYPE;
  using topology_t = topology::unstructured<POLICY_TYPE>;

  accessor(field_reference<DATA_TYPE, topology_t> const & ref)
    : accessor(ref.fid()) {}
  explicit accessor(std::size_t f) : reference_base(f) {}

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

  // These must be initialized to copy into a user's accessor parameter.
  size_t size_ = 0;
  DATA_TYPE * data_ = nullptr;

}; // struct accessor

} // namespace data
} // namespace flecsi
