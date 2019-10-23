/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */

#pragma once

#include <array>
#include <iostream>
#include <vector>

#include <flecsi/topology/mesh_types.h>

namespace flecsi {
namespace tutorial {

//----------------------------------------------------------------------------//
// Point type.
//----------------------------------------------------------------------------//

using point_t = std::array<double, 2>;

//----------------------------------------------------------------------------//
// Vertex type.
//----------------------------------------------------------------------------//

struct vertex_t : public flecsi::topology::mesh_entity_u<0, 1> {
  static constexpr size_t domain = 0;

  vertex_t(point_t & p) : p_(p) {}

  point_t const & coordinates() const {
    return p_;
  }

  void print(const char * string) {
    std::cout << string << " My id is " << id() << std::endl;
  } // print

private:
  point_t p_;

}; // struct vertex_t

//----------------------------------------------------------------------------//
// Edge type.
//----------------------------------------------------------------------------//

struct edge_t : public flecsi::topology::mesh_entity_u<1, 1> {
  static constexpr size_t domain = 0;
}; // struct edge_t

//----------------------------------------------------------------------------//
// Cell type.
//----------------------------------------------------------------------------//

struct cell_t : public flecsi::topology::mesh_entity_u<2, 1> {
  using id_t = flecsi::utils::id_t;
  static constexpr size_t domain = 0;

  void print(const char * string) {
    std::cout << string << " My id is " << id() << std::endl;
  } // print

  std::vector<size_t> create_entities(id_t cell_id,
    size_t dim,
    flecsi::topology::domain_connectivity_u<2> & c,
    id_t * e) {
    id_t * v = c.get_entities(cell_id, 0);

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

} // namespace tutorial
} // namespace flecsi
