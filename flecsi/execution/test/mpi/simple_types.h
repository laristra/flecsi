//
// Created by ollie on 3/17/17.
//

#ifndef FLECSI_SIMPLE_TYPES_H
#define FLECSI_SIMPLE_TYPES_H

#include "simple_entity_types.h"

struct simple_types_t
{
  //--------------------------------------------------------------------------//
  // Define local traits to satisfy mesh_topology requirements.
  //--------------------------------------------------------------------------//

  /// The dimension of the mesh
  static constexpr size_t num_dimensions = 2;

  /// The number of domains
  static constexpr size_t num_domains = 1;


  //--------------------------------------------------------------------------//
  // Define basic types.
  //--------------------------------------------------------------------------//

  /// Mesh vertex type
  using vertex_t = simple_vertex_t;

  /// Mesh cell type
  using cell_t = simple_cell_t;

  /// Convenience type
  template<size_t D>
  using domain_ = flecsi::topology::domain_<D>;

  ///
  // Definitions of burton mesh entities and their domain.
  // clang-format off
  ///
  using entity_types =
  std::tuple<
  std::pair<domain_<0>, vertex_t>,
  std::pair<domain_<0>, cell_t>
  >;

  ///
  // Connectivities are adjacencies of entities within a single domain.
  ///
  using connectivities =
  std::tuple<
  std::tuple<domain_<0>, vertex_t, cell_t>,
  std::tuple<domain_<0>, cell_t, vertex_t>,
  std::tuple<domain_<0>, cell_t, cell_t>
  >;

  ///
  // Bindings are adjacencies of entities across two domains.
  ///
  using bindings = std::tuple<>;

  //-------------------------------------------------------------------------//
  //
  //-------------------------------------------------------------------------//

  ///
  // \tparam M The topological domain.
  // \tparam D The topological dimension for which to create an entity.
  ///
  template<
    size_t M,
    size_t D,
      typename ST
  >
  static flecsi::topology::mesh_entity_base_t<num_domains> *
  create_entity(
    flecsi::topology::mesh_topology_base_t<ST> * mesh,
    size_t num_vertices
  )
  {
    switch(M) {

      case 0:
      {
        switch(D) {
          case 1:
            //return mesh->make<edge_t>(*mesh);

          default:
            assert(false && "invalid topological dimension");
        } // switch

        break;
      } // case

      default:
        assert(false && "invalid domain");

    } // switch
  } // create_entity

}; // class minimal_types_t

#endif //FLECSI_SIMPLE_TYPES_H
