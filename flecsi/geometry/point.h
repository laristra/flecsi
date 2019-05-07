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

#pragma once

//----------------------------------------------------------------------------//
//! @file
//! \date Initial file creation: Sep 23, 2015
//----------------------------------------------------------------------------//

#include <array>
#include <cmath>

#include <flecsi/utils/common.h>
#include <flecsi/utils/dimensioned_array.h>

namespace flecsi {

//----------------------------------------------------------------------------//
//! The point_u type defines an interface for storing and manipulating
//! coordinate data. The point_u type is implemented using \ref
//! dimensioned_array_u.
//!
//! @tparam TYPE      The type to use to represent coordinate values.
//! @tparam DIMENSION The dimension of the point.
//!
//! @ingroup geometry
//----------------------------------------------------------------------------//

template<typename TYPE, size_t DIMENSION>
using point_u = utils::dimensioned_array_u<TYPE, DIMENSION, 1>;

//----------------------------------------------------------------------------//
//! Multiplication operator.
//!
//! @tparam TYPE      The type to use to represent coordinate values.
//! @tparam DIMENSION The dimension of the point.
//!
//! @ingroup geometry
//----------------------------------------------------------------------------//

template<typename TYPE, size_t DIMENSION>
point_u<TYPE, DIMENSION> operator*(TYPE const val,
  point_u<TYPE, DIMENSION> const & p) {
  point_u<TYPE, DIMENSION> tmp(p);
  for(size_t d(0); d < DIMENSION; ++d) {
    tmp[d] *= val;
  } // for

  return tmp;
} // operator *

//----------------------------------------------------------------------------//
//! Return the distance between the given points.
//!
//! @tparam TYPE      The type to use to represent coordinate values.
//! @tparam DIMENSION The dimension of the point.
//!
//! @param a The first point.
//! @param b The second point.
//!
//! @ingroup geometry
//----------------------------------------------------------------------------//

template<typename TYPE, size_t DIMENSION>
TYPE
distance(point_u<TYPE, DIMENSION> const & a,
  point_u<TYPE, DIMENSION> const & b) {
  TYPE sum(0);
  for(size_t d(0); d < DIMENSION; ++d) {
    sum += utils::square(a[d] - b[d]);
  } // for

  return std::sqrt(sum);
} // distance

//----------------------------------------------------------------------------//
//! Return the midpoint between two points.
//!
//! @tparam TYPE      The type to use to represent coordinate values.
//! @tparam DIMENSION The dimension of the point.
//!
//! @param a The first point.
//! @param b The second point.
//!
//! @ingroup geometry
//----------------------------------------------------------------------------//

template<typename TYPE, size_t DIMENSION>
point_u<TYPE, DIMENSION>
midpoint(point_u<TYPE, DIMENSION> const & a,
  point_u<TYPE, DIMENSION> const & b) {
  return point_u<TYPE, DIMENSION>((a + b) / 2.0);
} // midpoint

//----------------------------------------------------------------------------//
//! Return the centroid of the given set of points.
//!
//! @tparam TYPE      The type to use to represent coordinate values.
//! @tparam DIMENSION The dimension of the point.
//!
//! @param points The points for which to find the centroid.
//!
//! @ingroup geometry
//----------------------------------------------------------------------------//

template<template<typename...> class CONTAINER, typename TYPE, size_t DIMENSION>
auto
centroid(CONTAINER<point_u<TYPE, DIMENSION>> const & points) {
  point_u<TYPE, DIMENSION> tmp(0.0);

  for(auto p : points) {
    tmp += p;
  } // for

  tmp /= points.size();

  return tmp;
} // centroid

//----------------------------------------------------------------------------//
//! Return the centroid of the given set of points.
//!
//! @tparam TYPE      The type to use to represent coordinate values.
//! @tparam DIMENSION The dimension of the point.
//!
//! @param points The points for which to find the centroid.
//!
//! @ingroup geometry
//----------------------------------------------------------------------------//

template<typename TYPE, size_t DIMENSION>
auto
centroid(std::initializer_list<point_u<TYPE, DIMENSION>> points) {
  point_u<TYPE, DIMENSION> tmp(0.0);

  for(auto p : points) {
    tmp += p;
  } // for

  tmp /= points.size();

  return tmp;
} // centroid

} // namespace flecsi

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
