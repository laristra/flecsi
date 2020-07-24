#ifndef flecsi_topology_structured_storage_policy_h
#define flecsi_topology_structured_storage_policy_h

#include <array>
#include <vector>

#include "flecsi/topo/structured/box_types.hh"
#include "flecsi/topo/structured/mesh_utils.hh"
#include "flecsi/topo/structured/structured_index_space.hh"

namespace flecsi {
namespace topology {
namespace structured_impl {

template<size_t MESH_DIMENSION>
struct structured_topology_storage_u {
  using box_t = flecsi::topology::structured_impl::box_t;
  using box_color_t = flecsi::topology::structured_impl::box_color_t;

  using topology_type =
    typename gen_tuple_type<structured_index_space_u, MESH_DIMENSION>::type;

  topology_type topology;

  structured_topology_storage_u() {
    // auto & context_ = flecsi::execution::context_t::instance();
    // color = context_.color();
  }

}; // structured_topology_storage_u

} // namespace structured_impl
} // namespace topology
} // namespace flecsi
#endif
