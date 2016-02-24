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

#include "flecsi/specializations/burton/burton_types.h"
#include "flecsi/mesh/mesh_topology.h"

namespace flecsi {

using point_t = burton_cell_t::point_t;

/*----------------------------------------------------------------------------*
 * burton_edge_t
 *----------------------------------------------------------------------------*/

point_t burton_edge_t::midpoint()
  {
    auto & mesh = static_cast<mesh_topology_t<burton_mesh_types_t> &>(mesh_);
    auto vs = mesh.entities<0,0>(this).to_vec();

    return point_t{0.5*(vs[0]->coordinates() + vs[1]->coordinates())};
  } // burton_edge_t::midpoint

/*----------------------------------------------------------------------------*
 * burton_quadrilateral_cell_t
 *----------------------------------------------------------------------------*/

using real_t = burton_mesh_traits_t::real_t;

real_t burton_quadrilateral_cell_t::area()
{
  auto & mesh = static_cast<mesh_topology_t<burton_mesh_types_t> &>(mesh_);
  auto vs = mesh.entities<0,0>(this).to_vec();

  using vector_t = burton_mesh_traits_t::vector_t;

  vector_t A = point_to_vector(vs[1]->coordinates() - vs[0]->coordinates());
  vector_t B = point_to_vector(vs[3]->coordinates() - vs[0]->coordinates());
  vector_t C = point_to_vector(vs[1]->coordinates() - vs[2]->coordinates());
  vector_t D = point_to_vector(vs[3]->coordinates() - vs[2]->coordinates());

  return 0.5*(cross_magnitude(A,B) + cross_magnitude(C,D));
} // burton_quadrilateral_cell_t::area

point_t burton_quadrilateral_cell_t::centroid()
  {
    auto & mesh = static_cast<mesh_topology_t<burton_mesh_types_t> &>(mesh_);
    auto vs = mesh.entities<0,0>(this).to_vec();

    point_t tmp{0,0};

    tmp += vs[0]->coordinates();
    tmp += vs[1]->coordinates();
    tmp += vs[2]->coordinates();
    tmp += vs[3]->coordinates();

    return 0.25*tmp;
  } // burton_quadrilateral_cell_t::centroid

} // namespace flecsi

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
