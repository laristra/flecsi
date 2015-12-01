#include <cinchtest.h>
#include <iostream>

#include "../../utils/common.h"
#include "../mesh_topology.h"

using namespace std;
using namespace flexi;

class Vertex : public mesh_entity<0, 1>{
public:
  template<size_t M>
  uint64_t precedence() const { return 0; }
};

class Edge : public mesh_entity<1, 1>{
public:

};

class Face : public mesh_entity<1, 1>{
public:

};

class Cell : public mesh_entity<2, 1>{
public:
  void set_precedence(size_t dim, uint64_t precedence) {}

  std::pair<size_t, size_t>
  create_entities(size_t dim, std::vector<flexi::id_t>& e,
                  flexi::id_t *v, size_t vertex_count){  

    e.resize(8);

    e[0] = v[0];
    e[1] = v[2];
    
    e[2] = v[1];
    e[3] = v[3];
    
    e[4] = v[0];
    e[5] = v[1];
    
    e[6] = v[2];
    e[7] = v[3];

    return {4, 2};
  }
};

class TestMesh2dType{
public:
  static constexpr size_t dimension = 2;

  static constexpr size_t num_domains = 1;

  using Float = double;

  using entity_types = std::tuple<
    std::pair<domain_<0>, Vertex>,
    std::pair<domain_<0>, Edge>,
    std::pair<domain_<0>, Cell>>;

  using connectivities = 
    std::tuple<std::tuple<domain_<0>, Vertex, Edge>,
               std::tuple<domain_<0>, Vertex, Cell>,
               std::tuple<domain_<0>, Edge, Vertex>,
               std::tuple<domain_<0>, Edge, Cell>,
               std::tuple<domain_<0>, Cell, Vertex>,
               std::tuple<domain_<0>, Cell, Edge>>;
};

using TestMesh = mesh_topology<TestMesh2dType>;

TEST(mesh_topology, traversal) {

  size_t width = 2;
  size_t height = 2;

  auto mesh = new TestMesh;

  vector<Vertex*> vs;

  size_t id = 0;
  for(size_t j = 0; j < height + 1; ++j){
    for(size_t i = 0; i < width + 1; ++i){
      auto v = mesh->make<Vertex, 0>();
      mesh->add_vertex<0>(v);
      vs.push_back(v); 
    }
  }

  id = 0;
  size_t width1 = width + 1;
  for(size_t j = 0; j < height; ++j){
    for(size_t i = 0; i < width; ++i){
      auto c = mesh->make<Cell, 0>();
      mesh->add_cell<0>(c);

      mesh->init_cell<0>(c,
                         {vs[i + j * width1],
                         vs[i + (j + 1) * width1],
                         vs[i + 1 + j * width1],
                         vs[i + 1 + (j + 1) * width1]}
                        );


    }
  }

  mesh->init<0>();

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
