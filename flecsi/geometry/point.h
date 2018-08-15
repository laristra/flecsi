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
//! The point__ type defines an interface for storing and manipulating
//! coordinate data. The point__ type is implemented using \ref
//! dimensioned_array__.
//!
//! @tparam TYPE      The type to use to represent coordinate values.
//! @tparam DIMENSION The dimension of the point.
//!
//! @ingroup geometry
//----------------------------------------------------------------------------//

template<typename TYPE, size_t DIMENSION>
using point__ = utils::dimensioned_array__<TYPE, DIMENSION, 1>;

//----------------------------------------------------------------------------//
//! Multiplication operator.
//!
//! @tparam TYPE      The type to use to represent coordinate values.
//! @tparam DIMENSION The dimension of the point.
//!
//! @ingroup geometry
//----------------------------------------------------------------------------//

template<typename TYPE, size_t DIMENSION>
point__<TYPE, DIMENSION>
operator*(TYPE const val, point__<TYPE, DIMENSION> const & p) {
  point__<TYPE, DIMENSION> tmp(p);
  for (size_t d(0); d < DIMENSION; ++d) {
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
distance(
    point__<TYPE, DIMENSION> const & a,
    point__<TYPE, DIMENSION> const & b) {
  TYPE sum(0);
  for (size_t d(0); d < DIMENSION; ++d) {
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
point__<TYPE, DIMENSION>
midpoint(
    point__<TYPE, DIMENSION> const & a,
    point__<TYPE, DIMENSION> const & b) {
  return point__<TYPE, DIMENSION>((a + b) / 2.0);
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
centroid(CONTAINER<point__<TYPE, DIMENSION>> const & points) {
  point__<TYPE, DIMENSION> tmp(0.0);

  for (auto p : points) {
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
centroid(std::initializer_list<point__<TYPE, DIMENSION>> points) {
  point__<TYPE, DIMENSION> tmp(0.0);

  for (auto p : points) {
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
