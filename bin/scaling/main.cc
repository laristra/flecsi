#include <cinchtest.h>
#include <iostream>

#include "flecsi/utils/common.h"
#include "flecsi/topology/mesh_topology.h"

using namespace std;
using namespace flecsi;
using namespace topology;

$ENTITIES

class TestMesh2dType{
public:
  static constexpr size_t num_dimensions = $NUM_DIMENSIONS;

  static constexpr size_t num_domains = $NUM_DOMAINS;

  using entity_types = std::tuple<
    $ENTITY_TYPES
    >;

  using connectivities = std::tuple<
    $CONNECTIVITIES
    >;

  using bindings = std::tuple<>;

  template<size_t M, size_t D>
  static mesh_entity_base_t<num_domains>*
  create_entity(mesh_topology_base_t* mesh, size_t num_vertices){
    switch(M){
      $CREATE_ENTITY
      default:
        assert(false && "invalid domain");
    }
  }
};

using TestMesh = mesh_topology_t<TestMesh2dType>;

TEST(mesh_topology, traversal) {

  size_t width = 2;
  size_t height = 2;

  auto mesh = new TestMesh;
  
  $INIT

  $TRAVERSAL
}
