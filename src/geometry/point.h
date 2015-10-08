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
template <typename T, size_t D> using point = dimensioned_array<T, D>;

/*!
  \function distance
 */
template <typename T, size_t D> T distance(point<T, D> &a, point<T, D> &b) {

  T sum(0);
  for (size_t d(0); d < D; ++d) {
    sum += square(a[d] - b[d]);
  } // for

  return std::sqrt(sum);
} // distance

} // namespace flexi

#endif // flexi_point_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
