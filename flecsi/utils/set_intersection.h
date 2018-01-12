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


#include <algorithm>

namespace flecsi {
namespace utils {

//!
//! \brief  Detect intersections of sorted lists.
//! \remark This function has complexity O(n + m)
//!
template<class InputIt1, class InputIt2>
bool
intersects(InputIt1 first1, InputIt1 last1, InputIt2 first2, InputIt2 last2) {
  while (first1 != last1 && first2 != last2) {
    if (*first1 < *first2) {
      ++first1;
      continue;
    }
    if (*first2 < *first1) {
      ++first2;
      continue;
    }
    return true;
  }
  return false;
}

#if 0
//!
//! \brief  Detect intersections of sorted lists.
//!
//! \remark When input1 is much smaller that input2, this gives O(n * log(m))
//!         time.
//!
template<class InputIt1, class InputIt2>
bool intersects(
  InputIt1 first1, InputIt1 last1,
  InputIt2 first2, InputIt2 last2)
{
  while (first1 != last1)
    if (std::binary_search(first2, last2, *first1++))
      return true;
  return false;
}
#endif

} // namespace utils
} // namespace flecsi
