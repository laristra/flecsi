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
class ntree_topology;

template<typename>
class set_topology;

template<typename>
class structured_mesh_topology;

template<typename>
class unstructured_mesh_topology;

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
struct core<T, std::void_t<utils::base_specialization_t<ntree_topology, T>>> {
  using type = utils::base_specialization_t<ntree_topology, T>;
};
template<class T>
struct core<T, std::void_t<utils::base_specialization_t<set_topology, T>>> {
  using type = utils::base_specialization_t<set_topology, T>;
};
template<class T>
struct core<T,
  std::void_t<utils::base_specialization_t<structured_mesh_topology, T>>> {
  using type = utils::base_specialization_t<structured_mesh_topology, T>;
};
template<class T>
struct core<T,
  std::void_t<utils::base_specialization_t<unstructured_mesh_topology, T>>> {
  using type = utils::base_specialization_t<unstructured_mesh_topology, T>;
};

inline std::size_t next_id;
// Use functions because these are needed during non-local initialization:
template<class>
std::size_t
id() {
  static auto ret = next_id++;
  return ret;
}
} // namespace detail

template<class T>
using core_t = typename detail::core<T>::type;

template<class T>
std::size_t
id() {
  return detail::id<std::remove_cv_t<T>>();
}

} // namespace topology
} // namespace flecsi

#endif
