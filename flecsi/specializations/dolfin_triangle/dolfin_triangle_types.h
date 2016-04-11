//
// Created by ollie on 3/31/16.
//

#ifndef FLECSI_DOLFIN_TRIANGLE_TYPES_H
#define FLECSI_DOLFIN_TRIANGLE_TYPES_H

#include "dolfin_triangle_entity_types.h"

namespace flecsi
{
struct dolfin_triangle_types_t {
  static constexpr size_t dimension = 2;
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
};
}
#endif //FLECSI_DOLFIN_TRIANGLE_TYPES_H
