#include <cinchtest.h>
#include <iostream>

#include "flecsi/utils/common.h"
#include "flecsi/mesh/mesh_topology.h"
#include "flecsi/mesh/mesh_types.h"
#include "flecsi/data/old_data.h"

using namespace std;
using namespace flecsi;
using namespace data_model;

#define FILTER(E) [&](auto E) -> bool

class Vertex : public mesh_entity_t<0, 2>{
public:
  Vertex(){}

  Vertex(mesh_topology_base_t &){}

  template<size_t M>
  uint64_t precedence() const { return 0; }
};

class Edge : public mesh_entity_t<1, 2>{
public:
  Edge(){}

  Edge(mesh_topology_base_t &){}
};

class Cell : public mesh_entity_t<2, 2>{
public:
  Cell(){}

  Cell(mesh_topology_base_t &){}

  void set_precedence(size_t dim, uint64_t precedence) {}

  std::vector<size_t>
  create_entities(flecsi::id_t cell_id, size_t dim, domain_connectivity<2> & c, flecsi::id_t * e){
    flecsi::id_t* v = c.get_entities(cell_id, 0);

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

  flecsi::index_vector_t
  create_bound_entities(size_t from_domain,
                        size_t to_domain,
                        size_t create_dim,
                        flecsi::id_t cell_id,
                        domain_connectivity<2>& primal_conn,
                        domain_connectivity<2>& domain_conn, 
                        flecsi::id_t *c) {
    
    flecsi::id_t* v = primal_conn.get_entities(cell_id, 0);
    flecsi::id_t* e = primal_conn.get_entities(cell_id, 1);

    switch(create_dim) {
      case 0:
        c[0] = v[0];
        c[1] = e[0];
        c[2] = e[2];

        c[3] = v[1];
        c[4] = e[1];
        c[5] = e[2];

        c[6] = v[2];
        c[7] = e[0];
        c[8] = e[3];

        c[9]  = v[3];
        c[10] = e[3];
        c[11] = e[1];

        return {3, 3, 3, 3};
      case 1:
        c[0] = v[0];
        c[1] = e[2];

        c[2] = v[1];
        c[3] = e[1];

        c[4] = v[3];
        c[5] = e[3];

        c[6] = v[2];
        c[7] = e[0];

        return {2, 2, 2, 2};
    }
  }
};

class Corner : public mesh_entity_t<0, 2>{
public:
  Corner(){}

  Corner(mesh_topology_base_t &){}
};

class Wedge : public mesh_entity_t<1, 2>{
public:
  Wedge(){}

  Wedge(mesh_topology_base_t &){}
};

class TestMesh2dType{
public:
  static constexpr size_t num_dimensions = 2;

  static constexpr size_t num_domains = 2;

  using entity_types = std::tuple<
    std::pair<domain_<0>, Vertex>,
    std::pair<domain_<0>, Edge>,
    std::pair<domain_<0>, Cell>,
    std::pair<domain_<1>, Corner>,
    std::pair<domain_<1>, Wedge>>;

  using connectivities = 
    std::tuple<std::tuple<domain_<0>, Vertex, Edge>,
               std::tuple<domain_<0>, Vertex, Cell>,
               std::tuple<domain_<0>, Edge, Vertex>,
               std::tuple<domain_<0>, Edge, Cell>,
               std::tuple<domain_<0>, Cell, Vertex>,
               std::tuple<domain_<0>, Cell, Edge>>;

  using bindings = 
    std::tuple<
              std::tuple<domain_<0>, domain_<1>, Cell, Corner>,
              std::tuple<domain_<0>, domain_<1>, Vertex, Corner>,
              std::tuple<domain_<1>, domain_<0>, Corner, Cell>,
              std::tuple<domain_<1>, domain_<0>, Corner, Edge>,
              std::tuple<domain_<0>, domain_<1>, Edge, Corner>,
              std::tuple<domain_<1>, domain_<0>, Corner, Vertex>,
              std::tuple<domain_<0>, domain_<1>, Cell, Wedge>,
              std::tuple<domain_<0>, domain_<1>, Edge, Wedge>,
              std::tuple<domain_<0>, domain_<1>, Vertex, Wedge>,
              std::tuple<domain_<1>, domain_<0>, Wedge, Cell>,
              std::tuple<domain_<1>, domain_<0>, Wedge, Edge>,
              std::tuple<domain_<1>, domain_<0>, Wedge, Vertex>>;

  template<size_t M, size_t D>
  static mesh_entity_base_t<num_domains>*
  create_entity(mesh_topology_base_t* mesh, size_t num_vertices){
    switch(M){
      case 0:{
        switch(D){
          case 1:
            return mesh->make<Edge>();
          default:
            assert(false);
        }
        break;
      }
      case 1:{
        switch(D){
          case 0:
            return mesh->make<Corner>();
          case 1:
            return mesh->make<Wedge>();
          default:
            assert(false);
        }
        break;
      }
      default:
        assert(false);
    }
  }
};

using TestMesh = mesh_topology_t<TestMesh2dType>;

TEST(serialize, serialize_mesh) {

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
      auto c = mesh->make<Cell>();
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
  mesh->init_bindings<1>();

  uint64_t size;
  char* buf = mesh->serialize(size);

  FILE* fp = fopen("mesh.out", "wb");
  fwrite(buf, 1, size, fp);
  fclose(fp);
}

TEST(serialize, unserialize_mesh) {
  auto mesh = new TestMesh;

  FILE* fp = fopen("mesh.out", "rb");
  fseek(fp, 0, SEEK_END);
  size_t size = ftell(fp);
  rewind(fp);
  char* buf = (char*)malloc(size);
  fread(buf, 1, size, fp);
  fclose(fp);

  mesh->unserialize(buf);

  mesh->dump();

  for(auto cell : mesh->entities<2>()) {
    CINCH_CAPTURE() << "------- cell id: " << cell.id() << endl;
    for(auto corner : mesh->entities<0, 0, 1>(cell)) {
      CINCH_CAPTURE() << "--- corner id: " << corner.id() << endl;
    }
  }

  for(auto vertex : mesh->entities<0>()) {
    CINCH_CAPTURE() << "------- vertex id: " << vertex.id() << endl;
    for(auto corner : mesh->entities<0, 0, 1>(vertex)) {
      CINCH_CAPTURE() << "--- corner id: " << corner.id() << endl;
    }
  }

  for(auto corner : mesh->entities<0, 1>()) {
    CINCH_CAPTURE() << "------- corner id: " << corner.id() << endl;
    for(auto cell : mesh->entities<2, 1, 0>(corner)) {
      CINCH_CAPTURE() << "--- cell id: " << cell.id() << endl;
    }
  }

  for(auto corner : mesh->entities<0, 1>()) {
    CINCH_CAPTURE() << "------- corner id: " << corner.id() << endl;
    for(auto edge : mesh->entities<1, 1, 0>(corner)) {
      CINCH_CAPTURE() << "--- edge id: " << edge.id() << endl;
    }
  }

  for(auto edge : mesh->entities<1>()) {
    CINCH_CAPTURE() << "------- edge id: " << edge.id() << endl;
    for(auto corner : mesh->entities<0, 0, 1>(edge)) {
      CINCH_CAPTURE() << "--- corner id: " << corner.id() << endl;
    }
  }

  for(auto corner : mesh->entities<0, 1>()) {
    CINCH_CAPTURE() << "------- corner id: " << corner.id() << endl;
    for(auto vertex : mesh->entities<0, 1, 0>(corner)) {
      CINCH_CAPTURE() << "--- vertex id: " << vertex.id() << endl;
    }
  }

  for(auto cell : mesh->entities<2>()) {
    CINCH_CAPTURE() << "------- cell id: " << cell.id() << endl;
    for(auto wedge : mesh->entities<1, 0, 1>(cell)) {
      CINCH_CAPTURE() << "--- wedge id: " << wedge.id() << endl;
    }
  }

  for(auto edge : mesh->entities<1>()) {
    CINCH_CAPTURE() << "------- edge id: " << edge.id() << endl;
    for(auto wedge : mesh->entities<1, 0, 1>(edge)) {
      CINCH_CAPTURE() << "--- wedge id: " << wedge.id() << endl;
    }
  }

  for(auto vertex : mesh->entities<0>()) {
    CINCH_CAPTURE() << "------- vertex id: " << vertex.id() << endl;
    for(auto wedge : mesh->entities<1, 0, 1>(vertex)) {
      CINCH_CAPTURE() << "--- wedge id: " << wedge.id() << endl;
    }
  }

  for(auto wedge : mesh->entities<1, 1>()) {
    CINCH_CAPTURE() << "------- wedge id: " << wedge.id() << endl;
    for(auto cell : mesh->entities<2, 1, 0>(wedge)) {
      CINCH_CAPTURE() << "--- cell id: " << cell.id() << endl;
    }
  }

  for(auto wedge : mesh->entities<1, 1>()) {
    CINCH_CAPTURE() << "------- wedge id: " << wedge.id() << endl;
    for(auto edge : mesh->entities<1, 1, 0>(wedge)) {
      CINCH_CAPTURE() << "--- edge id: " << edge.id() << endl;
    }
  }

  for(auto wedge : mesh->entities<1, 1>()) {
    CINCH_CAPTURE() << "------- wedge id: " << wedge.id() << endl;
    for(auto vertex : mesh->entities<0, 1, 0>(wedge)) {
      CINCH_CAPTURE() << "--- vertex id: " << vertex.id() << endl;
    }
  }

  auto f1 = FILTER(ent){
    return ent.id() > 5;
  };

  auto f2 = FILTER(ent){
    return ent.id() < 3;
  };

  auto f3 = FILTER(ent){
    return ent.id() % 2 == 0;
  };

  auto r1 = mesh->entities<1>().filter(f1);

  auto r2 = mesh->entities<1>().filter(f2);

  auto r3 = mesh->entities<1>().filter(f3);

  auto r4 = r3;

  r4 << r2[0];

  for(auto edge : (r1 | r2) & r3){
    CINCH_CAPTURE() << "---- filter edge id: " << edge.id() << endl;   
  }

  ASSERT_TRUE(CINCH_EQUAL_BLESSED("serialize.blessed"));
}

struct state_user_meta_data_t {
  void initialize(){}
};

using state_t = 
  data_t<state_user_meta_data_t, default_data_storage_policy_t>;

static const size_t N = 10;

TEST(serialize, serialize_state) {

  state_t& state = state_t::instance();
  state.register_state<double, flecsi_internal>("mass", N, 0);
  state.register_state<double, flecsi_internal>("density", N, 0);

  auto am = state.dense_accessor<double, flecsi_internal>("mass");
  auto ad = state.dense_accessor<double, flecsi_internal>("density");

  for(size_t i = 0; i < N; ++i){
    am[i] = i;
    ad[i] = i;
  }

  uint64_t size;
  char* buf = state.serialize(size);

  FILE* fp = fopen("state.out", "wb");
  fwrite(buf, 1, size, fp);
  fclose(fp);
}

TEST(serialize, unserialize_state) {

  state_t& state = state_t::instance();
  state.reset();

  state.register_state<double, flecsi_internal>("mass", N, 0);
  state.register_state<double, flecsi_internal>("density", N, 0);

  FILE* fp = fopen("state.out", "rb");
  fseek(fp, 0, SEEK_END);
  size_t size = ftell(fp);
  rewind(fp);
  char* buf = (char*)malloc(size);
  fread(buf, 1, size, fp);
  fclose(fp);

  state.unserialize(buf);

  auto am = state.dense_accessor<double, flecsi_internal>("mass");
  auto ad = state.dense_accessor<double, flecsi_internal>("density");

  double sum = 0;

  for(size_t i = 0; i < N; ++i){
    sum += am[i];
    sum += ad[i];
  }

  ASSERT_TRUE(sum == 90);
}
