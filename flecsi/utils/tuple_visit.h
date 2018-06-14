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

template <typename _Fn, typename _Tuple, size_t... _Idx>
void
__tuple_visit_impl(_Fn&& __f, _Tuple&& __t, index_sequence<_Idx...>)
{
  // This treats creates an initializer list so that the index
  // sequence is expanded properly. The (f(x), 0) uses the comma operator
  // so that f is evaluated, but the resulting value is discarded. The value
  // of the entire expression is 0. Attribution: Andy Prowl on stackoverflow.
  auto _l = { (__f(std::get<_Idx>(__t)), 0)... };
}

/*!
  Apply the given callable object to each element of the given tuple.

  @param __f Callable object.
  @param __t Tuple.
  
  @note This is written in the style of the standard library so that it
        can be used in a proposal to add this to the standard.
 */

template <typename _Fn, typename _Tuple>
void
tuple_visit(_Fn&& __f, _Tuple&& __t)
{
  using _Indices = make_index_sequence<tuple_size_v<decay_t<_Tuple>>>;
  __tuple_visit_impl(std::forward<_Fn>(__f), std::forward<_Tuple>(__t),
  _Indices{});
}

} // namespace utils
} // namespace flecsi
