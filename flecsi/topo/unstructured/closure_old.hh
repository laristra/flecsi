/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Triad National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

/*! @file */

#include "flecsi/flog.hh"
#include "flecsi/util/set_utils.hh"
#include "flecsi/util/type_traits.hh"

#include <map>
#include <set>
#include <vector>

namespace flecsi {
namespace topo {
namespace unstructured_impl {

template<typename Policy>
unstructured_base::coloring
closure(typename Policy::definition const & md,
  std::vector<size_t> const & primary_vec,
  MPI_Comm comm) {
  using namespace flecsi::util;

  unstructured_base::coloring coloring;

  constexpr size_t depth = Policy::primary::depth;
  constexpr size_t dimension = Policy::primary::dimension;
  constexpr size_t thru_dimension = Policy::primary::thru_dimension;

  auto communicator = typename Policy::communicator(comm);

  auto rank = communicator.rank();
  auto size = communicator.size();

  coloring.colors = size;
  coloring.idx_colorings.resize(1 + Policy::auxiliary_colorings);
  coloring.distribution.resize(communicator.size());
  coloring.distribution[rank].resize(1 + Policy::auxiliary_colorings);

  auto & primary_coloring = coloring.idx_colorings[0];
  auto & primary_coloring_info = coloring.distribution[rank][0];

  auto aux_coloring = coloring.idx_colorings.begin();
  ++aux_coloring; // + 1;
  auto aux_coloring_info = coloring.distribution[rank].begin();
  ++aux_coloring_info; // + 1;

  primary_coloring.primary =
    std::set<size_t>(primary_vec.begin(), primary_vec.end());
  auto & primary = primary_coloring.primary;
  std::set<size_t> aggregate_near_neighbors;
  auto core = primary;
  std::set<size_t> closure;

  // Collect neighbors up to depth deep
  for(size_t i{0}; i < depth; ++i) {

    // Form the closure of the current core
    closure = entity_neighbors<typename Policy::definition,
      dimension,
      dimension,
      thru_dimension>(md, core);

    // Subtract off just the new nearest neighbors
    auto nearest_neighbors = set_difference(closure, core);

    // Add these to the aggregate
    aggregate_near_neighbors =
      set_union(aggregate_near_neighbors, nearest_neighbors);

    // Update the core set
    core = set_union(core, nearest_neighbors);
  } // for

  // Form the closure of the collected neighbors
  auto near_neighbor_closure = entity_neighbors<typename Policy::definition,
    dimension,
    dimension,
    thru_dimension>(md, aggregate_near_neighbors);

  // Clean primaries from closure to get all neighbors
  auto aggregate_neighbors = set_difference(near_neighbor_closure, primary);

  /*
    Get the intersection of our near neighbors with the near neighbors
    of other ranks. This map of sets will only be populated with
    intersections that are non-empty
   */
  auto closure_intersection_map =
    communicator.get_intersection_info(aggregate_near_neighbors);

  /*
    Get the rank and offset information for our near neighbor
    dependencies. This also gives information about the ranks
    that access our shared entities.
   */
  auto primary_nn_info =
    communicator.get_primary_info(primary, aggregate_near_neighbors);

  /*
    Get the rank and offset information for all relevant neighbor
    dependencies. This information will be necessary for determining
    shared vertices.
   */
  auto primary_all_info =
    communicator.get_primary_info(primary, aggregate_neighbors);

  // Create a map version of the local info for lookups below.
  std::unordered_map<size_t, size_t> primary_indices_map;
  {
    size_t offset{0};
    for(auto i : primary) {
      primary_indices_map[offset++] = i;
    } // for
  } // scope

  // Create a map version of the remote info for lookups below.
  std::unordered_map<size_t, entity_info> remote_info_map;
  for(auto i : std::get<1>(primary_all_info)) {
    remote_info_map[i.id] = i;
  } // for

  // Populate exclusive and shared primary information
  {
    size_t offset{0};

    for(auto i : std::get<0>(primary_nn_info)) {
      if(i.size()) {
        primary_coloring.shared.insert(
          entity_info(primary_indices_map[offset], rank, offset, i));

        // Collect all colors with whom we require communication
        // to send shared information.
        primary_coloring_info.dependants =
          set_union(primary_coloring_info.dependants, i);
      }
      else {
        primary_coloring.exclusive.insert(
          entity_info(primary_indices_map[offset], rank, offset, i));
      } // if

      ++offset;
    } // for
  } // scope

  // Populate ghost primary information
  {
    for(auto i : std::get<1>(primary_nn_info)) {
      primary_coloring.ghost.insert(i);

      // Collect all colors with whom we require communication
      // to receive ghost information.
      primary_coloring_info.dependencies.insert(i.rank);
    } // for
  } // scope

  primary_coloring_info.exclusive = primary_coloring.exclusive.size();
  primary_coloring_info.shared = primary_coloring.shared.size();
  primary_coloring_info.ghost = primary_coloring.ghost.size();

  std::unordered_map<size_t, entity_info> shared_primary_map;
  {
    for(auto i : primary_coloring.shared) {
      shared_primary_map[i.id] = i;
    } // for
  } // scope

  //--------------------------------------------------------------------------//
  // Lambda function to apply to auxiliary index spaces
  //--------------------------------------------------------------------------//

  auto color_entity = [&](size_t idx, auto tuple_element) {
    using auxiliary_type = decltype(tuple_element);

    constexpr size_t dimension = auxiliary_type::dimension;
    constexpr size_t primary_dimension = auxiliary_type::primary_dimension;

    auto & aux_coloring = coloring.idx_colorings[idx + 1];
    auto & aux_coloring_info = coloring.distribution[rank][idx + 1];

    // Form the closure of this entity from the primary
    // auto auxiliary_closure =
    //   entity_closure<primary_dimension, dimension>(md,
    //   near_neighbor_closure);
    // TODO: near_neighbor_closure seemed to be missing information
    auto auxiliary_closure =
      entity_closure<typename Policy::definition, primary_dimension, dimension>(
        md, closure);

    // Assign entity ownership
    std::vector<std::set<size_t>> entity_requests(size);
    std::set<entity_info> entities;

    {
      size_t offset{0};
      for(auto i : auxiliary_closure) {
        auto referencers = entity_referencers<typename Policy::definition,
          primary_dimension,
          dimension>(md, i);

        size_t min_rank(std::numeric_limits<size_t>::max());
        std::set<size_t> shared_entities;

        // Iterate the direct referencers to assign entity ownership
        for(auto c : referencers) {

          // Check the remote info map to see if this primary is
          // off-color. If it is, compare it's rank for
          // the ownership logic below.
          if(remote_info_map.find(c) != remote_info_map.end()) {
            min_rank = std::min(min_rank, remote_info_map.at(c).rank);
            shared_entities.insert(remote_info_map.at(c).rank);
          }
          else {
            // If the referencing primary isn't in the remote info map
            // it is a local primary.

            // Add our rank to compare for ownership.
            min_rank = std::min(min_rank, size_t(rank));

            // If the local primary is shared, we need to add all of
            // the ranks that reference it.
            if(shared_primary_map.find(c) != shared_primary_map.end())
              shared_entities.insert(shared_primary_map.at(c).shared.begin(),
                shared_primary_map.at(c).shared.end());
          } // if

          // Iterate through the closure intersection map to see if the
          // indirect reference is part of another rank's closure, i.e.,
          // that it is an indirect dependency.
          for(auto ci : closure_intersection_map) {
            if(ci.second.find(c) != ci.second.end()) {
              shared_entities.insert(ci.first);
            } // if
          } // for
        } // for
        if(min_rank == rank) {
          // This is a entity that belongs to our rank.
          auto entry = entity_info(i, rank, offset++, shared_entities);
          entities.insert(entry);
        }
        else {
          // Add remote entity to the request for offset information.
          entity_requests[min_rank].insert(i);
        } // if
      } // for
    } // scope

    auto entity_offset_info =
      communicator.get_entity_info(entities, entity_requests);

    // Vertices index coloring.
    for(auto i : entities) {
      // if it belongs to other colors, its a shared entity
      if(i.shared.size()) {
        aux_coloring.shared.insert(i);
        // Collect all colors with whom we require communication
        // to send shared information.
        aux_coloring_info.dependants =
          set_union(aux_coloring_info.dependants, i.shared);
      }
      // otherwise, its exclusive
      else
        aux_coloring.exclusive.insert(i);
    } // for

    {
      size_t r(0);
      for(auto i : entity_requests) {

        auto offset(entity_offset_info[r].begin());
        for(auto s : i) {
          aux_coloring.ghost.insert(entity_info(s, r, *offset));
          // Collect all colors with whom we require communication
          // to receive ghost information.
          aux_coloring_info.dependencies.insert(r);
          // increment counter
          ++offset;
        } // for

        ++r;
      } // for
    } // scope
  }; // color_entity

  //--------------------------------------------------------------------------//
  // End lambda
  //--------------------------------------------------------------------------//

  tuple_visitor<typename Policy::auxiliary>(color_entity);

  return coloring;
} // closure

} // namespace unstructured_impl
} // namespace topo
} // namespace flecsi
