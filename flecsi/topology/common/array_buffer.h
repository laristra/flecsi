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

#include <flecsi/utils/id.h>

namespace flecsi {
namespace topology {

template<typename T>
class array_buffer_u
{
public:
  using item_t = T *;

  using iterator = item_t;

  using const_iterator = item_t;

  using ref_t = T &;

  array_buffer_u() : buf_(nullptr), size_(0), capacity_(0) {}

  FLECSI_INLINE_TARGET
  ref_t operator[](size_t index) const {
    return buf_[index];
  }

  FLECSI_INLINE_TARGET
  const ref_t back() const {
    return (*this)[size() - 1];
  }

  FLECSI_INLINE_TARGET
  size_t size() const {
    return size_;
  } // size

  FLECSI_INLINE_TARGET
  size_t capacity() const {
    return capacity_;
  } // capacity

  FLECSI_INLINE_TARGET
  item_t begin() {
    return buf_;
  }

  FLECSI_INLINE_TARGET
  item_t end() {
    return buf_ + size_;
  }

  FLECSI_INLINE_TARGET
  const item_t begin() const {
    return buf_;
  }

  FLECSI_INLINE_TARGET
  const item_t end() const {
    return buf_ + size_;
  }

  template<typename... Args>
  FLECSI_INLINE_TARGET void insert(Args &&... args) {}

  FLECSI_INLINE_TARGET
  void push_back(const ref_t x) {
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if(size_ >= capacity_) {
      std::cout << "adding " << size_ << " with cap " << capacity_
                << " on rank " << rank << std::endl
                << std::flush;
    }
    assert(size_ < capacity_ && "array buffer capacity exceeded");
    buf_[size_++] = x;
  }

  FLECSI_INLINE_TARGET
  void pushed() {
    ++size_;
  }

  FLECSI_INLINE_TARGET
  void clear() {
    size_ = 0;
  }

  FLECSI_INLINE_TARGET
  bool empty() const {
    return size_ == 0;
  }

  FLECSI_INLINE_TARGET
  void resize(size_t n) {
    assert(n <= capacity_);
    size_ = n;
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

  FLECSI_INLINE_TARGET
  item_t data() {
    return buf_;
  }

  FLECSI_INLINE_TARGET
  const item_t data() const {
    return buf_;
  }

  template<typename... Args>
  FLECSI_INLINE_TARGET void assign(Args &&... args) {
    assert(false && "unimplemented");
  }

  FLECSI_INLINE_TARGET
  void reserve(size_t n) {
    assert(false && "unimplemented");
  }

private:
  item_t buf_;
  // Since objects of this type are routinely initialized as the wrong type
  // and then subjected to reinterpret_cast, correct (undefined) behavior
  // depends on storing counts instead of pointers (and not doing arithmetic
  // except as the right type).
  size_t size_;
  size_t capacity_;
};

} // namespace topology
} // namespace flecsi
