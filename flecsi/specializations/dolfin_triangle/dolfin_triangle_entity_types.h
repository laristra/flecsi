//
// Created by ollie on 3/31/16.
//

#ifndef FLECSI_DOLFIN_TRIANGLE_ENTITY_TYPES_H
#define FLECSI_DOLFIN_TRIANGLE_ENTITY_TYPES_H

#include <flecsi/mesh/mesh_types.h>

namespace flecsi
{
// TODO: why do I have to define both a default constructor and a constructor
// taking mesh_topology_base_t???
class dolfin_vertex_t : public mesh_entity_t<0, 1> {
public:
  dolfin_vertex_t() = default;
  dolfin_vertex_t(mesh_topology_base_t*) {}
};

class dolfin_edge_t : public mesh_entity_t<1, 1> {
public:
  dolfin_edge_t() = default;
  dolfin_edge_t(mesh_topology_base_t &) {}
};

class dolfin_cell_t : public mesh_entity_t<2, 1> {
public:
  dolfin_cell_t() = default;
  dolfin_cell_t(mesh_topology_base_t&){}
  std::vector<size_t> create_entities(size_t dimension, id_t *e,
                                      id_t *v, size_t vertex_count) {
    e[0] = v[0];
    e[1] = v[1];

    e[2] = v[1];
    e[3] = v[2];

    e[4] = v[2];
    e[5] = v[0];
    return {2, 2, 2};
  }
};
}
#endif //FLECSI_DOLFIN_TRIANGLE_ENTITY_TYPES_H
