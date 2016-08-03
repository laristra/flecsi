/*~-------------------------------------------------------------------------~~*
 *  @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
 * /@@/////  /@@          @@////@@ @@////// /@@
 * /@@       /@@  @@@@@  @@    // /@@       /@@
 * /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
 * /@@////   /@@/@@@@@@@/@@       ////////@@/@@
 * /@@       /@@/@@//// //@@    @@       /@@/@@
 * /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
 * //       ///  //////   //////  ////////  // 
 * 
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~-------------------------------------------------------------------------~~*/

#include <cinchtest.h>

#include <vector>
#include <utility>

#include "flecsi/specializations/basic-tree/basic-tree.h"

using namespace std;
using namespace flecsi;
using namespace tree;

double uniform(){
  return double(rand())/RAND_MAX;
}

double uniform(double a, double b){
  return a + (b - a) * uniform();
}

struct policy{
  static const size_t dimension = 2;
  static const size_t max_entities_per_branch = 10;
  using floating_type = double;
};

TEST(basic_tree, test1) {
  using tree_t = basic_tree<policy>;
  using point_t = tree_t::point_t;
  using entity_t = tree_t::entity_t;

  tree_t t;

  vector<entity_t*> ents;

  for(size_t i = 0; i < 100000; ++i){
    point_t p = {uniform(), uniform()};
    auto e = t.make_entity(p);
    t.insert(e);
    ents.push_back(e);
  }

  auto s = t.find_in_radius({0.5, 0.3}, 0.01);

  for(auto e : ents){
    t.remove(e);
  }
}

/*~------------------------------------------------------------------------~--*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
