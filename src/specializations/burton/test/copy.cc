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

#include "../burton.h"

using namespace flecsi;

using vertex_t = burton_mesh_t::vertex_t;
using edge_t = burton_mesh_t::edge_t;
using cell_t = burton_mesh_t::cell_t;
using point_t = burton_mesh_t::point_t;
using vector_t = burton_mesh_t::vector_t;
using real_t = burton_mesh_t::real_t;

TEST(copy, copy) {
  burton_mesh_t a;

  const size_t width(10);
  const size_t height(10);

  a.init_parameters((height+1)*(width+1));

  std::vector<vertex_t *> vs;
  for(size_t j = 0; j < height + 1; ++j){
    for(size_t i = 0; i < width + 1; ++i){
      auto v = a.create_vertex({double(i), double(j)});
      vs.push_back(v);
    } // for
  } // for

  size_t width1 = width + 1;
  for(size_t j = 0; j < height; ++j){
    for(size_t i = 0; i < width; ++i){
      // go over vertices counter clockwise to define cell
      auto c = a.create_cell({vs[i + j * width1],
      vs[i + 1 + j * width1],
      vs[i + 1 + (j + 1) * width1],
      vs[i + (j + 1) * width1]});
    } // for
  } // for

  a.init();
  vs.clear();

  burton_mesh_t b;

  b.init_parameters((height+1)*(width+1));

  for(size_t j = 0; j < height + 1; ++j){
    for(size_t i = 0; i < width + 1; ++i){
      auto v = b.create_vertex({double(i)+0.05, double(j)+0.05});
      vs.push_back(v);
    } // for
  } // for

  for(size_t j = 0; j < height; ++j){
    for(size_t i = 0; i < width; ++i){
      // go over vertices counter clockwise to define cell
      auto c = b.create_cell({vs[i + j * width1],
      vs[i + 1 + j * width1],
      vs[i + 1 + (j + 1) * width1],
      vs[i + (j + 1) * width1]});
    } // for
  } // for

  b.init();

  for(auto v : a.vertices()){
    CINCH_CAPTURE() << "----------- vertex id: " << v.id()
      << " with coordinates " << v->coordinates() << std::endl;
  } // for

  for(auto v : b.vertices()){
    CINCH_CAPTURE() << "----------- vertex id: " << v.id()
      << " with coordinates " << v->coordinates() << std::endl;
  } // for

  std::cout << CINCH_DUMP() << std::endl;
} // TEST

/*~------------------------------------------------------------------------~--*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
