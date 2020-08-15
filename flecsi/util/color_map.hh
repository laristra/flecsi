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

#include <flecsi-config.h>

#include "flecsi/flog.hh"

namespace flecsi {
namespace util {

/*!
  The color_map type provides information for creating mappings of colors to
  processes, where the ratio of processes to colors is not one. Additionally,
  the color_map type provides information for partitioning a set of indices
  across a set of colors.
 */

struct color_map {

  /*!
    Construct a color map.

    @param processes The number of processes from the runtime.
    @param colors    The desired number of colors for the mapping (to be
                     partitioned onto the processes).
    @param indices   The number of indices that should be partitioned onto the
                     colors.
   */

  color_map(size_t processes, size_t colors, size_t indices)
    : colors_(colors), indices_(indices),
      domain_size_(std::min(processes, colors)), quotient_(indices / colors),
      remainder_(indices % colors), color_quotient_(colors / processes),
      color_remainder_(colors % processes), dist_(colors + 1) {
    init();
  }

  /*
    Initialization method.
   */

  void init() {
    dist_[0] = 0;
    for(size_t p{0}, offset{0}; p < domain_size_; ++p) {
      for(size_t c{0}; c < colors(p); ++c) {
        dist_[offset + 1] = dist_[offset] + indices(p, c);
        offset++;
      }
    }
  }

  /*!
    Return the launch domain size for this color map. The launch domain size
    will be the minimum of the number of processes and colors.
   */

  size_t domain_size() const {
    return domain_size_;
  }

  /*!
    Return the distribution of indices across colors.
   */

  std::vector<size_t> const & distribution() const {
    return dist_;
  }

  /*!
    The offset of the first color for the given process.
   */

  size_t color_offset(size_t process) const {
    return process * color_quotient_ +
           (process >= color_remainder_ ? color_remainder_ : process);
  }

  /*!
    The total number of colors.
   */

  size_t colors() const {
    return colors_;
  }

  /*!
    The number of colors for the given process.
   */

  size_t colors(size_t process) const {
    return color_quotient_ + (process < color_remainder_ ? 1 : 0);
  }

  /*!
    The global color id for the given process and color index
    (zero to < process colors).

    @param process The process (zero to < processes).
    @param color   The color index (zero to < process colors).
   */

  size_t color_id(size_t process, size_t color) const {
    return color_offset(process) + color;
  }

  /*!
    The offset of the first index for the given process and color.

    @param process The process (zero to < processes).
    @param color   The color index (zero to < process colors).
   */

  size_t index_offset(size_t process, size_t color) const {
    const size_t c = color_offset(process) + color;
    return c * quotient_ + (c >= remainder_ ? remainder_ : c);
  }

  /*!
    The color of the given index.

    @param index An index that is in the range of indices.
   */

  size_t index_color(size_t index) const {
    flog_assert(index < indices(),
      "index(" << index << " out-of-range(" << indices() << ")");

    const size_t lower = remainder_ * (quotient_ + 1);
    if(index < lower) {
      return index / (quotient_ + 1);
    }
    else {
      return remainder_ + (index - lower) / quotient_;
    }
  }

  /*!
    Return the process that owns the given color.
   */

  size_t process(size_t color) {
    flog_assert(
      color < colors_, "color(" << color << " out-of-range(" << colors_ << ")");

    const size_t lower = color_remainder_ * (color_quotient_ + 1);
    if(color < lower) {
      return color / (color_quotient_ + 1);
    }
    else {
      return color_remainder_ + (color - lower) / color_quotient_;
    }
  }

  /*!
    The total number of indices.
   */

  size_t indices() const {
    return indices_;
  }

  /*!
    The number of indices that are assigned to the given process and color.

    @param process The process (zero to < processes).
    @param color   The color index (zero to < process colors).
   */

  size_t indices(size_t process, size_t color) const {
    const size_t c = color_offset(process) + color;
    return quotient_ + (c < remainder_ ? 1 : 0);
  }

private:
  size_t colors_;
  size_t indices_;
  size_t domain_size_;
  size_t quotient_;
  size_t remainder_;
  size_t color_quotient_;
  size_t color_remainder_;

  std::vector<size_t> dist_;

}; // struct color_map

} // namespace util
} // namespace flecsi
