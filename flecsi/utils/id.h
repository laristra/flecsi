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

#if defined(_MSC_VER)
#include "uint128.h"
#endif

#include <cassert>
#include <climits>
#include <cstdint>
#include <iostream>

#include <flecsi/utils/target.h>
namespace flecsi {
namespace utils {

using local_id_t = __uint128_t;

template<std::size_t PBITS,
  std::size_t EBITS,
  std::size_t FBITS,
  std::size_t GBITS>
class id_
{
public:
  static_assert(PBITS + EBITS + FBITS + GBITS + 4 == 128,
    "invalid id bit configuration");

  static_assert(sizeof(std::size_t) * CHAR_BIT >= 64,
    "need std::size_t >= 64 bit");

  // Constructors and make()...

  id_() = default;
  id_(id_ &&) = default;
  id_(const id_ &) = default;

  explicit id_(const std::size_t local_id)
    : dimension_(0), domain_(0), partition_(0), entity_(local_id), flags_(0),
      global_(0) {}

  template<std::size_t D, std::size_t M>
  static id_ make(const std::size_t local_id,
    const std::size_t partition_id = 0,
    const std::size_t global = 0,
    const std::size_t flags = 0) {
    id_ global_id;
    global_id.dimension_ = D;
    global_id.domain_ = M;
    global_id.partition_ = partition_id;
    global_id.entity_ = local_id;
    global_id.flags_ = flags;
    global_id.global_ = global;

    return global_id;
  }

  // Assignment...

  id_ & operator=(id_ &&) = default;
  id_ & operator=(const id_ & id) = default;

  // Setters...

  void set_partition(const std::size_t partition) {
    partition_ = partition;
  }

  void set_flags(const std::size_t flags) {
    assert(flags < (1 << FBITS) && "flag bits exceeded");
    flags_ = flags;
  }

  void set_global(const std::size_t global) {
    global_ = global;
  }

  void set_local(const std::size_t local) {
    entity_ = local;
  }

  // Getters...

  FLECSI_INLINE_TARGET
  std::size_t dimension() const {
    return dimension_;
  }
  FLECSI_INLINE_TARGET
  std::size_t domain() const {
    return domain_;
  }
  FLECSI_INLINE_TARGET
  std::size_t partition() const {
    return partition_;
  }
  FLECSI_INLINE_TARGET
  std::size_t entity() const {
    return entity_;
  }

  std::size_t flags() const {
    return flags_;
  }

  std::size_t global() const {
    return global_;
  }

  // index_space_index(): same as entity getter
  FLECSI_INLINE_TARGET
  std::size_t index_space_index() const {
    return entity_;
  }

  // Construct "local ID"...
  // [entity][partition][domain][dimension]
  //
  // As id_ is used in FleCSI, local_id() takes this id_, which is:
  //    dd mm pppppppppppppppppppp eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee
  //    ffff gggggggggggggggggggggggggggggggggggggggggggggggggggggggggggg
  // and produces:
  //    eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee pppppppppppppppppppp mm dd
  // as the return value. In short, it tosses flags and global, and reverses
  // the order of dimension, domain, partition, and entity.
  FLECSI_INLINE_TARGET
  local_id_t local_id() const {
    local_id_t r = dimension_;
    r |= local_id_t(domain_) << 2;
    r |= local_id_t(partition_) << 4;
    r |= local_id_t(entity_) << (4 + PBITS);
    return r;
  }

  // Comparison (<, ==, !=)...

  FLECSI_INLINE_TARGET
  bool operator<(const id_ & id) const {
    return local_id() < id.local_id();
  }

  FLECSI_INLINE_TARGET
  bool operator==(const id_ & id) const {
    return local_id() == id.local_id();
  }

  FLECSI_INLINE_TARGET
  bool operator!=(const id_ & id) const {
    return !(*this == id);
  }

private:
  // As used elsewhere in FleCSI, this class amounts to:
  //    [dimension:2][domain:2][partition:20][entity:40][flags:4][global:60]
  // Or:
  //    dd mm pppppppppppppppppppp eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee
  //    ffff gggggggggggggggggggggggggggggggggggggggggggggggggggggggggggg
  // where "m" is domain.

  std::size_t dimension_ : 2;
  std::size_t domain_ : 2;
  std::size_t partition_ : PBITS;
  std::size_t entity_ : EBITS;
  std::size_t flags_ : FBITS;
  std::size_t global_ : GBITS;
}; // id_

} // namespace utils
} // namespace flecsi

// Defining operator<< out-of-namespace prevents an overload ambiguity problem
// that the unit-test code uncovered when the definition was in flecsi::utils.
inline std::ostream &
operator<<(std::ostream & ostr, const flecsi::utils::local_id_t x) {
  return ostr << uint64_t(x >> 64) << ":" << uint64_t(x);
}
