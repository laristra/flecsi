#include <cinchtest.h>
#include <iostream>

#include <flecsi/topology/mesh_topology.h>
#include <flecsi/topology/mesh_types.h>
#include <flecsi/utils/common.h>

using namespace std;
using namespace flecsi;
using namespace topology;

#define FILTER(E) [&](auto E) -> bool

class Vertex : public mesh_entity__<0, 2> {
public:
  Vertex() {}

  template<size_t M>
  uint64_t precedence() const {
    return 0;
  }
};

class Edge : public mesh_entity__<1, 2> {
public:
  Edge() {}
};

class Cell : public mesh_entity__<2, 2> {
public:
  using id_t = flecsi::utils::id_t;

  Cell() {}

  void set_precedence(size_t dim, uint64_t precedence) {}

  std::vector<size_t> create_entities(
      id_t cell_id,
      size_t dim,
      domain_connectivity__<2> & c,
      id_t * e) {

    id_t * v = c.get_entities(cell_id, 0);

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

  index_vector_t create_bound_entities(
      size_t from_domain,
      size_t to_domain,
      size_t create_dim,
      id_t cell_id,
      domain_connectivity__<2> & primal_conn,
      domain_connectivity__<2> & domain_conn,
      id_t * c) {

    id_t * v = primal_conn.get_entities(cell_id, 0);
    id_t * e = primal_conn.get_entities(cell_id, 1);

    switch (create_dim) {
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

        c[9] = v[3];
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

    return index_vector_t();
  }
};

class Corner : public mesh_entity__<0, 2> {
public:
  Corner() {}
};

class Wedge : public mesh_entity__<1, 2> {
public:
  Wedge() {}
};

class TestMesh2dType {
public:
  static constexpr size_t num_dimensions = 2;

  static constexpr size_t num_domains = 2;

  using entity_types = std::tuple<
      std::pair<domain_<0>, Vertex>,
      std::pair<domain_<0>, Edge>,
      std::pair<domain_<0>, Cell>,
      std::pair<domain_<1>, Corner>,
      std::pair<domain_<1>, Wedge>>;

  using connectivities = std::tuple<
      std::tuple<domain_<0>, Vertex, Edge>,
      std::tuple<domain_<0>, Vertex, Cell>,
      std::tuple<domain_<0>, Edge, Vertex>,
      std::tuple<domain_<0>, Edge, Cell>,
      std::tuple<domain_<0>, Cell, Vertex>,
      std::tuple<domain_<0>, Cell, Edge>>;

  using bindings = std::tuple<
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

  using index_subspaces = std::tuple<
    std::tuple<index_space_<0>, index_subspace_<0>>
  >;

  template<size_t M, size_t D, typename ST>
  static mesh_entity_base__<num_domains> *
  create_entity(mesh_topology_base__<ST> * mesh, size_t num_vertices) {
    switch (M) {
      case 0: {
        switch (D) {
          case 1:
            return mesh->template make<Edge>();
          default:
            assert(false);
        }
        break;
      }
      case 1: {
        switch (D) {
          case 0:
            return mesh->template make<Corner, 1>();
          case 1:
            return mesh->template make<Wedge, 1>();
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

using TestMesh = mesh_topology__<TestMesh2dType>;

TEST(mesh_topology, traversal) {

  size_t width = 2;
  size_t height = 2;

  auto mesh = new TestMesh;

  vector<Vertex *> vs;

  for (size_t j = 0; j < height + 1; ++j) {
    for (size_t i = 0; i < width + 1; ++i) {
      auto v = mesh->make<Vertex>();
      vs.push_back(v);
    }
  }

  size_t width1 = width + 1;
  for (size_t j = 0; j < height; ++j) {
    for (size_t i = 0; i < width; ++i) {
      auto c = mesh->make<Cell>();

      mesh->init_cell<0>(
          c, {vs[i + j * width1], vs[i + (j + 1) * width1],
              vs[i + 1 + j * width1], vs[i + 1 + (j + 1) * width1]});
    }
  }

  mesh->init<0>();
  mesh->init_bindings<1>();

  // mesh->dump();

  for (auto cell : mesh->entities<2>()) {
    CINCH_CAPTURE() << "------- cell id: " << cell.id() << endl;
    for (auto corner : mesh->entities<0, 0, 1>(cell)) {
      CINCH_CAPTURE() << "--- corner id: " << corner.id() << endl;
    }
  }

  for (auto vertex : mesh->entities<0>()) {
    CINCH_CAPTURE() << "------- vertex id: " << vertex.id() << endl;
    for (auto corner : mesh->entities<0, 0, 1>(vertex)) {
      CINCH_CAPTURE() << "--- corner id: " << corner.id() << endl;
    }
  }

  for (auto corner : mesh->entities<0, 1>()) {
    CINCH_CAPTURE() << "------- corner id: " << corner.id() << endl;
    for (auto cell : mesh->entities<2, 1, 0>(corner)) {
      CINCH_CAPTURE() << "--- cell id: " << cell.id() << endl;
    }
  }

  for (auto corner : mesh->entities<0, 1>()) {
    CINCH_CAPTURE() << "------- corner id: " << corner.id() << endl;
    for (auto edge : mesh->entities<1, 1, 0>(corner)) {
      CINCH_CAPTURE() << "--- edge id: " << edge.id() << endl;
    }
  }

  for (auto edge : mesh->entities<1>()) {
    CINCH_CAPTURE() << "------- edge id: " << edge.id() << endl;
    for (auto corner : mesh->entities<0, 0, 1>(edge)) {
      CINCH_CAPTURE() << "--- corner id: " << corner.id() << endl;
    }
  }

  for (auto corner : mesh->entities<0, 1>()) {
    CINCH_CAPTURE() << "------- corner id: " << corner.id() << endl;
    for (auto vertex : mesh->entities<0, 1, 0>(corner)) {
      CINCH_CAPTURE() << "--- vertex id: " << vertex.id() << endl;
    }
  }

  for (auto cell : mesh->entities<2>()) {
    CINCH_CAPTURE() << "------- cell id: " << cell.id() << endl;
    for (auto wedge : mesh->entities<1, 0, 1>(cell)) {
      CINCH_CAPTURE() << "--- wedge id: " << wedge.id() << endl;
    }
  }

  for (auto edge : mesh->entities<1>()) {
    CINCH_CAPTURE() << "------- edge id: " << edge.id() << endl;
    for (auto wedge : mesh->entities<1, 0, 1>(edge)) {
      CINCH_CAPTURE() << "--- wedge id: " << wedge.id() << endl;
    }
  }

  for (auto vertex : mesh->entities<0>()) {
    CINCH_CAPTURE() << "------- vertex id: " << vertex.id() << endl;
    for (auto wedge : mesh->entities<1, 0, 1>(vertex)) {
      CINCH_CAPTURE() << "--- wedge id: " << wedge.id() << endl;
    }
  }

  for (auto wedge : mesh->entities<1, 1>()) {
    CINCH_CAPTURE() << "------- wedge id: " << wedge.id() << endl;
    for (auto cell : mesh->entities<2, 1, 0>(wedge)) {
      CINCH_CAPTURE() << "--- cell id: " << cell.id() << endl;
    }
  }

  for (auto wedge : mesh->entities<1, 1>()) {
    CINCH_CAPTURE() << "------- wedge id: " << wedge.id() << endl;
    for (auto edge : mesh->entities<1, 1, 0>(wedge)) {
      CINCH_CAPTURE() << "--- edge id: " << edge.id() << endl;
    }
  }

  for (auto wedge : mesh->entities<1, 1>()) {
    CINCH_CAPTURE() << "------- wedge id: " << wedge.id() << endl;
    for (auto vertex : mesh->entities<0, 1, 0>(wedge)) {
      CINCH_CAPTURE() << "--- vertex id: " << vertex.id() << endl;
    }
  }

  auto f1 = FILTER(ent) {
    return ent.id() > 5;
  };

  auto f2 = FILTER(ent) {
    return ent.id() < 3;
  };

  auto f3 = FILTER(ent) {
    return ent.id() % 2 == 0;
  };

  auto r1 = mesh->entities<1>().filter(f1);

  auto r2 = mesh->entities<1>().filter(f2);

  auto r3 = mesh->entities<1>().filter(f3);

  auto r4 = r3;

  r4 << r2[0];

  for (auto edge : (r1 | r2) & r3) {
    CINCH_CAPTURE() << "---- filter edge id: " << edge.id() << endl;
  }

  ASSERT_TRUE(CINCH_EQUAL_BLESSED("bindings.blessed"));
}
