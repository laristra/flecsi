/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flexi_dimensioned_array_h
#define flexi_dimensioned_array_h

#include <array>
#include <cmath>

#include "../utils/common.h"

/*!
 * \file dimensioned_array.h
 * \authors bergen
 * \date Initial file creation: Sep 23, 2015
 */

namespace flexi {

/*!
  \class dimensioned_array dimensioned_array.h
  \brief dimensioned_array provides...
 */
template<typename T, size_t D>
class dimensioned_array_
{
public:
  dimensioned_array_(const dimensioned_array_& a)
    : data_(a.data_){}

  //! Default constructor
  dimensioned_array_(std::initializer_list<T> list) {
    assert(list.size() == D && "dimension size mismatch");
    std::copy(list.begin(), list.end(), data_.begin());
  } // dimensioned_array

  //!
  template<typename ... A>
  dimensioned_array_(A ... args) {
    data_ = { args ... };
  }

  //! Assignment operator (disabled)
  dimensioned_array_ & operator = (const dimensioned_array_ &) = delete;

  //! Destructor
   ~dimensioned_array_() {}

  /*!
    \brief 

    ADD COMMENT ABOUT SUPPORT FOR ENUMS
   */
  template<typename E>
  T & operator [](E e) {
    return data_[static_cast<size_t>(e)];
  } // operator ()

private:

  std::array<T, D> data_;

}; // class dimensioned_array_

} // namespace flexi

#endif // flexi_dimensioned_array_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
