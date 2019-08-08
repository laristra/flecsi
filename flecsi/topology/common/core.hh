/// \file

#ifndef FLECSI_TOPOLOGY_CORE_HH
#define FLECSI_TOPOLOGY_CORE_HH

#include "flecsi/utils/type_traits.hh"

namespace flecsi {
namespace topology {

// Declarations for the base topology types.

struct global_topology_t;
struct index_topology_t;

template<typename>
class ntree_topology_u;

template<typename>
class set_topology_u;

template<typename>
class structured_mesh_topology_u;

template<typename>
class unstructured_mesh_topology_u;

namespace detail {
template<class, class = void>
struct core;
template<class T>
struct core<T, std::enable_if_t<std::is_base_of_v<global_topology_t, T>>> {
  using type = global_topology_t;
};
template<class T>
struct core<T, std::enable_if_t<std::is_base_of_v<index_topology_t, T>>> {
  using type = index_topology_t;
};
template<class T>
struct core<T, std::void_t<utils::base_specialization_t<ntree_topology_u, T>>> {
  using type = utils::base_specialization_t<ntree_topology_u, T>;
};
template<class T>
struct core<T, std::void_t<utils::base_specialization_t<set_topology_u, T>>> {
  using type = utils::base_specialization_t<set_topology_u, T>;
};
template<class T>
struct core<T,
  std::void_t<utils::base_specialization_t<structured_mesh_topology_u, T>>> {
  using type = utils::base_specialization_t<structured_mesh_topology_u, T>;
};
template<class T>
struct core<T,
  std::void_t<utils::base_specialization_t<unstructured_mesh_topology_u, T>>> {
  using type = utils::base_specialization_t<unstructured_mesh_topology_u, T>;
};
} // namespace detail

template<class T>
using core_t = typename detail::core<T>::type;

} // namespace topology
} // namespace flecsi

#endif
