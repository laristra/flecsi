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

#if !defined(FLECSI_ENABLE_KOKKOS)
#error FLECSI_ENABLE_KOKKOS not defined! This file depends on Kokkos!
#endif

#include <kokkos.h>

#include <string>

namespace flecsi {
namespace execution {

/*!
  This function is a wrapper for Kokkos::parallel_for that has been adapted to
  work with FleCSI's topology iterator types. In particular, this function
  invokes a map from the normal kernel index space to the FleCSI index space,
  which may require indirection.
 */

template<typename ITERATOR, typename LAMBDA>
parallel_for(ITERATOR const iterator,
  LAMBDA const lambda,
  std::string const & name = "") {

  struct functor_t {

    functor_t(ITERATOR & iterator, LAMBDA & lambda)
      : iterator_(iterator), lambda_(lambda) {}

    KOKKOS_INLINE_FUNCTION void operator()(int i) const {
      lambda(iterator[i]);
    } // operator()

  private:
    ITERATOR & iterator_;
    LAMBDA & lambda_;

  }; // struct functor_t

  Kokkos::parallel_for(name, iterator.size(), functor_t{iterator, lambda});

} // parallel_for

/*!
  The forall_u type provides a pretty interface for invoking data-parallel
  execution.
 */

// We will need this when/if we begin to customize behavior based on
// ITERATOR type.
// template<typename ITERATOR> struct forall_u {};

template<typename ITERATOR>
struct forall_u {

  /*!
    Construct a forall_u instance.

    @param iterator A valid C++ RandomAccess iterator.
    @param name     An optional name that can be used for debugging.
   */

  forall_u(ITERATOR iterator, std::string const & name = "")
    : iterator_(iterator), name_(name) {}

  /*!
    The functor_u type wraps FleCSI iterators that have indirection.

    @tparam LAMBDA The user-defined lambda function.
   */

  template<typename LAMBDA>
  struct functor_u {

    functor_u(ITERATOR iterator, LAMBDA lambda, std::string const & name)
      : iterator_(iterator), lambda_(lambda), name_(name) {}

    void operator()(int64_t i) const {
      lambda_(iterator_[i]);
    } // operator()

  private:
    ITERATOR iterator_;
    LAMBDA lambda_;
    std::string & const name_;

  }; // struct functor_t

  /*!
    This overload of the insertion operator allows pretty syntax when invoking
    forall.

    Attribution: Nick Moss
   */

  template<typename CALLABLE>
  void operator<<(CALLABLE l) {
    Kokkos::parallel_for(
      name_, iterator_.size(), functor_t{iterator_, lambda_});
  } // operator<<

private:
  ITERATOR iterator_;
  std::string & const name_;

}; // struct forall_u

} // namespace execution
} // namespace flecsi
