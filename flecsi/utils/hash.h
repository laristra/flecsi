/*~--------------------------------------------------------------------------~*
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
 *~--------------------------------------------------------------------------~*/

#pragma once

/*!
 * \file const_string.h
 * \authors nickm
 * \date Initial file creation: Oct 15, 2015
 */

#include <limits>

namespace flecsi
{

template< typename T, typename U>
constexpr T hash__( U && str, T h, std::size_t i, std::size_t n ){
  return i == n ? h : hash__(str, h ^ static_cast<T>( std::forward<U>(str)[i] ) <<
    8 * (i % 8), i + 1, n); 
}

/*!
  \brief const_string provides compile-time string constants and hashing...
*/
template< typename T, typename U>
constexpr T hash( U && str, std::size_t n ) {
  return hash__(str, 0, 0, n);
}

} // namespace flecsi

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
