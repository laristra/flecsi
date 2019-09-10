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

#include <string>
#include <typeinfo> // typeid()

namespace flecsi {
namespace utils {

/*!
  Return the demangled name.

  @param name The string to demangle.

  @ingroup utils
 */

std::string demangle(const char * const name);

/*!
  Return the demangled name of the type T.

  @tparam T The type.

  @ingroup utils
 */

template<class T>
inline std::string
type() {
  return demangle(typeid(T).name());
} // type

/*!
  Return the demangled name of the type identified by type_info.

  @param std::type_info The type.

  @ingroup utils
 */

inline std::string
type(const std::type_info & type_info) {
  return demangle(type_info.name());
} // type

/// Dummy class template.
/// \tparam S a reference to a function or variable
template<auto & S>
struct Symbol {};
/// Return the name of the template argument.
/// \tparam a reference to a function or variable
/// \return demangled name
template<auto & S>
std::string
symbol() {
  constexpr int PFX = sizeof("flecsi::utils::Symbol<") - 1;
  const auto s = type<Symbol<S>>();
  return s.substr(PFX, s.size() - 1 - PFX);
}

} // namespace utils
} // namespace flecsi
