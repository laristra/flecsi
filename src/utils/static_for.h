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
 ******************************************************************************/
#pragma once

//! user includes
#include "detail/static_for.h"

namespace flecsi {
namespace utils {


////////////////////////////////////////////////////////////////////////////////
//! \brief Statically loop over integers from 0 to N, executing a function f
////////////////////////////////////////////////////////////////////////////////
template<size_t N, class F>
void static_for(  F&& f ) {
  auto indexes = std::make_index_sequence<N>();
  detail::static_for(indexes, std::forward<F>(f) );
}



} // namespace
} // namespace



/*~------------------------------------------------------------------------~--*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/

