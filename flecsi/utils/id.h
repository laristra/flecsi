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

#include <cassert>
#include <climits>
#include <cstdint>
#include <iostream>

namespace flecsi {
namespace utils {

// flecsi::utils::local_id_t
using local_id_t = std::uint64_t;

// flecsi::utils::id_
template<std::size_t PBITS, std::size_t EBITS>
class id_
{
public:
  static_assert(PBITS + EBITS + 4 == sizeof(local_id_t) * CHAR_BIT,
    "invalid id bit configuration");

  // It's unlikely that these would fail, but, for the sake of the partition
  // and entity getters, we might as well explicitly enforce them...
  static_assert(sizeof(std::size_t) * CHAR_BIT >= PBITS,
    "need std::size_t >= PBITS bitS");

  static_assert(sizeof(std::size_t) * CHAR_BIT >= EBITS,
    "need std::size_t >= EBITS bitS");

  // ------------------------
  // Constructors and make()
  // ------------------------

  id_() = default;
  id_(id_ &&) = default;
  id_(const id_ &) = default;

  explicit id_(const std::size_t local_id)
    : dimension_(0), domain_(0), partition_(0), entity_(local_id) {}

  template<std::size_t D, std::size_t M>
  static id_ make(const std::size_t local_id, const std::size_t partition_id) {
    id_ global_id;
    global_id.dimension_ = D;
    global_id.domain_ = M;
    global_id.partition_ = partition_id;
    global_id.entity_ = local_id;
    return global_id;
  }

  // ------------------------
  // Assignment
  // ------------------------

  id_ & operator=(id_ &&) = default;
  id_ & operator=(const id_ &) = default;

  // ------------------------
  // Getters
  // ------------------------

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

  // index_space_index(): same as entity getter
  std::size_t index_space_index() const {
    return entity();
  }

  // ------------------------
  // Construct "local ID"
  // ------------------------

  // Takes the current object, e.g the following with PBITS=20 and EBITS=40:
  //    dd mm pppppppppppppppppppp eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee
  // and produces:
  //    eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee pppppppppppppppppppp mm dd
  // as the return value.

  local_id_t local_id() const {
    local_id_t r = dimension_;
    r |= local_id_t(domain_) << 2;
    r |= local_id_t(partition_) << 4;
    r |= local_id_t(entity_) << (4 + PBITS);
    return r;
  }

  // ------------------------
  // Comparison (<, ==, !=)
  // ------------------------

  bool operator<(const id_ & id) const {
    return local_id() < id.local_id();
  }
  bool operator==(const id_ & id) const {
    return local_id() == id.local_id();
  }
  bool operator!=(const id_ & id) const {
    return !(*this == id);
  }

private:
  // As used elsewhere in FleCSI, this class amounts to:
  //    [dimension:2][domain:2][partition:20][entity:40]
  // Or:
  //    dd mm pppppppppppppppppppp eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee
  // where "m" is domain.

  std::size_t dimension_ : 2;
  std::size_t domain_ : 2;
  std::size_t partition_ : PBITS;
  std::size_t entity_ : EBITS;
}; // class id_

} // namespace utils
} // namespace flecsi
