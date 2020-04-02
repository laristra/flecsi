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

#include <flecsi/topology/index_space.h>
#include <flecsi/utils/array_ref.h>
#include <flecsi/utils/offset.h>

namespace flecsi {
namespace topology {

template<typename T>
using topology_storage_u = utils::vector_ref<T>;

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
struct identity_storage_u : view_tag {
  class iterator
  {
  public:
    using difference_type = std::ptrdiff_t;
    using value_type = simple_id;
    using pointer = void;
    using reference = value_type;
    using iterator_category = std::input_iterator_tag;

    iterator(simple_id i) : i_(i) {}

    bool operator==(const iterator & itr) const {
      return i_ == itr.i_;
    }

    bool operator!=(const iterator & itr) const {
      return i_ != itr.i_;
    }

    simple_id operator*() const {
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

  void clear() {
    size_ = 0;
  }

  bool empty() const {
    return size_ == 0;
  }

  void resize(size_t n) {
    size_ = n;
  }

  std::size_t size() const {
    return size_;
  }

  iterator begin() const {
    return iterator(0);
  }

  iterator end() const {
    return iterator(size_);
  }

private:
  size_t size_;
};

} // namespace topology
} // namespace flecsi
