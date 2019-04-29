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

#include <cstdint>
#include <functional>
#include <limits>
#include <sstream>
#include <typeinfo>

#include <flecsi/utils/export_definitions.h>
#include <flecsi/utils/id.h>
#include <flecsi/utils/offset.h>

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

namespace flecsi {
namespace utils {

//----------------------------------------------------------------------------//
// Typeification.
//----------------------------------------------------------------------------//

template<typename T, T M>
struct typeify {
  using TYPE = T;
  static constexpr T value = M;
};

template<typename T, T M>
constexpr T typeify<T, M>::value;

//----------------------------------------------------------------------------//
// Entity id type.
//----------------------------------------------------------------------------//

using id_t =
  id_<FLECSI_ID_PBITS, FLECSI_ID_EBITS, FLECSI_ID_FBITS, FLECSI_ID_GBITS>;

using offset_t = offset_u<16>;

//----------------------------------------------------------------------------//
// Index type
//----------------------------------------------------------------------------//

#ifndef FLECSI_COUNTER_TYPE
#define FLECSI_COUNTER_TYPE int32_t
#endif

using counter_t = FLECSI_COUNTER_TYPE;

//----------------------------------------------------------------------------//
// Square
//----------------------------------------------------------------------------//

//! P.O.D.
template<typename T>
inline T
square(const T & a) {
  return a * a;
}

//----------------------------------------------------------------------------//
// Unique Identifier Utilities
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
// This value is used by the Legion runtime backend to automatically
// assign task and field ids. The current maximum value that is allowed
// in legion_config.h is 1<<20.
//
// We are reserving 4096 places for internal use.
//----------------------------------------------------------------------------//

#if !defined(FLECSI_GENERATED_ID_MAX)
// 1044480 = (1<<20) - 4096
#define FLECSI_GENERATED_ID_MAX 1044480
#endif

//! Generate unique ids
template<typename T,
  std::size_t MAXIMUM = (std::numeric_limits<std::size_t>::max)()>
struct unique_id_t {
  static unique_id_t & instance() {
    static unique_id_t u;
    return u;
  } // instance

  auto next() {
    assert(id_ + 1 <= MAXIMUM && "id exceeds maximum value");
    return ++id_;
  } // next

private:
  unique_id_t() : id_(0) {}
  unique_id_t(const unique_id_t &) {}
  ~unique_id_t() {}

  std::size_t id_;
};

//! Create a unique name from the type, address, and unique id
template<typename T>
std::string
unique_name(const T * const t) {
  const void * const address = static_cast<const void *>(t);
  const std::size_t id = unique_id_t<T>::instance().next();
  std::stringstream ss;
  ss << typeid(T).name() << "-" << address << "-" << id;
  return ss.str();
} // unique_name

} // namespace utils
} // namespace flecsi
