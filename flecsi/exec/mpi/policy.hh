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

#include "flecsi/exec/mpi/reduction_wrapper.hh"
#include "flecsi/util/function_traits.hh"
#include "future.hh"
#include <flecsi/flog.hh>

#include <type_traits>
#include <utility> // forward

namespace flecsi {

template<auto & F,
  const exec::launch_domain & LAUNCH_DOMAIN,
  class REDUCTION,
  size_t ATTRIBUTES,
  typename... ARGS>
decltype(auto)
reduce(ARGS &&... args) {
  using R = typename util::function_traits<decltype(F)>::return_type;
  if constexpr(std::is_void_v<R>) {
    F(std::forward<ARGS>(args)...);
    return future<void>{};
  }
  else
    return future<R>{F(std::forward<ARGS>(args)...)};
}

} // namespace flecsi
