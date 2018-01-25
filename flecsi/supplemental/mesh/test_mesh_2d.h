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

/*! @file */

#include <flecsi/topology/mesh.h>
#include <flecsi/topology/mesh_topology.h>

//----------------------------------------------------------------------------//
// Enumeration to name index spaces
//----------------------------------------------------------------------------//

enum index_spaces : size_t
{
  vertices,
  edges,
  cells,
  cells_to_vertices
}; // enum index_spaces

namespace flecsi {
namespace supplemental {

//----------------------------------------------------------------------------//
// Entity types
//----------------------------------------------------------------------------//

using point_t = std::array<double, 2>;

struct vertex_t : public flecsi::topology::mesh_entity__<0, 1>
{
  vertex_t(point_t & p) : p_(p) {} 

  point_t const & coordinates() const { return p_; }

private:

  point_t p_;

}; // struct vertex_t

struct edge_t : public flecsi::topology::mesh_entity__<1, 1>
{
}; // struct edge_t

struct face_t : public flecsi::topology::mesh_entity__<1, 1>
{
}; // struct face_t

struct cell_t : public flecsi::topology::mesh_entity__<2, 1>
{
  using id_t = flecsi::utils::id_t;

  std::vector<size_t>
  create_entities(
    id_t cell_id,
    size_t dim,
    flecsi::topology::domain_connectivity__<2> & c,
    id_t * e
  )
  {
    id_t* v = c.get_entities(cell_id, 0);

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

//----------------------------------------------------------------------------//
// Mesh policy
//----------------------------------------------------------------------------//

struct test_mesh_2d_policy_t
{
  using id_t = flecsi::utils::id_t;

  flecsi_register_number_dimensions(2);
  flecsi_register_number_domains(1);

  flecsi_register_entity_types(
    flecsi_entity_type(index_spaces::vertices, 0, vertex_t),
//    flecsi_entity_type(index_spaces::edges, 0, edge_t),
    flecsi_entity_type(index_spaces::cells, 0, cell_t)
  );

  flecsi_register_connectivities(
    flecsi_connectivity(index_spaces::cells_to_vertices, 0, cell_t, vertex_t)
  );

  flecsi_register_bindings();

#ifdef FLECSI_TEST_MESH_INDEX_SUBSPACES
  using index_subspaces = std::tuple<
    std::tuple<flecsi::topology::index_space_<0>,
               flecsi::topology::index_subspace_<0>>
    >;
#endif

  template<
    size_t M,
    size_t D,
    typename ST
  >
  static flecsi::topology::mesh_entity_base__<num_domains> *
  create_entity(
    flecsi::topology::mesh_topology_base__<ST>* mesh,
    size_t num_vertices,
    id_t const & id
  )
  {
#if 0
    switch(M) {

      case 0:
      {
        switch(D) {
          case 1:
            return mesh->template make<edge_t>(*mesh);
          default:
            assert(false && "invalid topological dimension");
        } // switch

        break;
      } // scope

      default:
        assert(false && "invalid domain");
    } // switch
#endif
    return nullptr;
  } // create_entity

}; // struct test_mesh_2d_t

//----------------------------------------------------------------------------//
// Mesh type
//----------------------------------------------------------------------------//

struct test_mesh_2d_t :
  public flecsi::topology::mesh_topology__<test_mesh_2d_policy_t>
{

  auto
  cells() {
    return entities<2, 0>();
  } // cells

  auto
  cells(flecsi::partition_t p) {
    return entities<2, 0>(p);
  } // cells

  template<
    typename E,
    size_t M
  >
  auto
  vertices( 
    flecsi::topology::domain_entity__<M, E> & e
  )
  {
    return entities<0, 0>(e);
  } // vertices
/*
  template<
    typename E,
    size_t M
  >
  auto
  vertices( 
    flecsi::topology::domain_entity__<M, E> & e
  )
  const
  {
    return entities<0, 0>(e);
  } // vertices
*/

  auto
  vertices() {
    return entities<0, 0>();
  } // cells

}; // struct test_mesh_2d_t

} // namespace supplemental
} // namespace flecsi
