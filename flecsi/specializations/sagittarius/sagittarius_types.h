//
// Created by ollie on 4/5/16.
//

#ifndef FLECSI_SAGITTAIRUS_TYPES_H_H
#define FLECSI_SAGITTAIRUS_TYPES_H_H

#include "sagittarius_entity_types.h"

namespace flecsi
{
struct sagittarius_types {
  static constexpr size_t dimension = 2;
  static constexpr size_t num_domains = 1;

  using entity_types = std::tuple<
    std::pair<domain_<0>, sagittarius_vertex_t>,
    std::pair<domain_<0>, sagittarius_edge_t>,
    std::pair<domain_<0>, sagittarius_cell_t>
  >;

  using connectivities = std::tuple<
    std::tuple<domain_<0>, sagittarius_vertex_t, sagittarius_vertex_t>,
    std::tuple<domain_<0>, sagittarius_vertex_t, sagittarius_edge_t>,
    std::tuple<domain_<0>, sagittarius_vertex_t, sagittarius_cell_t>,
    std::tuple<domain_<0>, sagittarius_edge_t, sagittarius_vertex_t>,
    std::tuple<domain_<0>, sagittarius_edge_t, sagittarius_edge_t>,
    std::tuple<domain_<0>, sagittarius_edge_t, sagittarius_cell_t>,
    std::tuple<domain_<0>, sagittarius_cell_t, sagittarius_vertex_t>,
    std::tuple<domain_<0>, sagittarius_cell_t, sagittarius_edge_t>,
    std::tuple<domain_<0>, sagittarius_cell_t, sagittarius_cell_t>
  >;

  using bindings = std::tuple<>;
};
}
#endif //FLECSI_SAGITTAIRUS_TYPES_H_H
