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

namespace flecsi {
namespace utils {

template<size_t COUNT_BITS>
class offset__ {
public:
  static_assert(COUNT_BITS <= 32, "COUNT_BITS max exceeded");

  static constexpr uint64_t count_mask = (1ul << COUNT_BITS) - 1;
  static constexpr uint32_t count_max = 1ul << COUNT_BITS;

  offset__() : o_(0ul) {}

  offset__(uint64_t start, uint32_t count) : o_(start << COUNT_BITS | count) {}

  offset__(const offset__ & prev, uint32_t count)
      : o_(prev.end() << COUNT_BITS | count) {}

  uint64_t start() const {
    return o_ >> COUNT_BITS;
  }

  uint32_t count() const {
    return o_ & count_mask;
  }

  uint64_t end() const {
    return start() + count();
  }

  void set_count(uint32_t count) {
    assert(count < count_max);
    o_ = (o_ & ~count_mask) | count;
  }

  void set_offset(uint64_t offset) {
    o_ = (o_ & count_mask) | (offset << COUNT_BITS);
  }

  std::pair<size_t, size_t> range() const {
    uint64_t s = start();
    return {s, s + count()};
  }

private:
  uint64_t o_;
};

} // namespace utils
} // namespace flecsi
