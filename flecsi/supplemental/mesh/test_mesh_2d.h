/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_test_mesh_h
#define flecsi_execution_test_mesh_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: May 10, 2017
//----------------------------------------------------------------------------//

#include "flecsi/topology/mesh.h"
#include "flecsi/topology/mesh_topology.h"

//----------------------------------------------------------------------------//
// Enumeration to name index spaces
//----------------------------------------------------------------------------//

enum index_spaces : size_t
{
  cells = 0,
  vertices = 1,
  cells_to_vertices = 3
}; // enum index_spaces

namespace flecsi {
namespace supplemental {

//----------------------------------------------------------------------------//
// Entity types
//----------------------------------------------------------------------------//

struct vertex_t : public flecsi::topology::mesh_entity_t<0, 1>
{
}; // struct vertex_t

struct edge_t : public flecsi::topology::mesh_entity_t<1, 1>
{
}; // struct edge_t

struct face_t : public flecsi::topology::mesh_entity_t<1, 1>
{
}; // struct face_t

struct cell_t : public flecsi::topology::mesh_entity_t<2, 1>
{
  using id_t = flecsi::utils::id_t;

  std::vector<size_t>
  create_entities(
    id_t cell_id,
    size_t dim,
    flecsi::topology::domain_connectivity<2> & c,
    id_t * e
  )
  {
    id_t* v = c.get_entities(cell_id, 0);

    e[0] = v[0];
    e[1] = v[2];

    e[2] = v[1];
    e[3] = v[3];

    e[4] = v[0];
    e[5] = v[1];

    e[6] = v[2];
    e[7] = v[3];

    return {2, 2, 2, 2};
  } // create_entities

}; // struct cell_t

//----------------------------------------------------------------------------//
// Mesh policy
//----------------------------------------------------------------------------//

struct test_mesh_2d_policy_t
{
  flecsi_register_number_dimensions(2);
  flecsi_register_number_domains(1);

  flecsi_register_entity_types(
    flecsi_entity_type(index_spaces::vertices, 0, vertex_t),
    flecsi_entity_type(index_spaces::cells, 0, cell_t)
  );

  flecsi_register_connectivities(
    flecsi_connectivity(index_spaces::cells_to_vertices, 0, cell_t, vertex_t)
  );

  flecsi_register_bindings();

  template<
    size_t M,
    size_t D,
    typename ST
  >
  static flecsi::topology::mesh_entity_base_t<num_domains> *
  create_entity(
    flecsi::topology::mesh_topology_base_t<ST>* mesh,
    size_t num_vertices
  )
  {
#if 0
    switch(M) {

      case 0:
      {
        switch(D) {
          case 1:
            return mesh->template make<edge_t>(*mesh);
          default:
            assert(false && "invalid topological dimension");
        } // switch

        break;
      } // scope

      default:
        assert(false && "invalid domain");
    } // switch
#endif
  } // create_entity

}; // struct test_mesh_2d_t

//----------------------------------------------------------------------------//
// Mesh type
//----------------------------------------------------------------------------//

struct test_mesh_2d_t :
  public flecsi::topology::mesh_topology_t<test_mesh_2d_policy_t>
{
  decltype(auto)
  cells() {
    return entities<2,0>();
  } // cells
}; // struct test_mesh_2d_t

} // namespace supplemental
} // namespace flecsi

#endif // flecsi_execution_test_mesh_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
