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

  // FLAGS_UNMASK's "<< 59" would seem to require this... - martin
  static_assert(sizeof(std::size_t) * CHAR_BIT >= 64,
    "need std::size_t >= 64 bit");

  // FLAGS_UNMASK
  // 1000011111111111 1111111111111111 1111111111111111 1111111111111111
  static constexpr std::size_t FLAGS_UNMASK =
    ~(((std::size_t(1) << FBITS) - std::size_t(1)) << 59);

  id_() = default;
  id_(id_ &&) = default;
  id_(const id_ &) = default;

  explicit id_(const std::size_t local_id)
    : dimension_(0), domain_(0), partition_(0), entity_(local_id), flags_(0),
      global_(0) {}

  template<std::size_t D, std::size_t M>
  static id_ make(const std::size_t local_id,
    const std::size_t partition_id = 0,
    const std::size_t flags = 0,
    const std::size_t global = 0) {
    id_ global_id;
    global_id.dimension_ = D;
    global_id.domain_ = M;
    global_id.partition_ = partition_id;
    global_id.entity_ = local_id;
    global_id.global_ = global;
    global_id.flags_ = flags;

    return global_id;
  }

  /*
  // appears to be unused anywhere in FleCSI
  template<std::size_t M>
  static id_ make(const std::size_t dim,
    const std::size_t local_id,
    const std::size_t partition_id = 0,
    const std::size_t flags = 0,
    const std::size_t global = 0) {
    id_ global_id;
    global_id.dimension_ = dim;
    global_id.domain_ = M;
    global_id.partition_ = partition_id;
    global_id.entity_ = local_id;
    global_id.global_ = global;
    global_id.flags_ = flags;

    return global_id;
  }
  */

  /*
  // appears to be unused anywhere in FleCSI
  static id_ make(const std::size_t dim,
    const std::size_t local_id,
    const std::size_t partition_id = 0,
    const std::size_t flags = 0,
    const std::size_t global = 0,
    const std::size_t domain = 0) {
    id_ global_id;
    global_id.dimension_ = dim;
    global_id.domain_ = domain;
    global_id.partition_ = partition_id;
    global_id.entity_ = local_id;
    global_id.global_ = global;
    global_id.flags_ = flags;

    return global_id;
  }
  */

  // [entity][partition][domain][dimension]
  //
  // As id_ is used in FleCSI, local_id() takes this id_, which is:
  //    dd mm pppppppppppppppppppp eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee
  //    ffff gggggggggggggggggggggggggggggggggggggggggggggggggggggggggggg
  // and produces:
  //    eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee pppppppppppppppppppp mm dd
  // as the return value. In short, it tosses flags and global, and reverses
  // the order of dimension, domain, partition, and entity.
  local_id_t local_id() const {
    local_id_t r = dimension_;
    r |= local_id_t(domain_) << 2;
    r |= local_id_t(partition_) << 4;
    r |= local_id_t(entity_) << (4 + PBITS);
    return r;
  }

  /*
  // appears to be unused anywhere in FleCSI
  std::size_t global_id() const {
    constexpr std::size_t unmask = ~((std::size_t(1) << EBITS) - 1);
    return static_cast<std::size_t>((local_id() & unmask) | global_);
  }
  */

  void set_global(const std::size_t global) {
    global_ = global;
  }

  std::size_t global() const {
    return global_;
  }

  void set_partition(const std::size_t partition) {
    partition_ = partition;
  }

  id_ & operator=(id_ &&) = default;
  id_ & operator=(const id_ & id) = default;

  std::size_t dimension() const {
    return dimension_;
  }

  std::size_t domain() const {
    return domain_;
  }

  std::size_t partition() const {
    return partition_;
  }

  std::size_t entity() const {
    return entity_;
  }

  std::size_t index_space_index() const {
    return entity_;
  }

  std::size_t flags() const {
    return flags_;
  }

  void set_flags(const std::size_t flags) {
    assert(flags < (1 << FBITS) && "flag bits exceeded");
    flags_ = flags;
  }

  bool operator<(const id_ & id) const {
    return local_id() < id.local_id();
  }

  bool operator==(const id_ & id) const {
    // Each side of the below comparison amounts to:
    //    eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee pppppppppppppppppppp mm dd
    //    &
    //    1000011111111111111111111111111111111111 11111111111111111111 11 11
    // I haven't yet determined if/why that makes sense. -Martin

    return (local_id() & FLAGS_UNMASK) == (id.local_id() & FLAGS_UNMASK);
  }

  // As written, the following makes more sense than == does.
  // However, != != !(==), which is very strange, and possibly unintentional.
  bool operator!=(const id_ & id) const {
    return !(local_id() == id.local_id());
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
