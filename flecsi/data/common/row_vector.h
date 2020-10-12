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

#include <algorithm>
#include <stdint.h>

namespace flecsi {
namespace data {

template<typename T>
struct row_vector_u {

  using iterator = T *;
  using const_iterator = const T *;

  row_vector_u() = default;

  row_vector_u(uint32_t init_count) {
    count = init_count;
    capacity = count;
    datap = new T[count];
  }

  row_vector_u(const row_vector_u<T> & rhs) {
    assign(rhs.begin(), rhs.end());
  }

  ~row_vector_u() {
    delete[] datap;
  }

  row_vector_u<T> & operator=(const row_vector_u<T> & rhs) {
    if(&rhs != this)
      assign(rhs.begin(), rhs.end());
    return *this;
  }

  iterator begin() {
    return datap;
  }
  iterator end() {
    return datap + count;
  }
  const_iterator begin() const {
    return datap;
  }
  const_iterator end() const {
    return datap + count;
  }

  T & operator[](uint32_t index) {
    assert(index < count);
    return datap[index];
  } // operator ()

  const T & operator[](uint32_t index) const {
    assert(index < count);
    return datap[index];
  } // operator ()

  uint32_t size() const {
    return count;
  }

  T * data() {
    return datap;
  }

  const T * data() const {
    return datap;
  }

  void clear() {
    count = 0;
    capacity = 0;
    delete[] datap;
    datap = nullptr;
  }

  void assign(const_iterator first, const_iterator last) {
    resize(static_cast<uint32_t>(last - first));
    std::copy(first, last, datap);
  }

  void reserve(uint32_t new_cap) {
    if(new_cap <= capacity) {
      return;
    }

    auto new_data = new T[new_cap];
    std::copy_n(datap, count, new_data);
    delete[] datap;
    capacity = new_cap;
    datap = new_data;
  } // reserve

  void resize(uint32_t new_count) {
    reserve(new_count);
    count = new_count;
  } // resize

  void push_back(const T & value) {
    if(count == capacity) {
      reserve(count + 5);
    }
    datap[count] = value;
    count += 1;
  } // push_back

  void erase(const_iterator pos) {
    auto idx = pos - datap;
    assert(idx >= 0);
    assert(idx < count);
    std::copy(datap + idx + 1, end(), datap + idx);
    count -= 1;
  } // erase

  iterator insert(const_iterator pos, const T & value) {
    auto idx = pos - datap;
    assert(idx >= 0);
    assert(idx <= count);
    if(count == capacity) {
      reserve(count + 5);
    }
    auto newpos = datap + idx;
    std::copy_backward(newpos, end(), end() + 1);
    *newpos = value;
    count += 1;
    return newpos;
  } // insert

  uint32_t count = 0;
  uint32_t capacity = 0;
  T * datap = nullptr;

}; // row_vector_u

} // namespace data
} // namespace flecsi
