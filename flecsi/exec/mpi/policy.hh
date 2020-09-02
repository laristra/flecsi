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

namespace exec::detail {
template<class... PP, class... AA>
auto
replace_arguments(std::tuple<PP...> * /* to deduce PP */, AA &&... aa) {
  // Specify the template arguments explicitly to produce references to
  // unchanged arguments.
  return std::tuple<decltype(exec::replace_argument<PP>(std::forward<AA>(
    aa)))...>(exec::replace_argument<PP>(std::forward<AA>(aa))...);
}
} // namespace exec::detail

template<auto & F, class REDUCTION, size_t ATTRIBUTES, typename... ARGS>
auto
reduce(ARGS &&... args) {
  using Traits = util::function_traits<decltype(F)>;
  using R = typename Traits::return_type;
  auto args2 = exec::detail::replace_arguments(
    static_cast<typename Traits::arguments_type *>(nullptr),
    std::forward<ARGS>(args)...);
  if constexpr(std::is_void_v<R>) {
    std::apply(F, std::move(args2));
    return future<void>{};
  }
  else
    return future<R>{std::apply(F, std::move(args2))};
}

} // namespace flecsi
