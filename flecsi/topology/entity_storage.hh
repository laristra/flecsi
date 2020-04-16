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

#include "array_buffer.hh"
#include "index_space.hh"
#include <flecsi/utils/offset.hh>

namespace flecsi {
namespace topo {

template<typename T>
using topology_storage = array_buffer<T>;

class offset_storage_t
{
public:
  using offset_t = util::offset_t;

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
  topology_storage<offset_t> s_;
  size_t start_ = 0;
}; // class offset_storage_t

template<typename T>
class identity_storage
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

  void push_back(simple_id) {
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
  void assign(Args &&...) {
    assert(false && "invalid operation");
  }

  template<typename... Args>
  void insert(Args &&...) {}

  void reserve(size_t) {
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
}; // struct identity_storage

} // namespace topo
} // namespace flecsi
