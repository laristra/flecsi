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

#include <stdint.h>
#include <algorithm>

namespace flecsi {
namespace data {

template<typename T>
struct simple_vector_u {

  using iterator = T *;
  using const_iterator = const T *;

  simple_vector_u() {}

  simple_vector_u(size_t init_count) {
    count = init_count;
    capacity = count;
    data = new T[count];
  }

  simple_vector_u(const simple_vector_u<T>& rhs) = default;

  ~simple_vector_u() = default;

  simple_vector_u<T>& operator=(const simple_vector_u<T>& rhs) = default;

  iterator begin() { return data; }
  iterator end() { return data + count; }
  const_iterator begin() const { return data; }
  const_iterator end() const { return data + count; }

  T & operator[](size_t index) {
    assert(index < count);
    return data[index];
  } // operator ()

  const T & operator[](size_t index) const {
    assert(index < count);
    return data[index];
  } // operator ()

  uint32_t size() const { return count; }

  void clear() {
    count = 0;
    capacity = 0;
    delete [] data;
    data = nullptr;
  }

  void resize(uint32_t new_count) {
    if(new_count <= capacity) {
      count = new_count;
      return;
    }

    auto new_data = new T[new_count];
    std::copy_n(data, count, new_data);
    delete [] data;
    count = new_count;
    capacity = new_count;
    data = new_data;
  } // resize

  void push_back(const T & value) {
    if(count == capacity) {
      resize(count + 5);
      count -= 5;
    }
    data[count] = value;
    count += 1;
  } // push_back

  void erase(const_iterator pos) {
    auto idx = pos - data;
    assert(idx >= 0);
    assert(idx < count);
    std::copy(data + idx + 1, end(), data + idx);
    count -= 1;
  } // erase

  iterator insert(const_iterator pos, const T & value) {
    auto idx = pos - data;
    assert(idx >= 0);
    assert(idx <= count);
    if(count == capacity) {
      resize(count + 5);
      count -= 5;
    }
    auto newpos = data + idx;
    std::copy_backward(newpos, end(), end() + 1);
    *newpos = value;
    count += 1;
    return newpos;
  } // insert

  uint32_t count = 0;
  uint32_t capacity = 0;
  T* data = nullptr;
  // CRF temp
  uint64_t tmpoffset = 0;

};  // simple_vector_u


} // namespace data
} // namespace flecsi

