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

//
// Attribution:
// This logic is an adaptation of Paul Fultz's response on Stack Overflow
// (41453/how-can-i-add-reflection-to-a-c-application) that is updated to
// use standard library features that were not available at the time the
// code was written, and to fit the needs of FleCSI.
//

#include <type_traits>

#include <flecsi-config.h>

#if !defined(FLECSI_ENABLE_BOOST_PREPROCESSOR)
#error FLECSI_ENABLE_BOOST_PREPROCESSOR not defined! \
    This file depends on Boost.Preprocessor!
#endif

#include <boost/preprocessor.hpp>

#include <flecsi/utils/utility.h>

/*----------------------------------------------------------------------------*
 * Macro definitions.
 *----------------------------------------------------------------------------*/

#define flecsi_internal_remove(...) __VA_ARGS__
#define flecsi_internal_eat(...)

#define flecsi_internal_typeof(x) flecsi_internal_detail_typeof(flecsi_internal_detail_typeof_probe x, )
#define flecsi_internal_detail_typeof(...) flecsi_internal_detail_typeof_head(__VA_ARGS__)
#define flecsi_internal_detail_typeof_head(x, ...) flecsi_internal_remove x
#define flecsi_internal_detail_typeof_probe(...) (__VA_ARGS__),

//
// Strip the type from the reflection declaration, i.e., if 'x' is:
//
// double myvar
//
// then flecsi_internal_strip(x) is:
//
// myvar
//
#define flecsi_internal_strip(x) flecsi_internal_eat x

//
// Snip the reflection declaration (show without paranthesis), i.e., if 'x' is:
//
// (double myvar)
//
// then flecsi_internal_snip(x) is:
//
// double myvar
//
#define flecsi_internal_snip(x) flecsi_internal_remove x

///
///
///
#define declare_reflected(...)                                                 \
                                                                               \
  static const std::size_t num_reflected_ =                                    \
    BOOST_PP_VARIADIC_SIZE(__VA_ARGS__);                                       \
                                                                               \
  friend struct reflection;                                                    \
                                                                               \
  /* Unspecialized type declaration. */                                        \
  template<std::size_t N, typename S>                                          \
  struct reflection_variable_u {};                                             \
                                                                               \
  /* Each invocation of this creates an explicit specialization. */            \
  BOOST_PP_SEQ_FOR_EACH_I(                                                     \
    reflection_variable, data, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))

//
//
//
#define reflection_variable(r, data, i, t)                                     \
                                                                               \
  /* This line adds the member data. */                                        \
  flecsi_internal_snip(t);                                                                   \
                                                                               \
  /* Interface for each data member. */                                        \
  template<typename S>                                                         \
  struct reflection_variable_u<i, S> {                                         \
    S & self_;                                                                 \
                                                                               \
    /* Constructor */                                                          \
    reflection_variable_u(S & self) : self_(self) {}                           \
                                                                               \
    /* Return a const reference to the variable instance. */                   \
    typename std::add_const<flecsi_internal_typeof(t)>::type & get() const {                 \
      return self_.flecsi_internal_strip(t);                                                 \
    }                                                                          \
                                                                               \
  }; /* struct reflection_variable_u<i,S> */

namespace flecsi {
namespace utils {

struct reflection {
  ///
  /// Get the number of reflection vairables.
  ///
  template<typename T>
  struct num_variables {
    static constexpr std::size_t value = T::num_reflected_;
  }; // struct num_variables

  ///
  /// Get the reflection variable at index N.
  ///
  template<std::size_t N, typename T>
  static typename T::template reflection_variable_u<N, T> variable(T & t) {
    return typename T::template reflection_variable_u<N, T>(t);
  } // get_variable
}; // reflection

} // namespace utils
} // namespace flecsi
