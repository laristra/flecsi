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

template<size_t, typename>
class domain_entity_u;

template<typename item_t>
struct array_buffer_type_u {
  using type = item_t *;
};

template<size_t M, typename E>
struct array_buffer_type_u<domain_entity_u<M, E>> {
  using type = E *;
};

template<typename item_t>
struct array_buffer_type_u<item_t *> {
  using type = item_t *;
};

template<typename T>
struct array_buf_ref_type_u {
  using type = T &;
};

template<typename S>
struct array_buf_ref_type_u<S *> {
  using type = S *;
};

template<size_t M, class E>
struct array_buf_ref_type_u<domain_entity_u<M, E>> {
  using type = E *;
};

template<typename T, bool B>
struct array_buf_ref_get_u {
  FLECSI_INLINE_TARGET
  static T get(T a, size_t i) {
    return &a[i];
  }
};

template<typename T>
struct array_buf_ref_get_u<T, false> {
  FLECSI_INLINE_TARGET
  static auto get(T a, size_t i) -> decltype(a[i]) {
    return a[i];
  }
};

template<typename T>
class array_buffer_u
{
public:
  using item_t = typename array_buffer_type_u<T>::type;

  using iterator = item_t;

  using const_iterator = item_t;

  using ref_t = typename array_buf_ref_type_u<T>::type;

  array_buffer_u() : buf_(nullptr), size_(0), capacity_(0) {}

  FLECSI_INLINE_TARGET
  ref_t operator[](size_t index) {
    return array_buf_ref_get_u<item_t, std::is_pointer<ref_t>::value>::get(
      buf_, index);
  }

  FLECSI_INLINE_TARGET
  const ref_t operator[](size_t index) const {
    return array_buf_ref_get_u<const item_t,
      std::is_pointer<ref_t>::value>::get(buf_, index);
  }

  FLECSI_INLINE_TARGET
  ref_t back() {
    return array_buf_ref_get_u<item_t, std::is_pointer<ref_t>::value>::get(
      buf_, size_ - 1);
  }

  FLECSI_INLINE_TARGET
  const ref_t back() const {
    return array_buf_ref_get_u<item_t, std::is_pointer<ref_t>::value>::get(
      buf_, size_ - 1);
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
  size_t size_;
  size_t capacity_;
};

} // namespace topology
} // namespace flecsi
