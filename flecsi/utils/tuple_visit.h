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

#include <tuple>

namespace flecsi {
namespace utils {

template <typename _Tuple, typename _Fn, size_t... _Idx>
void
tuple_visit_impl(_Tuple&& t, _Fn&& f, std::index_sequence<_Idx...>)
{
  /*
    This creates an initializer list so that the index sequence is
    expanded properly. The (f(x), 0) uses the comma operator so
    that f is evaluated, but the resulting value is discarded. The value
    of the entire expression is 0.

    Attribution: Andy Prowl on stackoverflow.
   */
  auto _l = { (f(_Idx, std::get<_Idx>(t)), 0)... };
}

/*!
  Apply the given callable object to each element of the given tuple.

  @tparam _Tuple The tuple type.
  @tparam _Fn    Callable object type.

  @param f Callable object.
  @param t Tuple.
  
  @note This is written in the style of the standard library so that it
        can be used in a proposal to add this to the standard.
 */

template <typename _Tuple, typename _Fn>
void
tuple_visit(_Tuple&& t, _Fn&& f)
{
  using _Indices =
    std::make_index_sequence<std::tuple_size_v<std::decay_t<_Tuple>>>;

  tuple_visit_impl(std::forward<_Tuple>(t), std::forward<_Fn>(f),
    _Indices{});
}

/*!
  Apply the given callable object to each element of the given tuple type.

  @tparam _Tuple The tuple type. This type must be default constructible,
                 which implies that each element type must be default
                 constructible.
  @tparam _Fn    Callable object type.

  @param f Callable object.
  
  @note This is written in the style of the standard library so that it
        can be used in a proposal to add this to the standard.
 */

template <typename _Tuple, typename _Fn>
void
tuple_visit(_Fn&& f)
{
  static_assert(std::is_default_constructible<_Tuple>::value,
    "tuple_vist tuple type is not default constructible");

  using _Indices =
    std::make_index_sequence<std::tuple_size_v<std::decay_t<_Tuple>>>;

  tuple_visit_impl(std::forward<_Tuple>({}), std::forward<_Fn>(f),
    _Indices{});
}

} // namespace utils
} // namespace flecsi
