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

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly
#else
#include <flecsi/utils/debruijn.h>
#endif

#include <bitset>

namespace flecsi {


/*!
  Bitmasks for task types.

  \note This enumeration is not scoped so that users can do things
        like:
        \code
        task_type_t l(inner | idempotent);
        \endcode
 */

enum task_execution_type_mask_t : size_t {
  leaf = 1 << 0,
  inner = 1 << 1,
  idempotent = 1 << 2
}; // enum task_type_mask_t

#if 0
enum execution_type_t : size_t {
  leaf,
  inner,
  ldempotent
}; 
#endif

/*!
  Enumeration for launch types.

  \note This enumeration difines 2 different lounch types for the task:
        single and index
 */
enum launch_type_t : size_t {
  single,
  index 
}; // enum launch_type_t



namespace execution {

// This will be used by the task_hash_t type to create hash keys for
// task registration. If you add more task_type flags below, you will need
// to increase the task_type_bits accordingly, i.e., task_type_bits must
// be greater than or equal to the number of bits in the bitset for
// task_type_t below.

constexpr size_t task_execution_type_bits = 3;

/*!
  Use a std::bitset to store task_type information.

  @note This will most likely use 3 bytes of data for efficiency.
 */

using task_execution_type_t = std::bitset<task_execution_type_bits>;

enum execution_type_t : size_t {
  leaf,
  inner,
  idempotent
};

/*!
    launch_domain type is used in flecsi_execute_task to specify a
    launch type of the task (single or index) and # of index points 
    for index launch
 */

struct launch_domain_t
{
  launch_type_t launch_type_;
  size_t domain_size_=1;
};

#if 1
/*!
  Convert a processor mask to a processor type.
 */

inline execution_type_t
mask_to_type(execution_type_t m) {
  return static_cast<execution_type_t>(flecsi::utils::debruijn32_t::index(m));
} // mask_to_type

/*!
  Macro to create repetitive interfaces.
 */
#endif

#define test_boolean_interface(name)                                           \
  inline bool task_##name(const task_execution_type_t & l) {                              \
    return l.test(static_cast<size_t>(execution_type_t::name));                   \
  }

// clang-format off
test_boolean_interface(leaf)
test_boolean_interface(inner)
test_boolean_interface(idempotent)
// clang-format on

#undef test_boolean_interface

} // namespace execution
} // namespace flecsi
