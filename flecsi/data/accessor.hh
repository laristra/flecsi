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
struct accessor<raw, DATA_TYPE, PRIVILEGES> : reference_base {
  using value_type = DATA_TYPE;
  using element_type = std::
    conditional_t<privilege_write(PRIVILEGES), value_type, const value_type>;

  explicit accessor(std::size_t f) : reference_base(f) {}

  auto span() const {
    return s;
  }

  void bind(util::span<element_type> x) { // for bind_accessors
    s = x;
  }

private:
  util::span<element_type> s;
}; // struct accessor

template<class T, std::size_t P>
struct accessor<dense, T, P> : accessor<raw, T, P> {
  using base_type = accessor<raw, T, P>;
  using base_type::base_type;

  accessor(const base_type & b) : base_type(b) {}

  /*!
    Provide logical array-based access to the data referenced by this
    accessor.

    @param index The index of the logical array to access.
   */
  typename accessor::element_type & operator()(std::size_t index) const {
    const auto s = this->span();
    flog_assert(index < s.size(), "index out of range");
    return s[index];
  } // operator()

  base_type & get_base() {
    return *this;
  }
  const base_type & get_base() const {
    return *this;
  }
  friend base_type * get_null_base(accessor *) { // for task_prologue_t
    return nullptr;
  }
};

template<class T, std::size_t P, std::size_t OP = P>
struct ragged_accessor
  : accessor<raw, T, P>,
    util::with_index_iterator<const ragged_accessor<T, P, OP>> {
  using base_type = typename ragged_accessor::accessor;
  using typename base_type::element_type;
  using Offsets = accessor<dense, std::size_t, OP>;
  using row = util::span<element_type>;

  ragged_accessor(const base_type & b)
    : base_type(b), off(this->identifier()) {}

  row operator[](std::size_t i) const {
    // Without an extra element, we must store one endpoint implicitly.
    // Storing the end usefully ignores any overallocation.
    return this->span().first(off(i)).subspan(i ? off(i - 1) : 0);
  }
  std::size_t size() const noexcept { // not total!
    return off.span().size();
  }

  util::span<element_type> span() const {
    const auto s = off.span();
    return get_base().span().first(s.empty() ? 0 : s.back());
  }

  base_type & get_base() {
    return *this;
  }
  const base_type & get_base() const {
    return *this;
  }
  friend base_type * get_null_base(ragged_accessor *) { // for task_prologue_t
    return nullptr;
  }

  Offsets & get_offsets() {
    return off;
  }
  const Offsets & get_offsets() const {
    return off;
  }
  friend Offsets * get_null_offsets(ragged_accessor *) { // for task_prologue_t
    return nullptr;
  }

  template<class Topo, typename Topo::index_space S>
  static ragged_accessor parameter(
    const field_reference<T, data::ragged, Topo, S> & r) {
    return exec::replace_argument<base_type>(r.template cast<data::raw>());
  }

private:
  Offsets off;
};

template<class T, std::size_t P>
struct accessor<ragged, T, P>
  : ragged_accessor<T, P, privilege_repeat(ro, privilege_count(P))> {
  using accessor::ragged_accessor::ragged_accessor;
};

} // namespace data

template<class T, std::size_t P>
struct exec::detail::task_param<data::accessor<data::raw, T, P>> {
  template<class Topo, typename Topo::index_space S>
  static auto replace(const data::field_reference<T, data::raw, Topo, S> & r) {
    return data::accessor<data::raw, T, P>(r.fid());
  }
};
template<class T, std::size_t S>
struct exec::detail::task_param<data::accessor<data::dense, T, S>> {
  using type = data::accessor<data::dense, T, S>;
  template<class Topo, typename Topo::index_space Space>
  static type replace(
    const data::field_reference<T, data::dense, Topo, Space> & r) {
    return exec::replace_argument<typename type::base_type>(
      r.template cast<data::raw>());
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
template<class T, std::size_t P>
struct exec::detail::task_param<data::accessor<data::ragged, T, P>> {
  using type = data::accessor<data::ragged, T, P>;
  template<class Topo, typename Topo::index_space S>
  static type replace(
    const data::field_reference<T, data::ragged, Topo, S> & r) {
    return type::parameter(r);
  }
};

} // namespace flecsi
