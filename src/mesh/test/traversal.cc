#include <cinchtest.h>
#include <iostream>

#include "../../utils/common.h"
#include "../mesh_topology.h"

using namespace std;
using namespace flexi;

class Vertex : public MeshEntity<0>{
public:

};

class Edge : public MeshEntity<1>{
public:

};

class Face : public MeshEntity<1>{
public:

};

class Cell : public MeshEntity<2>{
public:

};

class TestMesh2dType{
public:
  static constexpr size_t dimension = 2;

  using Float = double;

  using EntityTypes = std::tuple<Vertex, Edge, Cell>;

  using TraversalPairs = 
    std::tuple<std::pair<Vertex, Edge>,
               std::pair<Vertex, Cell>,
               std::pair<Edge, Vertex>,
               std::pair<Edge, Cell>,
               std::pair<Cell, Vertex>,
               std::pair<Cell, Edge>>;

  static size_t numEntitiesPerCell(size_t dim){
    switch(dim){
    case 0:
      return 4;
    case 1:
      return 4;
    case 2:
      return 1;
    default:
      assert(false && "invalid dimension");
    }
  }

  static constexpr size_t verticesPerCell(){
    return 4;
  }
  
  static size_t numVerticesPerEntity(size_t dim){
    switch(dim){
    case 0:
      return 1;
    case 1:
      return 2;
    case 2:
      return 4;
    default:
      assert(false && "invalid dimension");
    }
  }
  
  static void createEntities(size_t dim, std::vector<flexi::id_t>& e,
                             flexi::id_t *v){
    assert(dim = 1);
    assert(e.size() == 8);
    
    e[0] = v[0];
    e[1] = v[2];
    
    e[2] = v[1];
    e[3] = v[3];
    
    e[4] = v[0];
    e[5] = v[1];
    
    e[6] = v[2];
    e[7] = v[3];
  }
};

using TestMesh = MeshTopology<TestMesh2dType>;

TEST(mesh_topology, traversal) {

  size_t width = 2;
  size_t height = 2;

  auto mesh = new TestMesh;

  vector<Vertex*> vs;

  size_t id = 0;
  for(size_t j = 0; j < height + 1; ++j){
    for(size_t i = 0; i < width + 1; ++i){
      auto v = mesh->make<Vertex>();
      vs.push_back(v);
      mesh->addVertex(v); 
    }
  }

  id = 0;
  size_t width1 = width + 1;
  for(size_t j = 0; j < height; ++j){
    for(size_t i = 0; i < width; ++i){
      auto c = mesh->make<Cell>();

      mesh->initCell(c,
                    {vs[i + j * width1],
                     vs[i + (j + 1) * width1],
                     vs[i + 1 + j * width1],
                     vs[i + 1 + (j + 1) * width1]}
                    );
    }
  }

  CINCH_CAPTURE() << "------------- forall cells, vertices" << endl;

  for(TestMesh::cell_index_iterator c(*mesh); !c.end(); ++c){
    CINCH_CAPTURE() << "------- cell id: " << *c << endl;
    for(TestMesh::vertex_index_iterator v(c); !v.end(); ++v){
      CINCH_CAPTURE() << "--------- vertex id: " << *v << endl;
      for(TestMesh::cell_index_iterator c2(v); !c2.end(); ++c2){
        CINCH_CAPTURE() << "cell2 id: " << *c2 << endl;
      }
    }
  }

  CINCH_CAPTURE() << "------------- forall cells, edges" << endl;

  for(TestMesh::cell_index_iterator c(*mesh); !c.end(); ++c){
    CINCH_CAPTURE() << "------- cell id: " << *c << endl;
    for(TestMesh::edge_index_iterator e(c); !e.end(); ++e){
      CINCH_CAPTURE() << "edge id: " << *e << endl;
    }
  }

  ASSERT_TRUE(CINCH_EQUAL_BLESSED("traversal.blessed"));
}
