/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

//
// Attribution:
// This logic is an adaptation of Paul Fultz's response on Stack Overflow
// (41453/how-can-i-add-reflection-to-a-c-application) that is updated to
// use standard library features that were not available at the time the
// code was written, and to fit the needs of FleCSI.
//

#ifndef flecsi_utils_reflection_h
#define flecsi_utils_reflection_h

///
/// \file
/// \date Initial file creation: Jan 10, 2017
///

#include <type_traits>

#include "flecsi.h"

#if !defined(ENABLE_BOOST_PREPROCESSOR)
  #error ENABLE_BOOST_PREPROCESSOR not defined! \
    This file depends on Boost.Preprocessor!
#endif

#include <boost/preprocessor.hpp>

#include "flecsi/utils/utility.h"

/*----------------------------------------------------------------------------*
 * Macro definitions.
 *----------------------------------------------------------------------------*/

#define __remove(...) __VA_ARGS__
#define __eat(...)

#define __typeof(x) __detail_typeof(__detail_typeof_probe x,)
#define __detail_typeof(...) __detail_typeof_head(__VA_ARGS__)
#define __detail_typeof_head(x, ...) __remove x
#define __detail_typeof_probe(...) (__VA_ARGS__),

//
// Strip the type from the reflection declaration, i.e., if 'x' is:
//
// double myvar
//
// then __strip(x) is:
//
// myvar
//
#define __strip(x) __eat x

//
// Snip the reflection declaration (show without paranthesis), i.e., if 'x' is:
//
// (double myvar)
//
// then __snip(x) is:
//
// double myvar
//
#define __snip(x) __remove x

///
///
///
#define declare_reflected(...)                                                 \
                                                                               \
  static const size_t num_reflected_ = BOOST_PP_VARIADIC_SIZE(__VA_ARGS__);    \
                                                                               \
  friend struct reflection;                                                    \
                                                                               \
  /* Unspecialized type declaration. */                                        \
  template<                                                                    \
    size_t N,                                                                  \
    typename S                                                                 \
  >                                                                            \
  struct reflection_variable__ {};                                             \
                                                                               \
  /* Each invocation of this creates an explicit specialization. */            \
  BOOST_PP_SEQ_FOR_EACH_I(reflection_variable, data,                           \
    BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))

//
//
//
#define reflection_variable(r, data, i, t)                                     \
                                                                               \
  /* This line adds the member data. */                                        \
  __snip(t);                                                                   \
                                                                               \
  /* Interface for each data member. */                                        \
  template<                                                                    \
    typename S                                                                 \
  >                                                                            \
  struct reflection_variable__<i,S>                                            \
  {                                                                            \
    S & self_;                                                                 \
                                                                               \
    /* Constructor */                                                          \
    reflection_variable__(                                                     \
      S & self                                                                 \
    )                                                                          \
    :                                                                          \
      self_(self)                                                              \
    {}                                                                         \
                                                                               \
    /* Return a const reference to the variable instance. */                   \
    typename std::add_const<__typeof(t)>::type &                               \
    get()                                                                      \
    const                                                                      \
    {                                                                          \
      return self_.__strip(t);                                                 \
    }                                                                          \
                                                                               \
  }; /* struct reflection_variable__<i,S> */

namespace flecsi {
namespace utils {

struct reflection
{
  ///
  /// Get the number of reflection vairables.
  ///
  template<
    typename T
  >
  struct num_variables
  {
    static constexpr size_t value = T::num_reflected_;
  }; // struct num_variables

  ///
  /// Get the reflection variable at index N.
  ///
  template<
    size_t N,
    typename T
  >
  static
  typename T::template reflection_variable__<N,T>
  variable(
    T & t
  )
  {
    return typename T::template reflection_variable__<N,T>(t);
  } // get_variable
}; // reflection

} // namespace utils
} // namespace flecsi

#endif // flecsi_utils_reflection_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
