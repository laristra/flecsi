/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2018, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */

///
/// \file
/// \date Initial file creation: Apr 3, 2018
///

#include <cinchtest.h>

#include <flecsi/execution/execution.h>
#include <flecsi/supplemental/coloring/add_colorings.h>
#include <flecsi/supplemental/mesh/test_mesh_2d.h>

#include <flecsi/data/dense_accessor.h>

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
// Type definitions
//----------------------------------------------------------------------------//

using point_t = flecsi::supplemental::point_t;
using index_t = flecsi::supplemental::index_t;
using vertex_t = flecsi::supplemental::vertex_t;
using cell_t = flecsi::supplemental::cell_t;
using mesh_t = flecsi::supplemental::test_mesh_2d_t;

using coloring_info_t = flecsi::coloring::coloring_info_t;
using adjacency_info_t = flecsi::coloring::adjacency_info_t;

template<size_t PS>
using mesh = data_client_handle__<mesh_t, PS>;

template<size_t EP, size_t SP, size_t GP>
using field = dense_accessor<size_t, EP, SP, GP>;

//----------------------------------------------------------------------------//
// Variable registration
//----------------------------------------------------------------------------//

flecsi_register_data_client(mesh_t, meshes, mesh1);
flecsi_register_field(
    mesh_t,
    hydro,
    pressure,
    size_t,
    dense,
    1,
    index_spaces::cells);

//----------------------------------------------------------------------------//
// Initialize mesh
//----------------------------------------------------------------------------//

void
initialize_mesh(mesh<wo> mesh) {
  auto & context = execution::context_t::instance();

  auto & vertex_map{context.index_map(index_spaces::vertices)};
  auto & reverse_vertex_map{context.reverse_index_map(index_spaces::vertices)};
  auto & cell_map{context.index_map(index_spaces::cells)};

  std::vector<vertex_t *> vertices;

#ifdef FLECSI_8_8_MESH
  const size_t width{8};
#else
  const size_t width{16};
#endif

  for (auto & vm : vertex_map) {
    const size_t mid{vm.second};
    const size_t row{mid / (width + 1)};
    const size_t column{mid % (width + 1)};
    // printf("vertex %lu: (%lu, %lu)\n", mid, row, column);
    point_t point({{(double)row, (double)column}});
    index_t index({{row, column}});

    vertices.push_back(mesh.make<vertex_t>(point, index));
  } // for

  size_t count{0};
  for (auto & cm : cell_map) {
    const size_t mid{cm.second};

    const size_t row{mid / width};
    const size_t column{mid % width};

    const size_t v0{(column) + (row) * (width + 1)};
    const size_t v1{(column + 1) + (row) * (width + 1)};
    const size_t v2{(column + 1) + (row + 1) * (width + 1)};
    const size_t v3{(column) + (row + 1) * (width + 1)};

    const size_t lv0{reverse_vertex_map[v0]};
    const size_t lv1{reverse_vertex_map[v1]};
    const size_t lv2{reverse_vertex_map[v2]};
    const size_t lv3{reverse_vertex_map[v3]};

    auto c{mesh.make<cell_t>(index_t{{row, column}})};
    mesh.init_cell<0>(
        c, {vertices[lv0], vertices[lv1], vertices[lv2], vertices[lv3]});
  } // for

  mesh.init<0>();
} // initizlize_mesh

flecsi_register_task(initialize_mesh, flecsi::execution, loc, single);

//----------------------------------------------------------------------------//
// Init field
//----------------------------------------------------------------------------//

void
init(mesh<ro> mesh, field<rw, rw, ro> h) {
  auto & context = execution::context_t::instance();
  auto rank = context.color();

  for (auto c : mesh.cells(owned)) {
    size_t val = c->index()[1] + 100 * (c->index()[0] + 100 * rank);
    h(c) = 1000000000 + val * 100;
  }

  for (auto c : mesh.cells(shared)) {
    h(c) += 5;
  }
} // init

flecsi_register_task(init, flecsi::execution, loc, single);

//----------------------------------------------------------------------------//
// Print field
//----------------------------------------------------------------------------//

void
print(mesh<ro> mesh, field<ro, ro, ro> h) {
  auto & context = execution::context_t::instance();
  auto rank = context.color();

  for (auto c : mesh.cells(exclusive)) {
    CINCH_CAPTURE() << "[" << rank << "]: exclusive cell id " << c->id<0>()
                    << "(" << c->index()[0] << "," << c->index()[1]
                    << "): " << h(c) << std::endl;
  }

  for (auto c : mesh.cells(shared)) {
    CINCH_CAPTURE() << "[" << rank << "]:    shared cell id " << c->id<0>()
                    << "(" << c->index()[0] << "," << c->index()[1]
                    << "): " << h(c) << std::endl;
  }

  for (auto c : mesh.cells(ghost)) {
    CINCH_CAPTURE() << "[" << rank << "]:     ghost cell id " << c->id<0>()
                    << "(" << c->index()[0] << "," << c->index()[1]
                    << "): " << h(c) << std::endl;
  }
} // print

flecsi_register_task(print, flecsi::execution, loc, single);

//----------------------------------------------------------------------------//
// Modify field
//----------------------------------------------------------------------------//

void
modify(mesh<ro> mesh, field<rw, rw, ro> h) {
  auto & context = execution::context_t::instance();
  auto rank = context.color();

  for (auto c : mesh.cells(owned)) {
    h(c) += 10 * (rank + 1);
  }

} // modify

flecsi_register_task(modify, flecsi::execution, loc, single);

//----------------------------------------------------------------------------//
// Top-Level Specialization Initialization
//----------------------------------------------------------------------------//

void
specialization_tlt_init(int argc, char ** argv) {
  clog(info) << "In specialization top-level-task init" << std::endl;

  coloring_map_t map{index_spaces::vertices, index_spaces::cells};
  flecsi_execute_mpi_task(add_colorings, flecsi::execution, map);

  auto & context{execution::context_t::instance()};
  auto & vinfo{context.coloring_info(index_spaces::vertices)};
  auto & cinfo{context.coloring_info(index_spaces::cells)};

  adjacency_info_t ai;
  ai.index_space = index_spaces::cells_to_vertices;
  ai.from_index_space = index_spaces::cells;
  ai.to_index_space = index_spaces::vertices;
  ai.color_sizes.resize(cinfo.size());

  for (auto & itr : cinfo) {
    size_t color{itr.first};
    const coloring::coloring_info_t & ci = itr.second;
    ai.color_sizes[color] = (ci.exclusive + ci.shared + ci.ghost) * 4;
  } // for

  context.add_adjacency(ai);
} // specialization_tlt_init

//----------------------------------------------------------------------------//
// SPMD Specialization Initialization
//----------------------------------------------------------------------------//

void
specialization_spmd_init(int argc, char ** argv) {
  auto mh = flecsi_get_client_handle(mesh_t, meshes, mesh1);
  flecsi_execute_task(initialize_mesh, flecsi::execution, single, mh);
} // specialization_spmd_init

//----------------------------------------------------------------------------//
// User driver.
//----------------------------------------------------------------------------//

void
driver(int argc, char ** argv) {
  auto ch = flecsi_get_client_handle(mesh_t, meshes, mesh1);
  auto ph = flecsi_get_handle(ch, hydro, pressure, size_t, dense, 0);

  auto f0 = flecsi_execute_task(init, flecsi::execution, single, ch, ph);
  f0.wait();

  auto f1 = flecsi_execute_task(print, flecsi::execution, single, ch, ph);
  f1.wait();

  auto f2 = flecsi_execute_task(modify, flecsi::execution, single, ch, ph);
  f2.wait();

  auto f3 = flecsi_execute_task(print, flecsi::execution, single, ch, ph);
  f3.wait();

  auto & context = execution::context_t::instance();
  if (context.color() == 0) {
    ASSERT_TRUE(CINCH_EQUAL_BLESSED("dense_data.blessed"));
  }

} // specialization_driver

//----------------------------------------------------------------------------//
// TEST.
//----------------------------------------------------------------------------//

TEST(dense_data, testname) {} // TEST

} // namespace execution
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
