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

#ifndef FLECSI_SAGITTAIRUS_TYPES_H_H
#define FLECSI_SAGITTAIRUS_TYPES_H_H

#include "flecsi/specializations/sagittarius/sagittarius_entity_types.h"

namespace flecsi
{
struct sagittarius_types {
  static constexpr size_t num_dimensions = 2;
  static constexpr size_t num_domains = 1;

  template<size_t D>
  using domain_ = flecsi::topology::domain_<D>;

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

  template<
    size_t M,
    size_t D
  >
  static flecsi::topology::mesh_entity_base_t<num_domains> *
  create_entity(
    flecsi::topology::mesh_topology_base_t * mesh,
    size_t num_vertices
  )
  {
    switch(M){
      case 0:{
        switch(D){
          case 1:
            return mesh->make<sagittarius_edge_t>(*mesh);
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
#endif //FLECSI_SAGITTAIRUS_TYPES_H_H
