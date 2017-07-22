/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_topology_mesh_definition_h
#define flecsi_topology_mesh_definition_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Nov 17, 2016
//----------------------------------------------------------------------------//

#include <set>
#include <vector>

#include "flecsi/geometry/point.h"

namespace flecsi {
namespace topology {

//----------------------------------------------------------------------------//
//! The mesh_definition__ type...
//!
//! @ingroup mesh-topology
//----------------------------------------------------------------------------//

template<size_t D>
class mesh_definition__
{
public:

  using point_t = point<double, D>;

  /// Default constructor
  mesh_definition__() {}

  /// Copy constructor (disabled)
  mesh_definition__(const mesh_definition__ &) = delete;

  /// Assignment operator (disabled)
  mesh_definition__ & operator = (const mesh_definition__ &) = delete;

  /// Destructor
  virtual ~mesh_definition__() {}
  
  ///
  /// Return the dimension of the mesh.
  ///
  static 
  constexpr
  size_t
  dimension()
  {
    return D;
  } // dimension

  //--------------------------------------------------------------------------//
  //! Abstract interface to get the number of entities.
  //!
  //! @param dimension The topological dimension of the request.
  //--------------------------------------------------------------------------//

  virtual size_t num_entities(size_t dimension) const = 0;

  //--------------------------------------------------------------------------//
  //! Abstract interface to get the entities of entities
  //--------------------------------------------------------------------------//

  virtual std::vector<size_t>
    entities(size_t from_dim, size_t to_dim, size_t from_entity_id) 
    const = 0;


  //--------------------------------------------------------------------------//
  //! Abstract interface to get the number of entities.
  //--------------------------------------------------------------------------//

  /// Return the set of vertices of a particular entity.
  /// \param [in] dimension  The entity dimension to query.
  /// \param [in] entity_id  The id of the entity in question.
  /// \remark This version returns a set.
  virtual std::set<size_t> 
  entities_set(size_t from_dim, size_t to_dim, size_t entity_id) 
  const
  {
    auto vvec = entities(from_dim, to_dim, entity_id);
    return std::set<size_t>(vvec.begin(), vvec.end());
  }


  virtual point_t vertex(size_t vertex_id) const = 0;

private:

}; // class mesh_definition__

} // namespace topology
} // namespace flecsi

#endif // flecsi_topology_mesh_definition_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
