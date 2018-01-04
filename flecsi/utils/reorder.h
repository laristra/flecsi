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


#include <iterator>
#include <utility>

namespace flecsi {
namespace utils {

//!
//! \brief Reorders an array in place
//! \remark this version maintains the order array
//! \param [in] order_begin The begin iterator for the order array
//! \param [in] order_end   The end iterator for the order array
//! \param [in,out] v The begin iterator for the value array
//!
template<typename order_iterator, typename value_iterator>
void
reorder(
    const order_iterator order_begin,
    const order_iterator order_end,
    const value_iterator v) {
  using index_t = typename std::iterator_traits<order_iterator>::value_type;
  using diff_t = typename std::iterator_traits<order_iterator>::difference_type;

  auto remaining = order_end - 1 - order_begin;
  for (index_t s = index_t(), d; remaining > 0; ++s) {
    for (d = order_begin[diff_t(s)]; d > s; d = order_begin[diff_t(d)])
      ;
    if (d == s) {
      --remaining;
      auto temp = v[diff_t(s)];
      while (d = order_begin[diff_t(d)], d != s) {
        std::swap(temp, v[diff_t(d)]);
        --remaining;
      }
      v[diff_t(s)] = temp;
    }
  }
}

//!
//! \brief Reorders an array in place
//! \remark this version destroys the order array for performance gains
//! \param [in,out] order_begin The begin iterator for the order array
//! \param [in,out] order_end   The end iterator for the order array
//! \param [in,out] v The begin iterator for the value array
//!
template<typename order_iterator, typename value_iterator>
void
reorder_destructive(
    const order_iterator order_begin,
    const order_iterator order_end,
    const value_iterator v) {
  using index_t = typename std::iterator_traits<order_iterator>::value_type;
  using diff_t = typename std::iterator_traits<order_iterator>::difference_type;

  auto remaining = order_end - 1 - order_begin;
  for (auto s = index_t(); remaining > 0; ++s) {
    auto d = order_begin[diff_t(s)];
    if (d == index_t(-1))
      continue;
    --remaining;
    auto temp = v[diff_t(s)];
    for (index_t d2; d != s; d = d2) {
      std::swap(temp, v[diff_t(d)]);
      std::swap(order_begin[diff_t(d)], d2 = index_t(-1));
      --remaining;
    }
    v[diff_t(s)] = temp;
  }
}

} // namespace utils
} // namespace flecsi
