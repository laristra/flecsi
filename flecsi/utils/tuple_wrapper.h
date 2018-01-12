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

struct generic_tuple_t {};

//!
//! \class tuple_wrapper_ tuple_wrapper.h
//! \brief tuple_wrapper_ provides...
//!
template<typename... Args>
struct tuple_wrapper_ : generic_tuple_t {

  using tuple_t = std::tuple<Args...>;

  tuple_wrapper_(Args... args) : t_(std::make_tuple(args...)) {}

  template<std::size_t I>
  decltype(auto) get() {
    return std::get<I>(t_);
  }

private:
  tuple_t t_;

}; // class tuple_wrapper_

} // namespace utils
} // namespace flecsi

