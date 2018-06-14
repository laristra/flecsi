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
#include <flecsi/execution/execution.h>
#include <flecsi/io/simple_definition.h>
#include <flecsi/coloring/dcrs_utils.h>
#include <flecsi/coloring/parmetis_colorer.h>
#include <flecsi/coloring/mpi_communicator.h>
#include <flecsi/utils/tuple_walker.h>

//#include <flecsi/supplemental/coloring/coloring_functions.h>
//#include <flecsi/supplemental/coloring/tikz.h>

#include <tuple>

namespace flecsi {
namespace execution {

using namespace coloring;

struct coloring_policy_t {

  using primary = primary_independent__<0, 2, 0, 2>;

  using auxiliary = std::tuple<
    auxiliary_independent__<1, 0, 2>,
    auxiliary_independent__<2, 1, 2>
  >;

  static constexpr size_t auxiliary_colorings =
    std::tuple_size<auxiliary>::value;

  using mesh_definition_t = flecsi::topology::mesh_definition__<2>;

}; // coloring_policy_t

template<typename PRIMARY_TYPE>
struct auxiliary_walker__
  : public flecsi::utils::tuple_walker__<auxiliary_walker__<PRIMARY_TYPE>>
{
  template<typename AUXILIARY_TYPE>
  void handle_type() {

    static_assert(AUXILIARY_TYPE::primary_dimension == PRIMARY_TYPE::dimension,
      "dimension mismatch");

    std::cout << "auxiliary dimension: " <<
      AUXILIARY_TYPE::dimension << std::endl;
  } // handle_type

private:

}; // struct auxiliary_walker__

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//

template<typename COLORING_POLICY>
void generic_coloring(
  typename COLORING_POLICY::mesh_definition_t & md,
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
  for(size_t i{0}; i<depth; ++i) {
    
    // Form the closure of the current core
    auto closure = entity_neighbors<dimension, dimension, thru_dimension>(
      md, core);

    // Subtract off just the new nearest neighbors
    auto nearest_neighbors = set_difference(closure, core);

    // Add these to the aggregate
    aggregate_near_neighbors = set_union(aggregate_near_neighbors,
      nearest_neighbors);

    // Update the core set
    core = set_union(core, nearest_neighbors);
  } // for

  // Form the closure of the collected neighbors
  auto near_neighbor_closure =
    entity_neighbors<dimension, dimension, thru_dimension>(
      md, aggregate_near_neighbors);

  // Clean primaries from closure to get all neighbors
  auto aggregate_neighbors =
    set_difference(near_neighbor_closure, primary);

  if(rank == 0) {
    std::cout << "aggregate near neighbors" << std::endl;
    for(auto n: aggregate_near_neighbors) {
      std::cout << n << std::endl;
    } // for

    std::cout << "aggregate neighbors" << std::endl;
    for(auto n: aggregate_neighbors) {
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
  for(auto i: primary) {
    primary_indices_map[offset++] = i;
  } // for
  } // scope
  
  // Create a map version of the remote info for lookups below.
  std::unordered_map<size_t, flecsi::coloring::entity_info_t> remote_info_map;
  for(auto i: std::get<1>(primary_all_info)) {
    remote_info_map[i.id] = i;
  } // for

  // Populate exclusive and shared primary information
  {
  size_t offset{0};

  for(auto i: std::get<0>(primary_nn_info)) {
    if(i.size()) {
      primary_coloring.shared.insert(
        flecsi::coloring::entity_info_t(primary_indices_map[offset],
        rank, offset, i));

      // Collect all colors with whom we require communication
      // to send shared information.
      primary_coloring_info.shared_users = flecsi::utils::set_union(
        primary_coloring_info.shared_users, i);
    }
    else {
      primary_coloring.exclusive.insert(
        flecsi::coloring::entity_info_t(primary_indices_map[offset],
        rank, offset, i));
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
  for(auto i: primary_coloring.shared) {
    shared_primary_map[i.id] = i;
  } // for
  } // scope

  auxiliary_walker__<typename COLORING_POLICY::primary> auxiliary_walker;
  auxiliary_walker.template walk_types<typename COLORING_POLICY::auxiliary>();
} // generic_coloring

inline void concept_coloring() {

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
