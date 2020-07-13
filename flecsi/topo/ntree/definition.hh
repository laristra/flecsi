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

#include <flecsi/util/geometry/point.hh>

#include <set>
#include <vector>

namespace flecsi {
namespace topology {
namespace ntree_impl {

/*!
  The definition type...

  @ingroup ntree-topology
 */

template<size_t DIMENSION, typename REAL_TYPE = double>
class definition
{
public:
  using point_t = util::point<REAL_TYPE, DIMENSION>;

  definition(const definition &) = delete;
  definition & operator=(const definition &) = delete;

  definition() {}
  virtual ~definition() {}

  /*!
    Return the spatial dimension.
   */

  static constexpr size_t dimension() {
    return DIMENSION;
  }

  virtual size_t local_num_entities() const = 0;
  virtual size_t global_num_entities() const = 0;
  virtual size_t distribution(const int & i) const = 0;
  virtual std::pair<size_t, size_t> offset(const int & i) const = 0;

}; // class definition

} // namespace ntree_impl
} // namespace topology
} // namespace flecsi
