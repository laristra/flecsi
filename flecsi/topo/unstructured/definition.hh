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

#include "flecsi/util/crs.hh"
#include "flecsi/util/geometry/point.hh"

#include <set>
#include <vector>

namespace flecsi {
namespace topo {
namespace unstructured_impl {

/*!
  The definition type...

  @ingroup mesh-topology
 */

template<size_t DIMENSION, typename REAL_TYPE = double>
class definition
{
public:
  using point_t = util::point<double, DIMENSION>;
  using connectivity_t = std::vector<std::vector<size_t>>;

  definition(const definition &) = delete;
  definition & operator=(const definition &) = delete;

  definition() {}
  virtual ~definition() {}

  /*!
    Return the dimension of the mesh.
   */

  static constexpr size_t dimension() {
    return DIMENSION;
  }

  /*!
    Abstract interface to get the number of entities.

    @param dimension The topological dimension of the request.
   */

  virtual size_t num_entities(size_t dimension) const = 0;

  /*!
    Abstract interface to get the entities of dimension \em to that define
    the entity of dimension \em from with the given identifier \em id.

    @param from_dimension The dimension of the entity for which the
                          definition is being requested.
    @param to_dimension   The dimension of the entities of the definition.
    @param id             The id of the entity for which the definition is
                          being requested.
   */

  virtual std::vector<size_t>
  entities(size_t from_dimension, size_t to_dimension, size_t id) const = 0;

  /*!
    Abstract interface to get the entities of dimension \em to that define
    the entity of dimension \em from.

    @param from_dimension The dimension of the entity for which the
                          definition is being requested.
    @param to_dimension   The dimension of the entities of the definition.
   */

  virtual const connectivity_t & entities(size_t from_dimension,
    size_t to_dimension) const = 0;

  /*!
    Abstract interface to get the entities of dimension \em to that define
    the entity of dimension \em from with the given identifier \em id.

    @param from_dimension The dimension of the entity for which the
                          definition is being requested.
    @param to_dimension   The dimension of the entities of the definition.
    @param id             The id of the entity for which the definition is
                          being requested.
   */

  virtual std::set<size_t>
  entities_set(size_t from_dimension, size_t to_dimension, size_t id) const {
    auto vvec = entities(from_dimension, to_dimension, id);
    return std::set<size_t>(vvec.begin(), vvec.end());
  } // entities_set

}; // class definition

} // namespace unstructured_impl
} // namespace topo
} // namespace flecsi
