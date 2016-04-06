#include <iostream>
#include <cinchtest.h>

/*
#include <cinchtest.h>

#include "../mesh_topology.h"

using namespace std;
using namespace flecsi;

class SideVertex : public MeshEntity{
public:
  SideVertex(size_t id) : MeshEntity(id){}
};

class SideEdge : public MeshEntity{
public:
  SideEdge(size_t id) : MeshEntity(id){}
};

class Side : public MeshEntity{
public:
  Side(size_t id) : MeshEntity(id){}
};

class Vertex : public MeshEntity{
public:
  Vertex(size_t id) : MeshEntity(id){}
};

class Edge : public MeshEntity{
public:
  Edge(size_t id) : MeshEntity(id){}
};

class Cell : public MeshEntity{
public:
  Cell(size_t id) : MeshEntity(id){}
  
  void addSide(Side* s){
    sides_.add(s);
  }

  EntityGroup<2>& getSides(){
    return sides_;
  }

private:
  EntityGroup<2> sides_;
};

class TestMesh2dType{
public:
  static constexpr size_t dimension = 2;

  using Id = uint64_t;

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

class TestDualMesh2dType{
public:
  static constexpr size_t dimension = 2;

  using Id = uint64_t;

  using EntityTypes = std::tuple<SideVertex, SideEdge, Side>;

  static size_t numEntitiesPerCell(size_t dim){
    switch(dim){
    case 0:
      return 3;
    case 1:
      return 3;
    case 2:
      return 1;
    default:
      assert(false && "invalid dimension");
    }
  }

  static constexpr size_t verticesPerCell(){
    return 3;
  }
  
  static size_t numVerticesPerEntity(size_t dim){
    switch(dim){
    case 0:
      return 1;
    case 1:
      return 2;
    case 2:
      return 3;
    default:
      assert(false && "invalid dimension");
    }
  }
  
  static void createEntities(size_t dim, std::vector<Id>& e, SideVertex** v){
    assert(dim = 1);
    assert(e.size() == 6);
    
    e[0] = v[0]->id();
    e[1] = v[1]->id();
    
    e[2] = v[2]->id();
    e[3] = v[3]->id();
    
    e[4] = v[4]->id();
    e[5] = v[5]->id();
  }
};

using TestMesh = MeshTopology<TestMesh2dType>;
using TestDualMesh = MeshTopology<TestDualMesh2dType>;

TEST(mesh_topology, dual) {

  size_t width = 2;
  size_t height = 2;

  auto mesh = new TestMesh;

  auto dualMesh = new TestDualMesh;

  vector<Vertex*> vs;

  size_t id = 0;
  for(size_t j = 0; j < height + 1; ++j){
    for(size_t i = 0; i < width + 1; ++i){
      auto v = new Vertex(id++);
      vs.push_back(v);
      mesh->addVertex(v); 
    }
  }

  id = 0;
  size_t sideVertexId = 0;
  size_t sideId = 0;
  size_t width1 = width + 1;
  for(size_t j = 0; j < height; ++j){
    for(size_t i = 0; i < width; ++i){
      auto c = new Cell(id++);

      mesh->addCell(c,
                    {vs[i + j * width1],
                     vs[i + (j + 1) * width1],
                     vs[i + 1 + j * width1],
                     vs[i + 1 + (j + 1) * width1]}
                    );

      auto v0 = new SideVertex(sideVertexId++);
      auto v1 = new SideVertex(sideVertexId++);
      auto v2 = new SideVertex(sideVertexId++);
      auto v3 = new SideVertex(sideVertexId++);
      auto v4 = new SideVertex(sideVertexId++);
      auto v5 = new SideVertex(sideVertexId++);
      auto v6 = new SideVertex(sideVertexId++);
      auto v7 = new SideVertex(sideVertexId++);
      auto v8 = new SideVertex(sideVertexId++);

      auto s1 = new Side(sideId++);
      dualMesh->addCell(s1, {v0, v1, v2});
      c->addSide(s1);

      auto s2 = new Side(sideId++);
      dualMesh->addCell(s2, {v1, v3, v2});
      c->addSide(s2);

      auto s3 = new Side(sideId++);
      dualMesh->addCell(s3, {v1, v4, v5});
      c->addSide(s3);

      auto s4 = new Side(sideId++);
      dualMesh->addCell(s4, {v1, v5, v3});
      c->addSide(s4);

      auto s5 = new Side(sideId++);
      dualMesh->addCell(s5, {v2, v7, v6});
      c->addSide(s5);

      auto s6 = new Side(sideId++);
      dualMesh->addCell(s6, {v2, v3, v7});
      c->addSide(s6);

      auto s7 = new Side(sideId++);
      dualMesh->addCell(s7, {v3, v5, v7});
      c->addSide(s7);

      auto s8 = new Side(sideId++);
      dualMesh->addCell(s8, {v5, v8, v7});
      c->addSide(s8);
    }
  }

  CINCH_CAPTURE() << "------------- forall cells, sides" << endl;

  for(TestMesh::CellIterator c(*mesh); !c.isend(); ++c){
    CINCH_CAPTURE() << "------- cell id: " << c->id() << endl;
    for(TestDualMesh::CellIterator s(*dualMesh, c->getSides());
	 	!s.isend(); ++s){
      CINCH_CAPTURE() << "--------- side id: " << s->id() << endl;
    }
  }

  CINCH_CAPTURE() << "------------- forall cells, sides, vertices" << endl;

  for(TestMesh::CellIterator c(*mesh); !c.isend(); ++c){
    CINCH_CAPTURE() << "------- cell id: " << c->id() << endl;
    for(TestDualMesh::CellIterator s(*dualMesh, c->getSides());
	 	!s.isend(); ++s){
      CINCH_CAPTURE() << "--------- side id: " << s->id() << endl;
      for(TestDualMesh::VertexIterator sv(s);
          !sv.isend(); ++sv){
        CINCH_CAPTURE() << "--------- side vertex id: " << sv->id() << endl;
      }
    }
  }

  ASSERT_TRUE(CINCH_EQUAL_BLESSED("dual.blessed"));
}
*/

TEST(mesh_topology, dual) {}
