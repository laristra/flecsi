/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016 Los Alamos National Laboratory, LLC
   All rights reserved
                                                                              */

#pragma once

/*! @file */

#include "flecsi/util/common.hh"
#include "flecsi/util/dimensioned_array.hh"

#include <array>
#include <cmath>

namespace flecsi {
namespace util {

//----------------------------------------------------------------------------//
//! The point type defines an interface for storing and manipulating
//! coordinate data. The point type is implemented using \ref
//! dimensioned_array.
//!
//! @tparam TYPE      The type to use to represent coordinate values.
//! @tparam DIMENSION The dimension of the point.
//!
//! @ingroup geometry
//----------------------------------------------------------------------------//

template<typename TYPE, size_t DIMENSION>
using point = dimensioned_array<TYPE, DIMENSION, 1>;

//----------------------------------------------------------------------------//
//! Multiplication operator.
//!
//! @tparam TYPE      The type to use to represent coordinate values.
//! @tparam DIMENSION The dimension of the point.
//!
//! @ingroup geometry
//----------------------------------------------------------------------------//

template<typename TYPE, size_t DIMENSION>
constexpr point<TYPE, DIMENSION> operator*(TYPE const val,
  point<TYPE, DIMENSION> const & p) {
  point<TYPE, DIMENSION> tmp(p);
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
distance(point<TYPE, DIMENSION> const & a, point<TYPE, DIMENSION> const & b) {
  TYPE sum(0);
  for(size_t d(0); d < DIMENSION; ++d) {
    sum += (square)(a[d] - b[d]);
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
constexpr point<TYPE, DIMENSION>
midpoint(point<TYPE, DIMENSION> const & a, point<TYPE, DIMENSION> const & b) {
  return point<TYPE, DIMENSION>((a + b) / 2.0);
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
constexpr auto
centroid(CONTAINER<point<TYPE, DIMENSION>> const & points) {
  point<TYPE, DIMENSION> tmp(0.0);

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
constexpr auto
centroid(std::initializer_list<point<TYPE, DIMENSION>> points) {
  point<TYPE, DIMENSION> tmp(0.0);

  for(auto p : points) {
    tmp += p;
  } // for

  tmp /= points.size();

  return tmp;
} // centroid

} // namespace util
} // namespace flecsi
