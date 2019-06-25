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
#include <flecsi/utils/ftest.h>
#include <flecsi/utils/geometry/point.h>

#include <flecsi/topology/ntree/interface.h>
#include <flecsi/utils/geometry/filling_curve.h>

using namespace flecsi;


// Specialization of an entity
template<size_t DIM, class KEY>
class my_entity_t: public flecsi::topology::ntree_entity_u<DIM,KEY> {};

template<size_t DIM, class KEY>
class my_tree_entity_holder_t:
  public flecsi::topology::ntree_entity_holder_u<DIM,KEY>{};

// Specialization of a tree branch
template<size_t DIM, class TREE_ENTITY_TYPE, class KEY>
class my_branch_t:
  public flecsi::topology::ntree_branch_u<DIM,TREE_ENTITY_TYPE,KEY>{
public:
  my_branch_t():
  flecsi::topology::ntree_branch_u<DIM,TREE_ENTITY_TYPE,KEY>(){}
  my_branch_t(const KEY& k):
  flecsi::topology::ntree_branch_u<DIM,TREE_ENTITY_TYPE,KEY>(k){}
};

struct tree_policy_t{
  using element_t = double;
  static constexpr size_t dimension = 3;
  using point_t = point_u<element_t,dimension>;
  using filling_curve_t = hilbert_curve_u<dimension,uint64_t>;
  using tree_entity_t = my_entity_t<dimension,filling_curve_t>;
  using tree_branch_t = my_branch_t<dimension,tree_entity_t,filling_curve_t>;
  using tree_entity_holder_t = my_tree_entity_holder_t<dimension,filling_curve_t>;
};

int
ntree_sanity(int argc, char ** argv) {
  FTEST();
  flecsi::topology::ntree_topology_u<tree_policy_t> ntree;
  // Add entities in the tree

  return 0;
} // TEST
