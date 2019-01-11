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

#include <flecsi/utils/hash.h>

#include <cstring>
#include <limits>
#include <stdexcept>

namespace flecsi {
namespace utils {

/*!
  \class const_string const_string.h
  \brief const_string provides compile-time string constants and hashing...
  
  @ingroup utils
 */

class const_string_t {
public:
  using hash_type_t = std::size_t;

  /*!
    Construct a constexpr string.
   
    @param str A string literal.
   */

  template<hash_type_t N>
  constexpr const_string_t(const char (&str)[N])
      : str_(str), size_(N - 1) {} // const_string_t

  /*!
    Return the string literal for this const_string_t.
   */

  constexpr const char * c_str() const {
    return str_;
  } // c_str

  /*!
    Return the size of the string literal.
   */

  constexpr hash_type_t size() const {
    return size_;
  } // size

  /*!
    Array accessor to string literal.
   */

  constexpr char operator[](const hash_type_t i) const {
    return i < size_ ? str_[i] : throw std::out_of_range("invalid index");
  } // operator []

  /*!
    Return the hash for the string literal.
   */

  constexpr hash_type_t hash() const {
    return flecsi::utils::string_hash<hash_type_t>(str_, size_);
  } // hash

private:
  
  constexpr bool equal_(const const_string_t & t, const std::size_t i) const {
    return i == size_ ? true : (*this)[i] == t[i] && equal_(t, i + 1);
  } // equal_

public:

  /*!
   */
  constexpr bool operator==(const const_string_t & t) const {
    return size_ == t.size_ && equal_(t, 0);
  } // operator ==

  /*!
   */
  constexpr bool operator!=(const const_string_t & t) const {
    return !(*this == t);
  } // operator !=

private:

  const char * const str_;
  const hash_type_t size_;

}; // const_string_t

/*!
 */

struct const_string_hasher_t {

  /*!
   */

  std::size_t operator()(const const_string_t & str) const {
    return str.hash();
  } // operator ()

}; // const_string_hasher_t

} // namespace utils
} // namespace flecsi
