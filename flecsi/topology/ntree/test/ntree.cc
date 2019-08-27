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
#include <flecsi/utils/ftest.hh>
#include <flecsi/utils/geometry/point.hh>
#include <flecsi/utils/geometry/filling_curve.hh>

#include <flecsi/topology/ntree/types.hh>

using namespace flecsi;

class tree_policy{
  static constexpr size_t dimension = 3; 
  using element_t = double; 
  using point_t = point_u<element_t,dimension>;  
  using key_t = morton_curve<dimension,int64_t>;
 
  // ------- Tree topology
  using tree_entity_t = flecsi::topology::ntree_entity_holder_u<dimension,key_t>;
  using entity_t = flecsi::topology::ntree_entity_u<dimension,key_t>;
  using node_t = flecsi::topology::ntree_node_u<dimension,tree_entity_t,key_t>;
}; // class tree_policy 

// Register a tree topology 
flecsi_register_topology(ntree_topology_t<tree_policy>,"ntree","test_tree"); 

int
ntree_sanity(int argc, char ** argv) {
  FTEST();
  return 0;
} // TEST
