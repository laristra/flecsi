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

#ifndef flecsi_utils_hash_h
#define flecsi_utils_hash_h

//!
//! \file hash.h
//! \date Initial file creation: Oct 15, 2015
//!

#include <utility>

namespace flecsi {
namespace utils {

//!
//!
//!
template<
  typename T,
  typename U
>
constexpr
T
hash__(
  U && str,
  const T h,
  const std::size_t i,
  const std::size_t n
)
{
  // An unstated assumption appears to be that n is the length of str, which is
  // a string type, and that i <= n. Otherwise, we're going to have problems.
  return i == n ?  h :
    hash__(str, h ^ static_cast<T>(std::forward<U>(str)[i]) << 8*(i%8),
      i + 1, n);
} // hash__

//!
//!
//!
template<
  typename T,
  typename U
>
constexpr
T
hash(
  U && str,
  const std::size_t n
)
{
  return hash__<T>(str, 0, 0, n);
} // hash

} // namespace utils
} // namespace flecsi

#endif // flecsi_utils_hash_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
