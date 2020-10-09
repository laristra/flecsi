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

#include <flecsi/coloring/concept.h>
#include <flecsi/coloring/dcrs_utils.h>
#include <flecsi/coloring/mpi_communicator.h>
#include <flecsi/coloring/parmetis_colorer.h>
#include <flecsi/execution/execution.h>
#include <flecsi/io/simple_definition.h>

#include <flecsi/utils/tuple_visit.h>

//#include <flecsi/supplemental/coloring/coloring_functions.h>
//#include <flecsi/supplemental/coloring/tikz.h>

#include <tuple>

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
// This function takes an independent coloring of the primary entity type and
// generates a dependency closure for the primary, and independent colorings
// and dependency closures for the auxilliaries.
//----------------------------------------------------------------------------//

template<typename COLORING_POLICY>
void
generic_coloring(typename COLORING_POLICY::mesh_definition_t & md,
  std::set<size_t> const & primary) {

  using namespace flecsi::topology;
  using namespace flecsi::utils;

  int rank, size;
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  flecsi::coloring::index_coloring_t primary_coloring;
  flecsi::coloring::coloring_info_t primary_coloring_info;

  constexpr size_t auxiliary_colorings = COLORING_POLICY::auxiliary_colorings;

  flecsi::coloring::index_coloring_t aux_coloring[auxiliary_colorings];
  flecsi::coloring::coloring_info_t aux_coloring_info[auxiliary_colorings];

  constexpr size_t depth = COLORING_POLICY::primary::depth;
  constexpr size_t dimension = COLORING_POLICY::primary::dimension;
  constexpr size_t thru_dimension = COLORING_POLICY::primary::thru_dimension;

  // Create a communicator instance to get neighbor information.
  auto communicator = std::make_shared<flecsi::coloring::mpi_communicator_t>();

  std::set<size_t> aggregate_near_neighbors;
  auto core = primary;

  // Collect neighbors up to depth deep
  for(size_t i{0}; i < depth; ++i) {

    // Form the closure of the current core
    auto closure =
      entity_neighbors<dimension, dimension, thru_dimension>(md, core);

    // Subtract off just the new nearest neighbors
    auto nearest_neighbors = set_difference(closure, core);

    // Add these to the aggregate
    aggregate_near_neighbors =
      set_union(aggregate_near_neighbors, nearest_neighbors);

    // Update the core set
    core = set_union(core, nearest_neighbors);
  } // for

  // Form the closure of the collected neighbors
  auto near_neighbor_closure =
    entity_neighbors<dimension, dimension, thru_dimension>(
      md, aggregate_near_neighbors);

  // Clean primaries from closure to get all neighbors
  auto aggregate_neighbors = set_difference(near_neighbor_closure, primary);

  if(rank == 0) {
    std::cout << "aggregate near neighbors" << std::endl;
    for(auto n : aggregate_near_neighbors) {
      std::cout << n << std::endl;
    } // for

    std::cout << "aggregate neighbors" << std::endl;
    for(auto n : aggregate_neighbors) {
      std::cout << n << std::endl;
    } // for

  } // rank

  /*
    Get the intersection of our near neighbors with the near neighbors
    of other ranks. This map of sets will only be populated with
    intersections that are non-empty
   */
  auto closure_intersection_map =
    communicator->get_intersection_info(aggregate_near_neighbors);

  /*
    Get the rank and offset information for our near neighbor
    dependencies. This also gives information about the ranks
    that access our shared entities.
   */
  auto primary_nn_info =
    communicator->get_primary_info(primary, aggregate_near_neighbors);

  /*
    Get the rank and offset information for all relevant neighbor
    dependencies. This information will be necessary for determining
    shared vertices.
   */
  auto primary_all_info =
    communicator->get_primary_info(primary, aggregate_neighbors);

  // Create a map version of the local info for lookups below.
  std::unordered_map<size_t, size_t> primary_indices_map;
  {
    size_t offset{0};
    for(auto i : primary) {
      primary_indices_map[offset++] = i;
    } // for
  } // scope

  // Create a map version of the remote info for lookups below.
  std::unordered_map<size_t, flecsi::coloring::entity_info_t> remote_info_map;
  for(auto i : std::get<1>(primary_all_info)) {
    remote_info_map[i.id] = i;
  } // for

  // Populate exclusive and shared primary information
  {
    size_t offset{0};

    for(auto i : std::get<0>(primary_nn_info)) {
      if(i.size()) {
        primary_coloring.shared.insert(flecsi::coloring::entity_info_t(
          primary_indices_map[offset], rank, offset, i));

        // Collect all colors with whom we require communication
        // to send shared information.
        primary_coloring_info.shared_users =
          flecsi::utils::set_union(primary_coloring_info.shared_users, i);
      }
      else {
        primary_coloring.exclusive.insert(flecsi::coloring::entity_info_t(
          primary_indices_map[offset], rank, offset, i));
      } // if

      ++offset;
    } // for
  } // scope

  primary_coloring_info.exclusive = primary_coloring.exclusive.size();
  primary_coloring_info.shared = primary_coloring.shared.size();
  primary_coloring_info.ghost = primary_coloring.ghost.size();

  std::unordered_map<size_t, flecsi::coloring::entity_info_t>
    shared_primary_map;
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

    constexpr size_t index_space = auxiliary_type::index_space;
    constexpr size_t dimension = auxiliary_type::dimension;
    constexpr size_t primary_dimension = auxiliary_type::primary_dimension;

    std::cout << "idx: " << idx << std::endl;
    std::cout << "index_space: " << index_space << std::endl;
    std::cout << "dimension: " << dimension << std::endl;
    std::cout << "primary_dimension: " << primary_dimension << std::endl;

    // Form the closure of this entity from the primary
    auto auxiliary_closure =
      entity_closure<primary_dimension, dimension>(md, near_neighbor_closure);

    using entity_info_t = flecsi::coloring::entity_info_t;

    // Assign entity ownership
    std::vector<std::set<size_t>> entity_requests(size);
    std::set<entity_info_t> entity_info;

    {
      size_t offset{0};
      for(auto i : auxiliary_closure) {
        auto referencers =
          entity_referencers<primary_dimension, dimension>(md, i);

        size_t min_rank((std::numeric_limits<size_t>::max)());
        std::set<size_t> shared_entities;

        // Iterate the direct referencers to assign entity ownership
        for(auto c : referencers) {

          // Check the remote info map to see if this primary is
          // off-color. If it is, compare it's rank for
          // the ownership logic below.
          if(remote_info_map.find(c) != remote_info_map.end()) {
            min_rank = (std::min)(min_rank, remote_info_map.at(c).rank);
            shared_entities.insert(remote_info_map.at(c).rank);
          }
          else {
            // If the referencing primary isn't in the remote info map
            // it is a local primary.

            // Add our rank to compare for ownership.
            min_rank = (std::min)(min_rank, size_t(rank));

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

          if(min_rank == rank) {
            // This is a entity that belongs to our rank.
            auto entry = entity_info_t(i, rank, offset++, shared_entities);
            entity_info.insert(entry);
          }
          else {
            // Add remote entity to the request for offset information.
            entity_requests[min_rank].insert(i);
          } // if
        } // for
      } // for
    } // scope

    auto entity_offset_info =
      communicator->get_entity_info(entity_info, entity_requests);

    // Vertices index coloring.
    for(auto i : entity_info) {
      // if it belongs to other colors, its a shared entity
      if(i.shared.size()) {
        aux_coloring[idx].shared.insert(i);
        // Collect all colors with whom we require communication
        // to send shared information.
        aux_coloring_info[idx].shared_users = flecsi::utils::set_union(
          aux_coloring_info[idx].shared_users, i.shared);
      }
      // otherwise, its exclusive
      else
        aux_coloring[idx].exclusive.insert(i);
    } // for

    {
      size_t r(0);
      for(auto i : entity_requests) {

        auto offset(entity_offset_info[r].begin());
        for(auto s : i) {
          aux_coloring[idx].ghost.insert(entity_info_t(s, r, *offset));
          // Collect all colors with whom we require communication
          // to receive ghost information.
          aux_coloring_info[idx].ghost_owners.insert(r);
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

  flecsi::utils::tuple_visit<typename COLORING_POLICY::auxiliary>(color_entity);

} // generic_coloring

using namespace coloring;

struct coloring_policy_t {

  // Note: in this model, the primary determines the support/range of
  // dependence, e.g., vertex dependencies cannot extend beyond a cell
  // dependency range. (I think that this is true.)

  // FIXME: change names to be better, i.e., through dimension and
  // depth are dependent information.
  using primary = primary_independent_u<0, 2, 0, 2>;

  using auxiliary = std::tuple<auxiliary_independent_u<1, 0, 2>,
    auxiliary_independent_u<2, 1, 2>>;

  static constexpr size_t auxiliary_colorings =
    std::tuple_size<auxiliary>::value;

  using mesh_definition_t = flecsi::topology::mesh_definition_u<2>;

}; // coloring_policy_t

inline void
concept_coloring() {

  // Read the mesh definition from file.
#ifdef FLECSI_8_8_MESH
  const size_t M(8), N(8);
  flecsi::io::simple_definition_t sd("simple2d-8x8.msh");
#else
  const size_t M(16), N(16);
  flecsi::io::simple_definition_t sd("simple2d-16x16.msh");
#endif

  // Create the dCRS representation for the distributed colorer.
  auto dcrs = flecsi::coloring::make_dcrs(sd);

  // Create a colorer instance to generate the primary coloring.
  auto colorer = std::make_shared<flecsi::coloring::parmetis_colorer_t>();

  // Generate the primary independent coloring
  auto primary = colorer->color(dcrs);

  // Generate the rest of the colorings
  generic_coloring<coloring_policy_t>(sd, primary);

} // concept_coloring

} // namespace execution
} // namespace flecsi
