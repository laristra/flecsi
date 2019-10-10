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
#define __FLECSI_PRIVATE__
#include <flecsi/data.hh>

#include <flecsi/utils/ftest.hh>
#include <flecsi/utils/geometry/filling_curve.hh>
#include <flecsi/utils/geometry/point.hh>

#include <flecsi/topology/ntree/interface.hh>
#include <flecsi/topology/ntree/types.hh>

using namespace flecsi;

struct sph_tree_policy {
  static constexpr size_t dimension = 3;
  using element_t = double;
  using point_t = point<element_t, dimension>;
  using key_t = morton_curve<dimension, int64_t>;

  // ------- Tree topology
  using tree_entity_t = flecsi::topology::ntree_entity_holder<dimension, key_t>;
  using entity_t = flecsi::topology::ntree_entity<dimension, key_t>;
  using node_t = flecsi::topology::ntree_node<dimension, tree_entity_t, key_t>;
}; // sph_tree_policy

using sph_ntree_topology = topology::ntree_topology<sph_tree_policy>;
data::topology_reference<sph_ntree_topology> sph_ntree;
data::coloring_slot<sph_ntree_topology> coloring;

int
ntree_driver(int argc, char ** argv) {
  FTEST();

  coloring.allocate();
  sph_ntree.allocate(coloring.get());

  return 0;
} // TEST

ftest_register_driver(ntree_driver);
