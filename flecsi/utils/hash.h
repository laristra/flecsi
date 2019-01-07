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

#include <cstddef>
#include <utility>

namespace flecsi {
namespace utils {
#if 0
////////////////////////////////////////////////////////////////////////////////
// String hash function for const_string_t type.
// Perhaps this should move into const_string.h
////////////////////////////////////////////////////////////////////////////////

template<typename T, typename U>
constexpr T
string_hash_u(U && str, const T h, const std::size_t i, const std::size_t n) {
  // An unstated assumption appears to be that n is the length of str, which is
  // a string type, and that i <= n. Otherwise, we're going to have problems.
  return i == n
    ? h
      : string_hash_u(
        str,
        h ^ static_cast<T>(std::forward<U>(str)[i]) << 8 * (i % 8),
        i + 1, n);
} // string_hash_u

template<typename T, typename U>
constexpr T
string_hash(U && str, const std::size_t n) {
  return string_hash_u<T>(str, 0, 0, n);
} // string_hash

#endif 

namespace hash {

////////////////////////////////////////////////////////////////////////////////
// Helper functions.
////////////////////////////////////////////////////////////////////////////////

constexpr size_t field_hash_version_bits = 3;
constexpr size_t field_max_versions = 1 << 3;

/*!
  Hashing function for client registration.

  @tparam NAMESPACE The namespace key.
  @tparam NAME      The name key.
 */

template<size_t MASK, size_t SHIFT>
inline constexpr size_t
bit_range(size_t key) {
  return (key >> SHIFT) & MASK;
} // bit_range

////////////////////////////////////////////////////////////////////////////////
// Client data hash interface.
////////////////////////////////////////////////////////////////////////////////

/*!
  Create a hash key suitable for registering a data client.

  @tparam NAMESPACE A namespace identifier.
  @tparam NAME      A name identifier.

  @ingroup utils
 */

template<size_t NAMESPACE, size_t NAME>
inline constexpr size_t
client_hash() {
  return NAMESPACE ^ NAME;
} // client_hash

////////////////////////////////////////////////////////////////////////////////
// Field data hash interface.
////////////////////////////////////////////////////////////////////////////////

/*!
  Create a hash key suitable for registering a field with the low-level
  field registry.

  \note This hash sets the most significant bit to '0' to be used as
        a boolean that distinguishes normal fields from internal
        data client fields.

  @tparam NAMESPACE A namespace identifier.
  @tparam NAME      A name identifier.

  @ingroup utils
 */

template<size_t NAMESPACE, size_t NAME>
inline constexpr size_t
field_hash(size_t version) {
  return ((NAMESPACE ^ NAME) << field_hash_version_bits | version) &
         ~(1ull << 63);
} // field_hash

inline size_t
field_hash(size_t nspace, size_t name, size_t version) {
  return ((nspace ^ name) << field_hash_version_bits | version) & ~(1ull << 63);
} // field_hash

inline constexpr size_t
field_hash_version(size_t key) {
  return bit_range<(1ul << field_hash_version_bits) - 1, 0>(key);
} // client_entity_index

/*!
  Check if the given key identifies an internal field.

  @param key The hash key.

  @ingroup utils
 */

bool inline is_internal(size_t key) {
  return key & (1ull << 63);
} // is_internal

////////////////////////////////////////////////////////////////////////////////
// Client entities hash interface.
////////////////////////////////////////////////////////////////////////////////

/*!
  Create a hash key suitable for registering client entity types with
  the low-level field registry.

  @tparam NAMESPACE A namespace identifier.
  @tparam NAME      A name identifier.
  @tparam INDEX     The associated index space.
  @tparam DOMAIN    The domain.
  @tparam DIMENSION The topological dimension.

  @ingroup utils
 */

template<size_t NAMESPACE,
  size_t NAME,
  size_t INDEX,
  size_t DOMAIN_, // FIXME: Somewhere DOMAIN is being defined
  size_t DIMENSION>
inline constexpr size_t
client_entity_hash() {
  return ((NAMESPACE ^ NAME) << 12) | (INDEX << 4) | (DOMAIN_ << 2) | DIMENSION;
} // client_entity_hash

/*!
  Recover the index space from a key to a client entity.

  @param key The client entity key.

  @ingroup utils
 */

inline constexpr size_t
client_entity_index(size_t key) {
  return bit_range<0xff, 4>(key);
} // client_entity_index

/*!
  Recover the domain from a key to a client entity.

  @param key The client entity key.

  @ingroup utils
 */

inline constexpr size_t
client_entity_domain(size_t key) {
  return bit_range<0x03, 2>(key);
} // client_entity_domain

/*!
  Recover the dimension from a key to a client entity.

  @param key The client entity key.

  @ingroup utils
 */

inline constexpr size_t
client_entity_dimension(size_t key) {
  return bit_range<0x03, 0>(key);
} // client_entity_dimension

////////////////////////////////////////////////////////////////////////////////
// Client adjacency hash interface.
////////////////////////////////////////////////////////////////////////////////

/*!
  Create a hash key suitable for registering client adjacency types with
  the low-level field registry.

  @tparam NAMESPACE      A namespace identifier.
  @tparam NAME           A name identifier.
  @tparam INDEX          The associated index space.
  @tparam FROM_DOMAIN    The from domain.
  @tparam TO_DOMAIN      The to domain.
  @tparam FROM_DIMENSION The topological from dimension.
  @tparam TO_DIMENSION   The topological to dimension.

  @ingroup utils
 */

template<size_t NAMESPACE,
  size_t NAME,
  size_t INDEX,
  size_t FROM_DOMAIN,
  size_t TO_DOMAIN,
  size_t FROM_DIMENSION,
  size_t TO_DIMENSION>
inline constexpr size_t
client_adjacency_hash() {
  return ((NAMESPACE ^ NAME) << 16) | (INDEX << 8) | (FROM_DOMAIN << 6) |
         (TO_DOMAIN << 4) | (FROM_DIMENSION << 2) | TO_DIMENSION;
} // client_adjacency_hash

////////////////////////////////////////////////////////////////////////////////
// Client index subspace hash interface.
////////////////////////////////////////////////////////////////////////////////

/*!
  Create a hash key suitable for registering client index subspaces with
  the low-level field registry.

  @tparam NAMESPACE      A namespace identifier.
  @tparam NAME           A name identifier.
  @tparam INDEX          The associated index space.
  @tparam INDEX_SUBSPACE The associated index subspace.

  @ingroup utils
 */

template<size_t NAMESPACE, size_t NAME, size_t INDEX, size_t INDEX_SUBSPACE>
inline constexpr size_t
client_index_subspace_hash() {
  return ((NAMESPACE ^ NAME) << 16) | (INDEX << 8) | INDEX_SUBSPACE;
} // client_adjacency_hash

/*!
  Recover the index space from a key to a client entity.

  @param key The client entity key.

  @ingroup utils
 */

inline constexpr size_t
client_adjacency_index(size_t key) {
  return bit_range<0xff, 8>(key);
} // client_adjacency_index

/*!
  Recover the domain from a key to a client entity.

  @param key The client entity key.

  @ingroup utils
 */

inline constexpr size_t
client_adjacency_from_domain(size_t key) {
  return bit_range<0x03, 6>(key);
} // client_adjacency_from_domain

/*!
  Recover the to domain from a key to a client entity.

  @param key The client entity key.

  @ingroup utils
 */

inline constexpr size_t
client_adjacency_to_domain(size_t key) {
  return bit_range<0x03, 4>(key);
} // client_adjacency_to_domain

/*!
  Recover the dimension from a key to a client entity.

  @param key The client entity key.

  @ingroup utils
 */

inline constexpr size_t
client_adjacency_from_dimension(size_t key) {
  return bit_range<0x03, 2>(key);
} // client_adjacency_from_dimension

/*!
  Recover the dimension from a key to a client entity.

  @param key The client entity key.

  @ingroup utils
 */

inline constexpr size_t
client_adjacency_to_dimension(size_t key) {
  return bit_range<0x03, 0>(key);
} // client_adjacency_to_dimension

////////////////////////////////////////////////////////////////////////////////
// Client internal field hash interface.
////////////////////////////////////////////////////////////////////////////////

/*!
  Create a hash key suitable for registering internal client field data with
  the low-level field registry.

  \note This hash sets the most significant bit to '1' to be used as
        a boolean that distinguishes data client fields from normal
        fields.

  @tparam LABEL       The internal label for the client field.
  @tparam NAMESPACE   The namespace of the data client instance.
  @tparam NAME        The name of the data client instance.
  @tparam INDEX_SPACE The index space id of the associated field.

  @ingroup utils
 */

template<
  size_t LABEL,
  size_t NAMESPACE,
  size_t NAME,
  size_t INDEX_SPACE
>
inline constexpr size_t
client_internal_field_hash() {
  return ((LABEL ^ (NAMESPACE ^ NAME)) << 8 | INDEX_SPACE) | (1ull << 63);
} // client_internal_field_hash

/*!
  Create a hash key suitable for registering internal client field data with
  the low-level field registry.

  \note This hash sets the most significant bit to '1' to be used as
        a boolean that distinguishes data client fields from normal
        fields.

  @param label       The internal label for the client field.
  @param nspace      The namespace of the data client instance.
  @param name        The name of the data client instance.
  @param index_space The index space id of the associated field.

  @ingroup utils
 */

inline size_t
client_internal_field_hash(
  size_t label,
  size_t nspace,
  size_t name,
  size_t index_space
)
{
  return ((label ^ (nspace ^ name)) << 8 | index_space) | (1ull << 63);
} // client_internal_field_hash

/*!
  Return the index space component of an internal client key.

  @param key The internal client key.

  @ingroup utils
 */

inline constexpr size_t
client_internal_field_index_space(size_t key) {
  return bit_range<0xff, 8>(key);
} // client_internal_field_index_space

////////////////////////////////////////////////////////////////////////////////
// Intermediate map hash interface.
////////////////////////////////////////////////////////////////////////////////

/*!
  Create a hash key suitable for mapping intermediate entity types.

  @tparam DIMENSION The entity dimension.
  @tparam DOMAIN    The entity domain.

  @ingroup utils
 */

template<size_t DIMENSION, size_t DOMAIN_>
inline constexpr size_t
intermediate_hash() {
  return (DIMENSION << 32) ^ DOMAIN_;
} // intermediate_hash

/*!
  Create a hash key suitable for mapping intermediate entity types.

  @tparam DIMENSION The entity dimension.
  @tparam DOMAIN    The entity domain.

  @ingroup utils
 */

inline size_t
intermediate_hash(size_t dimension, size_t domain) {
  return (dimension << 32) ^ domain;
} // intermediate_hash

////////////////////////////////////////////////////////////////////////////////
// Reduction hash interface.
////////////////////////////////////////////////////////////////////////////////

template<size_t OPERATOR_HASH, size_t DATA_HASH>
inline constexpr size_t
reduction_hash() {
  return (OPERATOR_HASH << 32) ^ DATA_HASH;
} // reduction_hash
} // namespace hash

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//

template<typename T, typename U>
inline constexpr T
string_hash(U &&str, const std::size_t n)
{
  if (n == 0)
    return 0;

  // String-to-integer hash function, based on prime numbers.
  // References:
  //    https://stackoverflow.com/questions/8317508/hash-function-for-a-string
  //    https://planetmath.org/goodhashtableprimes

  const T P = 3145739; // prime
  const T Q = 6291469; // prime, a bit less than 2x the first

  T h = 37; // prime
  for (std::size_t i = 0;  i < n;  ++i)
    h = (h * P) ^ (str[i] * Q);
  return h;
} // string_hash

//} // namespace hash
} // namespace utils
} // namespace flecsi
