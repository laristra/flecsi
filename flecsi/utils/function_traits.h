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

namespace flecsi {
namespace utils {

template<typename T>
struct function_traits_u : function_traits_u<decltype(&T::operator())> {};

template<typename R, typename... As>
struct function_traits_u<R(As...)> {
  using return_type = R;
  using arguments_type = std::tuple<As...>;
};

template<typename R, typename... As>
struct function_traits_u<R (*)(As...)> : public function_traits_u<R(As...)> {};

template<typename C, typename R, typename... As>
struct function_traits_u<R (C::*)(As...)> : public function_traits_u<R(As...)> {
  using owner_type = C;
};

template<typename C, typename R, typename... As>
struct function_traits_u<R (C::*)(As...) const>
    : public function_traits_u<R(As...)> {
  using owner_type = C;
};

template<typename C, typename R, typename... As>
struct function_traits_u<R (C::*)(As...) volatile>
    : public function_traits_u<R(As...)> {
  using owner_type = C;
};

template<typename C, typename R, typename... As>
struct function_traits_u<R (C::*)(As...) const volatile>
    : public function_traits_u<R(As...)> {
  using owner_type = C;
};

template<typename R, typename... As>
struct function_traits_u<std::function<R(As...)>>
    : public function_traits_u<R(As...)> {};

template<typename T>
struct function_traits_u<T &> : public function_traits_u<T> {};
template<typename T>
struct function_traits_u<const T &> : public function_traits_u<T> {};
template<typename T>
struct function_traits_u<volatile T &> : public function_traits_u<T> {};
template<typename T>
struct function_traits_u<const volatile T &> : public function_traits_u<T> {};
template<typename T>
struct function_traits_u<T &&> : public function_traits_u<T> {};
template<typename T>
struct function_traits_u<const T &&> : public function_traits_u<T> {};
template<typename T>
struct function_traits_u<volatile T &&> : public function_traits_u<T> {};
template<typename T>
struct function_traits_u<const volatile T &&> : public function_traits_u<T> {};

} // namespace utils
} // namespace flecsi
