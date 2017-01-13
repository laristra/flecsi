/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_topology_structured_mesh_topology_h
#define flecsi_topology_structured_mesh_topology_h

///
// \file structured_mesh_topology.h
// \authors bergen
// \date Initial file creation: Jan 13, 2017
///

namespace flecsi {
namespace topology {

///
// \class structured_mesh_topology_t structured_mesh_topology.h
// \brief structured_mesh_topology_t provides...
///
template<
  typename MT
>
class structured_mesh_topology_t
{
public:

  /// Default constructor
  structured_mesh_topology_t() {}

  /// Copy constructor (disabled)
  structured_mesh_topology_t(const structured_mesh_topology_t &) = delete;

  /// Assignment operator (disabled)
  structured_mesh_topology_t & operator = (const structured_mesh_topology_t &) = delete;

  /// Destructor
   ~structured_mesh_topology_t() {}

  size_t
  num_entities(
    size_t dim,
    size_t domain = 0
  )
  {
    return MT::num_entities(dim, domain);
  } // num_entities

private:

}; // class structured_mesh_topology_t

} // namespace topology
} // namespace flecsi

#endif // flecsi_topology_structured_mesh_topology_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
