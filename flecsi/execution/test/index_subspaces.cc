/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

#include <cinchdevel.h>

#include <flecsi/execution/context.h>
#include <flecsi/execution/execution.h>
#include <flecsi/supplemental/coloring/add_colorings.h>

#define FLECSI_TEST_MESH_INDEX_SUBSPACES 1
#include <flecsi/supplemental/mesh/test_mesh_2d.h>

#include <flecsi/data/dense_accessor.h>

clog_register_tag(devel_handle);

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
using mesh = data_client_handle__<mesh_t, PS>;

template<
  size_t EP,
  size_t SP,
  size_t GP
>
using field = dense_accessor<double, EP, SP, GP>;

//----------------------------------------------------------------------------//
// Variable registration
//----------------------------------------------------------------------------//

flecsi_register_data_client(mesh_t, clients, m);
flecsi_register_field(mesh_t, data, pressure, double, dense,
  1, index_spaces::cells);

//----------------------------------------------------------------------------//
// Initialization task
//----------------------------------------------------------------------------//

void initialize_mesh(mesh<wo> m) {

  {
  clog_tag_guard(devel_handle);
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
// Initialize pressure
//----------------------------------------------------------------------------//

void initialize_pressure(mesh<ro> m, field<rw, rw, ro> p) {
  size_t count{0};

  auto & context { execution::context_t::instance() };

  for(auto c: m.cells(owned)) {
    p(c) = (context.color() + 1)*1000.0 + count++;
  } // for

} // initialize_pressure

flecsi_register_task(initialize_pressure, flecsi::execution, loc, single);

//----------------------------------------------------------------------------//
// Update pressure
//----------------------------------------------------------------------------//

void update_pressure(mesh<ro> m, field<rw, rw, ro> p) {
  size_t count{0};

  for(auto c: m.cells(owned)) {
    p(c) = 2.0*p(c);
  } // for

} // initialize_pressure

flecsi_register_task(update_pressure, flecsi::execution, loc, single);

//----------------------------------------------------------------------------//
// Print task
//----------------------------------------------------------------------------//

void print_mesh(mesh<ro> m, field<ro, ro, ro> p) {
  {
  clog_tag_guard(devel_handle);
  clog(info) << "print_mesh task" << std::endl;
  } // scope

  auto & context = execution::context_t::instance();
  auto & vertex_map = context.index_map(index_spaces::vertices);
  auto & cell_map = context.index_map(index_spaces::cells);

  for(auto c: m.cells(owned)) {
    const size_t cid = c->template id<0>();

    {
    clog_tag_guard(devel_handle);
    clog(trace) << "color: " << context.color() << " cell id: (" <<
      cid << ", " << cell_map[cid] << ")" << std::endl;
    clog(trace) << "color: " << context.color() << " pressure: " <<
      p(c) << std::endl;
    } // scope

    size_t vcount(0);
    for(auto v: m.vertices(c)) {
      const size_t vid = v->template id<0>();

      {
      clog_tag_guard(devel_handle);
      clog(trace) << "color: " << context.color() << " vertex id: (" <<
        vid << ", " << vertex_map[vid] << ") " << vcount << std::endl;

      point_t coord = v->coordinates();
      clog(trace) << "color: " << context.color() << " coordinates: (" <<
        coord[0] << ", " << coord[1] << ")" << std::endl;
      } // scope

      vcount++;
    } // for
  } // for
} // print_mesh

flecsi_register_task(print_mesh, flecsi::execution, loc, single);

//----------------------------------------------------------------------------//
// Top-Level Specialization Initialization
//----------------------------------------------------------------------------//

void specialization_tlt_init(int argc, char ** argv) {

  {
  clog_tag_guard(devel_handle);
  clog(info) << "specialization_tlt_init function" << std::endl;
  } // scope

  coloring_map_t map { index_spaces::vertices, index_spaces::cells };

  flecsi_execute_mpi_task(add_colorings, flecsi::execution, map);

  auto & context { execution::context_t::instance() };

  auto & vinfo { context.coloring_info(index_spaces::vertices) };
  auto & cinfo { context.coloring_info(index_spaces::cells) };

  adjacency_info_t ai;
  ai.index_space = index_spaces::cells_to_vertices;
  ai.from_index_space = index_spaces::cells;
  ai.to_index_space = index_spaces::vertices;
  ai.color_sizes.resize(cinfo.size());

  for(auto & itr : cinfo){
    size_t color{itr.first};
    const coloring_info_t & ci = itr.second;
    ai.color_sizes[color] = (ci.exclusive + ci.shared + ci.ghost) * 4;
  } // for

  {
  clog_tag_guard(devel_handle);
  clog(info) << "Adding adjacency info: " << ai << std::endl;
  } // scope

  context.add_adjacency(ai);

  context.add_index_subspace(0, 1024);

} // specialization_tlt_init

//----------------------------------------------------------------------------//
// SPMD Specialization Initialization
//----------------------------------------------------------------------------//

void specialization_spmd_init(int argc, char ** argv) {

  {
  clog_tag_guard(devel_handle);
  clog(info) << "specialization_spmd_init function" << std::endl;
  } // scope

  auto mh = flecsi_get_client_handle(mesh_t, clients, m);
  flecsi_execute_task(initialize_mesh, flecsi::execution, single, mh);

} // specialization_spmd_ini

//----------------------------------------------------------------------------//
// User driver.
//----------------------------------------------------------------------------//

void driver(int argc, char ** argv) {

  auto mh = flecsi_get_client_handle(mesh_t, clients, m);
  auto ph = flecsi_get_handle(mh, data, pressure, double, dense, 0);

  flecsi_execute_task(initialize_pressure, flecsi::execution, single, mh, ph);
  flecsi_execute_task(update_pressure, flecsi::execution, single, mh, ph);
  flecsi_execute_task(print_mesh, flecsi::execution, single, mh, ph);

} // driver

} // namespace execution
} // namespace flecsi

DEVEL(devel_handle) {}

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
