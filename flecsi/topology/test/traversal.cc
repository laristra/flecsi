#include <cinchtest.h>
#include <iostream>

#include "flecsi/data/data.h"
#include "flecsi/utils/common.h"
#include "flecsi/topology/mesh_topology.h"

using namespace std;
using namespace flecsi;
using namespace topology;

class Vertex : public mesh_entity_t<0, 1>{
public:
  template<size_t M>
  uint64_t precedence() const { return 0; }
  Vertex() = default;
  Vertex(mesh_topology_base_t &) {}

};

class Edge : public mesh_entity_t<1, 1>{
public:
  Edge(mesh_topology_base_t &) {}
};

class Face : public mesh_entity_t<1, 1>{
public:
  Face(mesh_topology_base_t &) {}

};

class Cell : public mesh_entity_t<2, 1>{
public:

  using id_t = flecsi::utils::id_t;

  Cell(mesh_topology_base_t& mesh)
  : mesh_(mesh){}

  void set_precedence(size_t dim, uint64_t precedence) {}

  std::vector<size_t>
  create_entities(id_t cell_id, size_t dim, domain_connectivity<2> & c, id_t * e){
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

  void traverse();

private:
  mesh_topology_base_t& mesh_;
};

class TestMesh2dType{
public:
  static constexpr size_t num_dimensions = 2;

  static constexpr size_t num_domains = 1;

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

  using bindings = std::tuple<>;

  template<size_t M, size_t D>
  static mesh_entity_base_t<num_domains>*
  create_entity(mesh_topology_base_t* mesh, size_t num_vertices){
    switch(M){
      case 0:{
        switch(D){
          case 1:
            return mesh->make<Edge>(*mesh);
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

using TestMesh = mesh_topology_t<TestMesh2dType>;

void Cell::traverse(){
  auto& mesh = static_cast<TestMesh&>(mesh_);

  for(auto vertex : mesh.entities<0, 0>(this)) {
    cout << "------- traverse vertex id: " << vertex.id() << endl;
  }
}

TEST(mesh_topology, traversal) {

  size_t width = 2;
  size_t height = 2;

  auto mesh = new TestMesh;

  vector<Vertex*> vs;

  size_t id = 0;
  for(size_t j = 0; j < height + 1; ++j){
    for(size_t i = 0; i < width + 1; ++i){
      auto v = mesh->make<Vertex>();
      mesh->add_entity<0,0>(v);
      vs.push_back(v); 
    }
  }

  id = 0;
  size_t width1 = width + 1;
  for(size_t j = 0; j < height; ++j){
    for(size_t i = 0; i < width; ++i){
      auto c = mesh->make<Cell>(*mesh);

      mesh->add_entity<2, 0>(c);

      mesh->init_cell<0>(c,
                         {vs[i + j * width1],
                         vs[i + (j + 1) * width1],
                         vs[i + 1 + j * width1],
                         vs[i + 1 + (j + 1) * width1]}
                        );


    }
  }

  mesh->init<0>();

/*
  for(auto cell : mesh->entities<2>()) {
    cell->traverse();
  }
*/

  //mesh->dump();

  CINCH_CAPTURE() << "------------- forall cells, vertices" << endl;

  for(auto cell : mesh->entities<2>()) {
    CINCH_CAPTURE() << "------------- cell id: " << cell.id() << endl;
    for(auto vertex : mesh->entities<0>(cell)) {
      CINCH_CAPTURE() << "--- vertex id: " << vertex.id() << endl;
      for(auto cell2 : mesh->entities<2>(vertex)) {
        CINCH_CAPTURE() << "+ cell2 id: " << cell2.id() << endl;
      }
    }
  }

  CINCH_CAPTURE() << "------------- forall cells, edges" << endl;

  for(auto cell : mesh->entities<2>()) {
    CINCH_CAPTURE() << "------- cell id: " << cell.id() << endl;
    for(auto edge : mesh->entities<1>(cell)) {
      CINCH_CAPTURE() << "--- edge id: " << edge.id() << endl;
    }
  }

  CINCH_CAPTURE() << "------------- forall vertices, edges" << endl;

  for(auto vertex : mesh->entities<0>()) {
    CINCH_CAPTURE() << "------- vertex id: " << vertex.id() << endl;
    for(auto edge : mesh->entities<1>(vertex)) {
      CINCH_CAPTURE() << "--- edge id: " << edge.id() << endl;
    }
  }

  CINCH_CAPTURE() << "------------- forall vertices, cells" << endl;

  for(auto vertex : mesh->entities<0>()) {
    CINCH_CAPTURE() << "------- vertex id: " << vertex.id() << endl;
    for(auto cell : mesh->entities<2>(vertex)) {
      CINCH_CAPTURE() << "--- cell id: " << cell.id() << endl;
    }
  }

  CINCH_CAPTURE() << "------------- forall edges, cells" << endl;

  for(auto edge : mesh->entities<1>()) {
    CINCH_CAPTURE() << "------- edge id: " << edge.id() << endl;
    for(auto cell : mesh->entities<2>(edge)) {
      CINCH_CAPTURE() << "--- cell id: " << cell.id() << endl;
    }
  }

  CINCH_CAPTURE() << "------------- forall edges, vertices" << endl;

  for(auto edge : mesh->entities<1>()) {
    CINCH_CAPTURE() << "------- edge id: " << edge.id() << endl;
    for(auto vertex : mesh->entities<0>(edge)) {
      CINCH_CAPTURE() << "--- vertex id: " << vertex.id() << endl;
    }
  }

  ASSERT_TRUE(CINCH_EQUAL_BLESSED("traversal.blessed"));
}



//! \brief Tests the mesh_topology's destructor
TEST(mesh_topology, destructor) {

  // create a new data_client
  auto mesh = new TestMesh;

  // Register data
  flecsi_register_data(*mesh, hydro, pressure, double, global, 1);
  flecsi_register_data(*mesh, hydro, density, double, global, 1);

  // get all accessors to the data
  auto accs = flecsi_get_accessors(*mesh, hydro, double, global, 0);
  
  ASSERT_EQ( accs.size(), 2 );
  ASSERT_EQ( accs[0].label(), "density" );
  ASSERT_EQ( accs[1].label(), "pressure" );

  // store the runtime id
  auto rid = mesh->runtime_id();

  // delete the data_client
  delete mesh;

  // make sure the data is gone
  ASSERT_EQ( flecsi::data::storage_t::instance().count(rid), 0 );

}

//! \brief Tests the mesh_topology's move operator
TEST(mesh_topology, move) {

  // create new data_clients
  TestMesh mesh1, mesh2;
  auto rid1 = mesh1.runtime_id();

  // Register data
  flecsi_register_data(mesh1, hydro, pressure, double, global, 1);
  flecsi_register_data(mesh1, hydro, density, double, global, 1);

  // get all accessors to the data
  {
    auto accs = flecsi_get_accessors(mesh1, hydro, double, global, 0);
  
    ASSERT_EQ( accs.size(), 2 );
    ASSERT_EQ( accs[0].label(), "density" );
    ASSERT_EQ( accs[1].label(), "pressure" );
  }

  // create a new data client
  mesh2 = std::move(mesh1);
  auto rid2 = mesh1.runtime_id();

  // make sure the data is gone from data client 1
  ASSERT_EQ( flecsi::data::storage_t::instance().count(rid1), 0 );

  // it should show up in the new data client though
  {
    auto accs = flecsi_get_accessors(mesh2, hydro, double, global, 0);
  
    ASSERT_EQ( accs.size(), 2 );
    ASSERT_EQ( accs[0].label(), "density" );
    ASSERT_EQ( accs[1].label(), "pressure" );
  }

}

