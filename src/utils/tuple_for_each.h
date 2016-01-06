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
 * \file tuple_for_each.h
 * 
 * \brief A static for-each function for looping over tuples statically.
 *
 ******************************************************************************/
#pragma once

//! \brief  unomment to use bens original version
//#define USE_BENS_VERSION


// user includes
#include "detail/tuple_for_each.h"

namespace flexi {
namespace utils {




#ifdef USE_BENS_VERSION

////////////////////////////////////////////////////////////////////////////////
//! \brief Exectute something for each element of a tuple
//! \remark this is ben's version
////////////////////////////////////////////////////////////////////////////////
template<typename TupleType, typename FunctionType>
void tuple_for_each(TupleType && t, FunctionType f)
{
  detail::tuple_for_each(std::forward<TupleType>(t), f,
                         std::integral_constant<size_t, 0>());
}

// delete from scope
#undef USE_BENS_VERSION



#else 

////////////////////////////////////////////////////////////////////////////////
//! \brief Exectute something for each element of a tuple
//! \remark this is my version
////////////////////////////////////////////////////////////////////////////////
template<class Tuple, class F>
void tuple_for_each( Tuple&& tup, F&& f ) {
  auto indexes = detail::get_indexes(tup);
  detail::tuple_for_each(indexes, std::forward<Tuple>(tup), std::forward<F>(f) );
}

#endif


} // namespace
} // namespace



/*~------------------------------------------------------------------------~--*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/

