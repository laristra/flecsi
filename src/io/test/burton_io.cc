#include <cinchtest.h>

#include "../../specializations/burton.h"
#include "../burton_io.h"

#include <vector>

using namespace std;
using namespace flexi;

using vertex_t = burton_mesh_t::vertex_t;
using edge_t = burton_mesh_t::edge_t;
using cell_t = burton_mesh_t::cell_t;
using point_t = burton_mesh_t::point_t;

// test fixture for creating the mesh
class Burton : public ::testing::Test {
protected:
  virtual void SetUp() {
    vector<vertex_t*> vs;
  
    for(size_t j = 0; j < height + 1; ++j){
      for(size_t i = 0; i < width + 1; ++i){
	auto v = b.create_vertex({double(i), 2.0*double(j)});
	v->setRank(1);
	vs.push_back(v);
      }
    }

    size_t width1 = width + 1;
    for(size_t j = 0; j < height; ++j){
      for(size_t i = 0; i < width; ++i){
	auto c = 
	  b.create_cell({vs[i + j * width1],
		vs[i + (j + 1) * width1],
		vs[i + 1 + j * width1],
		vs[i + 1 + (j + 1) * width1]});
      }
    }

    b.init();
  }

  virtual void TearDown() { }

  burton_mesh_t b;
  const size_t width = 2;
  const size_t height = 2;
};

TEST_F(Burton, writemesh) {

  std::string filename="test/mesh.exo";
  mesh_io_t io;
  io.write(filename, b);

} // TEST
