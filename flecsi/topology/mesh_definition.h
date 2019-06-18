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

template<size_t DIMENSION, typename REAL_TYPE = double>
class mesh_definition_u
{
public:
  using point_t = point_u<double, DIMENSION>;
  
  //! \brief the data type for connectivity
  using connectivity_t = std::vector<std::vector<size_t>>;
  
  //! \brief a byte type used for migrating data
  using byte_t = unsigned char;

  using real_t = REAL_TYPE;

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
  //! the entity of dimension \em from.
  //!
  //! @param from_dimension The dimension of the entity for which the
  //!                       definition is being requested.
  //! @param to_dimension   The dimension of the entities of the definition.
  //--------------------------------------------------------------------------//

  virtual const connectivity_t &
  entities(size_t from_dimension, size_t to_dimension) const = 0;

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

  virtual const flecsi::coloring::crs_t &
  entities_crs(size_t from_dim, size_t to_dim) const {
  }
  
  virtual const std::vector<size_t> &
  local_to_global(size_t dim) const
  {}

  virtual const std::map<size_t, size_t> &
  global_to_local(size_t dim) const {}

  virtual void create_graph(
      size_t from_dimension,
      size_t to_dimension,
      size_t min_connections,
      flecsi::coloring::dcrs_t & dcrs ) const = 0;

  virtual void pack(
    size_t dimension,
    size_t local_id,
    std::vector<byte_t> & buffer ) const {};

  virtual void unpack(
    size_t dimension,
    size_t local_id,
    byte_t const * & buffer ) {};

  virtual void erase(
    size_t dimension,
    const std::vector<size_t> & local_ids ) {};

  virtual void build_connectivity() {};

  virtual void vertex(size_t id, real_t * coord ) const {};

}; // class mesh_definition_u

//----------------------------------------------------------------------------
// A utilitiy for casting to byte arrays
//----------------------------------------------------------------------------
template< typename DATA_TYPE, typename BUFFER_TYPE >
void cast_insert(DATA_TYPE const * const data, size_t len, BUFFER_TYPE & buf)
{
  using buf_value_t = std::decay_t<decltype(buf[0])>;
  auto n = sizeof(DATA_TYPE) * len;
  auto p = reinterpret_cast<const buf_value_t*>(data);
  buf.insert( buf.end(), p, p+n );
};

//----------------------------------------------------------------------------
// A utilitiy for casting from byte arrays
//----------------------------------------------------------------------------
template< typename BUFFER_TYPE, typename DATA_TYPE >
void uncast(BUFFER_TYPE const *& buffer, size_t len, DATA_TYPE * data)
{
  auto n = sizeof(DATA_TYPE) * len;
  auto p = reinterpret_cast<BUFFER_TYPE*>(data);
  std::copy( buffer, buffer+n, p );
  auto start = buffer+n;
  buffer += n;
};

} // namespace topology
} // namespace flecsi
