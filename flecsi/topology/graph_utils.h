/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_topology_graph_utils_h
#define flecsi_topology_graph_utils_h

#include "flecsi/topology/graph_definition.h"
#include "flecsi/utils/logging.h"
#include "flecsi/utils/set_utils.h"

///
/// \file
/// \date Initial file creation: Nov 21, 2016
///

namespace flecsi {
namespace topology {

///
/// Find the neighbors of the given entity id.
///
/// \tparam from_dim The topological dimension of the entity for which
///                  the neighbor information is being requested.
/// \tparam to_dim The topological dimension to search for neighbors.
/// \tparam thru_dim The topological dimension through which the neighbor
///                  connection exists.
///
/// \param gd The graph definition containing the topological connectivity
///           information.
/// \param entity_id The id of the entity in from_dim for which the neighbors
///           are to be found.
///
template<
  size_t from_dim,
  size_t to_dim,
  size_t thru_dim
>
std::set<size_t>
entity_neighbors(
  graph_definition_t & gd,
  size_t entity_id
)
{
  // Get the vertices of the requested id
  auto vertices = gd.vertex_set(from_dim, entity_id);

  // Put the results into set form
  std::set<size_t> neighbors;

  // Go through the entities of the to_dim
  for(size_t e(0); e<gd.num_entities(to_dim); ++e) {

    // Skip the input id if the dimensions are the same
    if(from_dim == to_dim && e == entity_id) {
      continue;
    } // if

    // Get the vertices that define the current entity from the to_dim
    auto other = gd.vertex_set(to_dim, e);

    // Get the intersection set
    auto intersect = flecsi::utils::set_intersection(vertices, other);

    // Add this entity id if the intersection shares at least
    // intersections vertices
    if(intersect.size() > thru_dim) {
      neighbors.insert(e);
    } // if
  } // for

  return neighbors;
} // entity_neighbors

///
/// Return the dependency closure of the given set.
///
/// \tparam from_dim The topological dimension of the entity for which
///                  the neighbor information is being requested.
/// \tparam to_dim The topological dimension to search for neighbors.
/// \tparam thru_dim The topological dimension through which the neighbor
///                  connection exists.
///
/// \param gd The graph definition containing the topological connectivity
///           information.
/// \param indices The entity indeces of the initial set.
/// \param intersections The number of intersections that constitute a
///                      neighboring entity.
///
template<
  size_t from_dim,
  size_t to_dim,
  size_t thru_dim
>
std::set<size_t>
entity_closure(
  graph_definition_t & gd,
  std::set<size_t> indices
)
{
  // Closure should include the initial set
  std::set<size_t> closure = indices;

  // Iterate over the entity indices and add all neighbors
  for(auto i: indices) {
    auto ncurr =
      entity_neighbors<from_dim, to_dim, thru_dim>(gd, i);

    closure = flecsi::utils::set_union(ncurr, closure);
  } // for

  return closure;
} // entity_closure

///
/// Return the cells that reference the given vertex id.
///
/// \tparam by_dim The topological dimension of the entities that
///                reference the vertex.
///
/// \param gd The graph definition containing the topological connectivity
///           information.
/// \param id The id of the vertex.
///
template<
  size_t by_dim
>
std::set<size_t>
vertex_referencers(
  graph_definition_t & gd,
  size_t id
)
{
  std::set<size_t> referencers;

  // Iterate over entities adding any entity that contains
  // the vertex id to the set.
  for(size_t e(0); e<gd.num_entities(by_dim); ++e) {

    // Get the vertex ids of current cell
    auto eset = gd.vertex_set(by_dim, e);

    // If the cell references this vertex add it
    if(eset.find(id) != eset.end()) {
      referencers.insert(e);
    } // if
  } // for

  return referencers;
} // vertex_referencers

///
/// Return the union of all vertices that are referenced by at least
/// one of the entities in the given set of indices.
///
/// \tparam by_dim The topological dimension of the entities that
///                reference the vertex.
///
/// \param gd The graph definition containing the topological connectivity
///           information.
/// \param indices The entity indeces.
///
template<
  size_t by_dim
>
std::set<size_t>
vertex_closure(
  graph_definition_t & gd,
  std::set<size_t> indices
)
{
  std::set<size_t> closure;

  // Iterate over the entities in indices and add any vertices that are
  // referenced by one of the entity indices
  for(auto i: indices) {
    auto vset = gd.vertex_set(by_dim, i);

    closure = flecsi::utils::set_union(closure, vset);
  } // for

  return closure;
} // vertex_closure

} // namespace topology
} // namespace flecsi

#endif // flecsi_topology_graph_utils_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
