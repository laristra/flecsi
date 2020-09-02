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

#include <Kokkos_Core.hpp>

namespace flecsi {
namespace exec {

/*!
  This function is a wrapper for Kokkos::parallel_for that has been adapted to
  work with FleCSI's topology iterator types. In particular, this function
  invokes a map from the normal kernel index space to the FleCSI index space,
  which may require indirection.
 */

template<typename ITERATOR, typename LAMBDA>
void
parallel_for(ITERATOR iterator, LAMBDA lambda, std::string name = "") {

  struct functor {

    functor(ITERATOR & iterator, LAMBDA & lambda)
      : iterator_(iterator), lambda_(lambda) {}

    KOKKOS_INLINE_FUNCTION void operator()(int i) const {
      lambda_(iterator_[i]);
    } // operator()

  private:
    ITERATOR & iterator_;
    LAMBDA & lambda_;

  }; // struct functor

  Kokkos::parallel_for(name, iterator.size(), functor{iterator, lambda});

} // parallel_for

/*!
  The forall type provides a pretty interface for invoking data-parallel
  execution.
 */

template<typename ITERATOR>
struct forall_t {

  /*!
    Construct a forall_t instance.

    @param iterator A valid C++ RandomAccess iterator.
    @param name     An optional name that can be used for debugging.
   */

  forall_t(ITERATOR iterator, std::string name = "")
    : iterator_(iterator), name_(name) {}

  /*!
    The functor type wraps FleCSI iterators that have indirection.

    @tparam LAMBDA The user-defined lambda function.
   */

  template<typename LAMBDA>
  struct functor {

    functor(ITERATOR iterator, LAMBDA lambda)
      : iterator_(iterator), lambda_(lambda) {}

    KOKKOS_INLINE_FUNCTION void operator()(int i) const {
      lambda_(iterator_[i]);
    } // operator()

  private:
    ITERATOR iterator_;
    LAMBDA lambda_;

  }; // struct functor

  template<typename LAMBDA>
  void operator+(LAMBDA lambda) {
    Kokkos::parallel_for(
      "  ", iterator_.size(), functor<LAMBDA>{iterator_, lambda});
  } // operator+

  /*!
    This overload of the insertion operator allows pretty syntax when invoking
    forall_t.

    Attribution: Nick Moss
   */
  template<typename CALLABLE>
  void operator<<(CALLABLE l) {
    Kokkos::parallel_for(name_, iterator_.size(), functor{iterator_, l});
  } // operator<<

private:
  ITERATOR iterator_;
  std::string name_;

}; // struct forall_t

#define forall(it, iterator, name)                                             \
  flecsi::exec::forall_t{iterator, name} + KOKKOS_LAMBDA(auto it)

/*!
  This function is a wrapper for Kokkos::parallel_reduce that has been adapted
  to work with FleCSI's topology iterator types.
 */
template<typename ITERATOR, typename LAMBDA, typename REDUCER>
void
parallel_reduce(ITERATOR iterator,
  LAMBDA lambda,
  REDUCER result,
  std::string name = "") {

  using value_type = typename REDUCER::value_type;

  struct functor {

    functor(ITERATOR & iterator, LAMBDA & lambda)
      : iterator_(iterator), lambda_(lambda) {}

    KOKKOS_INLINE_FUNCTION void operator()(int i, value_type & tmp) const {
      lambda_(iterator_[i], tmp);
    } // operator()

  private:
    ITERATOR & iterator_;
    LAMBDA & lambda_;

  }; // struct functor

  Kokkos::parallel_reduce(
    name, iterator.size(), functor{iterator, lambda}, result);

} // parallel_reduce

/*!
  The reduce_all type provides a pretty interface for invoking data-parallel
  reductions.
 */
template<typename ITERATOR, typename REDUCER>
struct reduceall_t {

  reduceall_t(ITERATOR iterator, REDUCER reducer, std::string name = "")
    : iterator_(iterator), reducer_(reducer), name_(name) {}

  using value_type = typename REDUCER::value_type;

  template<typename LAMBDA>
  struct functor {

    functor(ITERATOR & iterator, LAMBDA & lambda)
      : iterator_(iterator), lambda_(lambda) {}

    KOKKOS_INLINE_FUNCTION void operator()(int i, value_type & tmp) const {
      lambda_(iterator_[i], tmp);
    } // operator()

  private:
    ITERATOR iterator_;
    LAMBDA lambda_;

  }; // struct functor

  template<typename LAMBDA>
  void operator+(LAMBDA lambda) {
    Kokkos::parallel_reduce(
      name_, iterator_.size(), functor<LAMBDA>{iterator_, lambda}, reducer_);
  } // operator+

private:
  ITERATOR iterator_;
  REDUCER reducer_;
  std::string name_;

}; // forall_t

#define reduceall(it, tmp, iterator, reducer, name)                            \
  flecsi::exec::reduceall_t{iterator, reducer, name} +                         \
    KOKKOS_LAMBDA(auto it, auto & tmp)

//----------------------------------------------------------------------------//
//! Abstraction function for fine-grained, data-parallel interface.
//!
//! @tparam R range type
//! @tparam FUNCTION    The calleable object type.
//!
//! @param r range over which to execute \a function
//! @param function     The calleable object instance.
//!
//! @ingroup execution
//----------------------------------------------------------------------------//

template<class R, typename FUNCTION>
inline void
for_each(R && r, FUNCTION && function) {
  std::for_each(r.begin(), r.end(), std::forward<FUNCTION>(function));
} // for_each_u

//----------------------------------------------------------------------------//
//! Abstraction function for fine-grained, data-parallel interface.
//!
//! @tparam R range type
//! @tparam FUNCTION    The calleable object type.
//! @tparam REDUCTION   The reduction variabel type.
//!
//! @param r range over which to execute \a function
//! @param function     The calleable object instance.
//!
//! @ingroup execution
//----------------------------------------------------------------------------//

template<class R, typename FUNCTION, typename REDUCTION>
inline void
reduce_each(R && r, REDUCTION & reduction, FUNCTION && function) {
  for(const auto & e : r)
    function(e, reduction);
} // reduce_each_u

// Reducers extracted from Kokkos
namespace reducer {
template<typename TYPE>
using sum = Kokkos::Sum<TYPE>;
template<typename TYPE>
using prod = Kokkos::Prod<TYPE>;
template<typename TYPE>
using max = Kokkos::Max<TYPE>;
template<typename TYPE>
using min = Kokkos::Min<TYPE>;
template<typename TYPE>
using land = Kokkos::LAnd<TYPE>;
template<typename TYPE>
using lor = Kokkos::LOr<TYPE>;
template<typename TYPE>
using band = Kokkos::BAnd<TYPE>;
template<typename TYPE>
using bor = Kokkos::BOr<TYPE>;
template<typename TYPE>
using minmax = Kokkos::MinMax<TYPE>;
template<typename TYPE, typename IDX, typename SPACE>
using minloc = Kokkos::MinLoc<TYPE, IDX, SPACE>;
template<typename TYPE, typename IDX, typename SPACE>
using maxloc = Kokkos::MaxLoc<TYPE, IDX, SPACE>;
template<typename TYPE, typename IDX, typename SPACE>
using minmaxloc = Kokkos::MinMaxLoc<TYPE, IDX, SPACE>;
} // namespace reducer
} // namespace exec
} // namespace flecsi
