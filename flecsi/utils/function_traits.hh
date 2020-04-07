/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Triad National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

/*! @file */

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly!
#endif

#include <functional>
#include <tuple>

namespace flecsi {
namespace utils {

template<typename T>
struct function_traits : function_traits<decltype(&T::operator())> {};

template<typename R, typename... As>
struct function_traits<R(As...)> {
  using return_type = R;
  using arguments_type = std::tuple<As...>;
};

template<typename R, typename... As>
struct function_traits<R (*)(As...)> : public function_traits<R(As...)> {};

template<typename C, typename R, typename... As>
struct function_traits<R (C::*)(As...)> : public function_traits<R(As...)> {
  using owner_type = C;
};

template<typename C, typename R, typename... As>
struct function_traits<R (C::*)(As...) const>
  : function_traits<R (C::*)(As...)> {};

template<typename C, typename R, typename... As>
struct function_traits<R (C::*)(As...) volatile>
  : function_traits<R (C::*)(As...)> {};

template<typename C, typename R, typename... As>
struct function_traits<R (C::*)(As...) const volatile>
  : function_traits<R (C::*)(As...)> {};

template<typename R, typename... As>
struct function_traits<std::function<R(As...)>>
  : public function_traits<R(As...)> {};

template<typename T>
struct function_traits<T &> : public function_traits<T> {};
template<typename T>
struct function_traits<const T &> : public function_traits<T> {};
template<typename T>
struct function_traits<volatile T &> : public function_traits<T> {};
template<typename T>
struct function_traits<const volatile T &> : public function_traits<T> {};
template<typename T>
struct function_traits<T &&> : public function_traits<T> {};
template<typename T>
struct function_traits<const T &&> : public function_traits<T> {};
template<typename T>
struct function_traits<volatile T &&> : public function_traits<T> {};
template<typename T>
struct function_traits<const volatile T &&> : public function_traits<T> {};

} // namespace utils
} // namespace flecsi
