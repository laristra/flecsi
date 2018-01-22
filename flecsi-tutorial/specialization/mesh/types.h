#pragma once

#include <flecsi/topology/mesh.h>
#include <flecsi/topology/mesh_topology.h>
#include <specialization/mesh/entity_types.h>

namespace flecsi {
namespace tutorial {

enum index_spaces : size_t {
  vertices,
  edges,
  cells,
  vertices_to_cells,
  cells_to_vertices
}; // enum index_spaces

struct specialization_mesh_policy_t
{

  using id_t = flecsi::utils::id_t;

  flecsi_register_number_dimensions(2);
  flecsi_register_number_domains(1);

  flecsi_register_entity_types(
    flecsi_entity_type(index_spaces::vertices, 0, vertex_t),
    flecsi_entity_type(index_spaces::cells, 0, cell_t)
  );

  flecsi_register_connectivities(
    flecsi_connectivity(index_spaces::cells_to_vertices, 0, cell_t, vertex_t)
  );

  flecsi_register_bindings();

  template<
    size_t M,
    size_t D,
    typename ST
  >
  static flecsi::topology::mesh_entity_base__<1> *
  create_entity(
    flecsi::topology::mesh_topology_base__<ST>* mesh,
    size_t num_vertices,
    id_t const & id
  )
  {
    return nullptr;
  } // create_entity

}; // struct specialization_mesh_policy_t

} // namespace tutorial
} // namespace flecsi
