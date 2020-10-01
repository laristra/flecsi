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
#endif

#include "flecsi/data/field.hh"
#include "flecsi/util/array_ref.hh"
#include "flecsi/util/constant.hh"

#include <type_traits>

namespace flecsi {
namespace topo {

namespace detail {
template<class, class>
struct connect;
template<class P, class... VT>
struct connect<P, util::types<VT...>> {
  using type = util::key_tuple<util::key_type<VT::value,
    util::key_array<field<util::id, data::ragged>::definition<P, VT::value>,
      typename VT::type>>...>;
};

template<class, std::size_t>
struct connect_access;
template<class... VT, std::size_t Priv>
struct connect_access<util::key_tuple<VT...>, Priv> {
  using type = util::key_tuple<util::key_type<VT::value,
    util::key_array<
      typename VT::type::value_type::Field::template accessor1<Priv>,
      typename VT::type::keys>>...>;
};
} // namespace detail

// Construct a "sparse matrix" of field definitions; the row is the source
// index space (which is enough to determine the field type) and the column is
// the destination.
template<class P>
using connect_t = typename detail::connect<P, typename P::connectivities>::type;

namespace detail {
template<class P, std::size_t Priv>
using connect_access_t = typename connect_access<connect_t<P>, Priv>::type;
}

// A parallel sparse matrix of accessors.
template<class P, std::size_t Priv>
struct connect_access : detail::connect_access_t<P, Priv> {
  // The argument type is just connect_t<P>, but we want the VT pack.
  // Prior to C++20, accessor_member can't refer to the subobjects of a
  // connect_t, so the accessors must be initialized externally.
  template<class... VT>
  connect_access(const util::key_tuple<VT...> & c)
    : detail::connect_access_t<P, Priv>(
        make_from<std::decay_t<decltype(this->template get<VT::value>())>>(
          c.template get<VT::value>())...) {}

private:
  // The .get<>s here and above just access the elements in order, of course.
  template<class T, class U, auto... VV>
  static T make_from(const util::key_array<U, util::constants<VV...>> & m) {
    return {{typename T::value_type(m.template get<VV>().fid)...}};
  }
};

// For either kind of sparse matrix:
template<class F, class T>
void
connect_visit(F && f, T && t) {
  std::forward<T>(t).apply([&](auto &... rr) {
    (
      [&](auto & r) {
        for(auto & x : r)
          std::forward<F>(f)(x);
      }(rr),
      ...);
  });
}

// A "strong typedef" for T that supports overload resolution, template
// argument deduction, and limited arithmetic.
// The first parameter differentiates topologies/index spaces.
template<auto, class T = util::id>
struct id {
  using difference_type = std::make_signed_t<T>;

  id() = default; // allow trivial default initialization
  explicit id(T t) : t(t) {}

  T operator+() const {
    return t;
  }
  operator T() const {
    return t;
  }

  // Prevent assigning to transform_view results:
  id & operator=(const id &) & = default;

  id & operator++() & {
    ++t;
    return *this;
  }
  id operator++(int) & {
    id ret = *this;
    ++*this;
    return ret;
  }
  id & operator--() & {
    --t;
    return *this;
  }
  id operator--(int) & {
    id ret = *this;
    --*this;
    return ret;
  }

  id & operator+=(difference_type d) & {
    t += d;
    return *this;
  }
  id operator+(difference_type d) const {
    return d + *this;
  }
  void operator+(id) const = delete;
  friend id operator+(difference_type d, id i) {
    return i += d;
  }
  id & operator-=(difference_type d) & {
    t -= d;
    return *this;
  }
  id operator-(difference_type d) const {
    return d - *this;
  }
  difference_type operator-(id i) const { // also avoids ambiguity
    return difference_type(t) - difference_type(i.t);
  }

private:
  T t;
};

template<auto S, class C>
auto make_ids(C && c) { // NB: return value may be lifetime-bound to c
  return util::transform_view(
    std::forward<C>(c), [](const auto & x) { return id<S>(x); });
}

} // namespace topo
} // namespace flecsi
