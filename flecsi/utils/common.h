/*~--------------------------------------------------------------------------~*
 *  @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
 * /@@/////  /@@          @@////@@ @@////// /@@
 * /@@       /@@  @@@@@  @@    // /@@       /@@
 * /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
 * /@@////   /@@/@@@@@@@/@@       ////////@@/@@
 * /@@       /@@/@@//// //@@    @@       /@@/@@
 * /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
 * //       ///  //////   //////  ////////  //
 *
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_common_h
#define flecsi_common_h

#include <cstdint>
#include <sstream>
#include <typeinfo>

#include "flecsi/utils/id.h"

/*!
 * \file common.h
 * \authors bergen
 * \date Initial file creation: Sep 23, 2015
 */

#ifndef FLECSI_ID_PBITS
#define FLECSI_ID_PBITS 20
#endif

#ifndef FLECSI_ID_EBITS
#define FLECSI_ID_EBITS 40
#endif

#ifndef FLECSI_ID_FBITS
#define FLECSI_ID_FBITS 4
#endif

#ifndef FLECSI_ID_GBITS
#define FLECSI_ID_GBITS 60
#endif

namespace flecsi
{

using id_t = 
  id_<FLECSI_ID_PBITS, FLECSI_ID_EBITS, FLECSI_ID_FBITS, FLECSI_ID_GBITS>;

//! P.O.D.
template <typename T>
T square(const T & a)
{
  return a * a;
}

/*----------------------------------------------------------------------------*
 * C++ demangler
 *----------------------------------------------------------------------------*/

std::string demangle(const char* name);

template <class T>
std::string type() {
  return demangle(typeid(T).name());
} // type

/*----------------------------------------------------------------------------*
 * Unique Identifier Utilities
 *----------------------------------------------------------------------------*/

//! Generate unique ids
template<typename T>
struct unique_id_t {
  static unique_id_t & instance() {
    static unique_id_t u;
    return u;
  } // instance

  auto next() {
    return ++id_;
  } // next

private:

  unique_id_t() : id_(0) {}
  unique_id_t(const unique_id_t &) {}
  ~unique_id_t() {}

  size_t id_;
};

//! Create a unique name from the type, address, and unique id
template<typename T>
std::string unique_name(const T * t) {
  const void * address = static_cast<const void *>(t);
  size_t id = unique_id_t<T>::instance().next();
  std::stringstream ss;
  ss << typeid(T).name() << "-" << address << "-" << id;
  return ss.str();
} // unique_name

} // namespace flecsi

/*----------------------------------------------------------------------------*
 * Function Traits
 *----------------------------------------------------------------------------*/

template<typename T>
struct function_traits__ {};

template<typename R, typename ... As>
struct function_traits__<R(As ...)> {
  using return_type = R;
  using arg_type = std::tuple<As ...>;
}; // struct function_traits__

/*----------------------------------------------------------------------------*
 * Preprocessor String Utilities
 *----------------------------------------------------------------------------*/

#define _UTIL_STRINGIFY(s) #s
#define EXPAND_AND_STRINGIFY(s) _UTIL_STRINGIFY(s)

#endif // flecsi_common_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
