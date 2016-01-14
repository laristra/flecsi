/*~-------------------------------------------------------------------------~~*
 *     _   ______________     ___    __    ______
 *    / | / / ____/ ____/    /   |  / /   / ____/
 *   /  |/ / / __/ /  ______/ /| | / /   / __/   
 *  / /|  / /_/ / /__/_____/ ___ |/ /___/ /___   
 * /_/ |_/\____/\____/    /_/  |_/_____/_____/   
 * 
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~-------------------------------------------------------------------------~~*/
/*!
 *
 * \file tuple_zip.h
 * 
 * \brief Some helper functions for zipping tupples together.
 *
 ******************************************************************************/
#pragma once


// user includes
#include "detail/tuple_zip.h"

namespace flecsi {
namespace utils {


////////////////////////////////////////////////////////////////////////////////
//! \brief tie two tuples together
////////////////////////////////////////////////////////////////////////////////
template <class Tup1, class Tup2>
decltype(auto) tuple_tie(Tup1&& t1, Tup2&& t2)  {

  constexpr auto num_args1 = std::tuple_size<std::decay_t<Tup1>>::value;
  constexpr auto num_args2 = std::tuple_size<std::decay_t<Tup2>>::value;
  static_assert(num_args1 == num_args2, "The tuple sizes must be the same");

  return detail::tuple_tie_helper( std::forward<Tup1>(t1), std::forward<Tup2>(t2), 
                                   typename detail::gens<num_args2>::type() );

}


////////////////////////////////////////////////////////////////////////////////
//! \brief zip two tuples together
////////////////////////////////////////////////////////////////////////////////
template <class Tup1, class Tup2>
decltype(auto) tuple_zip(Tup1&& t1, Tup2&& t2)  {
  constexpr auto num_args1 = std::tuple_size<std::decay_t<Tup1>>::value;
  constexpr auto num_args2 = std::tuple_size<std::decay_t<Tup2>>::value;
  static_assert(num_args1 == num_args2, "The tuple sizes must be the same");

  return detail::tuple_zip_helper( std::forward<Tup1>(t1), std::forward<Tup2>(t2), 
                                   typename detail::gens<num_args2>::type() );
}


} // namespace
} // namespace



/*~------------------------------------------------------------------------~--*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
