/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

/*! @file */


#include <tuple>

namespace flecsi {
namespace utils {

template<typename T, std::size_t... Is>
decltype(auto)
tuple_function(T & f, std::tuple<> & t, std::index_sequence<Is...>) {
  return f(std::get<Is>(t)...);
} // tuple_function

template<typename T>
decltype(auto)
tuple_function(T & f, std::tuple<> & t) {
  return tuple_function(f, t, std::make_integer_sequence<std::size_t, 0>{});
} // tuple_function

template<typename T, typename... As, std::size_t... Is>
decltype(auto)
tuple_function(T & f, std::tuple<As...> & t, std::index_sequence<Is...>) {
  return f(std::get<Is>(t)...);
} // tuple_function

template<typename T, typename... As>
decltype(auto)
tuple_function(T & f, std::tuple<As...> & t) {
  return tuple_function(
      f, t, std::make_integer_sequence<std::size_t, sizeof...(As)>{});
} // tuple_function

/*
// FIXME: Don't need this
template<typename T, typename ... As, std::size_t ... Is>
std::function<void()> tuple_function_mpi(T & f, std::tuple<As ...> & t,
  std::index_sequence<Is ...>) {

  return std::bind(f, std::get<Is>(t) ...);
//  ext_legion_handshake_t::instance().shared_func_=shared_func_tmp;
//    return f(std::get<Is>(t) ...);
} // tuple_function

template<typename T, typename ... As>
std::function<void()> tuple_function_mpi(T & f, std::tuple<As ...> & t) {
  return tuple_function_mpi(f, t,
    std::make_integer_sequence<std::size_t, sizeof ... (As)>{});
} // tuple_function
*/

} // namespace utils
} // namespace flecsi
