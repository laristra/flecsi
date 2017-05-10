/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_test_mesh_h
#define flecsi_execution_test_mesh_h

///
/// \file
/// \date Initial file creation: May 10, 2017
///

#include "flecsi/topology/mesh_topology.h"

namespace flecsi {
namespace topology {

class Vertex : public mesh_entity_t<0, 1>{
public:
  template<size_t M>
  uint64_t precedence() const { return 0; }
  Vertex() = default;
  Vertex(mesh_topology_base_t &) {}

};

class Edge : public mesh_entity_t<1, 1>{
public:
  Edge(mesh_topology_base_t &) {}
};

class Face : public mesh_entity_t<1, 1>{
public:
  Face(mesh_topology_base_t &) {}

};

class Cell : public mesh_entity_t<2, 1>{
public:

  using id_t = flecsi::utils::id_t;

  Cell(mesh_topology_base_t& mesh)
  : mesh_(mesh){}

  void set_precedence(size_t dim, uint64_t precedence) {}

  std::vector<size_t>
  create_entities(id_t cell_id, size_t dim,
    domain_connectivity<2> & c, id_t * e)
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
  mesh_topology_base_t& mesh_;
};

class test_mesh_2d_policy_t {
public:
  static constexpr size_t num_dimensions = 2;

  static constexpr size_t num_domains = 1;

  using entity_types = std::tuple<
    std::pair<domain_<0>, Vertex>,
    std::pair<domain_<0>, Edge>,
    std::pair<domain_<0>, Cell>>;

  using connectivities =
    std::tuple<std::tuple<domain_<0>, Vertex, Edge>,
               std::tuple<domain_<0>, Vertex, Cell>,
               std::tuple<domain_<0>, Edge, Vertex>,
               std::tuple<domain_<0>, Edge, Cell>,
               std::tuple<domain_<0>, Cell, Vertex>,
               std::tuple<domain_<0>, Cell, Edge>>;

  using bindings = std::tuple<>;

  template<size_t M, size_t D>
  static mesh_entity_base_t<num_domains>*
  create_entity(mesh_topology_base_t* mesh, size_t num_vertices){
    switch(M){
      case 0:{
        switch(D){
          case 1:
            return mesh->make<Edge>(*mesh);
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

using test_mesh_2d_t = mesh_topology_t<test_mesh_2d_policy_t>;

} // namespace topology
} // namespace flecsi

#endif // flecsi_execution_test_mesh_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
