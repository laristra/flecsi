#pragma once

#include <vector>

#include <specialization/mesh/types.h>

namespace flecsi {
namespace tutorial {

//----------------------------------------------------------------------------//
// Mesh Specialization
//----------------------------------------------------------------------------//

struct specialization_mesh_t :
  public flecsi::topology::mesh_topology__<specialization_mesh_policy_t>
{

}; // specialization_mesh_t

using mesh_t = specialization_mesh_t;

//----------------------------------------------------------------------------//
// Type Definitions
//----------------------------------------------------------------------------//

template<
  size_t PS>
using mesh = data_client_handle__<mesh_t, PS>;

} // namespace tutorial
} // namespace flecsi
