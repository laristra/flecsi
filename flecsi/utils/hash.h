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

//----------------------------------------------------------------------------//
//! @file hash.h
//! @date Initial file creation: Oct 15, 2015
//----------------------------------------------------------------------------//

#include <cstddef>
#include <utility>

namespace flecsi {
namespace utils {

//----------------------------------------------------------------------------//
// Simple hash function interface.
//----------------------------------------------------------------------------//

namespace hash {

//----------------------------------------------------------------------------//
//! Hashing function for client registration.
//!
//! @tparam NAMESPACE The namespace key.
//! @tparam NAME      The name key.
//----------------------------------------------------------------------------//

template<
  size_t MASK,
  size_t SHIFT
>
inline
constexpr size_t
bit_range(
  size_t key
)
{
  return(key & MASK) >> SHIFT;
} // bit_range

//----------------------------------------------------------------------------//
//! Create a hash key suitable for registering a field with the low-level
//! field registry.
//!
//! @tparam NAMESPACE A namespace identifier.
//! @tparam NAME      A name identifier.
//!
//! @ingroup utils
//----------------------------------------------------------------------------//

template<
  size_t NAMESPACE,
  size_t NAME
>
inline
size_t
field_hash()
{
  return NAMESPACE ^ NAME;
} // field_hash__

//----------------------------------------------------------------------------//
//! Create a hash key suitable for registering client data with the low-level
//! field registry.
//!
//! @tparam NAMESPACE A namespace identifier.
//! @tparam NAME      A name identifier.
//! @tparam INDEX     The associated index space.
//! @tparam DOMAIN    The domain.
//! @tparam DIMENSION The topological dimension.
//!
//! @ingroup utils
//----------------------------------------------------------------------------//

template<
  size_t NAMESPACE,
  size_t NAME,
  size_t INDEX,
  size_t DOMAIN_, // FIXME: Somewhere DOMAIN is being defined
  size_t DIMENSION
>
inline
size_t
client_data_hash()
{
  return ((NAMESPACE ^ NAME) << 12) ^
    (INDEX << 4) ^
    (DOMAIN_ << 2) ^
    DIMENSION;
} // client_data_hash

//----------------------------------------------------------------------------//
//! Recover the index space from a key to client data.
//!
//! @param key The client data key.
//!
//! @ingroup utils
//----------------------------------------------------------------------------//

inline
size_t
client_data_index(
  size_t key
)
{
  return bit_range<0xff0, 4>(key);
} // client_data_index

//----------------------------------------------------------------------------//
//! Recover the domain from a key to client data.
//!
//! @param key The client data key.
//!
//! @ingroup utils
//----------------------------------------------------------------------------//

inline
size_t
client_data_domain(
  size_t key
)
{
  return bit_range<0x0c, 2>(key);
} // client_data_domain

//----------------------------------------------------------------------------//
//! Recover the dimension from a key to client data.
//!
//! @param key The client data key.
//!
//! @ingroup utils
//----------------------------------------------------------------------------//

inline
size_t
client_data_dimension(
  size_t key
)
{
  return bit_range<0x03, 0>(key);
} // client_data_dimension

} // namespace hash

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//

template<
  typename T,
  typename U
>
constexpr
T
string_hash__(
  U && str,
  const T h,
  const std::size_t i,
  const std::size_t n
)
{
  // An unstated assumption appears to be that n is the length of str, which is
  // a string type, and that i <= n. Otherwise, we're going to have problems.
  return i == n ?  h :
    string_hash__(str, h ^ static_cast<T>(std::forward<U>(str)[i]) << 8*(i%8),
      i + 1, n);
} // string_hash__

template<
  typename T,
  typename U
>
constexpr
T
string_hash(
  U && str,
  const std::size_t n
)
{
  return string_hash__<T>(str, 0, 0, n);
} // string_hash

} // namespace utils
} // namespace flecsi

#endif // flecsi_utils_hash_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
