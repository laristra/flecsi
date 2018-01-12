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


#include <cstring> // for std::size_t

namespace flecsi {
namespace utils {

//!
//! \class iterator iterator.h
//! \brief iterator provides...
//!
template<typename C, typename T>
class iterator {
public:
  using container_t = C;
  using type_t = T;

  //! Constructor from container and index
  iterator(container_t & values, const std::size_t index)
      : values_(values), index_(index) {}

  //! Destructor
  ~iterator() {}

  //! Copy constructor
  iterator(const iterator & it) : values_(it.values_), index_(it.index_) {}

  //! Assignment operator
  iterator & operator=(const iterator & it) {
    values_ = it.values_;
    index_ = it.index_;
    return *this;
  } // operator =

  //! Pre-increment operator
  iterator & operator++() {
    ++index_;
    return *this;
  } // operator ++

  //! Dereference operator
  type_t & operator*() {
    return values_[index_];
  } // operator *

  /*
  // Remark: operator-> normally returns a pointer, although it isn't required
  // to do so. I don't see it being used anywhere, and will comment it out for
  // now. If we eventually want it, let's talk about what we really want it to
  // do. -Martin S., 2017-may-24.
  //! Member-of-pointer operator
  type_t & operator -> () {
    return values_[index_];
  } // operator ->
  */

  //! Equivalence operator
  bool operator==(const iterator & it) const {
    return index_ == it.index_;
  } // operator ==

  //! Non-equivalence operator
  bool operator!=(const iterator & it) const {
    return index_ != it.index_;
  } // operator !=

private:
  container_t & values_;
  std::size_t index_;

}; // class iterator

} // namespace utils
} // namespace flecsi
