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

#include "flecsi/utils/array_ref.h"
#include <flecsi/utils/id.h>

namespace flecsi {
namespace topology {

/// An \c array_buffer_u describes a \c std::vector-like array.
/// Because it is often used with a type-erased \p T, it cannot be indexed.
/// \tparam T nominal element type
template<typename T>
class array_buffer_u
{
public:
  using item_t = T *;

  array_buffer_u() : buf_(nullptr), size_(0), capacity_(0) {}

  FLECSI_INLINE_TARGET
  size_t size() const {
    return size_;
  } // size

  FLECSI_INLINE_TARGET
  size_t capacity() const {
    return capacity_;
  } // capacity

  /// Convert to a typed \c vector_ref.
  /// \tparam U actual (dynamic) element type
  template<class U>
  operator utils::vector_ref<U>() const {
    return {{static_cast<U *>(buffer()), capacity()}, size()};
  }

  FLECSI_INLINE_TARGET
  bool empty() const {
    return size_ == 0;
  }

  FLECSI_INLINE_TARGET
  void set_buffer(item_t buf, size_t size) {
    buf_ = buf;
    size_ = size;
    capacity_ = size;
  }

  FLECSI_INLINE_TARGET
  void set_buffer(item_t buf, size_t capacity, bool initialized) {
    buf_ = buf;
    capacity_ = capacity;
    size_ = initialized ? capacity_ : 0;
  }

  FLECSI_INLINE_TARGET
  void set_buffer(item_t buf, size_t capacity, size_t size) {
    buf_ = buf;
    capacity_ = capacity;
    size_ = size;
  }

  FLECSI_INLINE_TARGET
  item_t buffer() {
    return buf_;
  }

  FLECSI_INLINE_TARGET
  const item_t buffer() const {
    return buf_;
  }

private:
  item_t buf_;
  size_t size_;
  size_t capacity_;
};

} // namespace topology
} // namespace flecsi
