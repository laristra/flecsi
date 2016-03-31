//
// Created by ollie on 3/31/16.
//

#ifndef FLECSI_DOLFIN_TRIANGLE_ENTITY_TYPES_H
#define FLECSI_DOLFIN_TRIANGLE_ENTITY_TYPES_H

#include <flecsi/mesh/mesh_types.h>

namespace flecsi
{

class dolfin_vertex_t : public mesh_entity_t<0, 1> {};
class dolfin_edge_t : public mesh_entity_t<1, 1> {};
class dolfin_cell_t : public mesh_entity_t<2, 1> {
  std::vector<size_t> create_entities(size_t dimension, id_t *e,
                                      id_t *v, size_t vertex_count) {
    e[0] = v[0];
    e[1] = v[1];

    e[2] = v[1];
    e[3] = v[2];

    e[4] = v[2];
    e[5] = v[0];
  }
};
}
#endif //FLECSI_DOLFIN_TRIANGLE_ENTITY_TYPES_H
