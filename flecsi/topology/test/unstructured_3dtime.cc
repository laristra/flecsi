#include <cinchtest.h>
#include <iostream>

#include <flecsi/topology/mesh_topology.h>
#include <flecsi/topology/mesh_types.h>
#include <flecsi/utils/common.h>

using namespace std;
using namespace flecsi;
using namespace topology;

double wtime()
{
  double y = -1;
  struct timeval tm;
  gettimeofday(&tm,NULL);
  y = (double)(tm.tv_sec)+(double)(tm.tv_usec)*1.e-6;
  return y;
};

struct smi_perf
{
  double meshgen;
  double v2v, v2e, v2f, v2c;
  double e2v, e2e, e2f, e2c;
  double f2v, f2e, f2f, f2c;
  double c2v, c2e, c2f, c2c;
};

#define FILTER(E) [&](auto E) -> bool

class Vertex : public mesh_entity__<0, 1> {
public:
};

class Edge : public mesh_entity__<1, 1> {
public:
};

class Face : public mesh_entity__<2, 1> {
public:
  std::vector<size_t>
  create_entities(id_t cell_id, size_t dim, domain_connectivity__<3> & c, id_t * e){
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
};

class Cell : public mesh_entity__<3, 1> {
public:
  using id_t = flecsi::utils::id_t;

  std::vector<size_t>
  create_entities(id_t cell_id, size_t dim, domain_connectivity__<3> & c, id_t * e){
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

};

class TestMesh3dType {
public:
  static constexpr size_t num_dimensions = 3;

  static constexpr size_t num_domains = 1;

  using entity_types = std::tuple<
      std::tuple<index_space_<0>, domain_<0>, Vertex>,
      std::tuple<index_space_<1>, domain_<0>, Edge>,
      std::tuple<index_space_<2>, domain_<0>, Face>,
      std::tuple<index_space_<3>, domain_<0>, Cell>>; 

  using connectivities = std::tuple<
      std::tuple<index_space_<4>, domain_<0>, Vertex, Edge>,
      std::tuple<index_space_<5>, domain_<0>, Vertex, Face>,
      std::tuple<index_space_<6>, domain_<0>, Vertex, Cell>,
      std::tuple<index_space_<7>, domain_<0>, Edge, Vertex>,
      std::tuple<index_space_<8>, domain_<0>, Edge, Face>,
      std::tuple<index_space_<9>, domain_<0>, Edge, Cell>,
      std::tuple<index_space_<10>, domain_<0>, Cell, Vertex>,
      std::tuple<index_space_<11>, domain_<0>, Cell, Edge>,
      std::tuple<index_space_<12>, domain_<0>, Cell, Face>>;

  using bindings = std::tuple<>;  

  using id_t = flecsi::utils::id_t;

  template<size_t M, size_t D, typename ST>
  static mesh_entity_base__<num_domains> *
  create_entity(mesh_topology_base__<ST> * mesh, size_t num_vertices, id_t & id) {
    switch (M) {
      case 0: {
        switch (D) {
          case 1:
            return mesh->template make<Edge>();
          case 2:
            return mesh->template make<Face>();
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

using TestMesh = mesh_topology__<TestMesh3dType>;

//----------------------------------------------------------------------------//
//// User driver.
////----------------------------------------------------------------------------//
namespace flecsi {
namespace execution {
void driver(int argc, char ** argv) {};
} //flecsi
} //execution

TEST(mesh_topology, traversal) {

  size_t Nx = 1;
  size_t Ny = 1;
  size_t Nz = 1;

  smi_perf *sp = new smi_perf; 

  auto ms = new mesh_storage__<TestMesh3dType::num_dimensions, 
			       TestMesh3dType::num_domains,
                               num_index_subspaces__<TestMesh3dType>::value>();  
  auto mesh = new TestMesh(ms);

  vector<Vertex *> vs;

  for (size_t k = 0; k < Nz + 1; ++k) {
    for (size_t j = 0; j < Ny + 1; ++j) {
     for (size_t i = 0; i < Nx + 1; ++i) {
      auto v = mesh->make<Vertex>();
      vs.push_back(v);
     }
   }
  }
 
  size_t width = Nx + 1;
  size_t height = Ny + 1; 
  for (size_t k = 0; k < Nz; ++k) {
    for (size_t j = 0; j < Ny; ++j) {
     for (size_t i = 0; i < Nx; ++i) {
      auto c = mesh->make<Cell>();

      mesh->init_cell<0>(
          c, {vs[i + j * width + k * width*height], 
              vs[i + 1 + j * width + k * width*height], 
              vs[i + 1 + (j+1) * width + k * width*height], 
              vs[i + (j+1) * width + k * width*height], 
              vs[i + j * width + (k+1) * width*height], 
              vs[i + 1 + j * width + (k+1) * width*height], 
              vs[i + 1 + (j+1) * width + (k+1) * width*height], 
              vs[i + (j+1) * width + (k+1) * width*height]}); 
     }
    }
  }

  mesh->init<0>();

  mesh->dump();
}
