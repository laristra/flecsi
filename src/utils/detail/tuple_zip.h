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

namespace flecsi {
namespace utils {
namespace detail {


//! helpers for generating sequences
template<size_t ...S>
struct seq { };

template<size_t N, size_t ...S>
struct gens : gens<N-1, N-1, S...> 
{ };

template<size_t ...S>
struct gens<0, S...> {
  using type = seq<S...>;
};

//! zip each tuple together
template <class Tup1, class Tup2, size_t ...S>
decltype(auto) tuple_zip_helper(Tup1&& t1, Tup2&& t2, seq<S...> s)  {
  using std::get;
  return std::make_tuple( std::make_pair( get<S>(t1), get<S>(t2) )...);
}


//! tie each tuple together
template <class Tup1, class Tup2, size_t ...S>
decltype(auto) tuple_tie_helper(Tup1&& t1, Tup2&& t2, seq<S...> s)  {
  using std::get;
  using std::forward_as_tuple;
  return std::make_tuple( forward_as_tuple( get<S>(t1), get<S>(t2) )...);
}

} // namespace
} // namespace
} // namespace
