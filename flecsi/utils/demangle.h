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

#include <flecsi/utils/export_definitions.h>

/*! @file */

namespace flecsi {
namespace utils {

/*!
  Return the demangled name.

  @param name The string to demangle.

  @ingroup utils
 */

FLECSI_EXPORT std::string demangle(const char * const name);

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

} // namespace utils
} // namespace flecsi
