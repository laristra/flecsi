
#include <iostream>

#include <flecsi-config.h>
#include <flecsi/data/data.h>
#include <flecsi/execution/execution.h> 

#include "mesh.h"
#include "coloring/add_colorings.h" 

//----------------------------------------------------------------------------//
// Register a mesh and its initialization task with runtime
//----------------------------------------------------------------------------//
namespace flecsi {
namespace simple_mesh {

flecsi_register_data_client(mesh_t, test_mesh, fmesh);
flecsi_register_task(initialize_mesh, flecsi::simple_mesh, loc, index);
} //namespace simple_mesh
} //namespace flecsi


//----------------------------------------------------------------------------//
// Namespaced user tasks and their registration
//----------------------------------------------------------------------------//
namespace flecsi {
namespace test_traversal {

void traverse_simple_mesh(simple_mesh::mesh_handle_t<ro> m)
{
  //Iterate over the vertices 
  for (auto v: m.vertices()) {
   std::cout<<"vertex id = "<<v->id()<<std::endl;
   // v->print("I am a vertex"); 
  }
   
  //Iterate over the cells 
  for (auto c: m.cells()) {
    c->print("I am a cell"); 
    auto cen = m.centroid(c); 
    std::cout<<"Centroid = {"<<cen[0]<<", "<<cen[1]<<"}"<<std::endl;

    for (auto cv: m.vertices(c)) {
     cv->print("Vertex : "); 
    }
  }
}

flecsi_register_task(traverse_simple_mesh, flecsi::test_traversal, loc, single);
} //namespace test_traversal
} //namespace flecsi

//----------------------------------------------------------------------------//
// Control points
//----------------------------------------------------------------------------//
namespace flecsi {
namespace execution {

using namespace flecsi::simple_mesh;
using namespace flecsi::simple_mesh_coloring; 

void
specialization_tlt_init(int argc, char ** argv) {

  std::cout << "In specialization top-level-task init" << std::endl;

  coloring_map_t map{index_spaces::vertices, index_spaces::cells};
  flecsi_execute_mpi_task(add_colorings, flecsi::simple_mesh_coloring, map);

  auto & context{execution::context_t::instance()};
  auto & vinfo{context.coloring_info(index_spaces::vertices)};
  auto & cinfo{context.coloring_info(index_spaces::cells)};

  coloring::adjacency_info_t ai;
  ai.index_space = index_spaces::cells_to_vertices;
  ai.from_index_space = index_spaces::cells;
  ai.to_index_space = index_spaces::vertices;
  ai.color_sizes.resize(cinfo.size());

  for(auto & itr : cinfo) {
    size_t color{itr.first};
    const coloring::coloring_info_t & ci = itr.second;
    ai.color_sizes[color] = (ci.exclusive + ci.shared + ci.ghost) * 4;
  } // for

  context.add_adjacency(ai);
} // specialization_tlt_init

void
specialization_spmd_init(int argc, char ** argv) {

  std::cout << "In specialization spmd init" << std::endl;

  auto mh = flecsi_get_client_handle(mesh_t, test_mesh, fmesh);

  flecsi_execute_task(initialize_mesh, flecsi::simple_mesh, index, mh);
} // specialization_spmd_init

void
driver(int argc, char ** argv) {

   std::cout << "In driver" << std::endl;
  
   auto mh = flecsi_get_client_handle(mesh_t, test_mesh, fmesh);

   flecsi_execute_task(traverse_simple_mesh, flecsi::test_traversal, single, mh);
} // driver

} // namespace execution
} // namespace flecsi
