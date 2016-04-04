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

#ifndef FLECSI_DOLFIN_TRIANGLE_MESH_H
#define FLECSI_DOLFIN_TRIANGLE_MESH_H

#include "flecsi/mesh/mesh_topology.h"
#include "dolfin_triangle_types.h"

/*!
 * \file dolfin_mesh.h
 * \authors ollie
 * \date Initial file creation: Mar. 31, 2016
 */

namespace flecsi
{

/*!
 * \class dolfin_triangle_mesh dolfin_triangle_mesh.h
 * \brief example mesh in Figure 1 and Figure 2 of the DOLFIN paper.
 */
template <typename mesh_type>
class dolfin_triangle_mesh_t : public mesh_topology_t<dolfin_triangle_types_t>
{
private:
  using super = mesh_topology_t<dolfin_triangle_types_t>;

public:
  auto num_vertices() const {
    return num_entities<0, 0>();
  }
  auto vertices() { return entities<0, 0>(); }
  template <typename Entity>
  auto vertices(Entity e) { return entities<0, 0>(e); }

  auto num_edges() const {
    return num_entities<1, 0>();
  }
  auto edges() { return entities<1, 0>(); }
  template <typename Entity>
  auto edges(Entity e) { return entities<1, 0>(e); }

  auto num_cells() const {
    return num_entities<2, 0>();
  }
  auto cells() { return entities<2, 0>(); }
  template <typename Entity>
  auto cells(Entity e) { return entities<2, 0>(e); }
};

}
#endif //FLECSI_DOLFIN_TRIANGLE_MESH_H
