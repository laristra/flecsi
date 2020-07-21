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

#include "flecsi/data/reference.hh"
#include "flecsi/exec/launch.hh"
#include "flecsi/util/array_ref.hh"
#include <flecsi/data/field.hh>

namespace flecsi {
namespace data {

template<typename DATA_TYPE, size_t PRIVILEGES>
struct accessor<singular, DATA_TYPE, PRIVILEGES> {
  using value_type = DATA_TYPE;
  using base_type = accessor<dense, DATA_TYPE, PRIVILEGES>;
  using element_type = typename base_type::element_type;

  accessor(const base_type & b) : base(b) {}

  element_type & get() const {
    return base(0);
  } // data
  operator element_type &() const {
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
  using element_type = std::
    conditional_t<privilege_write(PRIVILEGES), value_type, const value_type>;

  explicit accessor(std::size_t f) : reference_base(f) {}

  /*!
    Provide logical array-based access to the data referenced by this
    accessor.

    @param index The index of the logical array to access.
   */

  element_type & operator()(size_t index) const {
    flog_assert(index < s.size(), "index out of range");
    return s[index];
  } // operator()

  auto span() const {
    return s;
  }

private:
  friend void bind(accessor & a, util::span<element_type> s) {
    a.s = s;
  }

  util::span<element_type> s;
}; // struct accessor

} // namespace data

template<class T, std::size_t S>
struct exec::detail::task_param<data::accessor<data::dense, T, S>> {
  template<class Topo, typename Topo::index_space Space>
  static auto replace(
    const data::field_reference<T, data::dense, Topo, Space> & r) {
    return data::accessor<data::dense, T, S>(r.fid());
  }
};
template<class T, std::size_t S>
struct exec::detail::task_param<data::accessor<data::singular, T, S>> {
  using type = data::accessor<data::singular, T, S>;
  template<class Topo, typename Topo::index_space Space>
  static type replace(
    const data::field_reference<T, data::singular, Topo, Space> & r) {
    return exec::replace_argument<typename type::base_type>(
      r.template cast<data::dense>());
  }
};

} // namespace flecsi
