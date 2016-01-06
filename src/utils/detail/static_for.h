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
 * \file static_for.h
 * 
 * \brief A static for-each function for looping over sequences statically.
 *
 * \remark these are the hidden implementation details
 *
 ******************************************************************************/
#pragma once

namespace flexi {
namespace utils {
namespace detail {

//! \brief actuall call to functions
template<size_t... Is, class F>
void static_for( std::index_sequence<Is...>, F&& f ) {
  int unused[] = { 0, ( (void)f(Is), 0 )... };
}

} // namespace 
} // namespace
} // namespace



/*~------------------------------------------------------------------------~--*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/

