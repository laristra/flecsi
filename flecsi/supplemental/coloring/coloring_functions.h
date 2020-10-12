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

#include <cinchlog.h>

#include <flecsi/coloring/colorer.h>
#include <flecsi/coloring/communicator.h>
#include <flecsi/coloring/dcrs_utils.h>
#include <flecsi/execution/execution.h>
#include <flecsi/topology/mesh_definition.h>

clog_register_tag(coloring_functions);

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
//! @tparam
//----------------------------------------------------------------------------//

template<size_t DIMENSION,
  size_t ENTITY_DIM,
  typename CLOSURE_SET,
  typename ENTITY_MAP,
  typename INTERSECTION_MAP,
  typename INDEX_COLOR,
  typename COLOR_INFO>
void
color_entity(topology::mesh_definition_u<DIMENSION> const & md,
  coloring::communicator_t * communicator,
  CLOSURE_SET const & closure,
  ENTITY_MAP const & remote_info_map,
  ENTITY_MAP const & shared_cells_map,
  INTERSECTION_MAP const & closure_intersection_map,
  INDEX_COLOR & entities,
  COLOR_INFO & entity_color_info) {
  // some compile time constants
  constexpr auto cell_dim = DIMENSION;

  // some type aliases
  using entity_info_t = flecsi::coloring::entity_info_t;

  // info about the mpi communicator
  auto comm_size = communicator->size();
  auto rank = communicator->rank();

  // Form the entity closure
  auto entity_closure =
    flecsi::topology::entity_closure<cell_dim, ENTITY_DIM>(md, closure);

  // Assign entity ownership
  std::vector<std::set<size_t>> entity_requests(comm_size);
  std::set<entity_info_t> entity_info;

  {
    size_t offset(0);
    for(auto i : entity_closure) {

      // Get the set of cells that reference this entity.
      auto referencers =
        flecsi::topology::entity_referencers<cell_dim, ENTITY_DIM>(md, i);

#if 0
      {
        clog_tag_guard(coloring_functions);
        clog_container_one(info, i << " referencers", referencers, clog::space);
      } // guard
#endif

      size_t min_rank((std::numeric_limits<size_t>::max)());
      std::set<size_t> shared_entities;

      // Iterate the direct referencers to assign entity ownership.
      for(auto c : referencers) {

        // Check the remote info map to see if this cell is
        // off-color. If it is, compare it's rank for
        // the ownership logic below.
        if(remote_info_map.find(c) != remote_info_map.end()) {
          min_rank = (std::min)(min_rank, remote_info_map.at(c).rank);
          shared_entities.insert(remote_info_map.at(c).rank);
        }
        else {
          // If the referencing cell isn't in the remote info map
          // it is a local cell.

          // Add our rank to compare for ownership.
          min_rank = (std::min)(min_rank, size_t(rank));

          // If the local cell is shared, we need to add all of
          // the ranks that reference it.
          if(shared_cells_map.find(c) != shared_cells_map.end())
            shared_entities.insert(shared_cells_map.at(c).shared.begin(),
              shared_cells_map.at(c).shared.end());
        } // if

        // Iterate through the closure intersection map to see if the
        // indirect reference is part of another rank's closure, i.e.,
        // that it is an indirect dependency.
        for(auto ci : closure_intersection_map)
          if(ci.second.find(c) != ci.second.end())
            shared_entities.insert(ci.first);
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
  } // scope

  auto entity_offset_info =
    communicator->get_entity_info(entity_info, entity_requests);

  // Vertices index coloring.
  for(auto i : entity_info) {
    // if it belongs to other colors, its a shared entity
    if(i.shared.size()) {
      entities.shared.insert(i);
      // Collect all colors with whom we require communication
      // to send shared information.
      entity_color_info.shared_users =
        flecsi::utils::set_union(entity_color_info.shared_users, i.shared);
    }
    // otherwise, its exclusive
    else
      entities.exclusive.insert(i);
  } // for

  {
    size_t r(0);
    for(auto i : entity_requests) {

      auto offset(entity_offset_info[r].begin());
      for(auto s : i) {
        entities.ghost.insert(entity_info_t(s, r, *offset));
        // Collect all colors with whom we require communication
        // to receive ghost information.
        entity_color_info.ghost_owners.insert(r);
        // increment counter
        ++offset;
      } // for

      ++r;
    } // for
  } // scope

#if 0
  {
    clog_tag_guard(coloring_functions);
    clog_container_one(
      info,
      "exclusive entities("<<ENTITY_DIM<<")",
      entities.exclusive,
      clog::newline
    );
    clog_container_one(
      info, "shared entities("<<ENTITY_DIM<<")", entities.shared, clog::newline
    );
    clog_container_one(
      info, "ghost entities("<<ENTITY_DIM<<")" , entities.ghost, clog::newline
    );
  } // guard
#endif

  entity_color_info.exclusive = entities.exclusive.size();
  entity_color_info.shared = entities.shared.size();
  entity_color_info.ghost = entities.ghost.size();
} // color_entity

} // namespace execution
} // namespace flecsi
