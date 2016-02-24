/*~-------------------------------------------------------------------------~~*
 *  @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
 * /@@/////  /@@          @@////@@ @@////// /@@
 * /@@       /@@  @@@@@  @@    // /@@       /@@
 * /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
 * /@@////   /@@/@@@@@@@/@@       ////////@@/@@
 * /@@       /@@/@@//// //@@    @@       /@@/@@
 * /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
 * //       ///  //////   //////  ////////  //
 *
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~-------------------------------------------------------------------------~~*/
/*!
 *
 * \file tuple_visit.h
 *
 * \brief Loop through a list of tuples and apply a specific function.
 *
 * \remark this is the main implementation
 *
 ******************************************************************************/
#pragma once

namespace flecsi
{
namespace utils
{
namespace detail
{
//! support struct to iterate over the tuple(s)
template <size_t size>
struct visit_tuple_ws {
  template <typename Callable, typename Head, typename... Tail>
  static void visit(Callable && f, Head && aTuple, Tail &&... aTail)
  {
    using std::get;
    visit_tuple_ws<size - 1>::visit(std::forward<Callable>(f),
        std::forward<Head>(aTuple), std::forward<Tail>(aTail)...);
    f(get<size>(std::forward<Head>(aTuple)),
        get<size>(std::forward<Tail>(aTail))...);
  }
};

//! stop recursion here
template <>
struct visit_tuple_ws<0u> {
  template <typename Callable, typename Head, typename... Tail>
  static void visit(Callable && f, Head && aTuple, Tail &&... aTail)
  {
    using std::get;
    f(get<0>(std::forward<Head>(aTuple)), get<0>(std::forward<Tail>(aTail))...);
  }
};

} // namespace
} // namespace
} // namespace
