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

#include <flecsi/topology/common/array_buffer.h>
#include <flecsi/topology/index_space.h>
#include <flecsi/utils/offset.h>

namespace flecsi {
namespace topology {

template<typename T>
using topology_storage_u = array_buffer_u<T>;

class offset_storage_
{
public:
  using offset_t = utils::offset_t;

  const offset_t operator[](size_t i) const {
    return s_[i];
  }

  void add_count(uint32_t count) {
    offset_t o(start_, count);
    s_.push_back(o);
    start_ += count;
  }

  void add_end(size_t end) {
    assert(end > start_);
    add_count(static_cast<uint32_t>(end - start_));
  }

  std::pair<size_t, size_t> range(size_t i) const {
    return s_[i].range();
  }

  void clear() {
    s_.clear();
    start_ = 0;
  }

  auto & storage() {
    return s_;
  }

  const auto & storage() const {
    return s_;
  }

  size_t size() const {
    return s_.size();
  }

private:
  topology_storage_u<offset_t> s_;
  size_t start_ = 0;
};

template<typename T>
class identity_storage_u
{
public:
  class iterator
  {
  public:
    iterator(simple_id i) : i_(i) {}

    bool operator==(const iterator & itr) {
      return i_ == itr.i_;
    }

    bool operator!=(const iterator & itr) {
      return i_ != itr.i_;
    }

    simple_id operator*() {
      return i_;
    }

    iterator & operator++() {
      ++i_;
      return *this;
    }

  private:
    size_t i_;
  };

  simple_id operator[](size_t i) const {
    return i;
  }

  simple_id back() const {
    return size_ - 1;
  }

  void push_back(simple_id i) {
    assert(false && "invalid operation");
  }

  void pushed() {
    assert(false && "invalid operation");
  }

  void clear() {
    size_ = 0;
  }

  bool empty() const {
    return size_ == 0;
  }

  void resize(size_t n) {
    size_ = n;
  }

  size_t capacity() const {
    return size_;
  }

  template<typename... Args>
  void assign(Args &&... args) {
    assert(false && "invalid operation");
  }

  template<typename... Args>
  void insert(Args &&... args) {}

  void reserve(size_t n) {
    assert(false && "invalid operation");
  }

  iterator begin() const {
    return iterator(0);
  }

  iterator end() const {
    return iterator(size_ - 1);
  }

private:
  size_t size_;
};

} // namespace topology
} // namespace flecsi
