/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

#pragma once

#include <flecsi/execution/context.h>
#include <flecsi/execution/execution.h>
#include <flecsi/supplemental/coloring/add_colorings.h>
#include <flecsi/supplemental/mesh/test_mesh_2d.h>
#include <flecsi/data/dense_accessor.h>

clog_register_tag(harness);

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
// Type definitions
//----------------------------------------------------------------------------//

using point_t = flecsi::supplemental::point_t;
using vertex_t = flecsi::supplemental::vertex_t;
using cell_t = flecsi::supplemental::cell_t;
using mesh_t = flecsi::supplemental::test_mesh_2d_t;

using coloring_info_t = flecsi::coloring::coloring_info_t;
using adjacency_info_t = flecsi::coloring::adjacency_info_t;

template<
  size_t PS
>
using mesh = data_client_handle_u<mesh_t, PS>;

template<
  size_t EP,
  size_t SP,
  size_t GP
>
using field = dense_accessor<double, EP, SP, GP>;

//----------------------------------------------------------------------------//
// Create a mesh
//----------------------------------------------------------------------------//

flecsi_register_data_client(mesh_t, clients, m);

//----------------------------------------------------------------------------//
// Initialization task
//----------------------------------------------------------------------------//

inline void initialize_mesh(mesh<wo> m) {

  {
  clog_tag_guard(harness);
  clog(info) << "initialize_mesh task" << std::endl;
  } // scope

  auto & context { execution::context_t::instance() };

  auto & vertex_map { context.index_map(index_spaces::vertices) };
  auto & reverse_vertex_map
    { context.reverse_index_map(index_spaces::vertices) };
  auto & cell_map { context.index_map(index_spaces::cells) };

  std::vector<vertex_t *> vertices;

  const size_t width { 8 };
  const double dt { 1.0/width };

  for(auto & vm: vertex_map) {
    const size_t mid { vm.second };
    const size_t row { mid/(width+1) };
    const size_t column { mid%(width+1) };

    point_t p { column*dt, row*dt };
    vertices.push_back(m.make<vertex_t>(p));
  } // for

  size_t count{0};
  for(auto & cm: cell_map) {
    const size_t mid { cm.second };

    const size_t row { mid/width };
    const size_t column { mid%width };

    const size_t v0 { (column    ) + (row    ) * (width + 1) };
    const size_t v1 { (column + 1) + (row    ) * (width + 1) };
    const size_t v2 { (column + 1) + (row + 1) * (width + 1) };
    const size_t v3 { (column    ) + (row + 1) * (width + 1) };

    const size_t lv0 { reverse_vertex_map[v0] };
    const size_t lv1 { reverse_vertex_map[v1] };
    const size_t lv2 { reverse_vertex_map[v2] };
    const size_t lv3 { reverse_vertex_map[v3] };

    auto c { m.make<cell_t>() };
    m.init_cell<0>(c, { vertices[lv0], vertices[lv1],
      vertices[lv2], vertices[lv3] });
  } // for

  m.init<0>();

} // initialize_mesh

flecsi_register_task(initialize_mesh, flecsi::execution, loc, single);

//----------------------------------------------------------------------------//
// Top-Level Specialization Initialization
//----------------------------------------------------------------------------//

void specialization_tlt_init(int argc, char ** argv);

//----------------------------------------------------------------------------//
// SPMD Specialization Initialization
//----------------------------------------------------------------------------//

void specialization_spmd_init(int argc, char ** argv);

} // namespace execution
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
