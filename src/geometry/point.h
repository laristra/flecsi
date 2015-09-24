/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flexi_point_h
#define flexi_point_h

#include <array>
#include <cmath>

#include "../data/dimensioned_array.h"
#include "../utils/common.h"

/*!
 * \file point.h
 * \authors bergen
 * \date Initial file creation: Sep 23, 2015
 */

namespace flexi {

/*!
  \class point point.h
  \brief point defines an interface for storing and manipulating
  coordinate data associated with a geometric domain.

  The point type is implemented using \ref dimensioned_array.  Look there
  for more information on the point interface.
 */
template<typename T, size_t D>
using point = dimensioned_array<T,D>;

/*!
  \function distance
 */
template<typename T, size_t D>
T distance(point<T,D> & a, point<T,D> & b) {

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
