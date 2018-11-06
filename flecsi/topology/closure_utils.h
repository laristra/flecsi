/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

/*! @file */

#include <flecsi/topology/mesh_definition.h>
#include <flecsi/utils/logging.h>
#include <flecsi/utils/set_utils.h>
#include <flecsi/utils/type_traits.h>

namespace flecsi {
namespace topology {

/*!
  Find the neighbors of the given entity id.

  @tparam from_dim The topological dimension of the entity for which
                   the neighbor information is being requested.
  @tparam to_dim The topological dimension to search for neighbors.
  @tparam thru_dim The topological dimension through which the neighbor
                   connection exists.

  @param md The mesh definition containing the topological connectivity
            information.
  @param entity_id The id of the entity in from_dim for which the neighbors
            are to be found.
 */

template<size_t from_dim, size_t to_dim, size_t thru_dim, size_t D>
std::set<size_t>
entity_neighbors(const mesh_definition_u<D> & md, size_t entity_id) {
  // Get the vertices of the requested id
  auto vertices = md.entities_set(from_dim, 0, entity_id);

  // Put the results into set form
  std::set<size_t> neighbors;

  // Go through the entities of the to_dim
  for(size_t e(0); e < md.num_entities(to_dim); ++e) {

    // Skip the input id if the dimensions are the same
    if(from_dim == to_dim && e == entity_id) {
      continue;
    } // if

    // Get the vertices that define the current entity from the to_dim
    auto other = md.entities_set(to_dim, 0, e);

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

/*!
  Return the dependency closure of the given set.

  @tparam from_dim The topological dimension of the entity for which
                   the neighbor information is being requested.
  @tparam to_dim   The topological dimension to search for neighbors.
  @tparam thru_dim The topological dimension through which the neighbor
                   connection exists.

  @param md            The mesh definition containing the topological
                       connectivity information.
  @param indices       The entity indices of the initial set.
  @param intersections The number of intersections that constitute a
                       neighboring entity.
 */

template<size_t from_dim,
  size_t to_dim,
  size_t thru_dim,
  size_t D,
  typename U,
  typename = std::enable_if_t<utils::is_iterative_container_v<U>>>
std::set<size_t>
entity_neighbors(const mesh_definition_u<D> & md, U && indices) {
  clog_assert(from_dim == to_dim, "from_dim does not equal to to_dim");

  // Closure should include the initial set
  std::set<size_t> closure(
    std::forward<U>(indices).begin(), std::forward<U>(indices).end());

  using entityid = size_t;
  using vertexid = size_t;

  // essentially vertex to cell and cell to cell through vertices
  // connectivity.
  std::map<vertexid, std::vector<entityid>> vertex2entities;
  std::map<entityid, std::vector<entityid>> entity_neighbors;

  // build global entity to entity through # of shared vertices connectivity
  for(size_t cell(0); cell < md.num_entities(from_dim); ++cell) {
    std::map<entityid, size_t> cell_counts;
    for(auto vertex : md.entities(from_dim, 0, cell)) {
      // build vertex to cell connectivity, O(n_cells * n_polygon_sides)
      // of insert().
      vertex2entities[vertex].push_back(cell);

      // Count the number of times this cell shares a common vertex with
      // some other cell. O(n_cells * n_polygon_sides * vertex to cells degrees)
      for(auto other : vertex2entities[vertex]) {
        // for (auto other : md.entities(0, from_dim, vertex)) {
        if(other != cell)
          cell_counts[other] += 1;
      }
    }

    for(auto count : cell_counts) {
      if(count.second > thru_dim) {
        // append cell to cell through "dimension" connectivity, we need to
        // add both directions
        entity_neighbors[cell].push_back(count.first);
        entity_neighbors[count.first].push_back(cell);
      }
    }
  }

  for(auto i : indices) {
    for(auto n : entity_neighbors[i]) {
      closure.insert(n);
    }
  }

  return closure;
} // entity_closure

/*!
  Return the cells that reference the given vertex id.

  @tparam by_dim The topological dimension of the entities that
                 reference the vertex.

  @param md The mesh definition containing the topological connectivity
            information.
  @param id The id of the vertex.
 */

template<size_t from_dim, size_t to_dim, size_t D>
std::set<size_t>
entity_referencers(const mesh_definition_u<D> & md, size_t id) {
  std::set<size_t> referencers;

  // Iterate over entities adding any entity that contains
  // the vertex id to the set.
  for(size_t e(0); e < md.num_entities(from_dim); ++e) {

    // Get the vertex ids of current cell
    const auto & eset = md.entities(from_dim, to_dim, e);

    // If the cell references this vertex add it
    if(std::find(eset.begin(), eset.end(), id) != eset.end())
      referencers.insert(e);

  } // for

  return referencers;
} // entity_referencers

/*!
  Return the union of all vertices that are referenced by at least
  one of the entities in the given set of indices.

  @tparam by_dim The topological dimension of the entities that
                 reference the vertex.

  @param md      The mesh definition containing the topological
                 connectivity information.
  @param indices The entity indeces.
 */

template<size_t from_dim, size_t to_dim, size_t D, typename U>
std::set<size_t>
entity_closure(const mesh_definition_u<D> & md, U && indices) {
  std::set<size_t> closure;

  // Iterate over the entities in indices and add any vertices that are
  // referenced by one of the entity indices
  for(auto i : std::forward<U>(indices)) {
    const auto & vset = md.entities(from_dim, to_dim, i);
    closure.insert(vset.begin(), vset.end());
  } // for

  return closure;
} // entity_closure

} // namespace topology
} // namespace flecsi
