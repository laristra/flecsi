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

#include <algorithm>
#include <map>
#include <set>
#include <tuple>
#include <vector>

namespace flecsi {
namespace topo {
namespace unstructured_impl {

template<typename Definition>
struct pack_cells {
  static constexpr bool one_to_allv_callable = true;

  using return_type = std::vector<std::vector<std::size_t>>;

  pack_cells(Definition const & md, std::vector<std::size_t> const & dist)
    : md_(md), dist_(dist) {}

  return_type operator()(int rank, int) const {
    return_type c2v;
    c2v.reserve(dist_[rank + 1] - dist_[rank]);

    for(size_t i{dist_[rank]}; i < dist_[rank + 1]; ++i) {
      c2v.emplace_back(md_.entities(Definition::dimension(), 0, i));
    } // for

    return c2v;
  } // operator(int, int)

private:
  Definition const & md_;
  std::vector<std::size_t> const & dist_;
}; // struct pack_cells

template<typename Definition>
struct pack_vertices {
  static constexpr bool one_to_allv_callable = true;

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
  static constexpr bool all_to_allv_callable = true;

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
  static constexpr bool all_to_allv_callable = true;

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
  static constexpr bool all_to_allv_callable = true;

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
          indices.push_back({index_colors[i] /* color of this index */,
            naive.distribution[rank] + i /* index global id */});
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

/*
 */

struct migrate_cells {
  static constexpr bool all_to_allv_callable = true;

  using return_type =
    std::tuple<std::vector</* over cells */
                 std::tuple<std::array<std::size_t, 2> /* color, mid> */,
                   std::vector<std::size_t> /* cell definition (vertex mids) */
                   >>,
      std::map</* over cell mids */
        std::size_t,
        std::vector<std::size_t> /* cell-to-cell connectivity */
        >,
      std::map</* over vertex mids */
        std::size_t,
        std::vector<std::size_t> /* vertex-to-cell connectivity */
        >>;

  migrate_cells(util::dcrs const & naive,
    std::size_t colors,
    std::vector<std::size_t> const & index_colors,
    std::vector<std::vector<std::size_t>> & c2v,
    std::map<std::size_t, std::vector<std::size_t>> & v2c,
    std::map<std::size_t, std::vector<std::size_t>> & c2c,
    int rank)
    : size_(naive.distribution.size() - 1) {
    util::color_map cm(size_, colors, naive.distribution.back());

    for(std::size_t r{0}; r < std::size_t(size_); ++r) {
      std::vector<
        std::tuple<std::array<std::size_t, 2>, std::vector<std::size_t>>>
        cell_pack;
      std::map<std::size_t, std::vector<std::size_t>> v2c_pack;
      std::map<std::size_t, std::vector<std::size_t>> c2c_pack;

      for(std::size_t i{0}; i < naive.entries(); ++i) {
        if(cm.process(index_colors[i]) == r) {
          const std::array<std::size_t, 2> info{
            index_colors[i], naive.distribution[rank] + i};
          cell_pack.push_back(std::make_tuple(info, c2v[i]));

          /*
            If we have full connectivity information, we pack it up
            and send it. We do not send information that is potentially,
            or actually incomplete because additional communication
            will be required to resolve it regardless.
           */

          for(auto const & v : c2v[i]) {
            v2c_pack[v] = v2c[v];
          } // for

          c2c_pack[naive.distribution[rank] + i] =
            c2c[naive.distribution[rank] + i];

          /*
            Remove information that we are migrating. We can't remove
            vertex-to-cell information until the loop over ranks is done.
           */

          c2v[i].clear();
          c2c.erase(i);
        } // if
      } // for

      packs_.emplace_back(std::make_tuple(cell_pack, v2c_pack, c2c_pack));
    } // for

    c2v.clear();
    c2c.clear();
    v2c.clear();
  } // migrate_cells

  std::size_t count(int rank) const {
    flog_assert(rank < size_, "invalid rank");
    return util::serial_size<return_type>(packs_[rank]);
  }

  return_type operator()(int rank, int) const {
    flog_assert(rank < size_, "invalid rank");
    return packs_[rank];
  }

private:
  const int size_;
  std::vector<return_type> packs_;
}; // struct migrate_cells

} // namespace unstructured_impl
} // namespace topo
} // namespace flecsi
