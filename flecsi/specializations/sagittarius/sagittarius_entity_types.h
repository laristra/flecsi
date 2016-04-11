/*~--------------------------------------------------------------------------~*
 *  @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
 * /@@/////  /@@          @@////@@ @@////// /@@
 * /@@       /@@  @@@@@  @@    // /@@       /@@
 * /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
 * /@@////   /@@/@@@@@@@/@@       ////////@@/@@
 * /@@       /@@/@@//// //@@    @@       /@@/@@
 * /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
 * //       ///  //////   //////  ////////  //
 *
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~--------------------------------------------------------------------------~*/

#ifndef FLECSI_SAGITTARIUS_ENTITY_TYPES_H
#define FLECSI_SAGITTARIUS_ENTITY_TYPES_H

#include <flecsi/mesh/mesh_types.h>

namespace flecsi
{

class sagittarius_vertex_t : public mesh_entity_t<0, 1> {
public:
  sagittarius_vertex_t() = default;
  sagittarius_vertex_t(mesh_topology_base_t&) {}
};

class sagittarius_edge_t : public mesh_entity_t<1, 1> {
public:
  sagittarius_edge_t() = default;
  sagittarius_edge_t(mesh_topology_base_t&) {}
};

// abstrace class
class sagittarius_cell_t : public mesh_entity_t<2, 1> {
public:
  sagittarius_cell_t() = default;
  sagittarius_cell_t(mesh_topology_base_t&) {}
  // FIXME: can we make it a pure virtual function?
  virtual std::vector<size_t>
    create_entities(size_t dimension, id_t *entities,
                    id_t *vertices, size_t vertex_count) {};
};

class sagittarius_quad_t : public sagittarius_cell_t {
public:
  sagittarius_quad_t() = default;
  sagittarius_quad_t(mesh_topology_base_t&) {}

  std::vector<size_t>
  create_entities(size_t dimension, id_t *e,
                  id_t *v, size_t vertex_count) override {
    e[0] = v[0];
    e[1] = v[1];

    e[2] = v[1];
    e[3] = v[2];

    e[4] = v[2];
    e[5] = v[3];

    e[6] = v[3];
    e[7] = v[0];

    return {2, 2, 2, 2};
  }
};

class sagittarius_triangle_t : public  sagittarius_cell_t {
public:
  sagittarius_triangle_t() = default;
  sagittarius_triangle_t(mesh_topology_base_t&) {}

  std::vector<size_t>
  create_entities(size_t dimension, id_t *e,
                  id_t *v, size_t vertex_count) override {
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
#endif //FLECSI_SAGITTARIUS_ENTITY_TYPES_H
