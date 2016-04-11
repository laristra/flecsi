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

#ifndef FLECSI_SAGITTARIUS_MESH_H
#define FLECSI_SAGITTARIUS_MESH_H

#include <flecsi/mesh/mesh_topology.h>
#include "sagittarius_types.h"

namespace flecsi
{
template <typename mesh_type>
class sagittarius_mesh_t : public mesh_topology_t<mesh_type>
{
private:
  using super = mesh_topology_t<mesh_type>;

public:
  auto num_vertices() const {
    return super::template num_entities<0, 0>();
  }
  auto vertices() {
    return super::template entities<0, 0>();
  }
  template <typename Entity>
  auto vertices(Entity e) {
    return super::template entities<0, 0>(e);
  }

  auto num_edges() const {
    return super::template num_entities<1, 0>();
  }
  auto edges() {
    return super::template entities<1, 0>();
  }
  template <typename Entity>
  auto edges(Entity e) {
    return super::template entities<1, 0>(e);
  }

  auto num_cells() const {
    return super::template num_entities<2, 0>();
  }
  auto cells() {
    return super::template entities<2, 0>();
  }
  template <typename Entity>
  auto cells(Entity e) {
    return super::template entities<2, 0>(e);
  }
};
}
#endif //FLECSI_SAGITTARIUS_MESH_H
