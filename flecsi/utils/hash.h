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

/*!

  \brief const_string provides compile-time string constants and hashing...
*/
template< typename T, typename U >
constexpr T hash( U && str, std::size_t n ) {

  T h = 0;

  for (std::size_t i = 0; i < n; ++i) 
    h ^= static_cast<T>( std::forward<U>(str)[i] ) << 8 * (i % 8);

  return h;
}


} // namespace flecsi

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
