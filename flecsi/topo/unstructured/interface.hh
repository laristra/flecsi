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

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly!
#endif

#include "flecsi/data/accessor.hh"
#include "flecsi/data/topology.hh"
#include "flecsi/flog.hh"
#include "flecsi/topo/core.hh"
#include "flecsi/topo/unstructured/closure_utils.hh"
#include "flecsi/topo/unstructured/coloring_utils.hh"
#include "flecsi/topo/unstructured/types.hh"
#include "flecsi/topo/utility_types.hh"
#include "flecsi/util/color_map.hh"
#include "flecsi/util/dcrs.hh"
#include "flecsi/util/set_utils.hh"
#include "flecsi/util/tuple_visitor.hh"

#include <map>
#include <utility>

namespace flecsi {
namespace topo {

namespace unstructured_impl {

/*----------------------------------------------------------------------------*
  Method Implementations.
 *----------------------------------------------------------------------------*/

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
  coloring.index_colorings.resize(1 + Policy::auxiliary_colorings);
  coloring.distribution.resize(communicator.size());
  coloring.distribution[rank].resize(1 + Policy::auxiliary_colorings);

  auto & primary_coloring = coloring.index_colorings[0];
  auto & primary_coloring_info = coloring.distribution[rank][0];

  auto aux_coloring = coloring.index_colorings.begin();
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

    auto & aux_coloring = coloring.index_colorings[idx + 1];
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

/*----------------------------------------------------------------------------*
  Unstructured Topology.
 *----------------------------------------------------------------------------*/

template<typename Policy>
struct unstructured : unstructured_base,
                      with_ragged<Policy>,
                      with_meta<Policy> {
  using index_space = typename Policy::index_space;
  using index_spaces = typename Policy::index_spaces;

  template<std::size_t>
  struct access;

  unstructured(coloring const & c)
    : with_ragged<Policy>(c.colors), with_meta<Policy>(c.colors),
      part_(make_partitions(c,
        index_spaces(),
        std::make_index_sequence<index_spaces::size>())) {
    init_ragged(index_spaces());
    allocate_connectivities(c, connect_);
  }

  static inline const connect_t<Policy> connect_;
  static inline const lists_t<Policy> special_;
  util::key_array<repartitioned, index_spaces> part_;

  std::size_t colors() const {
    return part_.front().colors();
  }

  template<index_space S>
  data::region & get_region() {
    return part_.template get<S>();
  }
  template<index_space S>
  const data::partition & get_partition(field_id_t) const {
    return part_.template get<S>();
  }

  template<class F>
  void fields(F fn) {
    for(auto & r : part_)
      fn(resize::field, r.sz);
    connect_visit([&](const auto & f) { fn(f, *this); }, connect_);
    connect_visit([&](const auto & f) { fn(f, this->meta); }, special_);
  }

private:
  template<auto... Value, std::size_t... Index>
  util::key_array<repartitioned, util::constants<Value...>> make_partitions(
    unstructured_base::coloring const & c,
    util::constants<Value...> /* index spaces to deduce pack */,
    std::index_sequence<Index...>) {
    flog_assert(c.index_colorings.size() == sizeof...(Value),
      c.index_colorings.size()
        << " sizes for " << sizeof...(Value) << " index spaces");
    return {{make_repartitioned<Policy, Value>(
      c.colors, make_partial<is_size>(c.index_colorings[Index], c.colors))...}};
  }

  template<auto... VV, typename... TT>
  void allocate_connectivities(const unstructured_base::coloring & c,
    util::key_tuple<util::key_type<VV, TT>...> const & /* deduce pack */) {
    std::size_t entity = 0;
    (
      [&](TT const & row) {
        auto & cc = c.connectivity_sizes[entity++];
        std::size_t is{0};
        for(auto & fd : row) {
          auto & p = this->ragged.template get_partition<VV>(fd.fid);
          execute<cn_size>(cc[is++], p.sizes());
          p.resize();
        }
      }(connect_.template get<VV>()),
      ...);
  }

  template<index_space... SS>
  void init_ragged(util::constants<SS...>) {
    (this->template extend_offsets<SS>(), ...);
  }
}; // struct unstructured

/*----------------------------------------------------------------------------*
  Unstructured Access.
 *----------------------------------------------------------------------------*/

template<typename Policy>
template<std::size_t Privileges>
struct unstructured<Policy>::access {
private:
  using entity_list = typename Policy::entity_list;
  template<const auto & Field>
  using accessor = data::accessor_member<Field, Privileges>;
  util::key_array<resize::accessor<ro>, index_spaces> size_;
  connect_access<Policy, Privileges> connect_;
  list_access<Policy, Privileges> special_{unstructured::special_};

  template<index_space From, index_space To>
  auto & connectivity() {
    return connect_.template get<From>().template get<To>();
  }

  template<index_space From, index_space To>
  auto const & connectivity() const {
    return connect_.template get<From>().template get<To>();
  }

public:
  access() : connect_(unstructured::connect_) {}

  /*!
    Return an iterator to the parameterized index space.

    @tparam IndexSpace The index space identifier.
   */

  template<index_space IndexSpace>
  auto entities() {
    return make_ids<IndexSpace>(util::iota_view<util::id>(
      0, data::partition::row_size(size_.template get<IndexSpace>())));
  }

  /*!
    Return an iterator to the connectivity information for the parameterized
    index spaces.

    @tparam To   The connected index space.
    @tparam From The index space with connections.
   */

  template<index_space To, index_space From>
  auto entities(id<From> from) const {
    return make_ids<To>(connectivity<From, To>()[from]);
  }

  template<index_space I, entity_list L>
  auto special_entities() const {
    return make_ids<I>(special_.template get<I>().template get<L>()[0]);
  }

  template<class F>
  void bind(F f) {
    for(auto & a : size_)
      f(a);
    connect_visit(f, connect_);
    connect_visit(f, special_);
  }
}; // struct unstructured<Policy>::access

template<>
struct detail::base<unstructured> {
  using type = unstructured_base;
}; // struct detail::base<unstructured>

} // namespace topo
} // namespace flecsi
