/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flexi_point_h
#define flexi_point_h

#include <array>
#include <cmath>

#include "../utils/common.h"

/*!
 * \file point.h
 * \authors bergen
 * \date Initial file creation: Sep 23, 2015
 */

namespace flexi {

enum class axis : size_t { x = 0, y = 1, z = 2 };

/*!
  \class point point.h
  \brief point provides...
 */
template<typename T, size_t D>
class point_
{
public:

  //! Default constructor
  point_(std::initializer_list<T> list) {
    assert(list.size() == D && "dimension size mismatch");
    std::copy(list.begin(), list.end(), data_.begin());
  } // point

  //!
  template<typename ... A>
  point_(A ... args) {
    data_ = { args ... };
  }

  //! Copy constructor (disabled)
  point_(const point_ &) = delete;

  //! Assignment operator (disabled)
  point_ & operator = (const point_ &) = delete;

  //! Destructor
   ~point_() {}

  T & operator [](axis a) {
    return data_[static_cast<size_t>(a)];
  } // operator ()

private:

  std::array<T, D> data_;

}; // class point_

template<typename T, size_t D>
T distance(point_<T,D> & a, point_<T,D> & b) {

  T sum(square(a[0]-b[0]));
  if(D>1) {
    sum += square(a[1]-b[1]);
  }

  if(D>2) {
    sum += square(a[2]-b[2]);
  }

  return std::sqrt(sum);
} // distance

} // namespace flexi

#endif // flexi_point_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
