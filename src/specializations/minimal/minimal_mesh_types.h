/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flexi_minimal_mesh_types_h
#define flexi_minimal_mesh_types_h

#include "flexi/specializations/minimal/minimal_mesh_traits.h"
#include "flexi/specializations/minimal/minimal_entity_types.h"

/*!
 * \file minimal_mesh_types.h
 * \authors bergen
 * \date Initial file creation: Dec 26, 2015
 */

namespace flexi {

struct minimal_mesh_types_t {

  static constexpr size_t dimension = minimal_mesh_traits_t::dimension;
  static constexpr size_t num_domains = minimal_mesh_traits_t::num_domains;

  using vertex_t = minimal_vertex_t;
  using edge_t = minimal_edge_t;
  using face_t = minimal_face_t;
  using cell_t = minimal_cell_t;

  using entity_types =
    std::tuple<
      std::pair<domain_<0>, vertex_t>,
      std::pair<domain_<0>, edge_t>,
      std::pair<domain_<0>, face_t>,
      std::pair<domain_<0>, cell_t>,
      std::pair<domain_<1>, vertex_t>
    >;

  using connectivities =
    std::tuple<
      std::tuple<domain_<0>, vertex_t, edge_t>,
      std::tuple<domain_<0>, vertex_t, cell_t>
    >;

  using bindings =
    std::tuple<
      std::tuple<domain_<0>, domain_<1>, cell_t, vertex_t>
    >;

}; // struct minimal_mesh_types

} // namespace flexi

#endif // flexi_minimal_mesh_types_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
