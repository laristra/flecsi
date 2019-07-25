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

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly
#else
#include <flecsi/utils/debruijn.hh>
#endif

#include <bitset>

namespace flecsi {

/*!
  The task_attributes_mask_t type allows conversion from eunumeration
  to bit mask.
  
  @note This enumeration is not scoped so that users can create
        masks as in:
        @code
        task_attributes_mask_t m(inner | idempotent);
        @endcode
 */

enum task_attributes_mask_t : size_t {
  leaf =       0b00000001,
  inner =      0b00000010,
  idempotent = 0b00000100,
  loc =        0b00001000,
  toc =        0b00010000,
  mpi =        0b00100000
}; // task_attributes_mask_t

namespace execution {

/*!
  Enumeration of task types.
 */

enum task_type_t : size_t {
  leaf,
  inner,
  idempotent
}; // task_type_t

/*!
  Enumeration of processor types.
 */

enum task_processor_type_t : size_t {
  loc,
  toc,
  mpi
}; // task_processor_type_t

// Bits for representing task attributes
constexpr size_t task_attributes_bits = 8;
constexpr size_t task_type_bits = 3;

using task_attributes_bitset_t = std::bitset<task_attributes_bits>;

inline task_type_t
mask_to_task_type(size_t mask) {
  return static_cast<task_type_t>(
    flecsi::utils::debruijn32_t::index(mask));
} // mask_to_task_type

inline task_processor_type_t
mask_to_processor_type(size_t mask) {
  return static_cast<task_processor_type_t>(
    flecsi::utils::debruijn32_t::index(mask >> task_type_bits));
} // mask_to_processor_type

inline bool leaf_task(task_attributes_bitset_t const & bs) {
  return bs.test(static_cast<size_t>(task_type_t::leaf));
}

inline bool inner_task(task_attributes_bitset_t const & bs) {
  return bs.test(static_cast<size_t>(task_type_t::inner));
}

inline bool idempotent_task(task_attributes_bitset_t const & bs) {
  return bs.test(static_cast<size_t>(task_type_t::idempotent));
}

} // namespace execution
} // namespace flecsi
