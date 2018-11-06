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
#include <cstddef>
#include <cstdint>
#include <utility>

namespace flecsi {
namespace utils {

/**
 * @brief offset_u represents an offset range (a start index plus a count of
 * elements) in a single uint64_t. The lower COUNT_BITS bits are used for the
 * count and the remaining bits are used for the start index.
 *
 * @tparam COUNT_BITS Number of bits used for the count
 */
template<size_t COUNT_BITS>
class offset_u {
public:
  static_assert(COUNT_BITS <= 32, "COUNT_BITS max exceeded");

  /**
   * @brief Bitmask used to get the count bits, this is also the maximum value
   * for the count value.
   *
   * Since we ensure that COUNT_BITS <= 32, this will always fit into a
   * uint32_t.
   */
  static constexpr uint32_t count_mask = (1ul << COUNT_BITS) - 1;

  /**
   * @brief The maximum value of the start index value.
   */
  static constexpr uint64_t start_max = (1ul << (64 - COUNT_BITS)) - 1;

  offset_u() : o_(0ul) {}

  /**
   * @brief Construct a new offset range from a start index and a count.
   *
   * @param start Start index of the offset range
   * @param count The count (number) of elements
   */
  offset_u(uint64_t start, uint32_t count) : o_(start << COUNT_BITS | count) {
    assert(count <= count_mask);
    assert(start <= start_max);
  }

  /**
   * @brief Construct a new offset range from a previous offset range and a
   * count. The start index of this offset range is the end index of the
   * previous offset.
   *
   * @param prev  Previous offset range, this offset starts immediately after
   *              the previous offset range.
   * @param count The count (number) of elements
   */
  offset_u(const offset_u & prev, uint32_t count)
      : offset_u(prev.end(), count) {}

  /**
   * @brief Get the start index of the offset range.
   *
   * @return uint64_t Start index
   */
  uint64_t start() const {
    return o_ >> COUNT_BITS;
  }

  /**
   * @brief Get the count (number) of elements represented by this offset range.
   *
   * @return uint32_t Number of elements
   */
  uint32_t count() const {
    return o_ & count_mask;
  }

  /**
   * @brief Get the index pointing one element past the final element
   * represented by this offset range.
   *
   * @return uint64_t End index pointing to the first element past the last
   *                  element in this offset range
   */
  uint64_t end() const {
    return start() + count();
  }

  /**
   * @brief Set the count of this offset range.
   *
   * @param count New count value
   */
  void set_count(uint32_t count) {
    assert(count <= count_mask);
    o_ = (o_ & ~count_mask) | count;
  }

  /**
   * @brief Set the start index of this offset range.
   *
   * @param start New start index value
   */
  void set_offset(uint64_t start) {
    assert(start <= start_max);
    o_ = (o_ & count_mask) | (start << COUNT_BITS);
  }

  /**
   * @brief Get the range (start index to end index, inclusive) represented by
   * this offset range.
   *
   * @return std::pair<size_t, size_t> Range of this offset range
   */
  std::pair<size_t, size_t> range() const {
    uint64_t s = start();
    return {s, s + count()};
  }

private:
  /**
   * @brief The cobmined start index and count values. The lower COUNT_BITS are
   * used for the count value and the upper 64 - COUNT_BITS are used for the
   * start index
   */
  uint64_t o_;
};

} // namespace utils
} // namespace flecsi
