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

#include <cstdlib>
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
#define FLECSI_ID_EBITS 36
#endif

#ifndef FLECSI_ID_FBITS
#define FLECSI_ID_FBITS 4
#endif

#ifndef FLECSI_ID_RBITS
#define FLECSI_ID_RBITS 32
#endif

namespace flecsi
{

using id_t = 
  id_<FLECSI_ID_PBITS, FLECSI_ID_EBITS, FLECSI_ID_FBITS, FLECSI_ID_RBITS>;

//! P.O.D.
template <typename T>
T square(const T & a)
{
  return a * a;
}

/*----------------------------------------------------------------------------*
 * Simple Random Number Utilities
 *----------------------------------------------------------------------------*/

template<size_t start, size_t end>
size_t int_rand() {
  static_assert(end>start, "start parameter must be less than end");
  static_assert(end<RAND_MAX, "end parameter out-of-range");
  return start + size_t((end-start)*double(random())/RAND_MAX);
} // int_rand

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
}; // unique_name

} // namespace flecsi

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
