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

  template<class,template<class> class>
struct convert_tuple;

  template<typename... Args,template<class> class F>
struct convert_tuple<std::tuple<Args...>,F> {
  using type = std::tuple<F<Args>...>;
};

  template<class T,template<class> class F>
  using convert_tuple_t=typename convert_tuple<T,F>::type;

template<typename T, typename TO, bool E>
struct base_convert_tuple_type_ {
  using type = T;
};

template<typename T, typename TO>
struct base_convert_tuple_type_<T, TO, true> {
  using type = TO;
};

template<typename T, typename TO>
struct base_convert_tuple_type_<T, TO, false> {
  using type = T;
};

template<class B, typename TO, typename... Args>
struct base_convert_tuple_type;

template<class B, typename TO, typename... Args>
struct base_convert_tuple_type<B, TO, std::tuple<Args...>> {
  using type = std::tuple<typename base_convert_tuple_type_<Args,
    TO,
    std::is_base_of<B, Args>::value>::type...>;
};

} // namespace utils
} // namespace flecsi
