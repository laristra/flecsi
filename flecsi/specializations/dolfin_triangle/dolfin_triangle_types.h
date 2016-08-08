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

#ifndef FLECSI_DOLFIN_TRIANGLE_TYPES_H
#define FLECSI_DOLFIN_TRIANGLE_TYPES_H

#include "flecsi/specializations/dolfin_triangle/dolfin_triangle_entity_types.h"

namespace flecsi
{
struct dolfin_triangle_types_t {
  static constexpr size_t num_dimensions = 2;
  static constexpr size_t num_domains = 1;

  // FIXME: what exactly is a DOMAIN?
  using entity_types = std::tuple<
    std::pair<domain_<0>, dolfin_vertex_t>,
    std::pair<domain_<0>, dolfin_edge_t>,
    std::pair<domain_<0>, dolfin_cell_t>
  >;

  // TODO: vertex->vertex, edge->edge and cell->cell
  using connectivities = std::tuple<
    std::tuple<domain_<0>, dolfin_vertex_t, dolfin_vertex_t>,
    std::tuple<domain_<0>, dolfin_vertex_t, dolfin_edge_t>,
    std::tuple<domain_<0>, dolfin_vertex_t, dolfin_cell_t>,
    std::tuple<domain_<0>, dolfin_edge_t, dolfin_vertex_t>,
    std::tuple<domain_<0>, dolfin_edge_t, dolfin_edge_t>,
    std::tuple<domain_<0>, dolfin_edge_t, dolfin_cell_t>,
    std::tuple<domain_<0>, dolfin_cell_t, dolfin_vertex_t>,
    std::tuple<domain_<0>, dolfin_cell_t, dolfin_edge_t>,
    std::tuple<domain_<0>, dolfin_cell_t, dolfin_cell_t>
  >;

  using bindings = std::tuple<>;

  template<size_t M, size_t D>
  static mesh_entity_base_t<num_domains>*
  create_entity(mesh_topology_base_t* mesh, size_t num_vertices){
    switch(M){
      case 0:{
        switch(D){
          case 1:
            return mesh->make<dolfin_edge_t>(*mesh);
          default:
            assert(false && "invalid topological dimension");
        }
        break;
      }
      default:
        assert(false && "invalid domain");
    }
  }
};
}
#endif //FLECSI_DOLFIN_TRIANGLE_TYPES_H
