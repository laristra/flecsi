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

/*----------------------------------------------------------------------------*
  Functors
 *----------------------------------------------------------------------------*/

template<typename Definition>
struct pack_cells {
  using return_type = std::vector<std::vector<std::size_t>>;

  pack_cells(Definition const & md, std::vector<std::size_t> const & dist)
    : md_(md), dist_(dist) {}

  return_type operator()(int rank, int) const {
    return_type cells;
    cells.reserve(dist_[rank + 1] - dist_[rank]);

    for(size_t i{dist_[rank]}; i < dist_[rank + 1]; ++i) {
      cells.emplace_back(md_.entities(Definition::dimension(), 0, i));
    } // for

    return cells;
  } // operator(int, int)

private:
  Definition const & md_;
  std::vector<std::size_t> const & dist_;
}; // struct pack_cells

template<typename Definition>
struct pack_vertices {
  using return_type = std::vector<typename Definition::point>;

  pack_vertices(Definition const & md, std::vector<std::size_t> const & dist)
    : md_(md), dist_(dist) {}

  return_type operator()(int rank, int) const {
    return_type vertices;
    vertices.reserve(dist_[rank + 1] - dist_[rank]);

    for(size_t i{dist_[rank]}; i < dist_[rank + 1]; ++i) {
      vertices.emplace_back(md_.vertex(i));
    } // for

    return vertices;
  } // operator(int, int)

private:
  Definition const & md_;
  std::vector<std::size_t> const & dist_;
}; // struct pack_vertices

/*
  Send partial vertex-to-cell connectivity information to the rank that owns
  the vertex in the naive vertex distribution.
 */

struct vertex_referencers {
  using return_type = std::map<std::size_t, std::vector<std::size_t>>;

  vertex_referencers(
    std::map<std::size_t, std::vector<std::size_t>> const & vertex2cell,
    std::vector<std::size_t> const & dist,
    int rank)
    : size_(dist.size() - 1), rank_(rank) {
    references_.resize(size_);
    for(auto v : vertex2cell) {
      auto r = util::distribution_offset(dist, v.first);
      if(r != rank_) {
        for(auto c : v.second) {
          references_[r][v.first].emplace_back(c);
        } // for
      } // if
    } // for
  } // vertex_refernces

  std::size_t count(int rank) const {
    flog_assert(rank < size_, "invalid rank");
    return rank == rank_ ? 0
                         : util::serial_size<return_type>(references_[rank]);
  }

  return_type operator()(int rank, int) const {
    flog_assert(rank < size_, "invalid rank");
    return references_[rank];
  } // operator(int, int)

private:
  const int size_;
  const int rank_;
  std::vector<std::map<std::size_t, std::vector<std::size_t>>> references_;
}; // struct vertex_referencers

/*
  Send full vertex-to-cell connectivity information to all ranks that reference
  one of our vertices.
 */

struct cell_connectivity {

  using return_type = std::map<std::size_t, std::vector<std::size_t>>;

  cell_connectivity(std::vector<std::vector<std::size_t>> const & vertices,
    std::map<std::size_t, std::vector<std::size_t>> const & connectivity,
    std::vector<std::size_t> const & dist,
    int rank)
    : size_(dist.size() - 1), rank_(rank), connectivity_(size_) {

    int offset{0};
    for(auto r : vertices) {
      if(offset != rank_) {
        for(auto v : r) {
          connectivity_[offset][v] = connectivity.at(v);
        } // for
      } // for
      ++offset;
    } // for
  } // cell_connectivity

  std::size_t count(int rank) const {
    flog_assert(rank < size_, "invalid rank");
    return rank == rank_ ? 0
                         : util::serial_size<return_type>(connectivity_[rank]);
  }

  return_type operator()(int rank, int) const {
    flog_assert(rank < size_, "invalid rank");
    return connectivity_[rank];
  } // operator(int, int)

private:
  const int size_;
  const int rank_;
  std::vector<std::map<std::size_t, std::vector<std::size_t>>> connectivity_;
}; // struct cell_connectivity

/*
  Send cells to colors that own them.
 */

struct distribute_cells {

  using return_type = std::vector<std::array<std::size_t, 2>>;

  distribute_cells(util::dcrs const & naive,
    std::size_t colors,
    std::vector<std::size_t> const & index_colors,
    int rank)
    : size_(naive.distribution.size() - 1) {
    util::color_map cm(size_, colors, naive.distribution.back());

    for(std::size_t r{0}; r < std::size_t(size_); ++r) {
      std::vector<std::array<std::size_t, 2>> indices;

      for(std::size_t i{0}; i < naive.entries(); ++i) {
        if(cm.process(index_colors[i]) == r) {
          indices.push_back({index_colors[i], naive.distribution[rank] + i});
        } // if
      } // for

      cells_.emplace_back(indices);
    } // for
  } // distribute_cells

  std::size_t count(int rank) const {
    flog_assert(rank < size_, "invalid rank");
    return util::serial_size<return_type>(cells_[rank]);
  }

  return_type operator()(int rank, int) const {
    flog_assert(rank < size_, "invalid rank");
    return cells_[rank];
  }

private:
  const int size_;
  std::vector<std::vector<std::array<std::size_t, 2>>> cells_;
}; // struct distribute_cells

/*----------------------------------------------------------------------------*
  Interface
 *----------------------------------------------------------------------------*/

template<typename Definition>
util::dcrs
make_dcrs(Definition const & md,
  std::size_t through_dimension,
  MPI_Comm comm = MPI_COMM_WORLD) {
  auto [rank, size, group] = util::mpi::info(comm);

  // Get the cells for this rank
  util::color_map cm(size, size, md.num_entities(Definition::dimension()));
  auto cells = util::mpi::one_to_allv<pack_cells<Definition>>(
    {md, cm.distribution()}, comm);

  // Populate local vertex connectivity information
  std::size_t offset{cm.distribution()[rank]};
  std::size_t indices{cm.indices(rank, 0)};
  std::map<std::size_t, std::vector<std::size_t>> vertex2cell;

  std::size_t i{0};
  for(auto c : cells) {
    for(auto v : c) {
      vertex2cell[v].emplace_back(offset + i);
    } // for
    ++i;
  } // for

  // Request all referencers of our local vertices
  util::color_map vm(size, size, md.num_entities(0));
  auto referencers = util::mpi::all_to_allv<vertex_referencers>(
    {vertex2cell, vm.distribution(), rank}, comm);

  // Update our local connectivity information (We now have all vertex-to-cell
  // connectivity informaiton for the naive distribution of vertices that we
  // own.)
  i = 0;
  for(auto & r : referencers) {
    for(auto v : r) {
      // referencer_inverse[i].emplace_back(v.first);
      for(auto c : v.second) {
        vertex2cell[v.first].emplace_back(c);
      } // for
    } // for
    ++i;
  } // for

  // Remove duplicate referencers
  for(auto & v : vertex2cell) {
    auto & cs = v.second;
    std::sort(cs.begin(), cs.end());
    auto first = cs.begin();
    auto last = std::unique(first, cs.end());
    cs.erase(last, cs.end());
  } // for

  std::vector<std::vector<std::size_t>> referencer_inverse(size);

  for(auto const & v : vertex2cell) {
    for(auto c : v.second) {
      auto r = util::distribution_offset(cm.distribution(), c);
      if(r != rank) {
        referencer_inverse[r].emplace_back(v.first);
      } // if
    } // for
  } // for

  // Remove duplicate inverses
  for(auto & r : referencer_inverse) {
    std::sort(r.begin(), r.end());
    auto first = r.begin();
    auto last = std::unique(first, r.end());
    r.erase(last, r.end());
  } // for

  // Request vertex-to-cell connectivity for the cells that we own in the naive
  // cell distribution.
  auto connectivity = util::mpi::all_to_allv<cell_connectivity>(
    {referencer_inverse, vertex2cell, vm.distribution(), rank}, comm);

  for(auto & r : connectivity) {
    for(auto & v : r) {
      for(auto c : v.second) {
        vertex2cell[v.first].emplace_back(c);
      } // for
    } // for
  } // for

  // Remove duplicate referencers
  for(auto & v : vertex2cell) {
    auto & cs = v.second;
    std::sort(cs.begin(), cs.end());
    auto first = cs.begin();
    auto last = std::unique(first, cs.end());
    cs.erase(last, cs.end());
  } // for

  std::map<std::size_t, std::vector<std::size_t>> cell2cells;
  std::size_t c{offset};
  for(auto & cd /* cell definition */ : cells) {
    std::map<std::size_t, std::size_t> thru_counts;

    for(auto v : cd) {
      auto it = vertex2cell.find(v);
      if(it != vertex2cell.end()) {
        for(auto rc : vertex2cell[v]) {
          if(rc != c) {
            thru_counts[rc] += 1;
          } // if
        } // for
      } // if
    } // for

    for(auto tc : thru_counts) {
      if(tc.second > through_dimension) {
        cell2cells[c].emplace_back(tc.first);
        cell2cells[tc.first].emplace_back(c);
      } // if
    } // for

    ++c;
  } // for

  // Remove duplicate connections
  for(auto & c : cell2cells) {
    auto & cs = c.second;
    std::sort(cs.begin(), cs.end());
    auto first = cs.begin();
    auto last = std::unique(first, cs.end());
    cs.erase(last, cs.end());
  } // for

  util::dcrs dcrs;
  dcrs.distribution = cm.distribution();

  dcrs.offsets.emplace_back(0);
  for(std::size_t c{0}; c < indices; ++c) {
    for(auto cr : cell2cells[offset + c]) {
      dcrs.indices.emplace_back(cr);
    } // for

    dcrs.offsets.emplace_back(dcrs.offsets[c] + cell2cells[offset + c].size());
  } // for

  flog(info) << dcrs << std::endl;

  return dcrs;
} // make_dcrs

std::vector<std::vector<std::size_t>>
distribute(util::dcrs const & naive,
  size_t colors,
  std::vector<std::size_t> const & index_colors,
  MPI_Comm comm = MPI_COMM_WORLD) {
  auto [rank, size, group] = util::mpi::info(comm);

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

} // namespace unstructured_impl
} // namespace topo
} // namespace flecsi
