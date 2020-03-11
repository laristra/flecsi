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
#include"flecsi/data/reference.hh"

namespace flecsi {
namespace data {

template<typename DATA_TYPE, size_t PRIVILEGES>
struct accessor<singular, DATA_TYPE, PRIVILEGES> {
  using value_type = DATA_TYPE;
  using base_type = accessor<dense, DATA_TYPE, PRIVILEGES>;

  // This constructor is needed to avoid two user-defined conversions.
  template<class Topo>
  accessor(field_reference<DATA_TYPE, Topo> const & ref) : base(ref) {}
  accessor(const base_type & b) : base(b) {}

  DATA_TYPE & get() const {
    return base(0);
  } // data
  operator DATA_TYPE &() const {
    return get();
  } // value

  const accessor & operator=(const DATA_TYPE & value) const {
    return const_cast<accessor &>(*this) = value;
  } // operator=
  accessor & operator=(const DATA_TYPE & value) {
    get() = value;
    return *this;
  } // operator=

  base_type & get_base() {
    return base;
  }
  const base_type & get_base() const {
    return base;
  }
  friend base_type * get_null_base(accessor *) { // for task_prologue_t
    return nullptr;
  }

private:
  base_type base;
}; // struct accessor

template<typename DATA_TYPE, size_t PRIVILEGES>
struct accessor<dense, DATA_TYPE, PRIVILEGES> : reference_base {
  using value_type = DATA_TYPE;

  template<class Topo>
  accessor(field_reference<DATA_TYPE, Topo> const & ref)
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
