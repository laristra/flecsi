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

#include <set>
#include <vector>

#include <flecsi/geometry/point.h>

namespace flecsi {
namespace topology {

//----------------------------------------------------------------------------//
//! The mesh_definition_u type...
//!
//! @ingroup mesh-topology
//----------------------------------------------------------------------------//

template<size_t DIMENSION>
class mesh_definition_u
{
public:
  using point_t = point_u<double, DIMENSION>;

  /// Default constructor
  mesh_definition_u() {}

  /// Copy constructor (disabled)
  mesh_definition_u(const mesh_definition_u &) = delete;

  /// Assignment operator (disabled)
  mesh_definition_u & operator=(const mesh_definition_u &) = delete;

  /// Destructor
  virtual ~mesh_definition_u() {}

  ///
  /// Return the dimension of the mesh.
  ///
  static constexpr size_t dimension() {
    return DIMENSION;
  } // dimension

  //--------------------------------------------------------------------------//
  //! Abstract interface to get the number of entities.
  //!
  //! @param dimension The topological dimension of the request.
  //--------------------------------------------------------------------------//

  virtual size_t num_entities(size_t dimension) const = 0;

  //--------------------------------------------------------------------------//
  //! Abstract interface to get the entities of dimension \em to that define
  //! the entity of dimension \em from with the given identifier \em id.
  //!
  //! @param from_dimension The dimension of the entity for which the
  //!                       definition is being requested.
  //! @param to_dimension   The dimension of the entities of the definition.
  //! @param id             The id of the entity for which the definition is
  //!                       being requested.
  //--------------------------------------------------------------------------//

  virtual std::vector<size_t>
  entities(size_t from_dimension, size_t to_dimension, size_t id) const = 0;

  //--------------------------------------------------------------------------//
  //! Abstract interface to get the entities of dimension \em to that define
  //! the entity of dimension \em from with the given identifier \em id.
  //!
  //! @param from_dimension The dimension of the entity for which the
  //!                       definition is being requested.
  //! @param to_dimension   The dimension of the entities of the definition.
  //! @param id             The id of the entity for which the definition is
  //!                       being requested.
  //--------------------------------------------------------------------------//

  virtual std::set<size_t>
  entities_set(size_t from_dimension, size_t to_dimension, size_t id) const {
    auto vvec = entities(from_dimension, to_dimension, id);
    return std::set<size_t>(vvec.begin(), vvec.end());
  } // entities_set

}; // class mesh_definition_u

} // namespace topology
} // namespace flecsi
