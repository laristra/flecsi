/*~--------------------------------------------------------------------------~*
 *  @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
 * /@@/////  /@@          @@////@@ @@////// /@@
 * /@@       /@@  @@@@@  @@    // /@@       /@@
 * /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
 * /@@////   /@@/@@@@@@@/@@       ////////@@/@@
 * /@@       /@@/@@//// //@@    @@       /@@/@@
 * /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
 * //       ///  //////   //////  ////////  // 
 * 
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_const_string_h
#define flecsi_const_string_h

/*!
 * \file const_string.h
 * \authors nickm
 * \date Initial file creation: Oct 15, 2015
 */

#include <limits>

namespace flecsi {

/*!
  \class const_string const_string.h
  \brief const_string provides compile-time string constants and hashing...
 */

class const_string_t{
public:

  using hash_type_t = size_t;

  template<hash_type_t N>
  constexpr const_string_t(const char(&str)[N])
    : str_(str),
      size_(N - 1){}

  constexpr const char* c_str() const{
    return str_;
  }

  constexpr hash_type_t size() const{
    return size_;
  }
   
  constexpr char operator[](hash_type_t i) const{
    return i < size_ ? str_[i] : throw std::out_of_range("invalid index");
  }

  constexpr hash_type_t hash() const{
    hash_type_t h = 0;

    for(hash_type_t i = 0; i < size_; ++i){
      h ^= hash_type_t(str_[i]) << 8 * (i % 8);
    }

    return h;
  }

  constexpr bool operator==(const const_string_t& t) const{
    if(size_ != t.size_){
      return false;
    }

    for(hash_type_t i = 0; i < size_; ++i){
      if((*this)[i] != t[i]){
        return false;
      }
    }

    return true;
  }

  constexpr bool operator!=(const const_string_t& t) const{
    return !(*this == t);
  }

private:
  const char* const str_;
  const hash_type_t size_;
};

} // namespace flecsi

#endif // flecsi_const_string

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
