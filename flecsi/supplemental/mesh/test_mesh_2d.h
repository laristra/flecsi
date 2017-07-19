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

namespace flecsi {
namespace supplemental {

class Vertex : public flecsi::topology::mesh_entity_t<0, 1>{
public:
  template<size_t M>
  uint64_t precedence() const { return 0; }
  Vertex() = default;

};

class Edge : public flecsi::topology::mesh_entity_t<1, 1>{
public:
};

class Face : public flecsi::topology::mesh_entity_t<1, 1>{
public:

};

class Cell : public flecsi::topology::mesh_entity_t<2, 1>{
public:

  using id_t = flecsi::utils::id_t;

  void set_precedence(size_t dim, uint64_t precedence) {}

  std::vector<size_t>
  create_entities(id_t cell_id, size_t dim,
    flecsi::topology::domain_connectivity<2> & c, id_t * e)
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
  }

  void traverse();

private:
};

class test_mesh_2d_policy_t
{
public:

  flecsi_register_number_dimensions(2);
  flecsi_register_number_domains(1);

  flecsi_register_entity_types(
    flecsi_entity_type(0, 0, Vertex),
    flecsi_entity_type(1, 0, Edge),
    flecsi_entity_type(2, 0, Cell)
  );

  flecsi_register_connectivities(
    flecsi_connectivity(3, 0, Vertex, Edge),
    flecsi_connectivity(4, 0, Vertex, Cell),
    flecsi_connectivity(5, 0, Edge, Vertex),
    flecsi_connectivity(6, 0, Edge, Cell),
    flecsi_connectivity(7, 0, Cell, Vertex),
    flecsi_connectivity(8, 0, Cell, Edge)
  );

  flecsi_register_bindings();

  template<size_t M, size_t D, typename ST>
  static flecsi::topology::mesh_entity_base_t<num_domains>*
  create_entity(
    flecsi::topology::mesh_topology_base_t<ST>* mesh,
    size_t num_vertices
  )
  {
    switch(M){
      case 0:{
        switch(D){
          case 1:
            return mesh->template make<Edge>(*mesh);
          default:
            assert(false && "invalid topological dimension");
        }
        break;
      }
      default:
        assert(false && "invalid domain");
    }
  }
}; // class test_mesh_2d_t

using test_mesh_2d_t = flecsi::topology::mesh_topology_t<test_mesh_2d_policy_t>;

} // namespace supplemental
} // namespace flecsi

#endif // flecsi_execution_test_mesh_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
