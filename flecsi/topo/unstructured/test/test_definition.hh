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
#include "flecsi/util/geometry/point.hh"

#include <vector>

namespace flecsi {
namespace topo {
namespace unstructured_impl {

struct test_definition {
  using point = std::array<double, 3>;
  static constexpr std::size_t dimension() {
    return 2;
  }

  test_definition() {
    ids_.resize(num_entities(2));

    for(size_t c(0); c < num_entities(2); ++c) {
      ids_.push_back(std::vector<size_t>(cells_[c], cells_[c] + 4));
    }
  }

  /// Copy constructor (disabled)
  test_definition(const test_definition &) = delete;

  /// Assignment operator (disabled)
  test_definition & operator=(const test_definition &) = delete;

  /// Destructor
  ~test_definition() {}

  size_t num_entities(size_t topological_dimension) const {
    switch(topological_dimension) {
      case 2:
        return 16;
      case 0:
        return 25;
      default:
        flog_fatal("invalid topological dimension: " << topological_dimension);
    } // switch
  } // num_entities

  std::vector<size_t>
  entities(size_t from_dimension, size_t to_dimension, size_t entity_id) const {
    assert(from_dimension == 2);
    assert(to_dimension == 0);
    std::vector<size_t> ids(4);

    for(size_t i(0); i < 4; ++i) {
      ids[i] = cells_[entity_id][i];
    } // for

    return ids;
  } // vertices

  const std::vector<std::vector<size_t>> & entities(size_t from_dim,
    size_t to_dim) const {
    assert(from_dim == 2);
    assert(to_dim == 0);

    return ids_;
  } // entities

  point vertex(size_t id) const {
    return vertices_[id];
  } // vertex

  std::set<size_t>
  entities_set(size_t from_dimension, size_t to_dimension, size_t id) const {
    auto vvec = entities(from_dimension, to_dimension, id);
    return std::set<size_t>(vvec.begin(), vvec.end());
  } // entities_set

private:
  std::vector<std::vector<size_t>> ids_;

  std::vector<point> vertices_ = {{0.0, 0.0},
    {0.25, 0.0},
    {0.5, 0.0},
    {0.75, 0.0},
    {1.0, 0.0},
    {0.0, 0.25},
    {0.25, 0.25},
    {0.5, 0.25},
    {0.75, 0.25},
    {1.0, 0.25},
    {0.0, 0.5},
    {0.25, 0.5},
    {0.5, 0.5},
    {0.75, 0.5},
    {1.0, 0.5},
    {0.0, 0.75},
    {0.25, 0.75},
    {0.5, 0.75},
    {0.75, 0.75},
    {1.0, 0.75},
    {0.0, 1.0},
    {0.25, 1.0},
    {0.5, 1.0},
    {0.75, 1.0},
    {1.0, 1.0}}; // vertices_

  size_t cells_[16][4] = {{0, 1, 5, 6},
    {1, 2, 6, 7},
    {2, 3, 7, 8},
    {3, 4, 8, 9},
    {5, 6, 10, 11},
    {6, 7, 11, 12},
    {7, 8, 12, 13},
    {8, 9, 13, 14},
    {10, 11, 15, 16},
    {11, 12, 16, 17},
    {12, 13, 17, 18},
    {13, 14, 18, 19},
    {15, 16, 20, 21},
    {16, 17, 21, 22},
    {17, 18, 22, 23},
    {18, 19, 23, 24}}; // cells_

}; // class test_definition

} // namespace unstructured_impl
} // namespace topo
} // namespace flecsi
