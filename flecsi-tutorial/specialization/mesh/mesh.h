#pragma once

#include <iostream>
#include <vector>

#include <flecsi/data/data_client_handle.h>
#include <flecsi/data/dense_accessor.h>
#include <flecsi/data/common/privilege.h>
#include <specialization/mesh/types.h>

namespace flecsi {
namespace tutorial {

//----------------------------------------------------------------------------//
// Mesh Specialization
//----------------------------------------------------------------------------//

struct specialization_mesh_t :
  public flecsi::topology::mesh_topology__<specialization_mesh_policy_t>
{

  void print(const char * string) {
    std::cout << string << std::endl;
  } // print

  auto cells() {
    return entities<2,0>();
  } // cells

  auto cells(partition_t p) {
    return entities<2,0>(p);
  } // cells
}; // specialization_mesh_t

using mesh_t = specialization_mesh_t;

//----------------------------------------------------------------------------//
// Type Definitions
//----------------------------------------------------------------------------//

template<
  size_t PS>
using mesh = data_client_handle__<mesh_t, PS>;

template<
  size_t SHARED_PRIVILEGES
>
using field = dense_accessor<double, rw, SHARED_PRIVILEGES, ro>;

} // namespace tutorial
} // namespace flecsi
