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
 * \file set_intersect.h
 *
 * \brief detect set intersections.
 *
 ******************************************************************************/
#pragma once

// system includes
#include <algorithm>


namespace flecsi
{
namespace utils
{

////////////////////////////////////////////////////////////////////////////////
//! \brief Exectute something for each element of a tuple
//! \remark This function has complexity O(n + m)
////////////////////////////////////////////////////////////////////////////////
template<class InputIt1, class InputIt2>
bool intersects(InputIt1 first1, InputIt1 last1, InputIt2 first2, InputIt2 last2)
{
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
////////////////////////////////////////////////////////////////////////////////
//! \brief detect intersecitions of sorted lists
//!
//! \remark  When input1 is much smaller that input2, this gives O(n * log(m)) 
//!          time.
////////////////////////////////////////////////////////////////////////////////
template<class InputIt1, class InputIt2>
bool intersects(InputIt1 first1, InputIt1 last1, InputIt2 first2, InputIt2 last2) 
{
  while (first1 != last1)
    if (std::binary_search(first2, last2, *first1++))
      return true;
  return false;
}
#endif

} // namespace
} // namespace

/*~------------------------------------------------------------------------~--*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
