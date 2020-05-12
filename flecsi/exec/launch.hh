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
#error Do not include this file directly
#endif

#include <cstddef>
#include <type_traits>
#include <utility>

namespace flecsi {
namespace exec {

namespace detail {
// We care about value category, so we want to use perfect forwarding.
// However, such a template is a better match for some arguments than any
// single non-template overload, so we use SFINAE to detect that we have
// no replacement defined for an argument.
template<class = void>
struct task_param {};
template<class P, class A, class = void> // A is a reference type
struct replace_argument {
  static A replace(A a) {
    return static_cast<A>(a);
  }
};
template<class P, class A>
struct replace_argument<P,
  A,
  decltype(void(task_param<P>::replace(std::declval<A>())))> {
  static decltype(auto) replace(A a) {
    return task_param<P>::replace(static_cast<A>(a));
  }
};
} // namespace detail
// Replaces certain task arguments before conversion to the parameter type.
template<class P, class T>
decltype(auto)
replace_argument(T && t) {
  return detail::replace_argument<std::decay_t<P>, T &&>::replace(
    std::forward<T>(t));
}

enum class launch_type_t : size_t { single, index };

/// A launch domain with a static identity but a runtime size.
struct launch_domain {
  explicit constexpr launch_domain(std::size_t s = 0) : sz(s) {}
  void size(std::size_t s) {
    sz = s;
  }
  constexpr std::size_t size() const {
    return sz;
  }
  constexpr bool operator==(const launch_domain & o) const {
    return this == &o;
  }
  constexpr bool operator!=(const launch_domain & o) const {
    return !(*this == o);
  }

private:
  std::size_t sz;
};

} // namespace exec

inline constexpr exec::launch_domain single(1), index;

/*!
  Single or multiple future.

  @tparam Return The return type of the task.
  @tparam Launch FleCSI launch type: single/index.

  @ingroup execution
*/
template<typename Return,
  exec::launch_type_t Launch = exec::launch_type_t::single>
struct future;

#ifdef DOXYGEN // implemented per-backend
template<typename Return>
struct future<Return> {
  /// Wait on the task to finish.
  void wait() const;
  /// Get the task's result.
  Return get(bool silence_warnings = false) const;
};

template<typename Return>
struct future<Return, exec::launch_type_t::index> {
  /// Wait on all the tasks to finish.
  void wait(bool silence_warnings = false) const;
  /// Get the result of one of the tasks.
  Return get(std::size_t index = 0, bool silence_warnings = false) const;
  /// Get the number of tasks.
  std::size_t size() const;
};
#endif

namespace exec {
template<class R>
struct detail::task_param<future<R>> {
  static future<R> replace(const future<R, launch_type_t::index> &) {
    return {};
  }
};

template<class>
inline constexpr bool is_index_future = false;
template<class R>
inline constexpr bool is_index_future<future<R, launch_type_t::index>> = true;
} // namespace exec

} // namespace flecsi
