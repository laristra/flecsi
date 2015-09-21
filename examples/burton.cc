/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

#include <jali/specializations/burton.h>

#include <vector>

using namespace std;
using namespace jali;

using vertex_t = burton::vertex_t;
using edge_t = burton::edge_t;
using cell_t = burton::cell_t;

int main(int argc, char ** argv) {
  size_t width = 2;
  size_t height = 2;

  burton b;

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

  /*
   * Case A: Wedge flux across control volume boundary facet
   */

  /*
   * Case B: Wedge flux across cell boundary facet
   */

  /*
   * Case C: Wedge quadrature, i.e., cell integral
   */

  /*
   * Case D: Iterator examples
   */

  // wedges@cell

  // wedges@vertex

  // wedges@face

  // wedges@corner



  return 0;
} // main

/*~------------------------------------------------------------------------~--*
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
