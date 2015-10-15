/*~--------------------------------------------------------------------------~*
 *  @@@@@@@@ @@       @@@@@@@@ @@     @@ @@
 * /@@///// /@@      /@@///// //@@   @@ /@@
 * /@@      /@@      /@@       //@@ @@  /@@
 * /@@@@@@@ /@@      /@@@@@@@   //@@@   /@@
 * /@@////  /@@      /@@////     @@/@@  /@@
 * /@@      /@@      /@@        @@ //@@ /@@
 * /@@      /@@@@@@@@/@@@@@@@@ @@   //@@/@@
 * //       //////// //////// //     // //
 *
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~--------------------------------------------------------------------------~*/

#ifndef flexi_const_string_h
#define flexi_const_string_h

/*!
 * \file const_string.h
 * \authors nickm
 * \date Initial file creation: Oct 15, 2015
 */

#include <limits>

namespace flexi {

/*!
  \class const_string const_string.h
  \brief const_string provides compile-time string constants and hashing...
 */

class const_string_t{
public:
  template<size_t N>
  constexpr const_string_t(const char(&str)[N])
    : str_(str),
      size_(N - 1){}

  constexpr const char* c_str() const{
    return str_;
  }

  constexpr size_t size() const{
    return size_;
  }
   
  constexpr char operator[](size_t i) const{
    return i < size_ ? str_[i] : throw std::out_of_range("invalid index");
  }

  constexpr size_t hash() const{
    size_t h = 0;

    for(size_t i = 0; i < size_; ++i){
      h ^= size_t(str_[i]) << 8 * (i % 8);
    }

    return h;
  }

private:
  const char* const str_;
  const size_t size_;
};

} // namespace flexi

#endif // flexi_const_string

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
