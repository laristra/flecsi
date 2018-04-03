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
#include <flecsi/io/simple_definition.h>
#include <flecsi/coloring/dcrs_utils.h>
#include <flecsi/coloring/parmetis_colorer.h>
#include <flecsi/coloring/mpi_communicator.h>
#include <flecsi/supplemental/coloring/add_colorings.h>
#include <flecsi/data/mutator_handle.h>
#include <flecsi/data/dense_accessor.h>
#include <flecsi/data/mutator.h>
#include <flecsi/topology/mesh.h>
#include <flecsi/topology/mesh_topology.h>

using namespace std;
using namespace flecsi;
using namespace topology;
using namespace execution;
using namespace coloring;

clog_register_tag(coloring);

using index_t = std::array<size_t, 2>;

class vertex : public mesh_entity__<0, 1>{
public:
  template<size_t M>
  uint64_t precedence() const { return 0; }

  index_t index;
  vertex(index_t index) : index(index) {}
};

class edge : public mesh_entity__<1, 1>{
public:
};

class face : public mesh_entity__<1, 1>{
public:
};

class cell : public mesh_entity__<2, 1>{
public:
  index_t index;
  cell(index_t index) : index(index) {}

  using id_t = flecsi::utils::id_t;

  std::vector<size_t>
  create_entities(id_t cell_id, size_t dim, domain_connectivity__<2> & c, id_t * e){
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
  }

}; // class cell

enum index_spaces : size_t {
  vertices,
  edges,
  cells,
  vertices_to_cells,
  cells_to_vertices
}; // enum index_spaces

class test_mesh_types_t{
public:
  using id_t = flecsi::utils::id_t;

  flecsi_register_number_dimensions(2);
  flecsi_register_number_domains(1);

  flecsi_register_entity_types(
    flecsi_entity_type(index_spaces::vertices, 0, vertex),
    flecsi_entity_type(index_spaces::cells, 0, cell)
  );

  flecsi_register_connectivities(
    flecsi_connectivity(index_spaces::cells_to_vertices, 0, cell, vertex)
  );

  flecsi_register_bindings();

  template<size_t M, size_t D, typename ST>
  static mesh_entity_base__<num_domains>*
  create_entity(mesh_topology_base__<ST>* mesh, size_t num_vertices,
    id_t const & id){
    switch(M){
      case 0:{
        switch(D){
          case 1:
            //return mesh->template make<edge>(*mesh);
          default:
            assert(false && "invalid topological dimension");
        }
        break;
      }
      default:
        assert(false && "invalid domain");
    }
  }
};

struct test_mesh_t : public mesh_topology__<test_mesh_types_t> {
  auto vertices() {
    return entities<0,0>();
  } // vertices

  template< typename E, size_t M>
  auto vertices(flecsi::topology::domain_entity__<M, E> & e) {
    return entities<0, 0>(e);
  } // vertices

  auto cells() {
    return entities<2,0>();
  } // cells

  auto cells(partition_t p) {
    return entities<2,0>(p);
  } // cells
};

template<typename DC, size_t PS>
using client_handle_t = data_client_handle__<DC, PS>;

void initialize_mesh(client_handle_t<test_mesh_t, wo> mesh) {

  auto & context = execution::context_t::instance();

  auto & vertex_map { context.index_map(index_spaces::vertices) };
  auto & reverse_vertex_map
    { context.reverse_index_map(index_spaces::vertices) };
  auto & cell_map { context.index_map(index_spaces::cells) };

  std::vector<vertex*> vertices;

#ifdef FLECSI_8_8_MESH
  const size_t width { 8 };
#else
  const size_t width { 16 };
#endif

  for(auto & vm: vertex_map) {
    const size_t mid { vm.second };
    const size_t row { mid/(width+1) };
    const size_t column { mid%(width+1) };
    // printf("vertex %lu: (%lu, %lu)\n", mid, row, column);

    vertices.push_back(mesh.make<vertex>(index_t{{ row, column }}));
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

    auto c { mesh.make<cell>(index_t{{ row, column }}) };
    mesh.init_cell<0>(c, { vertices[lv0], vertices[lv1],
      vertices[lv2], vertices[lv3] });
  } // for

  mesh.init<0>();
} // initizlize_mesh

void init(client_handle_t<test_mesh_t, ro> mesh,
          dense_accessor<size_t, rw, rw, ro> h) {
  auto& context = execution::context_t::instance();
  auto rank = context.color();

  for (auto c : mesh.cells(owned)) {
    size_t val = c->index[1] + 100 * (c->index[0] + 100 * rank);
    h(c) = 1000000000 + val * 100;
  }

  for (auto c : mesh.cells(shared)) {
    h(c) += 5;
  }
} // init

void print(client_handle_t<test_mesh_t, ro> mesh,
           dense_accessor<size_t, ro, ro, ro> h) {
  auto& context = execution::context_t::instance();
  auto rank = context.color();

  for (auto c : mesh.cells(exclusive)) {
    CINCH_CAPTURE() << "[" << rank << "]: exclusive cell id " << c->id<0>()
        << "(" << c->index[0] << "," << c->index[1] << "): "
        << h(c) << std::endl;
  }

  for (auto c : mesh.cells(shared)) {
    CINCH_CAPTURE() << "[" << rank << "]:    shared cell id " << c->id<0>()
        << "(" << c->index[0] << "," << c->index[1] << "): "
        << h(c) << std::endl;
  }

  for (auto c : mesh.cells(ghost)) {
    CINCH_CAPTURE() << "[" << rank << "]:     ghost cell id " << c->id<0>()
        << "(" << c->index[0] << "," << c->index[1] << "): "
        << h(c) << std::endl;
  }
} // print

void modify(client_handle_t<test_mesh_t, ro> mesh,
            dense_accessor<size_t, rw, rw, ro> h) {
  auto& context = execution::context_t::instance();
  auto rank = context.color();

  for (auto c : mesh.cells(owned)) {
    h(c) += 10 * (rank + 1);
  }

} // modify

flecsi_register_data_client(test_mesh_t, meshes, mesh1);

flecsi_register_task_simple(initialize_mesh, loc, single);
flecsi_register_task_simple(init, loc, single);
flecsi_register_task_simple(print, loc, single);
flecsi_register_task_simple(modify, loc, single);

flecsi_register_field(test_mesh_t, hydro, pressure, size_t, dense, 1,
    index_spaces::cells);

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
// Specialization driver.
//----------------------------------------------------------------------------//

void specialization_tlt_init(int argc, char ** argv) {
  clog(info) << "In specialization top-level-task init" << std::endl;

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
    const coloring::coloring_info_t & ci = itr.second;
    ai.color_sizes[color] = (ci.exclusive + ci.shared + ci.ghost) * 4;
  } // for

  context.add_adjacency(ai);
} // specialization_tlt_init

void specialization_spmd_init(int argc, char ** argv) {
  auto mh = flecsi_get_client_handle(test_mesh_t, meshes, mesh1);
  flecsi_execute_task_simple(initialize_mesh, single, mh);
} // specialization_spmd_init

//----------------------------------------------------------------------------//
// User driver.
//----------------------------------------------------------------------------//

void driver(int argc, char ** argv) {
  auto ch = flecsi_get_client_handle(test_mesh_t, meshes, mesh1);
  auto ph = flecsi_get_handle(ch, hydro, pressure, size_t, dense, 0);

  auto f0 = flecsi_execute_task_simple(init, single, ch, ph);
  f0.wait();

  auto f1 = flecsi_execute_task_simple(print, single, ch, ph);
  f1.wait();

  auto f2 = flecsi_execute_task_simple(modify, single, ch, ph);
  f2.wait();

  auto f3 = flecsi_execute_task_simple(print, single, ch, ph);
  f3.wait();

  auto& context = execution::context_t::instance();
  if(context.color() == 0){
    ASSERT_TRUE(CINCH_EQUAL_BLESSED("dense_data.blessed"));
  }

} // specialization_driver

//----------------------------------------------------------------------------//
// TEST.
//----------------------------------------------------------------------------//

TEST(dense_data, testname) {

} // TEST

} // namespace execution
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
