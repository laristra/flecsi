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

#include "flecsi/flog.hh"
#include "flecsi/topo/unstructured/coloring_functors.hh"
#include "flecsi/topo/unstructured/types.hh"
#include "flecsi/util/color_map.hh"
#include "flecsi/util/dcrs.hh"
#include "flecsi/util/mpi.hh"
#include "flecsi/util/serialize.hh"

#include <iterator>
#include <map>
#include <utility>
#include <vector>

namespace flecsi {
namespace topo {
namespace unstructured_impl {

template<typename T>
void
force_unique(std::vector<T> & v) {
  std::sort(v.begin(), v.end());
  auto first = v.begin();
  auto last = std::unique(first, v.end());
  v.erase(last, v.end());
}

template<typename K, typename T>
void
force_unique(std::map<K, std::vector<T>> & m) {
  for(auto & v : m)
    force_unique(v.second);
}

template<typename T>
void
force_unique(std::vector<std::vector<T>> & vv) {
  for(auto & v : vv)
    force_unique(v);
}

/*!
 */

template<typename Definition>
auto
make_dcrs(Definition const & md,
  std::size_t through_dimension,
  MPI_Comm comm = MPI_COMM_WORLD) {
  auto [rank, size] = util::mpi::info(comm);

  util::color_map cm(size, size, md.num_entities(Definition::dimension()));

  /*
    Get the initial cells for this rank. The cells will be read by
    the root process and sent to each initially "owning" rank using
    a naive distribution.
   */

  auto c2v = util::mpi::one_to_allv<pack_cells<Definition>>(
    {md, cm.distribution()}, comm);

  /*
    Create a map of vertex-to-cell connectivity information from
    the initial cell distribution.
   */

  // Populate local vertex connectivity information
  std::size_t offset{cm.distribution()[rank]};
  std::size_t indices{cm.indices(rank, 0)};
  std::map<std::size_t, std::vector<std::size_t>> v2c;

  std::size_t i{0};
  for(auto c : c2v) {
    for(auto v : c) {
      v2c[v].emplace_back(offset + i);
    } // for
    ++i;
  } // for

  // Request all referencers of our connected vertices
  util::color_map vm(size, size, md.num_entities(0));
  auto referencers = util::mpi::all_to_allv<vertex_referencers>(
    {v2c, vm.distribution(), rank}, comm);

  /*
    Update our local connectivity information. We now have all
    vertex-to-cell connectivity informaiton for the naive distribution
    of cells that we own.
   */

  i = 0;
  for(auto & r : referencers) {
    for(auto v : r) {
      for(auto c : v.second) {
        v2c[v.first].emplace_back(c);
      } // for
    } // for
    ++i;
  } // for

  // Remove duplicate referencers
  force_unique(v2c);

  std::vector<std::vector<std::size_t>> referencer_inverse(size);

  for(auto const & v : v2c) {
    for(auto c : v.second) {
      auto r = util::distribution_offset(cm.distribution(), c);
      if(r != rank) {
        referencer_inverse[r].emplace_back(v.first);
      } // if
    } // for
  } // for

  // Remove duplicate inverses
  force_unique(referencer_inverse);

  // Request vertex-to-cell connectivity for the cells that are
  // on other ranks in the naive cell distribution.
  auto connectivity = util::mpi::all_to_allv<cell_connectivity>(
    {referencer_inverse, v2c, vm.distribution(), rank}, comm);

  for(auto & r : connectivity) {
    for(auto & v : r) {
      for(auto c : v.second) {
        v2c[v.first].emplace_back(c);
      } // for
    } // for
  } // for

  // Remove duplicate referencers
  force_unique(v2c);

  std::map<std::size_t, std::vector<std::size_t>> c2c;
  std::size_t c{offset};
  for(auto & cd /* cell definition */ : c2v) {
    std::map<std::size_t, std::size_t> thru_counts;

    for(auto v : cd) {
      auto it = v2c.find(v);
      if(it != v2c.end()) {
        for(auto rc : v2c[v]) {
          if(rc != c) {
            thru_counts[rc] += 1;
          } // if
        } // for
      } // if
    } // for

    for(auto tc : thru_counts) {
      if(tc.second > through_dimension) {
        c2c[c].emplace_back(tc.first);
        c2c[tc.first].emplace_back(c);
      } // if
    } // for

    ++c;
  } // for

  // Remove duplicate connections
  force_unique(c2c);

  util::dcrs dcrs;
  dcrs.distribution = cm.distribution();

  dcrs.offsets.emplace_back(0);
  for(std::size_t c{0}; c < indices; ++c) {
    for(auto cr : c2c[offset + c]) {
      dcrs.indices.emplace_back(cr);
    } // for

    dcrs.offsets.emplace_back(dcrs.offsets[c] + c2c[offset + c].size());
  } // for

  return std::make_tuple(dcrs, c2v, v2c, c2c);
} // make_dcrs

std::vector<std::vector<std::size_t>>
distribute(util::dcrs const & naive,
  size_t colors,
  std::vector<std::size_t> const & index_colors,
  MPI_Comm comm = MPI_COMM_WORLD) {
  auto [rank, size] = util::mpi::info(comm);

  auto color_primaries = util::mpi::all_to_allv<distribute_cells>(
    {naive, colors, index_colors, rank}, comm);

  util::color_map cm(size, colors, naive.distribution.back());
  const std::size_t offset = cm.color_offset(rank);
  std::vector<std::vector<std::size_t>> primaries(cm.colors(rank));

  for(auto cp : color_primaries) {
    for(auto c : cp) {
      primaries[std::get<0>(c) - offset].emplace_back(std::get<1>(c));
    } // for
  } // for

  return primaries;
} // distribute

auto
migrate(util::dcrs const & naive,
  size_t colors,
  std::vector<std::size_t> const & index_colors,
  std::vector<std::vector<std::size_t>> & c2v,
  std::map<std::size_t, std::vector<std::size_t>> & v2c,
  std::map<std::size_t, std::vector<std::size_t>> & c2c,
  MPI_Comm comm = MPI_COMM_WORLD) {
  auto [rank, size] = util::mpi::info(comm);

  auto migrated = util::mpi::all_to_allv<migrate_cells>(
    {naive, colors, index_colors, c2v, v2c, c2c, rank}, comm);

  std::map<std::size_t, std::vector<std::size_t>> primaries;
  std::vector<std::size_t> l2m;

  for(auto const & r : migrated) {
    auto const & cell_pack = std::get<0>(r);
    for(auto const & c : cell_pack) {
      auto const & info = std::get<0>(c);
      c2v.emplace_back(std::get<1>(c));
      l2m.emplace_back(std::get<1>(info));
      primaries[std::get<0>(info)].emplace_back(std::get<1>(info));
    } // for

    auto v2c_pack = std::get<1>(r);
    for(auto const & v : v2c_pack) {
      v2c.try_emplace(v.first, v.second);
    } // for

    auto c2c_pack = std::get<2>(r);
    for(auto const & c : c2c_pack) {
      c2c.try_emplace(c.first, c.second);
    } // for
  } // for

  return std::make_pair(primaries, l2m);
} // migrate

template<typename Policy>
auto
closure(std::vector<std::size_t> const & primary,
  MPI_Comm comm = MPI_COMM_WORLD) {
  auto [rank, size] = util::mpi::info(comm);

  unstructured_base::coloring coloring;
  coloring.colors = size;
  coloring.idx_allocs.resize(1 + Policy::auxiliary_colorings);
  coloring.idx_colorings.resize(1 + Policy::auxiliary_colorings);

  (void)rank;
  (void)primary;

  constexpr std::size_t depth = Policy::primary::depth;

  for(std::size_t d{0}; d < depth; ++d) {
    // get entity neighbors

  } // for

  return coloring;
} // closure

} // namespace unstructured_impl
} // namespace topo
} // namespace flecsi
