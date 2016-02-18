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

#ifndef flecsi_burton_types_h
#define flecsi_burton_types_h

#include "flecsi/mesh/mesh_topology.h"
#include "flecsi/specializations/burton/burton_mesh_traits.h"
#include "flecsi/specializations/burton/burton_entity_types.h"

/*!
 * \file burton_types.h
 * \authors bergen
 * \date Initial file creation: Sep 02, 2015
 */

namespace flecsi
{
/*----------------------------------------------------------------------------*
 * struct burton_mesh_types_t
 *----------------------------------------------------------------------------*/

/*!
  \class burton_mesh_types_t burton_types.h
  \brief A collection of type information needed to specialize the flecsi
    low-level mesh infrastructure for ALE methods.
*/
struct burton_mesh_types_t {
  /*--------------------------------------------------------------------------*
   * Define local traits to satisfy mesh_topology requirements.
   *--------------------------------------------------------------------------*/

  //! The dimension of the burton mesh picked up from burton_mesh_traits_t.
  static constexpr size_t dimension = burton_mesh_traits_t::dimension;

  //! The number of domains in burton mesh picked up from burton_mesh_traits_t.
  static constexpr size_t num_domains = burton_mesh_traits_t::num_domains;

  /*--------------------------------------------------------------------------*
   * Define basic types.
   *--------------------------------------------------------------------------*/

  //! Type for burton mesh vertices.
  using vertex_t = burton_vertex_t;

  //! Type for burton mesh edges.
  using edge_t = burton_edge_t;

  // Cell types
  //! Type for burton mesh cells.
  using cell_t = burton_cell_t;

  //! Type for burton mesh quadrilateral cells.
  using quadrilateral_cell_t = burton_quadrilateral_cell_t;

  //! Type for burton mesh corners.
  using corner_t = burton_corner_t;

  //! Type for burton mesh wedges.
  using wedge_t = burton_wedge_t;

  /*--------------------------------------------------------------------------*
   * Specify mesh parameterizations.
   *--------------------------------------------------------------------------*/

  //! Definitions of burton mesh entities and their domain.
  // clang-format off
  using entity_types =
      std::tuple<
        std::pair<domain_<0>, vertex_t>,
        std::pair<domain_<0>, edge_t>,
        std::pair<domain_<0>, cell_t>,
#if TMP_WEDGES
        std::pair<domain_<1>, wedge_t>,
#endif
        std::pair<domain_<1>, corner_t>
      >;

  //! Connectivities are adjacencies of entities within a single domain.
  using connectivities =
    std::tuple<
      std::tuple<domain_<0>, vertex_t, edge_t>,
      std::tuple<domain_<0>, vertex_t, cell_t>,
      std::tuple<domain_<0>, edge_t, vertex_t>,
      std::tuple<domain_<0>, edge_t, cell_t>,
      std::tuple<domain_<0>, cell_t, vertex_t>,
      std::tuple<domain_<0>, cell_t, edge_t>
      >;

  //! Bindings are adjacencies of entities across two domains.
  using bindings =
      std::tuple<

#if TMP_WEDGES
        std::tuple<domain_<0>, domain_<1>, cell_t, wedge_t>,
        std::tuple<domain_<0>, domain_<1>, edge_t, wedge_t>,
        std::tuple<domain_<0>, domain_<1>, vertex_t, wedge_t>,
#endif

        std::tuple<domain_<0>, domain_<1>, cell_t, corner_t>,
        std::tuple<domain_<0>, domain_<1>, vertex_t, corner_t>,

#if TMP_WEDGES
        std::tuple<domain_<1>, domain_<0>, wedge_t, cell_t>,
        std::tuple<domain_<1>, domain_<0>, wedge_t, edge_t>,
        std::tuple<domain_<1>, domain_<0>, wedge_t, vertex_t>,
#endif

        std::tuple<domain_<1>, domain_<0>, corner_t, cell_t>,
        std::tuple<domain_<1>, domain_<0>, corner_t, edge_t>,
        std::tuple<domain_<1>, domain_<0>, corner_t, vertex_t>
      >;
  // clang-format on

}; // struct burton_mesh_types_t

} // namespace flecsi

#endif // flecsi_burton_types_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
