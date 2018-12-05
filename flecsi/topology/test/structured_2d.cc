
#include <iostream>
#include <cinchtest.h>
#include <flecsi/coloring/box_types.h>
#include <flecsi/coloring/simple_box_colorer.h>
#include <flecsi/coloring/mpi_communicator.h>
#include <flecsi/execution/execution.h>
#include <flecsi/topology/structured_mesh_topology.h>
#include <flecsi/topology/mesh_storage.h>

using namespace std;

namespace flecsi {
namespace topology {
namespace test { 

class Vertex : public structured_mesh_entity__<0, 1>{
public:
};

class Edge : public structured_mesh_entity__<1, 1>{
public:

};

class Face : public structured_mesh_entity__<2, 1>{
public:
};


class TestMesh2dType{
public:
  static constexpr size_t num_dimensions = 2;
  static constexpr size_t num_domains = 1;

  using entity_types = std::tuple<
  std::tuple<index_space_<0>, domain_<0>, Vertex>,
  std::tuple<index_space_<1>, domain_<0>, Edge>,
  std::tuple<index_space_<2>, domain_<0>, Face>>;
};

struct TestMesh : public structured_mesh_topology__<TestMesh2dType> {};

flecsi_register_data_client(TestMesh, meshes, mesh0);

void task_tlt_init() {

  // Get the context instance.
  auto & context_ = execution::context_t::instance();

  int rank, size;
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  {
    clog_tag_guard(coloring);
    clog(info) << "add_colorings, rank: " << rank << std::endl;
  }

  // Define bounds for the box corresponding to cells
  size_t grid_size[2] = {4,4};
  size_t ncolors[2]={2,1};
  size_t nhalo = 1;
  size_t nhalo_domain = 1;
  size_t thru_dim = 0;

  coloring::box_coloring_t colored_cells;
  flecsi::coloring::coloring_info_t colored_cells_aggregate;

  // Create a colorer instance to generate the coloring.
  auto colorer = std::make_shared<flecsi::coloring::simple_box_colorer_t<2>>();

  //Create the coloring info for cells
  colored_cells = colorer->color(grid_size, nhalo, nhalo_domain, thru_dim, ncolors);

  //Create the aggregate type for cells
  colored_cells_aggregate = colorer->create_aggregate_color_info(colored_cells);

 // Create a communicator instance to get coloring_info_t from other ranks
  auto communicator = std::make_shared<flecsi::coloring::mpi_communicator_t>();

  // Gather the coloring info from all colors
  auto cell_coloring_info = communicator->gather_coloring_info(colored_cells_aggregate);

  //Add box coloring to context
  context_.add_box_coloring(2, colored_cells, cell_coloring_info);

}

flecsi_register_mpi_task(task_tlt_init, flecsi::topology::test);

void task_spmd_init(flecsi::data_client_handle__<TestMesh, flecsi::wo> mesh)
{
  for (auto c: mesh.entities<2>())
   cout << "---- face id: " << c.id(0) << endl; 

  //CINCH_CAPTURE() << "---- face id: " << c.id(0) << endl; 
  
  //CINCH_WRITE("structured_2d_new.blessed");
}

flecsi_register_task(task_spmd_init, flecsi::topology::test, loc,
    single|flecsi::leaf);

} //namespace test
} //namespace topology

namespace execution {

//----------------------------------------------------------------------------//
//// User driver.
////----------------------------------------------------------------------------//
void driver(int argc, char ** argv)
{
  //CINCH_WRITE("structured_2d_new.blessed");
};

//----------------------------------------------------------------------------//
//// User init.
////----------------------------------------------------------------------------//
void specialization_tlt_init(int argc, char ** argv) {
  std::cout << "TLT INIT" << std::endl;

  flecsi_execute_mpi_task(task_tlt_init, flecsi::topology::test);

} // specialization_tlt_init

void specialization_spmd_init(int argc, char ** argv) {
  auto & context = flecsi::execution::context_t::instance();
  std::cout << "SPMD INIT" << std::endl;
  using TestMesh = flecsi::topology::test::TestMesh;
  auto mesh_handle = flecsi_get_client_handle(
      TestMesh, meshes, mesh0);
  flecsi_execute_task(task_spmd_init, flecsi::topology::test, single,
      mesh_handle);
}

} //execution


} //flecsi

TEST(structured, 2Dsimple){
} // TEST

#if 0
namespace flecsi {
namespace execution {
//----------------------------------------------------------------------------//
// User driver.
//----------------------------------------------------------------------------//
void driver(int argc, char ** argv) {};
} //flecsi
} //execution

TEST(structured, simple){
  std::array<size_t, TestMesh2dType::num_dimensions> lower_bounds = {3,3};
  std::array<size_t, TestMesh2dType::num_dimensions> upper_bounds = {5,6};
  std::array<size_t, TestMesh2dType::num_dimensions> strides = {8,8};
  size_t primary_dim = 2; 
  
  auto ms = new structured_mesh_storage__<TestMesh2dType::num_dimensions,
                                          TestMesh2dType::num_domains>();
  
  auto mesh = new TestMesh(ms); 
  
  mesh->initialize(lower_bounds, upper_bounds, strides, primary_dim); 
  
  size_t nv, ne, nf;

  auto lbnd = mesh->primary_lower_bounds();
  auto ubnd = mesh->primary_upper_bounds();

  CINCH_CAPTURE() << "2D Logically structured mesh with bounds: [" <<lbnd[0]<<
  ", "<<lbnd[1]<<"] - ["<<ubnd[0]<<", "<<ubnd[1]<<"] \n"<< endl;

  nv = mesh->num_entities(0,0);
  ne = mesh->num_entities(1,0);
  nf = mesh->num_entities(2,0);
  
  CINCH_CAPTURE() << "NV = " << nv << endl;
  CINCH_CAPTURE() << "NE = " << ne << endl;
  CINCH_CAPTURE() << "NF = " << nf << endl;
  CINCH_CAPTURE()<<endl;
 
  //Loop over all vertices and test intra index space queries
  CINCH_CAPTURE()<<"------Vertices------"<<endl;
  for (auto vertex: mesh->entities<0>()){
   CINCH_CAPTURE() << "---- vertex id: " << vertex.id(0) << endl; 
   
   CINCH_CAPTURE() << "  -- indices "<< endl; 
   for (auto idv : mesh->get_global_box_indices_from_global_offset<0>(vertex.id(0)))
    CINCH_CAPTURE() << "  ---- " <<idv << endl; 
   
   auto bid = mesh->get_box_id_from_global_offset<0>(vertex.id(0));
   auto id = mesh->get_global_box_indices_from_global_offset<0>(vertex.id(0));
   auto offset = mesh->get_global_offset_from_global_box_indices<0>(bid, id); 
   CINCH_CAPTURE() << "  ---- offset " <<offset<< endl; 
   ASSERT_EQ(vertex.id(0),offset); 
 
   //V-->V
   CINCH_CAPTURE() << "  -- stencil [1 0] "  
                   <<mesh->stencil_entity< 1, 0, 0>(&vertex) << endl;
   CINCH_CAPTURE() << "  -- stencil [0 1] "  
                   <<mesh->stencil_entity< 0, 1, 0>(&vertex) << endl;
   CINCH_CAPTURE() << "  -- stencil [-1 0] " 
                   <<mesh->stencil_entity<-1, 0, 0>(&vertex) << endl; 
   CINCH_CAPTURE() << "  -- stencil [0 -1] " 
                   <<mesh->stencil_entity< 0,-1, 0>(&vertex) << endl;
   
   //V-->E
   CINCH_CAPTURE() << "  -- query V-->E "<< endl; 
   for (auto edge : mesh->entities<1,0>(&vertex))
   {
    CINCH_CAPTURE() << "  ---- " <<edge.id(0)<< endl;
   } 
   
   //V-->F
   CINCH_CAPTURE() << "  -- query V-->F "<< endl; 
   for (auto face : mesh->entities<2,0>(&vertex))
   {
    CINCH_CAPTURE() << "  ---- " <<face.id(0)<< endl;
   } 
  
   CINCH_CAPTURE()<<endl;
  }
  
  //Loop over all edges and test intra index space queries
  CINCH_CAPTURE()<<"------Edges------"<<endl;
  for (auto edge: mesh->entities<1>()){
   CINCH_CAPTURE() << "---- edge id: " << edge.id(0) << endl; 
   
   CINCH_CAPTURE() << "  -- indices "<< endl; 
   for (auto idv : mesh->get_global_box_indices_from_global_offset<1>(edge.id(0)))
    CINCH_CAPTURE() << "  ---- " <<idv << endl; 
   
   auto bid = mesh->get_box_id_from_global_offset<1>(edge.id(0));
   auto id = mesh->get_global_box_indices_from_global_offset<1>(edge.id(0));
   auto offset = mesh->get_global_offset_from_global_box_indices<1>(bid, id); 
   CINCH_CAPTURE() << "  ---- offset " <<offset<< endl; 
   ASSERT_EQ(edge.id(0),offset); 
 
   //E-->E
   CINCH_CAPTURE() << "  -- stencil [1 0] "  
                   <<mesh->stencil_entity< 1, 0, 0>(&edge) << endl;
   CINCH_CAPTURE() << "  -- stencil [0 1] "  
                   <<mesh->stencil_entity< 0, 1, 0>(&edge) << endl;
   CINCH_CAPTURE() << "  -- stencil [-1 0] " 
                   <<mesh->stencil_entity<-1, 0, 0>(&edge) << endl; 
   CINCH_CAPTURE() << "  -- stencil [0 -1] " 
                   <<mesh->stencil_entity< 0,-1, 0>(&edge) << endl;
  
   //E-->V
   CINCH_CAPTURE() << "  -- query E-->V "<< endl; 
   for (auto vertex : mesh->entities<0,0>(&edge))
   {
    CINCH_CAPTURE() << "  ---- " <<vertex.id(0)<< endl;
   } 
   
   //E-->F
   CINCH_CAPTURE() << "  -- query E-->F "<< endl; 
   for (auto face : mesh->entities<2,0>(&edge))
   { 
    CINCH_CAPTURE() << "  ---- " <<face.id(0)<< endl;
   } 
  
   CINCH_CAPTURE()<<endl;
  }

  //Loop over all faces and test intra index space queries
  CINCH_CAPTURE()<<"------Faces------"<<endl;
  for (auto face: mesh->entities<2>()){
   CINCH_CAPTURE() << "---- face id: " << face.id(0) << endl; 
   
   CINCH_CAPTURE() << "  -- indices "<< endl; 
   for (auto idv : mesh->get_global_box_indices_from_global_offset<2>(face.id(0)))
    CINCH_CAPTURE() << "  ---- " <<idv << endl; 
   
   auto bid = mesh->get_box_id_from_global_offset<2>(face.id(0));
   auto id = mesh->get_global_box_indices_from_global_offset<2>(face.id(0));
   auto offset = mesh->get_global_offset_from_global_box_indices<2>(bid, id); 
   CINCH_CAPTURE() << "  ---- offset " <<offset<< endl; 
   ASSERT_EQ(face.id(0),offset); 
 
   //F-->F
   CINCH_CAPTURE() << "  -- stencil [1 0] "  
                   <<mesh->stencil_entity< 1, 0, 0>(&face) << endl;
   CINCH_CAPTURE() << "  -- stencil [0 1] "  
                   <<mesh->stencil_entity< 0, 1, 0>(&face) << endl;
   CINCH_CAPTURE() << "  -- stencil [-1 0] " 
                   <<mesh->stencil_entity<-1, 0, 0>(&face) << endl; 
   CINCH_CAPTURE() << "  -- stencil [0 -1] " 
                   <<mesh->stencil_entity< 0,-1, 0>(&face) << endl;
  
   //V-->E
   CINCH_CAPTURE() << "  -- query F-->V "<< endl; 
   for (auto vertex : mesh->entities<0,0>(&face))
   {
    CINCH_CAPTURE() << "  ---- " <<vertex.id(0)<< endl;
   } 
   
   //F-->E
   CINCH_CAPTURE() << "  -- query F-->E "<< endl; 
   for (auto edge : mesh->entities<1,0>(&face))
   {
    CINCH_CAPTURE() << "  ---- " <<edge.id(0)<< endl;
   } 
  
   CINCH_CAPTURE()<<endl;
  }

  //CINCH_WRITE("structured_2d.blessed");
  ASSERT_TRUE(CINCH_EQUAL_BLESSED("structured_2d.blessed"));
  
  delete ms;
  delete mesh;
} // TEST
#endif 
