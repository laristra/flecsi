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

class TestMesh1dType{
public:
  static constexpr size_t num_dimensions = 1;
  static constexpr size_t num_domains = 1;

  using entity_types = std::tuple<
  std::tuple<index_space_<0>, domain_<0>, Vertex>,
  std::tuple<index_space_<1>, domain_<0>, Edge>>;
};

using TestMesh = structured_mesh_topology__<TestMesh1dType>;

namespace flecsi {
namespace execution {
//----------------------------------------------------------------------------//
//// User driver.
////----------------------------------------------------------------------------//
void driver(int argc, char ** argv) {};
} //flecsi
} //execution

TEST(structured, 1Dprimecell){

  std::array<size_t,TestMesh1dType::num_dimensions> lower_bounds = {0};
  std::array<size_t,TestMesh1dType::num_dimensions> upper_bounds = {2};
  std::array<size_t,TestMesh1dType::num_dimensions> strides = {3};
  size_t primary_dim = 1; 
  
  auto mst = new structured_mesh_storage__<TestMesh1dType::num_dimensions, 
                                          TestMesh1dType::num_domains>();
  auto mesh = new TestMesh(lower_bounds, upper_bounds, strides, primary_dim,  mst); 
  
  size_t nv, ne;
  auto lbnd = mesh->primary_lower_bounds();
  auto ubnd = mesh->primary_upper_bounds();

  CINCH_CAPTURE() << "1D Logically structured mesh with bounds: [" 
                  <<lbnd[0]<<"] - ["<<ubnd[0]<<"] \n"<< endl;

  nv = mesh->num_entities(0,0);
  ne = mesh->num_entities(1,0);
  
  CINCH_CAPTURE() << "NV = " << nv << endl;
  CINCH_CAPTURE() << "NE = " << ne << endl;
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
   CINCH_CAPTURE() << "  -- stencil [1] "  
                   <<mesh->stencil_entity< 1,0>(&vertex) << endl;
   CINCH_CAPTURE() << "  -- stencil [-1] " 
                   <<mesh->stencil_entity<-1,0>(&vertex) << endl; 
  
   //V-->E
   CINCH_CAPTURE() << "  -- query V-->E "<< endl; 
   for (auto edge : mesh->entities<1,0>(&vertex))
   {
    CINCH_CAPTURE() << "  ---- " <<edge.id(0)<< endl;
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
   CINCH_CAPTURE() << "  -- stencil [1] "  
                   <<mesh->stencil_entity< 1,0>(&edge) << endl;
   CINCH_CAPTURE() << "  -- stencil [-1] " 
                   <<mesh->stencil_entity<-1,0>(&edge) << endl; 

   //E-->V
   CINCH_CAPTURE() << "  -- query E-->V "<< endl; 
   for (auto vertex : mesh->entities<0,0>(&edge))
   {
    CINCH_CAPTURE() << "  ---- " <<vertex.id(0)<< endl;
   } 
  
   CINCH_CAPTURE()<<endl;
  }

  CINCH_WRITE("structured_1d.blessed");
//  ASSERT_TRUE(CINCH_EQUAL_BLESSED("structured_1d.blessed"));
  delete mst;
  delete mesh;
} // TEST
