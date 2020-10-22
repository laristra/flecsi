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

#include "flecsi/exec/fold.hh"

#include <Kokkos_Core.hpp>

namespace flecsi {
namespace exec {
namespace kok {
template<class R, class T, class = void>
struct wrap {
  using reducer = wrap;
  using value_type = T;
  using result_view_type = Kokkos::View<value_type, Kokkos::HostSpace>;

  KOKKOS_INLINE_FUNCTION
  void join(T & a, const T & b) const {
    a = R::combine(a, b);
  }

  KOKKOS_INLINE_FUNCTION
  void join(volatile T & a, const volatile T & b) const {
    a = R::combine(a, b);
  }

  KOKKOS_INLINE_FUNCTION
  void init(T & v) const {
    v = detail::identity_traits<R>::template value<T>;
  }

  // Also useful to read the value!
  KOKKOS_INLINE_FUNCTION
  T & reference() const {
    return t;
  }

  KOKKOS_INLINE_FUNCTION
  result_view_type view() const {
    return &t;
  }

  wrap & kokkos() {
    return *this;
  }

private:
  mutable T t;
};

// Kokkos's built-in reducers are just as effective as ours for generic
// types, although we can't provide Kokkos::reduction_identity in terms of
// our interface in C++17 because it has no extra template parameter via
// which to apply SFINAE.
template<class>
struct reducer; // undefined
template<>
struct reducer<fold::min> {
  template<class T>
  using type = Kokkos::Min<T>;
};
template<>
struct reducer<fold::max> {
  template<class T>
  using type = Kokkos::Max<T>;
};
template<>
struct reducer<fold::sum> {
  template<class T>
  using type = Kokkos::Sum<T>;
};
template<>
struct reducer<fold::product> {
  template<class T>
  using type = Kokkos::Prod<T>;
};

template<class R, class T>
struct wrap<R, T, decltype(void(reducer<R>()))> {
private:
  T t;
  typename reducer<R>::template type<T> native{t};

public:
  const T & reference() const {
    return t;
  }
  auto & kokkos() {
    return native;
  }
};
} // namespace kok

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
template<class R, class T, typename ITERATOR, typename LAMBDA>
T
parallel_reduce(ITERATOR iterator, LAMBDA lambda, std::string name = "") {

  using value_type = T;

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

  kok::wrap<R, T> result;
  Kokkos::parallel_reduce(
    name, iterator.size(), functor{iterator, lambda}, result.kokkos());
  return result.reference();

} // parallel_reduce

/*!
  The reduce_all type provides a pretty interface for invoking data-parallel
  reductions.
 */
template<class ITERATOR, class R, class T>
struct reduceall_t {

  reduceall_t(ITERATOR iterator, std::string name = "")
    : iterator_(iterator), name_(name) {}

  using value_type = T;

  template<typename LAMBDA>
  T operator->*(LAMBDA lambda) && {
    return parallel_reduce<R, T>(
      std::move(iterator_), std::move(lambda), name_);
  } // operator+

private:
  ITERATOR iterator_;
  std::string name_;

}; // forall_t

template<class R, class T, class I>
reduceall_t<I, R, T>
make_reduce(I i, std::string n) {
  return {std::move(i), n};
}

#define reduceall(it, tmp, iterator, R, T, name)                               \
  ::flecsi::exec::make_reduce<R, T>(iterator, name)                            \
      ->*KOKKOS_LAMBDA(auto it, T & tmp)

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

} // namespace exec
} // namespace flecsi
