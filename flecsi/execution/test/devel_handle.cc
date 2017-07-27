/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

#include <cinchdevel.h>
#include <vector>

#include "flecsi/execution/context.h"
#include "flecsi/execution/execution.h"
#include "flecsi/supplemental/coloring/add_colorings.h"
#include "flecsi/supplemental/mesh/test_mesh_2d.h"

clog_register_tag(devel_handle);

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
// Type definitions
//----------------------------------------------------------------------------//

template<
  typename T,
  size_t EP,
  size_t SP,
  size_t GP
>
using handle__ = data::legion::dense_handle_t<T, EP, SP, GP>;

template<
  typename DC,
  size_t PS
>
using client_handle__ = data_client_handle__<DC, PS>;

using vertex_t = flecsi::supplemental::vertex_t;
using cell_t = flecsi::supplemental::cell_t;
using mesh_t = flecsi::supplemental::test_mesh_2d_t;

using coloring_info_t = flecsi::coloring::coloring_info_t;
using adjacency_info_t = flecsi::coloring::adjacency_info_t;

//----------------------------------------------------------------------------//
// Variable registration
//----------------------------------------------------------------------------//

flecsi_register_data_client(mesh_t, clients, mesh);

//----------------------------------------------------------------------------//
// Initialization task
//----------------------------------------------------------------------------//

void initialize_mesh(client_handle__<mesh_t, dwd> mesh) {

  {
  clog_tag_guard(devel_handle);
  clog(info) << "initialize_mesh task" << std::endl;
  } // scope

  auto & context = execution::context_t::instance();

  auto & vertex_map = context.index_map(index_spaces::vertices);
  auto & reverse_vertex_map = context.reverse_index_map(index_spaces::vertices);
  auto & cell_map = context.index_map(index_spaces::cells);

  std::vector<vertex_t *> vertices;

  for(auto & vm: vertex_map) {
    {
    clog_tag_guard(devel_handle);
    clog(info) << "vertex: (" << vm.first << ", " << vm.second <<
      ")" << std::endl;
    } // scope

    vertices.push_back(mesh.make<vertex_t>());
  } // for

  const size_t width = 8;

  size_t count(0);
  for(auto & cm: cell_map) {
    const size_t mid = cm.second;

    const size_t row = mid/width;
    const size_t column = mid%width;

    const size_t v0 = (column    ) + (row    ) * (width + 1);
    const size_t v1 = (column + 1) + (row    ) * (width + 1);
    const size_t v2 = (column + 1) + (row + 1) * (width + 1);
    const size_t v3 = (column    ) + (row + 1) * (width + 1);

    const size_t lv0 = reverse_vertex_map[v0];
    const size_t lv1 = reverse_vertex_map[v1];
    const size_t lv2 = reverse_vertex_map[v2];
    const size_t lv3 = reverse_vertex_map[v3];

    auto c = mesh.make<cell_t>();
    mesh.init_cell<0>(c, { vertices[lv0], vertices[lv1],
      vertices[lv2], vertices[lv3] });
  } // for

  mesh.init<0>();
} // initialize_mesh

flecsi_register_task(initialize_mesh, loc, single);

//----------------------------------------------------------------------------//
// Print task
//----------------------------------------------------------------------------//

void print_mesh(client_handle__<mesh_t, dro> mesh) {
  for(auto c: mesh.cells()) {
    {
    clog_tag_guard(devel_handle);
    clog(trace) << "cell id: " << c->template id<0>() << std::endl;
    } // scope
  } // for
} // print_mesh

flecsi_register_task(print_mesh, loc, single);

//----------------------------------------------------------------------------//
// Top-Level Specialization Initialization
//----------------------------------------------------------------------------//

void specialization_tlt_init(int argc, char ** argv) {

  {
  clog_tag_guard(devel_handle);
  clog(info) << "specialization_tlt_init function" << std::endl;
  } // scope

  coloring_map_t map;
  map.vertices = index_spaces::vertices;
  map.cells = index_spaces::cells;

  flecsi_execute_mpi_task(add_colorings, map);

  auto & context = execution::context_t::instance();

  auto & vinfo = context.coloring_info(index_spaces::vertices);
  auto & cinfo = context.coloring_info(index_spaces::cells);

  adjacency_info_t ai;
  ai.index_space = index_spaces::cells_to_vertices;
  ai.from_index_space = index_spaces::cells;
  ai.to_index_space = index_spaces::vertices;
  ai.color_sizes.resize(cinfo.size());

  for(auto & itr : cinfo){
    size_t color = itr.first;
    const coloring_info_t & ci = itr.second;
    ai.color_sizes[color] = (ci.exclusive + ci.shared + ci.ghost) * 4;
  } // for

  {
  clog_tag_guard(devel_handle);
  clog(info) << "Adding adjacency info: " << ai << std::endl;
  } // scope

  context.add_adjacency(ai);

} // specialization_tlt_init

//----------------------------------------------------------------------------//
// SPMD Specialization Initialization
//----------------------------------------------------------------------------//

void specialization_spmd_init(int argc, char ** argv) {

  {
  clog_tag_guard(devel_handle);
  clog(info) << "specialization_spmd_init function" << std::endl;
  } // scope

  auto mh = flecsi_get_client_handle(mesh_t, clients, mesh);

  flecsi_execute_task(initialize_mesh, single, mh);

} // specialization_spmd_ini

//----------------------------------------------------------------------------//
// User driver.
//----------------------------------------------------------------------------//

void driver(int argc, char ** argv) {
} // driver

} // namespace execution
} // namespace flecsi

DEVEL(devel_handle) {}

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
