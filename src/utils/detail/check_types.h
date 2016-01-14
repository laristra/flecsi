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
 * \file check_types.h
 * 
 * \brief Some helper functions to check variable types.
 *
 ******************************************************************************/
#pragma once



namespace flecsi {
namespace utils {
namespace detail {



template<typename... Conds>
struct and_ : std::true_type {};

// combine conditions
template<typename Cond, typename... Conds>
struct and_<Cond, Conds...> : 
  std::conditional< Cond::value, 
                    and_<Conds...>,
                    std::false_type >::type 
{};


} // namespace
} // namespace
} // namespace
