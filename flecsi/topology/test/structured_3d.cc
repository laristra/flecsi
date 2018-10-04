#include <iostream>
#include <cinchtest.h>
#include "flecsi/topology/structured_mesh_topology.h"
#include "flecsi/topology/mesh_storage.h"

using namespace std;
using namespace flecsi;
using namespace topology;


class Vertex : public structured_mesh_entity__<0, 1>{
public:
};

class Edge : public structured_mesh_entity__<1, 1>{
public:
};

class Face : public structured_mesh_entity__<2, 1>{
public:
};

class Cell : public structured_mesh_entity__<3, 1>{
public:
};


class TestMesh3dType{
public:
  static constexpr size_t num_dimensions = 3;
  static constexpr size_t num_domains = 1;

  using entity_types = std::tuple<
  std::tuple<index_space_<0>, domain_<0>, Vertex>,
  std::tuple<index_space_<1>, domain_<0>, Edge>,
  std::tuple<index_space_<2>, domain_<0>, Face>,
  std::tuple<index_space_<3>, domain_<0>, Cell>>;
};

using TestMesh = structured_mesh_topology__<TestMesh3dType>;

namespace flecsi {
namespace execution {
//----------------------------------------------------------------------------//
//// User driver.
////----------------------------------------------------------------------------//
void driver(int argc, char ** argv) {};
} //flecsi
} //execution

TEST(structured, simple){
  std::array<size_t,TestMesh3dType::num_dimensions> lower_bounds{0,0,0};
  std::array<size_t,TestMesh3dType::num_dimensions> upper_bounds{2,1,1};
  std::array<size_t,TestMesh3dType::num_dimensions> strides{3,2,2};
  size_t primary_dim = 3;
  auto ms = new structured_mesh_storage__<TestMesh3dType::num_dimensions,
                                        TestMesh3dType::num_domains>();
  auto mesh = new TestMesh(lower_bounds, upper_bounds, strides, primary_dim, ms); 
  size_t nv, ne, nf, nc;

  auto lbnd = mesh->primary_lower_bounds();
  auto ubnd = mesh->primary_upper_bounds();

  CINCH_CAPTURE() << "3D Logically structured mesh with bounds: [" 
                  <<lbnd[0]<<", "<<lbnd[1]<<", "<<lbnd[2]<<"] - ["
                  <<ubnd[0]<<", "<<ubnd[1]<<", "<<ubnd[2]<<"] \n"<< endl;

  nv = mesh->num_entities(0,0);
  ne = mesh->num_entities(1,0);
  nf = mesh->num_entities(2,0);
  nc = mesh->num_entities(3,0);

  CINCH_CAPTURE() << "NV = " << nv << endl;
  CINCH_CAPTURE() << "NE = " << ne << endl;
  CINCH_CAPTURE() << "NF = " << nf << endl;
  CINCH_CAPTURE() << "NC = " << nc << endl;
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
   CINCH_CAPTURE() << "  -- stencil [ 1  0  0] " 
                   <<mesh->stencil_entity< 1, 0, 0, 0 >(&vertex) << endl;
   CINCH_CAPTURE() << "  -- stencil [-1  0  0] " 
                   <<mesh->stencil_entity<-1, 0, 0, 0 >(&vertex) << endl; 
   CINCH_CAPTURE() << "  -- stencil [ 0  1  0] " 
                   <<mesh->stencil_entity< 0, 1, 0, 0 >(&vertex) << endl;
   CINCH_CAPTURE() << "  -- stencil [ 0 -1  0] " 
                   <<mesh->stencil_entity< 0,-1, 0, 0 >(&vertex) << endl;
   CINCH_CAPTURE() << "  -- stencil [ 0  0  1] " 
                   <<mesh->stencil_entity< 0, 0, 1, 0 >(&vertex) << endl;
   CINCH_CAPTURE() << "  -- stencil [ 0  0 -1] " 
                   <<mesh->stencil_entity< 0, 0,-1, 0 >(&vertex) << endl; 
   
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
 
   //V-->C
   CINCH_CAPTURE() << "  -- query V-->C "<< endl; 
   for (auto cell : mesh->entities<3,0>(&vertex))
   {
    CINCH_CAPTURE() << "  ---- " <<cell.id(0)<< endl;
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
   CINCH_CAPTURE() << "  -- stencil [ 1  0  0] " 
                   <<mesh->stencil_entity< 1, 0, 0, 0 >(&edge) << endl;
   CINCH_CAPTURE() << "  -- stencil [-1  0  0] " 
                   <<mesh->stencil_entity<-1, 0, 0, 0 >(&edge) << endl; 
   CINCH_CAPTURE() << "  -- stencil [ 0  1  0] " 
                   <<mesh->stencil_entity< 0, 1, 0, 0 >(&edge) << endl;
   CINCH_CAPTURE() << "  -- stencil [ 0 -1  0] " 
                   <<mesh->stencil_entity< 0,-1, 0, 0 >(&edge) << endl;
   CINCH_CAPTURE() << "  -- stencil [ 0  0  1] " 
                   <<mesh->stencil_entity< 0, 0, 1, 0 >(&edge) << endl;
   CINCH_CAPTURE() << "  -- stencil [ 0  0 -1] " 
                   <<mesh->stencil_entity< 0, 0,-1, 0 >(&edge) << endl; 
  
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
 
   //E-->C
   CINCH_CAPTURE() << "  -- query E-->C "<< endl; 
   for (auto cell : mesh->entities<3,0>(&edge))
   { 
    CINCH_CAPTURE() << "  ---- " <<cell.id(0)<< endl;
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
   CINCH_CAPTURE() << "  -- stencil [ 1  0  0] " 
                   <<mesh->stencil_entity< 1, 0, 0, 0 >(&face) << endl;
   CINCH_CAPTURE() << "  -- stencil [-1  0  0] " 
                   <<mesh->stencil_entity<-1, 0, 0, 0 >(&face) << endl; 
   CINCH_CAPTURE() << "  -- stencil [ 0  1  0] " 
                   <<mesh->stencil_entity< 0, 1, 0, 0 >(&face) << endl;
   CINCH_CAPTURE() << "  -- stencil [ 0 -1  0] " 
                   <<mesh->stencil_entity< 0,-1, 0, 0 >(&face) << endl;
   CINCH_CAPTURE() << "  -- stencil [ 0  0  1] " 
                   <<mesh->stencil_entity< 0, 0, 1, 0 >(&face) << endl;
   CINCH_CAPTURE() << "  -- stencil [ 0  0 -1] " 
                   <<mesh->stencil_entity< 0, 0,-1, 0 >(&face) << endl; 
   
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
 
   //F-->C
   CINCH_CAPTURE() << "  -- query F-->C "<< endl; 
   for (auto cell : mesh->entities<3,0>(&face))
   {
    CINCH_CAPTURE() << "  ---- " <<cell.id(0)<< endl;
   } 
  
   CINCH_CAPTURE()<<endl;
  }

  //Loop over all cells and test intra index space queries
  CINCH_CAPTURE()<<"------Cells------"<<endl;
  for (auto cell: mesh->entities<3>()){
   CINCH_CAPTURE() << "---- cell id: " << cell.id(0) << endl; 
   
   CINCH_CAPTURE() << "  -- indices "<< endl; 
   for (auto idv : mesh->get_global_box_indices_from_global_offset<3>(cell.id(0)))
    CINCH_CAPTURE() << "  ---- " <<idv << endl; 
   
   auto bid = mesh->get_box_id_from_global_offset<3>(cell.id(0));
   auto id = mesh->get_global_box_indices_from_global_offset<3>(cell.id(0));
   auto offset = mesh->get_global_offset_from_global_box_indices<3>(bid, id); 
   CINCH_CAPTURE() << "  ---- offset " <<offset<< endl; 
   ASSERT_EQ(cell.id(0),offset); 
 
   //C-->C
   CINCH_CAPTURE() << "  -- stencil [ 1  0  0] " 
                   <<mesh->stencil_entity< 1, 0, 0, 0 >(&cell) << endl;
   CINCH_CAPTURE() << "  -- stencil [-1  0  0] " 
                   <<mesh->stencil_entity<-1, 0, 0, 0 >(&cell) << endl; 
   CINCH_CAPTURE() << "  -- stencil [ 0  1  0] " 
                   <<mesh->stencil_entity< 0, 1, 0, 0 >(&cell) << endl;
   CINCH_CAPTURE() << "  -- stencil [ 0 -1  0] " 
                   <<mesh->stencil_entity< 0,-1, 0, 0 >(&cell) << endl;
   CINCH_CAPTURE() << "  -- stencil [ 0  0  1] " 
                   <<mesh->stencil_entity< 0, 0, 1, 0 >(&cell) << endl;
   CINCH_CAPTURE() << "  -- stencil [ 0  0 -1] " 
                   <<mesh->stencil_entity< 0, 0,-1, 0 >(&cell) << endl; 
   
   //C-->V
   CINCH_CAPTURE() << "  -- query C-->V "<< endl; 
   for (auto vertex : mesh->entities<0,0>(&cell))
   {
    CINCH_CAPTURE() << "  ---- " <<vertex.id(0)<< endl;
   } 
   
   //C-->E
   CINCH_CAPTURE() << "  -- query C-->E "<< endl; 
   for (auto edge : mesh->entities<1,0>(&cell))
   {
    CINCH_CAPTURE() << "  ---- " <<edge.id(0)<< endl;
   }
 
   //C-->F
   CINCH_CAPTURE() << "  -- query C-->F "<< endl; 
   for (auto face : mesh->entities<2,0>(&cell))
   {
    CINCH_CAPTURE() << "  ---- " <<face.id(0)<< endl;
   } 
  
   CINCH_CAPTURE()<<endl;
  }

  //CINCH_WRITE("structured_3d.blessed");
  ASSERT_TRUE(CINCH_EQUAL_BLESSED("structured_3d.blessed"));
  delete ms;
  delete mesh;
} // TEST
