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

#include "flecsi/util/geometry/filling_curve.hh"
#include "flecsi/util/geometry/point.hh"
#include "flecsi/util/unit.hh"

#include "flecsi/topo/ntree/interface.hh"
#include "flecsi/topo/ntree/types.hh"

using namespace flecsi;

struct sph_tree : topo::specialization<topo::ntree, sph_tree> {
  static constexpr size_t dimension = 3;
  using element_t = double;
  using point_t = util::point<element_t, dimension>;
  using key_t = morton_curve<dimension, int64_t>;

  // ------- Tree topology
  using tree_entity_t = topo::ntree_entity_holder<dimension, key_t>;
  using entity_t = topo::ntree_entity<dimension, key_t>;
  using node_t = topo::ntree_node<dimension, tree_entity_t, key_t>;

  static coloring color() {
    return {};
  } // color
};

sph_tree::slot sph_ntree;
sph_tree::cslot coloring;

int
ntree_driver() {
  UNIT {
    coloring.allocate();
    sph_ntree.allocate(coloring.get());
  };
} // ntree_driver

flecsi::unit::driver<ntree_driver> driver;
