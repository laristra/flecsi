/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_tuple_function_h
#define flecsi_tuple_function_h

#include <utility>

/*!
 * \file tuple_function.h
 * \authors bergen
 * \date Initial file creation: Aug 01, 2016
 */

namespace flecsi {

template<typename T, typename ... As, size_t ... Is>
decltype(auto) tuple_function(T & f, std::tuple<As ...> & t,
  std::index_sequence<Is ...>) {
    return f(std::get<Is>(t) ...);
} // tuple_function

template<typename T, typename ... As>
decltype(auto) tuple_function(T & f, std::tuple<As ...> & t) {
  return tuple_function(f, t,
    std::make_integer_sequence<size_t, sizeof ... (As)>{});
} // tuple_function

} // namespace flecsi

#endif // flecsi_tuple_function_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
