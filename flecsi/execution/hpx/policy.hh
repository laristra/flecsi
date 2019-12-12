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

// #if !defined(__FLECSI_PRIVATE__)
// #error Do not include this file directly!
// #endif

// #include "flecsi/utils/function_traits.hh"
// #include "future.hh"
// #include <flecsi/execution/mpi/reduction_wrapper.hh>
// #include <flecsi/utils/flog.hh>

// #include <type_traits>
// #include <utility> // forward

// namespace flecsi {

// template<auto & F,
//   size_t LAUNCH_DOMAIN,
//   size_t REDUCTION,
//   size_t ATTRIBUTES,
//   typename... ARGS>
// decltype(auto)
// reduce(ARGS &&... args) {
//   using R = typename utils::function_traits<decltype(F)>::return_type;
//   execution::mpi_future<R> ret;
//   if constexpr(std::is_same_v<R, void>)
//     F(std::forward<ARGS>(args)...);
//   else
//     ret.set(F(std::forward<ARGS>(args)...));
//   return ret;
// }

// namespace execution {

// //--------------------------------------------------------------------------//
// // Reduction interface.
// //--------------------------------------------------------------------------//

// template<size_t HASH, typename TYPE>
// using reduction_wrapper = mpi::reduction_wrapper<HASH, TYPE>;

// } // namespace execution
// } // namespace flecsi
