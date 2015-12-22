/*~--------------------------------------------------------------------------~*
 *  @@@@@@@@ @@       @@@@@@@@ @@     @@ @@
 * /@@///// /@@      /@@///// //@@   @@ /@@
 * /@@      /@@      /@@       //@@ @@  /@@
 * /@@@@@@@ /@@      /@@@@@@@   //@@@   /@@
 * /@@////  /@@      /@@////     @@/@@  /@@
 * /@@      /@@      /@@        @@ //@@ /@@
 * /@@      /@@@@@@@@/@@@@@@@@ @@   //@@/@@
 * //       //////// //////// //     // // 
 * 
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~--------------------------------------------------------------------------~*/

#ifndef flexi_burton_types_h
#define flexi_burton_types_h

#include "flexi/mesh/mesh_topology.h"
#include "flexi/specializations/burton/burton_mesh_traits.h"
#include "flexi/specializations/burton/burton_entity_types.h"

/*!
 * \file burton_types.h
 * \authors bergen
 * \date Initial file creation: Sep 02, 2015
 */

namespace flexi {

/*----------------------------------------------------------------------------*
 * struct burton_mesh_types_t
 *----------------------------------------------------------------------------*/

/*!
  \class burton_mesh_types_t burton_types.h
  \brief The burton_mesh_types_t is a collection of type information needed
    to specialize the flexi low-level mesh infrastructure for
    ALE methods.
*/
struct burton_mesh_types_t {

  /*--------------------------------------------------------------------------*
   * Define local traits to satisfy mesh_topology requirements.
   *--------------------------------------------------------------------------*/

  static constexpr size_t dimension = burton_mesh_traits_t::dimension;
  static constexpr size_t num_domains = burton_mesh_traits_t::num_domains;

  /*--------------------------------------------------------------------------*
   * Define basic types.
   *--------------------------------------------------------------------------*/

  using vertex_t = burton_vertex_t<num_domains>;
  using edge_t = burton_edge_t<num_domains>;

  // Cell types
  using cell_t = burton_cell_t<num_domains>;
  using quadrilateral_cell_t = burton_quadrilateral_cell_t<num_domains>;

  using wedge_t = burton_wedge_t<num_domains>;
  using corner_t = burton_corner_t<num_domains>;

  /*--------------------------------------------------------------------------*
   * Specify mesh parameterizations.
   *--------------------------------------------------------------------------*/

  using entity_types =
    std::tuple<
      std::pair<domain_<0>, vertex_t>,
      std::pair<domain_<0>, edge_t>,
      std::pair<domain_<0>, cell_t>,
      std::pair<domain_<1>, vertex_t>,
      std::pair<domain_<1>, edge_t>,
      std::pair<domain_<1>, wedge_t>
    >;

  using connectivities = 
    std::tuple<
      std::tuple<domain_<0>, vertex_t, edge_t>,
      std::tuple<domain_<0>, vertex_t, cell_t>,
      std::tuple<domain_<0>, edge_t, vertex_t>,
      std::tuple<domain_<0>, edge_t, cell_t>,
      std::tuple<domain_<0>, cell_t, vertex_t>,
      std::tuple<domain_<0>, cell_t, edge_t>,
      std::tuple<domain_<1>, vertex_t, edge_t>,
      std::tuple<domain_<1>, vertex_t, wedge_t>,
      std::tuple<domain_<1>, edge_t, vertex_t>,
      std::tuple<domain_<1>, edge_t, wedge_t>,
      std::tuple<domain_<1>, wedge_t, vertex_t>,
      std::tuple<domain_<1>, wedge_t, edge_t>
    >;

  using bindings = std::tuple<
    std::pair<domain_<0>, domain_<1>>
    >;

}; // struct burton_mesh_types_t

} // namespace flexi

#endif // flexi_burton_types_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
