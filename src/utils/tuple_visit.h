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
 * \file tuple_visit.h
 * 
 * \brief Loop through a list of tuples and apply a specific function.
 *
 ******************************************************************************/
#pragma once

// user includes
#include "detail/tuple_visit.h"

namespace flexi {
namespace utils {

////////////////////////////////////////////////////////////////////////////////
//! \brief Loop through a list of tuples and apply a specific function
//! \remark f can be a functor which gets returned
////////////////////////////////////////////////////////////////////////////////
template<typename Callable, typename Head, typename... Tail>
Callable tuple_visit(Callable&& f, Head&& aTuple, Tail&&... aTail)
{
  const size_t size = std::tuple_size<typename std::remove_reference<Head>::type>::value;
  detail::visit_tuple_ws<size-1>::visit( std::forward<Callable>(f), 
                                         std::forward<Head>(aTuple), 
                                         std::forward<Tail>(aTail)... );
  return f;
}


} // namespace
} // namespace



/*~------------------------------------------------------------------------~--*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
