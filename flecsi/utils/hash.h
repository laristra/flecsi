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

///
/// \file 
/// \date Initial file creation: Oct 15, 2015
///

#include <limits>
#include <utility>

namespace flecsi {
namespace utils {

///
///
///
template<
  typename T,
  typename U
>
constexpr
T
hash__(
  U && str,
  T h,
  std::size_t i,
  std::size_t n
)
{
  return i == n ?  h :
    hash__(str, h ^ static_cast<T>(std::forward<U>(str)[i]) << 8*(i%8),
      i + 1, n); 
} // hash__

///
///
///
template<
  typename T,
  typename U
>
constexpr
T
hash(
  U && str,
  std::size_t n
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
