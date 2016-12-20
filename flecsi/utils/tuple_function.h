/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_utils_tuple_function_h
#define flecsi_utils_tuple_function_h

#include <utility>

/*!
 * \file
 * \date Initial file creation: Aug 01, 2016
 */

namespace flecsi {
namespace utils {

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


template<typename T, typename ... As, size_t ... Is>
std::function<void()> tuple_function_mpi(T & f, std::tuple<As ...> & t,
  std::index_sequence<Is ...>) {
  
  return std::bind(f, std::get<Is>(t) ...);
//  ext_legion_handshake_t::instance().shared_func_=shared_func_tmp;
//    return f(std::get<Is>(t) ...);
} // tuple_function

template<typename T, typename ... As>
std::function<void()> tuple_function_mpi(T & f, std::tuple<As ...> & t) {
  return tuple_function_mpi(f, t,
    std::make_integer_sequence<size_t, sizeof ... (As)>{});
} // tuple_function


} // namespace utils
} // namespace flecsi

#endif // flecsi_utils_tuple_function_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
