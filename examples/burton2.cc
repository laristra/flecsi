/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

#include <flexi/specializations/burton.h>

#include <vector>

using namespace std;
using namespace flexi;

using vertex_t = burton_mesh_t::vertex_t;
using edge_t = burton_mesh_t::edge_t;
using cell_t = burton_mesh_t::cell_t;
using point_t = burton_mesh_t::point_t;

int main(int argc, char ** argv) {
  size_t width = 2;
  size_t height = 2;

  burton_mesh_t b;

  vector<vertex_t*> vs;
  
  for(size_t j = 0; j < height + 1; ++j){
    for(size_t i = 0; i < width + 1; ++i){
      auto v = b.create_vertex({double(i), double(j)});
      vs.push_back(v);
    }
  }

  size_t width1 = width + 1;
  for(size_t j = 0; j < height; ++j){
    for(size_t i = 0; i < width; ++i){
      auto c = 
        b.create_cell({vs[i + j * width1],
                       vs[i + (j + 1) * width1],
                       vs[i + 1 + j * width1],
                       vs[i + 1 + (j + 1) * width1]});
    }
  }

  b.init();

  return 0;
} // main

/*~------------------------------------------------------------------------~--*
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
