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
#error Do not include this file directly
#else
#include <flecsi/utils/tuple_function.hh>
#endif

#include <functional>

namespace flecsi {
namespace execution {

/*!
  @tparam RETURN    Return value type.
  @tparam ARG_TUPLE Argument type (std::tuple).
 */

template<typename RETURN, typename ARG_TUPLE>
struct function_handle {
  using return_t = RETURN;
  using arg_tuple_t = ARG_TUPLE;

  /*!
    Constructor.

    @param key A hash key identifier for the function.
   */

  constexpr function_handle(const size_t key = 0) : key_(key) {}

  /*!
    Execute the function.
   */

  RETURN
  operator()(void * function, ARG_TUPLE && args) {
    auto user_function = (reinterpret_cast<RETURN (*)(ARG_TUPLE)>(function));
    return user_function(std::forward<ARG_TUPLE>(args));
  } // operator ()

  function_handle & operator=(const function_handle & fh) {
    this->key_ = fh.key_;
    return *this;
  } // operator =

  /*!
    Set the identifier key.
   */

  size_t set_key(size_t key) {
    key_ = key;
    return key_;
  } // set_key

  /*!
    Return the identifier key.
   */

  size_t get_key() const {
    return key_;
  } // key

private:
  size_t key_;

}; // class function_handle

} // namespace execution
} // namespace flecsi
