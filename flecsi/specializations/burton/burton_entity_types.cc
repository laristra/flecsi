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

#define burton_topology(m) \
  static_cast<mesh_topology_t<burton_mesh_types_t> &>((m))

using real_t = burton_mesh_traits_t::real_t;
using vector_t = burton_mesh_traits_t::vector_t;
using point_t = burton_mesh_traits_t::point_t;

/*----------------------------------------------------------------------------*
 * burton_vertex_t
 *----------------------------------------------------------------------------*/

void burton_vertex_t::set_coordinates(const point_t & coordinates)
	{
    auto & topology = burton_topology(mesh_);
		auto c = data_t::instance().dense_accessor<point_t, flecsi_internal>(
			"coordinates", topology.runtime_id());
		c[mesh_entity_base_t<num_domains>::template id<0>()] = coordinates;
	} // burton_vertex_t::set_coordinates

const point_t & burton_vertex_t::coordinates() const
	{
    auto & topology = burton_topology(mesh_);
		const auto c = data_t::instance().dense_accessor<point_t, flecsi_internal>(
		  "coordinates", topology.runtime_id());
		return c[mesh_entity_base_t<num_domains>::template id<0>()];
	} // burton_vertex_t::coordinates

/*----------------------------------------------------------------------------*
 * burton_edge_t
 *----------------------------------------------------------------------------*/

point_t burton_edge_t::midpoint() const
  {
    auto & topology = burton_topology(mesh_);
    auto vs = topology.entities<0,0>(this).to_vec();

    return point_t{0.5*(vs[0]->coordinates() + vs[1]->coordinates())};
  } // burton_edge_t::midpoint

real_t burton_edge_t::length() const
  {
    auto & topology = burton_topology(mesh_);
    auto vs = topology.entities<0,0>(this).to_vec();

    auto & a = vs[0]->coordinates();
    auto & b = vs[1]->coordinates();
    
    return std::sqrt( pow(a[0]-b[0],2) + pow(a[1]-b[1],2) );
  } // burton_edge_t::length

vector_t burton_edge_t::normal() const
  {
    auto & topology = burton_topology(mesh_);
    auto vs = topology.entities<0,0>(this).to_vec();

    auto & a = vs[0]->coordinates();
    auto & b = vs[1]->coordinates();

    return { a[1] - b[1], b[0] - a[0] };
  } // burton_edge_t::normal

real_t burton_quadrilateral_cell_t::area() const
  {
    auto & topology = burton_topology(mesh_);
    auto vs = topology.entities<0,0>(this).to_vec();

    using vector_t = burton_mesh_traits_t::vector_t;

    vector_t A = point_to_vector(vs[1]->coordinates() - vs[0]->coordinates());
    vector_t B = point_to_vector(vs[3]->coordinates() - vs[0]->coordinates());
    vector_t C = point_to_vector(vs[1]->coordinates() - vs[2]->coordinates());
    vector_t D = point_to_vector(vs[3]->coordinates() - vs[2]->coordinates());

    return 0.5*(cross_magnitude(A,B) + cross_magnitude(C,D));
  } // burton_quadrilateral_cell_t::area

/*----------------------------------------------------------------------------*
 * burton_quadrilateral_cell_t
 *----------------------------------------------------------------------------*/

point_t burton_quadrilateral_cell_t::centroid() const
  {
    auto & topology = burton_topology(mesh_);
    auto vs = topology.entities<0,0>(this).to_vec();

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
