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

/*! file */

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly!
#endif

#include <cstddef> // size_t

#include <flecsi/utils/type_traits.hh>

namespace flecsi {
namespace topology {

// Declarations for the base topology types.

struct global;
struct index;

struct canonical_base;
template<typename>
struct canonical;

struct ntree_base;
template<typename>
struct ntree;

struct set_base_t;
template<typename>
struct set;

struct structured_base;
template<typename>
struct structured;

struct unstructured_base;
template<typename>
struct unstructured;

namespace detail {

template<class, class = void>
struct core;

template<class T>
struct core<T, utils::voided<utils::base_specialization_t<canonical, T>>> {
  using type = utils::base_specialization_t<canonical, T>;
};

template<class T>
struct core<T, std::enable_if_t<std::is_base_of_v<global, T>>> {
  using type = global;
};

template<class T>
struct core<T, std::enable_if_t<std::is_base_of_v<index, T>>> {
  using type = index;
};

template<class T>
struct core<T, utils::voided<utils::base_specialization_t<ntree, T>>> {
  using type = utils::base_specialization_t<ntree, T>;
};

template<class T>
struct core<T, utils::voided<utils::base_specialization_t<set, T>>> {
  using type = utils::base_specialization_t<set, T>;
};

template<class T>
struct core<T, utils::voided<utils::base_specialization_t<structured, T>>> {
  using type = utils::base_specialization_t<structured, T>;
};

template<class T>
struct core<T, utils::voided<utils::base_specialization_t<unstructured, T>>> {
  using type = utils::base_specialization_t<unstructured, T>;
};

template<class T>
struct category {
  using type = T;
};

template<class P>
struct category<canonical<P>> : category<canonical_base> {};
template<class P>
struct category<ntree<P>> : category<ntree_base> {};
template<class P>
struct category<set<P>> : category<set_base_t> {};
template<class P>
struct category<structured<P>> : category<structured_base> {};
template<class P>
struct category<unstructured<P>> : category<unstructured_base> {};

inline std::size_t next_id;
// Use functions because these are needed during non-local initialization:
template<class>
std::size_t
id() {
  static auto ret = next_id++;
  return ret;
}
} // namespace detail

template<class T>
using core_t = typename detail::core<T>::type;
template<class T>
using category_t = typename detail::category<T>::type; // of a core type only

template<class T>
std::size_t
id() {
  return detail::id<std::remove_cv_t<T>>();
}

template<class, class = void>
inline constexpr std::size_t index_spaces = 1;
template<class T>
inline constexpr std::size_t index_spaces<T, decltype(void(T::index_spaces))> =
  T::index_spaces; // TIP: expression SFINAE uses T::index_spaces if defined

} // namespace topology
} // namespace flecsi
