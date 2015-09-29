#include <cinchtest.h>
#include <iostream>

#include "../mesh_topology.h"

using namespace std;
using namespace cinch;
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

  using Id = uint64_t;
  using Float = double;

  using EntityTypes = std::tuple<Vertex, Edge, Cell>;


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
  
  static void createEntities(size_t dim, std::vector<Id>& e, Vertex** v){
    assert(dim = 1);
    assert(e.size() == 8);
    
    e[0] = v[0]->id();
    e[1] = v[2]->id();
    
    e[2] = v[1]->id();
    e[3] = v[3]->id();
    
    e[4] = v[0]->id();
    e[5] = v[1]->id();
    
    e[6] = v[2]->id();
    e[7] = v[3]->id();
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

      mesh->addCell(c,
                    {vs[i + j * width1],
                     vs[i + (j + 1) * width1],
                     vs[i + 1 + j * width1],
                     vs[i + 1 + (j + 1) * width1]}
                    );
    }
  }

  CINCH_CAPTURE() << "------------- forall cells, vertices" << endl;

  for(TestMesh::CellIterator c(*mesh); !c.isend(); ++c){
    CINCH_CAPTURE() << "------- cell id: " << c->id() << endl;
    for(TestMesh::VertexIterator v(c); !v.isend(); ++v){
      CINCH_CAPTURE() << "--------- vertex id: " << v->id() << endl;
      for(TestMesh::CellIterator c2(v); !c2.isend(); ++c2){
        CINCH_CAPTURE() << "cell2 id: " << c2->id() << endl;
      }
    }
  }

  CINCH_CAPTURE() << "------------- forall cells, edges" << endl;

  for(TestMesh::CellIterator c(*mesh); !c.isend(); ++c){
    CINCH_CAPTURE() << "------- cell id: " << c->id() << endl;
    for(TestMesh::EdgeIterator e(c); !e.isend(); ++e){
      CINCH_CAPTURE() << "edge id: " << e->id() << endl;
    }
  }

  ASSERT_TRUE(CINCH_EQUAL_BLESSED("traversal.blessed"));
}
